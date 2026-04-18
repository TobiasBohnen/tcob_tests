#include "tests.hpp"

TEST_CASE("Core.UserObject.Construction")
{
    SUBCASE("default state")
    {
        user_object obj;
        REQUIRE_FALSE(obj.has_value());
        REQUIRE(obj.type() == typeid(void));
        REQUIRE(obj.get<i32>() == nullptr);
    }
    SUBCASE("SBO construction (i32)")
    {
        REQUIRE((sizeof(i32) <= 2 * sizeof(void*) && POD<i32>));

        user_object obj {42};
        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == typeid(i32));
        REQUIRE(obj.is<i32>());
        REQUIRE(*obj.get<i32>() == 42);
    }
    SUBCASE("SBO construction (f64)")
    {
        REQUIRE((sizeof(f64) <= 2 * sizeof(void*) && POD<f64>));

        user_object obj {3.14};
        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == typeid(f64));
        REQUIRE(obj.is<f64>());
        REQUIRE(*obj.get<f64>() == 3.14);
    }
    SUBCASE("heap construction (string)")
    {
        REQUIRE_FALSE((sizeof(string) <= 2 * sizeof(void*) && POD<string>));

        user_object obj {string("hello")};
        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == typeid(string));
        REQUIRE(obj.is<string>());
        REQUIRE(*obj.get<string>() == "hello");
    }
    SUBCASE("shared_ptr construction")
    {
        auto        ptr {std::make_shared<i32>(99)};
        user_object obj {ptr};
        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == typeid(i32));
        REQUIRE(obj.is<i32>());
        REQUIRE(*obj.get<i32>() == 99);
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
    SUBCASE("assign SBO (i32)")
    {
        user_object obj;
        obj = 10;
        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == typeid(i32));
        REQUIRE(*obj.get<i32>() == 10);
    }
    SUBCASE("assign heap (string)")
    {
        user_object obj;
        obj = string("world");
        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == typeid(string));
        REQUIRE(*obj.get<string>() == "world");
    }
    SUBCASE("assign shared_ptr")
    {
        auto        ptr {std::make_shared<i32>(55)};
        user_object obj;
        obj = ptr;
        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == typeid(i32));
        REQUIRE(*obj.get<i32>() == 55);
        REQUIRE(ptr.use_count() == 2);
    }
    SUBCASE("overwrite SBO with SBO")
    {
        user_object obj;
        obj = 1;
        REQUIRE(obj.type() == typeid(i32));
        obj = 2;
        REQUIRE(obj.type() == typeid(i32));
        REQUIRE(*obj.get<i32>() == 2);
    }
    SUBCASE("overwrite SBO with heap")
    {
        user_object obj;
        obj = 1;
        REQUIRE(obj.type() == typeid(i32));
        obj = string("now string");
        REQUIRE(obj.type() == typeid(string));
        REQUIRE(obj.get<i32>() == nullptr);
        REQUIRE(*obj.get<string>() == "now string");
    }
    SUBCASE("overwrite heap with SBO")
    {
        user_object obj;
        obj = string("heap");
        REQUIRE(obj.type() == typeid(string));
        obj = 1;
        REQUIRE(obj.type() == typeid(i32));
        REQUIRE(obj.get<string>() == nullptr);
        REQUIRE(*obj.get<i32>() == 1);
    }
    SUBCASE("overwrite shared_ptr with heap")
    {
        auto        ptr {std::make_shared<i32>(1)};
        user_object obj {ptr};
        REQUIRE(ptr.use_count() == 2);
        obj = string("replaced");
        REQUIRE(ptr.use_count() == 1);
        REQUIRE(*obj.get<string>() == "replaced");
    }
}

TEST_CASE("Core.UserObject.Is")
{
    SUBCASE("is() true on match (SBO)")
    {
        user_object obj {42};
        REQUIRE(obj.is<i32>());
    }
    SUBCASE("is() true on match (heap)")
    {
        user_object obj {string("hello")};
        REQUIRE(obj.is<string>());
    }
    SUBCASE("is() true on match (shared_ptr)")
    {
        auto        ptr {std::make_shared<i32>(1)};
        user_object obj {ptr};
        REQUIRE(obj.is<i32>());
    }
    SUBCASE("is() false on mismatch")
    {
        user_object obj {42};
        REQUIRE_FALSE(obj.is<f64>());
        REQUIRE_FALSE(obj.is<string>());
    }
    SUBCASE("is() false when empty")
    {
        user_object obj;
        REQUIRE_FALSE(obj.is<i32>());
    }
    SUBCASE("is() with const T")
    {
        user_object obj {42};
        REQUIRE(obj.is<i32 const>());
    }
}

TEST_CASE("Core.UserObject.Access")
{
    SUBCASE("get returns correct value (SBO)")
    {
        user_object obj {42};
        REQUIRE(*obj.get<i32>() == 42);
    }
    SUBCASE("get returns correct value (heap)")
    {
        user_object obj {string("hello")};
        REQUIRE(*obj.get<string>() == "hello");
    }
    SUBCASE("get returns correct value (shared_ptr)")
    {
        auto        ptr {std::make_shared<i32>(99)};
        user_object obj {ptr};
        REQUIRE(*obj.get<i32>() == 99);
    }
    SUBCASE("get returns nullptr on type mismatch")
    {
        user_object obj {100};
        REQUIRE(obj.get<f64>() == nullptr);
        REQUIRE(obj.get<string>() == nullptr);
    }
    SUBCASE("get const returns correct value")
    {
        user_object const obj {42};
        REQUIRE(*obj.get<i32>() == 42);
    }
    SUBCASE("get const returns nullptr on mismatch")
    {
        user_object const obj {42};
        REQUIRE(obj.get<f64>() == nullptr);
    }
}

TEST_CASE("Core.UserObject.Reset")
{
    SUBCASE("reset clears SBO")
    {
        user_object obj {42};
        REQUIRE(obj.has_value());
        obj.reset();
        REQUIRE_FALSE(obj.has_value());
        REQUIRE(obj.type() == typeid(void));
        REQUIRE(obj.get<i32>() == nullptr);
    }
    SUBCASE("reset clears heap")
    {
        user_object obj {string("test")};
        REQUIRE(obj.has_value());
        obj.reset();
        REQUIRE_FALSE(obj.has_value());
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
        REQUIRE_FALSE(obj.has_value());
    }
}

TEST_CASE("Core.UserObject.CopyMove")
{
    SUBCASE("copy SBO")
    {
        user_object a {7};
        user_object b {a};
        REQUIRE(*b.get<i32>() == 7);
    }
    SUBCASE("move SBO")
    {
        user_object a {7};
        user_object b {std::move(a)};
        REQUIRE(*b.get<i32>() == 7);
    }
    SUBCASE("copy heap")
    {
        user_object a {string("hello")};
        user_object b {a};
        REQUIRE(*b.get<string>() == "hello");
    }
    SUBCASE("move heap")
    {
        user_object a {string("hello")};
        user_object b {std::move(a)};
        REQUIRE(*b.get<string>() == "hello");
    }
    SUBCASE("copy shares shared_ptr ownership")
    {
        auto        ptr {std::make_shared<i32>(7)};
        user_object a {ptr};
        user_object b {a};
        REQUIRE(ptr.use_count() == 3);
        REQUIRE(*b.get<i32>() == 7);
    }
    SUBCASE("move transfers shared_ptr ownership")
    {
        auto        ptr {std::make_shared<i32>(7)};
        user_object a {ptr};
        REQUIRE(ptr.use_count() == 2);
        user_object b {std::move(a)};
        REQUIRE(ptr.use_count() == 2);
        REQUIRE(*b.get<i32>() == 7);
    }
}
