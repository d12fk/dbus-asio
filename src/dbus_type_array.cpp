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

#include "dbus_type_array.h"
#include "dbus_type_any.h"
#include "dbus_messageistream.h"
#include "dbus_messageostream.h"
#include "dbus_log.h"

#include <sstream>

DBus::Type::Array::Array(const std::string& signature)
    : m_signature(signature.substr(1))
{}

std::size_t DBus::Type::Array::size() const
{
    return m_contents.size();
}

std::size_t DBus::Type::Array::add(const DBus::Type::Any& v)
{
    if (m_signature.empty())
        m_signature = v.getSignature();
    else if (m_signature != v.getSignature())
        throw std::runtime_error(
            "adding value to Array with wrong signature: " + v.getSignature());
    m_contents.push_back(v);
    return size();
}

void DBus::Type::Array::marshall(MessageOStream& stream) const
{
    const size_t sizePos = stream.size();
    stream.writeUint32(0);

    // Size does not include any padding to the first element
    stream.pad(Type::getAlignment(m_signature));

    const size_t contentsStartPos = stream.size();
    for (auto& elem : m_contents) {
        elem.marshall(stream);
    }
    const uint32_t contentsSize = stream.size() - contentsStartPos;
    if (contentsSize > Array::MaximumSize)
        throw std::out_of_range("Array " + getSignature() +
            ": size " + std::to_string(contentsSize) + " exceeds 64 MiB");
    stream.data.replace(sizePos, 4, reinterpret_cast<const char*>(&contentsSize), sizeof(uint32_t));
}

void DBus::Type::Array::unmarshall(MessageIStream& stream)
{
    uint32_t size = 0;
    stream.read<uint32_t>(&size);
    if (size > Array::MaximumSize)
        throw std::out_of_range("Array " + getSignature() +
            ": size " + std::to_string(size) + " exceeds 64 MiB");

    stream.align(Type::getAlignment(m_signature));

    MessageIStream arrayStream(stream, size);
    while (!arrayStream.empty()) {
        m_contents.push_back(DBus::Type::create(m_signature));
        m_contents.back().unmarshall(arrayStream);
    };
}

std::string DBus::Type::Array::toString(const std::string& prefix) const
{
    std::stringstream ss;

    ss << name << " " << getSignature() << " : [\n";
    for (size_t i = 0; i < m_contents.size(); ++i) {
        ss << prefix << "   [" << i << "] "
           << m_contents[i].toString(prefix + "   ");
    }
    ss << prefix << "]\n";

    return ss.str();
}

std::string DBus::Type::Array::asString() const
{
    return getName() + " " + getSignature();
}
