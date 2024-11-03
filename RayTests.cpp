#include "tests.hpp"

TEST_CASE("GFX.Ray.Rect")
{
    {
        rect_f rect {10, 0, 20, 20};
        ray    ray {{{5, 5}, degree_f {90}}};
        REQUIRE(ray.intersect_rect(rect) == std::vector<ray::result> {{{30, 5}, 25}, {{10, 5}, 5}});
    }
    {
        rect_f rect {0, 0, 50, 50};
        ray    ray {{{60, 30}, degree_f {270}}};
        REQUIRE(ray.intersect_rect(rect) == std::vector<ray::result> {{{50, 30}, 10}, {{0, 30}, 60}});
    }
}

TEST_CASE("GFX.Ray.Circle")
{
    {
        ray ray {{{0, 10}, degree_f {90}}};
        REQUIRE(ray.intersect_circle({10, 10}, 4) == std::vector<ray::result> {{{6, 10}, 6}, {{14, 10}, 14}});
    }
}

TEST_CASE("GFX.Ray.Func")
{
    {
        easing::linear<point_f> func;
        func.StartValue = {2, 2};
        func.EndValue   = {12, 12};
        ray ray {{{1, 5}, degree_f {90}}};
        REQUIRE(ray.intersect_function(func, 0.01) == std::vector<ray::result> {{{5, 5}, 4}});
    }
}
