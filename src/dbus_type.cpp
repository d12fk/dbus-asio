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
#include "dbus_messageistream.h"
#include "dbus_messageostream.h"
#include "dbus_type_any.h"
#include "dbus_type_array.h"
#include "dbus_type_boolean.h"
#include "dbus_type_byte.h"
#include "dbus_type_dictentry.h"
#include "dbus_type_double.h"
#include "dbus_type_int16.h"
#include "dbus_type_int32.h"
#include "dbus_type_int64.h"
#include "dbus_type_objectpath.h"
#include "dbus_type_signature.h"
#include "dbus_type_string.h"
#include "dbus_type_struct.h"
#include "dbus_type_uint16.h"
#include "dbus_type_uint32.h"
#include "dbus_type_uint64.h"
#include "dbus_type_variant.h"

//
// Helper methods to extract native types from the opaque 'Any' type
//

bool DBus::Type::asBoolean(const Any& v)
{
    return static_cast<const Boolean&>(v);
}

double DBus::Type::asDouble(const Any& v)
{
    return static_cast<const Double&>(v);
}

std::string DBus::Type::asString(const Any& v)
{
    return static_cast<const String&>(v);
}

std::uint8_t DBus::Type::asByte(const Any& v)
{
    return static_cast<const Byte&>(v);
}

std::int16_t DBus::Type::asInt16(const Any& v)
{
    return static_cast<const Int16&>(v);
}

std::int32_t DBus::Type::asInt32(const Any& v)
{
    return static_cast<const Int32&>(v);
}

std::int64_t DBus::Type::asInt64(const Any& v)
{
    return static_cast<const Int64&>(v);
}

std::uint16_t DBus::Type::asUint16(const Any& v)
{
    return static_cast<const Uint16&>(v);
}

std::uint32_t DBus::Type::asUint32(const Any& v)
{
    return static_cast<const Uint32&>(v);
}

std::uint64_t DBus::Type::asUint64(const Any& v)
{
    return static_cast<const Uint64&>(v);
}



const DBus::Type::Array& DBus::Type::refArray(const Any& value)
{
    return static_cast<const DBus::Type::Array&>(value);
}

const DBus::Type::Struct& DBus::Type::refStruct(const Any& value)
{
    return static_cast<const DBus::Type::Struct&>(value);
}

const DBus::Type::Variant& DBus::Type::refVariant(const Any& value)
{
    return static_cast<const DBus::Type::Variant&>(value);
}

const DBus::Type::Signature& DBus::Type::refSignature(const Any& value)
{
    return static_cast<const DBus::Type::Signature&>(value);
}

const DBus::Type::DictEntry& DBus::Type::refDictEntry(const Any& value)
{
    return static_cast<const DBus::Type::DictEntry&>(value);
}



//
// Mapping methods to convert between abstract Generic types, to specific
// classes
//
std::size_t DBus::Type::getAlignment(const std::string& typeCode)
{
    switch (typeCode[0])
    {
        case Byte::code:        return Byte::alignment;
        case Boolean::code:     return Boolean::alignment;
        case Int16::code:       return Int16::alignment;
        case Int32::code:       return Int32::alignment;
        case Int64::code:       return Int64::alignment;
        case Uint16::code:      return Uint16::alignment;
        case Uint32::code:      return Uint32::alignment;
        case Uint64::code:      return Uint64::alignment;
        case Double::code:      return Double::alignment;
        case String::code:      return String::alignment;
        case ObjectPath::code:  return ObjectPath::alignment;
        case Signature::code:   return Signature::alignment;
        case Array::code:       return Array::alignment;
        case Variant::code:     return Variant::alignment;
        case Struct::code:      return Struct::alignment;
        case DictEntry::code:   return DictEntry::alignment;
        default:
            throw std::runtime_error("Type::getAlignment() called with invalid type code " + typeCode);
    }
}

DBus::Type::Any DBus::Type::create(const std::string& typeCode)
{
    switch (typeCode[0])
    {
        case Byte::code:        return Byte();
        case Boolean::code:     return Boolean();
        case Int16::code:       return Int16();
        case Int32::code:       return Int32();
        case Int64::code:       return Int64();
        case Uint16::code:      return Uint16();
        case Uint32::code:      return Uint32();
        case Uint64::code:      return Uint64();
        case Double::code:      return Double();
        case String::code:      return String();
        case ObjectPath::code:  return ObjectPath();
        case Signature::code:   return Signature();
        case Array::code:       return Array(typeCode);
        case Variant::code:     return Variant();
        case Struct::code:      return Struct(typeCode);
        case DictEntry::code:   return DictEntry(typeCode);
        default:
            throw std::runtime_error("Type::create() called with invalid type code " + typeCode);
    }
}

bool DBus::Type::isBasicTypeCode(const int code) {
    return (code == Byte::code || code == Boolean::code ||
            code == Int16::code || code == Uint16::code ||
            code == Int32::code || code == Uint32::code ||
            code == Int64::code || code == Uint64::code ||
            code == Double::code || code == String::code ||
            code == ObjectPath::code || code == Signature::code);
}

