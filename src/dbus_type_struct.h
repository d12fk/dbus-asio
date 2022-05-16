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

    class Type::Struct : public Container {
    public:
        Struct() = default;
        Struct(const Signature& signature);
        Struct(const std::string& signature);

        static constexpr const char code_start = '(';
        static constexpr const char code_end = ')';

        static constexpr const char *name = "Struct";
        static constexpr std::size_t alignment = 8;
        static constexpr const char code = code_start;

        std::string getName() const override { return name; }
        std::size_t getAlignment() const override { return alignment; };
        std::string getSignature() const override;

        void marshall(MessageOStream& stream) const override;
        void unmarshall(MessageIStream& stream) override;

        std::string toString(const std::string& prefix = "") const override;
        std::string asString() const override;

        void clear();
        void add(const DBus::Type::Any& v);

        std::size_t size() const { return m_elements.size(); }
        const Any& operator[](std::size_t pos) const { return m_elements.at(pos); }

    private:
        Signature m_Signature;
        std::vector<Any> m_elements;
    };

} // namespace DBus
