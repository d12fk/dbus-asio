#include "dbus_matchrule.h"
#include "dbus_names.h"
#include <catch2/catch.hpp>
#include <iostream>

namespace DBus {
namespace test {

TEST_CASE("MatchRule wildcard")
{
    REQUIRE(MatchRule().str() == "");
}

TEST_CASE("MatchRule::type()")
{
    REQUIRE(MatchRule().type(MatchRule::Type::MethodCall).str() == "type='method_call'");
    REQUIRE(MatchRule().type(MatchRule::Type::MethodReturn).str() == "type='method_return'");
    REQUIRE(MatchRule().type(MatchRule::Type::Error).str() == "type='error'");
    REQUIRE(MatchRule().type(MatchRule::Type::Signal).str() == "type='signal'");
}

TEST_CASE("MatchRule::sender()")
{
    REQUIRE(MatchRule().sender(":1.234").str() == "sender=':1.234'");
    REQUIRE(MatchRule().sender("well-known.name").str() == "sender='well-known.name'");
}

TEST_CASE("MatchRule::interface()")
{
    REQUIRE(MatchRule().interface("inter_face.Name").str() == "interface='inter_face.Name'");
}

TEST_CASE("MatchRule::member()")
{
    REQUIRE(MatchRule().member("MemberName_").str() == "member='MemberName_'");
}

TEST_CASE("MatchRule::path()")
{
    REQUIRE(MatchRule().path("/Org/Foo").str() == "path='/Org/Foo'");
    REQUIRE_THROWS_AS(MatchRule().path("/Org/Foo").pathNamespace("/Org/Bar"), InvalidMatchRule);
}

TEST_CASE("MatchRule::pathNamespace()")
{
    REQUIRE(MatchRule().pathNamespace("/Org/Foo").str() == "path_namespace='/Org/Foo'");
    REQUIRE_THROWS_AS(MatchRule().pathNamespace("/Org/Foo").path("/Org/Bar"), InvalidMatchRule);
}

TEST_CASE("MatchRule::destination()")
{
    REQUIRE(MatchRule().destination(":1.234").str() == "destination=':1.234'");
}

TEST_CASE("MatchRule::arg0Namespace()")
{
    REQUIRE(MatchRule().arg0Namespace("Name.Space").str() == "arg0namespace='Name.Space'");
}

TEST_CASE("MatchRule::arg()")
{
    REQUIRE(MatchRule().arg(0,"foo").arg(42,"bar").str() == "arg0='foo',arg42='bar'");
    REQUIRE(MatchRule().arg(0,"'").str() == "arg0=''\\'''");
    REQUIRE(MatchRule().arg(63,"\\").str() == "arg63='\\'");
    REQUIRE_THROWS_AS(MatchRule().arg(64,"/Org/Foo"), InvalidMatchRule);
}

TEST_CASE("MatchRule::argPath()")
{
    REQUIRE(MatchRule().argPath(63,"/foo/bar/").str() == "arg63path='/foo/bar/'");
    REQUIRE(MatchRule().argPath(0,"'").str() == "arg0path=''\\'''");
    REQUIRE(MatchRule().argPath(63,"\\").str() == "arg63path='\\'");
    REQUIRE_THROWS_AS(MatchRule().argPath(64,"/Org/Foo"), InvalidMatchRule);
}

} // namespace test
} // namespace DBus
