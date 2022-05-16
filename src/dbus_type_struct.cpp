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

#include "dbus_type_signature.h"
#include "dbus_type_struct.h"
#include "dbus_messageistream.h"
#include "dbus_messageostream.h"

#include <sstream>
#include <stdexcept>

/*
Structs and dict entries are marshalled in the same way as their contents, but
their alignment is always to an 8-byte boundary, even if their contents would
normally be less strictly aligned.
*/

DBus::Type::Struct::Struct(const Signature& signature)
    : m_Signature(signature)
{
}

DBus::Type::Struct::Struct(const std::string& signature)
    : m_Signature(signature.substr(1, signature.size() - 2))
{
}

void DBus::Type::Struct::add(const DBus::Type::Any& v)
{
    m_Signature = m_Signature.asString() + v.getSignature();
    m_elements.push_back(v);
}

std::string DBus::Type::Struct::getSignature() const
{
    if (m_Signature.empty())
        throw std::runtime_error("Struct::getSignature() called when undefined");

    return code_start + m_Signature.asString() + code_end;
}

// BUGWARN: This should work in all cases, but I'm thinking of moving it into
// a utility method (like all unmarshall).
void DBus::Type::Struct::clear()
{
    m_elements.clear();
}

void DBus::Type::Struct::marshall(MessageOStream& stream) const
{
    // A struct must start on an 8-byte boundary regardless of the type of the
    // struct fields.
    stream.pad8();

    for (auto element : m_elements) {
        element.marshall(stream);
    }
}

void DBus::Type::Struct::unmarshall(MessageIStream& stream)
{
    // A struct must start on an 8-byte boundary regardless of the type of the
    // struct fields.
    stream.align(8);
    std::string code = m_Signature.getNextTypeCode();
    while (!code.empty()) {
        m_elements.push_back(DBus::Type::create(code));
        m_elements.back().unmarshall(stream);
        code = m_Signature.getNextTypeCode();
    }
}

std::string DBus::Type::Struct::toString(const std::string& prefix) const
{
    std::ostringstream oss;
    const std::string contents_prefix(prefix + "   ");

    oss << name << " " << getSignature() << " : (\n";
    for (auto element : m_elements) {
        oss << contents_prefix << element.toString(contents_prefix);
    }
    oss << prefix << ")\n";

    return oss.str();
}

std::string DBus::Type::Struct::asString() const
{
    return getName() + " " + getSignature();
}
