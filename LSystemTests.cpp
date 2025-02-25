#include "tests.hpp"

TEST_CASE("GFX.LSystem.Algae")
{
    /*
    variables : A B
    constants : none
    axiom  : A
    rules  : (A → AB), (B → A)
    */

    l_system sys {};
    sys.add_rule('A', {"AB"});
    sys.add_rule('B', {"A"});
    REQUIRE(sys.generate("A", 0) == "A");
    REQUIRE(sys.generate("A", 1) == "AB");
    REQUIRE(sys.generate("A", 2) == "ABA");
    REQUIRE(sys.generate("A", 3) == "ABAAB");
    REQUIRE(sys.generate("A", 4) == "ABAABABA");
    REQUIRE(sys.generate("A", 5) == "ABAABABAABAAB");
    REQUIRE(sys.generate("A", 6) == "ABAABABAABAABABAABABA");
    REQUIRE(sys.generate("A", 7) == "ABAABABAABAABABAABABAABAABABAABAAB");
}

TEST_CASE("GFX.LSystem.FractalTree")
{
    /*
    variables : 0, 1
    constants: "[", "]"
    axiom  : 0
    rules  : (1 → 11), (0 → 1[0]0)
    */

    l_system sys {};
    sys.add_rule('1', {"11"});
    sys.add_rule('0', {"1[0]0"});
    REQUIRE(sys.generate("0", 1) == "1[0]0");
    REQUIRE(sys.generate("0", 2) == "11[1[0]0]1[0]0");
    REQUIRE(sys.generate("0", 3) == "1111[11[1[0]0]1[0]0]11[1[0]0]1[0]0");
}

TEST_CASE("GFX.LSystem.KochCurve")
{
    /*
    variables : F
    constants : + -
    start  : F
    rules  : (F → F+F-F-F+F)
    */

    l_system sys {};
    sys.add_rule('F', {"F+F-F-F+F"});
    REQUIRE(sys.generate("F", 1) == "F+F-F-F+F");
    REQUIRE(sys.generate("F", 2) == "F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F");
    REQUIRE(sys.generate("F", 3) == "F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F+F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F-F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F-F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F+F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F");
}
