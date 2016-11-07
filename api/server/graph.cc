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

extern "C" {
#include <glib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/sysinfo.h>
#include <sys/syscall.h>
#include <unistd.h>
}
#include <utility>
#include <thread>
#include <chrono>
#include "api/server/app.h"
#include "api/server/graph.h"

namespace {
void pg_fake_destroy(struct pg_brick *) {}
}

Graph::Graph(void) {
    // Init rpc queue
    queue_ = g_async_queue_new();
    started = false;
}

Graph::~Graph(void) {
    stop();
}

bool Graph::linkAndStalk(Graph::Brick eastBrick, Graph::Brick westBrick,
        Graph::Brick sniffer) {
    if (app::config.packet_trace) {
        if (pg_brick_chained_links(&app::pg_error, eastBrick.get(),
                                   sniffer.get(), westBrick.get()) < 0) {
            PG_ERROR_(app::pg_error);
            return false;
        }
    } else {
        if (pg_brick_link(eastBrick.get(), westBrick.get(),
                          &app::pg_error) < 0) {
            PG_ERROR_(app::pg_error);
            return false;
        }
    }
    return true;
}

void Graph::stop() {
    struct RpcQueue *a;
    std::map<std::string, app::Nic>::iterator n_it;

    if (!started)
        return;

    // Remove all NICs
    for (n_it = app::model.nics.begin();
         n_it != app::model.nics.end();
         n_it++) {
        nic_del(n_it->second);
    }

    // Stop vhost
    vhost_stop();

    // Stop poller thread
    exit();
    pthread_join(poller_thread, NULL);

    // Empty and unref queue
    a = (struct RpcQueue *)g_async_queue_try_pop(queue_);
    while (a != NULL) {
        g_free(a);
        a = (struct RpcQueue *)g_async_queue_try_pop(queue_);
    }
    g_async_queue_unref(queue_);

    // Byby packetgraph
    vnis_.clear();
    pg_stop();
    app::destroy_cgroup();
    started = false;
}

bool Graph::start(std::string dpdk_args) {
    struct ether_addr mac;

    // Start packetgraph
    if (!app::pg_start(dpdk_args)) {
        return false;
    }

    // DPDK open log for us and we WANT our logs back !
    app::Log::open();

    // Start Vhost
    vhost_start();

    // Create nic brick
    nic_ = Brick(pg_nic_new_by_id("port-0", 0, &app::pg_error),
                 pg_brick_destroy);
    if (nic_.get() == NULL) {
        LOG_WARNING_("cannot use DPDK port 0");
        // Try to create a pcap interface instead
        nic_ = Brick(pg_tap_new("port-0", NULL, &app::pg_error),
                     pg_brick_destroy);
        if (nic_.get() == NULL) {
            LOG_ERROR_("cannot create tap interface");
            return false;
        } else if (pg_tap_get_mac(nic_.get(), &mac) < 0) {
             LOG_ERROR_("cannot get mac of tap interface");
             return false;
        } else {
            LOG_INFO_("created tap interface %s", pg_tap_ifname(nic_.get()));
        }
    } else {
        // Try to increase mtu
        if (pg_nic_set_mtu(nic_.get(), 2000, &app::pg_error) < 0) {
            PG_WARNING_(app::pg_error);
        }
        pg_nic_get_mac(nic_.get(), &mac);
    }

    // Create sniffer brick
    pcap_file_ = fopen(("/tmp/butterfly-" + std::to_string(getpid()) +
                        "-main.pcap").c_str(), "w");
    std::string sniffer_name = "main-sniffer-" + std::to_string(getpid());
    sniffer_ = Brick(pg_print_new(sniffer_name.c_str(), pcap_file_,
                                  PG_PRINT_FLAG_PCAP | PG_PRINT_FLAG_CLOSE_FILE,
                                  NULL, &app::pg_error),
                    pg_brick_destroy);
    if (sniffer_.get() == NULL) {
        PG_ERROR_(app::pg_error);
        return false;
    }

    // Create vtep brick
    uint32_t ipp;
    inet_pton(AF_INET, app::config.external_ip.c_str(), &ipp);
    vtep_ = Brick(pg_vtep_new("vxlan", 1, 50, WEST_SIDE, ipp, mac,
                              PG_VTEP_DST_PORT, ALL_OPTI, &app::pg_error),
                  pg_brick_destroy);
    if (vtep_.get() == NULL) {
        PG_ERROR_(app::pg_error);
        return false;
    }

    linkAndStalk(nic_, vtep_, sniffer_);

    // Run poller
    pthread_create(&poller_thread, NULL, Graph::poller, this);

    started = true;
    return true;
}

#define POLLER_CHECK(c) ((c) & 1023)
#define FIREWALL_GC(c) ((c) == 100000)
void *Graph::poller(void *graph) {
    Graph *g = reinterpret_cast<Graph *>(graph);
    struct RpcUpdatePoll *list = NULL;
    struct RpcQueue *q = NULL;
    uint16_t pkts_count;
    struct pg_brick *nic = g->nic_.get();
    uint32_t size = 0;

    g_async_queue_ref(g->queue_);

    // Set CPU affinity for packetgraph processing
    Graph::set_cpu(app::config.graph_core_id);
    Graph::set_sched();

    /* The main packet poll loop. */
    for (uint32_t cnt = 0;; ++cnt) {
        /* Let's see if there is any update every 100 000 pools. */

        if (POLLER_CHECK(cnt)) {
            if (g->poller_update(&q)) {
                if (q) {
                list = &q->update_poll;
                size = list->size;
                }
            } else {
                LOG_DEBUG_("poll thread will now exit");
                break;
            }
        }

        /* Poll all pollable vhosts. */
        if (pg_brick_poll(nic, &pkts_count, &app::pg_error) < 0)
            PG_ERROR_(app::pg_error);
        for (uint32_t v = 0; v < size; v++) {
            if (pg_brick_poll(list->pollables[v],
                              &pkts_count, &app::pg_error) < 0) {
                PG_ERROR_(app::pg_error);
            }
        }
        sched_yield();

        /* Call firewall garbage callector. */
        if (FIREWALL_GC(cnt)) {
            cnt = 0;
            for (uint32_t v = 0; v < size; v++)
                pg_firewall_gc(list->firewalls[v]);
            usleep(5);
        }
    }
    g_async_queue_unref(g->queue_);
    g_free(q);
    pthread_exit(NULL);
}
#undef POLLER_CHECK
#undef FIREWALL_GC

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


#define gettid() syscall(SYS_gettid)

int Graph::set_sched() {
  app::config.tid = gettid();
  return 0;
}
#undef gettid

bool Graph::poller_update(struct RpcQueue **list) {
    struct RpcQueue *a;
    struct RpcQueue *tmp;

    // Unqueue calls
    a = (struct RpcQueue *) g_async_queue_try_pop(queue_);
    while (a != NULL) {
        switch (a->action) {
            case EXIT:
                g_free(a);
                return false;
            case VHOST_START:
                if (pg_vhost_start(app::config.socket_folder.c_str(),
                                   &app::pg_error) < 0) {
                    PG_ERROR_(app::pg_error);
                }
                break;
            case VHOST_STOP:
                pg_vhost_stop();
                break;
            case LINK:
                pg_brick_link(a->link.w, a->link.e, &app::pg_error);
                break;
            case UNLINK:
                pg_brick_unlink(a->unlink.b, &app::pg_error);
                break;
            case ADD_VNI:
                pg_vtep_add_vni(a->add_vni.vtep,
                                a->add_vni.neighbor,
                                a->add_vni.vni,
                                a->add_vni.multicast_ip,
                                &app::pg_error);
                break;
            case UPDATE_POLL:
                // Swap with the old list
                tmp = a;
                a = *list;
                *list = tmp;
                break;
            case FW_RELOAD:
                pg_firewall_reload(a->fw_reload.firewall, &app::pg_error);
                break;
            case FW_NEW:
                *(a->fw_new.result) = pg_firewall_new(a->fw_new.name,
                                                      a->fw_new.west_max,
                                                      a->fw_new.east_max,
                                                      a->fw_new.flags,
                                                      &app::pg_error);
                break;
            case BRICK_DESTROY:
                pg_brick_destroy(a->brick_destroy.b);
                break;

            default:
                LOG_ERROR_("brick poller has wrong RPC value");
                break;
        }
        g_free(a);
        a = (struct RpcQueue *) g_async_queue_try_pop(queue_);
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
    auto it = vnis_.find(nic.vni);
    if (it == vnis_.end()) {
        struct GraphVni v;
        v.vni = nic.vni;
        std::pair<uint32_t, struct GraphVni> p(nic.vni, v);
        vnis_.insert(p);
        it = vnis_.find(nic.vni);
        if (it == vnis_.end())
            return "";
    }
    struct GraphVni &vni = it->second;

    // Create vhost branch
    struct GraphNic gn;
    struct pg_brick *tmp_fw = NULL;

    gn.enable = true;
    gn.id = nic.id;
    name = "firewall-" + gn.id;
    fw_new(name.c_str(), 1, 1, PG_NO_CONN_WORKER, &tmp_fw);
    wait_empty_queue();
    if (tmp_fw == NULL) {
        LOG_ERROR_("Firewall creation failed");
        return "";
    }
    gn.firewall = Brick(tmp_fw, pg_fake_destroy);
    name = "antispoof-" + gn.id;
    struct ether_addr mac;
    nic.mac.bytes(mac.ether_addr_octet);
    gn.antispoof = Brick(pg_antispoof_new(name.c_str(), WEST_SIDE, &mac,
                                          &app::pg_error),
                         pg_brick_destroy);
    if (!gn.antispoof) {
        LOG_ERROR_("%s creation failed", name.c_str());
        return "";
    }

    if (nic.ip_anti_spoof) {
        // TODO(jerome.jutteau) patch this when antispoof brick will support
        // several IP for arp antispoofing.
        // Get the first IP of the VM
        std::string ip = nic.ip_list.front().str();
        uint32_t ipp;
        inet_pton(AF_INET, ip.c_str(), &ipp);
        pg_antispoof_arp_enable(gn.antispoof.get(), ipp);
    }

    name = "vhost-" + gn.id;
    gn.vhost = Brick(pg_vhost_new(name.c_str(), 1, 1, EAST_SIDE,
                                  &app::pg_error),
                     pg_brick_destroy);
    if (!gn.vhost) {
        LOG_ERROR_("%s creation failed", name.c_str());
        return "";
    }
    name = "sniffer-" + gn.id;
    gn.pcap_file = fopen(("/tmp/butterfly-" + std::to_string(getpid()) + "-" +
                          gn.id + ".pcap").c_str(), "w");
    gn.sniffer = Brick(pg_print_new(name.c_str(), gn.pcap_file,
                                    PG_PRINT_FLAG_PCAP |
                                    PG_PRINT_FLAG_CLOSE_FILE,
                                    NULL, &app::pg_error),
                       pg_brick_destroy);
    if (!gn.sniffer) {
        LOG_ERROR_("%s creation failed", name.c_str());
        return "";
    }

    // Link branch (inside)
    pg_brick_link(gn.firewall.get(), gn.antispoof.get(), &app::pg_error);
    linkAndStalk(gn.antispoof, gn.vhost, gn.sniffer);
    // Link branch to the vtep
    if (vni.nics.size() == 0) {
        // Link directly the firewall to the vtep
        link(vtep_, gn.firewall);
        add_vni(vtep_, gn.firewall, nic.vni);
    } else if (vni.nics.size() == 1) {
        // We have to insert a switch
        // - unlink the first firewall from the graph
        // - link a new switch to the vtep
        // - add the vni on the vtep with the switch
        // - link the first firewall to the switch
        // - re-link the first firewall to it's antispoof
        // - link the second firewall to the switch
        name = "switch-" + std::to_string(nic.vni);
        vni.sw = Brick(pg_switch_new(name.c_str(), 1, 30, EAST_SIDE,
                                     &app::pg_error), pg_brick_destroy);
        if (!vni.sw) {
            LOG_ERROR_("%s creation failed", name.c_str());
            return "";
        }

        Brick fw1 = vni.nics.begin()->second.firewall;
        Brick as1 = vni.nics.begin()->second.antispoof;
        unlink(fw1);
        link(vtep_, vni.sw);
        add_vni(vtep_, vni.sw, vni.vni);
        link(vni.sw, fw1);
        link(fw1, as1);
        link(vni.sw, gn.firewall);
    } else {
        // Switch already exist, just link the firewall to the switch
        link(vni.sw, gn.firewall);
    }

    // Add branch to the list of NICs
    std::pair<std::string, struct GraphNic> p(nic.id, gn);
    vni.nics.insert(p);

    // Update the list of pollable bricks
    update_poll();

    // Reload the firewall configuration
    fw_update(nic);
    app::set_cgroup();

    return std::string(pg_vhost_socket_path(gn.vhost.get(), &app::pg_error));
}

void Graph::nic_del(const app::Nic &nic) {
    if (!started) {
        LOG_ERROR_("Graph has not been stared");
        return;
    }

    auto vni_it = app::graph.vnis_.find(nic.vni);
    if (vni_it == app::graph.vnis_.end()) {
        LOG_ERROR_("NIC id: " + nic.id + " in vni: " +
            std::to_string(nic.vni) + " don't seems to exist.");
        return;
    }
    struct GraphVni &vni = vni_it->second;

    auto nic_it = vni.nics.find(nic.id);
    if (nic_it == vni.nics.end()) {
        LOG_ERROR_("NIC id: " + nic.id + " in vni: " +
            std::to_string(nic.vni) + " don't seems to exist in branch.");
        return;
    }

    // Disable branch and update poller
    struct GraphNic &n = nic_it->second;
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
        struct GraphNic &other = it->second;
        unlink(vni.sw);
        link(app::graph.vtep_, other.firewall);
        wait_empty_queue();
        vni.sw.reset();
    } else {
        // We just have to unlink the firewall from the switch
        unlink(n.firewall);
    }

    // Delete firewall in the processing thread
    brick_destroy(n.firewall);

    // Wait that queue is done before removing bricks
    wait_empty_queue();
    vni.nics.erase(nic_it);

    // Remove empty vni
    if (vni.nics.empty())
        vnis_.erase(vni.vni);
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

    auto vni_it = app::graph.vnis_.find(nic.vni);
    if (vni_it == app::graph.vnis_.end()) {
        LOG_ERROR_("NIC id: " + nic.id + " in vni: " +
            std::to_string(nic.vni) + " don't seems to exist.");
        return;
    }
    struct GraphVni &vni = vni_it->second;

    auto nic_it = vni.nics.find(nic.id);
    if (nic_it == vni.nics.end()) {
        LOG_ERROR_("NIC id: " + nic.id + " in vni: " +
            std::to_string(nic.vni) + " don't seems to exist in branch.");
        return;
    }

    *in = pg_brick_rx_bytes(nic_it->second.vhost.get());
    *out = pg_brick_tx_bytes(nic_it->second.vhost.get());
}

void Graph::nic_config_anti_spoof(const app::Nic &nic, bool enable) {
    if (!started) {
        LOG_ERROR_("Graph has not been stared");
        return;
    }

    auto vni_it = app::graph.vnis_.find(nic.vni);
    if (vni_it == app::graph.vnis_.end()) {
        LOG_ERROR_("NIC id: " + nic.id + " in vni: " +
            std::to_string(nic.vni) + " don't seems to exist.");
        return;
    }
    struct GraphVni &vni = vni_it->second;

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
        uint32_t ipp;
        inet_pton(AF_INET, ip.c_str(), &ipp);
        pg_antispoof_arp_enable(antispoof.get(), ipp);

    } else {
        pg_antispoof_arp_disable(antispoof.get());
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
        if (sg->second.members.size() > 0) {
            r += " (";
            for (auto ip = sg->second.members.begin();
                 ip != sg->second.members.end();) {
                r += " src host " + ip->str();
                if (++ip != sg->second.members.end())
                    r += " or";
            }
            r += ")";
        } else {
            std::string m = "no member in security group " + sg->second.id;
            app::log.warning(m);
            return "";
        }
    }

    // Build protocol part
    switch (rule.protocol) {
    case IPPROTO_ICMP:
        r += " and icmp";
        break;
    case IPPROTO_TCP:
        r += " and tcp";
        break;
    case IPPROTO_UDP:
        r += " and udp";
        break;
    case -1:
        // Allow all
        break;
    default:
        LOG_ERROR_("unsupported protocol");
        return "";
    }

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
        std::string fw_rule = fw_build_rule(it->second);
        if (fw_rule.length() == 0) {
            it++;
            continue;
        }
        r += "(" + fw_rule + ")";
        if (++it != sg.rules.end())
            r += "||";
    }
    // Special case when last rule is empty
    if (r.back() == '|') {
        r.pop_back();
        r.pop_back();
    }
    return r;
}

void Graph::fw_update(const app::Nic &nic) {
    if (!started) {
        LOG_ERROR_("Graph has not been stared");
        return;
    }

    // Get firewall brick
    auto itvni = vnis_.find(nic.vni);
    if (itvni == vnis_.end())
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
        std::string sg_rules = fw_build_sg(sit->second);
        if (sg_rules.length() == 0) {
            it++;
            continue;
        }
        in_rules += "(" + fw_build_sg(sit->second) + ")";
        if (++it != nic.security_groups.end())
            in_rules += "||";
    }
    // Special case when last rule is empty
    if (in_rules.back() == '|') {
        in_rules.pop_back();
        in_rules.pop_back();
    }

    // Set rules for the outgoing traffic: allow NIC's IPs
    std::string out_rules;
    for (auto it = nic.ip_list.begin(); it != nic.ip_list.end();) {
        out_rules += "(src host " + it->str() + ")";
        if (++it != nic.ip_list.end())
            out_rules += " || ";
    }

    // Push rules to the firewall
    pg_firewall_rule_flush(fw.get());
    std::string m;
    m = "rules (in) for nic " + nic.id + ": " + in_rules;
    app::log.debug(m);
    m = "rules (out) for nic " + nic.id + ": " + out_rules;
    app::log.debug(m);
    if (in_rules.length() > 0 &&
        (pg_firewall_rule_add(fw.get(), in_rules.c_str(), WEST_SIDE,
                              0, &app::pg_error) < 0)) {
        std::string m = "cannot build rules (in) for nic " + nic.id;
        app::log.error(m);
        return;
    }
    if (out_rules.length() > 0 &&
        pg_firewall_rule_add(fw.get(), out_rules.c_str(), EAST_SIDE,
                             1,  &app::pg_error) < 0) {
        std::string m = "cannot build rules (out) for nic " + nic.id;
        app::log.error(m);
        return;
    }

    // Reload firewall
    fw_reload(fw);
}

void Graph::fw_add_rule(const app::Nic &nic, const app::Rule &rule) {
    std::string m;
    if (!started) {
        LOG_ERROR_("Graph has not been stared");
        return;
    }

    std::string r = fw_build_rule(rule);
    if (r.length() == 0) {
        m = "cannot build rule (add) for nic " + nic.id;
        app::log.error(m);
        return;
    }

    m = "adding new rule to firewall of nic " + nic.id + ": " + r;
    app::log.debug(m);

    // Get firewall brick
    auto itvni = vnis_.find(nic.vni);
    if (itvni == vnis_.end()) {
        LOG_ERROR_("cannot find nic's firewall");
        return;
    }
    auto itnic = itvni->second.nics.find(nic.id);
    if (itnic == itvni->second.nics.end()) {
        m = "nic " + nic.id + " not found";
        app::log.error(m);
        return;
    }
    Brick &fw = itnic->second.firewall;

    // Add rule & reload firewall
    if (pg_firewall_rule_add(fw.get(), r.c_str(), WEST_SIDE,
                             0, &app::pg_error) < 0) {
        m = "cannot load rule (add) for nic " + nic.id;
        app::log.error(m);
        app::log.debug(r);
        return;
    }
    fw_reload(fw);
}

std::string Graph::dot() {
    // Build the graph from the physical NIC
    return app::graph_dot(nic_.get());
}

void Graph::exit() {
    struct RpcQueue *a = g_new(struct RpcQueue, 1);
    a->action = EXIT;
    g_async_queue_push(queue_, a);
}

void Graph::vhost_start() {
    struct RpcQueue *a = g_new(struct RpcQueue, 1);
    a->action = VHOST_START;
    g_async_queue_push(queue_, a);
}

void Graph::vhost_stop() {
    struct RpcQueue *a = g_new(struct RpcQueue, 1);
    a->action = VHOST_STOP;
    g_async_queue_push(queue_, a);
}

void Graph::link(Brick w, Brick e) {
    struct RpcQueue *a = g_new(struct RpcQueue, 1);
    a->action = LINK;
    a->link.w = w.get();
    a->link.e = e.get();
    g_async_queue_push(queue_, a);
}

void Graph::unlink(Brick b) {
    struct RpcQueue *a = g_new(struct RpcQueue, 1);
    a->action = UNLINK;
    a->unlink.b = b.get();
    g_async_queue_push(queue_, a);
}

void Graph::fw_reload(Brick b) {
    struct RpcQueue *a = g_new(struct RpcQueue, 1);
    a->action = FW_RELOAD;
    a->fw_reload.firewall = b.get();
    g_async_queue_push(queue_, a);
}

void Graph::fw_new(const char *name,
                   uint32_t west_max,
                   uint32_t east_max,
                   uint64_t flags,
                   struct pg_brick **result) {
    struct RpcQueue *a = g_new(struct RpcQueue, 1);
    a->action = FW_NEW;
    a->fw_new.name = name;
    a->fw_new.west_max = west_max;
    a->fw_new.east_max = east_max;
    a->fw_new.flags = flags;
    a->fw_new.result = result;
    g_async_queue_push(queue_, a);
}

void Graph::brick_destroy(Brick b) {
    struct RpcQueue *a = g_new(struct RpcQueue, 1);
    a->action = BRICK_DESTROY;
    a->brick_destroy.b = b.get();
    g_async_queue_push(queue_, a);
}

void Graph::add_vni(Brick vtep, Brick neighbor, uint32_t vni) {
    uint32_t multicast_ip = build_multicast_ip(vni);
    struct RpcQueue *a = g_new(struct RpcQueue, 1);
    a->action = ADD_VNI;
    a->add_vni.vtep = vtep.get();
    a->add_vni.neighbor = neighbor.get();
    a->add_vni.vni = vni;
    a->add_vni.multicast_ip = multicast_ip;
    g_async_queue_push(queue_, a);
}

void Graph::update_poll() {
    // Create a table with all pollable bricks
    std::map<uint32_t, struct GraphVni>::iterator vni_it;
    std::map<std::string, struct GraphNic>::iterator nic_it;
    struct RpcQueue *a = g_new(struct RpcQueue, 1);
    struct RpcUpdatePoll &p = a->update_poll;

    a->action = UPDATE_POLL;
    // Add physical NIC brick
    p.size = 0;
    // Add all vhost bricks
    for (vni_it = vnis_.begin();
            vni_it != vnis_.end();
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
    g_async_queue_push(queue_, a);
}

void Graph::wait_empty_queue() {
    while (g_async_queue_length_unlocked(queue_) > 0)
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
