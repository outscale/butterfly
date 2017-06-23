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

#ifndef API_COMMON_CRYPTO_H_
#define API_COMMON_CRYPTO_H_

#include <string>

/* Encryption facility. */
namespace Crypto {
    /* Ease key loading from a file.
     *
     * File must contain a 32 bytes key encoded in base64.
     * Raw 32 bytes key in written to key pointer.
     */
    bool KeyFromPath(const std::string &path, std::string *key);
    /* Encrypt / Decrypt in AES-256 and check inside integrity using SHA512.
     * Format:
     * - 16B: Initialization Vector (this information does not need to be secret)
     *
     *   All the following data is encrypted using AES-256 in CBC mode:
     *
     * - 64B: SHA-512 of message content (only to check content integrity)
     * - XB: message content
     * - Byte-padding in PKCS7 to reach 128bits block size
     *
     * @key        raw 32 bytes key (obtained using KeyFromPath)
     * @clear      clear text to encrypt
     * @encrypted  pointer to encrypted data in case of success
     * @return     true in case of success, false otherwise
     */
    bool EncAes256CbcSha512(const std::string &key,
                            const std::string &clear, std::string *encrypted);
    /*
     * @key        raw 32 bytes key (obtained using KeyFromPath)
     * @clear      encrypted text to decrypt
     * @encrypted  pointer to clear data in case of success
     * @return     true in case of success, false otherwise
     */
    bool DecAes256CbcSha512(const std::string &key,
                             const std::string &encrypted, std::string *clear);
}  // namespace Crypto
#endif  // API_COMMON_CRYPTO_H_
