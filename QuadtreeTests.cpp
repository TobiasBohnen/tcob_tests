#include "tests.hpp"

struct foo_point {
    point_f Position;
    auto    operator==(foo_point const&) const -> bool = default; // NOLINT
};
auto const get_bounds_point {[](foo_point const& f) -> rect_f { return {f.Position, size_f::One}; }};
template <>
struct std::hash<foo_point> {
    auto operator()(foo_point const& p) const -> std::size_t
    {
        std::size_t h1 = std::hash<float> {}(p.Position.X);
        std::size_t h2 = std::hash<float> {}(p.Position.Y);
        return h1 ^ (h2 << 1);
    }
};

struct foo_rect {
    rect_f Bounds;
    auto   operator==(foo_rect const&) const -> bool = default; // NOLINT
};
auto const get_bounds_rect {[](foo_rect const& f) { return f.Bounds; }};
template <>
struct std::hash<foo_rect> {
    auto operator()(foo_rect const& r) const -> std::size_t
    {
        std::size_t h1 = std::hash<float> {}(r.Bounds.X);
        std::size_t h2 = std::hash<float> {}(r.Bounds.Y);
        std::size_t h3 = std::hash<float> {}(r.Bounds.Width);
        std::size_t h4 = std::hash<float> {}(r.Bounds.Height);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

auto static CheckQuery(auto&& vec, auto&& exp)
{
    std::unordered_set set(vec.begin(), vec.end());
    REQUIRE(set.size() == vec.size());
    for (auto const& expVal : exp) {
        REQUIRE(set.contains(expVal));
    }
}

TEST_CASE("GFX.Quadtree.Query")
{
    SUBCASE("point_f")
    {
        quadtree<foo_point, +get_bounds_point, 2> tree {rect_f {0, 0, 100, 100}};
        REQUIRE(tree.add(foo_point {{5.f, 5.f}}));
        REQUIRE(tree.add(foo_point {{5.f, 25.f}}));
        REQUIRE(tree.add(foo_point {{55.f, 5.f}}));
        REQUIRE(tree.add(foo_point {{45.f, 25.f}}));
        REQUIRE(tree.add(foo_point {{75.f, 75.f}}));
        REQUIRE(tree.add(foo_point {{15.f, 15.f}}));
        REQUIRE(tree.add(foo_point {{95.f, 95.f}}));
        REQUIRE(tree.add(foo_point {{35.f, 35.f}}));
        auto q = tree.query({5.f, 0.f, 10.f, 30.f});
        CheckQuery(q, std::vector<foo_point> {foo_point {{5.f, 5.f}}, foo_point {{5.f, 25.f}}});

        q = tree.query({10.f, 10.f, 40.f, 40.f});

        CheckQuery(q, std::vector<foo_point> {foo_point {{45.f, 25.f}}, foo_point {{15.f, 15.f}}, foo_point {{35.f, 35.f}}});
    }

    SUBCASE("rect_f")
    {
        quadtree<foo_rect, +get_bounds_rect, 2> tree {rect_f {0, 0, 100, 100}};
        REQUIRE(tree.add(foo_rect {{5.f, 5.f, 5.f, 5.f}}));
        REQUIRE(tree.add(foo_rect {{5.f, 25.f, 5.f, 5.f}}));
        REQUIRE(tree.add(foo_rect {{55.f, 5.f, 5.f, 5.f}}));
        REQUIRE(tree.add(foo_rect {{45.f, 25.f, 5.f, 5.f}}));
        REQUIRE(tree.add(foo_rect {{75.f, 75.f, 5.f, 5.f}}));
        REQUIRE(tree.add(foo_rect {{15.f, 15.f, 5.f, 5.f}}));
        REQUIRE(tree.add(foo_rect {{94.f, 94.f, 5.f, 5.f}}));
        REQUIRE(tree.add(foo_rect {{35.f, 35.f, 5.f, 5.f}}));
        auto q = tree.query({5.f, 0.f, 10.f, 30.f});
        CheckQuery(q, std::vector<foo_rect> {foo_rect {{5.f, 5.f, 5.f, 5.f}}, foo_rect {{5.f, 25.f, 5.f, 5.f}}});

        // Additional query
        q = tree.query({10.f, 10.f, 40.f, 40.f});
        CheckQuery(q, std::vector<foo_rect> {foo_rect {{45.f, 25.f, 5.f, 5.f}}, foo_rect {{15.f, 15.f, 5.f, 5.f}}, foo_rect {{35.f, 35.f, 5.f, 5.f}}});
    }
}

TEST_CASE("GFX.Quadtree.Remove")
{
    {
        quadtree<foo_rect, +get_bounds_rect> tree {rect_f {0, 0, 100, 100}};
        foo_rect                             fr0 {{5.f, 5.f, 5.f, 5.f}};
        foo_rect                             fr1 {{5.f, 25.f, 5.f, 5.f}};
        REQUIRE(tree.add(fr0));
        REQUIRE(tree.add(fr1));
        auto q = tree.query({5.f, 0.f, 10.f, 30.f});
        CheckQuery(q, std::vector<foo_rect> {fr0, fr1});
        REQUIRE(tree.remove(fr1));

        q = tree.query({5.f, 0.f, 10.f, 30.f});
        CheckQuery(q, std::vector<foo_rect> {fr0});
    }
}

TEST_CASE("GFX.Quadtree.Replace")
{
    {
        quadtree<foo_rect, +get_bounds_rect, 2> tree {rect_f {0, 0, 100, 100}};
        foo_rect                                fr0 {{5.f, 5.f, 5.f, 5.f}};
        foo_rect                                fr1 {{5.f, 25.f, 5.f, 5.f}};
        foo_rect                                fr2 {{5.f, 25.f, 2.f, 2.f}};
        foo_rect                                fr3 {{55.f, 5.f, 5.f, 5.f}};
        foo_rect                                fr4 {{45.f, 45.f, 5.f, 5.f}};
        foo_rect                                fr5 {{10.f, 10.f, 3.f, 3.f}};
        foo_rect                                fr6 {{60.f, 60.f, 6.f, 6.f}};
        foo_rect                                fr7 {{75.f, 25.f, 4.f, 4.f}};

        REQUIRE(tree.add(fr0));
        REQUIRE(tree.add(fr1));
        REQUIRE(tree.add(fr3));
        REQUIRE(tree.add(fr4));
        REQUIRE(tree.add(fr5));
        REQUIRE(tree.add(fr6));
        REQUIRE(tree.add(fr7));

        auto q = tree.query({5.f, 0.f, 10.f, 30.f});
        CheckQuery(q, std::vector<foo_rect> {fr0, fr1, fr5});

        REQUIRE(tree.replace(fr1, fr2));

        q = tree.query({5.f, 0.f, 10.f, 30.f});
        CheckQuery(q, std::vector<foo_rect> {fr0, fr2, fr5});

        foo_rect fr8 {{8.f, 8.f, 4.f, 4.f}};
        REQUIRE(tree.replace(fr5, fr8));

        q = tree.query({5.f, 0.f, 10.f, 30.f});
        CheckQuery(q, std::vector<foo_rect> {fr0, fr2, fr8});

        auto all = tree.query({0.f, 0.f, 100.f, 100.f});
        CheckQuery(all, std::vector<foo_rect> {fr0, fr2, fr3, fr4, fr8, fr6, fr7});
    }
    {
        quadtree<foo_rect, +get_bounds_rect, 2> tree {rect_f {0, 0, 100, 100}};
        foo_rect                                fr0 {{5.f, 5.f, 5.f, 5.f}};
        foo_rect                                fr1 {{25.f, 25.f, 5.f, 5.f}};
        foo_rect                                fr2 {{55.f, 5.f, 5.f, 5.f}};
        foo_rect                                fr3 {{45.f, 45.f, 5.f, 5.f}};
        foo_rect                                fr4 {{70.f, 70.f, 5.f, 5.f}};

        REQUIRE(tree.add(fr0));
        REQUIRE(tree.add(fr1));
        REQUIRE(tree.add(fr2));
        REQUIRE(tree.add(fr3));
        REQUIRE(tree.add(fr4));

        auto q = tree.query({0.f, 0.f, 50.f, 50.f});
        CheckQuery(q, std::vector<foo_rect> {fr0, fr1, fr3});

        foo_rect fr5 {{60.f, 60.f, 5.f, 5.f}};
        REQUIRE(tree.replace(fr1, fr5));

        q = tree.query({0.f, 0.f, 50.f, 50.f});
        CheckQuery(q, std::vector<foo_rect> {fr0, fr3});

        q = tree.query({50.f, 50.f, 50.f, 50.f});
        CheckQuery(q, std::vector<foo_rect> {fr4, fr5});

        auto all = tree.query({0.f, 0.f, 100.f, 100.f});
        CheckQuery(all, std::vector<foo_rect> {fr0, fr2, fr3, fr4, fr5});
    }
}
