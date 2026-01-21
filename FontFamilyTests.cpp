#include "tests.hpp"

TEST_CASE("GFX.FontFamily.Fallback")
{
    SUBCASE("normal -> medium")
    {
        string fontName {"testFont0"};
        PrepareFile("/fonts/" + fontName + "-Medium.ttf");
        PrepareFile("/fonts/" + fontName + "-Light.ttf");
        PrepareFile("/fonts/" + fontName + "-Bold.ttf");

        font_family ff {fontName};
        font_family::FindSources(ff, "/fonts/" + fontName);
        auto style {ff.get_style({.IsItalic = false, .Weight = font::weight::Normal})};
        REQUIRE(style);
        REQUIRE(style->Weight == font::weight::Medium);

        io::delete_folder("fonts");
    }

    SUBCASE("medium -> normal")
    {
        string fontName {"testFont1"};
        PrepareFile("/fonts/" + fontName + ".ttf");
        PrepareFile("/fonts/" + fontName + "-Light.ttf");
        PrepareFile("/fonts/" + fontName + "-Bold.ttf");

        font_family ff {fontName};
        font_family::FindSources(ff, "/fonts/" + fontName);
        auto style {ff.get_style({.IsItalic = false, .Weight = font::weight::Medium})};
        REQUIRE(style);
        REQUIRE(style->Weight == font::weight::Normal);

        io::delete_folder("fonts");
    }

    SUBCASE("extra light -> light")
    {
        string fontName {"testFont2"};
        PrepareFile("/fonts/" + fontName + ".ttf");
        PrepareFile("/fonts/" + fontName + "-Light.ttf");
        PrepareFile("/fonts/" + fontName + "-Bold.ttf");

        font_family ff {fontName};
        font_family::FindSources(ff, "/fonts/" + fontName);
        auto style {ff.get_style({.IsItalic = false, .Weight = font::weight::ExtraLight})};
        REQUIRE(style);
        REQUIRE(style->Weight == font::weight::Light);

        io::delete_folder("fonts");
    }

    SUBCASE("extra bold -> semi bold")
    {
        string fontName {"testFont3"};
        PrepareFile("/fonts/" + fontName + ".ttf");
        PrepareFile("/fonts/" + fontName + "-Light.ttf");
        PrepareFile("/fonts/" + fontName + "-Medium.ttf");
        PrepareFile("/fonts/" + fontName + "-SemiBold.ttf");

        font_family ff {fontName};
        font_family::FindSources(ff, "/fonts/" + fontName);
        auto style {ff.get_style({.IsItalic = false, .Weight = font::weight::ExtraBold})};
        REQUIRE(style);
        REQUIRE(style->Weight == font::weight::SemiBold);

        io::delete_folder("fonts");
    }
}
