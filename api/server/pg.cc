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

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstddef>
#include "api/server/app.h"
#include "api/server/pg.h"

#define PG_ERROR_(e) if (e) { LOG_ERROR_((e)->message); }

namespace Pg {
    thread_local static pg_error *errp = NULL;

namespace {
    void print_and_free_errp() {
        PG_ERROR_(errp);
        pg_error_print(errp);
        pg_error_free(errp);
        errp = NULL;
    }
}

    bool start(int argc, char **argv) {
        struct pg_error *errp = NULL;

        /* Start packetgraph */
        pg_start(argc, argv, &errp);
        if (errp) {
            LOG_ERROR_("packetgraph start failed");
            print_and_free_errp();
            return false;
        }

        /* Start NIC */
        nic_start();

        /* Start Vhost */
        pg_vhost_start(app::config.socket_folder.c_str(), &errp);
        if (errp) {
            LOG_ERROR_("packetgraph vhost start failed");
            print_and_free_errp();
            return false;
        }

        return true;
    }

    void stop(void) {
            pg_vhost_stop();
        pg_stop();
    }

    int64_t refcount(struct pg_brick *brick) {
        return pg_brick_refcount(brick);
    }

    bool link(struct pg_brick *target, struct pg_brick *brick) {
        bool ret = pg_brick_link(target, brick, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    void unlink(struct pg_brick *brick) {
        pg_brick_unlink(brick, &errp);
        if (errp)
            print_and_free_errp();
    }

    bool poll(struct pg_brick *brick, uint16_t *count) {
        bool ret = pg_brick_poll(brick, count, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    int64_t pkts_count_get(struct pg_brick *brick, enum pg_side side) {
        return pg_brick_pkts_count_get(brick, side);
    }

    void destroy(struct pg_brick *brick) {
        pg_brick_destroy(brick);
        return;
    }

    struct pg_brick *switch_new(const char *name,
                                uint32_t west_max,
                                uint32_t east_max) {
        struct pg_brick *ret = pg_switch_new(name, west_max,
                             east_max, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    struct pg_brick *vhost_new(const char *name,
                               uint32_t west_max,
                               uint32_t east_max,
                               enum pg_side output) {
        struct pg_brick *ret = pg_vhost_new(name, west_max,
                            east_max, output, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    bool vhost_start(std::string dir) {
        if (!pg_vhost_start(dir.c_str(), &errp)) {
            print_and_free_errp();
            return false;
        }
        return true;
    }

    void vhost_stop(void) {
        pg_vhost_stop();
    }

    struct pg_brick *diode_new(const char *name,
                               uint32_t west_max,
                               uint32_t east_max,
                               enum pg_side output) {
        struct pg_brick *ret = pg_diode_new(name, west_max,
                            east_max, output, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    void nic_start(void) {
        pg_nic_start();
    }

    struct pg_brick *nic_new(const char *name,
                             uint32_t west_max,
                             uint32_t east_max,
                             enum pg_side output,
                             const char *ifname) {
        struct pg_brick *ret = pg_nic_new(name, west_max, east_max,
                          output, ifname, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    struct pg_brick *nic_new_by_id(const char *name,
                                   uint32_t west_max,
                                   uint32_t east_max,
                                   enum pg_side output,
                                   uint8_t portid) {
        struct pg_brick *ret = pg_nic_new_by_id(name, west_max,
                            east_max, output,
                            portid, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    void nic_get_mac(struct pg_brick *brick, struct ether_addr * addr) {
        return ::pg_nic_get_mac(brick, addr);
    }

    struct pg_brick *firewall_new(const char *name,
                                  uint32_t west_max,
                                  uint32_t east_max) {
        struct pg_brick *ret = pg_firewall_new(name, west_max,
                               east_max, PG_NO_CONN_WORKER, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    void firewall_gc(struct pg_brick *brick) {
        ::pg_firewall_gc(brick);
    }

    void firewall_rule_flush(struct pg_brick *brick) {
        ::pg_firewall_rule_flush(brick);
    }

    int firewall_rule_add(struct pg_brick *brick, std::string filter,
                          enum pg_side dir, int stateful) {
        int ret;
        ret = pg_firewall_rule_add(brick, filter.c_str(), dir,
                                   stateful, &errp);
        if (ret)
            print_and_free_errp();
        return ret;
    }

    int firewall_reload(struct pg_brick *brick) {
        return pg_firewall_reload(brick, &errp);
    }

    struct pg_brick *hub_new(const char *name,
                             uint32_t west_max,
                             uint32_t east_max) {
        struct pg_brick *ret = pg_hub_new(name, west_max,
                          east_max, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    struct pg_brick *vtep_new(const char *name,
                              uint32_t west_max,
                              uint32_t east_max,
                              enum pg_side output,
                              uint32_t ip,
                              struct ether_addr mac,
                              int no_copy) {
        struct pg_brick *ret = pg_vtep_new(name, west_max, east_max,
                output, ip, mac, no_copy, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    struct pg_brick *vtep_new(const char *name,
                              uint32_t west_max,
                              uint32_t east_max,
                              enum pg_side output,
                              std::string ip,
                              struct ether_addr mac,
                              int no_copy) {
        uint32_t ipp;
        inet_pton(AF_INET, ip.c_str(), &ipp);
        return vtep_new(name, west_max, east_max, output, ipp, mac, no_copy);
    }

    struct ether_addr *vtep_get_mac(struct pg_brick *brick) {
        return ::pg_vtep_get_mac(brick);
    }

    void vtep_add_vni(struct pg_brick *brick,
                      struct pg_brick *neighbor,
                      uint32_t vni,
                      uint32_t multicast_ip) {
        ::pg_vtep_add_vni(brick, neighbor, vni, multicast_ip, &errp);
        if (errp != NULL)
            print_and_free_errp();
    }

    struct pg_brick *print_new(const char *name,
                   uint32_t west_max,
                   uint32_t east_max,
                   FILE *output,
                   int flags,
                   uint16_t *type_filter) {
        struct pg_brick *ret = pg_print_new(name, west_max, east_max,
                            output, flags, type_filter,
                            &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    void print_set_flags(struct pg_brick *brick, int flags) {
        ::pg_print_set_flags(brick, flags);
    }

    std::string graph_dot(struct pg_brick *brick) {
            char buf[10000];
            FILE *fd = fmemopen(buf, 10000, "w+");
        if (!pg_graph_dot(brick, fd, &errp)) {
            print_and_free_errp();
            return std::string("");
        }
        return std::string(buf);
    }
}  // namespace Pg
