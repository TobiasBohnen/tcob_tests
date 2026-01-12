#include "tests.hpp"

TEST_CASE("Core.Random.MinMax")
{
    SUBCASE("default")
    {
        i32 const it = 5'000;

        rng rnd;
        SUBCASE("i32")
        {
            {
                i32 min = 8, max = 15;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i32 min = -10, max = 15;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i32 min = -5, max = -4;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i32 min = std::numeric_limits<i32>::min();
                i32 max = std::numeric_limits<i32>::max();
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i32 min = std::numeric_limits<i32>::min();
                i32 max = std::numeric_limits<i32>::max() - 1;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i32 min = 1'000'000'000;
                i32 max = 2'000'000'000;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i32 min = -2'000'000'000;
                i32 max = -1'000'000'000;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i32 min = -2'000'000'000;
                i32 max = 2'000'000'000;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i32 min = 42, max = 42;
                for (i32 i = 0; i < 100; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x == 42);
                }
            }
        }

        SUBCASE("u32")
        {
            {
                u32 min = 8, max = 15;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                u32 min = std::numeric_limits<u32>::min();
                u32 max = std::numeric_limits<u32>::max();
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                u32 min = 0;
                u32 max = std::numeric_limits<u32>::max() - 1;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                u32 min = 1'000'000'000;
                u32 max = 4'000'000'000;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
        }

        SUBCASE("i64")
        {
            {
                i64 min = 8, max = 15;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i64 min = std::numeric_limits<i64>::min();
                i64 max = std::numeric_limits<i64>::max();
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i64 min = -5'000'000'000'000'000'000LL;
                i64 max = 5'000'000'000'000'000'000LL;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
        }

        SUBCASE("i8")
        {
            {
                i8 min = -10, max = 10;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i8 min = std::numeric_limits<i8>::min();
                i8 max = std::numeric_limits<i8>::max();
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
        }

        SUBCASE("f32")
        {
            {
                f32 min = 8.f, max = 15.f;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                f32 min = -10.f, max = 15.f;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                f32 min = -5.f, max = -4.f;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                f32 min = -1e30f, max = 1e30f;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                f32 min = 0.0f, max = 0.001f;
                for (i32 i = 0; i < it; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                f32 min = 3.14f, max = 3.14f;
                for (i32 i = 0; i < 100; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x == min);
                }
            }
        }

        SUBCASE("f64")
        {
            {
                f64 min = 8.0, max = 15.0;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                f64 min = -1e100, max = 1e100;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                f64 min = 1e-10, max = 1e-9;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
        }
    }
    SUBCASE("split_mix_32")
    {
        random::prng_split_mix_32 rnd;
        SUBCASE("i32")
        {
            {
                i32 min = 8, max = 15;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i32 min = -10, max = 15;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                i32 min = -5, max = -4;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
        }
        SUBCASE("f32")
        {
            {
                f32 min = 8.f, max = 15.f;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                f32 min = -10.f, max = 15.f;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
            {
                f32 min = -5.f, max = -4.f;
                for (i32 i = 0; i < 5000; i++) {
                    auto x = rnd(min, max);
                    REQUIRE(x >= min);
                    REQUIRE(x <= max);
                }
            }
        }
    }
}

TEST_CASE("Core.Random.CopyState")
{
    rng r0 {12345};
    for (i32 i = 0; i < 1000; i++) {
        r0(0, 1);
    }

    rng r1 {r0.state()};
    i32 min = 8, max = 15;
    for (i32 i = 0; i < 1000; i++) {
        REQUIRE(r0(min, max) == r1(min, max));
    }
}

TEST_CASE("Core.Random.Copy")
{
    {
        rng r0 {12345};
        for (i32 i = 0; i < 1000; i++) {
            r0(0, 1);
        }

        rng r1 {r0};
        i32 min = 8, max = 15;
        for (i32 i = 0; i < 1000; i++) {
            REQUIRE(r0(min, max) == r1(min, max));
        }
    }
    {
        random::dice<6> r0 {12345};
        for (i32 i = 0; i < 1000; i++) {
            r0.roll();
        }
        random::dice<6> r1 {r0};
        for (i32 i = 0; i < 1000; i++) {
            REQUIRE(r0.roll() == r1.roll());
        }
        random::dice<6> r2 {};
        r2 = r1;
        for (i32 i = 0; i < 1000; i++) {
            i32 roll = r1.roll();
            REQUIRE(r0.roll() == roll);
            REQUIRE(r2.roll() == roll);
        }
    }
}

TEST_CASE("Core.Random.Seq")
{
    {
        rng              r0 {50413220};
        std::vector<i32> v {};
        v.reserve(10);
        for (i32 i = 0; i < 10; i++) {
            v.push_back(r0(0, 10));
        }

        REQUIRE(v == std::vector<i32> {4, 6, 7, 9, 4, 9, 10, 0, 4, 5});
    }
    {
        rng              r0 {54321};
        std::vector<i32> v {};
        v.reserve(10);
        for (i32 i = 0; i < 10; i++) {
            v.push_back(r0(0, 100));
        }

        REQUIRE(v == std::vector<i32> {70, 82, 59, 22, 51, 58, 49, 47, 59, 31});
    }
}

TEST_CASE("Core.Random.Shuffle")
{
    {
        std::vector<i32>     v {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        random::shuffle<i32> r0 {54321};
        r0(v);
        REQUIRE(v == std::vector<i32> {8, 5, 2, 4, 0, 3, 7, 6, 1, 9});
    }
    {
        std::array<i32, 10>  v {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        random::shuffle<i32> r0 {54321};
        r0(v);
        REQUIRE(v == std::array<i32, 10> {8, 5, 2, 4, 0, 3, 7, 6, 1, 9});
    }
}
