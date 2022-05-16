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

#pragma once

#include <stdexcept>
#include <string>
#include <map>

namespace DBus {

// https://dbus.freedesktop.org/doc/dbus-specification.html#message-bus-routing-match-rules

struct BusName;
struct UniqueName;
struct MemberName;
struct InterfaceName;
struct NamespaceName;

class ObjectPath;

using InvalidMatchRule = std::runtime_error;

    class MatchRule {
    public:
        static constexpr std::size_t MaximumIndex = 63;
        enum class Type{ MethodCall, MethodReturn, Signal, Error };

        MatchRule& type(Type type);
        MatchRule& sender(const BusName& name);
        MatchRule& interface(const InterfaceName& name);
        MatchRule& member(const MemberName& name);
        MatchRule& path(const ObjectPath& name);
        MatchRule& pathNamespace(const ObjectPath& name);
        MatchRule& destination(const UniqueName& name);
        MatchRule& arg0Namespace(const NamespaceName& name);
        MatchRule& arg(std::uint8_t index, const std::string& string);
        MatchRule& argPath(std::uint8_t index, const std::string& string);

        std::string str() const;

    protected:
        void escapeApostrophes(std::string& value);

        std::string m_type;
        std::string m_sender;
        std::string m_interface;
        std::string m_member;
        std::string m_path;
        std::string m_pathNamespace;
        std::string m_destination;
        std::string m_arg0Namespace;

        std::map<std::uint8_t, std::string> m_arg;
        std::map<std::uint8_t, std::string> m_argPath;
    };

} // namespace DBus
