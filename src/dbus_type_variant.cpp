// This file is part of dbus-asio
// Copyright 2018 Brightsign LLC
// Copyright 2022 OpenVPN Inc <heiko@openvpn.net>
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

#include "dbus_type_variant.h"
#include "dbus_type_signature.h"
#include "dbus_type.h"
#include "dbus_messageistream.h"
#include "dbus_messageostream.h"

#include <sstream>

/*
Variants are marshalled as the SIGNATURE of the contents (which must be a single
complete type), followed by a marshalled value with the type given by that
signature.

The variant has the same 1-byte alignment as the signature, which means that
alignment padding before a variant is never needed. Use of variants must not
cause a total message depth to be larger than 64, including other container
types such as structures. (See Valid Signatures.)
*/

DBus::Type::Variant::Variant(const DBus::Type::Any& v)
    : m_Value(v)
{
}

const DBus::Type::Any& DBus::Type::Variant::getValue() const
{
    return m_Value;
}

void DBus::Type::Variant::marshall(MessageOStream& stream) const
{

    // The marshalled SIGNATURE of a single complete type...
    stream.writeSignature(m_Value.getSignature());

    // ...followed by a marshaled value with the type given in the signature.
    m_Value.marshall(stream);
}

void DBus::Type::Variant::unmarshall(MessageIStream& stream)
{
    Type::Signature signature;
    signature.unmarshall(stream);
    m_Value = DBus::Type::create(signature.asString());
    m_Value.unmarshall(stream);
}

std::string DBus::Type::Variant::toString(const std::string& prefix) const
{
    std::ostringstream oss;
    const std::string content_prefix(prefix + "   ");

    oss << "Variant (" << m_Value.getSignature() << ")\n";
    oss << content_prefix << m_Value.toString(content_prefix);

    return oss.str();
}

std::string DBus::Type::Variant::asString() const
{
    return m_Value.asString();
}
