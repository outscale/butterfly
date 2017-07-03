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

#include <algorithm>
#include "api/server/api.h"
#include "api/server/app.h"
#include "api/common/crypto.h"

namespace ApiEncrypted {

static void BuildErr(Encrypted *res, Encrypted_Error_Code code) {
    res->set_allocated_error(new Encrypted_Error());
    res->mutable_error()->set_code(code);
}

void Process(const Encrypted &req, Encrypted *res) {
    if (!req.has_aes256cbc_sha512()) {
        LOG_ERROR_("encryption format not supported");
        BuildErr(res, Encrypted_Error::NOT_SUPPORTED);
        return;
    }
    std::string request_string;
    if (!app::config.encryption_key.length()) {
        LOG_ERROR_("encryption key not set");
        BuildErr(res, Encrypted_Error::NOT_SUPPORTED);
        return;
    }
    if (!Crypto::DecAes256CbcSha512(app::config.encryption_key,
                                    req.aes256cbc_sha512(),
                                    &request_string)) {
        LOG_ERROR_("cannot decrypt message");
        BuildErr(res, Encrypted_Error::REJECTED);
        return;
    }

    // Process request
    std::string response_string;
    try {
        Api::ProcessRequest(request_string, &response_string, true);
    } catch (std::exception &e) {
        LOG_ERROR_("internal error: %s", e.what());
        Api::BuildInternalError(&response_string);
    }

    std::string encrypted;
    if (!Crypto::EncAes256CbcSha512(app::config.encryption_key,
                                    response_string,
                                    &encrypted)) {
        LOG_ERROR_("cannot encrypt message");
        BuildErr(res, Encrypted_Error::INTERNAL_ERROR);
        return;
    }
    res->set_aes256cbc_sha512(encrypted);
}
}
