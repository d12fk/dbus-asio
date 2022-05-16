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

#include "dbus_log.h"
#include "dbus_utils.h"

#include "dbus_type.h"
#include "dbus_type_signature.h"
#include "dbus_type_struct.h"

#include "dbus_message.h"
#include "dbus_messageistream.h"
#include "dbus_messageostream.h"
#include "dbus_messageprotocol.h"

#include <iostream>

DBus::MessageProtocol::Ptr
DBus::MessageProtocol::start(asio::io_context& ioContext, Transport::Ptr transport)
{
    struct ShareableMsgProto : public MessageProtocol {
        ShareableMsgProto(asio::io_context& ioContext, Transport::Ptr transport)
            : MessageProtocol(ioContext, transport) {};
    };
    auto obj = std::make_shared<ShareableMsgProto>(ioContext, transport);
    obj->asyncReadMessage(ReadState::Peek);
    return obj;
}

DBus::MessageProtocol::MessageProtocol(asio::io_context& ioContext, Transport::Ptr transport)
    : m_ioContext(ioContext)
    , m_transport(transport)
{
    m_messageBuffer.reserve(256);
}

DBus::MessageProtocol::ReadBuffer
DBus::MessageProtocol::makeReadBuffer(ReadState state, MessageBuffer& buffer)
{
    if (state == ReadState::Peek) {
        buffer.resize(Message::Header::MinimumSize);
    }
    else { // ReadState::Receive
        OctetBuffer header = { buffer.data(), buffer.size() };
        buffer.resize(Message::Header::getMessageSize(header));
    }
    return { buffer.data(), buffer.size() };
}

void DBus::MessageProtocol::asyncReadMessage(ReadState state)
{
    if (state == ReadState::Peek) {
        m_transport->asyncPeek(
            makeReadBuffer(state, m_messageBuffer),
            [self = shared_from_this()]
            (const error_code& error, std::size_t bytes_read)
            {
                if (error)
                    return self->invokeErrorHandler(0, error);
                if (bytes_read == 0)
                    return self->releasePendingHandlers();
                if (bytes_read < Message::Header::MinimumSize)
                    return self->invokeErrorHandler(0, {
                        "short read peeking header",
                        "message protocol :: read message" });

                self->asyncReadMessage(ReadState::Receive);
            });
    } else {
        m_transport->asyncRead(
            makeReadBuffer(state, m_messageBuffer),
            m_unixFdBuffer,
            [self = shared_from_this()]
            (error_code error, std::size_t bytes_read) mutable
            {
                if (error)
                    return self->invokeErrorHandler(0, error);
                if (bytes_read == 0)
                    return self->releasePendingHandlers();

                MessageBuffer& msg = self->m_messageBuffer;
                UnixFdBuffer& fds = self->m_unixFdBuffer;
                OctetBuffer message(msg.data(), msg.size(), fds);

                if (bytes_read < Message::Header::MinimumSize ||
                    bytes_read < Message::Header::getMessageSize(message))
                    return self->invokeErrorHandler(0, {
                        "short read receiving message",
                        "message protocol :: read message" });

                Log::write(Log::TRACE, "\nDBus :: Receive : Message Data : %ld bytes, %ld FDs\n",
                           msg.size(), fds.size());
                Log::writeHex(Log::TRACE, "    ", msg.data(), msg.size());
                self->dispatchMessage(message);

                msg.clear();
                fds.clear();
                self->asyncReadMessage(ReadState::Peek);
            });
    }
}

void DBus::MessageProtocol::dispatchMessage(OctetBuffer& message)
{
    m_stats.bytes_recv += message.size();
    const Message::Header header(message);
    Log::write(Log::TRACE, "DBus :: Recv : dispatching %s message\n",
               Message::typeString(header.type).c_str());

    switch (header.type) {
    case Message::Type::MethodCall:
        ++m_stats.count_recv_methodcalls;
        invokeMethodCallHandler(header.getFullName(), Message::MethodCall(header, message));
        break;

    case Message::Type::MethodReturn:
        ++m_stats.count_recv_methodreturns;
        invokeMethodReturnHandler(header.replySerial, Message::MethodReturn(header, message));
        break;

    case Message::Type::Signal:
        ++m_stats.count_recv_signals;
        invokeSignalHandler(header.getFullName(), Message::Signal(header, message));
        break;

    case Message::Type::Error:
        ++m_stats.count_recv_errors;
        invokeErrorHandler(header.replySerial, Message::Error(header, message));
        break;

    default:
        Log::write(Log::WARNING, "DBus :: Ignoring unknown message type %d\n",
            static_cast<std::underlying_type_t<decltype(header.type)>>(header.type));
    }
}
