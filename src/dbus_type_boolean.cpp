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

#include "dbus_type_boolean.h"
#include "dbus_messageistream.h"
#include "dbus_messageostream.h"
#include <sstream>

DBus::Type::Boolean::Boolean(bool v)
    : m_Value(v)
{}

void DBus::Type::Boolean::marshall(MessageOStream& stream) const
{
    stream.writeBoolean(m_Value);
}

void DBus::Type::Boolean::unmarshall(MessageIStream& stream)
{
    uint32_t tmp;
    stream.read<uint32_t>(&tmp);
    m_Value = tmp == 0 ? false : true;
}

std::string DBus::Type::Boolean::toString(const std::string&) const
{
    std::stringstream ss;
    ss << "Boolean " << m_Value << '\n';
    return ss.str();
}

std::string DBus::Type::Boolean::asString() const
{
    std::stringstream ss;
    ss << m_Value;
    return ss.str();
}
