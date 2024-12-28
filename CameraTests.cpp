#include "tests.hpp"

class dummy_render_target : public render_target {
public:
    dummy_render_target()
        : render_target {nullptr}
    {
    }

protected:
    auto get_size() const -> size_i override { return _size; }
    void set_size(size_i newsize) override { _size = newsize; }
    void prepare_render(bool) override { }

    size_i _size;
};

TEST_CASE("GFX.Camera.Viewport")
{
    SUBCASE("size")
    {
        dummy_render_target tex;
        gfx::camera&        camera {tex.get_camera()};
        REQUIRE(camera.transformed_viewport() == rect_f {0, 0, 0, 0});

        tex.Size = {800, 600};
        REQUIRE(camera.transformed_viewport() == rect_f {0, 0, 800, 600});
    }
    SUBCASE("position")
    {
        dummy_render_target tex;
        gfx::camera&        camera {tex.get_camera()};
        camera.ViewOffset = {200, 300};
        camera.Position   = {400, 600};
        REQUIRE(camera.transformed_viewport() == rect_f {400, 600, 0, 0});

        tex.Size = {800, 600};
        REQUIRE(camera.transformed_viewport() == rect_f {400, 600, 800, 600});
    }
    SUBCASE("zoom in")
    {
        dummy_render_target tex;
        gfx::camera&        camera {tex.get_camera()};
        camera.Position = {400, 600};
        tex.Size        = {800, 600};

        camera.Zoom = {2, 4};
        REQUIRE(camera.transformed_viewport() == rect_f {600, 825, 800 / 2.f, 600 / 4.f});
    }
    SUBCASE("zoom out")
    {
        dummy_render_target tex;
        gfx::camera&        camera {tex.get_camera()};
        camera.Position = {400, 600};
        tex.Size        = {800, 600};

        camera.Zoom = {0.5f, 0.25f};
        REQUIRE(camera.transformed_viewport() == rect_f {0, -300, 800 * 2, 600 * 4});
    }
}
