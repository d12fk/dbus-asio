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

#include "dbus_names.h"
#include "dbus_type_objectpath.h"
#include "dbus_messageostream.h"

DBus::Type::ObjectPath::ObjectPath(const char* v)
    : ObjectPath(DBus::ObjectPath(v))
{}

DBus::Type::ObjectPath::ObjectPath(const std::string& v)
    : ObjectPath(DBus::ObjectPath(v))
{}

DBus::Type::ObjectPath::ObjectPath(const DBus::ObjectPath& v)
    : String(v)
{}

void DBus::Type::ObjectPath::marshall(MessageOStream& stream) const
{
    if (m_Value.empty())
        throw InvalidObjectPath("Cannot marshall empty ObjectPath");
    Type::String::marshall(stream);
}
