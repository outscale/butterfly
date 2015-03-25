/* Copyright 2014 Outscale SAS
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

#include <rte_config.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_eth_ring.h>
#include <unistd.h>
#include "common.h"
#include "utils/mempool.h"
#include "utils/config.h"
#include "bricks/brick.h"
#include "bricks/stubs.h"
#include "utils/eth.h"
#include "tests.h"

static const struct rte_eth_conf port_conf = {
	.rxmode = {
		.split_hdr_size = 0,
		.header_split   = 0, /**< Header Split disabled */
		.hw_ip_checksum = 0, /**< IP checksum offload disabled */
		.hw_vlan_filter = 0, /**< VLAN filtering disabled */
		.jumbo_frame    = 0, /**< Jumbo Frame Support disabled */
		.hw_strip_crc   = 0, /**< CRC stripped by hardware */
	},
	.txmode = {
		.mq_mode = ETH_MQ_TX_NONE,
	},
};

const uint8_t pcap_portid = 0;
const uint8_t ring_portid = 1;

#define RX_DSC_DEFAULT 128
#define TX_DSC_DEFAULT 512
#define NB_PKTS 128

static void test_nic_simple_flow(void)
{
	struct brick_config *config = brick_config_new("mybrick", 4, 4);
	struct brick_config *nic_config_to_west = nic_config_new("nicbrick",
								 4, 4,
								 pcap_portid,
								 RX_DSC_DEFAULT,
								 TX_DSC_DEFAULT,
								 WEST_SIDE);
	struct brick_config *nic_config_ring = nic_config_new("nicbrick",
							      4, 4,
							      ring_portid,
							      RX_DSC_DEFAULT,
							      TX_DSC_DEFAULT,
							      EAST_SIDE);
	struct brick *nic_west, *collect_west, *collect_east, *nic_ring;
	uint64_t pkts_mask;
	uint8_t nb_ports;
	int ret;
	int i = 0;
	int nb_iteration = 32;
	uint16_t nb_send_pkts;
	uint16_t total_send_pkts = 0;
	uint16_t total_get_pkts = 0;
	struct switch_error *error = NULL;
	struct rte_eth_stats info;

	nb_ports = rte_eth_dev_count();
	if (pcap_portid > ring_portid)
		g_assert(nb_ports >= pcap_portid + 1);
	else
		g_assert(nb_ports >= ring_portid + 1);

	ret = rte_eth_dev_configure(pcap_portid, 1, 1, &port_conf);
	g_assert(!ret);
	ret = rte_eth_dev_configure(ring_portid, 1, 1, &port_conf);
	g_assert(!ret);

	/* create a chain of a few nop brick with collectors on each sides */
	nic_west = brick_new("nic", nic_config_to_west, &error);
	g_assert(!error);
	nic_ring = brick_new("nic", nic_config_ring, &error);
	g_assert(!error);
	collect_east = brick_new("collect", config, &error);
	g_assert(!error);
	collect_west = brick_new("collect", config, &error);
	g_assert(!error);
	brick_link(nic_west, nic_ring, &error);
	g_assert(!error);
	brick_link(nic_west, collect_east, &error);
	g_assert(!error);
	brick_link(collect_west, nic_ring, &error);
	g_assert(!error);

	ret = start_eth(0);
	g_assert(ret >= 0);

	for (i = 0; i < nb_iteration * 6; ++i) {
		max_pkts = i * 2;
		if (max_pkts > 64)
			max_pkts = 64;
		struct rte_mbuf **result_pkts;
		/*poll packets to east*/
		brick_poll(nic_west, &nb_send_pkts, &error);
		g_assert(!error);
		pkts_mask = 0;
		/* collect pkts on the east */
		result_pkts = brick_west_burst_get(collect_east,
						   &pkts_mask, &error);
		g_assert(!error);
		if (nb_send_pkts) {
			g_assert(result_pkts);
			g_assert(pkts_mask);
			total_send_pkts += max_pkts;
		}
		pkts_mask = 0;
		/* check no pkts end here */
		result_pkts = brick_east_burst_get(collect_east,
						   &pkts_mask, &error);
		g_assert(!error);
		g_assert(!pkts_mask);
		g_assert(!result_pkts);
		brick_reset(collect_east, &error);
		g_assert(!error);
	}
	rte_eth_stats_get(ring_portid, &info);
	max_pkts = 64;
	g_assert(info.opackets == total_send_pkts);
	for (i = 0; i < nb_iteration; ++i) {
		/* poll packet to the west */
		brick_poll(nic_ring, &nb_send_pkts, &error);
		g_assert(!error);
		total_get_pkts += nb_send_pkts;
		brick_east_burst_get(collect_west, &pkts_mask, &error);
		g_assert(pkts_mask);
		pkts_mask = 0;
	}
	/* This assert allow us to check nb_send_pkts*/
	g_assert(total_get_pkts == total_send_pkts);
	g_assert(info.opackets == total_send_pkts);
	/* use packets_count in collect_west here to made
	 * another check when merge*/

	/* break the chain */
	brick_unlink(nic_west, &error);
	g_assert(!error);
	brick_unlink(collect_east, &error);
	g_assert(!error);
	brick_unlink(nic_ring, &error);
	g_assert(!error);

	/* destroy */
	brick_decref(nic_west, &error);
	g_assert(!error);
	brick_decref(nic_ring, &error);
	g_assert(!error);
	brick_decref(collect_east, &error);
	g_assert(!error);

	brick_config_free(nic_config_to_west);
	brick_config_free(nic_config_ring);
	brick_config_free(config);
	stop_eth(0);
}

static void test_nic_ring(void)
{

	struct rte_ring *r1, *r2;
	struct switch_error *error = NULL;
	int port1, port2;
	int nb_ports;
	int ret;
	struct brick *nic_west, *nic_east;
	struct brick_config *nic_config_to_west, *nic_config_to_east;
	/* struct rte_mbuf **result_pkts; */
	struct rte_mbuf *pkts[NB_PKTS];
	struct rte_mempool *mp = get_mempool();
	struct rte_eth_stats info;
	uint16_t nb_send_pkts;
	uint16_t i;

	memset(pkts, 0, MAX_PKTS_BURST * sizeof(struct rte_mbuf *));
	for (i = 0; i < NB_PKTS; i++) {
		pkts[i] = rte_pktmbuf_alloc(mp);
		g_assert(pkts[i]);
		pkts[i]->udata64 = i;
	}
	r1 = rte_ring_create("R1", 0, rte_socket_id(),
			     RING_F_SP_ENQ|RING_F_SC_DEQ);
	r2 = rte_ring_create("R2", 0, rte_socket_id(),
			     RING_F_SP_ENQ|RING_F_SC_DEQ);

	ret = rte_eth_from_rings("port1", &r1, 1, &r2, 1, rte_socket_id());
	g_assert(ret >= 0);
	nb_ports = rte_eth_dev_count();
	port1 = nb_ports - 1;
	ret = rte_eth_from_rings("port2", &r2, 1, &r1, 1, rte_socket_id());
	g_assert(ret >= 0);
	nb_ports = rte_eth_dev_count();
	port2 = nb_ports - 1;

	ret = rte_eth_dev_configure(port1, 1, 1, &port_conf);
	g_assert(!ret);
	ret = rte_eth_dev_configure(port2, 1, 1, &port_conf);
	g_assert(!ret);

	nic_config_to_west = nic_config_new("nicbrick", 4, 4,
					    port1,
					    RX_DSC_DEFAULT,
					    TX_DSC_DEFAULT,
					    WEST_SIDE);
	nic_config_to_east = nic_config_new("nicbrick", 4, 4,
					    port2,
					    RX_DSC_DEFAULT,
					    TX_DSC_DEFAULT,
					    EAST_SIDE);

	nic_west = brick_new("nic", nic_config_to_west, &error);
	g_assert(!error);
	nic_east = brick_new("nic", nic_config_to_east, &error);
	g_assert(!error);

	brick_link(nic_west, nic_east, &error);
	g_assert(!error);

	/*put packets into nic_west*/
	rte_eth_tx_burst(port2, 0, pkts, NB_PKTS);
	rte_eth_stats_get(port1, &info);
	g_assert(!info.opackets);
	g_assert(!info.ipackets);
	printf("p1: %lu\n", info.opackets);
	printf("p1: %lu\n", info.ipackets);
	rte_eth_stats_get(port2, &info);
	g_assert(info.opackets == NB_PKTS);
	g_assert(!info.ipackets);
	printf("p2: %lu\n", info.opackets);
	printf("p2: %lu\n", info.ipackets);

	/*We poll 64 pkts, so */
	brick_poll(nic_west, &nb_send_pkts, &error);
	rte_eth_stats_get(port1, &info);
	g_assert(!info.opackets);
	g_assert(info.ipackets == 64);
	printf("p1: %lu\n", info.opackets);
	printf("p1: %lu\n", info.ipackets);
	rte_eth_stats_get(port2, &info);
	g_assert(info.opackets == NB_PKTS + 64);
	g_assert(!info.ipackets);
	printf("p2: %lu\n", info.opackets);
	printf("p2: %lu\n", info.ipackets);

	/* there's no packets in the ring 2,
	 * so we check that nothing has change */
	brick_poll(nic_east, &nb_send_pkts, &error);
	rte_eth_stats_get(port1, &info);
	g_assert(!info.opackets);
	g_assert(info.ipackets == 64);
	printf("p1: %lu\n", info.opackets);
	printf("p1: %lu\n", info.ipackets);
	rte_eth_stats_get(port2, &info);
	g_assert(info.opackets == NB_PKTS + 64);
	g_assert(!info.ipackets);
	printf("p2: %lu\n", info.opackets);
	printf("p2: %lu\n", info.ipackets);

	brick_decref(nic_west, &error);
	g_assert(!error);
	brick_decref(nic_east, &error);
	g_assert(!error);

	brick_config_free(nic_config_to_west);
	brick_config_free(nic_config_to_east);
}

#undef RX_DSC_DEFAULT
#undef TX_DSC_DEFAULT
#undef NB_PKTS

void test_nic(void)
{
	g_test_add_func("/brick/pcap/nic-pcap",
			test_nic_simple_flow);
	g_test_add_func("/brick/pcap/nic-ring",
			test_nic_ring);
}
