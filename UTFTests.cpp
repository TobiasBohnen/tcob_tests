#include "tests.hpp"

TEST_CASE("Core.UTF.Length")
{
    REQUIRE(utf8::length("löwe") == 4);
    REQUIRE(utf8::length("straße") == 6);
    REQUIRE(utf8::length("") == 0);
    REQUIRE(utf8::length("hello") == 5);
    REQUIRE(utf8::length("日本語") == 3);
    REQUIRE(utf8::length("👍") == 1);
}

TEST_CASE("Core.UTF.ToUpper")
{
    REQUIRE(utf8::to_upper("löwe") == "LÖWE");
    REQUIRE(utf8::to_upper("straße") == "STRAẞE");
    REQUIRE(utf8::to_upper("") == "");
    REQUIRE(utf8::to_upper("already upper") == "ALREADY UPPER");
    REQUIRE(utf8::to_upper("γειά") == "ΓΕΙΆ");
    REQUIRE(utf8::to_upper("привет") == "ПРИВЕТ");
}

TEST_CASE("Core.UTF.ToLower")
{
    REQUIRE(utf8::to_lower("LÖWE") == "löwe");
    REQUIRE(utf8::to_lower("STRAẞE") == "straße");
    REQUIRE(utf8::to_lower("") == "");
    REQUIRE(utf8::to_lower("already lower") == "already lower");
    REQUIRE(utf8::to_lower("ΓΕΙΆ") == "γειά");
    REQUIRE(utf8::to_lower("ПРИВЕТ") == "привет");
}

TEST_CASE("Core.UTF.Capitalize")
{
    REQUIRE(utf8::capitalize("LÖWE im käfig") == "Löwe Im Käfig");
    REQUIRE(utf8::capitalize("lÖWE iM käfig") == "Löwe Im Käfig");
    REQUIRE(utf8::capitalize("") == "");
    REQUIRE(utf8::capitalize("a") == "A");
    REQUIRE(utf8::capitalize("   ") == "   ");
    REQUIRE(utf8::capitalize("multiple   spaces") == "Multiple   Spaces");
    REQUIRE(utf8::capitalize("ǆungla") == "ǅungla");
}

TEST_CASE("Core.UTF.Insert")
{
    REQUIRE(utf8::insert("löwe", "öä", 2) == "lööäwe");
    REQUIRE(utf8::insert("löwe", "x", 0) == "xlöwe");
    REQUIRE(utf8::insert("löwe", "x", 4) == "löwex");
    REQUIRE(utf8::insert("", "abc", 0) == "abc");
}

TEST_CASE("Core.UTF.Remove")
{
    utf8_string s0 {"ä123äabc"};
    REQUIRE(utf8::remove(s0, 0, 3) == "3äabc");
    REQUIRE(utf8::remove(s0, 1, 3) == "ääabc");
    REQUIRE(utf8::remove(s0, 2, 3) == "ä1abc");
    REQUIRE(utf8::remove(s0, 3, 3) == "ä12bc");
    REQUIRE(utf8::remove(s0, 0, 0) == s0);
    REQUIRE(utf8::remove(s0, 0, utf8::length(s0)) == "");
}

TEST_CASE("Core.UTF.Substr")
{
    utf8_string s0 {"ä123äabc"};
    REQUIRE(utf8::substr(s0, 0, 3) == "ä12");
    REQUIRE(utf8::substr(s0, 1, 3) == "123");
    REQUIRE(utf8::substr(s0, 2, 3) == "23ä");
    REQUIRE(utf8::substr(s0, 3, 3) == "3äa");
    REQUIRE(utf8::substr(s0, 0, 0) == "");
    REQUIRE(utf8::substr(s0, 0, utf8::length(s0)) == s0);
}

TEST_CASE("Core.UTF.ToUtf32")
{
    utf8_string s0 {"ä123äabc"};
    REQUIRE(utf8::to_utf32(s0) == U"ä123äabc");
    REQUIRE(utf8::to_utf32("") == U"");
    REQUIRE(utf8::to_utf32("日本語") == U"日本語");
}
