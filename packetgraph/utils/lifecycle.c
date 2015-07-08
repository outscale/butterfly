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
#include <rte_eal.h>
#include <rte_virtio_net.h>
#include <rte_ethdev.h>

#include "utils/lifecycle.h"
#include "bricks/vhost-user-brick.h"

static int vhost;

int packetgraph_start(int argc, char **argv,
		      const char *base_dir,
		      struct switch_error **errp)
{
	int ret;

	/* I should detect if RTE_LIBRTE_PMD_PCAP have been define,
	* but it's appear as it's never define, so i call it without the check.
	* The cmake of 3rdparty add it anyway */
	devinitfn_pmd_pcap_drv();

	#ifdef RTE_LIBRTE_PMD_RING
	devinitfn_pmd_ring_drv();
	#endif

	#ifdef RTE_LIBRTE_IGB_PMD
	devinitfn_pmd_igb_drv();
	#endif

	#ifdef RTE_LIBRTE_IXGBE_PMD
	devinitfn_rte_ixgbe_driver();
	#endif

	ret = rte_eal_init(argc, argv);
	if (ret < 0) {
		error_new("error durring eal initialisation");
		return -1;
	}
	if (base_dir != NULL) {
		if (!vhost_start(base_dir, errp))
			return -1;
		vhost = 1;
	}
	return ret;
}

void packetgraph_stop(void)
{
	if (vhost)
		vhost_stop();
}
