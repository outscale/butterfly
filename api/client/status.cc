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

static void SubSgStatusHelp(void) {
    cout << "usage: butterfly status [options...]" << endl << endl;
    cout << "Show butterfly status informations" << endl;
    GlobalParameterHelp();
}

int sub_status(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 3 && string(argv[2]) == "help") {
        SubSgStatusHelp();
        return 0;
    }

    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      app_status: true"
        "    }"
        "  }"
        "}";

    proto::Messages res;
    if (Request(req, &res, options, false) || CheckRequestResult(res))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    if (!res_0.has_app_status()) {
        cerr << "no status received" << endl;
        return 1;
    }
    MessageV0_AppStatusRes s = res_0.app_status();
    if (s.has_start_date())
        cout << "start date: " << to_string(s.start_date()) << endl;
    if (s.has_current_date())
        cout << "current date: " << to_string(s.current_date()) << endl;
    if (s.has_request_counter())
        cout << "request counter: " << to_string(s.request_counter()) << endl;
    if (s.has_graph_dot())
        cout << "dot graph: " << endl << s.graph_dot() << endl;
    return 0;
}
