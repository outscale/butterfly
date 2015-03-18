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
#include "bricks/brick-api.h"
}

namespace Brick {
	

	/* testing */
	int64_t refcount(struct brick &brick);
	
	/* relationship between bricks */
	bool west_link(struct brick &target, struct brick &brick);
	bool east_link(struct brick &target, struct brick &brick);
	void unlink(struct brick *brick);
	
	/* polling */
	bool poll(struct brick &brick, uint16_t *count);

	
	/* constructors */
	struct brick *nop_new(const char *name,
			      uint32_t west_max,
			      uint32_t east_max);
	
	
	struct brick *packetsgen_new(const char *name,
				uint32_t west_max,
				uint32_t east_max,
				enum side output);
	
	struct brick *diode_new(const char *name,
				uint32_t west_max,
				uint32_t east_max,
				enum side output);
	
	/* pkts count */
	int64_t pkts_count_get(struct brick &brick, enum side side);
	
	/* destructor (maybe we can therse remove this one)*/
	void destroy(struct brick *brick);
}
