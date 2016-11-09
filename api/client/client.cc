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
#include <google/protobuf/text_format.h>
#include <google/protobuf/stubs/common.h>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include "api/client/client.h"
#include "api/protocol/message.pb.h"
#include "api/version.h"

Options::Options() {
    endpoint = NULL;
    input = NULL;
    output = NULL;
    std_out = false;
    all_infos = false;
    version = false;
    verbose = false;
}

bool Options::parse(int argc, char **argv) {
    static GOptionEntry entries[] = {
        {"endpoint", 'e', 0, G_OPTION_ARG_STRING, &endpoint,
         "ZeroMQ endpoint to use (e.g. tcp://myhost:42)", "URL"},
        {"input", 'i', 0, G_OPTION_ARG_FILENAME, &input,
         "JSON description of message to send", "FILE"},
        {"output", 'o', 0, G_OPTION_ARG_FILENAME, &output,
         "JSON description of response message in a file", "FILE"},
        {"stdout", 's', 0, G_OPTION_ARG_NONE, &std_out,
         "JSON description of response message on stdout", NULL},
        {"all-infos", 'a', 0, G_OPTION_ARG_NONE, &all_infos,
         "Ask a summarized view of butterfly server", NULL},
        {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,
         "show details of on each operations", NULL},
        {"version", 'V', 0, G_OPTION_ARG_NONE, &version,
         "Show butterfly version and exit",
         NULL},
        { NULL }
    };
    GOptionContext *context = g_option_context_new("");
    g_option_context_add_main_entries(context, entries, NULL);

    GError *error = NULL;
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        std::cout << error->message << std::endl;
        return false;
    }
    return true;
}

bool Options::missing() {
    if (endpoint == NULL)
        return true;
    return all_infos == false && input == NULL;
}

int main(int argc, char **argv) {
    Options options;
    if (!options.parse(argc, argv))
        return 0;

    // Does user asked for version ?
    if (options.version) {
        std::cout << VERSION_INFO <<
        std::endl;
        return 0;
    }

    // Check missing arguments
    if (options.missing()) {
        std::cerr << "Some arguments are missing, use -h or --help for " \
        "more informations" << std::endl;
        return 1;
    }

    if (options.all_infos)
        return all_infos(options);
    else
        return request_from_human(options);
}

