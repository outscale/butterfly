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
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <streambuf>
#include <memory>
#include "api/common/crypto.h"

namespace Crypto {

#define AES_IV_SIZE 16
#define AES_BLOCK_SIZE 16
#define AES256_KEY_SIZE 32
#define SHA512_SIZE 64

static void unbase64(const std::string &in, std::string *out) {
    BIO *bio64, *mem;
    char *buffer;
    int len;
    /* that so great */
    void *inb = const_cast<void*>(reinterpret_cast<void const *>(in.c_str()));

    buffer = new char[in.length()];
    bio64 = BIO_new(BIO_f_base64());
    BIO_set_flags(bio64, BIO_FLAGS_BASE64_NO_NL);
    mem = BIO_new_mem_buf(inb, in.length());
    mem = BIO_push(bio64, mem);
    len = BIO_read(mem, buffer, in.length());
    out->clear();
    out->insert(0, buffer, len);
    free(buffer);
    BIO_free_all(mem);
}

bool KeyFromPath(const std::string &path, std::string *key) {
    std::ifstream file(path);
    if (!file)
        return false;

    file.seekg(0, std::ios::end);
    std::streampos len = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buf(len);
    file.read(&buf[0], len);
    std::string content(buf.data(), buf.size());

    std::string k;
    unbase64(content, &k);
    if (k.length() != AES256_KEY_SIZE)
        return false;

    *key = k;
    return true;
}

bool EncAes256CbcSha512(const std::string &key,
                        const std::string &clear,
                        std::string *encrypted) {
    if (key.length() != AES256_KEY_SIZE)
        return false;

    bool ret = false;
    int data_size = (clear.length() / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    int result_size = AES_IV_SIZE + SHA512_SIZE + data_size;
    uint8_t pad_length = data_size - clear.length();
    uint8_t *result = new uint8_t[result_size];
    uint8_t *iv = result;
    uint8_t *sha = iv + AES_IV_SIZE;
    uint8_t *data = sha + SHA512_SIZE;
    uint8_t computed_sha[SHA512_SIZE];
    uint8_t last_block[AES_BLOCK_SIZE];
    uint32_t unop = 0;
    int nop = 0;
    const uint8_t *k = reinterpret_cast<const uint8_t *>(key.c_str());
    const uint8_t *clear_ = reinterpret_cast<const uint8_t *>(clear.c_str());
    const uint8_t *clear_last_block = clear_ +
        (clear.length() / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;

    // Build last block content with padding
    memcpy(last_block, clear_last_block, clear.length() % AES_BLOCK_SIZE);
    memset(last_block + AES_BLOCK_SIZE - pad_length, pad_length,
           AES_BLOCK_SIZE);

    // Compute hash of clear data
    EVP_CIPHER_CTX *aes = NULL;
    EVP_MD_CTX *s = EVP_MD_CTX_create();
    EVP_DigestInit_ex(s, EVP_sha512(), NULL);
    EVP_DigestUpdate(s, clear.c_str(), clear.length());
    EVP_DigestFinal_ex(s, computed_sha, &unop);
    EVP_MD_CTX_destroy(s);
    if (unop != SHA512_SIZE)
        return false;

    // Generate IV
    if (!RAND_bytes(iv, AES_IV_SIZE))
        return false;

    // Start AES cipher
    aes = EVP_CIPHER_CTX_new();
    EVP_CipherInit_ex(aes, EVP_aes_256_cbc(), NULL, k, iv, 0);
    EVP_CIPHER_CTX_set_padding(aes, 0);

    // Encrypt SHA
    if (!EVP_CipherUpdate(aes, sha, &nop, computed_sha, SHA512_SIZE) ||
        nop != SHA512_SIZE)
        goto exit;

    // If we have more than one block, encrypt all data except last block
    if (clear.length() >= AES_BLOCK_SIZE &&
        (!EVP_CipherUpdate(aes, data, &nop, clear_, data_size - AES_BLOCK_SIZE)
         || nop != data_size - AES_BLOCK_SIZE))
        goto exit;

    // Encrypt last block
    if (!EVP_CipherUpdate(aes, data + (clear.length() / AES_BLOCK_SIZE) *
                          AES_BLOCK_SIZE,
              &nop, last_block, AES_BLOCK_SIZE) || nop != AES_BLOCK_SIZE)
        goto exit;

    // Finalyze encryption. Everything has been written => NULL
    nop = 0;
    if (!EVP_CipherFinal_ex(aes, NULL, &nop) || nop != 0)
        goto exit;

    // Write result to user's string
    encrypted->reserve(result_size);
    encrypted->insert(0, (const char *)result, result_size);
    ret = true;
exit:
    EVP_CIPHER_CTX_free(aes);
    return ret;
}

bool DecAes256CbcSha512(const std::string &key,
                        const std::string &encrypted,
                        std::string *clear) {
    if (key.length() != AES256_KEY_SIZE ||
        encrypted.length() < AES_IV_SIZE + SHA512_SIZE + AES_BLOCK_SIZE ||
        encrypted.length() % AES_BLOCK_SIZE != 0)
        return false;
    bool ret = false;
    const uint8_t *iv =
        reinterpret_cast<const uint8_t *>(encrypted.c_str());
    const uint8_t *k =
        reinterpret_cast<const uint8_t *>(key.c_str());
    const uint8_t *enc_blocks = iv + AES_IV_SIZE;
    int blocks_len = encrypted.length() - AES_IV_SIZE;
    uint32_t us;
    int s;
    EVP_CIPHER_CTX *aes = NULL;
    EVP_MD_CTX *sha;

    std::unique_ptr<uint8_t> clear_result(new uint8_t[blocks_len]);
    uint8_t *clear_sha = clear_result.get();
    uint8_t *clear_payload = clear_sha + SHA512_SIZE;
    int clear_payload_len;
    uint8_t computed_sha[SHA512_SIZE];
    uint8_t pad_len;

    // Prepare AES cipher with included IV
    aes = EVP_CIPHER_CTX_new();
    EVP_CipherInit_ex(aes, EVP_aes_256_cbc(), NULL, k, iv, 1);
    EVP_CIPHER_CTX_set_padding(aes, 0);

    // Decrypt data
    if (!EVP_CipherUpdate(aes, clear_result.get(), &s, enc_blocks, blocks_len)
        || s != blocks_len)
        goto exit;

    s = 0;
    if (!EVP_CipherFinal_ex(aes, NULL, &s) || s)
        goto exit;

    pad_len = clear_result.get()[blocks_len - 1];

    if (pad_len > AES_BLOCK_SIZE)
        goto exit;

    clear_payload_len = blocks_len - SHA512_SIZE - pad_len;

    // Compute SHA of decrypted data
    sha = EVP_MD_CTX_create();
    EVP_DigestInit_ex(sha, EVP_sha512(), NULL);
    EVP_DigestUpdate(sha, clear_payload, clear_payload_len);
    EVP_DigestFinal_ex(sha, computed_sha, &us);
    if (us != SHA512_SIZE)
        goto exit;
    EVP_MD_CTX_destroy(sha);

    // Check that SHA is correct
    if (memcmp(clear_sha, computed_sha, SHA512_SIZE))
        goto exit;

    // Copy decrypted data
    clear->reserve(clear_payload_len);
    clear->insert(0, (const char *)clear_payload, clear_payload_len);
    ret = true;
exit:
    EVP_CIPHER_CTX_free(aes);
    return ret;
}

#undef AES_IV_SIZE
#undef AES_BLOCK_SIZE
#undef AES256_KEY_SIZE
#undef SHA512_SIZE
}  // namespace Crypto

/* Simple unit test.
 * Build with:
 * g++ -std=c++11 -lcrypto -DUNIT_TESTS -I ../../ crypto.cc -o crypto
 */
#ifdef UNIT_TESTS
static int CheckAes256CbcSha512(const std::string msg) {
    std::string key, enc, dec;
    for (uint8_t i = 0; i < 32; i++) key += i;
    if (!Crypto::EncAes256CbcSha512(key, msg, &enc))
        return 1;

    if (!Crypto::DecAes256CbcSha512(key, enc, &dec))
        return 2;
    if (msg.length() != dec.length())
        return 3;
    return 0;
}

int main() {
    int ret;
    std::string msg;

    if (ret = CheckAes256CbcSha512(msg)) return ret;

    msg = std::string("hello");
    if (ret = CheckAes256CbcSha512(msg)) return ret;

    msg = std::string("one block string.");
    if (ret = CheckAes256CbcSha512(msg)) return ret;


    msg = std::string("long string ..............");
    if (ret = CheckAes256CbcSha512(msg)) return ret;

    msg = std::string("two block stringtwo block string");
    if (ret = CheckAes256CbcSha512(msg)) return ret;

    msg = std::string("long long string ........."
                      ".........................."
                      ".........................."
                      ".........................."
                      ".........................."
                      ".........................."
                      ".........................."
                      ".........................."
                      ".........................."
                      ".........................."
                      ".........................."
                      ".........................."
                      ".........................."
                      ".........................."
                      ".........................."
                      ".........................."
                      ".........................."
                      ".........................."
                      "..........................");
    if (ret = CheckAes256CbcSha512(msg)) return ret;

    unsigned char raw_key[] = {
        0x79, 0x00, 0x64, 0x20, 0x59, 0x14, 0xad, 0x2e, 0x6c, 0x70, 0xf1, 0x0f,
        0x3a, 0x7a, 0x67, 0x66, 0x7d, 0x6b, 0xfc, 0x69, 0xe7, 0x30, 0x7f, 0x5b,
        0xa7, 0xc0, 0x1a, 0x57, 0xc0, 0x69, 0xf3, 0xf6
    };
    msg = "eQBkIFkUrS5scPEPOnpnZn1r/GnnMH9bp8AaV8Bp8/Y=";
    Crypto::unbase64(msg, &msg);
    if (msg.length() != 32)
        return 1;
    for (int i = 0; i < 32; i++)
        if (static_cast<uint8_t>(msg[i]) !=
            static_cast<uint8_t>(raw_key[i]))
            return 1;
    return 0;
}
#endif /* #ifdef UNIT_TESTS */
