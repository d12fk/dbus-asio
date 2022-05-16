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

#include "dbus.h"
#include <cstdio>

int main()
{
    DBus::Log::setLevel(DBus::Log::WARNING);

    DBus::Log::write(DBus::Log::INFO, "System bus: %s\n",
        DBus::Platform::getSystemBus().c_str());
    DBus::Log::write(DBus::Log::INFO, "Session bus: %s\n",
        DBus::Platform::getSessionBus().c_str());

    DBus::asio::io_context ioc;
    auto conn = DBus::Connection::create(ioc);

    conn->connect(
        DBus::Platform::getSessionBus(),
        DBus::AuthenticationProtocol::create(),
        [conn](const DBus::Error& e, const std::string& guid, const std::string& name)
        {
            if (e)
                DBus::Log::write(
                    DBus::Log::ERROR,
                    "error: %s (%s)\n", e.message.c_str(), e.category.c_str());

            DBus::Log::write(
                DBus::Log::INFO,
                ">>> server guid: %s  my bus name: %s\n", guid.c_str(), name.c_str());

            conn->disconnect();
        });

    ioc.run();

    return 0;
}
