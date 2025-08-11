#include "tests.hpp"

TEST_CASE("GFX.UI.Styles")
{
    style_flags sfl_hover {.Hover = true};
    style_flags sfl_focus {.Focus = true};
    style_flags sfl_focus_hover {.Focus = true, .Hover = true};
    style_flags sfl_active {.Active = true};

    widget_flags fl_hover {.Focus = false, .Active = false, .Hover = true, .Checked = false, .Disabled = false};
    widget_flags fl_focus {.Focus = true, .Active = false, .Hover = false, .Checked = false, .Disabled = false};
    widget_flags fl_focus_hover {.Focus = true, .Active = false, .Hover = true, .Checked = false, .Disabled = false};
    widget_flags fl_active {.Focus = false, .Active = true, .Hover = false, .Checked = false, .Disabled = false};
    widget_flags fl_active_hover {.Focus = false, .Active = true, .Hover = true, .Checked = false, .Disabled = false};

    std::string name = "button";

    SUBCASE("flags 1")
    {
        style_collection s;
        // button
        auto             button  = s.create<widget_style>(name, {});
        button->Background       = colors::Black;
        // button:hover
        auto button_hover        = s.create<widget_style>(name, sfl_hover);
        button_hover->Background = colors::White;

        // button:focus > button
        REQUIRE(std::get<color>(dynamic_cast<widget_style*>(s.get({name, fl_focus, {}}))->Background) == std::get<color>(button->Background));
        // button:focus:hover > button:hover
        REQUIRE(std::get<color>(dynamic_cast<widget_style*>(s.get({name, fl_focus_hover, {}}))->Background) == std::get<color>(button_hover->Background));
        // button:active:hover > button:hover
        REQUIRE(std::get<color>(dynamic_cast<widget_style*>(s.get({name, fl_active_hover, {}}))->Background) == std::get<color>(button_hover->Background));
    }
    SUBCASE("flags 2")
    {
        style_collection s;
        // button
        auto             button        = s.create<widget_style>(name, {});
        button->Background             = colors::Black;
        // button:focus:hover
        auto button_focus_hover        = s.create<widget_style>(name, sfl_focus_hover);
        button_focus_hover->Background = colors::Red;

        // button:focus > button
        REQUIRE(std::get<color>(dynamic_cast<widget_style*>(s.get({name, fl_focus, {}}))->Background) == std::get<color>(button->Background));
        // button:hover > button
        REQUIRE(std::get<color>(dynamic_cast<widget_style*>(s.get({name, fl_hover, {}}))->Background) == std::get<color>(button->Background));
        // button:focus:hover >  button:focus:hover
        REQUIRE(std::get<color>(dynamic_cast<widget_style*>(s.get({name, fl_focus_hover, {}}))->Background) == std::get<color>(button_focus_hover->Background));
        // button:active:hover > button
        REQUIRE(std::get<color>(dynamic_cast<widget_style*>(s.get({name, fl_active_hover, {}}))->Background) == std::get<color>(button->Background));
    }
    SUBCASE("flags 3")
    {
        style_collection s;
        // button
        auto             button   = s.create<widget_style>(name, {});
        button->Background        = colors::Black;
        // button:active
        auto button_active        = s.create<widget_style>(name, sfl_active);
        button_active->Background = colors::White;
        // button:focus
        auto button_focus         = s.create<widget_style>(name, sfl_focus);
        button_focus->Background  = colors::Red;
        // button:hover
        auto button_hover         = s.create<widget_style>(name, sfl_hover);
        button_hover->Background  = colors::Yellow;

        // button:hover > button:hover
        REQUIRE(std::get<color>(dynamic_cast<widget_style*>(s.get({name, fl_hover, {}}))->Background) == std::get<color>(button_hover->Background));
        // button:focus:hover > button:hover
        REQUIRE(std::get<color>(dynamic_cast<widget_style*>(s.get({name, fl_focus_hover, {}}))->Background) == std::get<color>(button_hover->Background));
        // button:active:hover > button:hover
        REQUIRE(std::get<color>(dynamic_cast<widget_style*>(s.get({name, fl_active_hover, {}}))->Background) == std::get<color>(button_hover->Background));
    }

    SUBCASE("attributes 1")
    {
        style_collection s;

        auto incHoverStyle {s.create<button>(name, sfl_hover, {{"inc_button", {{rule::Equal(true)}}}})};
        incHoverStyle->Background = color {5, 5, 5, 5};
        auto incActiveStyle {s.create<button>(name, sfl_active, {{"inc_button", {{rule::Equal(true)}}}})};
        incActiveStyle->Background = color {6, 6, 6, 6};

        auto decHoverStyle {s.create<button>(name, sfl_hover, {{"dec_button", {{rule::Equal(true)}}}})};
        decHoverStyle->Background = color {7, 7, 7, 7};
        auto decActiveStyle {s.create<button>(name, sfl_active, {{"dec_button", {{rule::Equal(true)}}}})};
        decActiveStyle->Background = color {8, 8, 8, 8};

        auto style {s.create<button>(name, {})};
        style->Background = color {1, 1, 1, 1};

        auto hoverStyle {s.create<button>(name, sfl_hover)};
        style->Background = color {3, 3, 3, 3};

        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get({name, fl_hover, {{"inc_button", true}}}))->Background)
                == std::get<color>(incHoverStyle->Background));
        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get({name, fl_active, {{"inc_button", true}}}))->Background)
                == std::get<color>(incActiveStyle->Background));

        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get({name, fl_hover, {{"dec_button", true}}}))->Background)
                == std::get<color>(decHoverStyle->Background));
        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get({name, fl_active, {{"dec_button", true}}}))->Background)
                == std::get<color>(decActiveStyle->Background));
    }

    SUBCASE("attributes 2")
    {
        style_collection s;
        auto             style0 {s.create<button>(name, {})};
        style0->Background = color {1, 1, 1, 1};

        auto style1 {s.create<button>(name, sfl_hover)};
        *style1            = *style0;
        style0->Background = color {3, 3, 3, 3};

        auto style2 {s.create<button>(name, sfl_hover, {{"value", {rule::GreaterEqual(80), rule::Less(100)}}})};
        *style2            = *style1;
        style2->Background = color {5, 5, 5, 5};

        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get({name, fl_hover, {{"value", 70}}}))->Background)
                == std::get<color>(style1->Background));
        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get({name, fl_hover, {{"value", 80}}}))->Background)
                == std::get<color>(style2->Background));
        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get({name, fl_hover, {{"value", 90}}}))->Background)
                == std::get<color>(style2->Background));
        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get({name, fl_hover, {{"value", 100}}}))->Background)
                == std::get<color>(style1->Background));
        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get({name, fl_hover, {{"value", 110}}}))->Background)
                == std::get<color>(style1->Background));
    }

    SUBCASE("attributes 3")
    {
        style_collection s;
        auto             style0 {s.create<button>(name, {})};
        style0->Background = color {1, 1, 1, 1};

        auto style1 {s.create<button>(name, sfl_hover)};
        *style1            = *style0;
        style0->Background = color {3, 3, 3, 3};

        auto style2 {s.create<button>(name, sfl_hover, {{"orientation", {rule::Equal(orientation::Horizontal)}}})};
        *style2            = *style1;
        style2->Background = color {5, 5, 5, 5};

        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get({name, fl_hover, {{"orientation", orientation::Vertical}}}))->Background)
                == std::get<color>(style1->Background));
        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get({name, fl_hover, {{"orientation", orientation::Horizontal}}}))->Background)
                == std::get<color>(style2->Background));
    }

    SUBCASE("attributes 4")
    {
        style_collection s;
        auto             style0 {s.create<button>(name, {})};
        style0->Background = color {1, 1, 1, 1};

        auto style1 {s.create<button>(name, sfl_hover)};
        *style1            = *style0;
        style0->Background = color {3, 3, 3, 3};

        auto style2 {s.create<button>(name, sfl_hover,
                                      {{"value", {rule::Equal(100)}},
                                       {"orientation", {rule::Equal(orientation::Horizontal)}}})};
        *style2            = *style1;
        style2->Background = color {5, 5, 5, 5};

        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get({name, fl_hover, {{"value", 100}, {"orientation", orientation::Vertical}}}))->Background)
                == std::get<color>(style1->Background));
        REQUIRE(std::get<color>(dynamic_cast<button::style*>(s.get({name, fl_hover, {{"value", 100}, {"orientation", orientation::Horizontal}}}))->Background)
                == std::get<color>(style2->Background));
    }
}
