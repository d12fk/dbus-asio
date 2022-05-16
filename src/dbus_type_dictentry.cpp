// This file is part of dbus-asio
// Copyright 2018 Brightsign LLC
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
#include "dbus_type_dictentry.h"
#include "dbus_type.h"
#include "dbus_type_string.h"
#include "dbus_type_uint32.h"

#include "dbus_message.h"
#include "dbus_messageistream.h"
#include "dbus_messageostream.h"
#include "dbus_messageprotocol.h"

#include <sstream>
/*
Structs and dict entries are marshalled in the same way as their contents, but
their alignment is always to an 8-byte boundary, even if their contents would
normally be less strictly aligned.

A DICT_ENTRY works exactly like a struct, but rather than parentheses it uses
curly braces, and it has more restrictions. The restrictions are: it occurs only
as an array element type; it has exactly two single complete types inside the
curly braces; the first single complete type (the "key") must be a basic type
rather than a container type. Implementations must not accept dict entries
outside of arrays, must not accept dict entries with zero, one, or more than two
fields, and must not accept dict entries with non-basic-typed keys. A dict entry
is always a key-value pair.

The first field in the DICT_ENTRY is always the key. A message is considered
corrupt if the same key occurs twice in the same array of DICT_ENTRY. However,
for performance reasons implementations are not required to reject dicts with
duplicate keys.
*/

DBus::Type::DictEntry::DictEntry(const std::string& signature)
    : m_signature(signature.substr(1, signature.size() - 2))
{
}

DBus::Type::DictEntry::DictEntry(const DBus::Type::Any& key,
    const DBus::Type::Any& value)
{
    set(key, value);
}

void DBus::Type::DictEntry::set(const DBus::Type::Any& key,
    const DBus::Type::Any& value)
{
    if (!key.isBasicType())
        throw std::runtime_error("DictEntry key has invalid basic type: " +
                                 key.getSignature());
    m_Value = std::make_pair(key, value);
}

std::string DBus::Type::DictEntry::getSignature() const
{
    return code_start + key().getSignature() + value().getSignature() + code_end;
}

void DBus::Type::DictEntry::marshall(MessageOStream& stream) const
{
    stream.pad8();

    key().marshall(stream);
    value().marshall(stream);
}

void DBus::Type::DictEntry::unmarshall(MessageIStream& stream)
{
    stream.align(8);

    Any key = Type::create(m_signature.getNextTypeCode());
    Any value = Type::create(m_signature.getNextTypeCode());

    key.unmarshall(stream);
    value.unmarshall(stream);

    set(key, value);
}

std::string DBus::Type::DictEntry::toString(const std::string& prefix) const
{
    std::ostringstream oss;

    oss << "DictEntry " << getSignature() << " : {\n";
    oss << prefix << "   key:      " << key().toString();
    oss << prefix << "   value:    " << value().toString(prefix + "   ");
    oss << prefix << "}\n";

    return oss.str();
}

std::string DBus::Type::DictEntry::asString() const
{
    return getName() + " " + getSignature();
}
