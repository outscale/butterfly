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

static void sub_nic_list_help(void) {
    cout << "usage: butterfly nic list [options...]" << endl << endl;
    cout << "Show all created nics in butterfly" << endl;
    global_parameter_help();
}

static int sub_nic_list(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        sub_nic_list_help();
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
    if (request(req, &res, options, false) || check_request_result(res))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    int size = res_0.nic_list_size();
    for (int i = 0; i < size; i++) {
        cout << res_0.nic_list(i) << endl;
    }
    return 0;
}

static void sub_nic_stats_help(void) {
    cout << "usage: butterfly nic stats NIC [options...]" << endl << endl;
    cout << "Show some statistics of a vnic (in bytes)" << endl;
    global_parameter_help();
}

static int sub_nic_stats(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        sub_nic_stats_help();
        return 0;
    }

    if (argc <= 3) {
        sub_nic_stats_help();
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
    if (request(req, &res, options, false) || check_request_result(res))
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

static void sub_nic_details_help(void) {
    cout << "usage: butterfly details NIC [options...]" << endl << endl;
    cout << "Show vnic details" << endl;
    global_parameter_help();
}

static int sub_nic_details(int argc, char **argv,
                           const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        sub_nic_details_help();
        return 0;
    }

    if (argc <= 3) {
        sub_nic_details_help();
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
    if (request(req, &res, options, false) || check_request_result(res))
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
    return 0;
}

static int sg_list(string nic, vector<string> *list,
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
    if (request(req, &res, options, false) || check_request_result(res))
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

int sg_update(string nic, const vector<string> &list,
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
    return request(req, &res, options, false) || check_request_result(res);
}

static void sub_nic_sg_list_help(void) {
    cout << "usage: butterfly nic sg list NIC [options...]" << endl << endl;
    cout <<  "List security groups applied to a vnic" << endl;
    global_parameter_help();
}

static int sub_nic_sg_list(int argc, char **argv,
                           const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        sub_nic_sg_list_help();
        return 0;
    }

    if (argc <= 4) {
        sub_nic_sg_list_help();
        return 1;
    }
    string nic = string(argv[4]);
    vector<string> all_sg;
    if (sg_list(nic, &all_sg, options))
        return 1;
    for (string sg : all_sg)
        cout << sg << endl;
    return 0;
}

static void sub_nic_sg_add_help(void) {
    cout << "usage: butterfly nic sg add NIC SG [SG...] [options...]" << endl <<
        endl << "Add a security group to a vnic" << endl;
    global_parameter_help();
}

static int sub_nic_sg_add(int argc, char **argv,
                          const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        sub_nic_sg_add_help();
        return 0;
    }

    if (argc <= 5) {
        sub_nic_sg_add_help();
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
        sub_nic_sg_add_help();
        return 1;
    }

    vector<string> all_sg;
    if (sg_list(nic, &all_sg, options))
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

    return sg_update(nic, all_sg, options);
}

static void sub_nic_sg_set_help(void) {
    cout << "usage: butterfly nic sg set NIC SG [SG...] [options...]" << endl;
    cout << endl << "Overwrite security group listing of a vnic" << endl;
    global_parameter_help();
}

static int sub_nic_sg_set(int argc, char **argv,
                          const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        sub_nic_sg_set_help();
        return 0;
    }

    if (argc <= 5) {
        sub_nic_sg_set_help();
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

    return sg_update(nic, to_set, options);
}
static void sub_nic_sg_del_help(void) {
    cout << "usage: butterfly nic sg del NIC SG [SG...] [options...]" << endl;
    cout << endl << "Remove a security group from a vnic" << endl;
    global_parameter_help();
}

static int sub_nic_sg_del(int argc, char **argv,
                          const GlobalOptions &options) {
    if (argc >= 5 && string(argv[4]) == "help") {
        sub_nic_sg_del_help();
        return 0;
    }

    if (argc <= 5) {
        sub_nic_sg_del_help();
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
        sub_nic_sg_del_help();
        return 1;
    }

    vector<string> all_sg;
    if (sg_list(nic, &all_sg, options))
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
    return sg_update(nic, new_sg, options);
}

static void sub_nic_sg_help(void) {
    cout <<
        "butterfly nic sg subcommands:" << endl <<
        "    list  list security groups attached to a nic" << endl <<
        "    add   add one or more security group to a nic" << endl <<
        "    del   removes one or more security group of a nic" << endl <<
        "    set   update all security groups of a nic" << endl;
        global_parameter_help();
}

static int sub_nic_sg(int argc, char **argv, const GlobalOptions &options) {
    if (argc <= 3) {
        sub_nic_sg_help();
        return 1;
    }
    string cmd = string(argv[3]);
    if (cmd == "list") {
        return sub_nic_sg_list(argc, argv, options);
    } else if (cmd == "add") {
        return sub_nic_sg_add(argc, argv, options);
    } else if (cmd == "del") {
        return sub_nic_sg_del(argc, argv, options);
    } else if (cmd == "set") {
        return sub_nic_sg_set(argc, argv, options);
    } else if (cmd == "help") {
        sub_nic_sg_help();
        return 0;
    } else {
        cerr << "invalid nic sg subcommand " << cmd << endl;
        sub_nic_sg_help();
        return 1;
    }
}

NicAddOptions::NicAddOptions() {
    enable_antispoof = "false";
    bypass_filtering = "false";
}

int NicAddOptions::parse(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (i + 1 < argc && (string(argv[i]) == "--ip"))
            ips.push_back(string(argv[i + 1]));
        else if (i + 1 < argc && (string(argv[i]) == "--sg"))
            sgs.push_back(string(argv[i + 1]));
        else if (i + 1 < argc && (string(argv[i]) == "--mac"))
            mac = string(argv[i + 1]);
        else if (string(argv[i]) == "--enable-antispoof")
            enable_antispoof = "true";
        else if (i + 1 < argc && (string(argv[i]) == "--id"))
            id = string(argv[i + 1]);
        else if (i + 1 < argc && (string(argv[i]) == "--vni"))
            vni = string(argv[i + 1]);
        else if (string(argv[i]) == "--bypass-filtering")
            bypass_filtering = "true";
    }
    return !mac.length() || !id.length() || !vni.length();
}

static void sub_nic_add_help(void) {
    cout << "usage: butterfly nic add [options...]" << endl;
    cout  << endl <<
        "options:" << endl <<
        "    --ip IP             virtual interface's ip (v4 or v6)"
            << endl <<
        "    --mac MAC           virtual interface's mac (mandatory)" << endl <<
        "    --id ID             interface's id (mandatory)" << endl <<
        "    --vni VNI           virtual network id < 2^26 (mandatory)"
            << endl <<
        "    --enable-antispoof  enable antispoof protection (default: off)"
            << endl <<
        "    --bypass-filtering  remove all filters and protection" << endl;
    global_parameter_help();
}

static int sub_nic_add(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        sub_nic_add_help();
        return 0;
    }

    NicAddOptions o;
    if (o.parse(argc, argv)) {
        sub_nic_add_help();
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
        "        bypass_filtering: " + o.bypass_filtering +
        "      }"
        "    }"
        "  }"
        "}";

    proto::Messages res;
    if (request(req, &res, options, false) || check_request_result(res))
        return 1;

    MessageV0_Response res_0 = res.messages(0).message_0().response();
    if (!res_0.has_nic_add() || !res_0.nic_add().has_path()) {
        cerr << "no nic add result received" << endl;
        return 1;
    }
    cout << res_0.nic_add().path() << endl;
    return 0;
}

static void sub_nic_del_help(void) {
    cout << "usage: butterfly nic del NIC [NIC...] [options...]" << endl;
    cout << endl << "Destroy one or more vnic" << endl;
    global_parameter_help();
}

static int sub_nic_del(int argc, char **argv,
                       const GlobalOptions &options) {
    if (argc >= 4 && string(argv[3]) == "help") {
        sub_nic_details_help();
        return 0;
    }

    if (argc <= 3) {
        sub_nic_del_help();
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
        sub_nic_del_help();
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
    return request(req, &res, options, false) || check_request_result(res);
}

static void sub_nic_help(void) {
    cout <<
        "butterfly nic subcommands:" << endl <<
        "    list     list all nics id" << endl <<
        "    stats    show nic statistics" << endl <<
        "    details  prints nics's details" << endl <<
        "    sg       manage security groups attached to a nic " << endl <<
        "    add      create a new nic" << endl <<
        "    del      remove nic(s)" << endl;
        global_parameter_help();
}

int sub_nic(int argc, char **argv, const GlobalOptions &options) {
    if (argc <= 2) {
        sub_nic_help();
        return 1;
    }
    string cmd = string(argv[2]);
    if (cmd == "list") {
        return sub_nic_list(argc, argv, options);
    } else if (cmd == "stats") {
        return sub_nic_stats(argc, argv, options);
    } else if (cmd == "details") {
        return sub_nic_details(argc, argv, options);
    } else if (cmd == "sg") {
        return sub_nic_sg(argc, argv, options);
    } else if (cmd == "add") {
        return sub_nic_add(argc, argv, options);
    } else if (cmd == "del") {
        return sub_nic_del(argc, argv, options);
    } else if (cmd == "help") {
        sub_nic_help();
        return 0;
    } else {
        cerr << "invalid nic subcommand " << cmd << endl;
        sub_nic_help();
        return 1;
    }
}
