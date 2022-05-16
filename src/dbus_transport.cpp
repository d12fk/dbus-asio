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

#include "dbus_transport.h"

DBus::Transport::Ptr
DBus::Transport::create(asio::io_context& io_context)
{
    struct ShareableTransport : public Transport {
        ShareableTransport(asio::io_context& io_context)
            : Transport(io_context) {};
    };
    return std::make_shared<ShareableTransport>(io_context);
}


DBus::Transport::Transport(asio::io_context& ioContext)
    : m_socket(ioContext)
{
}
