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

#include <rte_config.h>
#include <rte_ethdev.h>
#include "utils/eth.h"

int start_eth(uint8_t port_id)
{
	int ret;

	ret = rte_eth_dev_start(port_id);
	if (ret < 0)
		return ret;
	rte_eth_promiscuous_enable(port_id);
	return 0;
}

void stop_eth(uint8_t port_id)
{
	rte_eth_dev_stop(port_id);
}
