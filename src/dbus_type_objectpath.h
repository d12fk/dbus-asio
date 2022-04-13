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

#include "dbus_type_string.h"

#include <stdexcept>

namespace DBus {

    struct ObjectPath;

    class Type::ObjectPath : public String {
    public:
        ObjectPath() = default;
        ObjectPath(const char* v);
        ObjectPath(const std::string& v);
        ObjectPath(const DBus::ObjectPath& v);

        static constexpr const char *name = "ObjectPath";
        static constexpr const char code = 'o';

        std::string getName() const override { return name; }
        std::string getSignature() const override { return std::string(1, code); };

        void marshall(MessageOStream& stream) const override;
    };

} // namespace DBus
