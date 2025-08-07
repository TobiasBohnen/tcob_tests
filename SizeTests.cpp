#include "tests.hpp"

TEST_CASE("Core.POD.Size")
{
    static_assert(std::is_copy_constructible_v<size_f>);
    static_assert(std::is_copy_assignable_v<size_f>);
    static_assert(std::is_move_constructible_v<size_f>);
    static_assert(std::is_move_assignable_v<size_f>);
    static_assert(POD<size_f>);

    SUBCASE("AspectRatio")
    {
        REQUIRE(size_i {1920, 1080}.aspect_ratio() == Approx(16.0f / 9.0f));
        REQUIRE(size_i {1280, 800}.aspect_ratio() == Approx(16.0f / 10.0f));
        REQUIRE(size_i {1024, 768}.aspect_ratio() == Approx(4.0f / 3.0f));
        REQUIRE(size_i {2560, 1080}.aspect_ratio() == Approx(2560.0f / 1080.0f));
        REQUIRE(size_i {800, 600}.aspect_ratio() == Approx(4.0f / 3.0f));

        REQUIRE(size_i {0, 0}.aspect_ratio() == 0);
        REQUIRE(size_i {0, 600}.aspect_ratio() == 0);
        REQUIRE(size_i {600, 0}.aspect_ratio() == 0);
    }

    SUBCASE("IntegerRatio")
    {
        REQUIRE(size_i {1920, 1080}.integer_ratio() == size_i {16, 9});
        REQUIRE(size_i {1280, 800}.integer_ratio() == size_i {8, 5});
        REQUIRE(size_i {1024, 768}.integer_ratio() == size_i {4, 3});
        REQUIRE(size_i {2560, 1080}.integer_ratio() == size_i {64, 27});
        REQUIRE(size_i {3440, 1440}.integer_ratio() == size_i {43, 18});

        REQUIRE(size_i {5120, 1440}.integer_ratio() == size_i {32, 9});
        REQUIRE(size_i {1280, 1024}.integer_ratio() == size_i {5, 4});
        REQUIRE(size_i {1440, 960}.integer_ratio() == size_i {3, 2});

        REQUIRE(size_i {0, 0}.integer_ratio() == size_i::Zero);
        REQUIRE(size_i {100, 0}.integer_ratio() == size_i::Zero);
    }
}
