#include "tests.hpp"

TEST_CASE("Core.Property.Copying")
{
    prop<i32> test {10};
    REQUIRE(test == 10);
    auto test2 {*test};
    REQUIRE(test2 == 10);
    test2 = 20;
    REQUIRE(test2 == 20);
    REQUIRE(test == 10);
}

TEST_CASE("Core.Property.Event")
{
    bool      changed {false};
    prop<i32> vt {0};
    vt.Changed.connect([&](i32) { changed = true; });
    REQUIRE_FALSE(vt == 100);
    vt = {100};
    REQUIRE(vt == 100);
    REQUIRE(changed == true);
}

TEST_CASE("Core.Property.Validate")
{
    i32 const     expected = 32;
    prop_val<i32> prop {{[&](i32 const&) { return expected; }}};
    prop.Changed.connect([&](i32 const& val) { REQUIRE(val == expected); });
    prop = 300;
    REQUIRE(expected == 32);
    REQUIRE(prop == expected);
}

TEST_CASE("Core.Property.Function")
{
    struct foo {
        i32 result {12};
    };

    foo          f;
    prop_fn<i32> prop {{&f,
                        [](void* f) -> i32 { return static_cast<foo*>(f)->result; },
                        [](void* f, i32 const& val) -> void { static_cast<foo*>(f)->result = val; }}};
    REQUIRE(f.result == 12);
    REQUIRE(prop == 12);

    prop = 300;
    REQUIRE(f.result == 300);
    REQUIRE(prop == f.result);
}

TEST_CASE("Core.Property.Field")
{
    {
        prop<i32> prop {0};
        prop = 234;
        REQUIRE(prop == 234);
        prop = 500;
        REQUIRE(prop == 500);
    }
    {
        prop<f32> prop {0.f};
        prop = 23.4f;
        REQUIRE(prop == 23.4f);
        prop = 50.0f;
        REQUIRE(prop == 50.0f);
    }
    {
        prop<u8> prop {u8 {9}};
        REQUIRE(prop == 9);
        prop = 12;
        REQUIRE(prop == 12);
    }
    {
        prop<bool> prop {false};
        REQUIRE(prop == false);
        prop = true;
        REQUIRE(prop == true);
    }
}

TEST_CASE("Core.Property.Compare")
{
    {
        prop<f32> prop {23.4f};
        REQUIRE(prop == 23.4f);
        REQUIRE(prop != 150.0f);
    }
    {
        prop<i32> prop {234};
        REQUIRE(prop > 1);
        REQUIRE(prop < 600);
    }
    {
        prop<f32> prop {23.4f};
        REQUIRE(prop > 1.0f);
        REQUIRE(prop < 150.0f);
    }
    {
        prop<f32> prop0 {23.4f};
        prop<f32> prop1 {23.4f};
        REQUIRE(prop0 == prop1);
    }
    {
        prop<f32> prop0 {23.4f};
        prop<f32> prop1 {12.3f};
        REQUIRE(prop0 != prop1);
    }
    {
        prop<f32> prop0 {23.4f};
        prop<f32> prop1 {12.3f};
        REQUIRE(prop0 > prop1);
    }
    {
        prop<f32> prop0 {12.3f};
        prop<f32> prop1 {23.4f};
        REQUIRE(prop0 < prop1);
    }
    {
        prop<degree_f> prop0 {degree_f {12.3f}};
        REQUIRE(prop0 > degree_f {2.3f});
    }
    {
        prop<degree_f> prop1 {degree_f {23.4f}};
        prop<degree_f> prop0 {degree_f {12.3f}};
        REQUIRE(prop0 < prop1);
    }
}

TEST_CASE("Core.Property.Arithmetic")
{
    SUBCASE("+")
    {
        {
            prop<i32> prop {0};
            prop = 234;
            prop += 100;
            REQUIRE(prop == 334);
        }
        {
            prop<point_i> prop {point_i::Zero};
            prop = {123, 456};
            prop += {100, 200};
            REQUIRE(prop == point_i {223, 656});
        }
        {
            prop<point_i> prop {point_i::Zero};
            prop = {123, 456};
            prop += point_i {100, 200};
            REQUIRE(prop == point_i {223, 656});
            prop += point_i {100, 200};
            REQUIRE(prop == point_i {323, 856});
        }
        {
            prop<i32> prop {0};
            prop = 0;
            prop = prop + 100;
            REQUIRE(prop == 100);
            prop = 100 + prop;
            REQUIRE(prop == 200);
        }
    }
    SUBCASE("-")
    {
        {
            prop<i32> prop {0};
            prop = 234;
            prop -= 100;
            REQUIRE(prop == 134);
        }
        {
            prop<point_i> prop {point_i::Zero};
            prop = {123, 456};
            prop -= {100, 200};
            REQUIRE(prop == point_i {23, 256});
        }
        {
            prop<i32> prop {0};
            prop = 200;
            prop = prop - 100;
            REQUIRE(prop == 100);
            prop = 100 - prop;
            REQUIRE(prop == 0);
        }
    }
    SUBCASE("*")
    {
        {
            prop<i32> prop {0};
            prop = 10;
            prop *= 100;
            REQUIRE(prop == 1000);
        }
        {
            prop<point_i> prop {point_i::Zero};
            static_assert(std::is_same_v<decltype(prop.operator->()), point_i const*>);
            prop = {123, 456};
            prop *= {100, 200};
            REQUIRE(prop == point_i {12300, 91200});
        }
        {
            prop<i32> prop {0};
            prop = 10;
            prop = prop * 100;
            REQUIRE(prop == 1000);
            prop = 100 * prop;
            REQUIRE(prop == 100000);
        }
    }
    SUBCASE("/")
    {
        {
            prop<i32> prop {0};
            prop = 200;
            prop /= 100;
            REQUIRE(prop == 2);
        }
        {
            prop<point_i> prop {point_i::Zero};
            prop = {500, 600};
            prop /= {100, 200};
            REQUIRE(prop == point_i {5, 3});
        }
        {
            prop<i32> prop {0};
            prop = 234;
            prop = prop / 100;
            REQUIRE(prop == 2);
            prop = 100 / prop;
            REQUIRE(prop == 50);
        }
    }
}

TEST_CASE("Core.Property.Subscript")
{
    {
        prop<std::vector<i32>> prop {std::vector<i32> {}};
        prop.mutate([](auto& vec) {
            vec.resize(100);
            vec[0]  = 100;
            vec[10] = 25;
        });

        REQUIRE(prop[0] == 100);
        REQUIRE(prop[10] == 25);
    }
    {
        prop<grid<i32>> prop {grid<i32> {}};
        prop.mutate([](auto& vec) {
            vec.resize({10, 20});
            vec[0, 10] = 100;
            vec[10, 2] = 25;
        });

        REQUIRE(prop[0, 10] == 100);
        REQUIRE(prop[10, 2] == 25);
    }
    {
        prop<std::vector<i32>> prop {{1, 2, 3, 5}};
        REQUIRE(prop[0] == 1);
        REQUIRE(prop[3] == 5);
    }
}

TEST_CASE("Core.Property.Bind")
{
    prop<i32> a {10};
    prop<i32> b {20};
    prop<i32> c {30};

    a.bind(b, c);

    SUBCASE("initial values are independent until set")
    {
        REQUIRE(static_cast<i32>(a) == 10);
        REQUIRE(static_cast<i32>(b) == 20);
        REQUIRE(static_cast<i32>(c) == 30);
    }

    SUBCASE("assigning A updates B + C")
    {
        a = 42;
        REQUIRE(static_cast<i32>(a) == 42);
        REQUIRE(static_cast<i32>(b) == 42);
        REQUIRE(static_cast<i32>(c) == 42);
    }

    SUBCASE("assigning B updates A + C")
    {
        b = 99;
        REQUIRE(static_cast<i32>(a) == 99);
        REQUIRE(static_cast<i32>(b) == 99);
        REQUIRE(static_cast<i32>(c) == 99);
    }

    SUBCASE("assigning C updates A + B")
    {
        c = 103;
        REQUIRE(static_cast<i32>(a) == 103);
        REQUIRE(static_cast<i32>(b) == 103);
        REQUIRE(static_cast<i32>(c) == 103);
    }

    SUBCASE("Changed call count")
    {
        i32 a_changed_count {0};
        i32 b_changed_count {0};
        i32 c_changed_count {0};

        a.Changed.connect([&](i32) { ++a_changed_count; });
        b.Changed.connect([&](i32) { ++b_changed_count; });
        c.Changed.connect([&](i32) { ++c_changed_count; });

        a = 123;
        REQUIRE(a_changed_count == 1);
        REQUIRE(b_changed_count == 1);
        REQUIRE(c_changed_count == 1);

        REQUIRE(static_cast<i32>(a) == 123);
        REQUIRE(static_cast<i32>(b) == 123);
        REQUIRE(static_cast<i32>(c) == 123);

        a = 123;
        b = 123;
        c = 123;

        REQUIRE(a_changed_count == 1);
        REQUIRE(b_changed_count == 1);
        REQUIRE(c_changed_count == 1);
    }
}

template <typename T>
class queue_source final {
public:
    using type              = std::remove_const_t<T>;
    using return_type       = type;
    using const_return_type = type;

    queue_source() = default;

    auto get() const -> const_return_type
    {
        auto front {_values.front()};
        _values.pop();
        return front;
    }

    auto set(type const& value, bool) -> bool
    {
        _values.push(value);
        return true;
    }

private:
    mutable std::queue<type> _values {};
};

TEST_CASE("Core.Property.CustomSource")
{
    tcob::detail::prop_base<i32, queue_source<i32>> prop {};
    prop = 100;
    prop = 200;
    prop = 400;

    REQUIRE(prop == 100);
    REQUIRE(prop == 200);
    REQUIRE(prop == 400);
}
