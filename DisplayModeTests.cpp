
#include "tcob/app/Platform.hpp"
#include "tcob/core/Size.hpp"
#include "tests.hpp"

TEST_CASE("Core.POD.DisplayMode")
{
    size_i const size {1920, 1080};
    f32 const    refresh {60};
    f32 const    pixelDens {1.0f};

    size_i const smallerWidth {1800, 1080};
    size_i const smallerHeight {1920, 500};
    f32 const    smallerRefresh {50};
    f32 const    smallerPixelDens {0.5f};

    REQUIRE(display_mode {size, pixelDens, refresh} > display_mode {size, pixelDens, smallerRefresh});
    REQUIRE(display_mode {size, pixelDens, refresh} > display_mode {smallerWidth, pixelDens, refresh});
    REQUIRE(display_mode {size, pixelDens, refresh} > display_mode {smallerHeight, pixelDens, refresh});
    REQUIRE(display_mode {size, pixelDens, refresh} < display_mode {size, smallerPixelDens, refresh});
}