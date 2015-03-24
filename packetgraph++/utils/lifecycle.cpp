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

// for NULL
#include <cstddef>

extern "C" {
#include "utils/lifecycle.h"
}

#include "wrapper/lifecycle.h"

namespace lifecycle {

	/**
	 * This function initialise packetsgraphlib
	 *
	 * @param	argc argc from main
	 * @param	argv argv from main
	 * @param	base_dir the directory where sockets will be create
	 * @return	false in case of falure true on sucess
	 */
	bool start(int argc, char **argv,
		  const char *base_dir)
	{
		struct switch_error *errp = NULL;
		int ret;

		ret = packetgraph_start(argc, argv,
					base_dir,
					&errp);
		if (!ret) {
			
			return false;
		}
		return true;
	}

	void stop(void)
	{
		packetgraph_stop();
	}
}
