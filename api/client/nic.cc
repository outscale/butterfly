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

static void SubNicListHelp(void) {
    cout << "usage: butterfly nic list [options...]" << endl << endl;
    cout << "Show all created nics in butterfly" << endl;
    GlobalParameterHelp();
}

static int SubNicList(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        SubNicListHelp();
        return 0;
    }

    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      nic_list: true"
        "    }"
        "  }"
        "}";

    proto::Messages res;
    if (Request(req, &res, options, false))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    int size = res_0.nic_list_size();
    for (int i = 0; i < size; i++) {
        cout << res_0.nic_list(i) << endl;
    }
    return 0;
}

static void SubNicStatsHelp(void) {
    cout << "usage: butterfly nic stats NIC [options...]" << endl << endl;
    cout << "Show some statistics of a vnic (in bytes)" << endl;
    GlobalParameterHelp();
}

static int SubNicStats(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        SubNicStatsHelp();
        return 0;
    }

    if (argc <= 3) {
        SubNicStatsHelp();
        return 1;
    }
    string nic = string(argv[3]);
    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      nic_stats: \"" + nic + "\""
        "    }"
        "  }"
        "}";

    proto::Messages res;

    if (Request(req, &res, options, false))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    if (!res_0.has_nic_stats()) {
        cerr << "no nic stats received" << endl;
        return 1;
    }

    MessageV0_NicStats stats = res_0.nic_stats();
    cout << "in: " << to_string(stats.in()) << endl;
    cout << "out: " << to_string(stats.out()) << endl;
    return 0;
}

static void SubNicDetailsHelp(void) {
    cout << "usage: butterfly details NIC [options...]" << endl << endl;
    cout << "Show vnic details" << endl;
    GlobalParameterHelp();
}

static int SubNicDetails(int argc, char **argv,
                           const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        SubNicDetailsHelp();
        return 0;
    }

    if (argc <= 3) {
        SubNicDetailsHelp();
        return 1;
    }
    string nic = string(argv[3]);
    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      nic_details: \"" + nic + "\""
        "    }"
        "  }"
        "}";

    proto::Messages res;

    if (Request(req, &res, options, false))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    if (res_0.nic_details_size() == 0) {
        cerr << "no nic details received" << endl;
        return 1;
    }

    MessageV0_Nic details = res_0.nic_details(0);
    cout << "id: " << details.id() << endl;
    cout << "mac: " << details.mac() << endl;
    cout << "vni: " << to_string(details.vni()) << endl;
    for (int i = 0; i < details.ip_size(); i++)
        cout << "ip: " << details.ip(i) << endl;
    if (details.has_ip_anti_spoof())
        cout << "antispoof: " <<
            (details.ip_anti_spoof() ? "true" : "false") << endl;
    if (details.has_sniff_target_nic_id())
        cout << "sniff target: " << details.sniff_target_nic_id() << endl;
    if (details.has_bypass_filtering())
        cout << "bypass filtering: " <<
            (details.bypass_filtering() ? "true" : "false") << endl;
    if (details.has_packet_trace())
        cout << "packet trace: " <<
            (details.packet_trace() ? "true" : "false") << endl;
    return 0;
}

static int SgList(string nic, vector<string> *list,
                   const GlobalOptions &options) {
    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      nic_details: \"" + nic + "\""
        "    }"
        "  }"
        "}";

    proto::Messages res;

    if (Request(req, &res, options, false))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    if (res_0.nic_details_size() == 0) {
        cerr << "no nic details received" << endl;
        return 1;
    }

    MessageV0_Nic details = res_0.nic_details(0);
    int size = details.security_group_size();
    for (int i = 0; i < size; i++)
        list->push_back(details.security_group(i));
    return 0;
}

int SgUpdate(string nic, const vector<string> &list,
              const GlobalOptions &options) {
    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      nic_update: {"
        "        id: \"" + nic + "\"";
    for (string sg : list)
        req += " security_group: \"" + sg + "\"";
    req +=
        "      }"
        "    }"
        "  }"
        "}";

    proto::Messages res;
    return Request(req, &res, options, false);
}

static void SubNicSgListHelp(void) {
    cout << "usage: butterfly nic sg list NIC [options...]" << endl << endl;
    cout <<  "List security groups applied to a vnic" << endl;
    GlobalParameterHelp();
}

static int SubNicSgList(int argc, char **argv,
                           const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        SubNicSgListHelp();
        return 0;
    }

    if (argc <= 4) {
        SubNicSgListHelp();
        return 1;
    }
    string nic = string(argv[4]);
    vector<string> all_sg;
    if (SgList(nic, &all_sg, options))
        return 1;
    for (string sg : all_sg)
        cout << sg << endl;
    return 0;
}

static void SubNicSgAddHelp(void) {
    cout << "usage: butterfly nic sg add NIC SG [SG...] [options...]" << endl <<
        endl << "Add a security group to a vnic" << endl;
    GlobalParameterHelp();
}

static int SubNicSgAdd(int argc, char **argv,
                          const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        SubNicSgAddHelp();
        return 0;
    }

    if (argc <= 5) {
        SubNicSgAddHelp();
        return 1;
    }

    string nic = string(argv[4]);
    vector<string> to_add;
    for (int i = 5; i < argc; i++) {
        string a = string(argv[i]);
        if (a[0] == '-')
            break;
        to_add.push_back(a);
    }
    if (!to_add.size()) {
        SubNicSgAddHelp();
        return 1;
    }

    vector<string> all_sg;
    if (SgList(nic, &all_sg, options))
        return 1;

    // merge existing list with new list
    uint32_t old_size = all_sg.size();
    all_sg.insert(all_sg.end(), to_add.begin(), to_add.end());
    sort(all_sg.begin(), all_sg.end());
    auto last = unique(all_sg.begin(), all_sg.end());
    all_sg.erase(last, all_sg.end());

    if (all_sg.size() == old_size) {
        return 0;
    }

    return SgUpdate(nic, all_sg, options);
}

static void SubNicSgSetHelp(void) {
    cout << "usage: butterfly nic sg set NIC SG [SG...] [options...]" << endl;
    cout << endl << "Overwrite security group listing of a vnic" << endl;
    GlobalParameterHelp();
}

static int SubNicSgSet(int argc, char **argv,
                          const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        SubNicSgSetHelp();
        return 0;
    }

    if (argc <= 5) {
        SubNicSgSetHelp();
        return 1;
    }
    string nic = string(argv[4]);
    vector<string> to_set;
    for (int i = 5; i < argc; i++) {
        string check = string(argv[i]);
        if (check[0] == '-')
            break;
        to_set.push_back(check);
    }
    if (!to_set.size()) {
        to_set.push_back("");
    }

    return SgUpdate(nic, to_set, options);
}

static void SubNicSgDelHelp(void) {
    cout << "usage: butterfly nic sg del NIC SG [SG...] [options...]" << endl;
    cout << endl << "Remove a security group from a vnic" << endl;
    GlobalParameterHelp();
}

static int SubNicSgDel(int argc, char **argv,
                          const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        SubNicSgDelHelp();
        return 0;
    }

    if (argc <= 5) {
        SubNicSgDelHelp();
        return 1;
    }

    string nic = string(argv[4]);
    vector<string> to_del;
    for (int i = 5; i < argc; i++) {
        string a = string(argv[i]);
        if (a[0] == '-')
            break;
        to_del.push_back(a);
    }
    if (!to_del.size()) {
        SubNicSgDelHelp();
        return 1;
    }

    vector<string> all_sg;
    if (SgList(nic, &all_sg, options))
        return 1;

    vector<string> new_sg;
    for (string sg : all_sg) {
        auto r = find(to_del.begin(), to_del.end(), sg);
        if (r == to_del.end())
            new_sg.push_back(sg);
    }

    if (new_sg.size() == all_sg.size()) {
        return 0;
    }
    return SgUpdate(nic, new_sg, options);
}

static void SubNicSgHelp(void) {
    cout <<
        "butterfly nic sg subcommands:" << endl <<
        "    list  list security groups attached to a nic" << endl <<
        "    add   add one or more security group to a nic" << endl <<
        "    del   removes one or more security group of a nic" << endl <<
        "    set   update all security groups of a nic" << endl;
        GlobalParameterHelp();
}

static int SubNicSg(int argc, char **argv, const GlobalOptions &options) {
    if (argc <= 3) {
        SubNicSgHelp();
        return 1;
    }
    string cmd = string(argv[3]);
    if (cmd == "list") {
        return SubNicSgList(argc, argv, options);
    } else if (cmd == "add") {
        return SubNicSgAdd(argc, argv, options);
    } else if (cmd == "del") {
        return SubNicSgDel(argc, argv, options);
    } else if (cmd == "set") {
        return SubNicSgSet(argc, argv, options);
    } else if (cmd == "help") {
        SubNicSgHelp();
        return 0;
    } else {
        cerr << "invalid nic sg subcommand " << cmd << endl;
        SubNicSgHelp();
        return 1;
    }
}

NicAddOptions::NicAddOptions() {
    packet_trace = "";
    packet_trace_path = "";
    enable_antispoof = "false";
    bypass_filtering = "false";
    type = "VHOST_USER_SERVER";
}

NicUpdateOptions::NicUpdateOptions() {
    packet_trace = "";
    packet_trace_path = "";
    enable_antispoof = "";
}

static inline bool CheckOption(int count, int argc, char **argv,
                               char const *option) {
    return count + 1 < argc && string(argv[count]) == option;
}

int NicAddOptions::Parse(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (CheckOption(i, argc, argv, "--ip"))
            ips.push_back(string(argv[i + 1]));
        else if (CheckOption(i, argc, argv, "--sg"))
            sgs.push_back(string(argv[i + 1]));
        else if (CheckOption(i, argc, argv, "--mac"))
            mac = string(argv[i + 1]);
        else if (string(argv[i]) == "--enable-antispoof")
            enable_antispoof = "true";
        else if (CheckOption(i, argc, argv, "--id"))
            id = string(argv[i + 1]);
        else if (CheckOption(i, argc, argv, "--type"))
            type = string(argv[i + 1]);
        else if (CheckOption(i, argc, argv, "--vni"))
            vni = string(argv[i + 1]);
        else if (string(argv[i]) == "--bypass-filtering")
            bypass_filtering = "true";
        else if (string(argv[i]) == "--packet-trace")
            packet_trace = "packet_trace: " + string(argv[i + 1]);
        else if (string(argv[i]) == "--trace-path")
            packet_trace_path = "packet_trace_path: \"" +
                                 string(argv[i + 1]) + "\"";
    }

    if (!packet_trace_path.empty() &&
        (packet_trace.empty() || packet_trace == "packet_trace: false"))
        return 1;

    if (type != "VHOST_USER_SERVER" && type != "TAP")
        return 1;
    return !mac.length() || !id.length() || !vni.length();
}

int NicUpdateOptions::Parse(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (CheckOption(i, argc, argv, "--ip"))
            ips.push_back(string(argv[i + 1]));
        else if (string(argv[i]) == "--enable-antispoof")
            enable_antispoof = "ip_anti_spoof: " + string(argv[i + 1]);
        else if (CheckOption(i, argc, argv, "--id"))
            id = string(argv[i + 1]);
        else if (string(argv[i]) == "--packet-trace")
            packet_trace = "packet_trace: " + string(argv[i + 1]);
        else if (string(argv[i]) == "--trace-path")
            packet_trace_path = "packet_trace_path: \"" +
                                string(argv[i + 1]) + "\"";
    }
    if (id.empty())
        return 1;
    if (!packet_trace_path.empty() &&
        (packet_trace.empty() || packet_trace == "packet_trace: false"))
        return 1;
    return 0;
}

static void SubNicAddHelp(void) {
    cout << "usage: butterfly nic add [options...]" << endl;
    cout  << endl <<
        "options:" << endl <<
        "    --ip IP             virtual interface's ip (v4 or v6)"
            << endl <<
        "    --mac MAC           virtual interface's mac (mandatory)" << endl <<
        "    --id ID             interface's id (mandatory)" << endl <<
        "    --type TYPE         nic type (VHOST_USER_SERVER or " <<
        "    TAP default: VHOST_USER_SERVER)" << endl <<
        "    --vni VNI           virtual network id < 2^26 (mandatory)"
            << endl <<
        "    --enable-antispoof  enable antispoof protection (default: off)"
            << endl <<
        "    --packet-trace      true/false  trace a nic or not" <<
        "    (default: use server behaviour)" << endl <<
        "    --trace-path PATH    where to store pcap file if packet-trace" <<
        "    was set true (default: PATH = /tmp/butterfly-PID-nic-NICID.pcap)"
            << endl <<
        "    --bypass-filtering  remove all filters and protection" << endl;
    GlobalParameterHelp();
}

static void SubNicUpdateHelp(void) {
    cout << "usage: butterfly nic update [options...]" << endl;
    cout  << endl <<
        "options:" << endl <<
        "    --ip IP             virtual interface's ip (v4 or v6)"
            << endl <<
        "    --id ID             interface's id (mandatory)" << endl <<
        "    --enable-antispoof  true/fasle  enable antispoof" << endl <<
        "    --packet-trace      true/false  trace a nic or not" <<
        "    (default: use server behaviour)" << endl <<
        "    --trace-path PATH    where to store pcap file if packet-trace" <<
        "    is set true (default: PATH = /tmp/butterfly-PID-nic-NICID.pcap)"
            << endl;
    GlobalParameterHelp();
}

static int SubNicAdd(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        SubNicAddHelp();
        return 0;
    }

    NicAddOptions o;
    if (o.Parse(argc, argv)) {
        SubNicAddHelp();
        return 1;
    }

    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      nic_add {"
        "        id: \"" + o.id + "\""
        "        mac: \"" + o.mac + "\""
        "        vni: " + o.vni;
    for (string ip : o.ips)
        req += " ip: \"" + ip + "\"";
    for (string sg : o.sgs)
        req += " security_group: \"" + sg + "\"";
    req +=
        "        ip_anti_spoof: " + o.enable_antispoof +
        "        type: " + o.type +
        "        " + o.packet_trace +
        "        " + o.packet_trace_path +
        "        bypass_filtering: " + o.bypass_filtering +
        "      }"
        "    }"
        "  }"
        "}";

    proto::Messages res;

    if (Request(req, &res, options, false))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    if (!res_0.has_nic_add() || !res_0.nic_add().has_path()) {
        cerr << "no nic add result received" << endl;
        return 1;
    }
    cout << res_0.nic_add().path() << endl;
    return 0;
}

static int SubNicUpdate(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        SubNicAddHelp();
        return 0;
    }

    NicUpdateOptions o;
    if (o.Parse(argc, argv)) {
        SubNicUpdateHelp();
        return 1;
    }

    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    request {"
        "      nic_update {"
        "        id: \"" + o.id + "\"";
    for (string ip : o.ips) {
        if (ip == "-e")
            req += " ip: \"\"";
        else if (!ip.empty())
            req += " ip: \"" + ip + "\"";
    }
    if (!o.enable_antispoof.empty())
        req += "  " + o.enable_antispoof + "";
    req +=
        "        " + o.packet_trace +
        "        " + o.packet_trace_path +
        "      }"
        "    }"
        "  }"
        "}";

    proto::Messages res;

    if (Request(req, &res, options, false)) {
        SubNicUpdateHelp();
        return 1;
    }
    return 0;
}

static void SubNicDelHelp(void) {
    cout << "usage: butterfly nic del NIC [NIC...] [options...]" << endl;
    cout << endl << "Destroy one or more vnic" << endl;
    GlobalParameterHelp();
}

static int SubNicDel(int argc, char **argv,
                       const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        SubNicDetailsHelp();
        return 0;
    }

    if (argc <= 3) {
        SubNicDelHelp();
        return 1;
    }

    vector<string> nics;
    for (int i = 3; i < argc; i++) {
        string a = string(argv[i]);
        if (a[0] == '-')
            break;
        nics.push_back(a);
    }
    if (!nics.size()) {
        SubNicDelHelp();
        return 1;
    }

    string req;
    for (string nic : nics) {
        req +=
            "messages {"
            "  revision: " PROTO_REV
            "  message_0 {"
            "    request {"
            "      nic_del: \"" + nic + "\""
            "    }"
            "  }"
            "}";
    }

    proto::Messages res;
    return Request(req, &res, options, false);
}

static void SubNicHelp(void) {
    cout <<
        "butterfly nic subcommands:" << endl <<
        "    list     list all nics id" << endl <<
        "    stats    show nic statistics" << endl <<
        "    details  prints nics's details" << endl <<
        "    sg       manage security groups attached to a nic " << endl <<
        "    add      create a new nic" << endl <<
        "    del      remove nic(s)" << endl <<
        "    update   update a nic" << endl <<
        "    help     print this and return 0" << endl;
        GlobalParameterHelp();
}

int SubNic(int argc, char **argv, const GlobalOptions &options) {
    if (argc <= 2) {
        SubNicHelp();
        return 1;
    }
    string cmd = string(argv[2]);
    if (cmd == "list") {
        return SubNicList(argc, argv, options);
    } else if (cmd == "stats") {
        return SubNicStats(argc, argv, options);
    } else if (cmd == "details") {
        return SubNicDetails(argc, argv, options);
    } else if (cmd == "sg") {
        return SubNicSg(argc, argv, options);
    } else if (cmd == "add") {
        return SubNicAdd(argc, argv, options);
    } else if (cmd == "update") {
        return SubNicUpdate(argc, argv, options);
    } else if (cmd == "del") {
        return SubNicDel(argc, argv, options);
    } else if (cmd == "help") {
        SubNicHelp();
        return 0;
    } else {
        cerr << "invalid nic subcommand " << cmd << endl;
        SubNicHelp();
        return 1;
    }
}
