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

#include <glib.h>
#include <memory>

extern "C" {
#include "bricks/brick-api.h"
}

#include "tests.h"
#include "wrapper/packetgraphpp.h"

static void test_shared_ptr_simple()
{
	uint16_t count;
	struct switch_error *error = NULL;
	std::shared_ptr<brick> br1(packetsgen_new("test", 4, 4, WEST_SIDE, &error),
				   brick_destroy);
	std::shared_ptr<brick> br2(packetsgen_new("test", 4, 4, EAST_SIDE, &error),
				   brick_destroy);

	g_assert(br1 != false);
	g_assert(br2 != false);
	g_assert(brick_west_link(&*br1, &*br2, &error));
	g_assert(brick_poll(&*br1, &count, &error));
	g_assert(count == 3);
	g_assert(brick_pkts_count_get(&*br2, WEST_SIDE) == 3);
}
static void test_shared_ptr_wrapper()
{
	uint16_t count;
	std::shared_ptr<brick> br1(Brick::packetsgen_new("test", 4, 4, WEST_SIDE),
				   Brick::destroy);
	std::shared_ptr<brick> br2(Brick::packetsgen_new("test", 4, 4, EAST_SIDE),
				   Brick::destroy);

	g_assert(br1 != false);
	g_assert(br2 != false);
	g_assert(Brick::west_link(*br1, *br2));
	g_assert(Brick::poll(*br1, &count));
	g_assert(count == 3);
	g_assert(Brick::pkts_count_get(*br2, WEST_SIDE) == 3);	
}


void test_shared_ptr()
{
	g_test_add_func("/brick/shared-ptr/simple",
			test_shared_ptr_simple);
	g_test_add_func("/brick/shared-ptr/wrapper",
			test_shared_ptr_wrapper);
}
