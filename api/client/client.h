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

#ifndef API_CLIENT_CLIENT_H_
#define API_CLIENT_CLIENT_H_

#include <google/protobuf/text_format.h>
#include <google/protobuf/stubs/common.h>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <vector>
#include "api/version.h"
#include "api/protocol/message.pb.h"

#define DEFAULT_ENDPOINT "tcp://127.0.0.1:9999"

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::to_string;
using std::exception;
using std::ifstream;
using std::stringstream;
using std::setfill;
using std::setw;
using std::hex;
using std::ostringstream;
using std::hash;

#define BUT_XSTR(s) BUT_STR(s)
#define BUT_STR(s) #s
#define PROTO_REV BUT_XSTR(PROTOS_REVISION)

struct GlobalOptions {
    GlobalOptions();
    void Parse(int argc, char **argv);
    string endpoint;
    bool version;
    bool verbose;
    bool help;
    string encryption_key;
};

struct RequestOptions {
    RequestOptions();
    void Parse(int argc, char **argv);
    bool to_stdout;
};

struct StatusOptions {
    StatusOptions();
    void Parse(int argc, char **argv);
    bool all;
};

struct NicAddOptions {
    NicAddOptions();
    int Parse(int argc, char **argv);
    vector<string> ips;
    vector<string> sgs;
    string mac;
    string enable_antispoof;
    string id;
    string vni;
    string bypass_filtering;
};

struct RuleAddOptions {
    RuleAddOptions();
    int Parse(int argc, char **argv);
    string sg;
    string direction;
    bool has_proto;
    int proto;
    bool has_port_start;
    uint16_t port_start;
    bool has_port_end;
    uint16_t port_end;
    string cidr;
    string sg_members;
};

void GlobalParameterHelp(void);
int SubSg(int argc, char **argv, const GlobalOptions &options);
int SubNic(int argc, char **argv, const GlobalOptions &options);
int sub_status(int argc, char **argv, const GlobalOptions &options);
int sub_shutdown(int argc, char **argv, const GlobalOptions &options);
int SubRequest(int argc, char **argv, const GlobalOptions &options);
int SubDump(int argc, char **argv, const GlobalOptions &options);
int sub_status(int argc, char **argv, const GlobalOptions &options);
int Request(const proto::Messages &request,
            proto::Messages *response,
            const GlobalOptions &options,
            bool response_to_stdout);
int Request(const string &req,
            proto::Messages *res,
            const GlobalOptions &options,
            bool response_to_stdout);
int CheckRequestResult(const proto::Messages &res);

#endif  // API_CLIENT_CLIENT_H_
