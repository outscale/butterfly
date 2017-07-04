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

#ifndef API_SERVER_API_H_
#define API_SERVER_API_H_

#include <google/protobuf/text_format.h>
#include <google/protobuf/stubs/common.h>
#include <string>
#include <vector>
#include "api/protocol/message.pb.h"
#include "api/server/model.h"
#include "api/server/app.h"

class Api {
 public:
    /* Process a request from a brut message
     * This will unpack the message and dispatch it to the right API version
     * @param  request request data from the client
     * @param  response response in which the function will write the response
                        to send to the client
     * @param  sub indicate if it is an encapsulated (encrypted) request or not
     */
    static void ProcessRequest(const std::string &request,
        std::string *response, bool sub);
    /* Build a standard internal error.
     * @param  response response containing internal error
     */
    static void BuildInternalError(std::string *response);
    // This structure centralize description of NicUpdate informations
    struct NicUpdate {
        std::string id;
        bool has_ip_anti_spoof;
        bool ip_anti_spoof;
        std::vector<app::Ip> ip;
        bool ip_overwrite;
        std::vector<std::string> security_groups;
        bool security_groups_overwrite;
    };

 protected:
    /* Dispatch a single message processing depending of message version
     * @param req request message
     * @param res response message
     */
    static void Dispatch(const proto::Message &req, proto::Message *rep);
    /* Create a NIC and add it in model
     * This method centralize NIC creation for all API versions
     * @param  nic the NIC to create, the object may completed during it's
     *             creation.
     * @param  error provide an app::Error object to fill in case of error
     *               can be NULL to ommit it.
     * @return  true if NIC has been created, false otherwise
     */
    static bool ActionNicAdd(app::Nic *nic, app::Error *error);
    /* Update a NIC by replacing provided NIC parameters
     * This method centralize NIC update for all API versions
     * @param  id NIC id to update
     * @param  error provide an app::Error object to fill in case of error
     *               can be NULL to ommit it.
     * @return  true if NIC has been updated, false otherwise
     */
    static bool ActionNicUpdate(const NicUpdate &update,
        app::Error *error);
    /* Delete a NIC and remove it from model
     * This method centralize NIC deletion for all API versions
     * @param  id NIC id to delete
     * @param  error provide an app::Error object to fill in case of error
     *               can be NULL to ommit it.
     * @return  true if NIC has been created, false otherwise
     */
    static bool ActionNicDel(std::string id, app::Error *error);
    /* Grab data from a NIC for NIC export
     * This method centralize NIC export for all API versions
     * @param  id NIC id to get data from
     * @param  data where to put NIC data
     * @param  error provide an app::Error object to fill in case of error
     *               can be NULL to ommit it.
     * @return  true if data has been well filled
     */
    static bool ActionNicExport(std::string id, std::string *data,
        app::Error *error);
    /* Grab NIC statistics
     * This method centralize NIC statistic collection for all API versions
     * @param  id NIC id to get statistics from
     * @param  in data comming in the NIC
     * @param  out data comming out of the NIC
     * @param  error provide an app::Error object to fill in case of error
     *               can be NULL to ommit it.
     * @return  true if data has been well filled
     */
    static bool ActionNicStats(std::string id, uint64_t *in, uint64_t *out,
        app::Error *error);
    /* Creation a Security Group and add it to the model
     * This method centralize SG creation or replace for all API versions
     * @param  sg security group to create or replace
     * @param  error provide an app::Error object to fill in case of error
     *               can be NULL to ommit it.
     * @return  true if SG has been created or replaced, false otherwise
     */
    static bool ActionSgAdd(const app::Sg &sg, app::Error *error);
    /* Delete a security group and remove it from model
     * This method centralize SG deletion for all API versions
     * @param  id security group id to delete
     * @param  error provide an app::Error object to fill in case of error
     *               can be NULL to ommit it.
     * @return  true if SG has been delete, false otherwise
     */
    static bool ActionSgDel(std::string id, app::Error *error);
    /* Add a rule to a Security Group and add it in the model
     * This method centralize app::Rule creation for all API versions
     * @param  sg_id security group id in which we add a rule
     * @param  rule rule to add in the security group
     * @param  sg security group to create or replace
     * @param  error provide an app::Error object to fill in case of error
     *               can be NULL to ommit it.
     * @return  true if rule has been added, false otherwise
     */
    static bool ActionSgRuleAdd(std::string sg_id, const app::Rule &rule,
        app::Error *error);
    /* Delete a rule from a Security Group and update the model
     * This method centralize app::Rule deletion for all API versions
     * @param  sg_id security group id in which we delete a rule
     * @param  rule rule to delete in the security group
     * @param  sg security group to create or replace
     * @param  error provide an app::Error object to fill in case of error
     *               can be NULL to ommit it.
     * @return  true if rule has been deleted, false otherwise
     */
    static bool ActionSgRuleDel(std::string sg_id, const app::Rule &rule,
        app::Error *error);
    /* Add a member to a Security Group and add it in the model
     * This method centralize Member creation for all API versions
     * @param  sg_id security group id in which we add a member
     * @param  ip member to add in the security group
     * @param  sg security group to create or replace
     * @param  error provide an app::Error object to fill in case of error
     *               can be NULL to ommit it.
     * @return  true if member has been added, false otherwise
     */
    static bool ActionSgMemberAdd(std::string sg_id, const app::Ip &ip,
        app::Error *error);
    /* Delete a member from a Security Group and update the model
     * This method centralize Member deletion for all API versions
     * @param  sg_id security group id in which we delete a member
     * @param  ip member to delete in the security group
     * @param  sg security group to create or replace
     * @param  error provide an app::Error object to fill in case of error
     *               can be NULL to ommit it.
     * @return  true if member has been deleted, false otherwise
     */
    static bool ActionSgMemberDel(std::string sg_id, const app::Ip &ip,
        app::Error *error);
    /* Build a dot graph representing all connected bricks
     * This method centralize dot representation building
     * @return  string representing the graphic in DOT language
     */
    static std::string ActionGraphDot();
    /* Shutdown the program
     * This method centralize program shutdown for all API versions
     */
    static void ActionAppQuit();

 private:
    /* Find all NICs who are concerned by a specific SG and update the
     * corresponding firewall.
     * @param  sg security group to update in graph
     */
    static void SgUpdate(const app::Sg &sg);
    /* Find all NICs who are concerned by a specific SG and only add a
     * single rule in the corresponding firewall.
     * @param  sg security group to update in graph
     * @param  rule rule to add in security group
     */
    static void SgUpdate(const app::Sg &sg, const app::Rule &rule);
    /* When a security group change it's members, it may impact other
     * security groups containing a rule allowing members of the modified
     * security group. This function perform call SgUpdate, looks for
     * security groups which has been impacted by the modified security
     * group and update corresponding NICs.
     * @param  sg modified security group
     */
    static void SgUpdateRuleMembers(const app::Sg &sg);
};

class Api0: public Api {
 public:
    /* Process a MessageV0 */
    static void Process(const MessageV0 &req, MessageV0 *res);

 private:
    /* Methods below construct message from request to response */
    static void NicAdd(const MessageV0_Request &req, MessageV0_Response *res);
    static void NicUpdate(const MessageV0_Request &req,
                           MessageV0_Response *res);
    static void NicDel(const MessageV0_Request &req, MessageV0_Response *res);
    static void NicList(const MessageV0_Request &req,
                         MessageV0_Response *res);
    static void NicDetails(const MessageV0_Request &req,
                            MessageV0_Response *res);
    static void NicExport(const MessageV0_Request &req,
                           MessageV0_Response *res);
    static void NicStats(const MessageV0_Request &req,
                          MessageV0_Response *res);
    static void SgAdd(const MessageV0_Request &req, MessageV0_Response *res);
    static void SgDel(const MessageV0_Request &req, MessageV0_Response *res);
    static void SgList(const MessageV0_Request &req, MessageV0_Response *res);
    static void SgRuleAdd(const MessageV0_Request &req,
                            MessageV0_Response *res);
    static void SgRuleDel(const MessageV0_Request &req,
                            MessageV0_Response *res);
    static void SgRuleList(const MessageV0_Request &req,
                             MessageV0_Response *res);
    static void SgMemberAdd(const MessageV0_Request &req,
                              MessageV0_Response *res);
    static void SgMemberDel(const MessageV0_Request &req,
                              MessageV0_Response *res);
    static void SgMemberList(const MessageV0_Request &req,
                               MessageV0_Response *res);
    static void AppStatus(const MessageV0_Request &req,
                           MessageV0_Response *res);
    static void AppQuit(const MessageV0_Request &req,
                         MessageV0_Response *res);
    static void AppConfig(const MessageV0_Request &req,
                           MessageV0_Response *res);
    static void SgDetails(const MessageV0_Request &req,
                          MessageV0_Response *res);
    /* Methods below pre-format some standard response */
    inline static void BuildOkRes(MessageV0_Response *res);
    inline static void BuildNokRes(MessageV0_Response *res);
    inline static void BuildNokRes(MessageV0_Response *res,
                                     std::string description);
    inline static void BuildNokRes(MessageV0_Response *res,
                                     const char *description);
    inline static void BuildNokRes(MessageV0_Response *res,
                                     const app::Error &error);
    /* Methods below permits to validate that the content of a Message is
     * is well formated as expected.
     */
    static bool ValidateNic(const MessageV0_Nic &nic);
    static bool ValidateNicUpdate(const MessageV0_NicUpdateReq &nic_update);
    static bool ValidateSg(const MessageV0_Sg &sg);
    static bool ValidateSgRule(const MessageV0_Rule &rule);
    static bool ValidateIp(const std::string &ip);
    static bool ValidateMac(const std::string &mac);
    static bool ValidateCidr(const MessageV0_Cidr &cidr);
    /* Methods to convert from this message version to object model
     * and reciprocally.
     */
    static bool Convert(const app::Nic &nic_model, MessageV0_Nic *nic_message);
    static bool Convert(const MessageV0_Nic &nic_message, app::Nic *nic_model);
    static bool Convert(const MessageV0_NicUpdateReq &nic_update_message,
                        Api::NicUpdate *nic_update_model);
    static bool Convert(const app::Sg &sg_model, MessageV0_Sg *sg_message);
    static bool Convert(const MessageV0_Sg &sg_message, app::Sg *sg_model);
    static bool Convert(const app::Rule &rule_model,
                        MessageV0_Rule *rule_message);
    static bool Convert(const MessageV0_Rule &rule_message,
                        app::Rule *rule_model);
    static bool Convert(const app::Ip &ip_model, std::string *ip_message);
    static bool Convert(const std::string &ip_message, app::Ip *ip_model);
    static bool Convert(const app::Mac &mac_model, std::string *mac_message);
    static bool Convert(const std::string &mac_message, app::Mac *mac_model);
    static bool Convert(const app::Cidr &cidr_model,
                        MessageV0_Cidr *cidr_message);
    static bool Convert(const MessageV0_Cidr &cidr_message,
                        app::Cidr *cidr_model);
    static bool Convert(const app::Error &error_model,
                        MessageV0_Error *error_message);
};

namespace ApiEncrypted {
    void Process(const Encrypted &req, Encrypted *res);
}

#endif  // API_SERVER_API_H_
