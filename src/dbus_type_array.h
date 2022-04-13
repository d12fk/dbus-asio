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

    class Type::Array : public Container {
    public:
        Array() = default;
        Array(const std::string& signature);

        static constexpr std::size_t MaximumSize = 67108864 /*64 MiB*/;

        static constexpr const char *name = "Array";
        static constexpr std::size_t alignment = 4;
        static constexpr const char code = 'a';

        std::string getName() const override { return name; }
        std::size_t getAlignment() const override { return alignment; }
        std::string getSignature() const override { return code + m_signature; }

        void marshall(MessageOStream& stream) const override;
        void unmarshall(MessageIStream& stream) override;

        std::string toString(const std::string& prefix = "") const override;
        std::string asString() const override;

        std::size_t size() const;
        std::size_t add(const Any& v);

        using ConstIterator = std::vector<Any>::const_iterator;
        ConstIterator begin() const { return m_contents.cbegin(); }
        ConstIterator end() const { return m_contents.cend(); }

        const Any& operator[](std::size_t pos) const { return m_contents.at(pos); }
        const Array* sis() const {return this;}

    protected:
        std::string m_signature;
        std::vector<Any> m_contents;
    };

} // namespace DBus

