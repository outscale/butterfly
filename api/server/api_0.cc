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

extern "C" {
#include <arpa/inet.h>
}
#include <algorithm>
#include "api/server/api.h"
#include "api/server/app.h"

void Api0::Process(const MessageV0 &req, MessageV0 *res) {
    if (res == nullptr)
        return;
    res->set_allocated_response(new MessageV0_Response);

    // Set back message_id if set
    if (req.has_message_id())
        res->set_message_id(req.message_id());

    auto rs = res->mutable_response();
    if (!req.has_request()) {
        BuildNokRes(rs, "MessageV0 appears to not have any request inside");
        return;
    }

    auto rq = req.request();

    if (rq.has_nic_add())
        NicAdd(rq, rs);
    else if (rq.has_nic_update())
        NicUpdate(rq, rs);
    else if (rq.has_nic_del())
        NicDel(rq, rs);
    else if (rq.has_nic_list())
        NicList(rq, rs);
    else if (rq.has_nic_details())
        NicDetails(rq, rs);
    else if (rq.has_nic_export())
        NicExport(rq, rs);
    else if (rq.has_nic_stats())
        NicStats(rq, rs);
    else if (rq.has_sg_add())
        SgAdd(rq, rs);
    else if (rq.has_sg_del())
        SgDel(rq, rs);
    else if (rq.has_sg_list())
        SgList(rq, rs);
    else if (rq.has_sg_rule_add())
        SgRuleAdd(rq, rs);
    else if (rq.has_sg_rule_del())
        SgRuleDel(rq, rs);
    else if (rq.has_sg_rule_list())
        SgRuleList(rq, rs);
    else if (rq.has_sg_member_add())
        SgMemberAdd(rq, rs);
    else if (rq.has_sg_member_del())
        SgMemberDel(rq, rs);
    else if (rq.has_sg_member_list())
        SgMemberList(rq, rs);
    else if (rq.has_app_status())
        AppStatus(rq, rs);
    else if (rq.has_app_quit())
        AppQuit(rq, rs);
    else if (rq.has_app_config())
        AppConfig(rq, rs);
    else if (rq.has_sg_details())
        SgDetails(rq, rs);
    else
        BuildNokRes(rs, "MessageV0 appears to not have any request");
}

void Api0::NicAdd(const MessageV0_Request &req, MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("NIC added");
    auto n = req.nic_add();
    if (!ValidateNic(n)) {
        BuildNokRes(res, "Bad NIC format");
        return;
    }

    app::Nic nic;
    if (!Convert(n, &nic)) {
        BuildNokRes(res, "Internal error");
        return;
    }

    app::Error error;
    if (!ActionNicAdd(&nic, &error)) {
        BuildNokRes(res, error);
        return;
    }

    res->set_allocated_nic_add(new MessageV0_NicAddRes);
    res->mutable_nic_add()->set_path(nic.path);
    BuildOkRes(res);
}

void Api0::NicUpdate(const MessageV0_Request &req,
    MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("NIC updated");
    auto n = req.nic_update();
    if (!ValidateNicUpdate(n)) {
        BuildNokRes(res, "Bad Nic update format");
        return;
    }

    Api::NicUpdate nic_update;
    if (!Convert(n, &nic_update)) {
        BuildNokRes(res, "Internal error");
        return;
    }

    app::Error error;
    if (!ActionNicUpdate(nic_update, &error)) {
        BuildNokRes(res, error);
        return;
    }

    BuildOkRes(res);
}

void Api0::NicDel(const MessageV0_Request &req, MessageV0_Response *res) {
    if (res == nullptr)
        return;

    app::log.Info("NIC deletion");
    app::Error error;
    if (!ActionNicDel(req.nic_del(), &error)) {
        BuildNokRes(res, error);
        return;
    }
    BuildOkRes(res);
}

void Api0::NicList(const MessageV0_Request &req, MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("NIC listing");
    BuildOkRes(res);
    app::Model &m = app::model;
    for (auto it=m.nics.begin(); it != m.nics.end(); it++)
        res->add_nic_list(it->second.id);
}

void Api0::NicDetails(const MessageV0_Request &req,
    MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("NIC details");
    app::Model &m = app::model;
    std::string id = req.nic_details();
    // Details of one NIC only
    if (id.length() > 0) {
        auto it = m.nics.find(id);
        if (it == m.nics.end()) {
            BuildNokRes(res, "NIC not found");
            return;
        }

        auto n = res->add_nic_details();
        if (!Convert(it->second, n)) {
            BuildNokRes(res, "Internal error");
            return;
        }
        BuildOkRes(res);
        return;
    }
    // Details of all NICs
    for (auto it=m.nics.begin(); it != m.nics.end(); it++) {
        auto n = res->add_nic_details();
        if (!Convert(it->second, n)) {
            BuildNokRes(res, "Internal error");
            return;
        }
    }
    BuildOkRes(res);
}

void Api0::NicExport(const MessageV0_Request &req,
    MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("NIC export");
    std::string nic_id = req.nic_export();
    std::string nic_data;
    app::Error err;
    if (!ActionNicExport(nic_id, &nic_data, &err)) {
        BuildNokRes(res, err);
        return;
    }
    res->set_allocated_nic_export(new MessageV0_NicExportRes);
    auto nic_data_res = res->mutable_nic_export();

    nic_data_res->set_allocated_fw_data(new MessageV0_FwData);
    auto fw_data = nic_data_res->mutable_fw_data();

    fw_data->set_data(nic_data);
    BuildOkRes(res);
}

void Api0::NicStats(const MessageV0_Request &req,
    MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("NIC stats");
    std::string nic_id = req.nic_stats();
    uint64_t in, out = 0;
    app::Error err;
    if (!ActionNicStats(nic_id, &in, &out, &err)) {
        BuildNokRes(res, err);
        return;
    }

    res->set_allocated_nic_stats(new MessageV0_NicStats);
    auto nic_stats = res->mutable_nic_stats();

    nic_stats->set_in(in);
    nic_stats->set_out(out);
    BuildOkRes(res);
}

void Api0::SgAdd(const MessageV0_Request &req, MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("Security Group add");
    auto s = req.sg_add();

    if (!ValidateSg(s)) {
        BuildNokRes(res, "Bad Security Group format");
        return;
    }

    app::Sg sg;
    if (!Convert(s, &sg)) {
        BuildNokRes(res, "Internal error");
        return;
    }

    app::Error error;

    if (!ActionSgAdd(sg, &error))
        BuildNokRes(res, error);
    else
        BuildOkRes(res);
}

void Api0::SgDel(const MessageV0_Request &req, MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("Security group delete");
    std::string sg_id = req.sg_del();

    app::Error error;

    if (!ActionSgDel(sg_id, &error))
        BuildNokRes(res, error);
    else
        BuildOkRes(res);
}

void Api0::SgList(const MessageV0_Request &req, MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("Security Group listing");
    app::Model &m = app::model;
    for (auto it=m.security_groups.begin();
         it != m.security_groups.end(); it++) {
        auto id = res->add_sg_list();
        *id = it->second.id;
    }
    BuildOkRes(res);
}

void Api0::SgRuleAdd(const MessageV0_Request &req,
    MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("Security Group rule add");
    auto rule_req = req.sg_rule_add();
    auto r = rule_req.rule();
    if (!ValidateSgRule(r)) {
        BuildNokRes(res, "Bad Security Group rule format");
        return;
    }

    std::string sg_id = rule_req.sg_id();
    app::Rule rule;
    if (!Convert(r, &rule)) {
        BuildNokRes(res, "Internal error");
        return;
    }

    app::Error error;

    if (!ActionSgRuleAdd(sg_id, rule, &error))
        BuildNokRes(res, error);
    else
        BuildOkRes(res);
}

void Api0::SgRuleDel(const MessageV0_Request &req, MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("Security Group rule deletion");
    auto rule_req = req.sg_rule_del();
    auto r = rule_req.rule();
    if (!ValidateSgRule(r)) {
        BuildNokRes(res, "Bad Security Group rule format");
        return;
    }

    std::string sg_id = rule_req.sg_id();
    app::Rule rule;
    if (!Convert(r, &rule)) {
        BuildNokRes(res, "Internal error");
        return;
    }

    app::Error error;

    if (!ActionSgRuleDel(sg_id, rule, &error))
        BuildNokRes(res, error);
    else
        BuildOkRes(res);
}

void Api0::SgRuleList(const MessageV0_Request &req,
    MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("Security Group rule list");
    std::string sg_id = req.sg_rule_list();
    app::Model &m = app::model;

    if (m.security_groups.find(sg_id) == m.security_groups.end()) {
        BuildNokRes(res, "Security Group not found");
        return;
    }

    app::Sg &sg = m.security_groups[sg_id];

    for (auto it=sg.rules.begin(); it != sg.rules.end(); it++) {
        auto r = res->add_sg_rule_list();
        if (!Convert(it->second, r)) {
            BuildNokRes(res, "Internal error");
            return;
        }
    }

    BuildOkRes(res);
}

void Api0::SgMemberAdd(const MessageV0_Request &req,
    MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("Security Group member add");
    auto member_req = req.sg_member_add();
    auto i = member_req.member();
    if (!ValidateIp(i)) {
        BuildNokRes(res, "Bad IP format");
        return;
    }

    std::string sg_id = member_req.sg_id();
    app::Ip ip;
    if (!Convert(i, &ip)) {
        BuildNokRes(res, "Internal error");
        return;
    }

    app::Error error;

    if (!ActionSgMemberAdd(sg_id, ip, &error))
        BuildNokRes(res, error);
    else
        BuildOkRes(res);
}

void Api0::SgMemberDel(const MessageV0_Request &req,
    MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("Security Group member deletion");
    auto member_req = req.sg_member_del();
    auto i = member_req.member();
    if (!ValidateIp(i)) {
        BuildNokRes(res, "Bad IP format");
        return;
    }

    std::string sg_id = member_req.sg_id();
    app::Ip ip;
    if (!Convert(i, &ip)) {
        BuildNokRes(res, "Internal error");
        return;
    }

    app::Error error;

    if (!ActionSgMemberDel(sg_id, ip, &error))
        BuildNokRes(res, error);
    else
        BuildOkRes(res);
}

void Api0::SgMemberList(const MessageV0_Request &req,
    MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("Security Group member list");
    std::string sg_id = req.sg_member_list();
    app::Model &m = app::model;

    if (m.security_groups.find(sg_id) == m.security_groups.end()) {
        BuildNokRes(res, "Security Group not found");
        return;
    }

    app::Sg &sg = m.security_groups[sg_id];

    for (auto it=sg.members.begin(); it != sg.members.end(); it++) {
        auto m = res->add_sg_member_list();
        if (!Convert(*it, m)) {
            BuildNokRes(res, "Internal error");
            return;
        }
    }
    BuildOkRes(res);
}

void Api0::AppStatus(const MessageV0_Request &req,
    MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("Application status");
    res->set_allocated_app_status(new MessageV0_AppStatusRes);
    auto a = res->mutable_app_status();
    a->set_start_date(app::stats.start_date);
    a->set_current_date(time(NULL));
    a->set_request_counter(app::stats.request_counter);
    a->set_graph_dot(ActionGraphDot());

    BuildOkRes(res);
}

void Api0::AppQuit(const MessageV0_Request &req, MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("Application quit");
    ActionAppQuit();
    BuildOkRes(res);
}

void Api0::AppConfig(const MessageV0_Request &req, MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("Application configuration");
    auto config = req.app_config();
    std::string err;
    if (config.has_log_level() && !app::log.SetLogLevel(config.log_level()))
        err = err + "log level: failed ";

    if (err.length() == 0) {
        BuildOkRes(res);
    } else {
        err = "Application configuration failed: " + err;
        BuildNokRes(res, err);
    }
}

void Api0::SgDetails(const MessageV0_Request &req,
                     MessageV0_Response *res) {
    if (res == nullptr)
        return;
    app::log.Info("Sg details");
    app::Model &m = app::model;
    std::string id = req.sg_details();
    // Details of one Sg only
    if (id.length() > 0) {
        auto it = m.security_groups.find(id);
        if (it == m.security_groups.end()) {
            BuildNokRes(res, "Sg not found");
            return;
        }

        auto n = res->add_sg_details();
        if (!Convert(it->second, n)) {
            BuildNokRes(res, "Internal error");
            return;
        }
        BuildOkRes(res);
        return;
    }
    // Details of all SGs
    for (auto it=m.security_groups.begin();
        it != m.security_groups.end(); it++) {
        auto n = res->add_sg_details();
        if (!Convert(it->second, n)) {
            BuildNokRes(res, "Internal error");
            return;
        }
    }
    BuildOkRes(res);
}

void Api0::BuildOkRes(MessageV0_Response *res) {
    if (res == nullptr)
        return;
    auto s = res->mutable_status();
    s->set_status(true);
}

void Api0::BuildNokRes(MessageV0_Response *res) {
    if (res == nullptr)
        return;
    auto s = res->mutable_status();
    s->set_status(false);
}

void Api0::BuildNokRes(MessageV0_Response *res, std::string description) {
    if (res == nullptr)
        return;
    LOG_DEBUG_("%s", description.c_str());
    BuildNokRes(res);
    auto s = res->mutable_status();
    s->set_allocated_error(new MessageV0_Error);
    auto e = s->mutable_error();
    e->set_description(description);
}

void Api0::BuildNokRes(MessageV0_Response *res, const char *description) {
    std::string d;
    if (description != nullptr)
        d = description;
    BuildNokRes(res, d);
}

void Api0::BuildNokRes(MessageV0_Response *res, const app::Error &error) {
    if (res == nullptr)
        return;
    auto s = res->mutable_status();
    s->set_allocated_error(new MessageV0_Error);
    auto e = s->mutable_error();

    if (!Convert(error, e)) {
        s->release_error();
        BuildNokRes(res, "Internal Error");
    }
    BuildNokRes(res);
}

bool Api0::ValidateNic(const MessageV0_Nic &nic) {
    // Check MAC
    if (!ValidateMac(nic.mac()))
        return false;

    // Check VNI < 2^24
    uint32_t vni = nic.vni();
    if (vni > 16777215)
        return false;

    // Check IP list
    for (int a = 0; a < nic.ip_size(); a++) {
        auto ip = nic.ip(a);
        if (!ValidateIp(ip))
            return false;
    }
    return true;
}

bool Api0::ValidateNicUpdate(const MessageV0_NicUpdateReq &nic_update) {
    // Check IP list
    if (nic_update.ip_size() == 1 && nic_update.ip(0).length() == 0) {
        return true;
    }
    for (int a = 0; a < nic_update.ip_size(); a++) {
        auto ip = nic_update.ip(a);
        if (!ValidateIp(ip))
            return false;
    }
    return true;
}

bool Api0::ValidateSg(const MessageV0_Sg &sg) {
    // Check Members
    for (int a = 0; a < sg.member_size(); a++) {
        auto ip = sg.member(a);
        if (!ValidateIp(ip))
            return false;
    }

    // Check Rules
    for (int a = 0; a < sg.rule_size(); a++) {
        auto rule = sg.rule(a);
        if (!ValidateSgRule(rule))
            return false;
    }
    return true;
}

bool Api0::ValidateSgRule(const MessageV0_Rule &rule) {
    // Check protocol
    int32_t protocol = rule.protocol();
    if (protocol < -1 || protocol > 255) {
        LOG_DEBUG_("protol number out of range");
        return false;
    }

    // Check that we have port_start and port_end if using TCP or UDP
    if (protocol == 6 || protocol == 17) {
        if (!rule.has_port_start() || !rule.has_port_end()) {
            LOG_DEBUG_("missing port informations");
            return false;
        }

        // Check port_start and port_end
        uint32_t port_start = rule.port_start();
        uint32_t port_end = rule.port_end();
        if (port_start > 65535 || port_end > 65535) {
            LOG_DEBUG_("bad port range");
            return false;
        }

        if (port_start > port_end) {
            LOG_DEBUG_("bad port range (start > end)");
            return false;
        }
    }

    // CIDR or security group
    if (rule.has_security_group() && rule.has_cidr()) {
        LOG_DEBUG_("cannot have CIDR and SG specified");
        return false;
    }

    // Validate CIDR
    if (rule.has_cidr()) {
        auto cidr = rule.cidr();
        if (!ValidateCidr(cidr))
            return false;
    }

    return true;
}

bool Api0::ValidateIp(const std::string &ip) {
    struct in6_addr a6;
    struct in_addr a4;
    const char *s = ip.c_str();
    if (inet_pton(AF_INET6, s, &a6) != 1 &&
        inet_pton(AF_INET, s, &a4) != 1)
        return false;
    return true;
}

bool Api0::ValidateMac(const std::string &mac) {
    if (mac.length() != 17)
        return false;
    const char *cstr = mac.c_str();
    unsigned int addr[6];
    if (sscanf(cstr, "%2x:%2x:%2x:%2x:%2x:%2x",
               &addr[0], &addr[1], &addr[2],
               &addr[3], &addr[4], &addr[5]) != 6)
        return false;
    for (int i = 0; i < 6; i++)
        if (addr[i] < 0 || addr[i] > 255)
            return false;
    return true;
}

bool Api0::ValidateCidr(const MessageV0_Cidr &cidr) {
    const char *ip = cidr.address().c_str();
    struct in6_addr a6;
    struct in_addr a4;
    if (inet_pton(AF_INET6, ip, &a6) == 1) {
        if (cidr.mask_size() > 128)
            return false;
    } else if (inet_pton(AF_INET, ip, &a4) == 1) {
        if (cidr.mask_size() > 32)
            return false;
    } else {
        return false;
    }
    return true;
}

bool Api0::Convert(const app::Nic &nic_model, MessageV0_Nic *nic_message) {
    if (nic_message == nullptr)
        return false;
    // Id
    nic_message->set_id(nic_model.id);
    // MAC
    auto mac = nic_message->mutable_mac();
    if (!Convert(nic_model.mac, mac))
        return false;
    // VNI
    nic_message->set_vni(nic_model.vni);
    // IP list of NIC
    for (auto it = nic_model.ip_list.begin();
         it != nic_model.ip_list.end(); it++) {
        auto ip = nic_message->add_ip();
        if (!Convert(*it, ip))
            return false;
    }
    // List of security groups
    for (auto it = nic_model.security_groups.begin();
         it != nic_model.security_groups.end(); it++) {
        auto sg = nic_message->add_security_group();
        *sg = *it;
    }
    // Antispoof IP
    nic_message->set_ip_anti_spoof(nic_model.ip_anti_spoof);
    // Sniff target
    if (nic_model.sniff_target_nic_id.length() > 0)
        nic_message->set_sniff_target_nic_id(nic_model.sniff_target_nic_id);
    // Bypass filterfing
    nic_message->set_bypass_filtering(nic_model.bypass_filtering);
    // Type
    nic_message->set_type(MessageV0_Nic_Type_VHOST_USER_SERVER);
    // Path
    if (nic_model.path.length() > 0)
        nic_message->set_path(nic_model.path);
    return true;
}

bool Api0::Convert(const MessageV0_Nic &nic_message, app::Nic *nic_model) {
    if (nic_model == nullptr)
        return false;
    // Id
    nic_model->id = nic_message.id();
    // MAC
    if (!Convert(nic_message.mac(), &nic_model->mac))
        return false;
    // VNI
    nic_model->vni = nic_message.vni();
    // IP list of NIC
    for (int a = 0; a < nic_message.ip_size(); a++) {
        app::Ip ip;
        if (!Convert(nic_message.ip(a), &ip))
            return false;
        // Don't add IP if we already have it
        auto res = std::find(nic_model->ip_list.begin(),
                             nic_model->ip_list.end(), ip);
        if (res != nic_model->ip_list.end())
            continue;
        nic_model->ip_list.push_back(ip);
    }
    // List of security groups
    for (int a = 0; a < nic_message.security_group_size(); a++) {
        // Don't add security group if we already have it
        std::string sg_id = nic_message.security_group(a);
        auto res = std::find(nic_model->security_groups.begin(),
                             nic_model->security_groups.end(), sg_id);
        if (res != nic_model->security_groups.end())
            continue;
        nic_model->security_groups.push_back(sg_id);
    }
    // Antispoof IP
    nic_model->ip_anti_spoof = false;
    if (nic_message.has_ip_anti_spoof())
        nic_model->ip_anti_spoof = nic_message.ip_anti_spoof();
    // Sniff target
    if (nic_message.has_sniff_target_nic_id())
        nic_model->sniff_target_nic_id = nic_message.sniff_target_nic_id();
    // Bypass filtering
    nic_model->bypass_filtering = false;
    if (nic_message.has_bypass_filtering())
        nic_model->bypass_filtering = nic_message.bypass_filtering();
    // Nic type
    if (nic_message.has_type())
        nic_model->type = static_cast<enum app::NicType>(nic_message.type());
    // Path
    if (nic_message.has_path())
        nic_model->path = nic_message.path();
    return true;
}

bool Api0::Convert(const MessageV0_NicUpdateReq &nic_update_message,
                    Api::NicUpdate *nic_update_model) {
    if (nic_update_model == nullptr)
        return false;
    // Id
    nic_update_model->id = nic_update_message.id();
    // Antispoof IP
    if (nic_update_message.has_ip_anti_spoof()) {
        nic_update_model->ip_anti_spoof = nic_update_message.ip_anti_spoof();
        nic_update_model->has_ip_anti_spoof = true;
    } else {
        nic_update_model->has_ip_anti_spoof = false;
    }
    // IP list of NIC
    nic_update_model->ip_overwrite =
        nic_update_message.ip_size() ? true : false;
    for (int a = 0; a < nic_update_message.ip_size(); a++) {
        app::Ip ip;
        if (nic_update_message.ip(a).length() == 0)
            continue;
        if (!Convert(nic_update_message.ip(a), &ip))
            return false;

        // Don't add IP if we already have it
        auto res = std::find(nic_update_model->ip.begin(),
                             nic_update_model->ip.end(), ip);
        if (res != nic_update_model->ip.end())
            continue;
        nic_update_model->ip.push_back(ip);
    }
    // List Security groups
    nic_update_model->security_groups_overwrite =
        nic_update_message.security_group_size() ? true : false;
    for (int a = 0; a < nic_update_message.security_group_size(); a++) {
        // Don't add security group if we already have it
        std::string sg_id = nic_update_message.security_group(a);
        if (sg_id.length() == 0)
            continue;
        auto res = std::find(nic_update_model->security_groups.begin(),
                             nic_update_model->security_groups.end(), sg_id);
        if (res != nic_update_model->security_groups.end())
            continue;
        nic_update_model->security_groups.push_back(sg_id);
    }
    return true;
}

bool Api0::Convert(const app::Sg &sg_model, MessageV0_Sg *sg_message) {
    if (sg_message == nullptr)
        return false;
    // Id
    sg_message->set_id(sg_model.id);
    // List of members
    for (auto it = sg_model.members.begin();
         it != sg_model.members.end(); it++) {
        auto m = sg_message->add_member();
        if (!Convert(*it, m))
            return false;
    }
    // List of rules
    for (auto it = sg_model.rules.begin();
         it != sg_model.rules.end(); it++) {
        auto r = sg_message->add_rule();
        if (!Convert(it->second, r))
            return false;
    }
    return true;
}

bool Api0::Convert(const MessageV0_Sg &sg_message, app::Sg *sg_model) {
    if (sg_model == nullptr)
        return false;
    // Id
    sg_model->id = sg_message.id();
    // List of members
    for (int a = 0; a < sg_message.member_size(); a++) {
        app::Ip ip;
        if (!Convert(sg_message.member(a), &ip))
            return false;

        // Don't add IP if we already have it
        auto res = std::find(sg_model->members.begin(),
                             sg_model->members.end(), ip);
        if (res != sg_model->members.end())
            continue;

        sg_model->members.push_back(ip);
    }
    // List of rules
    for (int a = 0; a < sg_message.rule_size(); a++) {
        app::Rule rule;
        if (!Convert(sg_message.rule(a), &rule))
            return false;
        std::hash<app::Rule> hf;
        std::size_t h = hf(rule);
        // Don't add rule if already exist
        if (sg_model->rules.find(h) != sg_model->rules.end())
            continue;
        std::pair<std::size_t, app::Rule> p(h, rule);
        sg_model->rules.insert(p);
    }
    return true;
}

bool Api0::Convert(const app::Rule &rule_model,
    MessageV0_Rule *rule_message) {
    if (rule_message == nullptr)
        return false;
    // Direction
    if (rule_model.direction == app::Rule::INBOUND)
        rule_message->set_direction(MessageV0_Rule_Direction_INBOUND);
    else if (rule_model.direction == app::Rule::OUTBOUND)
        rule_message->set_direction(MessageV0_Rule_Direction_OUTBOUND);
    else
        return false;
    // IP protocol
    rule_message->set_protocol(rule_model.protocol);
    // fill up ports for TCP and UDP
    if (rule_model.protocol == 6 || rule_model.protocol == 17) {
        if (rule_model.port_start == - 1 || rule_model.port_end == -1)
            return false;
        rule_message->set_port_start(rule_model.port_start);
        rule_message->set_port_end(rule_model.port_end);
    }
    // Convert CIDR _OR_ security_group options
    if (rule_model.security_group.length() > 0) {
        rule_message->set_security_group(rule_model.security_group);
    } else {
        rule_message->set_allocated_cidr(new MessageV0_Cidr);
        auto cidr = rule_message->mutable_cidr();
        if (!Convert(rule_model.cidr, cidr))
            return false;
    }
    return true;
}

bool Api0::Convert(const MessageV0_Rule &rule_message, app::Rule *rule_model) {
    if (rule_model == nullptr)
        return false;
    // Direction
    auto d = rule_message.direction();
    if (d == MessageV0_Rule_Direction_INBOUND)
        rule_model->direction = app::Rule::INBOUND;
    else if (d == MessageV0_Rule_Direction_OUTBOUND)
        rule_model->direction = app::Rule::OUTBOUND;
    // IP protocol
    rule_model->protocol = rule_message.protocol();
    // Start and End port if set
    if (rule_message.has_port_start())
        rule_model->port_start = rule_message.port_start();
    if (rule_message.has_port_end())
        rule_model->port_end = rule_message.port_end();
    // Convert CIDR _OR_ security_group options
    if (rule_message.has_security_group()) {
        rule_model->security_group = rule_message.security_group();
    } else if (rule_message.has_cidr()) {
        if (!Convert(rule_message.cidr(), &rule_model->cidr))
            return false;
    } else {
        return false;
    }
    return true;
}

bool Api0::Convert(const app::Ip &ip_model, std::string *ip_message) {
    if (ip_message == nullptr)
        return false;
    *ip_message = ip_model.Str();
    return true;
}

bool Api0::Convert(const std::string &ip_message, app::Ip *ip_model) {
    if (ip_model == nullptr)
        return false;
    *ip_model = ip_message;
    return true;
}

bool Api0::Convert(const app::Mac &mac_model, std::string *mac_message) {
    if (mac_message == nullptr)
        return false;
    *mac_message = mac_model.Str();
    return true;
}

bool Api0::Convert(const std::string &mac_message, app::Mac *mac_model) {
    if (mac_model == nullptr)
        return false;
    *mac_model = mac_message;
    return true;
}

bool Api0::Convert(const app::Cidr &cidr_model, MessageV0_Cidr *cidr_message) {
    if (cidr_message == nullptr)
        return false;
    // Address
    auto ip = cidr_message->mutable_address();
    if (!Convert(cidr_model.address, ip))
        return false;
    // Address mask
    cidr_message->set_mask_size(cidr_model.mask_size);
    return true;
}

bool Api0::Convert(const MessageV0_Cidr &cidr_message, app::Cidr *cidr_model) {
    if (cidr_model == nullptr)
        return false;
    // Address
    if (!Convert(cidr_message.address(), &cidr_model->address))
        return false;
    // Address mask
    cidr_model->mask_size = cidr_message.mask_size();
    return true;
}

bool Api0::Convert(const app::Error &error_model,
    MessageV0_Error *error_message) {
    if (error_message == nullptr)
        return false;
    // description
    if (error_model.description.length() > 0)
        error_message->set_description(error_model.description);
    // err_no
    if (error_model.has_err_no)
        error_message->set_err_no(error_model.err_no);
    // file
    if (error_model.file.length() > 0)
        error_message->set_file(error_model.file);
    // line
    if (error_model.has_line)
        error_message->set_line(error_model.line);
    // curs_pos
    if (error_model.has_curs_pos)
        error_message->set_curs_pos(error_model.curs_pos);
    // function
    if (error_model.function.length() > 0)
        error_message->set_function(error_model.function);
    return true;
}

