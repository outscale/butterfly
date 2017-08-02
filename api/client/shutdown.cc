/* Copyright 2016 Outscale SAS
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

#include "api/client/client.h"

static void SubSgShutdownHelp(void) {
    cout << "usage: butterfly shutdown [options...]" << endl << endl;
    cout << "Ask butterfly to quit" << endl;
    GlobalParameterHelp();
}

int sub_shutdown(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 3 && string(argv[2]) == "help") {
        SubSgShutdownHelp();
        return 0;
    }

    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      app_quit: true"
        "    }"
        "  }"
        "}";

    proto::Messages res;

    if (Request(req, &res, options, false))
        return 1;
    return 0;
}
