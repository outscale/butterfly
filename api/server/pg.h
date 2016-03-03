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

#ifndef API_SERVER_PG_H_
#define API_SERVER_PG_H_

extern "C" {
#include <packetgraph/packetgraph.h>
#include <packetgraph/common.h>
#include <packetgraph/utils/errors.h>
#include <packetgraph/switch.h>
#include <packetgraph/diode.h>
#include <packetgraph/vhost.h>
#include <packetgraph/nic.h>
#include <packetgraph/firewall.h>
#include <packetgraph/hub.h>
#include <packetgraph/print.h>
#include <packetgraph/vtep.h>
#include <packetgraph/antispoof.h>
}
#include <string>

/* Copy of DPDK's ether_addr structure
 * We can't use C++11 and dpdk. */
struct ether_addr {
    uint8_t    addr_bytes[6];
};

/* This is a simple wrapper of packetgraph library.
 * For documentation, you would prefer looking directly at packetgraph's one.
 */
namespace Pg {
    bool start(int argc, char **argv);

    void stop(void);

    int64_t refcount(struct pg_brick *brick);

    bool link(struct pg_brick *target, struct pg_brick *brick);

    void unlink(struct pg_brick *brick);

    bool poll(struct pg_brick *brick, uint16_t *count);

    int64_t pkts_count_get(struct pg_brick *brick, enum pg_side side);

    void destroy(struct pg_brick *brick);

    void fake_destroy(struct pg_brick *brick);

    static inline const char *brick_name(struct pg_brick *brick) {
        return pg_brick_name(brick);
    }

    static inline const char *brick_type(struct pg_brick *brick) {
        return pg_brick_type(brick);
    }

    struct pg_brick *switch_new(const char *name,
                                uint32_t west_max,
                                uint32_t east_max,
                                enum pg_side output);

    struct pg_brick *vhost_new(const char *name,
                uint32_t west_max,
                uint32_t east_max,
                enum pg_side output);

    bool vhost_start(std::string dir);

    void vhost_stop(void);

    const char *vhost_socket_path(struct pg_brick *vhost);

    struct pg_brick *diode_new(const char *name,
                uint32_t west_max,
                uint32_t east_max,
                enum pg_side output);

    void nic_start(void);

    struct pg_brick *nic_new(const char *name, const char *ifname);

    struct pg_brick *nic_new_by_id(const char *name, uint8_t portid);

    void nic_get_mac(struct pg_brick *brick,
             struct ether_addr *addr);

    void nic_get_stats(struct pg_brick *brick, struct pg_nic_stats *stats);

    struct pg_brick *firewall_new(const char *name,
                   uint32_t west_max,
                   uint32_t east_max,
                   uint64_t flags);

    void firewall_gc(struct pg_brick *brick);

    void firewall_rule_flush(struct pg_brick *brick);

    int firewall_rule_add(struct pg_brick *brick, std::string filter,
                          enum pg_side dir, int stateful);

    int firewall_reload(struct pg_brick *brick);

    struct pg_brick *hub_new(const char *name,
                  uint32_t west_max,
                  uint32_t east_max);

    struct pg_brick *vtep_new(const char *name,
                   uint32_t west_max,
                   uint32_t east_max,
                   enum pg_side output,
                   uint32_t ip,
                   struct ether_addr mac,
                   int no_copy);

    struct pg_brick *vtep_new(const char *name,
                   uint32_t west_max,
                   uint32_t east_max,
                   enum pg_side output,
                   std::string ip,
                   struct ether_addr mac,
                   int no_copy);

    struct ether_addr *vtep_get_mac(struct pg_brick *brick);

    void vtep_add_vni(struct pg_brick *brick,
              struct pg_brick *neighbor,
              uint32_t vni,
              uint32_t multicast_ip);

    struct pg_brick *print_new(const char *name,
                uint32_t west_max,
                uint32_t east_max,
                FILE *output,
                int flags,
                uint16_t *type_filter);

    void print_set_flags(struct pg_brick *brick, int flags);
    std::string graph_dot(struct pg_brick *brick);

    struct pg_brick *antispoof_new(const char *name,
                                   uint32_t west_max,
                                   uint32_t east_max,
                                   enum pg_side outside,
                                   struct ether_addr mac);

    void antispoof_arp_enable(struct pg_brick *brick, std::string ip);

    void antispoof_arp_disable(struct pg_brick *brick);

}  // namespace Pg

#endif  // API_SERVER_PG_H_
