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

#include "dbus_type.h"

namespace DBus {

    class Type::UnixFd : public Basic {
    public:
        UnixFd();
        UnixFd(int fd);
        UnixFd(const UnixFd& other);
        ~UnixFd() override;

        static constexpr const char *name = "UnixFd";
        static constexpr std::size_t alignment = 4;
        static constexpr const char code = 'h';

        std::string getName() const override { return name; }
        std::size_t getAlignment() const override { return alignment; }
        std::string getSignature() const override { return std::string(1, code); }

        void marshall(MessageOStream& stream) const override;
        void unmarshall(MessageIStream& stream) override;

        std::string toString(const std::string& = "") const override;
        std::string asString() const override;
        std::string fdName() const;

        int dup() const;

    protected:
        int m_fd;
    };

} // namespace DBus
