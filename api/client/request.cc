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

RequestOptions::RequestOptions() {
    to_stdout = false;
}

void RequestOptions::Parse(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--stdout")
            to_stdout = true;
    }
}

static void Help(void) {
    cout <<
        "usage: butterfly request REQUEST_FILE [options...]" << endl << endl <<
        "options:" << endl <<
        "    --stdout   prints protobuf response to stdout" << endl;
        GlobalParameterHelp();
}

int SubRequest(int argc, char **argv, const GlobalOptions &options) {
    if (argc <= 2) {
        Help();
        return 1;
    }
    string request_file_path = string(argv[2]);
    RequestOptions r_options;
    r_options.Parse(argc, argv);

    ifstream input_file(request_file_path);
    stringstream ss;
    ss << input_file.rdbuf();
    string input_text = ss.str();

    if (input_text.length() == 0) {
        cerr << "failed to open " << request_file_path << endl;
        return 1;
    }
    proto::Messages res;
    return Request(input_text, &res, options, r_options.to_stdout);
}

int Request(const proto::Messages &req,
            proto::Messages *res,
            const GlobalOptions &options,
            bool response_to_stdout) {
    // Print protobuf message
    if (options.verbose) {
        string human_message;
        google::protobuf::TextFormat::PrintToString(req, &human_message);
        cout << "#########################" << endl;
        cout << "# Message to be sent:" << endl;
        cout << human_message << endl;
        cout << "#########################" << endl;
    }

    // Convert protobuf to string (data)
    string str_request;
    if (!req.SerializeToString(&str_request)) {
        cerr << "Error, cannot convert protobuf to string before sending"
        << endl;
        return 1;
    }

    // Make ZeroMQ request
    zmqpp::context context;
    zmqpp::socket socket(context, zmqpp::socket_type::request);
    socket.connect(options.endpoint);
    if (!socket.send(str_request)) {
        cerr <<  "Error, cannot send message to endpoint" << endl;
        return 1;
    }

    // Get ZeroMQ response
    string str_response;
    if (!socket.receive(str_response)) {
        cerr <<  "Error, cannot receive message from endpoint" <<
        endl;
        return 1;
    }

    // Convert response to protobuf
    if (!res->ParseFromString(str_response)) {
        cerr << "Error while decoding response" << endl;
        return 1;
    }

    // Convert protobuf to human readable string
    string human_message;
    google::protobuf::TextFormat::PrintToString(*res, &human_message);

    // Print response
    if (options.verbose) {
        cout << "#########################" << endl;
        cout << "# Received message:" << endl;
        cout << human_message << endl;
        cout << "#########################" << endl;
    } else if (response_to_stdout) {
        cout << human_message;
    }
    return 0;
}

int Request(const string &req,
            proto::Messages *res,
            const GlobalOptions &options,
            bool response_to_stdout) {
    // Convert input string to protobuf
    proto::Messages proto_req;
    if (!google::protobuf::TextFormat::ParseFromString(req, &proto_req)) {
        cerr <<  "Error while encoding input to protobuf" << endl;
        return 1;
    }
    return Request(proto_req, res, options, response_to_stdout);
}

static void PrintError(MessageV0_Error error) {
    if (error.has_description())
        cerr << endl << "description: " << error.description() << endl;
    else if (error.has_err_no())
        cerr << endl << "errno: " << to_string(error.err_no()) << endl;
    else if (error.has_file())
        cerr << endl << "file: " << error.file() << endl;
    else if (error.has_line())
        cerr << endl << "errno: " << to_string(error.line()) << endl;
    else if (error.has_curs_pos())
        cerr << endl << "curs_pos: " << to_string(error.curs_pos()) << endl;
    else if (error.has_function())
        cerr << endl << "function: " << error.function() << endl;
    else
        cerr << " no details provided" << endl;
}

int CheckRequestResult(const proto::Messages &res) {
    if (res.messages_size() == 0) {
        cerr << "error: no message in response" << endl;
        return 1;
    }

    if (res.messages(0).has_error()) {
        cerr << "error in response: ";
        if (!res.messages(0).error().has_code())
            cerr << "no error code";
        else
            cerr << Error_Code_Name(res.messages(0).error().code());
        cerr << endl;
        return 1;
    }

    if (res.messages(0).has_message_0()) {
        MessageV0 res_0 = res.messages(0).message_0();
        if (!res_0.has_response()) {
            cerr << "error: no response in MessageV0" << endl;
            return 1;
        } else if (!res_0.response().status().status()) {
            cerr << "error in response";
            if (res_0.response().status().has_error()) {
                PrintError(res_0.response().status().error());
            }
             return 1;
        }
    } else {
        cerr << "error: no message recognized in response" << endl;
        return 1;
    }
    return 0;
}
