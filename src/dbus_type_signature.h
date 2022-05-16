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

#include "dbus_type.h"

#include <stdexcept>

namespace DBus {

    using InvalidSignature = std::runtime_error;

    class Type::Signature : public Basic {
    public:
        Signature() = default;
        Signature(const Signature& other) = default;
        Signature(const std::string& v, std::size_t height = 0);

        Signature& operator=(const Signature& other) = default;

        bool operator==(const Signature& other) {
            return m_Value == other.m_Value; }
        bool operator!=(const Signature& other) {
            return m_Value != other.m_Value; }

        static constexpr const char *name = "Signature";
        static constexpr std::size_t alignment = 1;
        static constexpr const char code = 'g';

        std::string getName() const override { return name; }
        std::size_t getAlignment() const override { return alignment; }
        std::string getSignature() const override { return std::string(1, code); }

        void marshall(MessageOStream& stream) const override;
        void unmarshall(MessageIStream& stream) override;

        std::string toString(const std::string&) const override;
        std::string asString() const override;

        bool empty() const { return m_Value.empty(); }
        std::string getNextTypeCode();

    protected:
        static constexpr std::size_t MaxLength = 255;
        static constexpr std::size_t MaxStructDepth = 32;
        static constexpr std::size_t MaxArrayDepth = 32;
        static constexpr std::size_t MaxDepth = MaxStructDepth + MaxArrayDepth;

        std::size_t m_typeCodeIndex = 0;
        std::string m_Value;
    };

} // namespace DBus
