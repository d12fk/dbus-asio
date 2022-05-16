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
#include "dbus_error.h"
#include "dbus_message.h"
#include "dbus_messageostream.h"
#include "dbus_type_struct.h"
#include "dbus_transport.h"

#include <memory>
#include <map>

namespace DBus {

using MessageBuffer = std::vector<std::uint8_t>;

class MessageProtocol : public std::enable_shared_from_this<MessageProtocol> {
public:
    using Ptr = std::shared_ptr<MessageProtocol>;

    struct Statistics {
        std::size_t count_send_errors;
        std::size_t count_send_signals;
        std::size_t count_send_methodcalls;
        std::size_t count_send_methodreturns;
        std::size_t count_recv_errors;
        std::size_t count_recv_signals;
        std::size_t count_recv_methodcalls;
        std::size_t count_recv_methodreturns;
        std::size_t bytes_send;
        std::size_t bytes_recv;
    };

    static Ptr start(asio::io_context& ioContext, Transport::Ptr transport);

    void stop()
    {
        stopping = true;
        m_transport->disconnect();
    }

    template<typename CompletionToken>
    auto sendMethodCall(
        std::uint32_t serial,
        const Message::MethodCall& methodCall,
        CompletionToken&& token)
    {
        Log::write(Log::TRACE, "DBus :: Send : preparing METHOD_CALL message\n");
        auto replySerial = methodCall.isReplyExpected() ? serial : 0;

        return asio::async_initiate<
            CompletionToken, void(const Error&, const Message::MethodReturn&)>(
            [this](auto&& handler, MessageOStream&& data, uint32_t replySerial) mutable
            {
                ++m_stats.count_send_methodcalls;
                m_stats.bytes_send += data.size();
                auto payload = std::make_shared<MessageOStream>(std::move(data));

                // Store handler if there is a reply expected
                if (replySerial) {
                    storeMethodReturnHandler(replySerial, std::move(handler));
                    m_transport->asyncWrite(
                        payload,
                        [self = shared_from_this(), replySerial, payload]
                        (const error_code& error, std::size_t) mutable -> void
                        {
                            if (error)
                                self->invokeErrorHandler(replySerial, error);
                        });
                } else {
                    m_transport->asyncWrite(
                        payload,
                        [self = shared_from_this(), payload, handler = std::move(handler)]
                        (const error_code& error, std::size_t) mutable -> void
                        {
                            handler(error, Message::MethodReturn());
                        });
                }
            },
            token, methodCall.marshall(serial), replySerial);
    }

    template<typename CompletionToken>
    auto sendMethodReturn(
        std::uint32_t serial,
        const Message::MethodReturn& methodReturn,
        CompletionToken&& token)
    {
        Log::write(Log::TRACE, "DBus :: Send : preparing METHOD_RETURN message\n");
        return asio::async_initiate<
            CompletionToken, void(const Error&)>(
            [this](auto&& handler, MessageOStream&& data) {
                ++m_stats.count_send_methodreturns;
                m_stats.bytes_send += data.size();
                auto payload = std::make_shared<MessageOStream>(std::move(data));
                m_transport->asyncWrite(
                    payload,
                    [self = shared_from_this(), payload, handler = std::move(handler)]
                    (const error_code& error, std::size_t) mutable -> void
                    {
                        handler(error);
                    });
            }, token, methodReturn.marshall(serial));
    }

    template<typename CompletionToken>
    auto sendSignal(
        std::uint32_t serial,
        const Message::Signal& signal,
        CompletionToken&& token)
    {
        Log::write(Log::TRACE, "DBus :: Send : preparing SIGNAL message\n");
        return asio::async_initiate<
            CompletionToken, void(const Error&)>(
            [this](auto&& handler, MessageOStream&& data) {
                ++m_stats.count_send_signals;
                m_stats.bytes_send += data.size();
                auto payload = std::make_shared<MessageOStream>(std::move(data));
                m_transport->asyncWrite(
                    payload,
                    [self = shared_from_this(), payload, handler = std::move(handler)]
                    (const error_code& error, std::size_t) mutable -> void
                    {
                        handler(error);
                    });
            }, token, signal.marshall(serial));
    }

    template<typename CompletionToken>
    auto sendError(
        std::uint32_t serial,
        const Message::Error& error,
        CompletionToken&& token)
    {
        Log::write(Log::TRACE, "DBus :: Send : preparing ERROR message\n");
        return asio::async_initiate<
            CompletionToken, void(const Error&)>(
            [this](auto&& handler, MessageOStream&& data) {
                ++m_stats.count_send_errors;
                m_stats.bytes_send += data.size();
                auto payload = std::make_shared<MessageOStream>(std::move(data));
                m_transport->asyncWrite(
                    payload,
                    [self = shared_from_this(), payload, handler = std::move(handler)]
                    (const error_code& error, std::size_t) mutable -> void
                    {
                        handler(error);
                    });
            }, token, error.marshall(serial));
    }


    template<typename CompletionToken>
    auto receiveMethodCall(
        const std::string& interface,
        CompletionToken&& token)
    {
        return asio::async_initiate<
            CompletionToken, void(const Message::MethodCall&)>(
            [this](auto&& handler, const std::string& interface) {
                this->storeMethodCallHandler(interface, std::forward<decltype(handler)>(handler));
            }, token, interface);
    }

    template<typename CompletionToken>
    auto receiveSignal(
        const std::string& signal,
        CompletionToken&& token)
    {
        return asio::async_initiate<
            CompletionToken, void(const Message::Signal&)>(
            [this](auto&& handler, const std::string& signal) {
                this->storeSignalHandler(signal, std::forward<decltype(handler)>(handler));
            }, token, signal);
    }

    bool cancelReceiveSignal(const std::string& signal)
    {
        // Invoke handler for this exact signal
        auto it = m_signalHandlers.find(signal);
        if (it != m_signalHandlers.end()) {
            m_signalHandlers.erase(it);
            return true;
        }
        return false;
    }

    template<typename CompletionToken>
    auto receiveError(CompletionToken&& token)
    {
        return asio::async_initiate<
            CompletionToken, void(const Error&)>(
            [this](auto&& handler) {
                m_errorHandler = StoredErrorHandler::create(
                    m_ioContext, std::move(handler));
            }, token);
    }

    struct Statistics getStats() { return m_stats; }

protected:
    MessageProtocol(asio::io_context& ioContext, Transport::Ptr transport);

    template<typename Handler>
    bool storeMethodReturnHandler(uint32_t serial, Handler&& handler)
    {
        const bool ok = m_methodReturnHandlers.emplace(
            std::make_pair(
                serial,
                StoredMethodReturnHandler::create(
                    m_ioContext,
                    std::move(handler)))).second;
        if (!ok)
            Log::write(Log::ERROR, "DBus :: METHOD RETURN : could not store handler\n");
        return ok;
    }

    template<typename Handler>
    bool storeMethodCallHandler(const std::string& interface, Handler&& handler)
    {
        const bool ok = m_methodCallHandlers.emplace(
            std::make_pair(
                interface,
                StoredMethodCallHandler::create(
                    m_ioContext,
                    std::move(handler)))).second;
        if (!ok)
            Log::write(Log::ERROR, "DBus :: METHOD CALL : could not store handler\n");
        return ok;
    }

    template<typename Handler>
    bool storeSignalHandler(const std::string& signal, Handler&& handler)
    {
        const bool ok = m_signalHandlers.emplace(
            std::make_pair(
                signal,
                StoredSignalHandler::create(
                    m_ioContext,
                    std::move(handler)))).second;
        if (!ok)
            Log::write(Log::ERROR, "DBus :: SIGNAL : could not store handler\n");
        return ok;
    }

    void invokeMethodReturnHandler(
        uint32_t replySerial,
        const Message::MethodReturn& methodReturn)
    {
        const auto it = m_methodReturnHandlers.find(replySerial);
        if (it != m_methodReturnHandlers.end()) {
            it->second->invoke({}, methodReturn);
            m_methodReturnHandlers.erase(it);
            return;
        }

        Log::write(Log::WARNING, "DBus :: METHOD RETURN : unexpected : " \
            "reply to #%u\n", replySerial);
    }

    void invokeMethodCallHandler(
        const std::string& interface,
        const Message::MethodCall& methodCall)
    {
        // Invoke handler for this exact interface
        auto it = m_methodCallHandlers.find(interface);
        if (it != m_methodCallHandlers.end()) {
            it->second->invoke(methodCall);
            m_methodCallHandlers.erase(it);
            return;
        }

        // Invoke handler for any method of this interface
        const std::string any_method(interface, 0, interface.rfind('.'));
        it = m_methodCallHandlers.find(any_method);
        if (it != m_methodCallHandlers.end()) {
            it->second->invoke(methodCall);
            m_methodCallHandlers.erase(it);
            return;
        }

        // Invoke handler for any interface (catch all)
        it = m_methodCallHandlers.find("");
        if (it != m_methodCallHandlers.end()) {
            it->second->invoke(methodCall);
            m_methodCallHandlers.erase(it);
            return;
        }

        Log::write(Log::INFO, "DBus :: METHOD CALL : unhandled : %s\n",
            interface.c_str());
    }

    void invokeSignalHandler(
        const std::string& name,
        const Message::Signal& signal)
    {
        // Invoke handler for this exact signal
        auto it = m_signalHandlers.find(name);
        if (it != m_signalHandlers.end()) {
            it->second->invoke(signal);
            m_signalHandlers.erase(it);
            return;
        }

        // Invoke handler for any signal (catch all)
        it = m_signalHandlers.find("");
        if (it != m_signalHandlers.end()) {
            it->second->invoke(signal);
            m_signalHandlers.erase(it);
            return;
        }

        Log::write(Log::INFO, "DBus :: SIGNAL : %s unhandled\n",
            name.c_str());
    }

    void invokeErrorHandler(
        uint32_t replySerial,
        const Error& error)
    {
        if (replySerial) {
            const auto it = m_methodReturnHandlers.find(replySerial);
            if (it != m_methodReturnHandlers.end()) {
                it->second->invoke(error, {});
                m_methodReturnHandlers.erase(it);
                return;
            }
        }

        if (m_errorHandler) {
            m_errorHandler->invoke(error);
            m_errorHandler.reset();
            return;
        }

        Log::write(Log::WARNING, "DBus :: ERROR : unhandled : %s " \
            "(reply to #%u)\n", error.message.c_str(), replySerial);
    }

    void releasePendingHandlers()
    {
        for (auto& elem : m_methodReturnHandlers)
            elem.second->invoke({}, {});
        for (auto& elem : m_methodCallHandlers)
            elem.second->invoke({});
        for (auto& elem : m_signalHandlers)
            elem.second->invoke({});
        if (m_errorHandler)
            m_errorHandler->invoke({});
    }

    using StoredMethodReturnHandler = StoredToken<const Error&, const Message::MethodReturn&>;
    using StoredMethodCallHandler = StoredToken<const Message::MethodCall&>;
    using StoredSignalHandler = StoredToken<const Message::Signal&>;
    using StoredErrorHandler = StoredToken<const Error&>;

    std::map<uint32_t, StoredMethodReturnHandler::Ptr> m_methodReturnHandlers;
    std::map<std::string, StoredMethodCallHandler::Ptr> m_methodCallHandlers;
    std::map<std::string, StoredSignalHandler::Ptr> m_signalHandlers;
    StoredErrorHandler::Ptr m_errorHandler;

    using ReadBuffer = asio::mutable_buffer;

    enum class ReadState { Peek, Receive };
    ReadBuffer makeReadBuffer(ReadState state, MessageBuffer& buffer);

    void asyncReadMessage(ReadState state);
    void dispatchMessage(OctetBuffer& message);

    bool stopping = false;
    MessageBuffer m_messageBuffer;
    UnixFdBuffer m_unixFdBuffer;
    asio::io_context& m_ioContext;
    Transport::Ptr m_transport;

    struct Statistics m_stats = {};
};

} // namespace DBus

