#include "tests.hpp"

TEST_CASE("Core.Quadtree.Rect")
{
    struct foo {
        rect_f Bounds;
        auto   operator==(foo const&) const -> bool = default; // NOLINT
    };

    auto const get_bounds {[](foo const& f) { return f.Bounds; }};

    quadtree<foo, +get_bounds> tree {rect_f {0, 0, 100, 100}};
    tree.add(foo {{5.f, 5.f, 5.f, 5.f}});
    tree.add(foo {{5.f, 25.f, 5.f, 5.f}});
    tree.add(foo {{55.f, 5.f, 5.f, 5.f}});
    tree.add(foo {{45.f, 25.f, 5.f, 5.f}});
    auto q = tree.query({5.f, 0.f, 10.f, 30.f});
    REQUIRE(q == std::vector<foo> {foo {{5.f, 5.f, 5.f, 5.f}}, foo {{5.f, 25.f, 5.f, 5.f}}});
}

TEST_CASE("Core.Quadtree.Point")
{
    struct foo {
        point_f Position;
        auto    operator==(foo const&) const -> bool = default; // NOLINT
    };

    auto const get_bounds {[](foo const& f) -> rect_f { return {f.Position, size_f::One}; }};

    quadtree<foo, +get_bounds> tree {rect_f {0, 0, 100, 100}};
    tree.add(foo {{5.f, 5.f}});
    tree.add(foo {{5.f, 25.f}});
    tree.add(foo {{55.f, 5.f}});
    tree.add(foo {{45.f, 25.f}});
    auto q = tree.query({5.f, 0.f, 10.f, 30.f});
    REQUIRE(q == std::vector<foo> {foo {{5.f, 5.f}}, foo {{5.f, 25.f}}});
}
