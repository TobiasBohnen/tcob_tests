#include "tests.hpp"

TEST_CASE("Core.UserObject.Construction")
{
    SUBCASE("default state")
    {
        user_object obj;

        REQUIRE(obj.has_value() == false);
        REQUIRE(obj.get<i32>() == nullptr);
    }

    SUBCASE("value construction (i32)")
    {
        user_object obj {42};

        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == typeid(i32));

        auto* v = obj.get<i32>();
        REQUIRE(v != nullptr);
        REQUIRE(*v == 42);
    }

    SUBCASE("value construction (string)")
    {
        user_object obj {string("hello")};

        REQUIRE(obj.type() == typeid(string));

        auto* v = obj.get<string>();
        REQUIRE(v != nullptr);
        REQUIRE(*v == "hello");
    }

    SUBCASE("Make<T> construction")
    {
        struct Foo {
            i32 a;
            i32 b;
            Foo(i32 x, i32 y)
                : a(x)
                , b(y)
            {
            }
        };

        auto obj {user_object::Make<Foo>(1, 2)};

        REQUIRE(obj.type() == typeid(Foo));

        auto* f = obj.get<Foo>();
        REQUIRE(f != nullptr);
        REQUIRE(f->a == 1);
        REQUIRE(f->b == 2);
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
}

TEST_CASE("Core.UserObject.TypeAndAccess")
{
    SUBCASE("type reporting")
    {
        user_object obj {3.14};
        REQUIRE(obj.type() == typeid(f64));
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

    SUBCASE("has_value behavior")
    {
        user_object obj;

        REQUIRE(!obj.has_value());

        obj = 123;

        REQUIRE(obj.has_value());
    }
}
