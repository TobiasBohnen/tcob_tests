#include "tests.hpp"

TEST_CASE("Core.UTF.Length")
{
    REQUIRE(utf8::length("löwe") == 4);
    REQUIRE(utf8::length("straße") == 6);
}

TEST_CASE("Core.UTF.ToUpper")
{
    REQUIRE(utf8::to_upper("löwe") == "LÖWE");
    REQUIRE(utf8::to_upper("straße") == "STRAßE");
}

TEST_CASE("Core.UTF.ToLower")
{
    REQUIRE(utf8::to_lower("LÖWE") == "löwe");
    REQUIRE(utf8::to_lower("STRAẞE") == "straße");
}

TEST_CASE("Core.UTF.Capitalize")
{
    REQUIRE(utf8::capitalize("LÖWE im käfig") == "Löwe Im Käfig");
    REQUIRE(utf8::capitalize("lÖWE iM käfig") == "Löwe Im Käfig");
}

TEST_CASE("Core.UTF.Insert")
{
    REQUIRE(utf8::insert("löwe", "öä", 2) == "lööäwe");
}

TEST_CASE("Core.UTF.Remove")
{
    utf8_string s0 {"ä123äabc"};
    REQUIRE(utf8::remove(s0, 0, 3) == "3äabc");
    REQUIRE(utf8::remove(s0, 1, 3) == "ääabc");
    REQUIRE(utf8::remove(s0, 2, 3) == "ä1abc");
    REQUIRE(utf8::remove(s0, 3, 3) == "ä12bc");
}

TEST_CASE("Core.UTF.Substr")
{
    utf8_string s0 {"ä123äabc"};
    REQUIRE(utf8::substr(s0, 0, 3) == "ä12");
    REQUIRE(utf8::substr(s0, 1, 3) == "123");
    REQUIRE(utf8::substr(s0, 2, 3) == "23ä");
    REQUIRE(utf8::substr(s0, 3, 3) == "3äa");
}