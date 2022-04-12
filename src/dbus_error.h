// This file is part of dbus-asio
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

// Wrapper class for the ASIO ErrorCode and DBus::Message::Error

#pragma once

#include "dbus_message.h"
#include "dbus_asio.h"

namespace DBus {

struct Error {
    Error()
        : m_error(false)
    {}

    Error(const std::string& message, const std::string& category = "")
        : category(category)
        , message(message)
        , m_error(true)
    {}

    Error(const error_code& error)
        : category(error.category().name())
        , message(error.message())
        , m_error(error)
    {}

    Error(const Message::Error& error)
        : category(error.getName())
        , message(error.getMessage())
        , m_error(!category.empty())
    {}

    explicit operator bool() const { return m_error; }

    std::string category;
    std::string message;

protected:
    bool m_error;
};

}
