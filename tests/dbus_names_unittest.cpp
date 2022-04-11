#include "dbus_names.h"
#include <catch2/catch.hpp>
#include <iostream>

namespace DBus {
namespace test {

using Catch::Matchers::Contains;

TEST_CASE("BusName Validation")
{
    REQUIRE_NOTHROW(BusName("-._"));
    REQUIRE_NOTHROW(BusName(":ab.7"));
    REQUIRE_NOTHROW(BusName(":1.234"));
    REQUIRE_NOTHROW(BusName("a.b.c.d"));
    REQUIRE_NOTHROW(BusName("_1.valid-name"));
    REQUIRE_NOTHROW(BusName("abcdefghijklmnopqrstuvwxyz.ABCDEFGHIJKLMNOPQRSTUVWXYZ._0123456789-"));
    REQUIRE_NOTHROW(BusName(
        "OKAY456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCD.F"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDE"));

    REQUIRE_THROWS_WITH(BusName(""),                    "name is empty");
    REQUIRE_THROWS_WITH(BusName("."),                   ". has empty element");
    REQUIRE_THROWS_WITH(BusName(":"),                   ": doesn't have two elements");
    REQUIRE_THROWS_WITH(BusName(":.b"),                 ":.b has empty element");
    REQUIRE_THROWS_WITH(BusName("a..b"),                "a..b has empty element");
    REQUIRE_THROWS_WITH(BusName("a!.b"),                "a!.b has invalid character");
    REQUIRE_THROWS_WITH(BusName(".invalid.name"),       ".invalid.name has empty element");
    REQUIRE_THROWS_WITH(BusName("invalid_name"),        "invalid_name doesn't have two elements");
    REQUIRE_THROWS_WITH(BusName("1.invalid.name"),      "1.invalid.name element starts with digit");
    REQUIRE_THROWS_WITH(BusName("invalid.name.2nd"),    "invalid.name.2nd element starts with digit");
    REQUIRE_THROWS_WITH(BusName(
        "INVALID789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCD.F"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"),
        Contains(" exceeds 255 characters"));
}

TEST_CASE("WellKnownName Validation")
{
    REQUIRE_NOTHROW(WellKnownName("-._"));
    REQUIRE_NOTHROW(WellKnownName("a.b.c.d"));
    REQUIRE_NOTHROW(WellKnownName("_1.valid-name"));
    REQUIRE_NOTHROW(WellKnownName("abcdefghijklmnopqrstuvwxyz.ABCDEFGHIJKLMNOPQRSTUVWXYZ._0123456789-"));
    REQUIRE_NOTHROW(WellKnownName(
        "OKAY456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCD.F"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDE"));

    REQUIRE_THROWS_WITH(WellKnownName(""),                  "name is empty");
    REQUIRE_THROWS_WITH(WellKnownName("."),                 ". has empty element");
    REQUIRE_THROWS_WITH(WellKnownName(":"),                 ": is not a well-known name");
    REQUIRE_THROWS_WITH(WellKnownName(":.b"),               ":.b is not a well-known name");
    REQUIRE_THROWS_WITH(WellKnownName("a..b"),              "a..b has empty element");
    REQUIRE_THROWS_WITH(WellKnownName("a!.b"),              "a!.b has invalid character");
    REQUIRE_THROWS_WITH(WellKnownName(":ab.7"),             ":ab.7 is not a well-known name");
    REQUIRE_THROWS_WITH(WellKnownName(":1.234"),            ":1.234 is not a well-known name");
    REQUIRE_THROWS_WITH(WellKnownName(".invalid.name"),     ".invalid.name has empty element");
    REQUIRE_THROWS_WITH(WellKnownName("invalid_name"),      "invalid_name doesn't have two elements");
    REQUIRE_THROWS_WITH(WellKnownName("1.invalid.name"),    "1.invalid.name element starts with digit");
    REQUIRE_THROWS_WITH(WellKnownName("invalid.name.2nd"),  "invalid.name.2nd element starts with digit");
    REQUIRE_THROWS_WITH(WellKnownName(
        "INVALID789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCD.F"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"),
        Contains(" exceeds 255 characters"));
}

TEST_CASE("UniqueName Validation")
{
    REQUIRE_NOTHROW(UniqueName(":_.-"));
    REQUIRE_NOTHROW(UniqueName(":ab.7"));
    REQUIRE_NOTHROW(UniqueName(":1.234"));
    REQUIRE_NOTHROW(UniqueName(":a.b.c.d"));
    REQUIRE_NOTHROW(UniqueName(":_1.valid-name"));
    REQUIRE_NOTHROW(UniqueName(":abcdefghijklmnopqrstuvwxyz.ABCDEFGHIJKLMNOPQRSTUVWXYZ._0123456789-"));
    REQUIRE_NOTHROW(UniqueName(
        ":OKAY56789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCD.F"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDE"));

    REQUIRE_THROWS_WITH(UniqueName(""),                 "name is empty");
    REQUIRE_THROWS_WITH(UniqueName("."),                ". is not a unique connection name");
    REQUIRE_THROWS_WITH(UniqueName(":"),                ": doesn't have two elements");
    REQUIRE_THROWS_WITH(UniqueName("-._"),              "-._ is not a unique connection name");
    REQUIRE_THROWS_WITH(UniqueName(":.b"),              ":.b has empty element");
    REQUIRE_THROWS_WITH(UniqueName(":a..b"),            ":a..b has empty element");
    REQUIRE_THROWS_WITH(UniqueName(":a:.b"),            ":a:.b has invalid character");
    REQUIRE_THROWS_WITH(UniqueName("a.b.c.d"),          "a.b.c.d is not a unique connection name");
    REQUIRE_THROWS_WITH(UniqueName(":.invalid.name"),   ":.invalid.name has empty element");
    REQUIRE_THROWS_WITH(UniqueName(":invalid_name"),    ":invalid_name doesn't have two elements");
    REQUIRE_THROWS_WITH(UniqueName(
        "INVALID789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCD.F"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"),
        Contains(" exceeds 255 characters"));
}

TEST_CASE("ErrorName Validation")
{
    REQUIRE_NOTHROW(ErrorName("_._"));
    REQUIRE_NOTHROW(ErrorName("a.b.c.d"));
    REQUIRE_NOTHROW(ErrorName("_1.valid.Name"));
    REQUIRE_NOTHROW(ErrorName("abcdefghijklmnopqrstuvwxyz.ABCDEFGHIJKLMNOPQRSTUVWXYZ._0123456789"));
    REQUIRE_NOTHROW(ErrorName(
        "OKAY456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCD.F"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDE"));

    REQUIRE_THROWS_WITH(ErrorName(""),                  "name is empty");
    REQUIRE_THROWS_WITH(ErrorName("."),                 ". has empty element");
    REQUIRE_THROWS_WITH(ErrorName(":"),                 ": is not a error name");
    REQUIRE_THROWS_WITH(ErrorName("-._"),               "-._ has invalid character");
    REQUIRE_THROWS_WITH(ErrorName(".b"),                ".b has empty element");
    REQUIRE_THROWS_WITH(ErrorName("ab.7"),              "ab.7 element starts with digit");
    REQUIRE_THROWS_WITH(ErrorName("a..b"),              "a..b has empty element");
    REQUIRE_THROWS_WITH(ErrorName("a .b"),              "a .b has invalid character");
    REQUIRE_THROWS_WITH(ErrorName(".invalid.name"),     ".invalid.name has empty element");
    REQUIRE_THROWS_WITH(ErrorName("invalid_name"),      "invalid_name doesn't have two elements");
    REQUIRE_THROWS_WITH(ErrorName(
        "INVALID789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCD.F"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"),
        "INVALID789abcdef... exceeds 255 characters");
}

TEST_CASE("InterfaceName Validation")
{
    REQUIRE_NOTHROW(InterfaceName("_._"));
    REQUIRE_NOTHROW(InterfaceName("a.b.c.d"));
    REQUIRE_NOTHROW(InterfaceName("_1.valid.Name"));
    REQUIRE_NOTHROW(InterfaceName("abcdefghijklmnopqrstuvwxyz.ABCDEFGHIJKLMNOPQRSTUVWXYZ._0123456789"));
    REQUIRE_NOTHROW(InterfaceName(
        "OKAY456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCD.F"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDE"));

    REQUIRE_THROWS_WITH(InterfaceName(""),                  "name is empty");
    REQUIRE_THROWS_WITH(InterfaceName("."),                 ". has empty element");
    REQUIRE_THROWS_WITH(InterfaceName(":"),                 ": is not a interface name");
    REQUIRE_THROWS_WITH(InterfaceName("-._"),               "-._ has invalid character");
    REQUIRE_THROWS_WITH(InterfaceName(".b"),                ".b has empty element");
    REQUIRE_THROWS_WITH(InterfaceName("ab.7"),              "ab.7 element starts with digit");
    REQUIRE_THROWS_WITH(InterfaceName("a..b"),              "a..b has empty element");
    REQUIRE_THROWS_WITH(InterfaceName("a .b"),              "a .b has invalid character");
    REQUIRE_THROWS_WITH(InterfaceName(".invalid.name"),     ".invalid.name has empty element");
    REQUIRE_THROWS_WITH(InterfaceName("invalid_name"),      "invalid_name doesn't have two elements");
    REQUIRE_THROWS_WITH(InterfaceName(
        "INVALID789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCD.F"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"),
        "INVALID789abcdef... exceeds 255 characters");
}

TEST_CASE("NamespaceName Validation")
{
    REQUIRE_NOTHROW(NamespaceName("_"));
    REQUIRE_NOTHROW(NamespaceName("ValidName"));
    REQUIRE_NOTHROW(NamespaceName("a.b.c.d"));
    REQUIRE_NOTHROW(NamespaceName("_1.valid.Name"));
    REQUIRE_NOTHROW(NamespaceName("abcdefghijklmnopqrstuvwxyz.ABCDEFGHIJKLMNOPQRSTUVWXYZ._0123456789"));
    REQUIRE_NOTHROW(NamespaceName(
        "OKAY456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCD.F"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDE"));

    REQUIRE_THROWS_WITH(NamespaceName(""),                  "name is empty");
    REQUIRE_THROWS_WITH(NamespaceName("."),                 ". has empty element");
    REQUIRE_THROWS_WITH(NamespaceName(":"),                 ": is not a namespace name");
    REQUIRE_THROWS_WITH(NamespaceName("-._"),               "-._ has invalid character");
    REQUIRE_THROWS_WITH(NamespaceName(".b"),                ".b has empty element");
    REQUIRE_THROWS_WITH(NamespaceName("ab.7"),              "ab.7 element starts with digit");
    REQUIRE_THROWS_WITH(NamespaceName("a..b"),              "a..b has empty element");
    REQUIRE_THROWS_WITH(NamespaceName("a .b"),              "a .b has invalid character");
    REQUIRE_THROWS_WITH(NamespaceName(".invalid.name"),     ".invalid.name has empty element");
    REQUIRE_THROWS_WITH(NamespaceName(
        "INVALID789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCD.F"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"),
        "INVALID789abcdef... exceeds 255 characters");
}

TEST_CASE("MemberName Validation")
{
    REQUIRE_NOTHROW(MemberName("_"));
    REQUIRE_NOTHROW(MemberName("Abcd"));
    REQUIRE_NOTHROW(MemberName("_1validName"));
    REQUIRE_NOTHROW(MemberName("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"));
    REQUIRE_NOTHROW(MemberName(
        "OKAY456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDE"));

    REQUIRE_THROWS_WITH(MemberName(""),                     "name is empty");
    REQUIRE_THROWS_WITH(MemberName("."),                    ". has invalid character");
    REQUIRE_THROWS_WITH(MemberName(":"),                    ": is not a member name");
    REQUIRE_THROWS_WITH(MemberName("-"),                    "- has invalid character");
    REQUIRE_THROWS_WITH(MemberName("7"),                    "7 starts with digit");
    REQUIRE_THROWS_WITH(MemberName(
        "INVALID789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"
        "0123456789abcdef0123456789ABCDEF0123456789abcdef0123456789ABCDEF"),
        "INVALID789abcdef... exceeds 255 characters");
}

TEST_CASE("ObjectPath Validation")
{
    REQUIRE_NOTHROW(ObjectPath("/"));
    REQUIRE_NOTHROW(ObjectPath("/foo"));
    REQUIRE_NOTHROW(ObjectPath("/foo/bar"));
    REQUIRE_NOTHROW(ObjectPath("/___/1___"));
    REQUIRE_NOTHROW(ObjectPath("/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"));

    REQUIRE_THROWS_WITH(ObjectPath(""),                     "path is empty");
    REQUIRE_THROWS_WITH(ObjectPath("foo/"),                 "foo/ doesn't start with slash");
    REQUIRE_THROWS_WITH(ObjectPath("/foo/"),                "/foo/ ends with slash");
    REQUIRE_THROWS_WITH(ObjectPath("/foo!/bar"),            "/foo!/bar has invalid character");
    REQUIRE_THROWS_WITH(ObjectPath("/foo//bar"),            "/foo//bar has // sequence");
}

} // namespace test
} // namespace DBus
