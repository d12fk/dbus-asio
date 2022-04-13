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

#include "dbus_type_boolean.h"
#include "dbus_type_byte.h"
#include "dbus_type_double.h"
#include "dbus_type_int16.h"
#include "dbus_type_int32.h"
#include "dbus_type_int64.h"
#include "dbus_type_objectpath.h"
#include "dbus_type_signature.h"
#include "dbus_type_string.h"
#include "dbus_type_uint16.h"
#include "dbus_type_uint32.h"
#include "dbus_type_uint64.h"

#include <exception>

namespace DBus {

    struct Name;
    struct ObjectPath;

    class Type::Any : public Type {
    protected:
        enum class Value {
            None, Byte, Boolean, Double,
            Int16, Int32, Int64, Uint16, Uint32, Uint64,
            String, ObjectPath, Signature,
            Array, Struct, Variant, DictEntry
        };

        Value m_valueType = Value::None;
        union {
            Byte m_byte;
            Boolean m_boolean;
            Double m_double;

            Int16 m_int16;
            Int32 m_int32;
            Int64 m_int64;

            Uint16 m_uint16;
            Uint32 m_uint32;
            Uint64 m_uint64;

            String m_string;
            ObjectPath m_objectPath;
            Signature m_signature;

            Array *m_array;
            Struct *m_struct;
            Variant *m_variant;
            DictEntry *m_dictEntry;
        };

        template<typename T> const T& tryVariantTo() const;

        const Type& value() const;
        Any& clone(const Any& v);
        void destroy();

    public:
        Any() {}
        Any(const Any& v);
        Any(const std::uint8_t& v);
        Any(const Byte& v);
        Any(const bool& v);
        Any(const Boolean& v);
        Any(const double& v);
        Any(const Double& v);
        Any(const std::int16_t& v);
        Any(const std::int32_t& v);
        Any(const std::int64_t& v);
        Any(const Int16& v);
        Any(const Int32& v);
        Any(const Int64& v);
        Any(const std::uint16_t& v);
        Any(const std::uint32_t& v);
        Any(const std::uint64_t& v);
        Any(const Uint16& v);
        Any(const Uint32& v);
        Any(const Uint64& v);
        Any(const char* v);
        Any(const std::string& v);
        Any(const Name& v);
        Any(const String& v);
        Any(const DBus::ObjectPath& v);
        Any(const DBus::Type::ObjectPath& v);
        Any(const Signature& v);
        Any(const Array& v);
        Any(const Struct& v);
        Any(const Variant& v);
        Any(const DictEntry& v);
        ~Any() override { destroy(); }

        Any& operator=(const Any&);

        explicit operator const Byte&() const;
        explicit operator const Boolean&() const;
        explicit operator const Double&() const;
        explicit operator const Int16&() const;
        explicit operator const Int32&() const;
        explicit operator const Int64&() const;
        explicit operator const Uint16&() const;
        explicit operator const Uint32&() const;
        explicit operator const Uint64&() const;
        explicit operator const String&() const;
        explicit operator const ObjectPath&() const;
        explicit operator const Signature&() const;
        explicit operator const Array&() const;
        explicit operator const Struct&() const;
        explicit operator const Variant&() const;
        explicit operator const DictEntry&() const;

        bool isBasicType() const;

        std::string getName() const override;
        std::size_t getAlignment() const override;
        std::string getSignature() const override;

        void marshall(MessageOStream& stream) const override;
        void unmarshall(MessageIStream& stream) override;

        std::string toString(const std::string& prefix = "") const override;
        std::string asString() const override;

        class BadCast : public std::bad_cast {
            const std::string m_what;
        public:
            BadCast(const std::string& what)
                : m_what(what)
            {}

            const char* what() const noexcept override {
                return m_what.c_str();
            }
        };
    };

} // namespace DBus
