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

#include <cstddef>

extern "C" {
#include "bricks/brick-api.h"
#include "utils/errors.h"
}

#include "wrapper/brick.h"

namespace Brick {
	thread_local static switch_error *errp = NULL;

	namespace {
		void print_and_free_errp()
		{
			error_print(errp);
			error_free(errp);
			errp = NULL;			
		}
	}


	int64_t refcount(struct brick &brick)
	{
		return brick_refcount(&brick);
	}
	
#include <stdio.h>


	bool west_link(struct brick &target, struct brick &brick)
	{
		bool ret = brick_west_link(&target, &brick, &errp);
		if (!ret)
			print_and_free_errp();
		return ret;
	}


	bool east_link(struct brick &target, struct brick &brick)
	{
		bool ret = brick_east_link(&target, &brick, &errp);
		if (!ret)
			print_and_free_errp();
		return ret;
	}


	void unlink(struct brick *brick)
	{
		brick_unlink(brick, &errp);
		if (errp)
			print_and_free_errp();
	}

	
	bool poll(struct brick &brick, uint16_t *count)
	{
		bool ret = brick_poll(&brick, count, &errp);
		if (!ret)
			print_and_free_errp();
		return ret;
	}


	struct brick *nop_new(const char *name,
		       uint32_t west_max,
		       uint32_t east_max)
	{
		brick *ret = nop_new(name, west_max, east_max, &errp);
		if (!ret)
			print_and_free_errp();
		return ret;

	}

	struct brick *switch_new(const char *name,
		       uint32_t west_max,
		       uint32_t east_max)
	{
		brick *ret = switch_new(name, west_max, east_max, &errp);
		if (!ret)
			print_and_free_errp();
		return ret;

	}

	struct brick *packetsgen_new(const char *name,
				uint32_t west_max,
				uint32_t east_max,
				enum side output)
	{
		brick *ret = packetsgen_new(name, west_max, east_max, output, &errp);
		if (!ret)
			print_and_free_errp();
		return ret;
	}

	struct brick *vhost_new(const char *name,
				uint32_t west_max,
				uint32_t east_max,
				enum side output)
	{
		brick *ret = vhost_new(name, west_max, east_max, output, &errp);
		if (!ret)
			print_and_free_errp();
		return ret;
	}



	struct brick *diode_new(const char *name,
				uint32_t west_max,
				uint32_t east_max,
				enum side output)
	{
		brick *ret = diode_new(name, west_max, east_max, output, &errp);
		if (!ret)
			print_and_free_errp();
		return ret;

	}


	int64_t pkts_count_get(struct brick &brick, enum side side)
	{
		return brick_pkts_count_get(&brick, side);
	}


	void destroy(struct brick *brick)
	{
		return brick_destroy(brick);
	}
}
