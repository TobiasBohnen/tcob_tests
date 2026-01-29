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
    sys.add_rule('A', {.Replacement = "AB"});
    sys.add_rule('B', {.Replacement = "A"});
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
    sys.add_rule('1', {.Replacement = "11"});
    sys.add_rule('0', {.Replacement = "1[0]0"});
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
    sys.add_rule('F', {.Replacement = "F+F-F-F+F"});
    REQUIRE(sys.generate("F", 1) == "F+F-F-F+F");
    REQUIRE(sys.generate("F", 2) == "F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F");
    REQUIRE(sys.generate("F", 3) == "F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F+F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F-F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F-F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F+F+F-F-F+F+F+F-F-F+F-F+F-F-F+F-F+F-F-F+F+F+F-F-F+F");
}

TEST_CASE("GFX.LSystem.LeftContext")
{
    /*
    Test left context matching
    axiom  : ABC
    rules  : (A < B → X)  // B becomes X only if A is to its left
    */
    l_system sys {};
    l_rule   rule;
    rule.Replacement = "X";
    rule.LeftContext = "A";
    sys.add_rule('B', rule);

    // B has A to its left, should become X
    REQUIRE(sys.generate("ABC", 1) == "AXC");

    // B doesn't have A to its left, should stay B
    REQUIRE(sys.generate("CBC", 1) == "CBC");
    REQUIRE(sys.generate("BBC", 1) == "BBC");

    // Multiple Bs, only the one after A transforms
    REQUIRE(sys.generate("BABC", 1) == "BAXC");
}

TEST_CASE("GFX.LSystem.RightContext")
{
    /*
    Test right context matching
    axiom  : ABC
    rules  : (B > C → Y)  // B becomes Y only if C is to its right
    */
    l_system sys {};
    l_rule   rule;
    rule.Replacement  = "Y";
    rule.RightContext = "C";
    sys.add_rule('B', rule);

    // B has C to its right, should become Y
    REQUIRE(sys.generate("ABC", 1) == "AYC");

    // B doesn't have C to its right, should stay B
    REQUIRE(sys.generate("ABD", 1) == "ABD");
    REQUIRE(sys.generate("ABB", 1) == "ABB");

    // Multiple Bs, only the one before C transforms
    REQUIRE(sys.generate("ABCB", 1) == "AYCB");
}

TEST_CASE("GFX.LSystem.BothContexts")
{
    /*
    Test both left and right context
    axiom  : ABCD
    rules  : (A < B > C → Z)  // B becomes Z only if A is left and C is right
    */
    l_system sys {};
    l_rule   rule;
    rule.Replacement  = "Z";
    rule.LeftContext  = "A";
    rule.RightContext = "C";
    sys.add_rule('B', rule);

    // B has both A to left and C to right, should become Z
    REQUIRE(sys.generate("ABCD", 1) == "AZCD");

    // B missing left context
    REQUIRE(sys.generate("XBCD", 1) == "XBCD");

    // B missing right context
    REQUIRE(sys.generate("ABXD", 1) == "ABXD");

    // B missing both contexts
    REQUIRE(sys.generate("XBXD", 1) == "XBXD");

    // Multiple Bs, only the one with both contexts transforms
    REQUIRE(sys.generate("BABCD", 1) == "BAZCD");
}

TEST_CASE("GFX.LSystem.MultiCharContext")
{
    /*
    Test multi-character context matching
    axiom  : AABCC
    rules  : (AA < B > CC → W)  // B becomes W only if AA is left and CC is right
    */
    l_system sys {};
    l_rule   rule;
    rule.Replacement  = "W";
    rule.LeftContext  = "AA";
    rule.RightContext = "CC";
    sys.add_rule('B', rule);

    // B has AA to left and CC to right
    REQUIRE(sys.generate("AABCC", 1) == "AAWCC");

    // Only one A to the left (insufficient context)
    REQUIRE(sys.generate("ABCC", 1) == "ABCC");

    // Only one C to the right (insufficient context)
    REQUIRE(sys.generate("AABC", 1) == "AABC");

    // No match
    REQUIRE(sys.generate("ABCD", 1) == "ABCD");
}

TEST_CASE("GFX.LSystem.ContextAtBoundaries")
{
    /*
    Test context matching at string boundaries
    */
    l_system sys {};
    l_rule   rule;
    rule.Replacement = "X";
    rule.LeftContext = "A";
    sys.add_rule('B', rule);

    // B at start has no left context, should not match
    REQUIRE(sys.generate("BC", 1) == "BC");

    l_system sys2 {};
    l_rule   rule2;
    rule2.Replacement  = "Y";
    rule2.RightContext = "C";
    sys2.add_rule('B', rule2);

    // B at end has no right context, should not match
    REQUIRE(sys2.generate("AB", 1) == "AB");
}
TEST_CASE("GFX.LSystem.ContextSensitiveGrowth")
{
    /*
    Practical example: Signal propagation from A
    axiom  : ABBBBC
    rules  : (A < B → X)
    Only the B immediately after A should transform
    */
    l_system sys {};

    l_rule rule;
    rule.Replacement = "X";
    rule.LeftContext = "A";
    sys.add_rule('B', rule);

    // Only first B (after A) transforms
    REQUIRE(sys.generate("ABBBBC", 1) == "AXBBBC");

    // After another iteration, X blocks further As, so no more changes to B
    REQUIRE(sys.generate("ABBBBC", 2) == "AXBBBC");
}

TEST_CASE("GFX.LSystem.ContextPropagation")
{
    /*
    Demonstrate signal propagation over multiple iterations
    axiom  : SBBBB
    rules  : (S < B → S), (B → B)  // B adjacent to S becomes S
    */
    l_system sys {};

    l_rule propagate;
    propagate.Replacement = "S";
    propagate.LeftContext = "S";
    sys.add_rule('B', propagate);

    // Signal spreads one position per iteration
    REQUIRE(sys.generate("SBBBB", 0) == "SBBBB");
    REQUIRE(sys.generate("SBBBB", 1) == "SSBBB");
    REQUIRE(sys.generate("SBBBB", 2) == "SSSBB");
    REQUIRE(sys.generate("SBBBB", 3) == "SSSSB");
    REQUIRE(sys.generate("SBBBB", 4) == "SSSSS");
}

TEST_CASE("GFX.LSystem.MultipleContextRules")
{
    /*
    Multiple rules for the same symbol with different contexts
    axiom  : ABBC
    rules  : (A < B → X, probability 1.0)
             (B > C → Y, probability 1.0)
    */
    l_system sys {};

    l_rule left_rule;
    left_rule.Replacement = "X";
    left_rule.LeftContext = "A";
    left_rule.Probability = 1.0f;
    sys.add_rule('B', left_rule);

    l_rule right_rule;
    right_rule.Replacement  = "Y";
    right_rule.RightContext = "C";
    right_rule.Probability  = 1.0f;
    sys.add_rule('B', right_rule);

    // First B (pos 1): has A to left → becomes X
    // Second B (pos 2): has C to right → becomes Y
    REQUIRE(sys.generate("ABBC", 1) == "AXYC");
}
TEST_CASE("GFX.LSystem.Priority")
{
    l_system sys {};

    l_rule low_priority;
    low_priority.Replacement = "B";
    low_priority.Priority    = 0;
    sys.add_rule('A', low_priority);

    l_rule high_priority;
    high_priority.Replacement = "C";
    high_priority.Priority    = 1;
    sys.add_rule('A', high_priority);

    // High priority rule should win
    REQUIRE(sys.generate("A", 1) == "C");
}

TEST_CASE("GFX.LSystem.MinIteration")
{
    l_system sys {};

    // Rule only applies when iteration >= 2
    l_rule delayed_rule;
    delayed_rule.Replacement  = "B";
    delayed_rule.MinIteration = 2;
    sys.add_rule('A', delayed_rule);

    // generate("A", 1) runs loop with i=0, rule needs i>=2, so no match
    REQUIRE(sys.generate("A", 1) == "A");

    // generate("A", 2) runs loop with i=0,1, both < 2, so no match
    REQUIRE(sys.generate("A", 2) == "A");

    // generate("A", 3) runs loop with i=0,1,2, at i=2 rule applies
    REQUIRE(sys.generate("A", 3) == "B");
}

TEST_CASE("GFX.LSystem.MaxIteration")
{
    l_system sys {};

    // Rule only applies when iteration <= 1
    l_rule early_rule;
    early_rule.Replacement  = "AA";
    early_rule.MaxIteration = 1;
    sys.add_rule('A', early_rule);

    // i=0: A→AA
    REQUIRE(sys.generate("A", 1) == "AA");

    // i=0: A→AA → "AA"
    // i=1: A→AA, A→AA → "AAAA"
    REQUIRE(sys.generate("A", 2) == "AAAA");

    // i=0: A→AA → "AA"
    // i=1: A→AA, A→AA → "AAAA"
    // i=2: A→A, A→A, A→A, A→A (max exceeded, no rule applies) → "AAAA"
    REQUIRE(sys.generate("A", 3) == "AAAA");

    // Should stay the same since no rules apply at i>=2
    REQUIRE(sys.generate("A", 4) == "AAAA");
}

TEST_CASE("GFX.LSystem.IterationWindow")
{
    l_system sys {};

    // Rule only applies when 1 <= iteration <= 2
    l_rule windowed;
    windowed.Replacement  = "B";
    windowed.MinIteration = 1;
    windowed.MaxIteration = 2;
    sys.add_rule('A', windowed);

    // i=0: rule needs i>=1, doesn't apply
    REQUIRE(sys.generate("A", 1) == "A");

    // i=0: A→A, i=1: A→B (rule applies at i=1)
    REQUIRE(sys.generate("A", 2) == "B");

    // i=0: A→A, i=1: A→B, i=2: B→B (rule applies at i=1 and i=2)
    REQUIRE(sys.generate("A", 3) == "B");

    // i=0: A→A, i=1: A→B, i=2: B→B, i=3: B→B (rule doesn't apply at i=3)
    REQUIRE(sys.generate("A", 4) == "B");
}

TEST_CASE("GFX.LSystem.PhasedGrowthSimple")
{
    l_system sys {};

    // Phase 1: iterations 0-1, X→XA
    l_rule phase1;
    phase1.Replacement  = "XA";
    phase1.MaxIteration = 1;
    sys.add_rule('X', phase1);

    // Phase 2: iterations 2+, X→XB
    l_rule phase2;
    phase2.Replacement  = "XB";
    phase2.MinIteration = 2;
    sys.add_rule('X', phase2);

    // Keep A and B stable
    sys.add_rule('A', {"A"});
    sys.add_rule('B', {"B"});

    // i=0: X→XA
    REQUIRE(sys.generate("X", 1) == "XA");

    // i=0: X→XA, i=1: X→XA, A→A
    REQUIRE(sys.generate("X", 2) == "XAA");

    // i=0: X→XA, i=1: X→XA, i=2: X→XB, A→A
    REQUIRE(sys.generate("X", 3) == "XBAA");
}