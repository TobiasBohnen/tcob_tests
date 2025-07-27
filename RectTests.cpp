#include "tests.hpp"

TEST_CASE("Core.POD.Rect")
{
    static_assert(std::is_copy_constructible_v<rect_f>);
    static_assert(std::is_copy_assignable_v<rect_f>);
    static_assert(std::is_move_constructible_v<rect_f>);
    static_assert(std::is_move_assignable_v<rect_f>);
    static_assert(POD<rect_f>);

    SUBCASE("Construction")
    {
        // default constructor
        {
            rect_f r;
            REQUIRE(r.left() == 0.f);
            REQUIRE(r.top() == 0.f);
            REQUIRE(r.width() == 0.f);
            REQUIRE(r.height() == 0.f);
        }
        {
            rect_i r;
            REQUIRE(r.left() == 0);
            REQUIRE(r.top() == 0);
            REQUIRE(r.width() == 0);
            REQUIRE(r.height() == 0);
        }
        {
            rect_u r;
            REQUIRE(r.left() == 0);
            REQUIRE(r.top() == 0);
            REQUIRE(r.width() == 0);
            REQUIRE(r.height() == 0);
        }
        // x,y,w,h constructor
        {
            f32    x {2.45f};
            f32    y {4.21f};
            f32    w {12.15f};
            f32    h {34.22f};
            rect_f r {x, y, w, h};
            REQUIRE(r.left() == x);
            REQUIRE(r.top() == y);
            REQUIRE(r.width() == w);
            REQUIRE(r.height() == h);
        }
        {
            i32    x {2};
            i32    y {4};
            i32    w {12};
            i32    h {45};
            rect_i r {x, y, w, h};
            REQUIRE(r.left() == x);
            REQUIRE(r.top() == y);
            REQUIRE(r.width() == w);
            REQUIRE(r.height() == h);
        }
        {
            u32    x {2};
            u32    y {4};
            u32    w {12};
            u32    h {45};
            rect_u r {x, y, w, h};
            REQUIRE(r.left() == x);
            REQUIRE(r.top() == y);
            REQUIRE(r.width() == w);
            REQUIRE(r.height() == h);
        }
        // copy constructor
        {
            rect_u p1 {2, 4, 3, 4};
            rect_u p2 {p1};
            REQUIRE(p1 == p2);
        }
        {
            rect_i p1 {2, 4, 3, 4};
            rect_u p2 {p1};
            REQUIRE(p1.left() == p2.left());
            REQUIRE(p1.top() == p2.top());
            REQUIRE(p1.width() == p2.width());
            REQUIRE(p1.height() == p2.height());
        }
    }

    SUBCASE("Equality")
    {
        {
            rect_f r1;
            rect_f r2;
            REQUIRE(r1 == r2);
        }
        {
            rect_i r1;
            rect_i r2;
            REQUIRE(r1 == r2);
        }
        {
            rect_u r1;
            rect_u r2;
            REQUIRE(r1 == r2);
        }

        {
            f32    x {2.45f};
            f32    y {4.21f};
            f32    w {12.45f};
            f32    h {44.21f};
            rect_f r1 {x, y, w, h};
            rect_f r2 {x, y, w, h};
            REQUIRE(r1 == r2);
        }
        {
            i32    x {2};
            i32    y {4};
            i32    w {12};
            i32    h {44};
            rect_i r1 {x, y, w, h};
            rect_i r2 {x, y, w, h};
            REQUIRE(r1 == r2);
        }
        {
            u32    x {2};
            u32    y {4};
            u32    w {12};
            u32    h {44};
            rect_u r1 {x, y, w, h};
            rect_u r2 {x, y, w, h};
            REQUIRE(r1 == r2);
        }

        {
            rect_u r1 {0, 1, 2, 3};
            rect_u r2 {0, 1, 2, 99};
            REQUIRE(r1 != r2);
        }
        {
            rect_u r1 {0, 1, 2, 3};
            rect_u r2 {0, 1, 99, 3};
            REQUIRE(r1 != r2);
        }
        {
            rect_u r1 {0, 1, 2, 3};
            rect_u r2 {0, 99, 2, 3};
            REQUIRE(r1 != r2);
        }
        {
            rect_u r1 {0, 1, 2, 3};
            rect_u r2 {99, 1, 2, 3};
            REQUIRE(r1 != r2);
        }
        {
            f32 const x0 {2.45f};
            f32 const tol {0.1f};
            f32 const x1 {x0 + tol};
            f32 const x2 {x0 - tol};
            f32 const x3 {x0 + tol + std::numeric_limits<f32>::epsilon()};
            f32 const x4 {x0 - tol - std::numeric_limits<f32>::epsilon()};
            REQUIRE(rect_f {0, 0, 0, 0}.equals(rect_f {tol, 0, 0, 0}, tol));
            REQUIRE(rect_f {x0, 0, 0, 0}.equals(rect_f {x1, 0, 0, 0}, tol));
            REQUIRE(rect_f {0, x0, 0, 0}.equals(rect_f {0, x1, 0, 0}, tol));
            REQUIRE(rect_f {0, 0, x0, 0}.equals(rect_f {0, 0, x1, 0}, tol));
            REQUIRE(rect_f {0, 0, 0, x0}.equals(rect_f {0, 0, 0, x1}, tol));
            REQUIRE(rect_f {x0, 0, 0, 0}.equals(rect_f {x2, 0, 0, 0}, tol));
            REQUIRE_FALSE(rect_f {x0, 0, 0, 0}.equals(rect_f {x3, 0, 0, 0}, tol));
            REQUIRE_FALSE(rect_f {x0, 0, 0, 0}.equals(rect_f {x4, 0, 0, 0}, tol));
        }
    }

    SUBCASE("Contains")
    {
        {
            f32    x {0.5f};
            f32    y {2.5f};
            f32    w {12.5f};
            f32    h {3.5f};
            rect_f r {x, y, w, h};
            REQUIRE(r.contains(point_f {0.75f, 3.9f}));
            REQUIRE_FALSE(r.contains(point_f {0.25f, 3.9f}));
            REQUIRE_FALSE(r.contains(point_f {0.75f, 7.9f}));
        }
        {
            i32    x {2};
            i32    y {4};
            i32    w {12};
            i32    h {45};
            rect_i r {x, y, w, h};
            REQUIRE(r.contains(point_f {3, 5}));
            REQUIRE_FALSE(r.contains(point_f {1, 17}));
            REQUIRE_FALSE(r.contains(point_f {15, 5}));
        }
        {
            i32    x {0};
            i32    y {0};
            i32    w {10};
            i32    h {10};
            rect_i r {x, y, w, h};
            REQUIRE(r.contains(point_i {0, 0}));
            REQUIRE_FALSE(r.contains(point_i {0, 10}));
            REQUIRE_FALSE(r.contains(point_i {10, 0}));
            REQUIRE_FALSE(r.contains(point_i {10, 10}));
        }
        {
            i32    x {2};
            i32    y {4};
            i32    w {12};
            i32    h {45};
            rect_i r {x, y, w, h};
            REQUIRE(r.contains(rect_i {3, 5, 1, 1}));
            REQUIRE_FALSE(r.contains(rect_i {3, 5, 20, 20}));
        }
    }

    SUBCASE("Intersects")
    {
        {
            rect_f r1 {0.5f, 1.5f, 2.5f, 3.5f};
            rect_f r2 {0.35f, 0.5f, 2.5f, 3.5f};
            REQUIRE(r1.intersects(r2));
            REQUIRE(r2.intersects(r1));
        }
        {
            rect_f r1 {0.5f, 1.5f, 2.5f, 3.5f};
            rect_f r2 {0.5f, 5.1f, 2.5f, 3.5f};
            REQUIRE_FALSE(r1.intersects(r2));
            REQUIRE_FALSE(r2.intersects(r1));
        }
        {
            rect_f r1 {0.5f, 1.5f, 3.0f, 3.5f};
            rect_f r2 {2.0f, 2.0f, 2.5f, 4.0f};
            REQUIRE(r1.intersects(r2));
            REQUIRE(r2.intersects(r1));
        }

        {
            rect_f r1 {1.0f, 1.5f, 3.0f, 3.5f};
            rect_f r2 {0.5f, 2.0f, 2.5f, 4.0f};
            REQUIRE(r1.intersects(r2));
            REQUIRE(r2.intersects(r1));
        }

        {
            rect_f r1 {0.5f, 1.5f, 3.0f, 3.5f};
            rect_f r2 {1.0f, 2.0f, 2.5f, 4.0f};
            REQUIRE(r1.intersects(r2));
            REQUIRE(r2.intersects(r1));
        }

        {
            rect_f r1 {0.5f, 1.5f, 3.0f, 3.5f};
            rect_f r2 {1.0f, 1.0f, 2.5f, 2.5f};
            REQUIRE(r1.intersects(r2));
            REQUIRE(r2.intersects(r1));
        }

        {
            rect_f r1 {0.5f, 1.5f, 2.5f, 3.5f};
            rect_f r2 {0.5f, 1.5f, 2.5f, 3.5f};
            REQUIRE(r1.intersects(r2));
            REQUIRE(r2.intersects(r1));
        }

        {
            rect_f r1 {0.0f, 0.0f, 3.0f, 3.0f};
            rect_f r2 {2.0f, 2.0f, 3.0f, 3.0f};
            REQUIRE(r1.intersects(r2));
            REQUIRE(r2.intersects(r1));
        }
        {
            rect_f r1 {0.5f, 1.5f, 2.5f, 3.5f};
            rect_f r2 {3.5f, 1.5f, 2.5f, 3.5f};
            REQUIRE_FALSE(r1.intersects(r2));
            REQUIRE_FALSE(r2.intersects(r1));
        }

        {
            rect_f r1 {0.5f, 0.5f, 2.5f, 3.5f};
            rect_f r2 {0.5f, 4.0f, 2.5f, 1.5f};
            REQUIRE_FALSE(r1.intersects(r2));
            REQUIRE_FALSE(r2.intersects(r1));
        }

        {
            rect_f r1 {0.5f, 1.5f, 2.5f, 3.5f};
            rect_f r2 {0.5f, 0.5f, 2.5f, 1.0f};
            REQUIRE_FALSE(r1.intersects(r2));
            REQUIRE_FALSE(r2.intersects(r1));
        }

        {
            rect_f r1 {1.0f, 2.0f, 0.0f, 0.0f};
            rect_f r2 {3.0f, 4.0f, 0.0f, 0.0f};
            REQUIRE_FALSE(r1.intersects(r2));
            REQUIRE_FALSE(r2.intersects(r1));
        }

        {
            rect_f r1 {0.5f, 1.5f, 2.5f, 3.5f};
            rect_f r2 {1.0f, 2.0f, 2.0f, 3.0f};
            REQUIRE(r1.intersects(r2));
            REQUIRE(r2.intersects(r1));
        }

        {
            rect_f r1 {0.5f, 1.5f, 2.5f, 3.5f};
            rect_f r2 {2.5f, 1.5f, 3.5f, 3.5f};
            REQUIRE(r1.intersects(r2));
            REQUIRE(r2.intersects(r1));
        }

        {
            rect_f r1 {0.5f, 1.5f, 2.5f, 3.5f};
            rect_f r2 {3.0f, 4.0f, 4.5f, 5.0f};
            REQUIRE_FALSE(r1.intersects(r2));
            REQUIRE_FALSE(r2.intersects(r1));
        }

        {
            rect_f r1 {0.5f, 1.5f, 2.5f, 3.5f};
            rect_f r2 {-1.0f, 1.0f, 0.0f, 4.0f};
            REQUIRE_FALSE(r1.intersects(r2));
            REQUIRE_FALSE(r2.intersects(r1));
        }

        {
            rect_i r1 {0, 10, 20, 30};
            rect_i r2 {20, 10, 20, 30};
            REQUIRE_FALSE(r1.intersects(r2));
            REQUIRE_FALSE(r2.intersects(r1));

            REQUIRE(r1.intersects(r2, true));
            REQUIRE(r2.intersects(r1, true));
        }
    }

    SUBCASE("AsIntersectionWith")
    {
        {
            rect_f r1 {10, 20, 50, 50};
            rect_f r2 {50, 50, 30, 30};
            REQUIRE(r1.as_intersection_with(r2) == rect_f {50, 50, 10, 20});
        }
        {
            rect_f r1 {10, 20, 50, 50};
            rect_f r2 {0, 0, 30, 30};
            REQUIRE(r1.as_intersection_with(r2) == rect_f {10, 20, 20, 10});
        }
        {
            rect_f r1 {10, 10, 30, 30};
            rect_f r2 {0, 0, 60, 60};
            REQUIRE(r1.as_intersection_with(r2) == r1);
        }
    }

    SUBCASE("Center")
    {
        {
            rect_i r1 {5, 3, 10, 11};
            REQUIRE(r1.center() == (point_f {10, 8.5f}));
            REQUIRE(r1.local_center() == (point_f {5.f, 5.5f}));
        }
        {
            rect_f r1 {5, 3, 10, 11};
            REQUIRE(r1.center() == (point_f {10, 8.5f}));
            REQUIRE(r1.local_center() == (point_f {5.f, 5.5f}));
        }
    }
    SUBCASE("Structured Binding")
    {
        rect_f r1 {0.5f, 1.5f, 2.5f, 3.5f};
        auto [l, t] {r1.Position};
        auto [w, h] {r1.Size};
        REQUIRE(l == r1.left());
        REQUIRE(t == r1.top());
        REQUIRE(w == r1.width());
        REQUIRE(h == r1.height());
    }
    SUBCASE("Lerp")
    {
        rect_f r1 {5, 8, 16, 25};
        rect_f r2 {10, 16, 32, 50};
        rect_f r3 {rect_f::Lerp(r1, r2, 0.5f)};
        REQUIRE(r3.left() == 7.5f);
        REQUIRE(r3.top() == 12.f);
        REQUIRE(r3.width() == 24.f);
        REQUIRE(r3.height() == 37.5f);
    }
    SUBCASE("FromLTRB")
    {
        rect_i r1 {rect_i::FromLTRB(10, 20, 30, 40)};
        REQUIRE(r1.left() == 10);
        REQUIRE(r1.right() == 30);
        REQUIRE(r1.top() == 20);
        REQUIRE(r1.bottom() == 40);

        REQUIRE(r1.width() == 20);
        REQUIRE(r1.height() == 20);
    }
    SUBCASE("Format")
    {
        REQUIRE(std::format("{}", rect_f {10.123f, 23.434f, 20, 0.12345f}) == "(x:10.123,y:23.434,w:20,h:0.12345)");
    }
}
