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

static void SubSgListHelp(void) {
    cout << "usage: butterfly sg list [options...]" << endl << endl;
    cout << "List all available security groups" << endl;
    GlobalParameterHelp();
}

static int SubSgList(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        SubSgListHelp();
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

    if (Request(req, &res, options, false) || CheckRequestResult(res))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    int size = res_0.sg_list_size();
    for (int i = 0; i < size; i++) {
        cout << res_0.sg_list(i) << endl;
    }
    return 0;
}

static void SubSgAddHelp(void) {
    cout << "usage: butterfly sg add SG [SG...] [options...]" << endl << endl;
    cout << "Create a new security group" << endl;
    cout <<
    "Note that creating a new security group when it already " << endl <<
    "exists will overwrite the existing SG with an empty one." << endl;
    GlobalParameterHelp();
}

static int SubSgAdd(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        SubSgAddHelp();
        return 0;
    }

    if (argc <= 3) {
        SubSgAddHelp();
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
        SubSgAddHelp();
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
    return Request(req, &res, options, false) || CheckRequestResult(res);
}

static void SubSgDelHelp(void) {
    cout << "usage: butterfly sg del SG [SG...] [options...]" << endl << endl;
    cout << "Delete one or more security group" << endl;
    GlobalParameterHelp();
}

static int SubSgDel(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        SubSgDelHelp();
        return 0;
    }

    if (argc <= 3) {
        SubSgDelHelp();
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
        SubSgDelHelp();
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
    return Request(req, &res, options, false) || CheckRequestResult(res);
}

static string RuleHash(const MessageV0_Rule &r) {
    string human_message;
    google::protobuf::TextFormat::PrintToString(r, &human_message);
    hash<string> fn;
    size_t hash = fn(human_message);
    ostringstream out;
    out << setfill('0') << setw(16) << hex << hash;
    return out.str();
}

static void RulePrint(const MessageV0_Rule &r) {
    cout << RuleHash(r) << " | " <<
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

static void SubSgRuleListHelp(void) {
    cout << "usage: butterfly sg rule list SG [options...]" << endl << endl;
    cout << "List all firewalling rules of a security group" << endl;
    GlobalParameterHelp();
}

static int SubSgRuleList(int argc, char **argv,
                            const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        SubSgRuleListHelp();
        return 0;
    }

    if (argc <= 4) {
        SubSgRuleListHelp();
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

    if (Request(req, &res, options, false) || CheckRequestResult(res))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    int size = res_0.sg_rule_list_size();
    for (int i = 0; i < size; i++) {
        RulePrint(res_0.sg_rule_list(i));
    }
    return 0;
}

RuleAddOptions::RuleAddOptions() {
    direction = "INBOUND";
    has_proto = false;
    has_port_start = false;
    has_port_end = false;
}

static int CidrToProtobuf(const string &in, string *out) {
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

int RuleAddOptions::Parse(int argc, char **argv) {
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
            if (CidrToProtobuf(tmp, &cidr)) {
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

static void SubSgRuleAddHelp(void) {
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
    GlobalParameterHelp();
}

static int SubSgRuleAdd(int argc, char **argv,
                            const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        SubSgRuleAddHelp();
        return 0;
    }

    RuleAddOptions o;
    if (o.Parse(argc, argv)) {
        SubSgRuleAddHelp();
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

    if (Request(req, &res, options, false) || CheckRequestResult(res))
        return 1;
    return 0;
}

static void SubSgRuleDelHelp(void) {
    cout << "first usage: butterfly sg rule del SG RULE_HASH" << endl;
    cout << "second usage: butterfly sg rule del SG [options...]" << endl;
    cout << endl << "Remove a firewalling rule from a security group" << endl;
    cout << "You can get RULE_HASH from sg rule list subcommand" << endl;
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
    GlobalParameterHelp();
}

static bool CheckHash(const string &param) {
    return param[0] == '-';
}

static int SubSgHashedRuleDel(int argc, char **argv,
                                  const GlobalOptions &options) {
    string sg(argv[4]);
    string hash(argv[5]);
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

    if (Request(req, &res, options, false) || CheckRequestResult(res))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    int size = res_0.sg_rule_list_size();
    string delete_req;
    for (int i = 0; i < size; i++) {
        if (hash == RuleHash(res_0.sg_rule_list(i))) {
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
    return Request(delete_req, &delete_res, options, false) ||
            CheckRequestResult(delete_res);
}

static int SubSgParamRuleDel(int argc, char **argv,
                                 const GlobalOptions &options) {
    RuleAddOptions opts;
    if (opts.Parse(argc, argv)) {
        SubSgRuleDelHelp();
        return 1;
    }

    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      sg_rule_del {"
        "        sg_id: \"" + opts.sg + "\""
        "        rule {"
        "          direction: " + opts.direction +
        "          protocol: " + to_string(opts.proto);
    if (opts.has_port_start) {
        req += "   port_start: " + to_string(opts.port_start) +
               "   port_end     : " + to_string(opts.port_end);
    }
    if (opts.cidr.length())
        req += "   cidr { " + opts.cidr + " }";
    else if (opts.sg_members.length())
        req += "   security_group: \"" + opts.sg_members + "\"";
    req +=
        "        }"
        "      }"
        "    }"
        "  }"
        "}";

    proto::Messages res;
    return Request(req, &res, options, false) || CheckRequestResult(res);
}
static int SubSgRuleDel(int argc, char **argv,
                            const GlobalOptions &options) {
    if ((argc >= 5 && strcmp(argv[4], "help") == 0) || (argc <= 5)) {
        SubSgRuleDelHelp();
        return 0;
    }

    string sg(argv[4]);
    string param(argv[5]);
    if (!CheckHash(param))
        return SubSgHashedRuleDel(argc, argv, options);
    return SubSgParamRuleDel(argc, argv, options);
}

static void SubSgRuleHelp(void) {
    cout <<
        "butterfly sg rule subcommands:" << endl <<
        "    list  list security group rules" << endl <<
        "    add   create a new rule" << endl <<
        "    del   remove rule" << endl;
    GlobalParameterHelp();
}

static int SubSgRule(int argc, char **argv, const GlobalOptions &options) {
    if (argc <= 3) {
        SubSgRuleHelp();
        return 1;
    }
    string cmd = string(argv[3]);
    if (cmd == "list") {
        return SubSgRuleList(argc, argv, options);
    } else if (cmd == "add") {
        return SubSgRuleAdd(argc, argv, options);
    } else if (cmd == "del") {
        return SubSgRuleDel(argc, argv, options);
    } else if (cmd == "help") {
        SubSgRuleHelp();
        return 0;
    } else {
        cerr << "invalid sg rule subcommand " << cmd << endl;
        SubSgRuleHelp();
        return 1;
    }
}

static void SubSgMemberListHelp(void) {
    cout << "usage: butterfly sg member list SG [options...]" << endl << endl;
    cout << "List members of a security group" << endl;
    GlobalParameterHelp();
}

static int sub_sg_member_list(int argc, char **argv,
                              const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        SubSgMemberListHelp();
        return 0;
    }

    if (argc <= 5) {
        SubSgMemberListHelp();
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

    if (Request(req, &res, options, false) || CheckRequestResult(res))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    int size = res_0.sg_member_list_size();
    for (int i = 0; i < size; i++) {
        cout << res_0.sg_member_list(i) << endl;
    }
    return 0;
}

static void SubSgMemberAddHelp(void) {
    cout << "usage: butterfly sg member add SG IP [options...]" << endl << endl;
    cout << "Add an member (IP) to a security group" << endl;
    GlobalParameterHelp();
}

static int SubSgMemberAdd(int argc, char **argv,
                             const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        SubSgMemberAddHelp();
        return 0;
    }

    if (argc < 6) {
        SubSgMemberAddHelp();
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
    return Request(req, &res, options, false) || CheckRequestResult(res);
}

static void SubSgMemberDelHelp(void) {
    cout << "usage: butterfly sg member del SG IP [options...]" << endl << endl;
    cout << "Remove a member (IP) from a security group" << endl;
    GlobalParameterHelp();
}

static int sub_sg_member_del(int argc, char **argv,
                             const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        SubSgMemberDelHelp();
        return 0;
    }

    if (argc <= 6) {
        SubSgMemberDelHelp();
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
    return Request(req, &res, options, false) || CheckRequestResult(res);
}

static void SubSgMemberHelp(void) {
    cout <<
        "butterfly sg member subcommands:" << endl <<
        "    list  list members of a security group" << endl <<
        "    add   add member to a security group" << endl <<
        "    del   remove member of a security group" << endl;
    GlobalParameterHelp();
}

static int SubSgMember(int argc, char **argv, const GlobalOptions &options) {
    if (argc <= 3) {
        SubSgMemberHelp();
        return 1;
    }
    string cmd = string(argv[3]);
    if (cmd == "list") {
        return sub_sg_member_list(argc, argv, options);
    } else if (cmd == "add") {
        return SubSgMemberAdd(argc, argv, options);
    } else if (cmd == "del") {
        return sub_sg_member_del(argc, argv, options);
    } else if (cmd == "help") {
        SubSgMemberHelp();
        return 0;
    } else {
        cerr << "invalid sg member subcommand " << cmd << endl;
        SubSgMemberHelp();
        return 1;
    }
}

static void SubSgHelp(void) {
    cout <<
        "butterfly sg subcommands:" << endl <<
        "    list    list security groups" << endl <<
        "    add     create one or more security groups" << endl <<
        "    del     remove one or more security groups" << endl <<
        "    rule    manage security group rules" << endl <<
        "    member  manage security group members" << endl;
    GlobalParameterHelp();
}

int SubSg(int argc, char **argv, const GlobalOptions &options) {
    if (argc <= 2) {
        SubSgHelp();
        return 1;
    }
    string cmd = string(argv[2]);
    if (cmd == "list") {
        return SubSgList(argc, argv, options);
    } else if (cmd == "add") {
        return SubSgAdd(argc, argv, options);
    } else if (cmd == "del") {
        return SubSgDel(argc, argv, options);
    } else if (cmd == "rule") {
        return SubSgRule(argc, argv, options);
    } else if (cmd == "member") {
        return SubSgMember(argc, argv, options);
    } else if (cmd == "help") {
        SubSgHelp();
        return 0;
    } else {
        cerr << "invalid sg subcommand " << cmd << endl;
        SubSgHelp();
        return 1;
    }
}
