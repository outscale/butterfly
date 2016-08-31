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

int request(const proto::Messages &req,
            proto::Messages *res,
            const Options &options) {
    // Print protobuf message
    if (options.verbose) {
        std::string human_message;
        google::protobuf::TextFormat::PrintToString(req, &human_message);
        std::cout << "#########################" << std::endl;
        std::cout << "# Message to be sent:" << std::endl;
        std::cout << human_message << std::endl;
        std::cout << "#########################" << std::endl;
    }

    // Convert protobuf to string (data)
    std::string str_request;
    if (!req.SerializeToString(&str_request)) {
        std::cerr << "Error, cannot convert protobuf to string before sending"
        << std::endl;
        return 1;
    }

    // Make ZeroMQ request
    zmqpp::context context;
    zmqpp::socket socket(context, zmqpp::socket_type::request);
    socket.connect(options.endpoint);
    if (!socket.send(str_request)) {
        std::cerr <<  "Error, cannot send message to endpoint" << std::endl;
        return 1;
    }

    // Get ZeroMQ response
    std::string str_response;
    if (!socket.receive(str_response)) {
        std::cerr <<  "Error, cannot receive message from endpoint" <<
        std::endl;
        return 1;
    }

    // Convert response to protobuf
    if (!res->ParseFromString(str_response)) {
        std::cerr << "Error while decoding response" << std::endl;
        return 1;
    }

    // Convert protobuf to human readable string
    std::string human_message;
    google::protobuf::TextFormat::PrintToString(*res, &human_message);

    // Print response
    if (options.verbose) {
        std::cout << "#########################" << std::endl;
        std::cout << "# Received message:" << std::endl;
        std::cout << human_message << std::endl;
        std::cout << "#########################" << std::endl;
    }

    // Write json data to stdout if no output has been defined
    if (options.std_out) {
        std::cout << human_message;
        return 0;
    }

    // Write json data to file
    if (options.output) {
        std::ofstream out(options.output);
        out << human_message;
        out.close();
    }
    return 0;
}

int request_from_human(const Options &options) {
    // Read input data
    std::ifstream input_file(options.input);
    std::stringstream ss;
    ss << input_file.rdbuf();
    std::string input_text = ss.str();

    // Convert input strinf to protobuf
    proto::Messages req;
    if (!google::protobuf::TextFormat::ParseFromString(input_text, &req)) {
        std::cerr <<  "Error while decoding input";
        return 1;
    }

    proto::Messages res;
    return request(req, &res, options);
}
