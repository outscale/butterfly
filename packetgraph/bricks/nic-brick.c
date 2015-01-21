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

#include <rte_config.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>

#include "common.h"

#include "bricks/brick.h"
#include "bricks/stubs.h"

#include "utils/mempool.h"
#include "utils/bitmask.h"
#include "packets/packets.h"

#define RTE_TEST_RX_DESC_DEFAULT 128
#define RTE_TEST_TX_DESC_DEFAULT 512

struct nic_state {
	struct brick brick;
	struct rte_mbuf *pkts[MAX_PKTS_BURST];
	struct rte_mbuf *exit_pkts[MAX_PKTS_BURST];
	uint8_t portid;
	/* side of the switch the packet came from and are send */
	enum side output;
};


/* The fastpath data function of the nic_brick just forward the bursts */
static int nic_burst(struct brick *brick, enum side side, uint16_t edge_index,
		     struct rte_mbuf **pkts, uint16_t nb_pkts,
		     uint64_t pkts_mask, struct switch_error **errp)
{
	uint16_t count = 0;
	uint16_t pkts_lost;
	struct nic_state *state = brick_get_state(brick, struct nic_state);
	struct rte_mbuf **exit_pkts = state->exit_pkts;

	if (state->output == side) {
		*errp = error_new("Bursting on wrong direction");
		return 0;
	}

	count = packets_pack(exit_pkts,
			     pkts, pkts_mask);

	packets_incref(pkts, pkts_mask);
	pkts_lost = rte_eth_tx_burst_wrap(state->portid, 0,
					  exit_pkts,
					  count);
	if (unlikely(pkts_lost < count))
		packets_free(exit_pkts, mask_firsts(count) &
			     ~mask_firsts(pkts_lost));
	return 1;
}

static int nic_poll_forward(struct nic_state *state,
			    struct brick *brick,
			    uint16_t nb_pkts,
			    struct switch_error **errp)
{
	struct brick_side *s = &brick->sides[flip_side(state->output)];
	int ret;
	uint64_t pkts_mask;

	pkts_mask = mask_firsts(nb_pkts);

	ret = brick_side_forward(s, state->output,
				 state->pkts, nb_pkts, pkts_mask, errp);
	packets_free(state->pkts, pkts_mask);
	return ret;
}

static int nic_poll(struct brick *brick, uint16_t *pkts_cnt,
		    struct switch_error **errp)
{
	uint16_t nb_pkts;
	struct nic_state *state =
		brick_get_state(brick, struct nic_state);

	nb_pkts = rte_eth_rx_burst(state->portid, 0,
				   state->pkts, MAX_PKTS_BURST);
	if (!nb_pkts) {
		*pkts_cnt = 0;
		return 1;
	}

	*pkts_cnt = nb_pkts;
	return nic_poll_forward(state, brick, nb_pkts, errp);
}

static int nic_init(struct brick *brick, struct brick_config *config,
		    struct switch_error **errp)
{
	struct nic_state *state = brick_get_state(brick, struct nic_state);
	struct nic_config *nic_config;
	uint8_t nb_ports;
	struct rte_mempool *mp = get_mempool();
	int ret;

	if (!config) {
		*errp = error_new("config is NULL");
		return 0;
	}
	nic_config = config->nic;

	if (!nic_config) {
		*errp = error_new("config->nic is NULL");
		return 0;
	}

	state->output = nic_config->output;
	if (error_is_set(errp))
		return 0;

	state->portid = (uint8_t)nic_config->portid;

	nb_ports = rte_eth_dev_count();
	if (nb_ports < 1 || state->portid >= nb_ports) {
		*errp = error_new("Invalid portid");
		return 0;
	}

	ret = rte_eth_rx_queue_setup(state->portid, 0, nic_config->rte_rx_desc,
				     rte_eth_dev_socket_id(state->portid),
				     NULL,
				     mp);
	if (ret < 0) {
		*errp = error_new("Setup faild for port rx queue 0, port %d",
				  state->portid);
		return 0;
	}

	ret = rte_eth_tx_queue_setup(state->portid, 0, nic_config->rte_tx_desc,
				     rte_eth_dev_socket_id(state->portid),
				     NULL);
	if (ret < 0) {
		*errp = error_new("Setup faild for port tx queue 0, port %d",
				  state->portid);
		return 0;
	}

	brick->burst = nic_burst;
	brick->poll = nic_poll;

	return 1;
}

static struct brick_ops nic_ops = {
	.name		= "nic",
	.state_size	= sizeof(struct nic_state),

	.init		= nic_init,

	.unlink		= brick_generic_unlink,
};

brick_register(struct nic_state, &nic_ops);
