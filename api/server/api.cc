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

#include <google/protobuf/text_format.h>
#include <iostream>
#include <functional>
#include <string>
#include <utility>
#include <algorithm>
#include <map>
#include "api/server/api.h"
#include "api/server/app.h"
#include "api/server/model.h"
#include "api/protocol/message.pb.h"
#include "api/version.h"

void Api::ProcessRequest(const std::string &request, std::string *response,
                         bool sub) {
    if (response == nullptr)
        return;
    // Increment request counter
    app::stats.request_counter++;

    // Request printer
    std::string human_message;
    google::protobuf::TextFormat::Printer printer;

    // Decode request
    proto::Messages reqs;
    if (!reqs.ParseFromString(request) || !reqs.IsInitialized()) {
        LOG_ERROR_("error while decoding request");
        proto::Message r;
        r.set_revision(PROTOS_REVISION);
        r.set_allocated_error(new proto::Error);
        r.mutable_error()->set_code(proto::Error_Code_PARSE_ERROR);
        r.SerializeToString(response);
        printer.PrintToString(r, &human_message);
        app::log.Debug(human_message);
        return;
    }

    // Log request
    printer.PrintToString(reqs, &human_message);
    app::log.Debug(human_message);

    proto::Messages reps;

    if (app::config.encryption_key.length() > 0) {
        if (reqs.has_encrypted() && sub) {
            LOG_WARNING_("encryption inside encryption, reject");
            proto::Message *rep = reps.add_messages();
            rep->set_revision(PROTOS_REVISION);
            rep->set_allocated_error(new proto::Error);
            rep->mutable_error()->set_code(proto::Error_Code_REJECTED);
        } else if (reqs.has_encrypted() && !sub) {
            app::log.Debug("encrypted message(s) received");
            reps.set_allocated_encrypted(new Encrypted);
            auto req_enc = reqs.encrypted();
            auto rep_enc = reps.mutable_encrypted();
            ApiEncrypted::Process(req_enc, rep_enc);
        } else if (!reqs.has_encrypted() && sub) {
            app::log.Debug("dispatch decrypted messages");
            for (int i = 0; i < reqs.messages_size(); i++) {
                auto rep = reps.add_messages();
                auto req = reqs.messages(i);
                Dispatch(req, rep);
            }
         } else {  // !reqs.has_encrypted() && !sub
            LOG_WARNING_("unencrypted message received, reject");
            proto::Message *rep = reps.add_messages();
            rep->set_revision(PROTOS_REVISION);
            rep->set_allocated_error(new proto::Error);
            rep->mutable_error()->set_code(proto::Error_Code_REJECTED);
        }
    } else if (reqs.messages_size() > 0) {
        app::log.Debug("dispatch clear messages");
        for (int i = 0; i < reqs.messages_size(); i++) {
            auto rep = reps.add_messages();
            auto req = reqs.messages(i);
            Dispatch(req, rep);
        }
    } else if (reqs.has_encrypted()) {
        LOG_WARNING_("received encrypted message, cannot decrypt");
        proto::Message *rep = reps.add_messages();
        rep->set_revision(PROTOS_REVISION);
        rep->set_allocated_error(new proto::Error);
        rep->mutable_error()->set_code(proto::Error_Code_REJECTED);
    } else {
        LOG_WARNING_("received empty message ?");
        proto::Message *rep = reps.add_messages();
        rep->set_revision(PROTOS_REVISION);
        rep->set_allocated_error(new proto::Error);
        rep->mutable_error()->set_code(proto::Error_Code_REJECTED);
    }

    reps.SerializeToString(response);

    // Log response
    printer.PrintToString(reps, &human_message);
    app::log.Debug(human_message);
}

void Api::BuildInternalError(std::string *response) {
    proto::Message rep;
    google::protobuf::TextFormat::Printer printer;
    std::string human_message;

    rep.set_revision(PROTOS_REVISION);
    rep.set_allocated_error(new proto::Error);
    rep.mutable_error()->set_code(proto::Error_Code_INTERNAL_ERROR);
    rep.SerializeToString(response);
    printer.PrintToString(rep, &human_message);
    app::log.Debug(human_message);
}

void Api::Dispatch(const proto::Message &req, proto::Message *rep) {
    if (rep == nullptr)
        return;
    // Put the current revision number of the protocol in response
    rep->set_revision(PROTOS_REVISION);

    if (req.has_message_0()) {
        app::log.Debug("dispatch to MessageV0");
        rep->set_allocated_message_0(new MessageV0);
        auto req_0 = req.message_0();
        auto rep_0 = rep->mutable_message_0();
        Api0::Process(req_0, rep_0);
    } else {
        LOG_ERROR_("message version not supported");
        rep->set_allocated_error(new proto::Error);
        rep->mutable_error()->set_code(proto::Error_Code_VERSION_NOT_MANAGED);
    }
}

bool Api::ActionNicAdd(app::Nic *nic, app::Error *error) {
    auto it = app::model.nics.find(nic->id);
    // Do we already have this NIC ?
    if (it != app::model.nics.end()) {
        std::string m = "NIC already exists with id " + nic->id;
        app::log.Warning(m);
        // Disable NIC in packetgraph
        app::graph.NicDel(it->second);
        // Remove NIC from model
        app::model.nics.erase(nic->id);
        // Retry !
        return Api::ActionNicAdd(nic, error);
    }

    // Create Nic in graph
    if (!app::graph.NicAdd(nic)) {
        app::log.Error("NIC creation failed");
        return false;
    }

    // Add NIC in model
    std::pair<std::string, app::Nic> p(nic->id, *nic);
    app::model.nics.insert(p);
    return true;
}

bool Api::ActionNicUpdate(const Api::NicUpdate &update,
    app::Error *error) {
    // Do we have this NIC ?
    auto itn = app::model.nics.find(update.id);
    if (itn == app::model.nics.end()) {
        std::string m = "NIC does not exist with id " + update.id;
        app::log.Error(m);
        if (error != nullptr)
            error->description = m;
        return false;
    }
    // Get the nic
    app::Nic &n = itn->second;

    bool need_fw_update = false;
    bool need_anti_spoof_update = false;

    // Update IP if needed
    if (update.ip_overwrite && n.ip_list != update.ip) {
        n.ip_list = update.ip;
        need_fw_update = true;
        need_anti_spoof_update = true;
    }

    // Update security groups if needed
    if (update.security_groups_overwrite &&
        n.security_groups != update.security_groups) {
        n.security_groups = update.security_groups;
        need_fw_update = true;
    }

    // Update antispoof if needed
    if (update.has_ip_anti_spoof &&
        update.ip_anti_spoof != n.ip_anti_spoof) {
        n.ip_anti_spoof = update.ip_anti_spoof;
        need_anti_spoof_update = true;
    }

    if (need_anti_spoof_update) {
        app::graph.NicConfigAntiSpoof(n, n.ip_anti_spoof);
    }

    if (need_fw_update)
        app::graph.FwUpdate(n);

    return true;
}

bool Api::ActionNicDel(std::string id, app::Error *error) {
    auto nic = app::model.nics.find(id);
    // Do we have this NIC ?
    if (nic == app::model.nics.end()) {
        std::string m = "NIC does not exist with this id " + id;
        app::log.Warning(m);
        return true;
    }

    app::graph.NicDel(nic->second);

    // Remove NIC from model
    app::model.nics.erase(id);

    return true;
}


bool Api::ActionNicExport(std::string id, std::string *data,
    app::Error *error) {
    if (data == nullptr)
        return false;
    // Do we have this NIC ?
    auto nic = app::model.nics.find(id);
    if (nic == app::model.nics.end()) {
        std::string m = "NIC does not exist with id " + id;
        app::log.Error(m);
        if (error != nullptr)
            error->description = m;
        return false;
    }

    *data = app::graph.NicExport(nic->second);

    return true;
}

bool Api::ActionNicStats(std::string id, uint64_t *in, uint64_t *out,
    app::Error *error) {
    if (in == nullptr || out == nullptr)
        return false;

    auto nic = app::model.nics.find(id);
    if (nic == app::model.nics.end()) {
        std::string m = "NIC does not exist with id " + id;
        app::log.Error(m);
        if (error != nullptr)
            error->description = m;
        return false;
    }

    app::graph.NicGetStats(nic->second, in, out);
    return true;
}

void Api::SgUpdate(const app::Sg &sg) {
    std::map<std::string, app::Nic>::iterator it;
    std::vector<std::string>::iterator sg_it;
    auto found = false;
    // Iterate for each NICs and check if we need to update the firewall.
    for (it = app::model.nics.begin(); it != app::model.nics.end(); it++) {
        app::Nic &nic = it->second;
        for (sg_it = nic.security_groups.begin();
            sg_it != nic.security_groups.end();
            sg_it++) {
            if (*sg_it == sg.id) {
                app::graph.FwUpdate(nic);
                found = true;
            }
        }
    }
    if (!found) {
        std::string m = "security group " + sg.id +
            " update didn't updated any NIC";
        app::log.Warning(m);
    }
}

void Api::SgUpdate(const app::Sg &sg, const app::Rule &rule) {
    std::map<std::string, app::Nic>::iterator it;
    std::vector<std::string>::iterator sg_it;
    auto found = false;
    // Iterate for each NICs and check if we need to update the firewall.
    for (it = app::model.nics.begin(); it != app::model.nics.end(); it++) {
        app::Nic &nic = it->second;
        for (sg_it = nic.security_groups.begin();
             sg_it != nic.security_groups.end();
             sg_it++) {
            if (*sg_it == sg.id) {
                app::graph.FwAddRule(nic, rule);
                found = true;
            }
        }
    }
    if (!found) {
        std::string m = "security group " + sg.id +
            " update didn't add a rule in any NIC";
        app::log.Warning(m);
    }
}

void Api::SgUpdateRuleMembers(const app::Sg &modified_sg) {
    std::map<std::string, app::Nic>::iterator it;
    std::map<std::string, app::Nic>::iterator it_end;
    std::vector<std::string>::iterator sg_it;
    std::map<std::size_t, app::Rule>::iterator rule_it;
    std::map<std::size_t, app::Rule>::iterator sg_end;
    std::vector<std::string> updated_sgs;
    std::map<std::string, app::Sg>::iterator sg;
    bool found = false;

    for (it_end = app::model.nics.end(),
         it = app::model.nics.begin(); it != it_end; it++) {
        app::Nic &nic = it->second;
        for (sg_it = nic.security_groups.begin();
             sg_it != nic.security_groups.end();
             sg_it++) {
            sg = app::model.security_groups.find(*sg_it);
            if (sg == app::model.security_groups.end())
                continue;

            for (sg_end = sg->second.rules.end(),
                 rule_it = sg->second.rules.begin();
                 rule_it != sg_end;
                 rule_it++) {
                if (rule_it->second.security_group == modified_sg.id &&
                    std::find(updated_sgs.begin(),
                              updated_sgs.end(),
                              sg->second.id) == updated_sgs.end()) {
                    SgUpdate(sg->second);
                    updated_sgs.push_back(sg->second.id);
                    found = true;
                    break;
                }
            }
        }
    }
    if (!found) {
        std::string m = "modification of security group " + modified_sg.id +
            " didn't update any other security group";
        app::log.Debug(m);
    }
}

bool Api::ActionSgAdd(const app::Sg &sg, app::Error *error) {
    auto itn = app::model.security_groups.find(sg.id);

    // Do we already have this security group ?
    if (itn != app::model.security_groups.end()) {
        std::string m = "Security group already exists with this id " + sg.id;
        app::log.Warning(m);
        // Check if the securiy group content is the same.
        app::Sg &original_sg = itn->second;
        if (sg == original_sg)
            return true;
        // If not, let's update model
        original_sg = sg;
    } else {
        std::pair<std::string, app::Sg> p(sg.id, sg);
        app::model.security_groups.insert(p);
    }

    SgUpdate(sg);
    SgUpdateRuleMembers(sg);
    return true;
}

bool Api::ActionSgDel(std::string id, app::Error *error) {
    auto m = app::model.security_groups.find(id);
    if (m == app::model.security_groups.end()) {
        std::string m = "Security group does not exist with this id " + id;
        app::log.Warning(m);
        return true;
    }
    // Save security group before removing it
    app::Sg sg = m->second;
    app::model.security_groups.erase(id);
    // Update graph
    SgUpdate(sg);
    SgUpdateRuleMembers(sg);
    return true;
}

bool Api::ActionSgRuleAdd(std::string sg_id, const app::Rule &rule,
    app::Error *error) {
    // Do we have this security group ?
    auto m = app::model.security_groups.find(sg_id);
    if (m == app::model.security_groups.end()) {
        std::string m = "Security group does not exist with this id " + sg_id;
        app::log.Warning(m);
        // Create missing security group
        app::Sg nsg;
        nsg.id = sg_id;
        std::pair<std::string, app::Sg> p(sg_id, nsg);
        app::model.security_groups.insert(p);
        // Try again
        return Api::ActionSgRuleAdd(sg_id, rule, error);
    }

    app::Sg &sg = m->second;

    // Compute rule's hash
    std::hash<app::Rule> hf;
    std::size_t h = hf(rule);

    // Does rule already exist in security group ?
    if (sg.rules.find(h) != sg.rules.end()) {
        std::string m = "Rule already exist in security group " + sg_id;
        app::log.Warning(m);
        return true;
    }

    // Add rule to security group
    std::pair<std::size_t, app::Rule> p(h, rule);
    sg.rules.insert(p);

    // Update graph
    SgUpdate(sg, rule);
    return true;
}

bool Api::ActionSgRuleDel(std::string sg_id, const app::Rule &rule,
    app::Error *error) {
    // Do we have this security group ?
    auto m = app::model.security_groups.find(sg_id);
    if (m == app::model.security_groups.end()) {
        std::string m = "Can't delete rule from a non-existing security " \
            "group " + sg_id;
        app::log.Warning(m);
        return true;
    }

    app::Sg &sg = m->second;

    // Compute rule's hash
    std::hash<app::Rule> hf;
    std::size_t h = hf(rule);

    // Does rule exist in security group ?
    if (sg.rules.find(h) == sg.rules.end()) {
        std::string m = "Can't delete non-existing rule from security " \
           "group " + sg_id;
        app::log.Warning(m);
        return true;
    }

    // Remove rule from security group
    sg.rules.erase(h);

    // Update graph
    SgUpdate(sg);
    return true;
}

bool Api::ActionSgMemberAdd(std::string sg_id, const app::Ip &ip,
    app::Error *error) {
    // Do we have this security group ?
    auto m = app::model.security_groups.find(sg_id);
    if (m == app::model.security_groups.end()) {
        app::log.Warning("Security group does not exist with this id",
                         sg_id.c_str());
        // Create missing security group
        app::Sg nsg;
        nsg.id = sg_id;
        std::pair<std::string, app::Sg> p(sg_id, nsg);
        app::model.security_groups.insert(p);
        m = app::model.security_groups.find(sg_id);
        if (m == app::model.security_groups.end()) {
            LOG_ERROR_("Really cannot create security group with id %s",
                       sg_id.c_str());
            return false;
        }
    }

    app::Sg &sg = m->second;

    // Does member already exist in security group ?
    auto res = std::find(sg.members.begin(), sg.members.end(), ip);
    if (res != sg.members.end()) {
        app::log.Warning("member %s already exist in security group %s",
                         res->Str().c_str(), sg_id.c_str());
        return true;
    }

    // Add member to security group
    sg.members.push_back(ip);

    // Update graph
    SgUpdateRuleMembers(sg);
    return true;
}

bool Api::ActionSgMemberDel(std::string sg_id, const app::Ip &ip,
    app::Error *error) {
    // Do we have this security group ?
    auto m = app::model.security_groups.find(sg_id);
    if (m == app::model.security_groups.end()) {
        app::log.Warning("Can't delete member from a non-existing security " \
            "group %s", sg_id.c_str());
        return true;
    }

    app::Sg &sg = m->second;

     // Does member exist in security group ?
    auto res = std::find(sg.members.begin(), sg.members.end(), ip);
    if (res == sg.members.end()) {
        app::log.Warning("Can't delete non-existing member from security " \
            "group ", sg_id.c_str());
        return true;
    }

    // Delete member from security group
    sg.members.erase(res);

    // Update graph
    SgUpdateRuleMembers(sg);
    return true;
}

std::string Api::ActionGraphDot() {
    return app::graph.Dot();
}

void Api::ActionAppQuit() {
    app::request_exit = true;
}

