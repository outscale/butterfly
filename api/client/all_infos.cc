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

static inline void build_req_message_0(proto::Messages *req) {
    req->add_messages();
    req->mutable_messages(0)->set_revision(PROTOS_REVISION);
    req->mutable_messages(0)->set_allocated_message_0(new MessageV0);
    req->mutable_messages(0)->mutable_message_0()->
        set_allocated_request(new MessageV0_Request);
}

int all_infos(const Options &options) {
    // Build nic_details request (empty id => all nics)
    proto::Messages req_nics;
    build_req_message_0(&req_nics);
    req_nics.mutable_messages(0)->mutable_message_0()->
        mutable_request()->set_nic_details("");

    proto::Messages res_nics;
    if (request(req_nics, &res_nics, options) != 0)
        return 1;

    MessageV0_Response res_nics_0 = res_nics.messages(0).message_0().response();
    int nic_nb = res_nics_0.nic_details_size();
    std::cout << "# Virtual NICS (" <<
        std::to_string(nic_nb) << ")" << std::endl;
    for (int i = 0; i < nic_nb; i++) {
        MessageV0_Nic nic = res_nics_0.nic_details(i);
        std::cout << "## " << nic.id() << std::endl;
        std::cout << " - mac: " << nic.mac() << std::endl;
        std::cout << " - vni: " << nic.vni() << std::endl;
        for (int j = 0; j < nic.ip_size(); j++) {
            std::cout << " - ip: " << nic.ip(j) << std::endl;
        }
        for (int j = 0; j < nic.security_group_size(); j++) {
            std::cout << " - sg: " << nic.security_group(j) << std::endl;
        }
        if (nic.has_ip_anti_spoof()) {
            std::cout << " - ip antispoof: " <<
                nic.ip_anti_spoof() << std::endl;
        }
        if (nic.has_sniff_target_nic_id()) {
            std::cout << " - sniff_target_nic_id: " <<
                nic.sniff_target_nic_id() << std::endl;
        }
    }

    // Get security group listing
    proto::Messages req_sg;
    build_req_message_0(&req_sg);
    req_sg.mutable_messages(0)->mutable_message_0()->
        mutable_request()->set_sg_list(true);

    proto::Messages res_sg;
    if (request(req_sg, &res_sg, options) != 0)
        return 1;

    MessageV0_Response res_sg_0 = res_sg.messages(0).message_0().response();
    int sg_nb = res_sg_0.sg_list_size();
    std::cout << "# Security Groups (" <<
        std::to_string(sg_nb) << ")" << std::endl;
    for (int i = 0; i < sg_nb; i++) {
        std::string sg_name = res_sg_0.sg_list(i);
        std::cout << "## " << sg_name << std::endl;

        // Ask for security group rules
        proto::Messages req_sg_rules;
        build_req_message_0(&req_sg_rules);
        req_sg_rules.mutable_messages(0)->mutable_message_0()->
            mutable_request()->set_sg_rule_list(sg_name);
        proto::Messages res_sg_rules;
        if (request(req_sg_rules, &res_sg_rules, options) != 0)
            return 1;

        int sg_rules_nb =
            res_sg_rules.messages(0).message_0().response().sg_rule_list_size();
        std::cout << "### Rules (" << sg_rules_nb << ")" << std::endl;
        for (int j = 0; j < sg_rules_nb; j++) {
            MessageV0_Rule r =
                res_sg_rules.messages(0).message_0().response().sg_rule_list(j);
            std::cout << " - direction: " <<
                 MessageV0_Rule_Direction_Name(r.direction()) << std::endl;
            std::cout << " - protocol: " << r.protocol() << std::endl;
            if (r.has_port_start())
                std::cout << " - port start: " << r.port_start() << std::endl;
            if (r.has_port_end())
                std::cout << " - port end: " << r.port_end() << std::endl;
            if (r.has_cidr()) {
                std::cout << " - cidr: " << r.cidr().address() << "/" <<
                    r.cidr().mask_size() << std::endl;
            }
            if (r.has_security_group()) {
                std::cout << " - sg members: " << r.security_group() <<
                    std::endl;
            }
            if (j != sg_rules_nb - 1)
                std::cout << std::endl;
        }

        // Ask for security group members
        proto::Messages req_sg_members;
        build_req_message_0(&req_sg_members);
        req_sg_members.mutable_messages(0)->mutable_message_0()->
            mutable_request()->set_sg_member_list(sg_name);
        proto::Messages res_sg_members;
        if (request(req_sg_members, &res_sg_members, options) != 0)
            return 1;

        int sg_members_nb =
        res_sg_members.messages(0).message_0().response().sg_member_list_size();
        std::cout << "### Members (" << sg_members_nb << ")" << std::endl;
        for (int j = 0; j < sg_members_nb; j++) {
            std::cout << " - " <<
            res_sg_members.messages(0).message_0().response().sg_member_list(j)
                << std::endl;
        }
    }
    return 0;
}
