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

#include <memory>
#include <sstream>

#include "dbus_asio.h"
#include "dbus_utils.h"
#include "dbus_platform.h"
#include "dbus_transport.h"

namespace DBus {

// See Authentication state diagrams
class AuthenticationProtocol : public std::enable_shared_from_this<AuthenticationProtocol>
{
public:
    using Ptr = std::shared_ptr<AuthenticationProtocol>;

    struct Statistics {
        std::size_t count_send;
        std::size_t count_recv;
        std::size_t bytes_send;
        std::size_t bytes_recv;
    };

    static Ptr create();

    bool unixFdNegotiated() { return m_unixFdNegotiated; }

    template<typename CompletionToken>
    auto asyncAuthenticate(
        Transport::Ptr transport,
        CompletionToken&& token)
    {
        m_transport = transport;
        m_state = Starting;
        m_server_guid.clear();

        return asio::async_compose<
            CompletionToken, void(const error_code&, const std::string&)>(
            [self = shared_from_this()]
            (auto& composition, const error_code& error = {}, const std::size_t size = 0) mutable
            {
                if (size) {
                    ++self->m_stats.count_recv;
                    self->m_stats.bytes_recv += size;
                }
                if (error)
                    return composition.complete(error, "");

                std::string& response = self->m_data;
                Command command = UNKNOWN;
                if (response.size()) {
                    Log::write(Log::TRACE, "DBus :: Recv : Auth Cmd :\n");
                    Log::writeHex(Log::TRACE, "    ", response);
                    if (response.size() < 3)
                        return composition.complete(make_error_code(errc::protocol_error), "");
                    response.resize(response.size() - 2); // remove trailing \r\n
                    Log::write(Log::TRACE, "    %s\n", response.c_str());
                    command = self->extractCommand(response);
                }

                if (self->m_state == Starting)
                    self->sendCredentials(std::move(composition));
                else if (self->m_state == SendingCredentials)
                    self->sendAuth(std::move(composition));
                else if (self->m_state == WaitingForData)
                    if (command == DATA)
                        self->handleData(response, std::move(composition));
                    else if (command == REJECTED)
                        self->handleRejected(response, std::move(composition));
                    else if (command == ERROR)
                        self->handleError(response, std::move(composition));
                    else if (command == OK)
                        self->handleOk(response, std::move(composition));
                    else
                        self->sendError(response, std::move(composition));
                else if (self->m_state == WaitingForOK)
                    if (command == OK)
                        self->handleOk(response, std::move(composition));
                    else if (command == REJECTED)
                        self->handleRejected(response, std::move(composition));
                    else if (command == DATA || command == ERROR)
                        self->sendCancel(std::move(composition));
                    else
                        self->sendError(response, std::move(composition));
                else if (self->m_state == WaitingForReject)
                    if (command == REJECTED)
                        self->handleRejected(response, std::move(composition));
                    else
                        return composition.complete(make_error_code(errc::protocol_error), "");
                else if (self->m_state == WaitingForAgreeUnixFD)
                    if (command == AGREE_UNIX_FD)
                        self->handleAgreeUniXFD(std::move(composition));
                    else
                        self->sendBegin(std::move(composition));
                else if (self->m_state == Finishing)
                    return composition.complete(error, self->m_server_guid);
                else {
                    Log::write(Log::ERROR, "DBus :: ERROR : unhandled auth state");
                    return composition.complete(make_error_code(errc::protocol_error), "");
                }
            },
            token);
    }

    struct Statistics getStats() { return m_stats; }

protected:
    AuthenticationProtocol();

    enum State {
        Starting,
        SendingCredentials,
        WaitingForData,
        WaitingForOK,
        WaitingForReject,
        WaitingForAgreeUnixFD,
        Finishing
    };

    enum Command { OK, DATA, ERROR, REJECTED, AGREE_UNIX_FD, UNKNOWN };
    const std::array< std::pair<const std::string, Command>, 5 > m_commands = {
        std::make_pair("OK", OK),
        std::make_pair("DATA", DATA),
        std::make_pair("ERROR", ERROR),
        std::make_pair("REJECTED", REJECTED),
        std::make_pair("AGREE_UNIX_FD", AGREE_UNIX_FD),
    };

    Command extractCommand(std::string& response);
    std::string toHex(const std::string& input);

    template<typename Handler>
    void sendCommand(bool expectResponse, std::string&& command, Handler&& handler)
    {
        ++m_stats.count_send;
        m_stats.bytes_send += command.size();
        Log::write(Log::TRACE, "DBus :: Send : Auth Cmd : %ld bytes\n    %s",
                   command.size(), (command[0] == 0 ? "\\0\n" : command.c_str()));
        Log::writeHex(Log::TRACE, "    ", command);
        m_data = std::move(command);
        m_transport->asyncAuthExchange(
            expectResponse ? Transport::ResponseExpected : Transport::NoResponseExpected,
            m_buffer, std::forward<Handler>(handler));
    }

    template<typename Handler>
    void sendCredentials(Handler&& handler)
    {
        // No credentials sent here, but we have to send the null byte anyway
        sendCommand(false, std::string(1, '\0'), std::forward<Handler>(handler));
        m_state = SendingCredentials;
    }

    template<typename Handler>
    void sendAuth(Handler&& handler)
    {
        std::ostringstream cmd;
        const std::string uid = std::to_string(Platform::getUID());
        cmd << "AUTH EXTERNAL " << toHex(uid) << "\r\n";
        sendCommand(true, cmd.str(), std::forward<Handler>(handler));
        m_state = WaitingForOK;
    }

    template<typename Handler>
    void sendData(const std::string& data, Handler&& handler)
    {
        std::ostringstream cmd;
        cmd << "DATA " << toHex(data) << "\r\n";
        sendCommand(true, cmd.str(), std::forward<Handler>(handler));
        m_state = WaitingForData;
    }

    template<typename Handler>
    void sendNegotiateUnixFD(Handler&& handler)
    {
        sendCommand(true, "NEGOTIATE_UNIX_FD\r\n", std::forward<Handler>(handler));
        m_state = WaitingForAgreeUnixFD;
    }

    template<typename Handler>
    void sendError(const std::string& message, Handler&& handler)
    {
        std::ostringstream cmd;
        cmd << "ERROR";
        if (message.size())
            cmd << ' ' << message;
        cmd << "\r\n";
        sendCommand(true, cmd.str(), std::forward<Handler>(handler));
        m_state = WaitingForData;
    }

    template<typename Handler>
    void sendCancel(Handler&& handler)
    {
        sendCommand(true, "CANCEL\r\n", std::forward<Handler>(handler));
        m_state = WaitingForReject;
    }

    template<typename Handler>
    void sendBegin(Handler&& handler)
    {
        sendCommand(false, "BEGIN\r\n", std::forward<Handler>(handler));
        m_state = Finishing;
    }

    template<typename Handler>
    void handleOk(
        const std::string& guid, Handler&& handler)
    {
        m_server_guid = guid;
        Log::write(Log::INFO, "DBus :: Auth OK : guid %s\n",
            m_server_guid.c_str());
        sendNegotiateUnixFD(std::forward<Handler>(handler));
    }

    template<typename Handler>
    void handleError(
        const std::string& message, Handler&& handler)
    {
        Log::write(Log::ERROR, "DBus :: Auth ERROR : %s\n",
            message.c_str());
        sendCancel(std::forward<Handler>(handler));
    }

    template<typename Handler>
    void handleRejected(
        const std::string& message, Handler&& handler)
    {
        Log::write(Log::WARNING, "DBus :: Auth REJECTED : %s\n",
            message.c_str());
        handler(make_error_code(errc::protocol_error), 0);
    }

    template<typename Handler>
    void handleAgreeUniXFD(Handler&& handler)
    {
        Log::write(Log::INFO, "DBus :: Auth AGREE_UNIX_FD\n");
        m_unixFdNegotiated = true;
        sendBegin(std::forward<Handler>(handler));
    }

    template<typename Handler>
    void handleData(const std::string& data, Handler&& handler)
    {
        Log::write(Log::INFO, "DBus :: Auth DATA %s\n", data.c_str());
        sendCancel(std::forward<Handler>(handler));
    }

    std::shared_ptr<Transport> m_transport;
    State m_state = Starting;
    std::string m_data;
    DynamicStringBuffer m_buffer = asio::dynamic_buffer(m_data);
    std::string m_server_guid;
    bool m_unixFdNegotiated = false;

    struct Statistics m_stats = {};
};

} // namespace DBus
