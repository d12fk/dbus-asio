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

#include "dbus_type_any.h"

namespace DBus {

    class Signature;

    class Type::DictEntry : public Container {
    public:
        DictEntry(const std::string& signature);
        DictEntry(const DBus::Type::Any& key, const DBus::Type::Any& value);

        static constexpr const char *name = "DictEntry";
        static constexpr const char code_start = '{';
        static constexpr const char code_end = '}';

        static constexpr std::size_t alignment = 8;
        static constexpr const char code = code_start;

        std::string getName() const override { return name; }
        std::size_t getAlignment() const override { return alignment; };
        std::string getSignature() const override;

        void marshall(MessageOStream& stream) const override;
        void unmarshall(MessageIStream& stream) override;

        const Any& key() const { return m_Value.first; };
        const Any& value() const { return m_Value.second; };

        std::string toString(const std::string& prefix = "") const override;
        std::string asString() const override;

        void set(const DBus::Type::Any& key, const DBus::Type::Any& value);

    protected:
        Signature m_signature;
        std::pair<Any, Any> m_Value = {};
    };

} // namespace DBus
