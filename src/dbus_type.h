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

#include <string>
#include <vector>
#include <functional>

namespace DBus {

    class MessageIStream;
    class MessageOStream;

    class Type {
    public:
        class Any;
        class Basic;
        class Container;

        class Int16;
        class Int32;
        class Int64;

        class Uint16;
        class Uint32;
        class Uint64;

        class Byte;
        class Boolean;
        class Double;

        class String;
        class ObjectPath;
        class Signature;

        class Array;
        class Struct;
        class DictEntry;
        class Variant;

        // Virtual methods subclasses implement
        virtual ~Type() = default;
        virtual std::string getName() const = 0;
        virtual std::string getSignature() const = 0;
        virtual std::size_t getAlignment() const = 0;

        virtual void marshall(MessageOStream&) const = 0;
        virtual void unmarshall(MessageIStream&) = 0;

        // each non-container type should end with \n
        // as convenience to the calling function
        virtual std::string toString(const std::string& /*prefix*/) const = 0;
        virtual std::string asString() const = 0;

        // Helper methods to return native types
        static bool asBoolean(const Any& v);
        static double asDouble(const Any& v);
        static std::string asString(const Any& v);
        static std::uint8_t asByte(const Any& v);
        static std::int16_t asInt16(const Any& v);
        static std::int32_t asInt32(const Any& v);
        static std::int64_t asInt64(const Any& v);
        static std::uint16_t asUint16(const Any& v);
        static std::uint32_t asUint32(const Any& v);
        static std::uint64_t asUint64(const Any& v);

        // Helper methods to return type-safe DBus references
        static const Array& refArray(const Any& v);
        static const Struct& refStruct(const Any& v);
        static const Variant& refVariant(const Any& v);
        static const Signature& refSignature(const Any& v);
        static const DictEntry& refDictEntry(const Any& v);

        // Helper Methods for during marshalling
        static DBus::Type::Any create(const std::string& type);
        static std::size_t getAlignment(const std::string& typeCode);
        static bool isBasicTypeCode(const int code);
    };

    class Type::Basic : public Type {};

    class Type::Container : public Type {};

} // namespace DBus
