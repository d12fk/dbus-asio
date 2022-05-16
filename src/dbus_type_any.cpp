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

#include "dbus_names.h"
#include "dbus_type_any.h"
#include "dbus_type_array.h"
#include "dbus_type_struct.h"
#include "dbus_type_variant.h"
#include "dbus_type_dictentry.h"

#include <new>

DBus::Type::Any& DBus::Type::Any::clone(const Any& v)
{
    m_valueType = v.m_valueType;
    switch (m_valueType) {
    case Value::Byte:       new(&m_byte) Byte(v);             break;
    case Value::Double:     new(&m_double) Double(v);         break;
    case Value::UnixFd:     new(&m_unixFd) UnixFd(v);         break;
    case Value::Boolean:    new(&m_boolean) Boolean(v);       break;
    case Value::Int16:      new(&m_int16) Int16(v);           break;
    case Value::Int32:      new(&m_int32) Int32(v);           break;
    case Value::Int64:      new(&m_int64) Int64(v);           break;
    case Value::Uint16:     new(&m_uint16) Uint16(v);         break;
    case Value::Uint32:     new(&m_uint32) Uint32(v);         break;
    case Value::Uint64:     new(&m_uint64) Uint64(v);         break;
    case Value::String:     new(&m_string) String(v);         break;
    case Value::Signature:  new(&m_signature) Signature(v);   break;
    case Value::ObjectPath: new(&m_objectPath) ObjectPath(v); break;

    case Value::Array:     m_array     = new Array(*v.m_array);         break;
    case Value::Struct:    m_struct    = new Struct(*v.m_struct);       break;
    case Value::Variant:   m_variant   = new Variant(*v.m_variant);     break;
    case Value::DictEntry: m_dictEntry = new DictEntry(*v.m_dictEntry); break;

    case Value::None:       break;
    }
    return *this;
}

void DBus::Type::Any::destroy()
{
    switch(m_valueType) {
    case Value::Byte:       m_byte.~Byte();             break;
    case Value::Signature:  m_signature.~Signature();   break;
    case Value::ObjectPath: m_objectPath.~ObjectPath(); break;
    case Value::Boolean:    m_boolean.~Boolean();       break;
    case Value::Int16:      m_int16.~Int16();           break;
    case Value::Int32:      m_int32.~Int32();           break;
    case Value::Int64:      m_int64.~Int64();           break;
    case Value::Uint16:     m_uint16.~Uint16();         break;
    case Value::Uint32:     m_uint32.~Uint32();         break;
    case Value::Uint64:     m_uint64.~Uint64();         break;
    case Value::Double:     m_double.~Double();         break;
    case Value::UnixFd:     m_unixFd.~UnixFd();         break;
    case Value::String:     m_string.~String();         break;

    case Value::Array:      delete m_array;             break;
    case Value::Struct:     delete m_struct;            break;
    case Value::Variant:    delete m_variant;           break;
    case Value::DictEntry:  delete m_dictEntry;         break;

    case Value::None:       break;
    }
}



DBus::Type::Any::Any(const Any& v)
{
    clone(v);
}

DBus::Type::Any::Any(const std::uint8_t& v)
    : m_valueType(Value::Byte)
{
    new(&m_byte) Byte(v);
}

DBus::Type::Any::Any(const Byte& v)
    : m_valueType(Value::Byte)
{
    new(&m_byte) Byte(v);
}

DBus::Type::Any::Any(const bool& v)
    : m_valueType(Value::Boolean)
{
    new(&m_boolean) Boolean(v);
}

DBus::Type::Any::Any(const Boolean& v)
    : m_valueType(Value::Boolean)
{
    new(&m_boolean) Boolean(v);
}

DBus::Type::Any::Any(const std::int16_t& v)
    : m_valueType(Value::Int16)
{
    new(&m_int16) Int16(v);
}

DBus::Type::Any::Any(const Int16& v)
    : m_valueType(Value::Int16)
{
    new(&m_int16) Int16(v);
}

DBus::Type::Any::Any(const std::int32_t& v)
    : m_valueType(Value::Int32)
{
    new(&m_int32) Int32(v);
}

DBus::Type::Any::Any(const Int32& v)
    : m_valueType(Value::Int32)
{
    new(&m_int32) Int32(v);
}

DBus::Type::Any::Any(const std::int64_t& v)
    : m_valueType(Value::Int64)
{
    new(&m_int64) Int64(v);
}

DBus::Type::Any::Any(const Int64& v)
    : m_valueType(Value::Int64)
{
    new(&m_int64) Int64(v);
}

DBus::Type::Any::Any(const std::uint16_t& v)
    : m_valueType(Value::Uint16)
{
    new(&m_uint16) Uint16(v);
}

DBus::Type::Any::Any(const Uint16& v)
    : m_valueType(Value::Uint16)
{
    new(&m_uint16) Uint16(v);
}

DBus::Type::Any::Any(const std::uint32_t& v)
    : m_valueType(Value::Uint32)
{
    new(&m_uint32) Uint32(v);
}

DBus::Type::Any::Any(const Uint32& v)
    : m_valueType(Value::Uint32)
{
    new(&m_uint32) Uint32(v);
}

DBus::Type::Any::Any(const std::uint64_t& v)
    : m_valueType(Value::Uint64)
{
    new(&m_uint64) Uint64(v);
}

DBus::Type::Any::Any(const Uint64& v)
    : m_valueType(Value::Uint64)
{
    new(&m_uint64) Uint64(v);
}

DBus::Type::Any::Any(const UnixFd& v)
    : m_valueType(Value::UnixFd)
{
    new(&m_unixFd) UnixFd(v);
}

DBus::Type::Any::Any(const double& v)
    : m_valueType(Value::Double)
{
    new(&m_double) Double(v);
}

DBus::Type::Any::Any(const Double& v)
    : m_valueType(Value::Double)
{
    new(&m_double) Double(v);
}

DBus::Type::Any::Any(const char* v)
    : m_valueType(Value::String)
{
    new(&m_string) String(v);
}

DBus::Type::Any::Any(const std::string& v)
    : m_valueType(Value::String)
{
    new(&m_string) String(v);
}

DBus::Type::Any::Any(const Name& v)
    : m_valueType(Value::String)
{
    new(&m_string) String(v);
}

DBus::Type::Any::Any(const String& v)
    : m_valueType(Value::String)
{
    new(&m_string) String(v);
}

DBus::Type::Any::Any(const Signature& v)
    : m_valueType(Value::Signature)
{
    new(&m_signature) Signature(v);
}

DBus::Type::Any::Any(const DBus::ObjectPath& v)
    : m_valueType(Value::ObjectPath)
{
    new(&m_objectPath) ObjectPath(v);
}

DBus::Type::Any::Any(const DBus::Type::ObjectPath& v)
    : m_valueType(Value::ObjectPath)
{
    new(&m_objectPath) ObjectPath(v);
}


DBus::Type::Any::Any(const Array& v)
    : m_valueType(Value::Array)
    , m_array(new Array(v))
{}

DBus::Type::Any::Any(const Struct& v)
    : m_valueType(Value::Struct)
    , m_struct(new Struct(v))
{}

DBus::Type::Any::Any(const Variant& v)
    : m_valueType(Value::Variant)
    , m_variant(new Variant(v))
{}

DBus::Type::Any::Any(const DictEntry& v)
    : m_valueType(Value::DictEntry)
    , m_dictEntry(new DictEntry(v))
{}



DBus::Type::Any& DBus::Type::Any::operator=(const Any& v)
{
    destroy();
    return clone(v);
}



template<typename T>
const T& DBus::Type::Any::tryVariantTo() const
{
    std::string what;
    if (m_valueType == Value::None)
        what = "empty object";
    else
        what = value().getName();

    if (m_valueType == Value::Variant) {
        try {
            return static_cast<const T&>(m_variant->getValue());
        } catch (const BadCast& e) {
            throw BadCast(what + " > " + e.what());
        } catch (...) {
            throw;
        }
    }

    throw BadCast(what + ": cannot cast to " + T::name);
}

DBus::Type::Any::operator const Byte&() const
{
    if (m_valueType == Value::Byte)
        return m_byte;
    return tryVariantTo<Byte>();
}

DBus::Type::Any::operator const Boolean&() const
{
    if (m_valueType == Value::Boolean)
        return m_boolean;
    return tryVariantTo<Boolean>();
}

DBus::Type::Any::operator const Int16&() const
{
    if (m_valueType == Value::Int16)
        return m_int16;
    return tryVariantTo<Int16>();
}

DBus::Type::Any::operator const Int32&() const
{
    if (m_valueType == Value::Int32)
        return m_int32;
    return tryVariantTo<Int32>();
}

DBus::Type::Any::operator const Int64&() const
{
    if (m_valueType == Value::Int64)
        return m_int64;
    return tryVariantTo<Int64>();
}

DBus::Type::Any::operator const Uint16&() const
{
    if (m_valueType == Value::Uint16)
        return m_uint16;
    return tryVariantTo<Uint16>();
}

DBus::Type::Any::operator const Uint32&() const
{
    if (m_valueType == Value::Uint32)
        return m_uint32;
    return tryVariantTo<Uint32>();
}

DBus::Type::Any::operator const Uint64&() const
{
    if (m_valueType == Value::Uint64)
        return m_uint64;
    return tryVariantTo<Uint64>();
}

DBus::Type::Any::operator const Double&() const
{
    if (m_valueType == Value::Double)
        return m_double;
    return tryVariantTo<Double>();
}

DBus::Type::Any::operator const UnixFd&() const
{
    if (m_valueType == Value::UnixFd)
        return m_unixFd;
    return tryVariantTo<UnixFd>();
}

DBus::Type::Any::operator const String&() const
{
    if (m_valueType == Value::String)
        return m_string;
    if (m_valueType == Value::ObjectPath)
        return m_objectPath;
    return tryVariantTo<String>();
}

DBus::Type::Any::operator const Signature&() const
{
    if (m_valueType == Value::Signature)
        return m_signature;
    return tryVariantTo<Signature>();
}

DBus::Type::Any::operator const ObjectPath&() const
{
    if (m_valueType == Value::ObjectPath)
        return m_objectPath;
    return tryVariantTo<ObjectPath>();
}

DBus::Type::Any::operator const Array&() const
{
    if (m_valueType == Value::Array)
        return *m_array;
    return tryVariantTo<Array>();
}

DBus::Type::Any::operator const Struct&() const
{
    if (m_valueType == Value::Struct)
        return *m_struct;
    return tryVariantTo<Struct>();
}

DBus::Type::Any::operator const Variant&() const
{
    if (m_valueType == Value::Variant)
        return *m_variant;
    std::string what;
    if (m_valueType == Value::None)
        what = "empty object";
    else
        what = value().getName();
    throw BadCast(what + ": can't cast to Variant");
}

DBus::Type::Any::operator const DictEntry&() const
{
    if (m_valueType == Value::DictEntry)
        return *m_dictEntry;
    return tryVariantTo<DictEntry>();
}

bool DBus::Type::Any::isBasicType() const
{
    return Type::isBasicTypeCode(getSignature()[0]);
}

const DBus::Type& DBus::Type::Any::value() const
{
    switch(m_valueType) {
    case Value::Byte:       return m_byte;
    case Value::Boolean:    return m_boolean;
    case Value::Int16:      return m_int16;
    case Value::Int32:      return m_int32;
    case Value::Int64:      return m_int64;
    case Value::Uint16:     return m_uint16;
    case Value::Uint32:     return m_uint32;
    case Value::Uint64:     return m_uint64;
    case Value::Double:     return m_double;
    case Value::UnixFd:     return m_unixFd;
    case Value::String:     return m_string;
    case Value::Signature:  return m_signature;
    case Value::ObjectPath: return m_objectPath;
    case Value::Array:      return *m_array;
    case Value::Struct:     return *m_struct;
    case Value::Variant:    return *m_variant;
    case Value::DictEntry:  return *m_dictEntry;
    default:
        throw std::runtime_error("value: Any object has no type");
    }
}

std::string DBus::Type::Any::getName() const
{
    if (m_valueType == Value::None)
        return "Any{none}";
    else
        return "Any{" + value().getName() + "}";
}

std::size_t DBus::Type::Any::getAlignment() const
{
    return value().getAlignment();
}

std::string DBus::Type::Any::getSignature() const
{
    return value().getSignature();
}

void DBus::Type::Any::marshall(MessageOStream& stream) const
{
    return value().marshall(stream);
}

void DBus::Type::Any::unmarshall(MessageIStream& stream)
{
    switch(m_valueType) {
    case Value::Byte:       return m_byte.unmarshall(stream);
    case Value::Boolean:    return m_boolean.unmarshall(stream);
    case Value::Int16:      return m_int16.unmarshall(stream);
    case Value::Int32:      return m_int32.unmarshall(stream);
    case Value::Int64:      return m_int64.unmarshall(stream);
    case Value::Uint16:     return m_uint16.unmarshall(stream);
    case Value::Uint32:     return m_uint32.unmarshall(stream);
    case Value::Uint64:     return m_uint64.unmarshall(stream);
    case Value::Double:     return m_double.unmarshall(stream);
    case Value::UnixFd:     return m_unixFd.unmarshall(stream);
    case Value::String:     return m_string.unmarshall(stream);
    case Value::Signature:  return m_signature.unmarshall(stream);
    case Value::ObjectPath: return m_objectPath.unmarshall(stream);
    case Value::Array:      return m_array->unmarshall(stream);
    case Value::Struct:     return m_struct->unmarshall(stream);
    case Value::Variant:    return m_variant->unmarshall(stream);
    case Value::DictEntry:  return m_dictEntry->unmarshall(stream);
    case Value::None:
        throw std::runtime_error("unmarshall: Any object has no type");
    }
}

std::string DBus::Type::Any::toString(const std::string& prefix) const
{
    return value().toString(prefix);
}

std::string DBus::Type::Any::asString() const
{
    return value().asString();
}
