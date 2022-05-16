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

#include "dbus_type_int64.h"
#include "dbus_messageistream.h"
#include "dbus_messageostream.h"
#include <iomanip>
#include <sstream>

DBus::Type::Int64::Int64(const std::int64_t v)
    : m_Value(v)
{}

void DBus::Type::Int64::marshall(MessageOStream& stream) const
{
    stream.writeInt64(m_Value);
}

void DBus::Type::Int64::unmarshall(MessageIStream& stream)
{
    stream.read<std::int64_t>(&m_Value);
}

std::string DBus::Type::Int64::toString(const std::string&) const
{
    std::ostringstream oss;

    oss << "Int64 "
        << m_Value << " (0x" << std::hex << std::setfill('0') << std::setw(16)
        << m_Value << ")\n";

    return oss.str();
}

std::string DBus::Type::Int64::asString() const
{
    return std::to_string(m_Value);
}
