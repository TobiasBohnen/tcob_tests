#include "tests.hpp"

TEST_CASE("Core.POD.Color")
{
    static_assert(std::is_copy_constructible_v<color>);
    static_assert(std::is_copy_assignable_v<color>);
    static_assert(std::is_move_constructible_v<color>);
    static_assert(std::is_move_assignable_v<color>);
    static_assert(POD<color>);

    SUBCASE("Construction")
    {
        {
            color p;
            REQUIRE(p.A == 0);
            REQUIRE(p.R == 0);
            REQUIRE(p.G == 0);
            REQUIRE(p.B == 0);
        }
        {
            color p {10, 20, 30, 40};
            REQUIRE(p.R == 10);
            REQUIRE(p.G == 20);
            REQUIRE(p.B == 30);
            REQUIRE(p.A == 40);
        }
        {
            color p1 {10, 20, 30, 40};
            color p2 {p1};
            REQUIRE(p2.R == 10);
            REQUIRE(p2.G == 20);
            REQUIRE(p2.B == 30);
            REQUIRE(p2.A == 40);
        }
        {
            u32   col {0xffeeddcc};
            color p {color::FromRGBA(col)};
            REQUIRE(p.R == 0xff);
            REQUIRE(p.G == 0xee);
            REQUIRE(p.B == 0xdd);
            REQUIRE(p.A == 0xcc);
            REQUIRE(p.value() == col);
        }
        {
            u32   col {0xffeedd};
            color p {color::FromRGB(col)};
            REQUIRE(p.R == 0xff);
            REQUIRE(p.G == 0xee);
            REQUIRE(p.B == 0xdd);
            REQUIRE(p.A == 0xff);
            REQUIRE(p.value() == (col << 8 | 255));
        }
        {
            u32   col {0xffeeddcc};
            color p {color::FromABGR(col)};
            REQUIRE(p.A == 0xff);
            REQUIRE(p.B == 0xee);
            REQUIRE(p.G == 0xdd);
            REQUIRE(p.R == 0xcc);
            REQUIRE(p.value() == std::byteswap(col));
        }
    }

    SUBCASE("Equality")
    {
        {
            color p1 {10, 20, 30, 40};
            color p2 {10, 20, 30, 40};
            REQUIRE(p1 == p2);
        }
        {
            color p1 {colors::Beige};
            color p2 {color::FromRGBA(p1.value())};
            REQUIRE(p1 == p2);
        }
        {
            color p1 {10, 20, 30, 40};
            color p2 {40, 30, 20, 10};
            REQUIRE_FALSE(p1 == p2);
        }
    }

    SUBCASE("Lerp")
    {
        color pexp {0x7f, 0x7f, 0x7f, 0xFF};
        color p1 {0, 0, 0, 0xFF};
        color p2 {0xFF, 0xFF, 0xFF, 0xFF};
        color pact = color::Lerp(p1, p2, 0.5);
        REQUIRE(pact == pexp);
    }

    SUBCASE("PreMultiplyAlpha")
    {
        color pexp {25, 50, 100, 0x80};
        color p1 {50, 100, 200, 0x80};
        color pact {p1.as_alpha_premultiplied()};
        REQUIRE(pact == pexp);
    }

    SUBCASE("FromString")
    {
        REQUIRE(color::FromString("Red") == colors::Red);
        REQUIRE(color::FromString("red") == colors::Red);
        REQUIRE(color::FromString("rEd") == colors::Red);
        REQUIRE(color::FromString("Blue") == colors::Blue);
        REQUIRE(color::FromString("RebeccaPurple") == colors::RebeccaPurple);

        REQUIRE(color::FromString("#FF00FF") == color {0XFF, 0X00, 0XFF, 0XFF});
        REQUIRE(color::FromString("#ABCD01") == color {0xAB, 0XCD, 0X01, 255});
        REQUIRE(color::FromString("#FFAABB14") == color {0XFF, 0XAA, 0XBB, 0X14});
        REQUIRE(color::FromString("#01020304") == color {1, 2, 3, 4});

        REQUIRE(color::FromString("#123") == color {0X11, 0X22, 0X33});
        REQUIRE(color::FromString("#FAB") == color {0XFF, 0XAA, 0XBB});

        REQUIRE(color::FromString("#1236") == color {0X11, 0X22, 0X33, 0x66});
        REQUIRE(color::FromString("#FAB9") == color {0XFF, 0XAA, 0XBB, 0x99});
    }

    SUBCASE("Literals")
    {
        using namespace tcob::literals;
        REQUIRE(color::FromString("Red") == "Red"_color);
        REQUIRE(color::FromString("Blue") == "Blue"_color);
        REQUIRE(color::FromString("RebeccaPurple") == "RebeccaPurple"_color);
    }

    SUBCASE("HSLA")
    {
        REQUIRE(color::FromHSLA({degree_f {0}, 0, 0}) == colors::Black);
        REQUIRE(color::FromHSLA({degree_f {240}, 1.f, 0.5f}) == colors::Blue);
        REQUIRE(color::FromHSLA({degree_f {0}, 1.f, 0.5f}) == colors::Red);
        REQUIRE(color::FromHSLA({degree_f {43}, 0.75, 0.5}) == color {223, 169, 32});

        REQUIRE(hsx {degree_f {0}, 0, 0} == colors::Black.to_hsl());
        REQUIRE(hsx {degree_f {240}, 1.f, 0.5f} == colors::Blue.to_hsl());
        REQUIRE(hsx {degree_f {0}, 1.f, 0.5f} == colors::Red.to_hsl());
    }

    SUBCASE("HSVA")
    {
        REQUIRE(color::FromHSVA({degree_f {0}, 0, 0}) == colors::Black);
        REQUIRE(color::FromHSVA({degree_f {240}, 1.f, 1.f}) == colors::Blue);
        REQUIRE(color::FromHSVA({degree_f {0}, 1.f, 1.f}) == colors::Red);
        REQUIRE(color::FromHSVA({degree_f {43}, 0.8565f, 0.851f}) == color {217, 164, 31});
        REQUIRE(color::FromHSVA({degree_f {45}, 0.5f, 0.75f}) == color::FromString("#BFA760"));
        REQUIRE(color::FromHSVA({degree_f {90}, 0.5f, 0.75f}) == color::FromString("#8FBF60"));
        REQUIRE(color::FromHSVA({degree_f {135}, 0.5f, 0.75f}) == color::FromString("#60BF78"));
        REQUIRE(color::FromHSVA({degree_f {180}, 0.5f, 0.75f}) == color::FromString("#60BFBF"));
        REQUIRE(color::FromHSVA({degree_f {225}, 0.5f, 0.75f}) == color::FromString("#6078BF"));
        REQUIRE(color::FromHSVA({degree_f {270}, 0.5f, 0.75f}) == color::FromString("#8F60BF"));
        REQUIRE(color::FromHSVA({degree_f {315}, 0.5f, 0.75f}) == color::FromString("#BF60A7"));

        REQUIRE(hsx {degree_f {0}, 0, 0} == colors::Black.to_hsv());
        REQUIRE(hsx {degree_f {240}, 1.f, 1.f} == colors::Blue.to_hsv());
        REQUIRE(hsx {degree_f {0}, 1.f, 1.f} == colors::Red.to_hsv());
    }

    SUBCASE("Format")
    {
        REQUIRE(std::format("{}", colors::SandyBrown) == "(r:244,g:164,b:96,a:255)");
    }
}

TEST_CASE("GFX.ColorGradient.Basic")
{
    SUBCASE("single color")
    {
        color_gradient grad {colors::Green};
        auto const     cols {grad.colors()};
        for (auto const& c : cols) {
            REQUIRE(c == colors::Green);
        }
    }

    SUBCASE("two colors grayscale")
    {
        color_gradient grad {colors::Black, colors::White};
        auto const     cols {grad.colors()};
        for (usize i {0}; i < cols.size(); ++i) {
            u8 val = static_cast<u8>(i);
            REQUIRE(cols[i] == color {val, val, val, 255});
        }

        color_gradient grad2 {colors::Black, colors::White};
        REQUIRE(grad == grad2);
    }

    SUBCASE("two colors red to blue")
    {
        color_gradient grad {colors::Red, colors::Blue};
        auto const     cols {grad.colors()};
        REQUIRE(cols[0] == color {255, 0, 0, 255});
        REQUIRE(cols[127] == color {127, 0, 127, 255});
        REQUIRE(cols[255] == color {0, 0, 255, 255});
    }

    SUBCASE("three stops")
    {
        color_gradient grad {colors::Red, colors::Green, colors::Blue};
        auto const     cols {grad.colors()};

        // First stop
        REQUIRE(cols[0] == colors::Red);
        // Middle stop (should be Green at ~127)
        REQUIRE(cols[127] == colors::Green);
        // Last stop
        REQUIRE(cols[255] == colors::Blue);
    }

    SUBCASE("premultiplied alpha")
    {
        color          semiRed {255, 0, 0, 128};
        color_gradient grad {semiRed, colors::Transparent};
        auto const     cols {grad.colors(true)};

        // First color should be premultiplied red
        REQUIRE(cols[0] == semiRed.as_alpha_premultiplied());
        // Last color should be transparent (all zeros)
        REQUIRE(cols[255] == colors::Transparent);
    }

    SUBCASE("inequality")
    {
        color_gradient grad1 {colors::Red, colors::Blue};
        color_gradient grad2 {colors::Red, colors::Green};
        REQUIRE_FALSE(grad1 == grad2);
    }
}
