#include "tests.hpp"

TEST_CASE("GFX.Ray.Rect")
{
    {
        rect_f rect {10, 0, 20, 20};
        ray    ray {{5, 5}, degree_f {90}};
        REQUIRE(ray.intersect_rect(rect) == std::vector<point_f> {{30, 5}, {10, 5}});
    }
    {
        rect_f rect {0, 0, 50, 50};
        ray    ray {{60, 30}, degree_f {270}};
        REQUIRE(ray.intersect_rect(rect) == std::vector<point_f> {{50, 30}, {0, 30}});
    }
}
