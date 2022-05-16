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

#include "dbus_type_unixfd.h"
#include "dbus_messageistream.h"
#include "dbus_messageostream.h"

#include <iomanip>
#include <sstream>

#include <fcntl.h>
#include <unistd.h>

DBus::Type::UnixFd::UnixFd()
    : m_fd(-1)
{}

DBus::Type::UnixFd::UnixFd(int fd)
    : m_fd(::dup(fd))
{}

DBus::Type::UnixFd::UnixFd(const UnixFd& other)
    : m_fd(::dup(other.m_fd))
{}

DBus::Type::UnixFd::~UnixFd() {
    ::close(m_fd);
}

void DBus::Type::UnixFd::marshall(MessageOStream& stream) const
{
    stream.writeUnixFd(dup());
}

#include "dbus_log.h"
void DBus::Type::UnixFd::unmarshall(MessageIStream& stream)
{
    m_fd = ::dup(stream.readUnixFd());
}

std::string DBus::Type::UnixFd::toString(const std::string&) const
{
    std::ostringstream oss;
    oss << name << " " << fdName() << " (" << m_fd << ")\n";
    return oss.str();
}

std::string DBus::Type::UnixFd::asString() const
{
    return std::to_string(m_fd);
}

std::string DBus::Type::UnixFd::fdName() const
{
    if (m_fd < 0)
        return "invalid";

    char fd_name[512] = "unknown";
    int dirfd = ::open("/proc/self/fd/", O_DIRECTORY|O_PATH);

    if (dirfd != -1) {
        ::readlinkat(dirfd, asString().c_str(), fd_name, sizeof(fd_name));
        ::close(dirfd);
    }

    return fd_name;
}

int DBus::Type::UnixFd::dup() const {
    return ::dup(m_fd);
}

