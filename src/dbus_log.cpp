// This file is part of dbus-asio
// Copyright 2018 Brightsign LLC
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

#include "dbus_log.h"

static size_t g_Level = DBus::Log::WARNING;

bool DBus::Log::isActive(size_t type)
{
    if (type < g_Level) {
        return false;
    }
    return true;
}

void DBus::Log::write(size_t type, const char* msg, ...)
{
    if (!isActive(type)) {
        return;
    }

    va_list ap;
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    flush();

    va_end(ap);
}

void DBus::Log::writeHex(size_t type, const std::string& prefix,
                         const std::string& hex)
{
    writeHex(type, prefix, hex.data(), hex.size());
}

void DBus::Log::writeHex(std::size_t type, const std::string& prefix,
    const void *data, std::size_t size)
{
    if (data == nullptr || !isActive(type)) {
        return;
    }

    std::size_t column = 0;
    constexpr std::size_t maxColumns = 24;
    constexpr std::size_t dividerAfter = 8;
    char asciiBuf[maxColumns + maxColumns / dividerAfter + 1] = {};
    char *ascii = asciiBuf;
    const unsigned char *first = static_cast<const std::uint8_t *>(data);

    std::ostringstream oss;
    oss << prefix;
    for (std::size_t i = 0; i < size; ++i) {
        const unsigned char byte = *(first + i);
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<const unsigned>(byte) << ' ';
        *ascii++ = ::isprint(byte) ? byte : '.';
        if (++column == maxColumns) {
            column = 0;
            oss << "| " << asciiBuf << '\n';
            std::memset(asciiBuf, 0, sizeof(asciiBuf));
            ascii = asciiBuf;
            if (i + 1 < size) { // pad the next line if there is one.
                oss << std::string(prefix.size(), ' ');
            }
        }
        else if (column % dividerAfter == 0) {
            oss << ' ';
            *ascii++ = ' ';
        }
    }
    // Finish the last line
    if (column > 0) {
        const std::size_t bytes = (maxColumns - column) * 3;
        const std::size_t dividers = ((maxColumns - 1) / dividerAfter) - (column / dividerAfter);
        oss << std::string(bytes + dividers, ' ') << "| " << asciiBuf << '\n';
    }

    write(type, oss.str().c_str());
}

void DBus::Log::flush() { fflush(stderr); }

void DBus::Log::setLevel(size_t lowest_visible_level)
{
    g_Level = lowest_visible_level;
}
