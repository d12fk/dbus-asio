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

#include "dbus_matchrule.h"
#include "dbus_type_objectpath.h"
#include "dbus_names.h"

#include <sstream>

// https://dbus.freedesktop.org/doc/dbus-specification.html#message-bus-routing-match-rules

DBus::MatchRule&
DBus::MatchRule::type(Type type)
{
    if (type == Type::MethodCall)
        m_type = "method_call";
    else if (type == Type::MethodReturn)
        m_type = "method_return";
    else if (type == Type::Signal)
        m_type = "signal";
    else if (type == Type::Error)
        m_type = "error";
    return *this;
}

DBus::MatchRule&
DBus::MatchRule::sender(const BusName& name)
{
    m_sender = name;
    return *this;
}

DBus::MatchRule&
DBus::MatchRule::interface(const InterfaceName& name)
{
    m_interface = name;
    return *this;
}

DBus::MatchRule&
DBus::MatchRule::member(const MemberName& name)
{
    m_member = name;
    return *this;
}

DBus::MatchRule&
DBus::MatchRule::path(const ObjectPath& name)
{
    if (!m_pathNamespace.empty())
        throw InvalidMatchRule(
            "path and path_namespace are not allowed together.");
    m_path = name;
    return *this;
}

DBus::MatchRule&
DBus::MatchRule::pathNamespace(const ObjectPath& name)
{
    if (!m_path.empty())
        throw InvalidMatchRule(
            "path and path_namespace are not allowed together.");
    m_pathNamespace = name;
    return *this;
}

DBus::MatchRule&
DBus::MatchRule::destination(const UniqueName& name)
{
    m_destination = name;
    return *this;
}

DBus::MatchRule&
DBus::MatchRule::arg0Namespace(const NamespaceName& name)
{
    m_arg0Namespace = name;
    return *this;
}

DBus::MatchRule&
DBus::MatchRule::arg(std::uint8_t index, const std::string& string)
{
    if (index > MaximumIndex)
        throw InvalidMatchRule(
            "arg index exceeds " + std::to_string(MaximumIndex));
    m_arg[index] = string;
    escapeApostrophes(m_arg[index]);
    return *this;
}

DBus::MatchRule&
DBus::MatchRule::argPath(std::uint8_t index, const std::string& string)
{
    if (index > MaximumIndex)
        throw InvalidMatchRule(
            "arg path index exceeds " + std::to_string(MaximumIndex));
    m_argPath[index] = string;
    escapeApostrophes(m_argPath[index]);
    return *this;
}

std::string DBus::MatchRule::str() const
{
    std::ostringstream matchRule;

    if (!m_type.empty())
        matchRule << ",type='" << m_type << "'";
    if (!m_sender.empty())
        matchRule << ",sender='" << m_sender << "'";
    if (!m_interface.empty())
        matchRule << ",interface='" << m_interface << "'";
    if (!m_member.empty())
        matchRule << ",member='" << m_member << "'";
    if (!m_path.empty())
        matchRule << ",path='" << m_path << "'";
    if (!m_pathNamespace.empty())
        matchRule << ",path_namespace='" << m_pathNamespace << "'";
    if (!m_destination.empty())
        matchRule << ",destination='" << m_destination << "'";
    if (!m_arg0Namespace.empty())
        matchRule << ",arg0namespace='" << m_arg0Namespace << "'";

    for (auto it = m_arg.cbegin(); it != m_arg.cend(); ++it)
        matchRule << ",arg" + std::to_string(it->first) + "='" << it->second << "'";

    for (auto it = m_argPath.cbegin(); it != m_argPath.cend(); ++it)
        matchRule << ",arg" + std::to_string(it->first) + "path='" << it->second << "'";

    // Return empty string for match all rule
    if (matchRule.tellp() == 0)
        return "";

    // Remove the leading comma from match rule
    return matchRule.str().substr(1);
}

void DBus::MatchRule::escapeApostrophes(std::string& value)
{
    const std::string what("'");
    const std::string with("'\\''");

    std::size_t pos = 0;
    while ((pos = value.find(what, pos)) != value.npos) {
        value.replace(pos, what.size(), with);
        pos += with.size();
    }
}
