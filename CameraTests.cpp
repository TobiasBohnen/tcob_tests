#include "tests.hpp"

TEST_CASE("GFX.Camera.Frustum")
{
    SUBCASE("size")
    {
        gfx::camera camera;
        REQUIRE(camera.get_transformed_viewport() == rect_f {0, 0, 0, 0});

        camera.Size = {800, 600};
        REQUIRE(camera.get_transformed_viewport() == rect_f {0, 0, 800, 600});
    }
    SUBCASE("position")
    {
        gfx::camera camera;
        camera.Offset   = {200, 300};
        camera.Position = {400, 600};
        REQUIRE(camera.get_transformed_viewport() == rect_f {400, 600, 0, 0});

        camera.Size = {800, 600};
        REQUIRE(camera.get_transformed_viewport() == rect_f {400, 600, 800, 600});
    }
    SUBCASE("zoom in")
    {
        gfx::camera camera;
        camera.Position = {400, 600};
        camera.Size     = {800, 600};

        camera.Zoom = {2, 4};
        REQUIRE(camera.get_transformed_viewport() == rect_f {600, 825, 800 / 2.f, 600 / 4.f});
    }
    SUBCASE("zoom out")
    {
        gfx::camera camera;
        camera.Position = {400, 600};
        camera.Size     = {800, 600};

        camera.Zoom = {0.5f, 0.25f};
        REQUIRE(camera.get_transformed_viewport() == rect_f {0, -300, 800 * 2, 600 * 4});
    }
}
