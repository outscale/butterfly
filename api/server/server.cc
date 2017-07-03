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
#include <unistd.h>
}
#include <thread>
#include "api/server/server.h"
#include "api/server/app.h"
#include "api/server/api.h"

ApiServer::ApiServer(std::string zmq_endpoint, bool *end_trigger) {
    endpoint_ = zmq_endpoint;
    end_ = end_trigger;
    Prepare();
}

void
ApiServer::RunThreaded() {
    std::thread t(StaticLoop, this);
    t.detach();
}

void
ApiServer::Loop() {
    try {
        zmqpp::message request;
        zmqpp::message response;
        if (!socket_->receive(request, true))
            return;
        LOG_DEBUG_("ZMQ received a message");
        Process(request, &response);
        LOG_DEBUG_("ZMQ send");
        if (!socket_->send(response, true)) {
            LOG_ERROR_("failed to send ZMQ message");
            return;
        }
    } catch (std::exception &e) {
        LOG_ERROR_("got exception: %s", e.what());
    }
}

void
ApiServer::Run() {
    while (42) {
        Loop();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (end_ != nullptr && *end_ == true)
            break;
    }
}

void
ApiServer::Prepare() {
    socket_ = std::make_shared < zmqpp::socket >(
        context_,
        zmqpp::socket_type::reply);
    try {
        socket_->bind(endpoint_);
    } catch (std::exception & e) {
        LOG_ERROR_("failed to bind API socket");
        throw;
    }
}

void ApiServer::StaticLoop(ApiServer *me) {
    if (me != NULL)
        me->Run();
}

void
ApiServer::Process(const zmqpp::message &req, zmqpp::message *res) {
    // Extract message from ZMQ
    LOG_DEBUG_("unpack request from a ZMQ message");
    std::string request_string = req.get(0);

    // Process request
    std::string response_string;
    try {
        Api::ProcessRequest(request_string, &response_string, false);
    } catch (std::exception &e) {
        LOG_ERROR_("internal error: %s", e.what());
        Api::BuildInternalError(&response_string);
    }

    // Pack message in ZMQ
    LOG_DEBUG_("pack response in a ZMQmessage");
    *res << response_string;
}
