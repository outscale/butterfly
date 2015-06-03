/* Copyright 2014 Nodalink EURL
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

#ifndef _UTILS_CONFIG_H_
#define _UTILS_CONFIG_H_

#include "common.h"
#include "utils/errors.h"

struct diode_config {
	enum side output;
};

struct vhost_config {
	enum side output;
};

struct nic_config {
	enum side output;
	uint32_t portid;
	uint32_t rte_tx_desc;
	uint32_t rte_rx_desc;
};

struct brick_config {
	/* The unique name of the brick brick in the graph */
	char *name;
	/* The maximum number of west edges */
	uint32_t west_max;
	/* The maximum number of east edges */
	uint32_t east_max;

	struct diode_config *diode;
	struct vhost_config *vhost;
	struct nic_config *nic;
};

struct brick_config *diode_config_new(const char *name, uint32_t west_max,
				      uint32_t east_max, enum side output);

struct brick_config *brick_config_init(struct brick_config *config,
				       const char *name,
				       uint32_t west_max,
				       uint32_t east_max);

struct brick_config *brick_config_new(const char *name, uint32_t west_max,
				      uint32_t east_max);

struct brick_config *vhost_config_new(const char *name, uint32_t west_max,
				      uint32_t east_max,
				      enum side output);

struct brick_config *nic_config_new(const char *name, uint32_t west_max,
				    uint32_t east_max,
				    uint8_t portid,
				    uint16_t rte_tx_desc,
				    uint16_t rte_rx_desc,
				    enum side output);

void brick_config_free(struct brick_config *config);

#endif
