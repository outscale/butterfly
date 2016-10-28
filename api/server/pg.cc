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
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <cstddef>
}
#include "api/server/app.h"
#include "api/server/pg.h"

#define PG_ERROR_(e) if (e) { LOG_ERROR_("%s", (e)->message); }

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

        return true;
    }

    bool start(std::string dpdk_args) {
        gint dpdk_argc;
        char **dpdk_argv;
        GError *err;

        dpdk_args.insert(0, "dpdk ");
        app::log.debug(dpdk_args);
        if (!g_shell_parse_argv(dpdk_args.c_str(),
                                &dpdk_argc, &dpdk_argv, &err)) {
            app::log.error("dpdk arguments parsing failed: %s", err->message);
            g_error_free(err);
            return false;
        }
        // From dpdk documentation:
        // """
        // After the call to rte_eal_init(), all arguments argv[x] with
        // x < ret may be modified and should not be accessed by the
        // application.
        // """
        // So using g_strfreev(dpdk_argv) to free dpdk_argv after start will
        // segfault.
        return start(dpdk_argc, dpdk_argv);
    }

    void stop(void) {
        pg_vhost_stop();
        pg_stop();
    }

    bool link(struct pg_brick *target, struct pg_brick *brick) {
        LOG_DEBUG_("link [%s] to [%s]\n",
                   Pg::brick_name(target),
                   Pg::brick_name(brick));
        if (pg_brick_link(target, brick, &errp) < 0) {
            print_and_free_errp();
            return false;
        }
        return true;
    }

    void unlink(struct pg_brick *brick) {
        LOG_DEBUG_("unlink [%s]\n", Pg::brick_name(brick));
        pg_brick_unlink(brick, &errp);
        if (errp)
            print_and_free_errp();
    }

    bool poll(struct pg_brick *brick, uint16_t *count) {
        if (pg_brick_poll(brick, count, &errp) < 0) {
            print_and_free_errp();
            return false;
        }
        return true;
    }

    int64_t pkts_count_get(struct pg_brick *brick, enum pg_side side) {
        return pg_brick_pkts_count_get(brick, side);
    }

    void destroy(struct pg_brick *brick) {
        pg_brick_destroy(brick);
        return;
    }

    void fake_destroy(struct pg_brick *brick) {
        return;
    }

    struct pg_brick *switch_new(const char *name,
                                uint32_t west_max,
                                uint32_t east_max,
                                enum pg_side output) {
        struct pg_brick *ret = pg_switch_new(name, west_max,
                                             east_max, output, &errp);
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
        if (pg_vhost_start(dir.c_str(), &errp) < 0) {
            print_and_free_errp();
            return false;
        }
        return true;
    }

    void vhost_stop(void) {
        pg_vhost_stop();
    }

    const char *vhost_socket_path(struct pg_brick *vhost) {
        // this methode can not fail...
        return pg_vhost_socket_path(vhost, &errp);
    }


    uint64_t rx_bytes(struct pg_brick *brick) {
        return pg_brick_rx_bytes(brick);
    }

    uint64_t tx_bytes(struct pg_brick *brick) {
        return pg_brick_tx_bytes(brick);
    }

    struct pg_brick *diode_new(const char *name,
                               uint32_t west_max,
                               uint32_t east_max,
                               enum pg_side output) {
        struct pg_brick *ret = pg_diode_new(name, output, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    void nic_start(void) {
        pg_nic_start();
    }

    struct pg_brick *nic_new(const char *name, const char *ifname) {
        struct pg_brick *ret = pg_nic_new(name, ifname, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    struct pg_brick *nic_new_by_id(const char *name, uint8_t portid) {
        struct pg_brick *ret = pg_nic_new_by_id(name, portid, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    void nic_get_mac(struct pg_brick *brick, struct ether_addr * addr) {
        g_assert(strcmp(pg_brick_type(brick), "nic") == 0);
        return ::pg_nic_get_mac(brick, addr);
    }

    bool nic_set_mtu(struct pg_brick *brick, uint16_t mtu) {
        if (pg_nic_set_mtu(brick, mtu, &errp) < 0) {
            print_and_free_errp();
            return false;
        }
        return true;
    }

    struct pg_brick *firewall_new(const char *name,
                                  uint32_t west_max,
                                  uint32_t east_max,
                                  uint64_t flags) {
        struct pg_brick *ret = pg_firewall_new(name, west_max,
                               east_max, flags, &errp);
        if (!ret)
            print_and_free_errp();
        printf("\n");
        return ret;
    }

    void firewall_gc(struct pg_brick *brick) {
        g_assert(strcmp(pg_brick_type(brick), "firewall") == 0);
        ::pg_firewall_gc(brick);
    }

    void firewall_rule_flush(struct pg_brick *brick) {
        ::pg_firewall_rule_flush(brick);
    }

    bool firewall_rule_add(struct pg_brick *brick, std::string filter,
                          enum pg_side dir, int stateful) {
        g_assert(strcmp(pg_brick_type(brick), "firewall") == 0);
        if (pg_firewall_rule_add(brick, filter.c_str(), dir,
                                 stateful, &errp) < 0) {
            print_and_free_errp();
            return false;
        }
        return true;
    }

    bool firewall_reload(struct pg_brick *brick) {
        g_assert(strcmp(pg_brick_type(brick), "firewall") == 0);
        if (pg_firewall_reload(brick, &errp) < 0) {
            print_and_free_errp();
            return false;
        }
        return true;
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
                              struct ether_addr mac) {
        struct pg_brick *ret = pg_vtep_new(name, west_max, east_max,
                output, ip, mac, PG_VTEP_DST_PORT, ALL_OPTI, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    struct pg_brick *vtep_new(const char *name,
                              uint32_t west_max,
                              uint32_t east_max,
                              enum pg_side output,
                              std::string ip,
                              struct ether_addr mac) {
        uint32_t ipp;
        inet_pton(AF_INET, ip.c_str(), &ipp);
        return vtep_new(name, west_max, east_max, output, ipp, mac);
    }

    struct ether_addr *vtep_get_mac(struct pg_brick *brick) {
        g_assert(strcmp(pg_brick_type(brick), "vtep") == 0);
        return ::pg_vtep_get_mac(brick);
    }

    void vtep_add_vni(struct pg_brick *brick,
                      struct pg_brick *neighbor,
                      uint32_t vni,
                      uint32_t multicast_ip) {
        g_assert(strcmp(pg_brick_type(brick), "vtep") == 0);
        ::pg_vtep_add_vni(brick, neighbor, vni, multicast_ip, &errp);
        if (errp != NULL)
            print_and_free_errp();
    }

    struct pg_brick *print_new(const char *name,
                               FILE *output,
                               int flags,
                               uint16_t *type_filter) {
        struct pg_brick *ret = pg_print_new(name, output, flags, type_filter,
                                            &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    void print_set_flags(struct pg_brick *brick, int flags) {
        g_assert(strcmp(pg_brick_type(brick), "print") == 0);
        ::pg_print_set_flags(brick, flags);
    }

    std::string graph_dot(struct pg_brick *brick) {
            char buf[10000];
            FILE *fd = fmemopen(buf, 10000, "w+");
        if (pg_brick_dot(brick, fd, &errp) < 0) {
            print_and_free_errp();
            return std::string("");
        }
        fflush(fd);
        std::string ret(buf);
        fclose(fd);
        return ret;
    }

    struct pg_brick *antispoof_new(const char *name,
                                   enum pg_side outside,
                                   struct ether_addr mac) {
        struct pg_brick *ret = pg_antispoof_new(name, outside, &mac, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    void antispoof_arp_enable(struct pg_brick *brick, std::string ip) {
        uint32_t a;
        g_assert(strcmp(pg_brick_type(brick), "antispoof") == 0);
        if (inet_pton(AF_INET, ip.c_str(), static_cast<uint32_t*>(&a)))
            pg_antispoof_arp_enable(brick, a);
    }

    void antispoof_arp_disable(struct pg_brick *brick) {
        g_assert(strcmp(pg_brick_type(brick), "antispoof") == 0);
        pg_antispoof_arp_disable(brick);
    }

    struct pg_brick *tap_new(const char *name,
                             const char *ifname) {
        struct pg_brick *ret = pg_tap_new(name, ifname, &errp);
        if (!ret)
            print_and_free_errp();
        return ret;
    }

    const char *tap_ifname(struct pg_brick *brick) {
        return pg_tap_ifname(brick);
    }

    bool tap_get_mac(struct pg_brick *brick, struct ether_addr *mac) {
        return pg_tap_get_mac(brick, mac) == 0;
    }
}  // namespace Pg
