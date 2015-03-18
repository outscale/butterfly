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
 *
 * based on packetgraph/tests/tests.c Copyrith Nodalink EURL
 *
 */

#include <glib.h>

#include "wrapper/lifecycle.h"
#include "tests.h"

int main(int argc, char **argv)
{
	bool ret;
	g_test_init(&argc, &argv, NULL);

	/* initialize DPDK */
	ret = lifecycle::start(argc, argv, "./");
	g_assert(ret);

	test_shared_ptr();

	return g_test_run();
}
