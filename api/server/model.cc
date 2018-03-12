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
#include <arpa/inet.h>
}
#include <string.h>
#include <string>
#include <iostream>
#include <functional>
#include "api/server/model.h"

namespace app {

Nic::Nic() {
    vni = 0;
    ip_anti_spoof = false;
    packet_trace = false;
    packet_trace_path = "";
    bypass_filtering = false;
    type = VHOST_USER_SERVER;
}

Error::Error() {
    has_line = false;
    has_curs_pos = false;
    has_err_no = false;
}

Rule::Rule() {
    direction = DIRECTION_NB;
    protocol = 0;
    port_start = 0;
    port_end = 0;
}

bool Cidr::operator== (const Cidr& a) const {
    return (address == a.address &&
            mask_size == a.mask_size);
}

bool Rule::operator== (const Rule& a) const {
    return (direction == a.direction &&
            protocol == a.protocol &&
            port_start == a.port_start &&
            port_end == a.port_end &&
            cidr == a.cidr);
}

Sg Sg::operator= (const Sg& a) {
    id = a.id;
    members = a.members;
    rules = a.rules;
    return *this;
}

bool Sg::operator== (const Sg& a) const {
    return (id == a.id &&
            members == a.members &&
            rules == a.rules);
}

Cidr::Cidr() {
    mask_size = 0;
}

std::string Cidr::Str() const {
    return address.Str() + "/" + std::to_string(mask_size);
}

Ip::Ip() {
    type_ = Ip::NONE;
    memset(data_, 0, 16);
}

Ip::Ip(std::string ip_string) {
    Set(ip_string);
}

Ip::type_t Ip::Type() const {
    return type_;
}

std::string Ip::Str() const {
    return ip_;
}

bool Ip::Bytes(uint8_t *data) const {
    if (data == nullptr)
        return false;
    if (type_ == Ip::V4)
        memcpy(reinterpret_cast<void *>(data),
               reinterpret_cast<const void *>(data_), 4);
    else if (type_ == Ip::V6)
        memcpy(reinterpret_cast<void *>(data),
               reinterpret_cast<const void *>(data_), 16);
    else
        return false;
    return true;
}

bool Ip::operator== (const Ip& a) const {
    return memcmp(data_, a.data_, 16) == 0;
}

Ip Ip::operator= (const std::string& a) {
    Set(a);
    return *this;
}

bool Ip::Set(std::string a) {
    struct in6_addr a6;
    struct in_addr a4;
    if (inet_pton(AF_INET, a.c_str(), &a4) == 1) {
        type_ = Ip::V4;
        memcpy(reinterpret_cast<void *>(data_),
               reinterpret_cast<void *>(&a4), 4);
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &a4, str, INET_ADDRSTRLEN);
        ip_ = str;
    } else if (inet_pton(AF_INET6, a.c_str(), &a6) == 1) {
        type_ = Ip::V6;
        memcpy(reinterpret_cast<void *>(data_),
               reinterpret_cast<void *>(a6.s6_addr), 16);
        char str[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &a6, str, INET6_ADDRSTRLEN);
        ip_ = str;
    } else {
        return false;
    }
    return true;
}

Mac::Mac() {
    memset(data_, 0, 6);
}

Mac::Mac(std::string mac_string) {
    Set(mac_string);
}

std::string Mac::Str() const {
    return mac_;
}

bool Mac::Bytes(uint8_t *data) const {
    if (data == nullptr)
        return false;
    memcpy(reinterpret_cast<void *>(data),
           reinterpret_cast<const void *>(data_), 6);
    return true;
}

bool Mac::operator== (const Mac& a) const {
    return memcmp(data_, a.data_, 6) == 0;
}

Mac Mac::operator= (const std::string& a) {
    Set(a);
    return *this;
}

bool Mac::Set(std::string a) {
    if (a.length() != 17)
        return false;
    const char *cstr = a.c_str();
    unsigned int addr[6];
    if (sscanf(cstr, "%2x:%2x:%2x:%2x:%2x:%2x",
               &addr[0], &addr[1], &addr[2],
               &addr[3], &addr[4], &addr[5]) != 6)
        return false;
    for (int i = 0; i < 6; i++) {
        if (addr[i] > 255)
            return false;
        else
            data_[i] = addr[i];
    }
    mac_ = a;
    return true;
}
}  // namespace app

std::hash<app::Ip>::result_type
std::hash<app::Ip>::operator() (argument_type const& a) const {
    return result_type(std::hash<std::string>()(a.Str()));
}

std::hash<app::Mac>::result_type
std::hash<app::Mac>::operator() (argument_type const& a) const {
    return result_type(std::hash<std::string>()(a.Str()));
}

std::hash<app::Rule>::result_type
std::hash<app::Rule>::operator() (argument_type const& a) const {
    std::string rule = std::to_string(a.direction) + "-" +
                       std::to_string(a.protocol) + "-" +
                       std::to_string(a.port_start) + "-" +
                       std::to_string(a.port_end) + "-" +
                       a.cidr.Str() + "-" +
                       a.security_group;
    return result_type(std::hash<std::string>()(rule));
}

std::hash<app::Cidr>::result_type
std::hash<app::Cidr>::operator() (argument_type const& a) const {
    return result_type(std::hash<std::string>()(a.Str()));
}
