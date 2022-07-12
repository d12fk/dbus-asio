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

#include "dbus_asio.h"
#include "dbus_auth.h"
#include "dbus_messageprotocol.h"
#include "dbus_transport.h"

#include <memory>
#include <type_traits>

namespace DBus {

static constexpr char const *Name = "org.freedesktop.DBus";
static constexpr char const *Object = "/org/freedesktop/DBus";
static constexpr char const *Interface = "org.freedesktop.DBus";

enum RequestNameFlag {
    None = 0,
    AllowReplacement = 1,
    ReplaceExisting = 2,
    DoNotQueue = 4
};

enum RequestNameReply {
    PrimaryOwner = 1,
    InQueue = 2,
    Exists = 3,
    AlreadyOwner = 4
};

enum ReleaseNameReply {
    Released = 1,
    NonExistent = 2,
    NotOwner = 3
};

class Connection : public std::enable_shared_from_this<Connection> {
public:
    using Ptr = std::shared_ptr<Connection>;

    static Ptr create(asio::io_context& ioContext)
    {
        struct ShareableConnection : public Connection {
            ShareableConnection(asio::io_context& ioContext)
                : Connection(ioContext) {};
        };
        return std::make_shared<ShareableConnection>(ioContext);
    }

    template<typename CompletionToken>
    auto connectSystemBus(CompletionToken&& token)
    {
        return connect(Platform::getSystemBus(), std::move(token));
    }

    template<typename CompletionToken>
    auto connectSessionBus(CompletionToken&& token)
    {
        return connect(Platform::getSessionBus(), std::move(token));
    }

    template<typename CompletionToken>
    auto connect(
        const std::string& busPath,
        CompletionToken&& token)
    {
        return connect(
            busPath,
            AuthenticationProtocol::create(),
            std::move(token));
    }

    template<typename CompletionToken>
    auto connect(
        const std::string& busPath,
        AuthenticationProtocol::Ptr authProto,
        CompletionToken&& token)
    {
        enum { starting, connecting, authenticating, requestingName };
        return asio::async_compose<
            CompletionToken, void(const Error&, const std::string&, const std::string&)>(
            [self = shared_from_this(), state = starting, &busPath, authProto]
            (auto& composition, const Error& error = {}, const std::string& data = {}) mutable
            {
                switch (state) {
                case starting:
                    state = connecting;
                    self->m_transport->asyncConnect(busPath, std::move(composition));
                    return;

                case connecting:
                    if (error)
                        break;
                    state = authenticating;
                    authProto->asyncAuthenticate(self->m_transport, std::move(composition));
                    return;

                case authenticating:
                    if (error)
                        break;
                    state = requestingName;
                    self->m_authStats = authProto->getStats();
                    self->m_serverGuid = data;
                    self->m_msgProto = MessageProtocol::start(self->m_ioContext, self->m_transport);
                    self->hello(std::move(composition));
                    return;

                case requestingName:
                    self->m_uniqueName = data;
                    break;
                }
                composition.complete(error, self->m_uniqueName, self->m_serverGuid);
            }, token);
    }

    void disconnect()
    {
        m_nextSerial = 1;
        m_msgProto->stop();
    }

    bool connected()
    {
        return m_transport->connected();
    }

    template<typename CompletionToken>
    auto sendMethodCall(
        const Message::MethodCall& methodCall,
        CompletionToken&& token)
    {
        return m_msgProto->sendMethodCall(
            m_nextSerial++, methodCall, std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto sendMethodReturn(
        const Message::MethodReturn& methodReturn,
        CompletionToken&& token)
    {
        return m_msgProto->sendMethodReturn(
            m_nextSerial++, methodReturn, std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto sendSignal(
        const Message::Signal& signal,
        CompletionToken&& token)
    {
        return m_msgProto->sendSignal(
            m_nextSerial++, signal, std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto sendError(
        const Message::Error& error,
        CompletionToken&& token)
    {
        return m_msgProto->sendError(
            m_nextSerial++, error, std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto receiveMethodCall(
        const std::string& interface,
        CompletionToken&& token)
    {
        return m_msgProto->receiveMethodCall(
            interface, std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto receiveSignal(
        const std::string& signal,
        CompletionToken&& token)
    {
        return m_msgProto->receiveSignal(
            signal, std::forward<CompletionToken>(token));
    }

    auto cancelReceiveSignal(const std::string& signal)
    {
        return m_msgProto->cancelReceiveSignal(signal);
    }

    template<typename CompletionToken>
    auto receiveError(
        CompletionToken&& token)
    {
        return m_msgProto->receiveError(
            std::forward<CompletionToken>(token));
    }


    //////////////////////////////////////////////
    // Methods for the standard DBus interfaces //
    //////////////////////////////////////////////

    template<typename CompletionToken>
    auto getAllProperties(
        const BusName& destination,
        const ObjectPath& path,
        const InterfaceName& interface,
        CompletionToken&& token)
    {
        return sendMethodCall({ destination,
            {path, "org.freedesktop.DBus.Properties", "GetAll"}, {interface} },
            std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto getProperty(
        const BusName& destination,
        const ObjectPath& path,
        const InterfaceName& interface,
        const MemberName& property,
        CompletionToken&& token)
    {
        return sendMethodCall({ destination,
            {path, "org.freedesktop.DBus.Properties", "Get"}, {interface, property} },
            std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto setProperty(
        const BusName& destination,
        const ObjectPath& path,
        const InterfaceName& interface,
        const MemberName& property,
        CompletionToken&& token)
    {
        return sendMethodCall({ destination,
            {path, "org.freedesktop.DBus.Properties", "Set"}, {interface, property} },
            std::forward<CompletionToken>(token));
    }


    //////////////////////////////////////////
    // Methods for the Message Bus Messages //
    //////////////////////////////////////////

    template<typename CompletionToken>
    auto requestName(
        const WellKnownName& name,
        std::uint32_t flags,
        CompletionToken&& token)
    {
        return sendMethodCall({ DBus::Name,
            {DBus::Object, DBus::Interface, "RequestName"}, {name, flags} },
            std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto releaseName(
        const WellKnownName& name,
        CompletionToken&& token)
    {
        return sendMethodCall({ DBus::Name,
            {DBus::Object, DBus::Interface, "ReleaseName"}, {name} },
            std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto listQueuedOwners(
        const WellKnownName& name,
        CompletionToken&& token)
    {
        return sendMethodCall({ DBus::Name,
            {DBus::Object, DBus::Interface, "ListQueuedOwners"}, {name} },
            std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto listNames(CompletionToken&& token)
    {
        return sendMethodCall({ DBus::Name,
            {DBus::Object, DBus::Interface, "ListNames"} },
            std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto listActivatableNames(CompletionToken&& token)
    {
        return sendMethodCall({ DBus::Name,
            {DBus::Object, DBus::Interface, "ListActivatableNames"} },
            std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto nameHasOwner(
        const WellKnownName& name,
        CompletionToken&& token)
    {
        return sendMethodCall({ DBus::Name,
            {DBus::Object, DBus::Interface, "NameHasOwner"}, {name} },
            std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto getNameOwner(
        const WellKnownName& name,
        CompletionToken&& token)
    {
        return sendMethodCall({ DBus::Name,
            {DBus::Object, DBus::Interface, "GetNameOwner"}, {name} },
            std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto getConnectionUnixUser(
        const BusName& name,
        CompletionToken&& token)
    {
        return sendMethodCall({ DBus::Name,
            {DBus::Object, DBus::Interface, "GetConnectionUnixUser"}, {name} },
            std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto getConnectionUnixProcessID(
        const BusName& name,
        CompletionToken&& token)
    {
        return sendMethodCall({ DBus::Name,
            {DBus::Object, DBus::Interface, "GetConnectionUnixProcessID"}, {name} },
            std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto addMatch(const MatchRule& rule, CompletionToken&& token)
    {
        return sendMethodCall({ DBus::Name,
            {DBus::Object, DBus::Interface, "AddMatch"}, {rule.str()} },
            std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto removeMatch(const MatchRule& rule, CompletionToken&& token)
    {
        return sendMethodCall({ DBus::Name,
            {DBus::Object, DBus::Interface, "RemoveMatch"}, {rule.str()} },
            std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto getId(CompletionToken&& token)
    {
        return sendMethodCall({ DBus::Name,
            {DBus::Object, DBus::Interface, "GetId"} },
            std::forward<CompletionToken>(token));
    }

    template<typename CompletionToken>
    auto becomeMonitor(
        const std::vector<MatchRule>& rules,
        CompletionToken&& token)
    {
        DBus::Type::Array rulesArray("as");
        for (const auto& rule : rules)
            rulesArray.add(rule.str());

        return sendMethodCall({DBus::Name,
            {DBus::Object, DBus::Interface, "BecomeMonitor"}, {rulesArray, Type::Uint32(0)} },
            std::forward<CompletionToken>(token));
    }

    std::string getStats() const
    {
        std::ostringstream stats;
        MessageProtocol::Statistics msgStats = m_msgProto->getStats();

        stats << "Connection stats:" << '\n';
        stats << " count_send_auth_commands: " << m_authStats.count_send << '\n';
        stats << " count_receive_auth_commands: " << m_authStats.count_recv << '\n';
        stats << " count_send_methodcalls: " << msgStats.count_send_methodcalls << '\n';
        stats << " count_receive_methodcalls: " << msgStats.count_recv_methodcalls << '\n';
        stats << " count_send_methodreturns: " << msgStats.count_send_methodreturns << '\n';
        stats << " count_receive_methodreturns: " << msgStats.count_recv_methodreturns << '\n';
        stats << " count_send_errors: " << msgStats.count_send_errors << '\n';
        stats << " count_receive_errors: " << msgStats.count_recv_errors << '\n';
        stats << " count_send_signals: " << msgStats.count_send_signals << '\n';
        stats << " count_receive_signals: " << msgStats.count_recv_signals << '\n';
        stats << " bytes_send_auth: " << m_authStats.bytes_send << '\n';
        stats << " bytes_receive_auth: " << m_authStats.bytes_recv << '\n';
        stats << " bytes_send_message: " << msgStats.bytes_send << '\n';
        stats << " bytes_receive_message: " << msgStats.bytes_recv << '\n';

        return stats.str();
    }

protected:
    Connection(asio::io_context& ioContext)
        : m_ioContext(ioContext)
        , m_transport(Transport::create(ioContext))
    {}

    template<typename CompletionToken>
    auto hello(CompletionToken&& token)
    {
         return asio::async_initiate<
            CompletionToken, void(const Error&, const std::string&)>(
            [this](auto&& handler) mutable
            {
                sendMethodCall({ DBus::Name, {DBus::Object, DBus::Interface, "Hello"} },
                        [self = shared_from_this(), handler = std::forward<decltype(handler)>(handler)]
                        (const Error& error, const Message::MethodReturn& methodReturn) mutable
                        {
                            if (error)
                                handler(error, "");
                            else
                                handler(error, DBus::Type::asString(methodReturn.getParameter(0)));
                        });
            }, token);
    }

    struct AuthenticationProtocol::Statistics m_authStats = {};

    asio::io_context& m_ioContext;
    Transport::Ptr m_transport;
    MessageProtocol::Ptr m_msgProto;

    std::string m_serverGuid;
    std::string m_uniqueName;

    std::uint32_t m_nextSerial = 1;
};

}
