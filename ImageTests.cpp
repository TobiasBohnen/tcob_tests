#include "tests.hpp"

TEST_CASE("GFX.Image.FlipHorizontally")
{
    SUBCASE("4x4 x 4")
    {
        u32 const                                           channels {4};
        constexpr size_i                                    size {4, 4};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};
        std::vector<u8> target {
            12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3,
            28, 29, 30, 31, 24, 25, 26, 27, 20, 21, 22, 23, 16, 17, 18, 19,
            44, 45, 46, 47, 40, 41, 42, 43, 36, 37, 38, 39, 32, 33, 34, 35,
            60, 61, 62, 63, 56, 57, 58, 59, 52, 53, 54, 55, 48, 49, 50, 51};

        auto image {image::Create(size, image::format::RGBA, source)};
        image.flip_horizontally();
        std::vector<u8> data1 {image.data().begin(), image.data().end()};
        REQUIRE(data1 == target);
    }
    SUBCASE("4x4 x 3")
    {
        u32 const                                           channels {3};
        constexpr size_i                                    size {4, 4};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
            12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
            24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
            36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47};
        std::vector<u8> target {
            9, 10, 11, 6, 7, 8, 3, 4, 5, 0, 1, 2,
            21, 22, 23, 18, 19, 20, 15, 16, 17, 12, 13, 14,
            33, 34, 35, 30, 31, 32, 27, 28, 29, 24, 25, 26,
            45, 46, 47, 42, 43, 44, 39, 40, 41, 36, 37, 38};

        auto image {image::Create(size, image::format::RGB, source)};
        image.flip_horizontally();
        std::vector<u8> data1 {image.data().begin(), image.data().end()};
        REQUIRE(data1 == target);
    }
    SUBCASE("3x3 x 4")
    {
        u32 const                                           channels {4};
        constexpr size_i                                    size {3, 3};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
            12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
            24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};
        std::vector<u8> target {
            8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3,
            20, 21, 22, 23, 16, 17, 18, 19, 12, 13, 14, 15,
            32, 33, 34, 35, 28, 29, 30, 31, 24, 25, 26, 27};

        auto image {image::Create(size, image::format::RGBA, source)};
        image.flip_horizontally();
        std::vector<u8> data1 {image.data().begin(), image.data().end()};
        REQUIRE(data1 == target);
    }
    SUBCASE("3x3 x 3")
    {
        u32 const                                           channels {3};
        constexpr size_i                                    size {3, 3};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8,
            9, 10, 11, 12, 13, 14, 15, 16, 17,
            18, 19, 20, 21, 22, 23, 24, 25, 26};
        std::vector<u8> target {
            6, 7, 8, 3, 4, 5, 0, 1, 2,
            15, 16, 17, 12, 13, 14, 9, 10, 11,
            24, 25, 26, 21, 22, 23, 18, 19, 20};

        auto image {image::Create(size, image::format::RGB, source)};
        image.flip_horizontally();
        std::vector<u8> data1 {image.data().begin(), image.data().end()};
        REQUIRE(data1 == target);
    }
}

TEST_CASE("GFX.Image.FlipVertically")
{
    SUBCASE("5x5 x 4")
    {
        u32 const                                           channels {4};
        constexpr size_i                                    size {5, 5};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
            40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
            60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
            80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99};
        std::vector<u8> target {
            80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
            60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
            40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

        auto image {image::Create(size, image::format::RGBA, source)};
        image.flip_vertically();
        std::vector<u8> data1 {image.data().begin(), image.data().end()};
        REQUIRE(data1 == target);
    }
    SUBCASE("4x4 x 4")
    {
        u32 const                                           channels {4};
        constexpr size_i                                    size {4, 4};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};
        std::vector<u8> target {
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

        auto image {image::Create(size, image::format::RGBA, source)};
        image.flip_vertically();
        std::vector<u8> data1 {image.data().begin(), image.data().end()};
        REQUIRE(data1 == target);
    }
    SUBCASE("3x3 x 4")
    {
        u32 const                                           channels {4};
        constexpr size_i                                    size {3, 3};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
            12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
            24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};
        std::vector<u8> target {
            24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
            12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

        auto image {image::Create(size, image::format::RGBA, source)};
        image.flip_vertically();
        std::vector<u8> data1 {image.data().begin(), image.data().end()};
        REQUIRE(data1 == target);
    }
    SUBCASE("3x3 x 3")
    {
        u32 const                                           channels {3};
        constexpr size_i                                    size {3, 3};
        std::array<u8, size.Width * size.Height * channels> source {
            0, 1, 2, 3, 4, 5, 6, 7, 8,
            9, 10, 11, 12, 13, 14, 15, 16, 17,
            18, 19, 20, 21, 22, 23, 24, 25, 26};
        std::vector<u8> target {
            18, 19, 20, 21, 22, 23, 24, 25, 26,
            9, 10, 11, 12, 13, 14, 15, 16, 17,
            0, 1, 2, 3, 4, 5, 6, 7, 8};

        auto image {image::Create(size, image::format::RGB, source)};
        image.flip_vertically();
        std::vector<u8> data1 {image.data().begin(), image.data().end()};
        REQUIRE(data1 == target);
    }
}

TEST_CASE("GFX.Image.GetSetPixel")
{
    SUBCASE("4x4 x 4")
    {
        u32 const                                           channels {4};
        constexpr size_i                                    size {4, 4};
        std::array<u8, size.Width * size.Height * channels> source {};

        auto image {image::Create(size, image::format::RGBA, source)};

        REQUIRE(image.get_pixel({0, 0}) == color {0, 0, 0, 0});
        image.set_pixel({0, 0}, colors::GoldenRod);
        REQUIRE(image.get_pixel({0, 0}) == colors::GoldenRod);

        REQUIRE(image.get_pixel({3, 3}) == color {0, 0, 0, 0});
        image.set_pixel({3, 3}, colors::SandyBrown);
        REQUIRE(image.get_pixel({3, 3}) == colors::SandyBrown);

        REQUIRE(image.get_pixel({2, 2}) == color {0, 0, 0, 0});
        image.set_pixel({2, 2}, colors::RebeccaPurple);
        REQUIRE(image.get_pixel({2, 2}) == colors::RebeccaPurple);
    }
    SUBCASE("4x4 x 3")
    {
        u32 const                                           channels {3};
        constexpr size_i                                    size {4, 4};
        std::array<u8, size.Width * size.Height * channels> source {};

        auto image {image::Create(size, image::format::RGB, source)};

        REQUIRE(image.get_pixel({0, 0}) == color {0, 0, 0, 255});
        image.set_pixel({0, 0}, colors::GoldenRod);
        REQUIRE(image.get_pixel({0, 0}) == colors::GoldenRod);

        REQUIRE(image.get_pixel({3, 3}) == color {0, 0, 0, 255});
        image.set_pixel({3, 3}, colors::SandyBrown);
        REQUIRE(image.get_pixel({3, 3}) == colors::SandyBrown);

        REQUIRE(image.get_pixel({2, 2}) == color {0, 0, 0, 255});
        image.set_pixel({2, 2}, colors::RebeccaPurple);
        REQUIRE(image.get_pixel({2, 2}) == colors::RebeccaPurple);
    }
}

TEST_CASE("GFX.Image.CountColors")
{
    SUBCASE("RGB")
    {
        image img {image::CreateEmpty({10, 10}, image::format::RGB)};
        img.set_pixel({4, 4}, colors::White);
        REQUIRE(img.count_colors() == 2);
        img.set_pixel({6, 6}, colors::Red);
        REQUIRE(img.count_colors() == 3);
        img.set_pixel({7, 7}, colors::Blue);
        REQUIRE(img.count_colors() == 4);
    }
    SUBCASE("RGBA")
    {
        image img {image::CreateEmpty({10, 10}, image::format::RGBA)};
        img.set_pixel({4, 4}, colors::White);
        REQUIRE(img.count_colors() == 2);
        img.set_pixel({6, 6}, colors::Red);
        REQUIRE(img.count_colors() == 3);
        img.set_pixel({7, 7}, colors::Blue);
        REQUIRE(img.count_colors() == 4);
    }
}

TEST_CASE("GFX.Image.DataRect")
{
    u32 const                                           channels {4};
    constexpr size_i                                    size {5, 5};
    std::array<u8, size.Width * size.Height * channels> source {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
        60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
        80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99};
    auto image {image::Create(size, image::format::RGBA, source)};

    std::vector<u8> target0 {
        48, 49, 50, 51, 52, 53, 54, 55,
        68, 69, 70, 71, 72, 73, 74, 75};
    std::vector<u8> data0 {image.data({2, 2, 2, 2})};
    REQUIRE(data0 == target0);

    std::vector<u8> target1 {
        0, 1, 2, 3};
    std::vector<u8> data1 {image.data({0, 0, 1, 1})};
    REQUIRE(data1 == target1);

    std::vector<u8> target2 {
        96, 97, 98, 99};
    std::vector<u8> data2 {image.data({4, 4, 1, 1})};
    REQUIRE(data2 == target2);

    std::vector<u8> target3 {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    std::vector<u8> data3 {image.data({0, 0, 5, 1})};
    REQUIRE(data3 == target3);

    std::vector<u8> target4 {
        0, 1, 2, 3,
        20, 21, 22, 23,
        40, 41, 42, 43,
        60, 61, 62, 63,
        80, 81, 82, 83};
    std::vector<u8> data4 {image.data({0, 0, 1, 5})};
    REQUIRE(data4 == target4);

    std::vector<u8> target5(source.begin(), source.end());
    std::vector<u8> data5 {image.data({0, 0, 5, 5})};
    REQUIRE(data5 == target5);

    std::vector<u8> target6 {
        40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59};
    std::vector<u8> data6 {image.data({0, 2, 5, 1})};
    REQUIRE(data6 == target6);
}

TEST_CASE("GFX.Image.RemoveAlpha")
{

    u32 const                                           channels {4};
    constexpr size_i                                    size {4, 4};
    std::array<u8, size.Width * size.Height * channels> source {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};
    std::vector<u8> target {
        0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14,
        16, 17, 18, 20, 21, 22, 24, 25, 26, 28, 29, 30,
        32, 33, 34, 36, 37, 38, 40, 41, 42, 44, 45, 46,
        48, 49, 50, 52, 53, 54, 56, 57, 58, 60, 61, 62};

    auto            image {image::Create(size, image::format::RGBA, source)};
    auto            newImage {remove_alpha {}(image)};
    std::vector<u8> data1 {newImage.data().begin(), newImage.data().end()};
    REQUIRE(data1 == target);
}

auto create_solid_color_image(i32 width, i32 height, color c) -> image
{
    auto img = image::CreateEmpty(size_i {width, height}, image::format::RGB);
    img.fill(rect_i {{0, 0}, {width, height}}, c);
    return img;
}

auto create_gradient_image(i32 width, i32 height) -> image
{
    auto img = image::CreateEmpty(size_i {width, height}, image::format::RGB);
    for (i32 y {0}; y < height; y++) {
        for (i32 x {0}; x < width; x++) {
            u8 const value {static_cast<u8>((x * 255) / width)};
            img.set_pixel(point_i {x, y}, color {value, value, value});
        }
    }
    return img;
}
TEST_CASE("GFX.Image.OctTreeQuant")
{
    SUBCASE("single color")
    {
        octree_quantizer quantizer {256};
        auto             img = quantizer(create_solid_color_image(10, 10, color {255, 0, 0}));

        REQUIRE(img.count_colors() == 1);
    }

    SUBCASE("two colors")
    {
        octree_quantizer quantizer {256};
        auto             img = image::CreateEmpty(size_i {10, 10}, image::format::RGB);

        for (i32 y {0}; y < 10; y++) {
            for (i32 x {0}; x < 10; x++) {
                img.set_pixel(point_i {x, y}, (x < 5) ? color {255, 0, 0} : color {0, 0, 255});
            }
        }

        auto newImg = quantizer(img);
        REQUIRE(newImg.count_colors() == 2);
    }

    SUBCASE("16 color limit")
    {
        octree_quantizer quantizer {16};
        auto             img = create_gradient_image(256, 10);

        auto newImg = quantizer(img);

        REQUIRE(newImg.count_colors() <= 16);
    }

    SUBCASE("8 color limit")
    {
        octree_quantizer quantizer {8};
        auto             img = create_gradient_image(256, 10);

        auto newImg = quantizer(img);

        REQUIRE(newImg.count_colors() <= 8);
    }

    SUBCASE("256 color limit")
    {
        octree_quantizer quantizer {256};
        auto             img = create_gradient_image(256, 10);

        auto newImg = quantizer(img);

        REQUIRE(newImg.count_colors() <= 256);
    }

    SUBCASE("colors preserved")
    {
        octree_quantizer quantizer {256};
        auto             img = image::CreateEmpty(size_i {4, 2}, image::format::RGB);

        std::array<color, 8> p {{color {255, 0, 0},
                                 color {0, 255, 0},
                                 color {0, 0, 255},
                                 color {255, 255, 0},
                                 color {255, 0, 255},
                                 color {0, 255, 255},
                                 color {255, 255, 255},
                                 color {0, 0, 0}}};

        img.set_pixel(point_i {0, 0}, p[0]);
        img.set_pixel(point_i {1, 0}, p[1]);
        img.set_pixel(point_i {2, 0}, p[2]);
        img.set_pixel(point_i {3, 0}, p[3]);
        img.set_pixel(point_i {0, 1}, p[4]);
        img.set_pixel(point_i {1, 1}, p[5]);
        img.set_pixel(point_i {2, 1}, p[6]);
        img.set_pixel(point_i {3, 1}, p[7]);

        auto newImg = quantizer(img);

        REQUIRE(newImg.count_colors() == 8);
        auto const& info {newImg.info()};

        std::unordered_set<color> colors;
        auto const [width, height] {info.Size};
        for (i32 y {0}; y < height; ++y) {
            for (i32 x {0}; x < width; ++x) {
                colors.insert(newImg.get_pixel({x, y}));
            }
        }

        REQUIRE(colors.size() == 8);
        for (auto c : p) {
            REQUIRE(colors.contains(c));
        }
    }

    SUBCASE("single pixel image")
    {
        octree_quantizer quantizer {256};
        auto             img = image::CreateEmpty(size_i {1, 1}, image::format::RGB);
        img.set_pixel(point_i {0, 0}, color {0, 0, 0});

        auto newImg = quantizer(img);

        REQUIRE(newImg.count_colors() == 1);
    }

    SUBCASE("100x100 image")
    {
        octree_quantizer quantizer {256};
        auto             img = create_gradient_image(100, 100);

        auto newImg = quantizer(img);

        REQUIRE(newImg.count_colors() <= 256);
    }
}
