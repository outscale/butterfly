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

#ifndef API_SERVER_GRAPH_H_
#define API_SERVER_GRAPH_H_

extern "C" {
#include <glib.h>
}
#include <mutex>
#include <memory>
#include <map>
#include <string>
#include <vector>
#include "api/server/app.h"

#define GRAPH_VHOST_MAX_SIZE 50

class Graph {
 public:
    Graph();
    ~Graph();
    /** Prepare the common part of the graph and run the poll thread
     * The poll thread is responsible of getting packets from all pollable
     * bricks of the graph.
     * Sometime, the thread release a mutex permetting the API to change
     * the graph configuration.
     * @param  dpdk_args dpdk arguments in one string
     * @return true if graph has started correctly, false otherwise
     */
    bool Start(std::string dpdk_args);
    /** Exit and clean graph. */
    void Stop(void);
    /** Inject a NIC in packetgraph including all needed bricks
     * @param  nic model and informations of the NIC.
     *         the object may be modified to add creation informations
     *         like vhost-user path and so.
     * @return  true if Nic has been well created, false otherwise.
     */
    bool NicAdd(app::Nic *nic);
    /** Remove a NIC from the graph with related dependencies.
     * @param  id NIC id to delete.
     */
    void NicDel(const app::Nic &nic);
    /** Get a snapshot containing the current firewall state attached to the NIC.
     * @param  id id of the NIC attached to the firewall.
     * @return an opaque string containing firewall's state data.
     */
    std::string NicExport(const app::Nic &nic);
    /** Get NIC statistics.
     * @param  id id of the NIC
     * @param  in number of bytes who has been received by the NIC
     * @param  out number of bytes who has been transmitted by the NIC
     */
    void NicGetStats(const app::Nic &nic, uint64_t *in, uint64_t *out);
    /** Enable on disable IP antispoof on the NIC.
     * @param  id id of the NIC
     * @param  enable true to enable IP antispoof, false otherwise
     */
    void NicConfigAntiSpoof(const app::Nic &nic, bool enable);
    /** Rebuild all firewall rules of a NIC
     * @param  nic model of the NIC
     */
    void FwUpdate(const app::Nic &nic);
    /** Add a single rule in the corresponding firewall of a NIC.
     * @param  nic nic model
     * @param  rule model of the rule
     */
    void FwAddRule(const app::Nic &nic, const app::Rule &rule);
    /** Build a graphic description in dot language (graphviz project).
     * @return  a string describing the whole graph
     */
    std::string Dot();

 private:
    /* Check if graph has been started or not. */
    bool started;

    /* Define a smart pointer for bricks. */
    typedef std::shared_ptr<struct pg_brick> BrickShrPtr;

    /* All possible structure to pass through the queue.
     * This should ONLY concern actions who needs the graph to be stopped.
     */
    enum RpcAction {
        EXIT,
        VHOST_START,
        VHOST_STOP,
        LINK,
        UNLINK,
        ADD_VNI,
        UPDATE_POLL,
        FW_RELOAD,
        FW_NEW,
        BRICK_DESTROY,
    };

    struct RpcLink {
        struct pg_brick *w;
        struct pg_brick *e;
    };

    struct RpcUnlink {
        struct pg_brick *b;
    };

    struct RpcAddVni {
        struct pg_brick *vtep;
        struct pg_brick *neighbor;
        uint32_t vni;
        union {
            uint32_t multicast_ip4;
            uint8_t multicast_ip6[16];
        };
    };

    struct RpcFwReload {
        struct pg_brick *firewall;
    };

    struct RpcFwNew {
        const char *name;
        uint32_t west_max;
        uint32_t east_max;
        uint64_t flags;
        struct pg_brick **result;
    };

    struct RpcBrickDestroy {
        struct pg_brick *b;
    };

    // This rpc message is kept by the poller
    struct RpcUpdatePoll {
        struct pg_brick *pollables[GRAPH_VHOST_MAX_SIZE];
        struct pg_brick *firewalls[GRAPH_VHOST_MAX_SIZE];
        uint32_t size;
    };

    struct RpcQueue {
        enum RpcAction action;
        struct RpcLink link;
        struct RpcUnlink unlink;
        struct RpcAddVni add_vni;
        struct RpcUpdatePoll update_poll;
        struct RpcFwReload fw_reload;
        struct RpcFwNew fw_new;
        struct RpcBrickDestroy brick_destroy;
    };

    /* Wrappers to ease RPC actions. */
    void exit();
    void vhost_start();
    void vhost_stop();
    void link(BrickShrPtr w, BrickShrPtr e);
    void unlink(BrickShrPtr b);
    void add_vni(BrickShrPtr vtep, BrickShrPtr neighbor, uint32_t vni);
    void update_poll();
    void fw_reload(BrickShrPtr b);
    void fw_new(const char *name,
                uint32_t west_max,
                uint32_t east_max,
                uint64_t flags,
                struct pg_brick **result);
    void brick_destroy(BrickShrPtr b);
    void WaitEmptyQueue();

    /** Threaded function to poll graph. */
    static void *Poller(void *g);
    pthread_t poller_thread;
    /**
     * Set scheduler affinity so the current thread only run a on a
     * specific CPU.
     */
    static inline int SetCpu(int core_id);
    static inline int SetSched();

    /* Set physical nic MTU from config. */
    inline void SetConfigMtu();

    /**
     * Called by the poller, run all pending actions in the queue
     * @param    list list of bricks the poller needs (struct RpcUpdatePoll)
     * @return  true if poller must continue polling, otherwhise exit.
     */
    inline bool PollerUpdate(struct RpcQueue **list);

    /**
     * Build a rule string based on a rule model
     * @param   a list of security groups to apply
     * @return  return a pcap filter string of the rule
     */
    std::string FwBuildRule(const app::Rule &rule);

    /**
     * Build a big rule string based on a security group
     * @param   a list of security groups to apply
     * @return  return a pcap filter rule of the whole SG
     */
    std::string FwBuildSg(const app::Sg &sg);

    /**
     * Try to link @eastBrick to @westBrick, and add @sniffer betwin those
     * bricks
     */
    bool LinkAndStalk(BrickShrPtr eastBrick, BrickShrPtr westBrick,
                      BrickShrPtr sniffer);

    /* VM branch. */
    struct GraphNic {
       std::string id;
       // head is a pointer to the first brick in the branch
       BrickShrPtr head;
       BrickShrPtr firewall;
       BrickShrPtr antispoof;
       BrickShrPtr vhost;
       BrickShrPtr sniffer;
       FILE *pcap_file;
       // If we should add this branch or not to our poll updates
       bool enable;
    };

    /* VNI branch. */
    struct GraphVni {
       uint32_t vni;
       /* Switch brick */
       BrickShrPtr sw;
       /* nic id -> nic branch */
       std::map<std::string, struct GraphNic> nics;
    };

    GraphNic *FindNic(const app::Nic &nic);

    /* Global branch. */
    BrickShrPtr nic_;
    BrickShrPtr vtep_;
    bool isVtep6_;
    BrickShrPtr sniffer_;
    FILE *pcap_file_;
    /* vni -> vni branch */
    std::map<uint32_t, struct GraphVni> vnis_;

    /** Instruction queue to packetgraph thread. */
    GAsyncQueue *queue_;
};

#endif  // API_SERVER_GRAPH_H_
