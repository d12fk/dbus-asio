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

#include "dbus_type_array.h"
#include "dbus_octetbuffer.h"
#include <functional>
#include <sstream>

#include <byteswap.h>

namespace DBus {

struct BusName;
struct ErrorName;
struct ObjectPath;
struct MemberName;
struct InterfaceName;
class MessageOStream;

using InvalidMessage = std::runtime_error;

class Message {
public:
    static constexpr std::size_t MaximumSize = 134217728; /*128 MiB*/

    enum class Type {
        Invalid = 0,
        MethodCall = 1,
        MethodReturn = 2,
        Error = 3,
        Signal = 4
    };

    static std::string typeString(Type t) {
        switch (t) {
            case Type::MethodCall:   return "MethodCall";
            case Type::MethodReturn: return "MethodReturn";
            case Type::Error:        return "Error";
            case Type::Signal:       return "Signal";
            default:                 return "INVALID";
        }
    }

    class MethodCall;
    class MethodReturn;
    class Error;
    class Signal;

    enum Flag {
        None = 0,
        NoReplyExpected = 0x01,
        NoAutoStart = 0x02,
        AllowInteractiveAuthorization = 0x04,
        _Mask = 0x07,
    };

    static std::string flagString(uint8_t flags) {
        if (flags == 0)
            return "None";
        std::ostringstream oss;
        if (flags & NoReplyExpected)
            oss << "NO_REPLY_EXPECTED ";
        if (flags & NoAutoStart)
            oss << "NO_AUTO_START ";
        if (flags & AllowInteractiveAuthorization)
            oss << "ALLOW_INTERACTIVE_AUTHORIZATION ";
        return oss.str();
    }

    enum class Endian { Little, Big };

    inline static Endian endianness(const OctetBuffer& message)
    {
        if (message[0] == 'l')
            return Endian::Little;
        else if (message[0] == 'B')
            return Endian::Big;
        else
            throw std::runtime_error("message endian marker invalid");
    }

    inline static bool swapRequired(const Endian endian)
    {
        return (endian == Endian::Little && __BYTE_ORDER != __LITTLE_ENDIAN)
            || (endian == Endian::Big    && __BYTE_ORDER != __BIG_ENDIAN);
    }

    inline static uint32_t correctEndianess(const Endian endian, uint32_t value)
    {
        return swapRequired(endian) ? bswap_32(value): value;
    }

    struct Header {
        // Initial header consists of byte, byte, byte, byte, uint32_t, uint32_t
        // the next element is the size of the array of field info data
        // making a total of 16 bytes
        static constexpr std::size_t MinimumSize = 16;
        static constexpr std::size_t MaximumSize = MinimumSize + DBus::Type::Array::MaximumSize;

        static std::size_t getSize(const OctetBuffer& message);
        static std::size_t getMessageSize(const OctetBuffer& message);

        enum Field {
            Invalid = 0,
            // The object to send a call to, or the object a signal is emitted from.
            Path = 1,
            // The interface to invoke a method call on, or that a signal is emitted from.
            Interface = 2,
            // The member, either the method name or signal name.
            Member = 3,
            // The name of the error that occurred, for errors.
            ErrorName = 4,
            // The serial number of the message this message is a reply to.
            ReplySerial = 5,
            // The name of the connection this message is intended for.
            Destination = 6,
            // Unique name of the sending connection.
            Sender = 7,
            // The signature of the message body.
            Signature = 8,
            // The number of Unix file descriptors that accompany the message.
            UnixFds = 9,
        };

        struct Fields : public DBus::Type::Array
        {
            Fields(const Header& header);
            std::size_t add(Field type, const DBus::Type::Any& value);
        };

        Header() = default;
        Header(OctetBuffer& buffer);

        void marshall(MessageOStream& stream);

        std::string getFullName() const {
            return interface + '.' + member;
        }

        std::string toString(const std::string& prefix = "") const;

        std::size_t size = 0;

        Endian endianness;
        Type type = Type::Invalid;
        uint8_t flags = 0;
        uint32_t bodySize = 0;
        uint32_t serial = 0;

        // Header fields
        std::string path;
        std::string interface;
        std::string member;
        std::string errorName;
        uint32_t replySerial = 0;
        std::string destination;
        DBus::Type::Signature signature;
        std::string sender;
        uint32_t unixFds = 0;
    };

    // Tuple of PATH, INTERFACE and MEMBER for MethodCall and Signal messages
    struct Identifier {
        Identifier(const ObjectPath& path, const InterfaceName& interface,
            const MemberName& member);

        std::string path;
        std::string interface;
        std::string member;
    };

    class Parameters {
    public:
        // We have a series of ctor's to permit the basic prototypes to be declared
        // inline, with Parameters("param") etc. Everyone else will need
        // to use a temporary variable and add()
        Parameters() = default;
        Parameters(Parameters&&) = default;
        Parameters(const Parameters&) = default;

        template<typename... Param>
        Parameters(const Param&... param)
            : m_parameters({ DBus::Type::Any(param)... })
        {}

        Parameters& operator=(Parameters&&) = default;
        Parameters& operator=(const Parameters&) = default;

        void add(const DBus::Type::Any& value);

        size_t getParameterCount() const;
        const DBus::Type::Any& getParameter(size_t idx) const;

        std::string getSignature() const;
        void marshall(MessageOStream& stream) const;

        std::string toString(const std::string& prefix = "") const;

    protected:
        std::vector<DBus::Type::Any> m_parameters;
    };

    Message() = default;
    Message(const Header& header, OctetBuffer& body);

    explicit operator bool() const { return m_Header.type != Type::Invalid; }

    // Getters for header information
    uint32_t getSerial() const { return m_Header.serial; }
    uint32_t getReplySerial() const { return m_Header.replySerial; }
    uint32_t getUnixFdCount() const { return m_Header.unixFds; }
    std::string getPath() const { return m_Header.path; }
    std::string getMember() const { return m_Header.member; }
    std::string getSender() const { return m_Header.sender; }
    std::string getSignature() const { return m_Header.signature.asString(); }
    std::string getInterface() const { return m_Header.interface; }
    std::string getErrorName() const { return m_Header.errorName; }
    std::string getDestination() const { return m_Header.destination; }

    void addParameter(const DBus::Type::Any& value)
    {
        m_Parameters.add(value);
    }

    const DBus::Type::Any& getParameter(size_t idx) const
    {
        return m_Parameters.getParameter(idx);
    }

    size_t getParameterCount() const
    {
        return m_Parameters.getParameterCount();
    }

    bool isReplyExpected() const
    {
        return (m_Header.flags & Flag::NoReplyExpected) == 0;
    }

    MessageOStream marshall(std::uint32_t serial) const;

protected:
    void unmarshallBody(
        Endian endianness,
        const DBus::Type::Signature& signature,
        OctetBuffer& bodydata);

    Header m_Header;
    Parameters m_Parameters;
};

class Message::MethodCall : public Message {
public:
    MethodCall() = default;
    // This is for outgoing method calls
    MethodCall(
        const BusName& destination, const Identifier& name,
        const Parameters& params = Parameters(), uint32_t flags = 0);
    // This is for receiving method calls
    MethodCall(const Header& header, OctetBuffer& body);

    inline std::string getFullName() const { return m_Header.getFullName(); }
    inline std::string getObject() const { return getPath(); }
    inline std::string getMethod() const { return getMember(); }
};

class Message::MethodReturn : public Message {
public:
    MethodReturn() = default;
    // This is for sending outgoing replies
    MethodReturn(const BusName& destination, uint32_t replySerial);
    // This is for receiving method returns
    MethodReturn(const Header& header, OctetBuffer& body);
};

class Message::Error : public Message {
public:
    Error() = default;
    // This is for outgoing Errors
    Error(const BusName & destination, uint32_t replySerial,
          const ErrorName& name, const std::string& message);
    // This is for receiving Errors
    Error(const Header& header, OctetBuffer& body);

    std::string getName() const;
    std::string getMessage() const;
};

class Message::Signal : public Message {
public:
    Signal() = default;
    // This is for outgoing broadcast signals
    Signal(const Identifier& name);
    // This is for outgoing unicast signals
    Signal(const BusName& destination, const Identifier& name);
    // This is for receiving signals
    Signal(const Header& header, OctetBuffer& body);
};


} // namespace DBus
