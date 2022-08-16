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

#include "dbus_asio.h"
#include "dbus_log.h"
#include "dbus_messageistream.h"
#include "dbus_messageostream.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <memory>

namespace DBus {

using DynamicStringBuffer =
    asio::dynamic_string_buffer<
        std::string::value_type,
        std::string::traits_type,
        std::string::allocator_type>;

class Transport : public std::enable_shared_from_this<Transport> {
public:
    using Ptr = std::shared_ptr<Transport>;
    using EndpointType = asio::local::stream_protocol::socket::endpoint_type;

    static Ptr create(asio::io_context& io_context);

    template<typename Handler>
    void asyncConnect(const std::string& bus_path, Handler&& handler)
    {
        m_socket.async_connect(bus_path, std::forward<Handler>(handler));
    }

    void disconnect()
    {
        if (m_socket.is_open())
            m_socket.close();
    }

    bool connected()
    {
        return m_socket.is_open();
    }

    template<typename MutableBuffer, typename Handler>
    void asyncPeek(const MutableBuffer& buffer, Handler&& handler)
    {
        m_socket.async_receive(buffer, asio::socket_base::message_peek,
            [self = shared_from_this(), buffer, handler = std::move(handler)]
            (const error_code& error, std::size_t bytes_read)
            {
                if (!self->m_socket.is_open())
                    return handler({}, 0);
                if (error && error.value() == EAGAIN)
                    return self->asyncPeek(buffer, handler);
                handler(error, bytes_read);
            });
    }

    template<typename MutableBuffer, typename Handler>
    void asyncRead(const MutableBuffer& buffer, UnixFdBuffer& fds, Handler&& handler)
    {
        m_socket.async_wait(asio::local::stream_protocol::socket::wait_read,
            [self = shared_from_this(), buffer, &fds, handler = std::move(handler)]
            (const error_code& error) mutable
            {
                if (!self->m_socket.is_open())
                    return handler({}, 0);
                if (error && error.value() == EAGAIN)
                    return self->asyncRead(buffer, fds, handler);
                if (error)
                    return handler(error, 0);

                struct iovec iov;
                struct msghdr msg = {};
                msg.msg_iov = &iov;
                msg.msg_iovlen = 1;

                std::size_t fdbufSize = CMSG_SPACE(self->m_maxRecvUnixFds * sizeof(int));
                std::unique_ptr<char> fdbuf(::new char[fdbufSize]);
                msg.msg_control = fdbuf.get();
                msg.msg_controllen = fdbufSize;

                ssize_t read_total = 0;
                do {
                    iov = { static_cast<char*>(buffer.data()) + read_total,
                            buffer.size() - read_total };
                    ssize_t read = ::recvmsg(self->m_socket.native_handle(), &msg, 0);
                    if (read == 0)
                        return handler({}, 0);
                    if (read == -1) {
                        if (errno == EAGAIN)
                            continue;
                        return handler(error_code(errno, system_category()), 0);
                    }

                    read_total += read;

                    for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
                        cmsg != NULL;
                        cmsg = CMSG_NXTHDR(&msg, cmsg)) {

                        if (cmsg->cmsg_level != SOL_SOCKET ||
                            cmsg->cmsg_type != SCM_RIGHTS)
                            continue;

                        int *fdptr = (int *)CMSG_DATA(cmsg);
                        std::size_t len = (cmsg->cmsg_len - CMSG_LEN(0)) / sizeof(int);
                        fds.assign(fdptr, fdptr + len);
                    }
                } while (buffer.size() - read_total > 0);

                handler({}, read_total);
            });
    }

    template<typename Handler>
    void asyncWrite(MessageOStream::Ptr payload, Handler&& handler)
    {
        asio::const_buffer buffer( payload->data.data(), payload->data.size() );
        Log::write(Log::TRACE, "DBus :: Send : Message Data : %ld bytes, %ld FDs\n",
                   buffer.size(), payload->fds.size());
        Log::writeHex(Log::TRACE, "    ", buffer.data(), buffer.size());

        if (payload->fds.empty()) {
            asio::async_write(m_socket, buffer,
                [self = shared_from_this(), payload, handler = std::move(handler)]
                (const error_code& error, std::size_t size) mutable {
                    Log::write(Log::TRACE, "DBus :: Send : message complete\n");
                    handler(error, size);
                });
        } else {
            // send unix fds along the data
            m_socket.async_wait(asio::local::stream_protocol::socket::wait_write,
                [self = shared_from_this(), payload, handler = std::move(handler)]
                (const error_code& error) mutable
                {
                    if (error)
                        return handler(error, 0);

                    struct iovec iov = {
                        const_cast<char*>(payload->data.data()),
                        payload->data.size() };

                    struct msghdr msg = {};
                    msg.msg_iov = &iov;
                    msg.msg_iovlen = 1;

                    const size_t fdlen = payload->fds.size() * sizeof(int);
                    std::size_t fdbufSize = CMSG_SPACE(fdlen);
                    std::unique_ptr<char> fdbuf(::new char[fdbufSize]);
                    msg.msg_control = fdbuf.get();
                    msg.msg_controllen = fdbufSize;

                    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
                    cmsg->cmsg_len = CMSG_LEN(fdlen);
                    cmsg->cmsg_level = SOL_SOCKET;
                    cmsg->cmsg_type = SCM_RIGHTS;

                    std::memcpy(CMSG_DATA(cmsg), payload->fds.data(), fdlen);
                    ssize_t ret = ::sendmsg(self->m_socket.native_handle(), &msg, 0);
                    payload->clearUnixFds();

                    Log::write(Log::TRACE, "DBus :: Send : message complete\n");
                    if (ret == -1)
                        handler(error_code(errno, system_category()), 0);
                    else
                        handler({}, ret);
                });
        }
    }

    std::string getStats() const;

protected:
    Transport(asio::io_context& ioContext);

    friend class AuthenticationProtocol;
    using AuthProtoMode = enum { NoResponseExpected, ResponseExpected };

    template<typename DynamicBuffer, typename Handler>
    void asyncAuthExchange(AuthProtoMode mode, DynamicBuffer&& buffer, Handler&& handler)
    {
        if (mode == NoResponseExpected)
            asio::async_write(m_socket, std::forward<DynamicBuffer>(buffer), std::forward<Handler>(handler));
        else {
            asio::async_write(m_socket, std::forward<DynamicBuffer>(buffer),
                [self = shared_from_this(), &buffer, handler = std::forward<Handler>(handler)]
                (const error_code& error, std::size_t bytes_transferred) mutable {
                    if (error) {
                        handler(error, bytes_transferred);
                        return;
                    }
                    asio::async_read_until(self->m_socket, std::forward<DynamicBuffer>(buffer), "\n", std::forward<Handler>(handler));
                }
            );
        }
    }

    asio::local::stream_protocol::socket m_socket;
    std::size_t m_maxRecvUnixFds = 16;
};

} // namespace DBus
