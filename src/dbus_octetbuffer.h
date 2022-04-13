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

#include <cstddef>
#include <cstdint>
#include <vector>

namespace DBus {

using UnixFdBuffer = std::vector<int>;

class OctetBuffer {
    const uint8_t* m_data;
    size_t m_size;
    UnixFdBuffer m_fds;

public:
    OctetBuffer(const uint8_t* data, size_t size);
    OctetBuffer(OctetBuffer& other, std::size_t size);
    OctetBuffer(const uint8_t* data, std::size_t dataSize,
                const UnixFdBuffer& fds);

    size_t size() const;
    const uint8_t* data() const;
    void remove_prefix(size_t count);
    bool empty() const;
    uint8_t operator[](unsigned long index) const;
    void copy(uint8_t* data, size_t size) const;
    size_t find(uint8_t byte) const;
    int getUnixFd(std::uint32_t index) const;
};

} // namespace DBus
