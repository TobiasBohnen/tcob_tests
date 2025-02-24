#include "tests.hpp"

TEST_CASE("GFX.LSystem.Algae")
{
    /*
    variables : A B
    constants : none
    axiom  : A
    rules  : (A → AB), (B → A)
    */

    l_system sys {"A", {{'A', "AB"}, {'B', "A"}}};
    REQUIRE(sys.generate(0) == "A");
    REQUIRE(sys.generate(1) == "AB");
    REQUIRE(sys.generate(2) == "ABA");
    REQUIRE(sys.generate(3) == "ABAAB");
    REQUIRE(sys.generate(4) == "ABAABABA");
    REQUIRE(sys.generate(5) == "ABAABABAABAAB");
    REQUIRE(sys.generate(6) == "ABAABABAABAABABAABABA");
    REQUIRE(sys.generate(7) == "ABAABABAABAABABAABABAABAABABAABAAB");
}

TEST_CASE("GFX.LSystem.FractalTree")
{
    /*
    variables : 0, 1
    constants: "[", "]"
    axiom  : 0
    rules  : (1 → 11), (0 → 1[0]0)
    */

    l_system sys {"0", {{'1', "11"}, {'0', "1[0]0"}}};
    REQUIRE(sys.generate(1) == "1[0]0");
    REQUIRE(sys.generate(2) == "11[1[0]0]1[0]0");
    REQUIRE(sys.generate(3) == "1111[11[1[0]0]1[0]0]11[1[0]0]1[0]0");
}

TEST_CASE("GFX.LSystem.KochCurve")
{
    /*
    variables : F
    constants : + -
    start  : F
    rules  : (F → F+F-F-F+F)
    */

    l_system sys {"F", {{'F', "F+F-F-F+F"}}};
    REQUIRE(sys.generate(1) == "F+F-F-F+F");
    REQUIRE(sys.generate(2) == "F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F");
    REQUIRE(sys.generate(3) == "F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F+F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F-F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F-F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F+F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F");
}
