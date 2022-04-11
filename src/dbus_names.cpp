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

#include "dbus_names.h"

static constexpr std::array<const char*, 7> nameStr = {
    "bus name",
    "unique connection name",
    "well-known name",
    "error name",
    "interface name",
    "namespace name",
    "member name"
};

DBus::Name::operator std::string() const
{
    return m_name;
}

// Restrictions that apply to names in general:
//
//   * Each name must only contain the ASCII characters
//     "[A-Z][a-z][0-9]_" and must not begin with a digit.
//   * Names must not exceed the maximum name length of 255.
//   * Names must contain at least one character.
//
// Additions for interface, error and bus name:
//
//   * Names are composed of 2 or more elements separated by a
//     period ('.') character.
//   * All elements must contain at least one character.
//
// Additions for bus names:
//
//   * Bus names that start with a colon (':') character are unique
//     connection names. Other bus names are called well-known bus names.
//   * Elements that are part of a unique connection name may
//     begin with a digit.
//   * Elements can also contain the hyphen character ('-'), while it
//     being discouraged in new bus names.
void
DBus::Name::validate(const std::string& name, Type type) const
{
    if (name.empty())
        throw InvalidName("name is empty");
    if (name.size() > MaximumSize)
        throw InvalidName(name.substr(0, 16) + "... exceeds " +
            std::to_string(MaximumSize) + " characters");

    bool isUnique = false;
    if (name[0] == ':') {
        if (type != BusName && type != UniqueName)
            throw InvalidName(name + " is not a " + nameStr[type]);
        isUnique = true;
    } else if (type == UniqueName) {
        throw InvalidName(name + " is not a " + nameStr[type]);
    }

    const bool allowHyphen =
        type == BusName || type == UniqueName || type == WellKnownName;
    const bool needPeriod = type != MemberName && type != NamespaceName;
    const bool allowPeriod = type != MemberName;
    bool havePeriod = false;

    char prev = '.';
    for (int i = isUnique ? 1 : 0; i < name.size(); ++i) {
        const char ch = name[i];
        if (allowPeriod && ch == '.') {
            if (prev == '.')
                throw InvalidName(name +" has empty element");
            havePeriod = true;
        }

        if (!isUnique && prev == '.' && (ch >= '0' && ch <= '9')) {
            const char *element = allowPeriod ? " element" : "";
            throw InvalidName(name + element + " starts with digit");
        }

        if ((allowHyphen && ch == '-') || (allowPeriod && ch == '.') ||
            ch == '_' || (ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9'))
            prev = ch;
        else
            throw InvalidName(name +" has invalid character");
    }

    if (needPeriod && !havePeriod)
        throw InvalidName(name +" doesn't have two elements");
}


DBus::BusName::BusName(const std::string& name)
{
    validate(name, Type::BusName);
    m_name = name;
}

DBus::UniqueName::UniqueName(const std::string& name)
{
    validate(name, Type::UniqueName);
    m_name = name;
}

DBus::WellKnownName::WellKnownName(const std::string& name)
{
    validate(name, Type::WellKnownName);
    m_name = name;
}

DBus::InterfaceName::InterfaceName(const std::string& name)
{
    validate(name, Type::InterfaceName);
    m_name = name;
}

DBus::ErrorName::ErrorName(const std::string& name)
{
    validate(name, Type::ErrorName);
    m_name = name;
}

DBus::NamespaceName::NamespaceName(const std::string& name)
{
    validate(name, Type::NamespaceName);
    m_name = name;
}

DBus::MemberName::MemberName(const std::string& name)
{
    validate(name, Type::MemberName);
    m_name = name;
}



DBus::BusName::BusName(const char* name)
    : BusName(std::string(name))
{}

DBus::UniqueName::UniqueName(const char* name)
    : UniqueName(std::string(name))
{}

DBus::WellKnownName::WellKnownName(const char* name)
    : WellKnownName(std::string(name))
{}

DBus::InterfaceName::InterfaceName(const char* name)
    : InterfaceName(std::string(name))
{}

DBus::ErrorName::ErrorName(const char* name)
    : ErrorName(std::string(name))
{}

DBus::NamespaceName::NamespaceName(const char* name)
    : NamespaceName(std::string(name))
{}

DBus::MemberName::MemberName(const char* name)
    : MemberName(std::string(name))
{}



// The following rules define a valid object path.
//
//   * The path may be of any length.
//   * The path must begin with an ASCII '/' character, and must consist of
//     elements separated by slash characters.
//   * Each element must only contain the ASCII characters "[A-Z][a-z][0-9]_"
//   * No element may be the empty string.
//   * Multiple '/' characters cannot occur in sequence.
//   * A trailing '/' character is not allowed unless the path is the root
//     path (a single '/' character).
//
DBus::ObjectPath::ObjectPath(const std::string& path)
{
    if (path.empty())
        throw InvalidObjectPath("path is empty");

    if (path.front() != '/')
        throw InvalidObjectPath(path + " doesn't start with slash");

    if (path.size() > 1) {
        if (path.back() == '/')
            throw InvalidObjectPath(path + " ends with slash");

        char prev = ' ';
        for (const auto& ch : path) {
            if (ch == '/' && prev == '/')
                throw InvalidObjectPath(path + " has // sequence");

            if ( ch == '/' || ch == '_'  || (ch >= 'A' && ch <= 'Z') ||
                (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') )
                prev = ch;
            else
                throw InvalidObjectPath(path + " has invalid character");
        }
    }
    m_name = path;
}

DBus::ObjectPath::ObjectPath(const char* path)
    : ObjectPath(std::string(path))
{}

