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

#include "dbus_type_signature.h"
#include "dbus_type_struct.h"
#include "dbus_type_array.h"
#include "dbus_type_dictentry.h"
#include "dbus_type_variant.h"
#include "dbus_messageistream.h"
#include "dbus_messageostream.h"
#include "dbus_log.h"

#include <sstream>

DBus::Type::Signature::Signature(const std::string& v, std::size_t level)
    : m_Value(v)
{
    auto logAndThrow = [this](const char *error) {
        Log::write(Log::ERROR,
            "DBus :: signature '%s' invalid : %s\n",
            m_Value.c_str(), error);
        throw InvalidSignature(m_Value + "' : " + error);
    };

    enum Container { Array, Struct, DictEntry };

    struct ContainerStack {
        inline std::size_t size() { return m_stack.size(); };
        inline std::size_t level() { return m_arrayLevel + m_structLevel; };

        inline bool push(Container type) {
            m_stack.push_back(type);
            if (type == Struct)
                return ++m_structLevel <= MaxStructDepth;
            else if (type == Array)
                return ++m_arrayLevel <= MaxArrayDepth;
            return true;
        };

        inline void pop() {
            if (m_stack.empty())
                return;
            if (topmostIs(Struct))
                --m_structLevel;
            else if (topmostIs(Array))
                --m_arrayLevel;
            m_stack.resize(m_stack.size() - 1);
            while (topmostIs(Array))
                pop();
        };

        inline bool topmostIs(const Container type) const {
            return m_stack.size() && m_stack.back() == type;
        };

    protected:
        std::size_t m_arrayLevel = 0;
        std::size_t m_structLevel = 0;
        std::vector<Container> m_stack;
    };

    if (m_Value.size() > MaxLength)
        logAndThrow("length exceeds 255");

    ContainerStack stack;
    bool haveStructElement;
    bool haveDictEntryKey;
    bool haveDictEntryVal;

    for (const int code : m_Value) {
        // Check the key type if we're inside a DICT_ENTRY
        if (stack.topmostIs(DictEntry)) {
            if (!haveDictEntryKey) {
                if (!isBasicTypeCode(code))
                    logAndThrow("DictEntry key is not a basic type");
                haveDictEntryKey = true;
                continue;
            } else if (code != Type::DictEntry::code_end) {
                if (haveDictEntryVal)
                    logAndThrow("DictEntry has multiple values");
                haveDictEntryVal = true;
            }
        }
        // Count anything as element if we're inside a STRUCT
        else if (stack.topmostIs(Struct) && code != Type::Struct::code_end) {
            haveStructElement = true;
        }

        if (code == Array::code) {
            if (stack.push(Array) == false)
                logAndThrow("Arrays nested more than 32 times");
            if (level + stack.level() > MaxDepth)
                logAndThrow("total message depth larger then 64");
        } else if (code == Struct::code_start) {
            if (stack.push(Struct) == false)
                logAndThrow("Structs nested more than 32 times");
            if (level + stack.level() > MaxDepth)
                logAndThrow("total message depth larger then 64");
            haveStructElement = false;
        } else if (code == Struct::code_end) {
            if (!stack.topmostIs(Struct) || !haveStructElement)
                logAndThrow("Struct end unexpected");
            stack.pop();
        } else if (code == DictEntry::code_start) {
            if (!stack.topmostIs(Array))
                logAndThrow("DictEntry outside of an ARRAY");
            stack.push(DictEntry);
            haveDictEntryKey = false;
            haveDictEntryVal = false;
        } else if (code == DictEntry::code_end) {
            if (!stack.topmostIs(DictEntry) || !haveDictEntryVal)
                logAndThrow("DictEntry end unexpected");
            stack.pop();
        } else if (isBasicTypeCode(code) || code == Variant::code) {
            if (stack.topmostIs(Array))
                stack.pop();
        } else {
            logAndThrow("unknown type code");
        }
    }

    if (stack.size()) {
        if (stack.topmostIs(DictEntry))
            logAndThrow("DictEntry incomplete");
        else if (stack.topmostIs(Array))
            logAndThrow("Array incomplete");
        else if (stack.topmostIs(Struct))
            logAndThrow("Struct incomplete");
    }
}

void DBus::Type::Signature::marshall(MessageOStream& stream) const
{
    stream.writeSignature(m_Value);
}

void DBus::Type::Signature::unmarshall(MessageIStream& stream)
{
    const uint8_t size = stream.read();
    stream.read(m_Value, size);
    stream.read(); // read null;
}

std::string DBus::Type::Signature::getNextTypeCode()
{
    if (m_typeCodeIndex >= m_Value.size()) {
        m_typeCodeIndex = 0;
        return "";
    }

    // Handle STRUCT
    if (m_Value[m_typeCodeIndex] == Struct::code_start) {
        // Find the matching parenthesis, and skip over element codes
        std::size_t open_brackets = 1;
        std::size_t start = m_typeCodeIndex;
        while (++m_typeCodeIndex < m_Value.size()) {
            if (m_Value[m_typeCodeIndex] == Struct::code_start) {
                ++open_brackets;
            } else if (m_Value[m_typeCodeIndex] == Struct::code_end) {
                if (--open_brackets == 0) {
                    return m_Value.substr(start, ++m_typeCodeIndex - start);
                }
            }
        }
    }
    // Handle DICT_ENTRY
    else if (m_Value[m_typeCodeIndex] == DictEntry::code_start) {
        // Find the matching brace, and skip over element codes
        std::size_t open_brackets = 1;
        std::size_t start = m_typeCodeIndex;
        while (++m_typeCodeIndex < m_Value.size()) {
            if (m_Value[m_typeCodeIndex] == DictEntry::code_start) {
                ++open_brackets;
            } else if (m_Value[m_typeCodeIndex] == DictEntry::code_end) {
                if (--open_brackets == 0) {
                    return m_Value.substr(start, ++m_typeCodeIndex - start);
                }
            }
        }
    }

    std::string typeCode(1, m_Value[m_typeCodeIndex]);
    // Handle ARRAY
    if (m_Value[m_typeCodeIndex++] == Array::code)
        typeCode += getNextTypeCode();

    return typeCode;
}

std::string DBus::Type::Signature::toString(const std::string&) const
{
    std::ostringstream oss;
    oss << "Signature '" << m_Value << "'\n";
    return oss.str();
}

std::string DBus::Type::Signature::asString() const
{
    return m_Value;
}
