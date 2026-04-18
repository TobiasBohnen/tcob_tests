#include "tests.hpp"

TEST_CASE("Core.UserObject.Construction")
{
    SUBCASE("default state")
    {
        user_object obj;
        REQUIRE(obj.has_value() == false);
        REQUIRE(obj.get<i32>() == nullptr);
        REQUIRE(obj.type() == typeid(void));
    }

    SUBCASE("value construction (i32)")
    {
        user_object obj {42};
        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == typeid(i32));
        auto* v {obj.get<i32>()};
        REQUIRE(v != nullptr);
        REQUIRE(*v == 42);
    }

    SUBCASE("value construction (string)")
    {
        user_object obj {string("hello")};
        REQUIRE(obj.type() == typeid(string));
        auto* v {obj.get<string>()};
        REQUIRE(v != nullptr);
        REQUIRE(*v == "hello");
    }

    SUBCASE("shared_ptr construction")
    {
        auto        ptr {std::make_shared<i32>(99)};
        user_object obj {ptr};
        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == typeid(i32));
        auto* v {obj.get<i32>()};
        REQUIRE(v != nullptr);
        REQUIRE(*v == 99);
    }

    SUBCASE("shared_ptr construction shares ownership")
    {
        auto        ptr {std::make_shared<i32>(42)};
        user_object obj {ptr};
        REQUIRE(ptr.use_count() == 2);
    }
}

TEST_CASE("Core.UserObject.Assignment")
{
    SUBCASE("assign i32")
    {
        user_object obj;
        obj = 10;
        REQUIRE(obj.type() == typeid(i32));
        REQUIRE(*obj.get<i32>() == 10);
    }

    SUBCASE("assign string")
    {
        user_object obj;
        obj = string("world");
        REQUIRE(obj.type() == typeid(string));
        REQUIRE(*obj.get<string>() == "world");
    }

    SUBCASE("assign shared_ptr")
    {
        auto        ptr {std::make_shared<i32>(55)};
        user_object obj;
        obj = ptr;
        REQUIRE(obj.type() == typeid(i32));
        REQUIRE(*obj.get<i32>() == 55);
        REQUIRE(ptr.use_count() == 2);
    }

    SUBCASE("overwrite changes type")
    {
        user_object obj;
        obj = 1;
        REQUIRE(obj.type() == typeid(i32));
        obj = string("now string");
        REQUIRE(obj.type() == typeid(string));
        REQUIRE(obj.get<i32>() == nullptr);
        REQUIRE(*obj.get<string>() == "now string");
    }

    SUBCASE("overwrite shared_ptr releases previous")
    {
        auto        ptr {std::make_shared<i32>(1)};
        user_object obj {ptr};
        REQUIRE(ptr.use_count() == 2);
        obj = string("replaced");
        REQUIRE(ptr.use_count() == 1);
    }
}

TEST_CASE("Core.UserObject.TypeAndAccess")
{
    SUBCASE("type reporting")
    {
        user_object obj {3.14};
        REQUIRE(obj.type() == typeid(f64));
    }

    SUBCASE("type reporting with is")
    {
        user_object obj {3.14};
        REQUIRE(obj.is<f64>());
    }

    SUBCASE("get returns nullptr on mismatch")
    {
        user_object obj {100};
        REQUIRE(obj.get<f64>() == nullptr);
        REQUIRE(obj.get<string>() == nullptr);
    }

    SUBCASE("reset clears state")
    {
        user_object obj {string("test")};
        REQUIRE(obj.has_value());
        obj.reset();
        REQUIRE(!obj.has_value());
        REQUIRE(obj.type() == typeid(void));
        REQUIRE(obj.get<string>() == nullptr);
    }

    SUBCASE("reset releases shared_ptr")
    {
        auto        ptr {std::make_shared<i32>(1)};
        user_object obj {ptr};
        REQUIRE(ptr.use_count() == 2);
        obj.reset();
        REQUIRE(ptr.use_count() == 1);
    }

    SUBCASE("has_value behavior")
    {
        user_object obj;
        REQUIRE(!obj.has_value());
        obj = 123;
        REQUIRE(obj.has_value());
        obj.reset();
        REQUIRE(!obj.has_value());
    }

    SUBCASE("copy shares ownership")
    {
        auto        ptr {std::make_shared<i32>(7)};
        user_object a {ptr};
        user_object b {a}; // NOLINT
        REQUIRE(ptr.use_count() == 3);
        REQUIRE(*b.get<i32>() == 7);
    }
}
