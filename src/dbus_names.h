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

#pragma once

#include <array>
#include <string>
#include <stdexcept>

namespace DBus {

    using InvalidName = std::runtime_error;
    using InvalidObjectPath = std::runtime_error;


    class Name {
    public:
        static constexpr std::size_t MaximumSize = 255;
        operator std::string() const;

    protected:
        enum Type {
            BusName, UniqueName, WellKnownName, ErrorName,
            InterfaceName, NamespaceName, MemberName };
        Name() = default;
        void validate(const std::string& name, Type type) const;
        std::string m_name;
    };

    struct BusName : public Name {
        BusName(BusName&&) = default;
        BusName(const BusName&) = default;
        BusName(const char* name);
        BusName(const std::string& name);
    protected:
        BusName() = default;
    };

    struct UniqueName : public BusName {
        UniqueName(UniqueName&&) = default;
        UniqueName(const UniqueName&) = default;
        UniqueName(const char* name);
        UniqueName(const std::string& name);
    };

    struct WellKnownName : public BusName {
        WellKnownName(WellKnownName&&) = default;
        WellKnownName(const WellKnownName&) = default;
        WellKnownName(const char* name);
        WellKnownName(const std::string& name);
    };

    struct InterfaceName : public Name {
        InterfaceName(InterfaceName&&) = default;
        InterfaceName(const InterfaceName&) = default;
        InterfaceName(const char* name);
        InterfaceName(const std::string& name);
    };

    struct ErrorName : public Name {
        ErrorName(ErrorName&&) = default;
        ErrorName(const ErrorName&) = default;
        ErrorName(const char* name);
        ErrorName(const std::string& name);
    };

    struct NamespaceName : public Name {
        NamespaceName(NamespaceName&&) = default;
        NamespaceName(const NamespaceName&) = default;
        NamespaceName(const char* name);
        NamespaceName(const std::string& name);
    };

    struct MemberName : public Name {
        MemberName(MemberName&&) = default;
        MemberName(const MemberName&) = default;
        MemberName(const char* name);
        MemberName(const std::string& name);
    };

    struct ObjectPath : public Name {
        ObjectPath(ObjectPath&&) = default;
        ObjectPath(const ObjectPath&) = default;
        ObjectPath(const char* path);
        ObjectPath(const std::string& path);
    };

}
