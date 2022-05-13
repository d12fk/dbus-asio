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

#include "dbus_type_any.h"
#include "dbus_type_array.h"
#include "dbus_type_byte.h"
#include "dbus_type_signature.h"
#include "dbus_type_string.h"
#include "dbus_type_struct.h"
#include "dbus_type_uint32.h"
#include "dbus_type_variant.h"

#include "dbus_log.h"
#include "dbus_names.h"
#include "dbus_message.h"
#include "dbus_messageistream.h"
#include "dbus_messageostream.h"

#include <sstream>

//
// Message::Header::Fields
//
DBus::Message::Header::Fields::Fields(const Header& header)
    : DBus::Type::Array("a(yv)")
{
    // Check the required fields that could have been passed in empty
    if (header.type != Message::Type::Signal && header.destination.empty())
        throw InvalidMessage("Message without destination");
    if (header.type == Message::Type::Signal) {
        if (header.path.empty())
            throw InvalidMessage("Signal without ObjectPath");
        if (header.interface.empty())
            throw InvalidMessage("Signal without Interface");
    }
    else if (header.type == Message::Type::MethodCall && header.path.empty())
        throw InvalidMessage("MethodCall without ObjectPath");

    if (!header.destination.empty())
        add(Header::Field::Destination, header.destination);
    if (!header.path.empty())
        add(Header::Field::Path, DBus::Type::ObjectPath(header.path));
    if (!header.interface.empty())
        add(Header::Field::Interface, header.interface);
    if (!header.member.empty())
        add(Header::Field::Member, header.member);
    if (!header.errorName.empty())
        add(Header::Field::ErrorName, header.errorName);
    if (header.replySerial != 0)
        add(Header::Field::ReplySerial, header.replySerial);
    if (!header.sender.empty())
        add(Header::Field::Sender, header.sender);
    if (!header.signature.empty())
        add(Header::Field::Signature, header.signature);
    if (header.unixFds > 0)
        add(Header::Field::UnixFds, header.unixFds);
}

std::size_t
DBus::Message::Header::Fields::add(Field type, const DBus::Type::Any& value)
{
    DBus::Type::Struct field;
    field.add(DBus::Type::Byte(type));
    field.add(DBus::Type::Variant(value));
    return DBus::Type::Array::add(field);
}



//
// Message::Header
//
DBus::Message::Header::Header(OctetBuffer& message)
{
    size = getSize(message);
    endianness = Message::endianness(message);
    MessageIStream istream(message, swapRequired(endianness));

    DBus::Type::Struct header("(yyyyuua(yv))");
    header.unmarshall(istream);

    // Capture the basic parameters
    type = static_cast<Message::Type>(DBus::Type::asByte(header[1]));
    flags = DBus::Type::asByte(header[2]);
    bodySize = DBus::Type::asUint32(header[4]);
    serial = DBus::Type::asUint32(header[5]);

    for (auto& elem : DBus::Type::refArray(header[6])) {
        const DBus::Type::Struct& field = DBus::Type::refStruct(elem);
        Header::Field name = static_cast<Header::Field>(DBus::Type::asByte(field[0]));
        switch (name) {
        case Header::Field::Path:
            path = DBus::Type::asString(field[1]);
            break;
        case Header::Field::Interface:
            interface = DBus::Type::asString(field[1]);
            break;
        case Header::Field::Member:
            member = DBus::Type::asString(field[1]);
            break;
        case Header::Field::ErrorName:
            errorName = DBus::Type::asString(field[1]);
            break;
        case Header::Field::ReplySerial:
            replySerial = DBus::Type::asUint32(field[1]);
            break;
        case Header::Field::Destination:
            destination = DBus::Type::asString(field[1]);
            break;
        case Header::Field::Sender:
            sender = DBus::Type::asString(field[1]);
            break;
        case Header::Field::Signature:
            signature = DBus::Type::refSignature(field[1]);
            break;
        case Header::Field::UnixFds:
            unixFds = DBus::Type::asUint32(field[1]);
            break;
        default:
            break;
        }
    }

    message.remove_prefix(size);
}

void DBus::Message::Header::marshall(MessageOStream& stream)
{
    //  The signature of the header is:
    //      "yyyyuua(yv)"
    //
    //  Written out more readably, this is:
    //      BYTE, BYTE, BYTE, BYTE, UINT32, UINT32, ARRAY of STRUCT of (BYTE,VARIANT)

    // 1st BYTE
    // Endianness flag; ASCII 'l' for little-endian or ASCII 'B' for big-endian.
    // Both header and body are in this endianness.
    stream.writeByte(__BYTE_ORDER == __LITTLE_ENDIAN ? 'l' : 'B');

    // 2nd BYTE
    // Message type. Unknown types must be ignored. Currently-defined types are
    // described below.
    stream.writeByte(static_cast<uint8_t>(type));

    // 3rd BYTE
    // Bitwise OR of flags. Unknown flags must be ignored. Currently-defined flags
    // are described below.
    stream.writeByte(flags);

    // 4th BYTE
    // Major protocol version of the sending application. If the major protocol
    // version of the receiving application does not match, the applications will
    // not be able to communicate and the D-Bus connection must be disconnected.
    // The major protocol version for this version of the specification is 1.
    stream.writeByte(1);

    // 1st UINT32
    // Length in bytes of the message body, starting from the end of the header.
    // The header ends after its alignment padding to an 8-boundary.
    stream.writeUint32(bodySize);

    // 2nd UINT32
    // The serial of this message, used as a cookie by the sender to identify the
    // reply corresponding to this request. This must not be zero.
    stream.writeUint32(serial);

    // ARRAY of STRUCT of(BYTE,VARIANT) i.e. the header fields
    Fields headerFields(*this);
    headerFields.marshall(stream);

    // Both header & header_fields constitute the header, which must end of an
    // 8 byte boundary. (See the phrase: "The header ends after its alignment padding to
    // an 8-boundary.") Therefore we add the padding here.
    stream.pad8();
}


std::string
DBus::Message::Header::toString(const std::string& prefix) const
{
    std::ostringstream oss;

    oss << prefix << "Endianness: " << (endianness == Endian::Big ? "big" : "little") << '\n';
    oss << prefix << "Type: " << typeString(type) << '\n';
    oss << prefix << "Flags: " << flagString(flags) << '\n';
    oss << prefix << "Version: 1" << '\n';
    oss << prefix << "Body length: " << bodySize << '\n';
    oss << prefix << "Serial: #" << serial << '\n';

    if (replySerial)
        oss << prefix << "Reply Serial: #" << replySerial << '\n';
    if (!sender.empty())
        oss << prefix << "Sender: " << sender << '\n';
    if (!destination.empty())
        oss << prefix << "Destination: " << destination << '\n';
    if (!path.empty())
        oss << prefix << "Path: " << path << '\n';
    if (!interface.empty())
        oss << prefix << "Interface: " << interface << '\n';
    if (!member.empty())
        oss << prefix << "Member: " << member << '\n';
    if (!errorName.empty())
        oss << prefix << "Error Name: " << errorName << '\n';
    if (!signature.empty())
        oss << prefix << "Signature: " << signature.asString() << '\n';
    if (unixFds)
        oss << prefix << "Unix FDs: " << unixFds << '\n';

    return oss.str();
}

std::size_t
DBus::Message::Header::getSize(const OctetBuffer& message)
{
    std::size_t size = Message::Header::MinimumSize;
    if (message.size() < size)
        return 0; // TODO: throw instead?

    // Add size of the header fields array
    const Endian endianness = Message::endianness(message);
    size += correctEndianess(endianness, *(uint32_t*)(message.data() + 12));
    // Add padding to an 8 byte boundary
    size += (size % 8 == 0) ? 0 : 8 - (size % 8);

    if (size > Message::Header::MaximumSize)
        throw std::out_of_range("DBus message error: maximum header size exceeded");

    return size;
}

std::size_t
DBus::Message::Header::getMessageSize(const OctetBuffer& message)
{
    if (message.size() < Message::Header::MinimumSize)
        return 0; // TODO: throw instead?

    const Endian endianness = Message::endianness(message);
    std::size_t body_size = correctEndianess(endianness, *(uint32_t*)(message.data() + 4));
    std::size_t msg_size = getSize(message) + body_size;

    if (msg_size > Message::MaximumSize)
        throw std::out_of_range("DBus message error: maximum message size exceeded");

    return msg_size;
}



//
// Message::Identifier
//
DBus::Message::Identifier::Identifier(const ObjectPath& path, const InterfaceName& interface,
    const MemberName& member)
    : path(path)
    , interface(interface)
    , member(member)
{}



//
// Message::Parameters
//
std::string
DBus::Message::Parameters::getSignature() const
{
    std::string signature;
    for (const auto& type : m_parameters) {
        signature += type.getSignature();
    }
    return signature;
}

void DBus::Message::Parameters::marshall(
    MessageOStream& stream) const
{
    for (const auto& type : m_parameters) {
        type.marshall(stream);
    }
}

size_t DBus::Message::Parameters::getParameterCount() const
{
    return m_parameters.size();
}

const DBus::Type::Any&
DBus::Message::Parameters::getParameter(size_t idx) const
{
    return m_parameters[idx];
}

std::string
DBus::Message::Parameters::toString(const std::string& prefix) const
{
    std::ostringstream oss;
    for (const auto& param : m_parameters) {
        oss << prefix << param.toString(prefix);
    }
    return oss.str();
}

DBus::Message::Parameters::Parameters(
    const DBus::Type::Any& v)
    : m_parameters({v})
{}

DBus::Message::Parameters::Parameters(
    const DBus::Type::Any& v1, const DBus::Type::Any& v2)
    : m_parameters({v1, v2})
{}

void DBus::Message::Parameters::add(const DBus::Type::Any& value)
{
    m_parameters.push_back(value);
}



//
// Message
//
DBus::Message::Message(const Header& header, OctetBuffer& body)
    : m_Header(header)
{
    // Empty bodies have no parameters
    if (body.size()) {
        unmarshallBody(header.endianness, header.signature, body);
    }
    if (Log::isActive(Log::TRACE)) {
        std::string prefix = "    ";
        Log::write(Log::TRACE, "DBus :: Message : Header :\n%s",
            header.toString(prefix).c_str());
        if (m_Header.bodySize)
            Log::write(Log::TRACE, "DBus :: Message : Data :\n%s\n",
                m_Parameters.toString(prefix).c_str());
    }
}

void DBus::Message::unmarshallBody(
    Endian endianness,
    const DBus::Type::Signature& signature,
    OctetBuffer& body)
{
    MessageIStream stream(body, swapRequired(endianness));
    DBus::Type::Struct parameter_fields(signature);
    parameter_fields.unmarshall(stream);

    size_t count = parameter_fields.size();
    for (size_t i = 0; i < count; ++i) {
        m_Parameters.add(parameter_fields[i]);
    }
}

DBus::MessageOStream
DBus::Message::marshall(std::uint32_t serial) const
{
    MessageOStream body;
    m_Parameters.marshall(body);

    Header header = m_Header;
    MessageOStream headerData;
    header.serial = serial;
    header.signature = m_Parameters.getSignature();
    header.bodySize = body.data.size();
    header.unixFds = body.fds.size();
    header.marshall(headerData);

    MessageOStream packet;
    packet.write(headerData);
    packet.write(body);

    if (Log::isActive(Log::TRACE)) {
        std::string prefix = "    ";
        Log::write(Log::TRACE, "DBus :: Message : Header :\n%s",
            header.toString(prefix).c_str());
        if (header.bodySize)
            Log::write(Log::TRACE, "DBus :: Message : Data :\n%s",
                m_Parameters.toString(prefix).c_str());
    }

    return packet;
}



//
// Message::MethodCall
//
DBus::Message::MethodCall::MethodCall(const Message::Header& header,
    OctetBuffer& body)
    : Message(header, body)
{}

DBus::Message::MethodCall::MethodCall(
    const BusName& destination,
    const Identifier& name,
    const Parameters& params,
    uint32_t flags)
{

    if (flags & Message::Flag::AllowInteractiveAuthorization) {
        DBus::Log::write(Log::ERROR,
            "DBus :: ALLOW_INTERACTIVE_AUTHORIZATION is not supported.");
        flags &= ~Message::Flag::AllowInteractiveAuthorization;
    }
    // Ignore any extraneous flags, also.
    flags &= Message::Flag::_Mask;

    m_Header.flags = flags;
    m_Header.type = Message::Type::MethodCall;
    m_Header.destination = destination;
    m_Header.path = name.path;
    m_Header.interface = name.interface;
    m_Header.member = name.member;
    m_Parameters = params;
}



//
// Message::MethodReturn
//
DBus::Message::MethodReturn::MethodReturn(
    const BusName& destination,
    uint32_t replySerial)
{
    m_Header.flags = Flag::NoReplyExpected;
    m_Header.type = Message::Type::MethodReturn;
    m_Header.destination = destination;
    m_Header.replySerial = replySerial;
}

DBus::Message::MethodReturn::MethodReturn(const Message::Header& header,
    OctetBuffer& body)
    : Message(header, body)
{}



//
// Message::Error
//
DBus::Message::Error::Error(
    const BusName& destination,
    uint32_t replySerial,
    const ErrorName& name,
    const std::string& message)
{
    m_Header.flags = Flag::NoReplyExpected;
    m_Header.type = Message::Type::Error;
    m_Header.destination = destination;
    m_Header.replySerial = replySerial;
    m_Header.errorName = name;
    m_Parameters.add(message);
}

DBus::Message::Error::Error(const DBus::Message::Header& header,
    OctetBuffer& body)
    : Message(header, body)
{
    Log::write(Log::WARNING,
        "DBus :: Error : received as reply to message #%d : %s\n",
        getReplySerial(), getMessage().c_str());
}

std::string DBus::Message::Error::getName() const
{
    return getErrorName();
}

std::string DBus::Message::Error::getMessage() const
{
    if (m_Parameters.getParameterCount() == 0)
        return "";
    return DBus::Type::asString(m_Parameters.getParameter(0));
}



//
// Message::Signal
//
DBus::Message::Signal::Signal(
    const Message::Identifier& name)
{
    m_Header.flags = Flag::NoReplyExpected;
    m_Header.type = Message::Type::Signal;
    m_Header.path = name.path;
    m_Header.interface = name.interface;
    m_Header.member = name.member;
}

DBus::Message::Signal::Signal(
    const BusName& destination,
    const Message::Identifier& name)
    : Signal(name)
{
    m_Header.destination = destination;
}

DBus::Message::Signal::Signal(const Message::Header& header,
    OctetBuffer& body)
    : Message(header, body)
{}


