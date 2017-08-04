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
#include "api/common/crypto.h"

GlobalOptions::GlobalOptions() {
    endpoint = DEFAULT_ENDPOINT;
    verbose = false;
    version = false;
    help = false;
}

void GlobalOptions::Parse(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (i + 1 < argc &&
            (string(argv[i]) == "--endpoint" ||
             string(argv[i]) == "-e"))
            endpoint = string(argv[i + 1]);
        else if (string(argv[i]) == "-v" ||
                 string(argv[i]) == "--verbose")
            verbose = true;
        else if (string(argv[i]) == "-V" ||
                 string(argv[i]) == "--version")
            version = true;
        else if (string(argv[i]) == "-h" ||
                 string(argv[i]) == "--help")
            help = true;
        if (i + 1 < argc &&
            (string(argv[i]) == "--key" ||
             string(argv[i]) == "-k")) {
            string path = string(argv[i + 1]);
            if (!Crypto::KeyFromPath(path, &encryption_key)) {
                cerr << "cannot load encryption key" << endl;
            }
        }
    }
}

void GlobalParameterHelp(void) {
        cout << endl <<
        "global options:" << endl <<
        "    --endpoint, -e  endpoint to use (default: " <<
        DEFAULT_ENDPOINT << ")" << endl <<
        "    --verbose, -v   show details of each operation" << endl <<
        "    --key, -k       path to encryption key (32 raw bytes)" << endl;
}

static void Help(void) {
    cout <<
        "usage: butterfly [subcommands] [options...]" << endl << endl <<
        "butterfly subcommands:" << endl <<
        "    nic       manage virtual interfaces" << endl <<
        "    sg        manage security groups" << endl <<
        "    status    show informations about butterfly" << endl <<
        "    shutdown  ask butterflyd to stop" << endl <<
        "    request   send a protobuf request to butterfly" << endl <<
        "    dump      extract Butterfly configuration" << endl <<
        endl <<

        "options:" << endl <<
        "    --version, -V   show butterfly version" << endl <<
        "    --help, -h      show this help" << endl;
        GlobalParameterHelp();
}

int main(int argc, char **argv) {
    GlobalOptions options;
    options.Parse(argc, argv);

    if (options.version) {
        cout << VERSION_INFO << endl;
        return 0;
    }

    if (options.help) {
        Help();
        return 0;
    }

    if (argc < 2) {
        Help();
        return 1;
    }

    string cmd = string(argv[1]);
    if (cmd == "nic") {
        return SubNic(argc, argv, options);
    } else if (cmd == "sg") {
        return SubSg(argc, argv, options);
    } else if (cmd == "status") {
        return sub_status(argc, argv, options);
    } else if (cmd == "shutdown") {
        return sub_shutdown(argc, argv, options);
    } else if (cmd == "request") {
        return SubRequest(argc, argv, options);
    } else if (cmd == "dump") {
        return SubDump(argc, argv, options);
    } else {
        cerr << "invalid subcommand " << cmd << endl;
        Help();
        return 1;
    }
}
