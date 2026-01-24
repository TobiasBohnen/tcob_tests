#include "tests.hpp"

struct TestValue2D {
    std::array<f32, 2> Position;
    u32                ID;

    auto get_dimensions() const -> std::array<f32, 2> { return Position; }
    auto operator==(TestValue2D const& other) const -> bool { return ID == other.ID; }
};

struct TestValue3D {
    std::array<f32, 3> Position;
    u32                ID;

    auto get_dimensions() const -> std::array<f32, 3> { return Position; }
    auto operator==(TestValue3D const& other) const -> bool { return ID == other.ID; }
};

struct TestValue4D {
    std::array<f32, 4> Position;
    u32                ID;

    auto get_dimensions() const -> std::array<f32, 4> { return Position; }
    auto operator==(TestValue4D const& other) const -> bool { return ID == other.ID; }
};

using namespace tcob;

TEST_CASE("Core.KDTree.2D")
{
    kd_tree<TestValue2D, 2>::bounds_type worldBounds = {{0.0f, 0.0f}, {100.0f, 100.0f}};
    kd_tree<TestValue2D, 2, 2, 4>        tree(worldBounds);

    SUBCASE("Insertion and Query")
    {
        tree.add({.Position = {10.0f, 10.0f}, .ID = 1});
        tree.add({.Position = {20.0f, 20.0f}, .ID = 2});
        tree.add({.Position = {80.0f, 80.0f}, .ID = 3});

        auto results = tree.query({{0.0f, 0.0f}, {30.0f, 30.0f}});
        REQUIRE(results.size() == 2);

        auto results2 = tree.query({{70.0f, 70.0f}, {100.0f, 100.0f}});
        REQUIRE(results2.size() == 1);
        REQUIRE(results2[0].ID == 3);
    }

    SUBCASE("Removal")
    {
        TestValue2D val = {.Position = {50.0f, 50.0f}, .ID = 42};
        tree.add(val);
        REQUIRE(tree.query(worldBounds).size() == 1);

        tree.remove(val);
        REQUIRE(tree.query(worldBounds).size() == 0);
    }

    SUBCASE("Replace")
    {
        TestValue2D val1 = {.Position = {10.0f, 10.0f}, .ID = 1};
        tree.add(val1);

        SUBCASE("In Place")
        {
            TestValue2D val1_updated = {.Position = {11.0f, 11.0f}, .ID = 1};
            tree.replace(val1, val1_updated);
            auto results = tree.query({{10.5f, 10.5f}, {12.0f, 12.0f}});
            REQUIRE(results.size() == 1);
            REQUIRE(results[0].Position[0] == 11.0f);
        }

        SUBCASE("Cross Boundary")
        {
            TestValue2D crossed = {.Position = {90.0f, 90.0f}, .ID = 1};
            tree.replace(val1, crossed);
            auto results = tree.query({{85.0f, 85.0f}, {95.0f, 95.0f}});
            REQUIRE(results.size() == 1);
            REQUIRE(results[0].ID == 1);
        }
    }

    SUBCASE("find_nearest")
    {
        tree.clear();
        tree.add({.Position = {10, 10}, .ID = 1});
        tree.add({.Position = {90, 90}, .ID = 2});
        tree.add({.Position = {50, 50}, .ID = 3});
        tree.add({.Position = {55, 10}, .ID = 4});

        auto nearest = tree.find_nearest({45.0f, 45.0f});
        REQUIRE(nearest);
        REQUIRE(nearest->ID == 3);

        auto boundary = tree.find_nearest({60.0f, 12.0f});
        REQUIRE(boundary);
        REQUIRE(boundary->ID == 4);
    }
}

TEST_CASE("Core.KDTree.3D")
{
    using tree_3d               = kd_tree<TestValue3D, 3, 2, 8>;
    tree_3d::bounds_type bounds = {{0, 0, 0}, {255, 255, 255}};
    tree_3d              tree(bounds);

    SUBCASE("Color Space Palette Matching")
    {
        tree.add({.Position = {255, 0, 0}, .ID = 0xFF0000}); // Red
        tree.add({.Position = {0, 255, 0}, .ID = 0x00FF00}); // Green
        tree.add({.Position = {0, 0, 255}, .ID = 0x0000FF}); // Blue

        auto nearest = tree.find_nearest({200.0f, 20.0f, 10.0f});
        REQUIRE(nearest);
        REQUIRE(nearest->ID == 0xFF0000);
    }

    SUBCASE("nearest color")
    {
        using tree_3d               = kd_tree<TestValue3D, 3, 2, 8>;
        tree_3d::bounds_type bounds = {{0, 0, 0}, {255, 255, 255}};
        tree_3d              tree(bounds);

        // 1. Create a 256-color palette
        std::vector<TestValue3D> palette;
        for (u32 i = 0; i < 256; ++i) {
            f32 r = static_cast<f32>((i * 131) % 256);
            f32 g = static_cast<f32>((i * 193) % 256);
            f32 b = static_cast<f32>((i * 241) % 256);

            TestValue3D entry = {.Position = {r, g, b}, .ID = i};
            palette.push_back(entry);
            tree.add(entry);
        }

        // 2. Define the brute-force reference function
        auto find_nearest = [&](std::array<f32, 3> const& c) -> u32 {
            u32 bestIdx {0};
            f32 minDist {std::numeric_limits<f32>::max()};
            for (auto const& p : palette) {
                f32 dr   = p.Position[0] - c[0];
                f32 dg   = p.Position[1] - c[1];
                f32 db   = p.Position[2] - c[2];
                f32 dist = (dr * dr) + (dg * dg) + (db * db);
                if (dist < minDist) {
                    minDist = dist;
                    bestIdx = p.ID;
                }
            }
            return bestIdx;
        };

        // 3. Test a variety of sample colors
        SUBCASE("Verify parity across color spectrum")
        {
            std::vector<std::array<f32, 3>> testColors = {
                {10.5f, 10.5f, 10.5f},
                {250.0f, 0.0f, 0.0f},
                {128.0f, 128.0f, 128.0f},
                {45.0f, 200.0f, 10.0f},
                {0.0f, 0.0f, 0.0f},
                {255.0f, 255.0f, 255.0f}};

            for (auto const& color : testColors) {
                REQUIRE(find_nearest(color) == tree.find_nearest(color)->ID);
            }
        }

        SUBCASE("Stress test random colors")
        {
            // Check 100 random points to ensure no edge cases in the backtracking logic
            for (int i = 0; i < 100; ++i) {
                f32                r     = static_cast<f32>(rand() % 256);
                f32                g     = static_cast<f32>(rand() % 256);
                f32                b     = static_cast<f32>(rand() % 256);
                std::array<f32, 3> color = {r, g, b};

                REQUIRE(tree.find_nearest(color)->ID == find_nearest(color));
            }
        }
    }

    SUBCASE("find_nearest")
    {
        tree.add({.Position = {127.0f, 128.0f, 128.0f}, .ID = 1}); // Side 0 (Left)
        tree.add({.Position = {140.0f, 128.0f, 128.0f}, .ID = 2}); // Side 1 (Right)
        tree.add({.Position = {250.0f, 250.0f, 250.0f}, .ID = 3}); // Side 1 (Right)

        {
            auto nearest = tree.find_nearest({128.0f, 128.0f, 128.0f});
            REQUIRE(nearest);
            REQUIRE(nearest->ID == 1);
        }
        {
            auto nearest = tree.find_nearest({140.0f, 120.0f, 128.0f});
            REQUIRE(nearest);
            REQUIRE(nearest->ID == 2);
        }
        {
            auto nearest = tree.find_nearest({200.0f, 220.0f, 328.0f});
            REQUIRE(nearest);
            REQUIRE(nearest->ID == 3);
        }
    }
}

TEST_CASE("Core.KDTree.4D")
{
    using tree_4d                    = kd_tree<TestValue4D, 4, 4, 10>;
    tree_4d::bounds_type worldBounds = {{0.0f, 0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f, 100.0f}};
    tree_4d              tree(worldBounds);

    SUBCASE("4D Hypercube Query")
    {
        tree.add({.Position = {50.0f, 50.0f, 50.0f, 10.0f}, .ID = 401});
        tree.add({.Position = {50.0f, 50.0f, 50.0f, 90.0f}, .ID = 402});

        tree_4d::bounds_type queryBounds = {{40.0f, 40.0f, 40.0f, 0.0f}, {60.0f, 60.0f, 60.0f, 20.0f}};
        auto                 results     = tree.query(queryBounds);
        REQUIRE(results.size() == 1);
        REQUIRE(results[0].ID == 401);
    }

    SUBCASE("Exhaustive Axis Cycling")
    {
        for (u32 i = 0; i < 20; ++i) {
            f32 v = static_cast<f32>(i * 4);
            tree.add({.Position = {v, v, v, v}, .ID = i});
        }

        auto midResults = tree.query({{20, 20, 20, 20}, {40, 40, 40, 40}});
        REQUIRE(midResults.size() > 0);
        for (auto const& v : midResults) {
            for (usize d = 0; d < 4; ++d) {
                REQUIRE(v.Position[d] >= 20.0f);
                REQUIRE(v.Position[d] <= 40.0f);
            }
        }
    }

    SUBCASE("find_nearest")
    {
        tree.clear();
        tree.add({.Position = {0, 0, 0, 0}, .ID = 100});
        tree.add({.Position = {0, 0, 0, 50}, .ID = 200});
        tree.add({.Position = {5, 5, 5, 5}, .ID = 300});

        auto n1 = tree.find_nearest({2.0f, 2.0f, 2.0f, 2.0f});
        REQUIRE(n1);
        REQUIRE(n1->ID == 100);

        auto n2 = tree.find_nearest({0.0f, 0.0f, 0.0f, 48.0f});
        REQUIRE(n2);
        REQUIRE(n2->ID == 200);
    }
}
