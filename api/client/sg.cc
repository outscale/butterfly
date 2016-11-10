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

static void sub_sg_list_help(void) {
    cout << "usage: butterfly sg list [options...]" << endl << endl;
    cout << "List all available security groups" << endl;
    global_parameter_help();
}

static int sub_sg_list(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        sub_sg_list_help();
        return 0;
    }

    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      sg_list: true"
        "    }"
        "  }"
        "}";

    proto::Messages res;
    if (request(req, &res, options, false) || check_request_result(res))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    int size = res_0.sg_list_size();
    for (int i = 0; i < size; i++) {
        cout << res_0.sg_list(i) << endl;
    }
    return 0;
}

static void sub_sg_add_help(void) {
    cout << "usage: butterfly sg add SG [SG...] [options...]" << endl << endl;
    cout << "Create a new security group" << endl;
    cout <<
    "Note that creating a new security group when it already " << endl <<
    "exists will overwrite the existing SG with an empty one." << endl;
    global_parameter_help();
}

static int sub_sg_add(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        sub_sg_add_help();
        return 0;
    }

    if (argc <= 3) {
        sub_sg_add_help();
        return 1;
    }

    vector<string> sgs;
    for (int i = 3; i < argc; i++) {
        string a = string(argv[i]);
        if (a[0] == '-')
            break;
        sgs.push_back(a);
    }
    if (!sgs.size()) {
        sub_sg_add_help();
        return 1;
    }

    string req;
    for (string sg : sgs) {
        req +=
            "messages {"
            "  revision: " PROTO_REV
            "   message_0 {"
            "     request {"
            "       sg_add {"
            "         id: \"" + sg + "\""
            "       }"
            "    }"
            "  }"
            "}";
    }

    proto::Messages res;
    return request(req, &res, options, false) || check_request_result(res);
}

static void sub_sg_del_help(void) {
    cout << "usage: butterfly sg del SG [SG...] [options...]" << endl << endl;
    cout << "Delete one or more security group" << endl;
    global_parameter_help();
}

static int sub_sg_del(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        sub_sg_del_help();
        return 0;
    }

    if (argc <= 3) {
        sub_sg_del_help();
        return 1;
    }

    vector<string> sgs;
    for (int i = 3; i < argc; i++) {
        string a = string(argv[i]);
        if (a[0] == '-')
            break;
        sgs.push_back(a);
    }
    if (!sgs.size()) {
        sub_sg_del_help();
        return 1;
    }

    string req;
    for (string sg : sgs) {
        req +=
            "messages {"
            "  revision: " PROTO_REV
            "  message_0 {"
            "    request {"
            "      sg_del: \"" + sg + "\""
            "    }"
            "  }"
            "}";
    }

    proto::Messages res;
    return request(req, &res, options, false) || check_request_result(res);
}

static string rule_hash(const MessageV0_Rule &r) {
    string human_message;
    google::protobuf::TextFormat::PrintToString(r, &human_message);
    hash<string> fn;
    size_t hash = fn(human_message);
    ostringstream out;
    out << setfill('0') << setw(16) << hex << hash;
    return out.str();
}

static void rule_print(const MessageV0_Rule &r) {
    cout << rule_hash(r) << " | " <<
        "direction: " <<
            MessageV0_Rule_Direction_Name(r.direction()) << " - " <<
        "IP protocol: " << to_string(r.protocol()) << " - " <<
        "port start: " <<
            (r.has_port_start() ? to_string(r.port_start()) : "Ø") << " - " <<
        "port end: " <<
            (r.has_port_end() ? to_string(r.port_end()) : "Ø") << " - " <<
        "cidr: " << (r.has_cidr() ?
            r.cidr().address() + "/" + to_string(r.cidr().mask_size()) :
            "Ø") << " - " <<
        "members of sg: " <<
            (r.has_security_group() ? r.security_group() : "Ø") << endl;
}

static void sub_sg_rule_list_help(void) {
    cout << "usage: butterfly sg rule list SG [options...]" << endl << endl;
    cout << "List all firewalling rules of a security group" << endl;
    global_parameter_help();
}

static int sub_sg_rule_list(int argc, char **argv,
                            const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        sub_sg_rule_list_help();
        return 0;
    }

    if (argc <= 4) {
        sub_sg_rule_list_help();
        return 1;
    }

    string sg = string(argv[4]);
    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      sg_rule_list: \"" + sg + "\""
        "    }"
        "  }"
        "}";

    proto::Messages res;
    if (request(req, &res, options, false) || check_request_result(res))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    int size = res_0.sg_rule_list_size();
    for (int i = 0; i < size; i++) {
        rule_print(res_0.sg_rule_list(i));
    }
    return 0;
}

RuleAddOptions::RuleAddOptions() {
    direction = "INBOUND";
    has_proto = false;
    has_port_start = false;
    has_port_end = false;
}

static int cidr_to_protobuf(const string &in, string *out) {
    string address;
    string mask;
    string *w = &address;
    for (char c : in) {
        if (c == '/') {
            w = &mask;
            continue;
        }
        *w += c;
    }

    if (!address.length() || !mask.length())
        return 1;
    *out = string(
        "address: \"" + address + "\""
        "mask_size: " + mask);
    return 0;
}

int RuleAddOptions::parse(int argc, char **argv) {
    if (argc <= 7) {
        return 1;
    }
    sg = argv[4];
    for (int i = 5; i < argc; i++) {
        if (i + 1 < argc && (string(argv[i]) == "--dir")) {
            direction = string(argv[i + 1]);
            if (direction == "in") {
                direction = "INBOUND";
            } else if (direction == "out") {
                cerr << "outbound not supported at the moment" << endl;
                return 1;
            } else {
                cerr << "--dir argument not recognized" << endl;
                return 1;
            }
        } else if (i + 1 < argc && (string(argv[i]) == "--ip-proto")) {
            string p = string(argv[i + 1]);
            if (p == "tcp") {
                proto = 6;
            } else if (p == "udp") {
                proto = 17;
            } else if (p == "icmp") {
                proto = 1;
            } else if (p == "all") {
                proto = -1;
            } else {
                try {
                    proto = stoi(p);
                } catch (exception &e) {
                    cerr << "--ip-proto argument not recognized" << endl;
                    return 1;
                }
            }
            has_proto = true;
        } else if (i + 1 < argc && (string(argv[i]) == "--port")) {
            string p = string(argv[i + 1]);
            try {
                int pi = stoi(p);
                if (pi < 0 || pi > 65535)
                    throw exception();
                has_port_start = true;
                port_start = pi;
                has_port_end = true;
                port_end = pi;
            } catch (exception &e) {
                cerr << "--port argument not recognized" << endl;
                return 1;
            }
        } else if (i + 1 < argc && (string(argv[i]) == "--port-start")) {
            string p = string(argv[i + 1]);
            try {
                int pi = stoi(p);
                if (pi < 0 || pi > 65535)
                    throw;
                has_port_start = true;
                port_start = pi;
            } catch (exception &e) {
                cerr << "--port-start argument not recognized" << endl;
                return 1;
            }
        } else if (i + 1 < argc && (string(argv[i]) == "--port-end")) {
            string p = string(argv[i + 1]);
            try {
                int pi = stoi(p);
                if (pi < 0 || pi > 65535)
                    throw;
                has_port_end = true;
                port_end = pi;
            } catch (exception &e) {
                cerr << "--port-end argument not recognized" << endl;
                return 1;
            }
        } else if (i + 1 < argc && (string(argv[i]) == "--cidr")) {
            string tmp = string(argv[i + 1]);
            if (cidr_to_protobuf(tmp, &cidr)) {
                cerr << "--cidr format not recognized" << endl;
                return 1;
            }
        } else if (i + 1 < argc && (string(argv[i]) == "--sg-members")) {
            sg_members = string(argv[i + 1]);
        }
    }

    if ((proto == 6 || proto == 17) &&
            (!has_port_start || !has_port_end || port_start > port_end)) {
        cerr << "wrong port configuration" << endl;
        return 1;
    }
    if (cidr.length() && sg_members.length()) {
        cerr << "--cidr can't be set with --sg-members" << endl;
        return 1;
    }
    if (!cidr.length() && !sg_members.length()) {
        cerr << "you must set --cidr or --sg-members" << endl;
        return 1;
    }
    if (!has_proto) {
        cerr << "--ip-proto is mandatory" << endl;
        return 1;
    }
    return 0;
}

static void sub_sg_rule_add_help(void) {
    cout << "usage: butterfly sg rule add SG [options...]" << endl << endl;
    cout << "Add a new firewalling rule inside a security group" << endl;
    cout << "options:" << endl <<
        "    --dir DIRECTION    rule direction (default: in)" << endl <<
        "    --ip-proto PROTO   IP protocol to allow (mandatory)" << endl <<
        "    --port PORT        open a single port" << endl <<
        "    --port-start PORT  port range start" << endl <<
        "    --port-end PORT    port range end" << endl <<
        "    --cidr CIDR        adress mask to allow in CIDR format" << endl <<
        "    --sg-members SG    security group members to allow" << endl <<
        endl <<
        "DIRECTION:" << endl << endl <<
        "Can only be 'in' (for inbound) at the moment, 'out' (for outbound) "
        "not supported yet" << endl << endl <<
        "PROTO:" << endl << endl <<
        "Must be 'tcp', 'udp', 'icmp', a number between 0 and 255 "
        "or 'all' to allow all protocols" << endl << endl <<
        "PORT:" << endl << endl <<
        "if you set udp or tcp in protocol, you can set a port between"
        " 0 and 65535" << endl << endl <<
        "Notes: you MUST set either --cidr or --sg-members" << endl;
    global_parameter_help();
}

static int sub_sg_rule_add(int argc, char **argv,
                            const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        sub_sg_rule_add_help();
        return 0;
    }

    RuleAddOptions o;
    if (o.parse(argc, argv)) {
        sub_sg_rule_add_help();
        return 1;
    }

    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      sg_rule_add {"
        "        sg_id: \"" + o.sg + "\""
        "        rule {"
        "          direction: " + o.direction +
        "          protocol: " + to_string(o.proto);
    if (o.has_port_start)
        req += "   port_start: " + to_string(o.port_start) +
               "   port_end: " + to_string(o.port_end);
    if (o.cidr.length())
        req += "   cidr { " + o.cidr + " }";
    else if (o.sg_members.length())
        req += "   security_group: \"" + o.sg_members + "\"";
    req +=
        "        }"
        "      }"
        "    }"
        "  }"
        "}";

    proto::Messages res;
    if (request(req, &res, options, false) || check_request_result(res))
        return 1;
    return 0;
}

static void sub_sg_rule_del_help(void) {
    cout << "usage: butterfly sg rule del SG RULE_HASH [options...]" << endl;
    cout << endl << "Remove a firewalling rule from a security group" << endl;
    cout << "You can get RULE_HASH from sg rule list subcommand" << endl;
    global_parameter_help();
}

static int sub_sg_rule_del(int argc, char **argv,
                            const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        sub_sg_rule_add_help();
        return 0;
    }

    if (argc <= 5) {
        sub_sg_rule_del_help();
        return 1;
    }

    string sg = string(argv[4]);
    string hash = string(argv[5]);
    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      sg_rule_list: \"" + sg + "\""
        "    }"
        "  }"
        "}";

    proto::Messages res;
    if (request(req, &res, options, false) || check_request_result(res))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    int size = res_0.sg_rule_list_size();
    string delete_req;
    for (int i = 0; i < size; i++) {
        if (hash == rule_hash(res_0.sg_rule_list(i))) {
            string rule_string;
            google::protobuf::TextFormat::PrintToString(res_0.sg_rule_list(i),
                                                        &rule_string);
            delete_req +=
                "messages {"
                "  revision: " PROTO_REV
                "  message_0 {"
                "    request {"
                "      sg_rule_del {"
                "        sg_id: \"" + sg + "\""
                "        rule {" + rule_string + "}"
                "      }"
                "    }"
                " }"
                "}";
            break;
        }
    }

    if (!delete_req.length()) {
        cerr << "no rule found with hash " << hash << endl;
        return 1;
    }

    proto::Messages delete_res;
    return request(delete_req, &delete_res, options, false) ||
        check_request_result(delete_res);
}

static void sub_sg_rule_help(void) {
    cout <<
        "butterfly sg rule subcommands:" << endl <<
        "    list  list security group rules" << endl <<
        "    add   create a new rule" << endl <<
        "    del   remove rule" << endl;
    global_parameter_help();
}

static int sub_sg_rule(int argc, char **argv, const GlobalOptions &options) {
    if (argc <= 3) {
        sub_sg_rule_help();
        return 1;
    }
    string cmd = string(argv[3]);
    if (cmd == "list") {
        return sub_sg_rule_list(argc, argv, options);
    } else if (cmd == "add") {
        return sub_sg_rule_add(argc, argv, options);
    } else if (cmd == "del") {
        return sub_sg_rule_del(argc, argv, options);
    } else if (cmd == "help") {
        sub_sg_rule_help();
        return 0;
    } else {
        cerr << "invalid sg rule subcommand " << cmd << endl;
        sub_sg_rule_help();
        return 1;
    }
}

static void sub_sg_member_list_help(void) {
    cout << "usage: butterfly sg member list SG [options...]" << endl << endl;
    cout << "List members of a security group" << endl;
    global_parameter_help();
}

static int sub_sg_member_list(int argc, char **argv,
                              const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        sub_sg_member_list_help();
        return 0;
    }

    if (argc <= 5) {
        sub_sg_member_list_help();
        return 1;
    }

    string sg = string(argv[4]);
    string req =
       "messages {"
       "  revision: " PROTO_REV
       "  message_0 {"
       "    request {"
       "      sg_member_list: \"" + sg + "\""
       "    }"
       "  }"
       "}";

    proto::Messages res;
    if (request(req, &res, options, false) || check_request_result(res))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    int size = res_0.sg_member_list_size();
    for (int i = 0; i < size; i++) {
        cout << res_0.sg_member_list(i) << endl;
    }
    return 0;
}

static void sub_sg_member_add_help(void) {
    cout << "usage: butterfly sg member add SG IP [options...]" << endl << endl;
    cout << "Add an member (IP) to a security group" << endl;
    global_parameter_help();
}

static int sub_sg_member_add(int argc, char **argv,
                             const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        sub_sg_member_add_help();
        return 0;
    }

    if (argc <= 6) {
        sub_sg_member_add_help();
        return 1;
    }
    string sg = string(argv[4]);
    string ip = string(argv[5]);
    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      sg_member_add {"
        "        sg_id: \"" + sg + "\""
        "        member: \"" + ip + "\""
        "      }"
        "    }"
        "  }"
        "}";
    proto::Messages res;
    return request(req, &res, options, false) || check_request_result(res);
}

static void sub_sg_member_del_help(void) {
    cout << "usage: butterfly sg member del SG IP [options...]" << endl << endl;
    cout << "Remove a member (IP) from a security group" << endl;
    global_parameter_help();
}

static int sub_sg_member_del(int argc, char **argv,
                             const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        sub_sg_member_del_help();
        return 0;
    }

    if (argc <= 6) {
        sub_sg_member_del_help();
        return 1;
    }
    string sg = string(argv[4]);
    string ip = string(argv[5]);
    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      sg_member_del {"
        "        sg_id: \"" + sg + "\""
        "        member: \"" + ip + "\""
        "      }"
        "    }"
        "  }"
        "}";
    proto::Messages res;
    return request(req, &res, options, false) || check_request_result(res);
}

static void sub_sg_member_help(void) {
    cout <<
        "butterfly sg member subcommands:" << endl <<
        "    list  list members of a security group" << endl <<
        "    add   add member to a security group" << endl <<
        "    del   remove member of a security group" << endl;
    global_parameter_help();
}

static int sub_sg_member(int argc, char **argv, const GlobalOptions &options) {
    if (argc <= 3) {
        sub_sg_member_help();
        return 1;
    }
    string cmd = string(argv[3]);
    if (cmd == "list") {
        return sub_sg_member_list(argc, argv, options);
    } else if (cmd == "add") {
        return sub_sg_member_add(argc, argv, options);
    } else if (cmd == "del") {
        return sub_sg_member_del(argc, argv, options);
    } else if (cmd == "help") {
        sub_sg_member_help();
        return 0;
    } else {
        cerr << "invalid sg member subcommand " << cmd << endl;
        sub_sg_member_help();
        return 1;
    }
}

static void sub_sg_help(void) {
    cout <<
        "butterfly sg subcommands:" << endl <<
        "    list    list security groups" << endl <<
        "    add     create one or more security groups" << endl <<
        "    del     remove one or more security groups" << endl <<
        "    rule    manage security group rules" << endl <<
        "    member  manage security group members" << endl;
    global_parameter_help();
}

int sub_sg(int argc, char **argv, const GlobalOptions &options) {
    if (argc <= 2) {
        sub_sg_help();
        return 1;
    }
    string cmd = string(argv[2]);
    if (cmd == "list") {
        return sub_sg_list(argc, argv, options);
    } else if (cmd == "add") {
        return sub_sg_add(argc, argv, options);
    } else if (cmd == "del") {
        return sub_sg_del(argc, argv, options);
    } else if (cmd == "rule") {
        return sub_sg_rule(argc, argv, options);
    } else if (cmd == "member") {
        return sub_sg_member(argc, argv, options);
    } else if (cmd == "help") {
        sub_sg_help();
        return 0;
    } else {
        cerr << "invalid sg subcommand " << cmd << endl;
        sub_sg_help();
        return 1;
    }
}
