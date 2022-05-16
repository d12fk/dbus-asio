// This file is part of dbus-asio
// Copyright 2018 Brightsign LLC
// Copyright 2022 OpenVPN Inc. <heiko@openvpn.net>
//
// This library is free software: you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, version 3, or at your
// option any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// The GNU Lesser General Public License version 3 is included in the
// file named COPYING. If you do not have this file see
// <http://www.gnu.org/licenses/>.

#include "dbus_auth.h"


DBus::AuthenticationProtocol::Ptr
DBus::AuthenticationProtocol::create()
{
    struct ShareableAuthProto : public AuthenticationProtocol {
        ShareableAuthProto()
            : AuthenticationProtocol() {};
    };
    return std::make_shared<ShareableAuthProto>();
}

DBus::AuthenticationProtocol::AuthenticationProtocol()
{}

DBus::AuthenticationProtocol::Command
DBus::AuthenticationProtocol::extractCommand(std::string& response)
{
    for (auto& c : m_commands) {
        auto cmdSize = c.first.size();
        if (response.substr(0, cmdSize) != c.first)
            continue;
        if (response.size() > cmdSize) {
            if (response[cmdSize] != ' ')
                continue;
            cmdSize += 1;
        }
        // erase command from response
        response.erase(0, cmdSize);
        return c.second;
    }
    return UNKNOWN;
}

std::string
DBus::AuthenticationProtocol::toHex(const std::string& input)
{
    std::ostringstream hexout;
    hexout << std::hex << std::setfill('0');
    for (auto ch : input) {
        hexout << std::setw(2) << static_cast<unsigned>(ch);
    }
    return hexout.str();
}

