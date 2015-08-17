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

#include <glib.h>
#include <mutex>
#include <memory>
#include <map>
#include <string>
#include <vector>
#include "api/server/app.h"
#include "api/server/pg.h"

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
     * @param  argc argument count to pass to packetgraph
     * @param  argv arguments strings to pass to packetgraph
     * @return true if graph has started correctly, false otherwise
     */
    bool start(int argc, char **argv);
    /** Inject a NIC in packetgraph including all needed bricks
     * @param  nic model and informations of the NIC.
     */
    std::string nic_add(const app::Nic &nic);
    /** Remove a NIC from the graph with related dependencies.
     * @param  id NIC id to delete.
     */
    void nic_del(std::string id);
    /** Get a snapshot containing the current firewall state attached to the NIC.
     * @param  id id of the NIC attached to the firewall.
     * @return an opaque string containing firewall's state data.
     */
    std::string nic_export(std::string id);
    /** Get NIC statistics.
     * @param  id id of the NIC
     * @param  in number of bytes who has been received by the NIC
     * @param  out number of bytes who has been transmitted by the NIC
     */
    void nic_get_stats(std::string id, uint64_t *in, uint64_t *out);
    /** Enable on disable IP antispoof on the NIC.
     * @param  id id of the NIC
     * @param  enable true to enable IP antispoof, false otherwise
     */
    void nic_config_anti_spoof(std::string nic_id, bool enable);
    /** Update all ip of a NIC
     * @param  id id of the NIC
     * @param  ip_list list of all IP the NIC have
     */
    void nic_config_ip(std::string id, const std::vector<app::Ip> &ip_list);
    /** Rebuild all firewall rules of a NIC
     * @param  nic model of the NIC
     */
    void fw_update(const app::Nic &nic);
    /** Add a single rule in the corresponding firewall of a NIC.
     * @param  id id of the NIC
     * @param  rule model of the rule
     */
    void fw_add_rule(std::string nic_id, const app::Rule &rule);
    /** Build a graphic description in dot language (graphviz project).
     * @return  a string describing the whole graph
     */
    std::string dot();

 private:
    /* Check if graph has been started or not. */
    bool started;

    /* Define a smart pointer for bricks. */
    typedef std::shared_ptr<struct pg_brick> Brick;

    /* All possible structure to pass through the queue.
     * This should ONLY concern actions who needs the graph to be stopped.
     */
    enum rpc_action {
        EXIT,
        LINK,
        UNLINK,
        FIREWALL_RELOAD,
        ADD_VNI,
        UPDATE_POLL
    };

    struct rpc_link {
        struct pg_brick *w;
        struct pg_brick *e;
    };

    struct rpc_unlink {
        struct pg_brick *b;
    };

    struct rpc_firewall_reload {
        struct pg_brick *b;
    };

    struct rpc_add_vni {
        struct pg_brick *vtep;
        struct pg_brick *neighbor;
        uint32_t vni;
        uint32_t multicast_ip;
    };

    // This rpc message is kept by the poller
    struct rpc_update_poll {
        struct pg_brick *pollables[GRAPH_VHOST_MAX_SIZE];
        struct pg_brick *firewalls[GRAPH_VHOST_MAX_SIZE];
        uint32_t size;
    };

    struct rpc_queue {
        enum rpc_action action;
        struct rpc_link link;
        struct rpc_unlink unlink;
        struct rpc_firewall_reload firewall_reload;
        struct rpc_add_vni add_vni;
        struct rpc_update_poll update_poll;
    };

    /* Wrappers to ease RPC actions. */
    void exit();
    void link(Brick w, Brick e);
    void unlink(Brick b);
    void firewall_reload(Brick b);
    void add_vni(Brick vtep, Brick neighbor, uint32_t vni);
    void update_poll();

    /** Threaded function to poll graph. */
    static void *poller(void *g);
    pthread_t poller_thread;
    /**
     * Set scheduler affinity so the current thread only run a on a
     * specific CPU.
     */
    static inline int set_cpu(int core_id);
    /**
     * Called by the poller, run all pending actions in the queue
     * @param    list list of bricks the poller needs (struct rpc_update_poll)
     * @return  true if poller must continue polling, otherwhise exit.
     */
    inline bool poller_update(struct rpc_queue **list);

    /**
     * Build a rule string based on a rule model
     * @param   a list of security groups to apply
     * @return  return a pcap filter string of the rule
     */
    std::string fw_build_rule(const app::Rule &rule);

    /**
     * Build a big rule string based on a security group
     * @param   a list of security groups to apply
     * @return  return a pcap filter rule of the whole SG
     */
    std::string fw_build_sg(const app::Sg &sg);

    /**
     * Convert a VNI to a mutlicast IP
     * @param   vni vni integer to convert
     * @return  multicast IP
     */
    uint32_t build_multicast_ip(uint32_t vni);

    /* VM branch. */
    struct graph_nic {
       std::string id;
       Brick firewall;
       Brick antispoof;
       Brick vhost;
    };

    /* VNI branch. */
    struct graph_vni {
       uint32_t vni;
       /* Switch brick */
       Brick sw;
       /* nic id -> nic branch */
       std::map<std::string, struct graph_nic> nics;
    };

    /* Global branch. */
    Brick nic;
    Brick vtep;
    /* vni -> vni branch */
    std::map<uint32_t, struct graph_vni> vnis;

    /** Instruction queue to packetgraph thread. */
    GAsyncQueue *queue;
};

#endif  // API_SERVER_GRAPH_H_
