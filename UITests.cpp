#include "tests.hpp"

TEST_CASE("GFX.UI.Bounds")
{
    using namespace tcob::literals;

    form form0 {"", nullptr, {60, 80, 800, 700}};
    auto panel0 {form0.create_container<panel>(dock_style::Fill, "Panel0")};
    auto panel0Layout {panel0->get_layout<fixed_layout>()};
    panel0->Flex = {100_pct, 100_pct};
    auto button0 {panel0Layout->create_widget<button>({10, 20, 200, 100}, "button0")};

    auto cPanel0 {panel0Layout->create_widget<panel>({230, 280, 250, 500}, "cPanel0")};
    auto cPanel0Layout0 {cPanel0->create_layout<fixed_layout>()};
    auto cButton0 {cPanel0Layout0->create_widget<button>({5, 50, 200, 100}, "button0")};

    style_collection& styles {*form0.Styles};

    auto buttonStyle {styles.create<button>("button", {})};
    buttonStyle->Border.Size = 2_px;
    buttonStyle->Margin      = {4_px};
    buttonStyle->Padding     = {8_px};

    auto panelStyle {styles.create<panel>("panel", {})};
    panelStyle->Border.Size = 10_px;
    panelStyle->Margin      = {20_px};
    panelStyle->Padding     = {40_px};

    form0.fixed_update(milliseconds {0});

    REQUIRE(panel0->global_position() == point_f {60, 80});
    REQUIRE(button0->global_position() == point_f {135, 165});
    REQUIRE(cPanel0->global_position() == point_f {355, 425});
    REQUIRE(cButton0->global_position() == point_f {425, 540});

    REQUIRE(panel0->content_bounds() == rect_f {65, 65, 670, 570});
    REQUIRE(button0->content_bounds() == rect_f {23, 33, 174, 74});
    REQUIRE(cPanel0->content_bounds() == rect_f {295, 345, 120, 370});
    REQUIRE(cButton0->content_bounds() == rect_f {18, 63, 174, 74});

    REQUIRE(panel0->global_content_bounds() == rect_f {125, 145, 670, 570});
    REQUIRE(button0->global_content_bounds() == rect_f {148, 178, 174, 74});
    REQUIRE(cPanel0->global_content_bounds() == rect_f {420, 490, 120, 370});
    REQUIRE(cButton0->global_content_bounds() == rect_f {438, 553, 174, 74});

    REQUIRE(panel0->hit_test_bounds() == rect_f {80, 100, 760, 660});
    REQUIRE(button0->hit_test_bounds() == rect_f {139, 169, 192, 92});
    REQUIRE(cPanel0->hit_test_bounds() == rect_f {375, 445, 210, 270});
    REQUIRE(cButton0->hit_test_bounds() == rect_f {429, 544, 111, 92});
}
