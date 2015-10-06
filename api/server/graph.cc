/* Copyright 2015 Outscale SAS
 *
 * This file is part of Butterfly.
 *
 * Butterfly is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as published
 * by the Free Software Foundation.
 *
 * Butterfly is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Butterfly.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/sysinfo.h>
#include <utility>
#include <thread>
#include <chrono>
#include "api/server/app.h"
#include "api/server/pg.h"
#include "api/server/graph.h"

Graph::Graph(void) {
    // Init rpc queue
    queue = g_async_queue_new();
    started = false;
}

Graph::~Graph(void) {
    stop();
}

bool Graph::linkAndStalk(Graph::Brick eastBrick, Graph::Brick westBrick,
			 Graph::Brick sniffer) {
    if (app::config.packet_trace) {
        if (!Pg::link(eastBrick.get(), sniffer.get()) ||
            !Pg::link(sniffer.get(), westBrick.get())) {
            LOG_ERROR_("Link fail");
            return false;
        }
    } else {
        if (!Pg::link(eastBrick.get(), westBrick.get())) {
            LOG_ERROR_("Link fail");
            return false;
        }
    }
    return true;
}

void Graph::stop() {
    struct rpc_queue *a;

    if (!started)
        return;

    // Stop poller thread
    exit();
    pthread_join(poller_thread, NULL);

    // Empty and unref queue
    a = (struct rpc_queue *)g_async_queue_try_pop(queue);
    while (a != NULL) {
        g_free(a);
        a = (struct rpc_queue *)g_async_queue_try_pop(queue);
    }
    g_async_queue_unref(queue);

    // Byby packetgraph
    vnis.clear();
    Pg::vhost_stop();
    Pg::stop();
    started = false;
}

bool Graph::start(int argc, char **argv) {
    struct ether_addr mac;

    // Start packetgraph
    if (!Pg::start(argc, argv)) {
        return false;
    }

    // Init global for certain bricks
    Pg::nic_start();
    Pg::vhost_start(app::config.socket_folder);
    app::Log::open();

    // Create nic brick
    nic = Brick(Pg::nic_new_by_id("port-0", 1, 1, WEST_SIDE, 0),
                Pg::destroy);
    if (nic.get() == NULL) {
        LOG_ERROR_("brick-nic failed");
        return false;
    }

    // Create sniffer brick
    sniffer = Brick(Pg::print_new("main-sniffer", 1, 1, NULL,
                                  PG_PRINT_FLAG_MAX, NULL),
                    Pg::destroy);
    if (sniffer.get() == NULL) {
        LOG_ERROR_("brick-sniffer failed");
        return false;
    }

    // Create vtep brick
    Pg::nic_get_mac(nic.get(), &mac);
    vtep = Brick(Pg::vtep_new("vxlan", 1, 50, WEST_SIDE,
                       app::config.external_ip, mac, 1),
                       Pg::destroy);
    if (nic.get() == NULL) {
        LOG_ERROR_("brick-vtep failed");
        return false;
    }

    linkAndStalk(nic, vtep, sniffer);

    // Run poller
    pthread_create(&poller_thread, NULL, Graph::poller, this);

    started = true;
    return true;
}

void *Graph::poller(void *graph) {
    Graph *g = reinterpret_cast<Graph *>(graph);
    struct rpc_update_poll *list = NULL;
    struct rpc_queue *q = NULL;
    uint64_t cnt = 0;
    uint16_t pkts_count;
    struct pg_brick *nic = g->nic.get();

    g_async_queue_ref(g->queue);

    // Set CPU affinity for packetgraph processing
    Graph::set_cpu(app::config.graph_core_id);

    /* The main packet poll loop. */
    for (;;) {
        /* Let's see if there is any update every 100 000 pools. */
        if (cnt++ == 100000) {
            cnt = 0;
            if (g->poller_update(&q)) {
                list = q ? &q->update_poll : NULL;
            } else {
                LOG_DEBUG_("poll thread will now exit");
                break;
            }
        }

        if (list != NULL) {
            /* Poll NIC. */
            Pg::poll(nic, &pkts_count);

            /* Poll all pollable vhosts. */
            for (uint32_t v = 0; v < list->size; v++)
                Pg::poll(list->pollables[v], &pkts_count);

            /* Call firewall garbage callector. */
            if (cnt == 50000) {
                for (uint32_t v = 0; v < list->size; v++)
                    Pg::firewall_gc(list->firewalls[v]);
            }
        }
    }
    g_async_queue_unref(g->queue);
    g_free(q);
    pthread_exit(NULL);
}

int Graph::set_cpu(int core_id) {
    cpu_set_t cpu_set;
    pthread_t t;

    if (core_id < 0 || core_id >= get_nprocs())
        return EINVAL;

    t = pthread_self();
    CPU_ZERO(&cpu_set);
    CPU_SET(core_id, &cpu_set);
    return pthread_setaffinity_np(t, sizeof(cpu_set_t), &cpu_set);
}

bool Graph::poller_update(struct rpc_queue **list) {
    struct rpc_queue *a;
    struct rpc_queue *tmp;

    // Unqueue calls
    a = (struct rpc_queue *) g_async_queue_try_pop(queue);
    while (a != NULL) {
        switch (a->action) {
            case EXIT:
                return false;
            case LINK:
                Pg::link(a->link.w, a->link.e);
                break;
            case UNLINK:
                Pg::unlink(a->unlink.b);
                break;
            case ADD_VNI:
                Pg::vtep_add_vni(a->add_vni.vtep,
                                 a->add_vni.neighbor,
                                 a->add_vni.vni,
                                 a->add_vni.multicast_ip);
                break;
            case UPDATE_POLL:
                // Swap with the old list
                tmp = a;
                a = *list;
                *list = tmp;
                break;
            case FW_RELOAD:
                Pg::firewall_reload(a->fw_reload.firewall);
                break;
            case FW_THREAD_REGISTER:
                Pg::firewall_thread_register(a->fw_thread_register.firewall);
                break;
            default:
                LOG_ERROR_("brick poller has wrong RPC value");
                break;
        }
        g_free(a);
        a = (struct rpc_queue *) g_async_queue_try_pop(queue);
    }

    return true;
}

std::string Graph::nic_add(const app::Nic &nic) {
    std::string name;

    if (!started) {
        LOG_ERROR_("Graph has not been stared");
        return "";
    }

    // Create VNI if it does not exists
    auto it = vnis.find(nic.vni);
    if (it == vnis.end()) {
        struct graph_vni v;
        v.vni = nic.vni;
        std::pair<uint32_t, struct graph_vni> p(nic.vni, v);
        vnis.insert(p);
        it = vnis.find(nic.vni);
        if (it == vnis.end())
            return "";
    }
    struct graph_vni &vni = it->second;

    // Create vhost branch
    struct graph_nic gn;
    gn.enable = true;
    gn.id = nic.id;
    name = "firewall-" + gn.id;
    gn.firewall = Brick(Pg::firewall_new(name.c_str(),
                        1, 1, PG_NO_CONN_WORKER), Pg::destroy);
    fw_thread_register(gn.firewall);
    wait_empty_queue();
    name = "antispoof-" + gn.id;
    struct ether_addr mac;
    nic.mac.bytes(mac.addr_bytes);
    gn.antispoof = Brick(Pg::antispoof_new(name.c_str(), 1, 1, EAST_SIDE, mac),
                         Pg::destroy);
    name = "vhost-" + gn.id;
    gn.vhost = Brick(Pg::vhost_new(name.c_str(), 1, 1, WEST_SIDE),
                     Pg::destroy);
    name = "sniffer-" + gn.id;
    gn.sniffer = Brick(Pg::print_new(name.c_str(), 1, 1, NULL,
                                     PG_PRINT_FLAG_MAX, NULL),
		       Pg::destroy);
    // Link branch (inside)
    Pg::link(gn.firewall.get(), gn.antispoof.get());
    linkAndStalk(gn.antispoof, gn.vhost, gn.sniffer);
    // Link branch to the vtep
    if (vni.nics.size() == 0) {
        // Link directly the firewall to the vtep
        link(vtep, gn.firewall);
        add_vni(vtep, gn.firewall, nic.vni);
    } else if (vni.nics.size() == 1) {
        // We have to insert a switch
        // - unlink the first firewall from the graph
        // - link a new switch to the vtep
        // - add the vni on the vtep with the switch
        // - link the first firewall to the switch
        // - link the second firewall to the switch
        name = "switch-" + std::to_string(nic.vni);
        vni.sw = Brick(Pg::switch_new(name.c_str(), 1, 30), Pg::destroy);
        Brick fw1 = vni.nics.begin()->second.firewall;
        unlink(fw1);
        link(vtep, vni.sw);
        add_vni(vtep, vni.sw, vni.vni);
        link(vni.sw, fw1);
        link(vni.sw, gn.firewall);
    } else {
        // Switch already exist, just link the firewall to the switch
        link(vni.sw, gn.firewall);
    }

    // Add branch to the list of NICs
    std::pair<std::string, struct graph_nic> p(nic.id, gn);
    vni.nics.insert(p);

    // Update the list of pollable bricks
    update_poll();

    // Reload the firewall configuration
    fw_update(nic);

    return std::string(Pg::vhost_socket_path(gn.vhost.get()));
}

void Graph::nic_del(const app::Nic &nic) {
    if (!started) {
        LOG_ERROR_("Graph has not been stared");
        return;
    }

    auto vni_it = app::graph.vnis.find(nic.vni);
    if (vni_it == app::graph.vnis.end()) {
        LOG_ERROR_("NIC id: " + nic.id + " in vni: " +
            std::to_string(nic.vni) + " don't seems to exist.");
        return;
    }
    struct graph_vni &vni = vni_it->second;

    auto nic_it = vni.nics.find(nic.id);
    if (nic_it == vni.nics.end()) {
        LOG_ERROR_("NIC id: " + nic.id + " in vni: " +
            std::to_string(nic.vni) + " don't seems to exist in branch.");
        return;
    }

    // Disable branch and update poller
    struct graph_nic &n = nic_it->second;
    n.enable = false;
    update_poll();

    // Disconnect branch from vtep or switch
    if (vni.nics.size() == 1) {
        // We should only have the firewall directly connected to the vtep
        unlink(n.firewall);
    } else if (vni.nics.size() == 2) {
        // We have do:
        // - unlink the switch (which unlink all firewalls).
        // - connect the other firewall to the vtep
        // - destroy the switch

        auto it = vni.nics.begin();
        if (it->second.id == nic.id)
            it++;
        struct graph_nic &other = it->second;
        unlink(vni.sw);
        link(app::graph.vtep, other.firewall);
        wait_empty_queue();
        vni.sw.reset();
    } else {
        // We just have to unlink the firewall from the switch
        unlink(n.firewall);
    }

    // Wait that queue is done before removing bricks
    wait_empty_queue();
    vni.nics.erase(nic_it);

    // Remove empty vni
    if (vni.nics.empty())
        vnis.erase(vni.vni);
}

std::string Graph::nic_export(const app::Nic &nic) {
    if (!started) {
        LOG_ERROR_("Graph has not been stared");
        return "";
    }

    // TODO(jerome.jutteau)
    std::string data = "";
    return data;
}

void Graph::nic_get_stats(const app::Nic &nic, uint64_t *in, uint64_t *out) {
    if (!started) {
        LOG_ERROR_("Graph has not been stared");
        return;
    }
    *in = *out = 0;

    auto vni_it = app::graph.vnis.find(nic.vni);
    if (vni_it == app::graph.vnis.end()) {
        LOG_ERROR_("NIC id: " + nic.id + " in vni: " +
            std::to_string(nic.vni) + " don't seems to exist.");
        return;
    }
    struct graph_vni &vni = vni_it->second;

    auto nic_it = vni.nics.find(nic.id);
    if (nic_it == vni.nics.end()) {
        LOG_ERROR_("NIC id: " + nic.id + " in vni: " +
            std::to_string(nic.vni) + " don't seems to exist in branch.");
        return;
    }

    // TODO(jerome.jutteau) get stats from vhost when this will be available
    /*
    struct pg_vhost_stats stats;
    Pg::vhost_get_stats(nic_it->second.vhost.get(), &stats);
    *in = stats.ibytes;
    *out = stats.obytes;
    */
}

void Graph::nic_config_anti_spoof(const app::Nic &nic, bool enable) {
    if (!started) {
        LOG_ERROR_("Graph has not been stared");
        return;
    }

    auto vni_it = app::graph.vnis.find(nic.vni);
    if (vni_it == app::graph.vnis.end()) {
        LOG_ERROR_("NIC id: " + nic.id + " in vni: " +
            std::to_string(nic.vni) + " don't seems to exist.");
        return;
    }
    struct graph_vni &vni = vni_it->second;

    auto nic_it = vni.nics.find(nic.id);
    if (nic_it == vni.nics.end()) {
        LOG_ERROR_("NIC id: " + nic.id + " in vni: " +
            std::to_string(nic.vni) + " don't seems to exist in branch.");
        return;
    }

    Brick &antispoof = nic_it->second.antispoof;
    if (enable) {
        if (nic.ip_list.size() == 0) {
            LOG_ERROR_("cannot enable ARP antispoof with no given ip for nic " +
                       nic.id);
            return;
        }
        // TODO(jerome.jutteau) patch this when antispoof brick will support
        // several IP for arp antispoofing.
        // Get the first IP of the VM
        std::string ip = nic.ip_list.front().str();
        Pg::antispoof_arp_enable(antispoof.get(), ip);

    } else {
        Pg::antispoof_arp_disable(antispoof.get());
    }
}

std::string Graph::fw_build_rule(const app::Rule &rule) {
    // Note that we only take into account inbound rules
    if (rule.direction == app::Rule::OUTBOUND)
        return "";

    std::string r;

    // Build source
    if (rule.security_group.length() == 0) {
        r += "src net " + rule.cidr.address.str() +
             "/" +  std::to_string(rule.cidr.mask_size);
    } else {
        auto sg = app::model.security_groups.find(rule.security_group);
        if (sg == app::model.security_groups.end()) {
            std::string m = "security group " + rule.security_group +
                            " not available";
            app::log.error(m);
            return "";
        }
        r += " (";
        for (auto ip = sg->second.members.begin();
             ip != sg->second.members.end();) {
            r += " src host " + ip->str();
            if (++ip != sg->second.members.end())
                r += " or";
        }
    }

    // Build protocol part
    r += " and";

    if (rule.protocol == IPPROTO_ICMP)
        r += " ip proto icmp";

    if (rule.protocol == IPPROTO_TCP)
        r += " tcp";

    if (rule.protocol == IPPROTO_UDP)
        r += " udp";

    if (rule.protocol == IPPROTO_TCP || rule.protocol == IPPROTO_UDP) {
        if (rule.port_start < 65536 && rule.port_end < 65536) {
            if (rule.port_start == rule.port_end) {
                r += " dst port " + std::to_string(rule.port_end);
            } else if (rule.port_start < rule.port_end) {
                r += " dst portrange " + std::to_string(rule.port_start) +
                     "-" + std::to_string(rule.port_end);
            } else {
                LOG_ERROR_("invalid port range");
                return "";
            }
        } else {
            LOG_ERROR_("invalid port range");
            return "";
        }
    }

    return r;
}

std::string Graph::fw_build_sg(const app::Sg &sg) {
    std::string r;
    for (auto it = sg.rules.begin(); it != sg.rules.end();) {
        r += "(" + fw_build_rule(it->second) + ")";
        if (++it != sg.rules.end())
            r += " || ";
    }
    return r;
}

void Graph::fw_update(const app::Nic &nic) {
    if (!started) {
        LOG_ERROR_("Graph has not been stared");
        return;
    }

    // Get firewall brick
    auto itvni = vnis.find(nic.vni);
    if (itvni == vnis.end())
        return;
    auto itnic = itvni->second.nics.find(nic.id);
    if (itnic == itvni->second.nics.end())
        return;
    Brick &fw = itnic->second.firewall;

    // For each security groups, build rules inside a BIG one
    std::string in_rules;
    for (auto it = nic.security_groups.begin();
          it != nic.security_groups.end();) {
        auto sit = app::model.security_groups.find(*it);
        if (sit == app::model.security_groups.end()) {
            it++;
            continue;
        }
        in_rules += "( " + fw_build_sg(sit->second) + " )";
        if (++it != nic.security_groups.end())
            in_rules += " || ";
    }

    // Set rules for the outgoing traffic: allow NIC's IPs
    std::string out_rules;
    for (auto it = nic.ip_list.begin(); it != nic.ip_list.end();) {
        out_rules += "(src host " + it->str() + ")";
        if (++it != nic.ip_list.end())
            out_rules += " || ";
    }

    // Push rules to the firewall
    Pg::firewall_rule_flush(fw.get());
    std::string m;
    m = "rules (in) for nic " + nic.id + ": " + in_rules;
    app::log.debug(m);
    m = "rules (out) for nic " + nic.id + ": " + out_rules;
    app::log.debug(m);
    if (in_rules.length() > 0 &&
        Pg::firewall_rule_add(fw.get(), in_rules, WEST_SIDE, 0)) {
        std::string m = "cannot build rules (in) for nic " + nic.id;
        app::log.error(m);
        return;
    }
    if (out_rules.length() > 0 &&
        Pg::firewall_rule_add(fw.get(), out_rules, EAST_SIDE, 1)) {
        std::string m = "cannot build rules (out) for nic " + nic.id;
        app::log.error(m);
        return;
    }

    // Reload firewall
    fw_reload(fw);
}

void Graph::fw_add_rule(const app::Nic &nic, const app::Rule &rule) {
    if (!started) {
        LOG_ERROR_("Graph has not been stared");
        return;
    }

    std::string r = fw_build_rule(rule);
    if (r.length() == 0) {
        std::string m = "cannot build rule (add) for nic " + nic.id;
        app::log.error(m);
    }

    // Get firewall brick
    auto itvni = vnis.find(nic.vni);
    if (itvni == vnis.end())
        return;
    auto itnic = itvni->second.nics.find(nic.id);
    if (itnic == itvni->second.nics.end()) {
        std::string m = "nic " + nic.id + " not found";
        app::log.error(m);
        return;
    }
    Brick &fw = itnic->second.firewall;

    // Add rule & reload firewall
    if (Pg::firewall_rule_add(fw.get(), r, WEST_SIDE, 0)) {
        std::string m = "cannot load rule (add) for nic " + nic.id;
        app::log.error(m);
        app::log.debug(r);
        return;
    }
    Pg::firewall_reload(fw.get());
}

std::string Graph::dot() {
    // Build the graph from the physical NIC
    return Pg::graph_dot(nic.get());
}

void Graph::exit() {
    struct rpc_queue *a = g_new(struct rpc_queue, 1);
    a->action = EXIT;
    g_async_queue_push(queue, a);
}

void Graph::link(Brick w, Brick e) {
    struct rpc_queue *a = g_new(struct rpc_queue, 1);
    a->action = LINK;
    a->link.w = w.get();
    a->link.e = e.get();
    g_async_queue_push(queue, a);
}

void Graph::unlink(Brick b) {
    struct rpc_queue *a = g_new(struct rpc_queue, 1);
    a->action = UNLINK;
    a->unlink.b = b.get();
    g_async_queue_push(queue, a);
}

void Graph::fw_reload(Brick b) {
    struct rpc_queue *a = g_new(struct rpc_queue, 1);
    a->action = FW_RELOAD;
    a->fw_reload.firewall = b.get();
    g_async_queue_push(queue, a);
}

void Graph::fw_thread_register(Brick b) {
    struct rpc_queue *a = g_new(struct rpc_queue, 1);
    a->action = FW_THREAD_REGISTER;
    a->fw_thread_register.firewall = b.get();
    g_async_queue_push(queue, a);
}

void Graph::add_vni(Brick vtep, Brick neighbor, uint32_t vni) {
    uint32_t multicast_ip = build_multicast_ip(vni);
    struct rpc_queue *a = g_new(struct rpc_queue, 1);
    a->action = ADD_VNI;
    a->add_vni.vtep = vtep.get();
    a->add_vni.neighbor = neighbor.get();
    a->add_vni.vni = vni;
    a->add_vni.multicast_ip = multicast_ip;
    g_async_queue_push(queue, a);
}

void Graph::update_poll() {
    // Create a table with all pollable bricks
    std::map<uint32_t, struct graph_vni>::iterator vni_it;
    std::map<std::string, struct graph_nic>::iterator nic_it;
    struct rpc_queue *a = g_new(struct rpc_queue, 1);
    struct rpc_update_poll &p = a->update_poll;

    a->action = UPDATE_POLL;
    // Add physical NIC brick
    p.size = 0;
    // Add all vhost bricks
    for (vni_it = vnis.begin();
            vni_it != vnis.end();
            vni_it++) {
        for (nic_it = vni_it->second.nics.begin();
                nic_it != vni_it->second.nics.end();
                nic_it ++) {
            if (p.size + 1 >= GRAPH_VHOST_MAX_SIZE) {
                LOG_ERROR_("Not enough pollable bricks slot available");
                break;
            }
            if (!nic_it->second.enable)
                continue;
            p.pollables[p.size] = nic_it->second.vhost.get();
            p.firewalls[p.size] = nic_it->second.firewall.get();
            p.size++;
        }
    }

    // Pass this new listing to packetgraph thread
    g_async_queue_push(queue, a);
}

void Graph::wait_empty_queue() {
    while (g_async_queue_length_unlocked(queue) > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

uint32_t Graph::build_multicast_ip(uint32_t vni) {
    // Build mutlicast IP, CIDR: 224.0.0.0/4
    // (224.0.0.0 to 239.255.255.255)
    // 224 and 239 are already used.
    uint32_t multicast_ip = htonl(vni);
    reinterpret_cast<uint8_t *>(& multicast_ip)[0] = 230;
    return multicast_ip;
}
