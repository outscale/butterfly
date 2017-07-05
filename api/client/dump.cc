/* Copyright 2017 Outscale SAS
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

static void SubDumpHelp(void) {
    cout << "usage: butterfly dump [options...]" << endl << endl <<
    "Dump all butterfly configuration (nics and security groups) to "
    "stdout." << endl <<
    "The generated data contains all requests ready to send to an other "
    "Butterfly" << endl <<
    "Example: " << endl <<
    "- butterfly dump > butterfly.dump" << endl <<
    "- butterfly shutdown" << endl <<
    "- start butterfly again" << endl <<
    "- butterfly request butterfly.dump" << endl;
    GlobalParameterHelp();
}

static void HumanPrint(const proto::Messages &req) {
        string human_message;
        google::protobuf::TextFormat::PrintToString(req, &human_message);
        cout << human_message << endl;
}

int SubDump(int argc, char **argv, const GlobalOptions &options) {
    if (argc >= 3 && string(argv[2]) == "help") {
        SubDumpHelp();
        return 0;
    }

    // Get all nics and security groups details
    string req =
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    message_id: \"nics\""
        "    request {"
        "      nic_details: \"\""
        "    }"
        "  }"
        "}"
        "messages {"
        "  revision: " PROTO_REV
        "  message_0 {"
        "    message_id: \"sgs\""
        "    request {"
        "      sg_details: \"\""
        "    }"
        "  }"
        "}";
    proto::Messages res;
    if (Request(req, &res, options, false) || CheckRequestResult(res))
        return 1;

    // Find responses for nics and sg
    MessageV0_Response *res_nics = NULL;
    MessageV0_Response *res_sgs = NULL;
    for (int i = 0; i < res.messages_size(); i++) {
        proto::Message *m = res.mutable_messages(i);
        if (m->has_message_0() && m->message_0().has_message_id()) {
            if (m->message_0().message_id() == "nics")
                res_nics = m->mutable_message_0()->mutable_response();
            if (m->message_0().message_id() == "sgs")
                res_sgs = m->mutable_message_0()->mutable_response();
        }
    }

    // Build and print request corresponding to the dump
    proto::Messages msg;
    if (res_nics && res_nics->nic_details_size() > 0) {
        for (int i = 0; i < res_nics->nic_details_size(); i++) {
            proto::Message *m = msg.add_messages();
            m->set_revision(PROTOS_REVISION);
            MessageV0 *m0 = m->mutable_message_0();
            MessageV0_Request *r0 = m0->mutable_request();
            MessageV0_Nic *n = new MessageV0_Nic(res_nics->nic_details(i));
            r0->set_allocated_nic_add(n);
        }
    }
    if (res_sgs && res_sgs->sg_details_size() > 0) {
        for (int i = 0; i < res_sgs->sg_details_size(); i++) {
            proto::Message *m = msg.add_messages();
            m->set_revision(PROTOS_REVISION);
            MessageV0 *m0 = m->mutable_message_0();
            MessageV0_Request *r0 = m0->mutable_request();
            MessageV0_Sg *sg = new MessageV0_Sg(res_sgs->sg_details(i));
            r0->set_allocated_sg_add(sg);
        }
    }
    HumanPrint(msg);
    return 0;
}
