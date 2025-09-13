#include "tests.hpp"
#include <string>

using namespace tcob::data;

TEST_CASE("Data.Config.Get")
{
    object t;
    t["section1"]["valueBool"]          = true;
    t["section1"]["valueStr"]           = "test123";
    t["section1"]["valueSec"]["a"]      = 100;
    t["section1"]["valueSec"]["b"]      = false;
    t["section1"]["valueSec"]["c"]["l"] = 1;
    t["section1"]["valueSec"]["c"]["m"] = 32;
    t["section1"]["valueArr"]           = std::vector<i32> {3, 5, 9, 13};
    t["section1"]["valueFloat"]         = 123.45;
    t["section1"]["max"]                = std::numeric_limits<u64>::max();

    t["section2"]["valueBool"]  = false;
    t["section2"]["valueStr"]   = "test456";
    t["section2"]["valueInt"]   = 42;
    t["section2"]["valueFloat"] = 456.78;
    t["section2"]["value.Str"]  = "123";
    t["section2"]["value=Str"]  = "123";

    SUBCASE("try_get function")
    {
        object obj {};
        REQUIRE(t.try_get(obj, "section1"));

        bool b = false;
        REQUIRE(obj.try_get<bool>(b, "valueBool"));

        REQUIRE_FALSE(obj.try_get<bool>(b, "valueBoolXXX"));
        REQUIRE_FALSE(obj.try_get<bool>(b, "valueFloat"));

        REQUIRE(t.try_get<bool>(b, "section1", "valueSec", "b"));

        std::vector<i32> vec {4, 5, 6};
        REQUIRE(obj.try_get(vec, "valueArr"));
        REQUIRE(vec == std::vector<i32> {3, 5, 9, 13});
    }
    SUBCASE("'as' function")
    {
        REQUIRE(t.as<bool>("section1", "valueBool") == true);
        REQUIRE(t.as<std::string>("section1", "valueStr") == "test123");
        REQUIRE(t.as<std::string>("section2", "value.Str") == "123");
        REQUIRE(t.as<std::string>("section2", "value=Str") == "123");
        REQUIRE(t.as<f64>("section1", "valueFloat") == 123.45);
        REQUIRE(t.as<f64>("section1", "valueSec", "a") == 100);
        REQUIRE(t.as<bool>("section1", "valueSec", "b") == false);
        REQUIRE(t.as<f64>("section1", "valueSec", "c", "l") == 1);
        REQUIRE(t.as<f64>("section1", "valueArr", 2) == 9);

        object obj {t.as<object>("section1")};
        REQUIRE(obj.as<bool>("valueBool") == true);
        REQUIRE(obj.as<std::string>("valueStr") == "test123");
        REQUIRE(obj.as<f64>("valueFloat") == 123.45);

        REQUIRE(obj.as<u64>("max") == std::numeric_limits<u64>::max());
    }
    SUBCASE("subscript")
    {
        REQUIRE(t["section1"]["valueBool"].as<bool>() == true);
        REQUIRE(t["section1"]["valueStr"].as<std::string>() == "test123");
        REQUIRE(t["section1"]["valueFloat"].as<f64>() == 123.45);
        REQUIRE(t["section1"]["valueSec"]["a"].as<f64>() == 100);
        REQUIRE(t["section1"]["valueSec"]["b"].as<bool>() == false);

        REQUIRE(t["section2"]["valueBool"].as<bool>() == false);
        REQUIRE(t["section2"]["valueStr"].as<std::string>() == "test456");
        REQUIRE(t["section2"]["valueFloat"].as<f64>() == 456.78);
    }
    SUBCASE("get non-native types")
    {
        REQUIRE(t["section2"]["valueFloat"].as<f32>() == 456.78f);
        REQUIRE(t["section2"]["valueFloat"].get<i32>().error() == error_code::TypeMismatch);
    }
    SUBCASE("get everything as string")
    {
        REQUIRE(t["section1"]["valueBool"].as<std::string>() == "true");
        REQUIRE(t["section2"]["valueBool"].as<std::string>() == "false");
        REQUIRE(t["section2"]["valueStr"].as<std::string>() == "test456");
        REQUIRE(t["section2"]["valueInt"].as<std::string>() == "42");
        REQUIRE(t["section2"]["valueFloat"].as<std::string>() == std::to_string(456.78));
        REQUIRE(t["section1"]["valueArr"].as<std::string>() == "[ 3, 5, 9, 13 ]");
        REQUIRE(t["section1"]["valueSec"].as<std::string>() == "{ a = 100, b = false, c = { l = 1, m = 32 } }");
    }
}

TEST_CASE("Data.Config.Set")
{
    SUBCASE("set function")
    {
        object t;
        t.set("section1", "valueBool", true);
        t.set("section1", "valueStr", "test123");
        t.set("section1", "valueFloat", 123.45);
        t.set("section1", "valueSec", "a", 95);
        t.set("section1", "valueArr", 0, 42);

        REQUIRE(t.as<bool>("section1", "valueBool") == true);
        REQUIRE(t.as<std::string>("section1", "valueStr") == "test123");
        REQUIRE(t.as<f64>("section1", "valueFloat") == 123.45);
        REQUIRE(t.as<f64>("section1", "valueSec", "a") == 95);
        REQUIRE(t.as<f64>("section1", "valueArr", 0) == 42);
    }
    SUBCASE("subscript")
    {
        object t;
        t["section1"]["valueBool"]  = true;
        t["section1"]["valueStr"]   = "test123";
        t["section1"]["valueFloat"] = 123.45;

        REQUIRE(t["section1"]["valueBool"].as<bool>() == true);
        REQUIRE(t["section1"]["valueStr"].as<std::string>() == "test123");
        REQUIRE(t["section1"]["valueFloat"].as<f64>() == 123.45);
    }
    SUBCASE("set non-native types")
    {
        object t;
        t["section1"]["f32"] = 123.45f;
        t["section1"]["i32"] = 123;
        REQUIRE(t["section1"]["f32"].as<f32>() == 123.45f);
        REQUIRE(t["section1"]["i32"].as<i32>() == 123);

        char const* x {"123"};
        t["section1"]["valueStr"] = x;
        REQUIRE(t["section1"]["valueStr"].as<std::string>() == x);
    }
    SUBCASE("replace with object")
    {
        object t;
        t["section1"]["value"] = 123.45f;
        REQUIRE(t["section1"]["value"].as<f32>() == 123.45f);

        t["section1"]["value"]["cvalue"] = 100;
        REQUIRE(t["section1"]["value"]["cvalue"].as<i32>() == 100);
    }
    SUBCASE("monostate")
    {
        object t;
        t["value"] = 123.45f;
        REQUIRE(t["value"].as<f32>() == 123.45f);
        t["value"] = std::monostate {};
        REQUIRE(t.has("value"));
        REQUIRE_FALSE(t["value"].is<f32>());
        REQUIRE(t.is<std::monostate>("value"));
    }
}

TEST_CASE("Data.Config.Has")
{
    object t;
    t["valueBool"] = false;

    t["section1"]["valueBool"]             = true;
    t["section1"]["valueSec"]["a"]         = 100;
    t["section1"]["valueSec"]["valueBool"] = false;

    t["section2"]["valueBool"] = false;

    REQUIRE(t.has("valueBool"));
    REQUIRE(t.has("section1", "valueBool"));
    REQUIRE(t.has("section2", "valueBool"));
    REQUIRE(t.has("section1", "valueSec", "valueBool"));
}

TEST_CASE("Data.Config.Is")
{
    object t;
    t["valueBool"] = false;

    t["section1"]["valueBool"]             = true;
    t["section1"]["valueFloat"]            = 2.0;
    t["section1"]["valueInt"]              = 32;
    t["section1"]["valueSec"]["a"]         = 100;
    t["section1"]["valueSec"]["valueBool"] = false;
    t["section1"]["valueSec2"]["a"]        = 100;
    t["section1"]["valueSec2"]["arr"]      = std::vector<std::variant<i32, bool, std::string>> {0, false, "ok"};
    t["section1"]["valueArr"]              = std::vector<std::variant<i32, bool, std::string>> {0, false, "ok"};

    t["section2"]["valueBool"] = false;

    REQUIRE(t.is<bool>("valueBool"));
    REQUIRE(t.is<bool>("section1", "valueBool"));
    REQUIRE(t.is<bool>("section2", "valueBool"));
    REQUIRE(t.is<bool>("section1", "valueSec", "valueBool"));
    REQUIRE(t.is<i64>("section1", "valueArr", 0));
    REQUIRE(t.is<bool>("section1", "valueArr", 1));
    REQUIRE(t.is<std::string>("section1", "valueArr", 2));
    REQUIRE(t.is<i64>("section1", "valueSec2", "arr", 0));
    REQUIRE(t.is<bool>("section1", "valueSec2", "arr", 1));
    REQUIRE(t.is<std::string>("section1", "valueSec2", "arr", 2));
    REQUIRE(t.is<f64>("section1", "valueFloat"));
    REQUIRE(t.is<i64>("section1", "valueInt"));

    REQUIRE_FALSE(t.is<i64>("section1", "valueFloat"));
    REQUIRE_FALSE(t.is<bool>("section1", "valueInt"));
}

TEST_CASE("Data.Config.Object")
{
    SUBCASE("modify section")
    {
        object t;
        t["section1"]["valueSection"] = object {};
        REQUIRE(t["section1"]["valueSection"].is<object>());

        object obj {t["section1"]["valueSection"].as<object>()};

        object obj1 {t["section1"]["valueSection"].as<object>()};
        obj["a"] = 100;
        REQUIRE(obj1["a"].as<f64>() == 100);
        obj["b"] = false;
        REQUIRE(obj1["b"].as<bool>() == false);
        obj["xyz"] = "testString";
        REQUIRE(obj1["xyz"].as<std::string>() == "testString");

        object obj2 {t["section1"]["valueSection"].as<object>()};
        REQUIRE(obj2["a"].as<f64>() == 100);
        REQUIRE(obj2["b"].as<bool>() == false);
        REQUIRE(obj2["xyz"].as<std::string>() == "testString");
    }

    SUBCASE("adding and removing object")
    {
        object t;
        object obj {{"a", 100}, {"b", false}, {"xyz", "testString"}};
        t.set("section1", obj);

        REQUIRE(t.has("section1"));
        object sec2 {t["section1"].as<object>()};
        REQUIRE(sec2["a"].as<f64>() == 100);
        REQUIRE(sec2["xyz"].as<std::string>() == "testString");
        REQUIRE(sec2["b"].as<bool>() == false);

        t.set("section1", nullptr);
        REQUIRE_FALSE(t.has("section1"));
    }

    SUBCASE("try_make")
    {
        {
            object s0;
            s0["x"] = 100;
            s0["y"] = 200;
            point_f p;
            REQUIRE(s0.try_make<f32, f32>(p, "x", "y"));
            REQUIRE(p == point_f {100, 200});
        }
        {
            object s0;
            s0["x"] = 100;
            s0["y"] = 200;
            size_f p;
            REQUIRE(s0.try_make<f32, f32>(p, "x", "y"));
            REQUIRE(p == size_f {100, 200});
        }
        {
            object s0;
            s0["x"] = 100;
            s0["z"] = 200;
            point_f p;
            REQUIRE_FALSE(s0.try_make<f32, f32>(p, "x", "y"));
        }
        {
            object s0;
            s0["x"] = 100;
            s0["y"] = false;
            point_f p;
            REQUIRE_FALSE(s0.try_make<f32, f32>(p, "x", "y"));
        }
    }

    SUBCASE("clone")
    {
        object original;
        original["section1"]["a"] = 100;

        object copy = original;
        REQUIRE(copy["section1"]["a"].as<i32>() == 100);
        copy["section1"]["a"] = 200;
        REQUIRE(original["section1"]["a"].as<i32>() == 200);
        REQUIRE(copy["section1"]["a"].as<i32>() == 200);

        object clone = original.clone(true);
        REQUIRE(clone["section1"]["a"].as<i32>() == 200);
        clone["section1"]["a"] = 400;
        REQUIRE(original["section1"]["a"].as<i32>() == 200);
        REQUIRE(copy["section1"]["a"].as<i32>() == 200);
        REQUIRE(clone["section1"]["a"].as<i32>() == 400);
    }

    SUBCASE("merge")
    {
        {
            object s0;
            s0["section1"] = object {{"a", 100}, {"b", 200}};
            s0["section2"] = object {{"a", 300}};

            object s1;
            s1["section1"] = object {{"a", 150}, {"c", 400}};
            s1["section3"] = object {{"a", 500}};

            s0.merge(s1, true);

            REQUIRE(s0["section1"]["a"].as<i32>() == 150);
            REQUIRE(s0["section1"]["b"].as<i32>() == 200);
            REQUIRE(s0["section1"]["c"].as<i32>() == 400);
            REQUIRE(s0["section2"]["a"].as<i32>() == 300);
            REQUIRE(s0["section3"]["a"].as<i32>() == 500);
        }

        {
            object s0;
            s0["section1"]["a"] = 100;
            s0["section1"]["b"] = 200;
            s0["section2"]["a"] = 300;

            object s1;
            s1["section1"]["a"] = 150;
            s1["section1"]["c"] = 400;
            s1["section3"]["a"] = 500;

            s0.merge(s1, false);

            REQUIRE(s0["section1"]["a"].as<i32>() == 100);
            REQUIRE(s0["section1"]["b"].as<i32>() == 200);
            REQUIRE(s0["section1"]["c"].as<i32>() == 400);
            REQUIRE(s0["section2"]["a"].as<i32>() == 300);
            REQUIRE(s0["section3"]["a"].as<i32>() == 500);
        }
        {
            object s0;
            s0["texture"]["tex1"]["source"] = "tex1.png";

            object s1;
            s1["texture"]["tex2"]["source"] = "tex2.png";

            object tex;
            tex.merge(s0, true);
            tex.merge(s1, true);

            REQUIRE(tex["texture"]["tex1"]["source"].as<std::string>() == "tex1.png");
            REQUIRE(tex["texture"]["tex2"]["source"].as<std::string>() == "tex2.png");
        }
    }

    SUBCASE("removing keys")
    {
        object obj {};
        obj["a"]      = 100;
        obj["b"]      = false;
        obj["xyz"]    = "testString";
        obj["c"]["d"] = 1;
        obj["c"]["e"] = 2;

        REQUIRE(obj.has("a"));
        REQUIRE(obj.has("b"));
        REQUIRE(obj.has("xyz"));
        REQUIRE(obj.has("c", "d"));
        REQUIRE(obj.has("c", "e"));

        obj["a"]      = nullptr;
        obj["b"]      = nullptr;
        obj["xyz"]    = nullptr;
        obj["c"]["d"] = nullptr;
        obj["c"]["e"] = nullptr;

        REQUIRE_FALSE(obj.has("a"));
        REQUIRE_FALSE(obj.has("b"));
        REQUIRE_FALSE(obj.has("xyz"));
        REQUIRE_FALSE(obj.has("c", "d"));
        REQUIRE_FALSE(obj.has("c", "e"));

        // delete non-existing key
        REQUIRE_FALSE(obj.has("c", "x"));
        obj["c"]["x"]["s"] = nullptr;
        REQUIRE_FALSE(obj.has("c", "x"));
        REQUIRE_FALSE(obj.has("c", "x", "s"));
    }

    SUBCASE("equality")
    {
        object test;
        test["a"]      = 100;
        test["b"]      = 200;
        test["c"]      = array {1, 2, 3};
        test["d"]["a"] = 100;
        test["d"]["b"] = 300;
        test["d"]["c"] = 400;

        object good;
        good["a"]      = 100;
        good["b"]      = 200;
        good["c"]      = array {1, 2, 3};
        good["d"]["a"] = 100;
        good["d"]["b"] = 300;
        good["d"]["c"] = 400;

        REQUIRE(test == good);

        object bad0;
        bad0["a"] = 100;
        bad0["b"] = 200;
        bad0["c"] = true;
        bad0["d"] = false;

        REQUIRE_FALSE(test == bad0);

        object bad1;
        bad1["a"]      = 100;
        bad1["b"]      = 200;
        bad1["c"]      = array {1, 2, 1};
        bad1["d"]["a"] = 100;
        bad1["d"]["b"] = 300;
        bad1["d"]["c"] = 400;
        REQUIRE_FALSE(test == bad1);
    }

    SUBCASE("get_type")
    {
        object t;
        t["string"]      = "abc";
        t["float"]       = 1.2;
        t["int"]         = 100;
        t["bool"]        = true;
        t["array"]       = array {1, 2, 3};
        t["object"]["a"] = 1;
        t["object"]["b"] = 2;
        t["object"]["c"] = 3;

        REQUIRE(t.get_type("string") == type::String);
        REQUIRE(t.get_type("float") == type::Float);
        REQUIRE(t.get_type("int") == type::Integer);
        REQUIRE(t.get_type("bool") == type::Bool);
        REQUIRE(t.get_type("array") == type::Array);
        REQUIRE(t.get_type("object") == type::Object);
        REQUIRE(t.get_type("foobar") == type::Null);
    }
}

TEST_CASE("Data.Config.Array")
{
    SUBCASE("access items")
    {
        object t;
        t["section1"]["valueArray"] = array {1, "a", true};

        REQUIRE(t["section1"]["valueArray"].is<array>());
        array arr {t["section1"]["valueArray"].as<array>()};
        REQUIRE(arr.size() == 3);
        REQUIRE(arr[0].is<i64>());
        REQUIRE(arr[1].is<std::string>());
        REQUIRE(arr[2].is<bool>());
        REQUIRE(arr[0].as<f64>() == 1);
        REQUIRE(arr[1].as<std::string>() == "a");
        REQUIRE(arr[2].as<bool>() == true);
    }

    SUBCASE("modify array")
    {
        object t;
        t["section1"]["valueArray"] = array {1, "a", true};

        REQUIRE(t["section1"]["valueArray"].is<array>());

        array arr {t["section1"]["valueArray"].as<array>()};
        REQUIRE(arr.size() == 3);

        array arr1 {t["section1"]["valueArray"].as<array>()};
        arr[0] = 100;
        REQUIRE(arr1[0].as<f64>() == 100);
        arr[1] = false;
        REQUIRE(arr1[1].as<bool>() == false);
        arr[2] = "testString";
        REQUIRE(arr1[2].as<std::string>() == "testString");

        array arr2 {t["section1"]["valueArray"].as<array>()};
        REQUIRE(arr2[0].as<f64>() == 100);
        REQUIRE(arr2[1].as<bool>() == false);
        REQUIRE(arr2[2].as<std::string>() == "testString");
    }

    SUBCASE("auto grow")
    {
        array a;
        a[100] = 1;
        REQUIRE(a.size() == 101);
        REQUIRE(a[100].as<i32>() == 1);
    }

    SUBCASE("from vector")
    {
        {
            std::vector<i32> vec {1, 2, 3, 4, 5, 6};
            array            testArray {std::span<i32 const> {vec}};
            REQUIRE(testArray.size() == vec.size());
            for (usize i {0}; i < vec.size(); ++i) {
                REQUIRE(testArray[static_cast<isize>(i)].as<i32>() == vec[i]);
            }
        }
        {
            std::vector<f64> vec {1.1, 2.2, 3.3, 4.4, 5.5, 6.6};
            array            testArray {std::span<f64 const> {vec}};
            REQUIRE(testArray.size() == vec.size());
            for (usize i {0}; i < vec.size(); ++i) {
                REQUIRE(testArray[static_cast<isize>(i)].as<f64>() == vec[i]);
            }
        }
    }

    SUBCASE("from values")
    {
        {
            std::vector<i32> vec {1, 2, 3, 4, 5, 6};
            array            testArray {1, 2, 3, 4, 5, 6};
            REQUIRE(testArray.size() == vec.size());
            for (usize i {0}; i < vec.size(); ++i) {
                REQUIRE(testArray[static_cast<isize>(i)].as<i32>() == vec[i]);
            }
        }
        {
            std::vector<f64> vec {1.1, 2.2, 3.3, 4.4, 5.5, 6.6};
            array            testArray {1.1, 2.2, 3.3, 4.4, 5.5, 6.6};
            REQUIRE(testArray.size() == vec.size());
            for (usize i {0}; i < vec.size(); ++i) {
                REQUIRE(testArray[static_cast<isize>(i)].as<f64>() == vec[i]);
            }
        }
    }

    SUBCASE("equality")
    {
        array test {1, 2, 3};
        array good {1, 2, 3};

        REQUIRE(test == good);

        array bad {1, true, 3};

        REQUIRE_FALSE(test == bad);
    }

    SUBCASE("get_type")
    {
        array arr {"a", 1.2, 3, true, array {}, object {}};

        REQUIRE(arr.get_type(0) == type::String);
        REQUIRE(arr.get_type(1) == type::Float);
        REQUIRE(arr.get_type(2) == type::Integer);
        REQUIRE(arr.get_type(3) == type::Bool);
        REQUIRE(arr.get_type(4) == type::Array);
        REQUIRE(arr.get_type(5) == type::Object);
        REQUIRE(arr.get_type(6) == type::Null);
    }

    SUBCASE("make")
    {
        {
            array   pointArr {1, 2};
            point_f point {pointArr.make<point_f, f32, f32>()};
            REQUIRE(point == point_f {1, 2});
        }
        {
            array   pointArr {1, 2};
            point_f point {pointArr.make<point_f, f32, f32>(0, 1)};
            REQUIRE(point == point_f {1, 2});
        }
        {
            array   pointArr {"x", 12, false, 23};
            point_f point {pointArr.make<point_f, f32, f32>(1, 3)};
            REQUIRE(point == point_f {12, 23});
        }
    }
}

TEST_CASE("Data.Config.TcobTypes")
{
    SUBCASE("to/from")
    {
        object obj;

        obj["point"] = point_i {100, 350};
        REQUIRE(obj.is<point_i>("point"));
        REQUIRE(obj["point"].as<point_i>() == point_i {100, 350});

        obj["color"] = color {15, 30, 12, 0};
        REQUIRE(obj.is<color>("color"));
        REQUIRE(obj["color"].as<color>() == color {15, 30, 12, 0});

        obj["size"] = size_i {300, 450};
        REQUIRE(obj.is<size_i>("size"));
        REQUIRE(obj["size"].as<size_i>() == size_i {300, 450});

        obj["rect"] = rect_f {4.5f, 2.5f, 30.1f, 45.01f};
        REQUIRE(obj.is<rect_f>("rect"));
        REQUIRE(obj["rect"].as<rect_f>() == rect_f {4.5f, 2.5f, 30.1f, 45.01f});

        obj["alignments"] = alignments {.Horizontal = horizontal_alignment::Centered, .Vertical = vertical_alignment::Middle};
        REQUIRE(obj.is<alignments>("alignments"));
        REQUIRE(obj["alignments"].as<alignments>() == alignments {.Horizontal = horizontal_alignment::Centered, .Vertical = vertical_alignment::Middle});

        obj["point_particle_emitter::settings"] = point_particle_emitter::settings {.Template = {}, .IsExplosion = true, .SpawnArea = {1, 2, 3, 4}, .SpawnRate = 100, .Lifetime = 100s};
        REQUIRE(obj.is<point_particle_emitter::settings>("point_particle_emitter::settings"));
        REQUIRE(obj["point_particle_emitter::settings"].as<point_particle_emitter::settings>() == point_particle_emitter::settings {.Template = {}, .IsExplosion = true, .SpawnArea = {1, 2, 3, 4}, .SpawnRate = 100, .Lifetime = 100s});
        obj["point_particle_emitter::settings2"] = point_particle_emitter::settings {.Template = {}, .IsExplosion = true, .SpawnArea = {1, 2, 3, 4}, .SpawnRate = 100};
        REQUIRE(obj.is<point_particle_emitter::settings>("point_particle_emitter::settings2"));
        REQUIRE(obj["point_particle_emitter::settings2"].as<point_particle_emitter::settings>() == point_particle_emitter::settings {.Template = {}, .IsExplosion = true, .SpawnArea = {1, 2, 3, 4}, .SpawnRate = 100});
    }

    SUBCASE("from object")
    {
        object obj;

        obj["point"] = object {{"x", 100}, {"y", 350}};
        REQUIRE(obj.is<point_i>("point"));
        REQUIRE(obj.as<point_i>("point") == point_i {100, 350});

        obj["size"] = object {{"width", 100}, {"height", 350}};
        REQUIRE(obj.is<size_i>("size"));
        REQUIRE(obj.as<size_i>("size") == size_i {100, 350});

        obj["rect"] = object {{"x", 100}, {"y", 350}, {"width", 200}, {"height", 450}};
        REQUIRE(obj.is<rect_i>("rect"));
        REQUIRE(obj.as<rect_i>("rect") == rect_i {100, 350, 200, 450});
    }

    SUBCASE("from array")
    {
        object obj;

        obj["point"] = array {100, 350};
        REQUIRE(obj.is<point_i>("point"));
        REQUIRE(obj.as<point_i>("point") == point_i {100, 350});

        obj["size"] = array {100, 350};
        REQUIRE(obj.is<size_i>("size"));
        REQUIRE(obj.as<size_i>("size") == size_i {100, 350});

        obj["rect"] = array {100, 350, 200, 450};
        REQUIRE(obj.is<rect_i>("rect"));
        REQUIRE(obj.as<rect_i>("rect") == rect_i {100, 350, 200, 450});
    }
}

TEST_CASE("Data.Config.STLTypes")
{
    SUBCASE("vector")
    {
        {
            object obj;
            obj["stringArray"] = std::vector<std::string> {"One", "Two", "Three"};
            auto objectarr0    = obj["stringArray"].as<std::vector<std::string>>();
            REQUIRE(objectarr0 == std::vector<std::string> {"One", "Two", "Three"});

            obj["intArray"] = std::vector<i32> {1, 2, 3};
            auto objectarr1 = obj["intArray"].as<std::vector<i32>>();
            REQUIRE(objectarr1 == std::vector<i32> {1, 2, 3});

            obj["intArray2"] = std::vector<i64> {0, 5, 10};
            auto objectarr2  = obj["intArray2"].as<std::vector<i64>>();
            REQUIRE(objectarr2 == std::vector<i64> {0, 5, 10});
        }
    }

    SUBCASE("array")
    {
        {
            object obj;
            obj["stringArray"] = std::vector<std::string> {"One", "Two", "Three"};
            auto objectarr0    = obj["stringArray"].as<std::array<std::string, 3>>();
            REQUIRE(objectarr0 == std::array<std::string, 3> {"One", "Two", "Three"});

            obj["intArray"] = std::vector<i32> {1, 2, 3};
            auto objectarr1 = obj["intArray"].as<std::array<i32, 3>>();
            REQUIRE(objectarr1 == std::array<i32, 3> {1, 2, 3});

            obj["stringArray2"] = std::array<std::string, 3> {"a", "b", "c"};
            auto objectarr2     = obj["stringArray2"].as<std::array<std::string, 3>>();
            REQUIRE(objectarr2 == std::array<std::string, 3> {"a", "b", "c"});

            obj["intArray2"] = std::array<i32, 3> {0, 5, 10};
            auto objectarr3  = obj["intArray2"].as<std::array<i32, 3>>();
            REQUIRE(objectarr3 == std::array<i32, 3> {0, 5, 10});
        }
    }

    SUBCASE("tuple")
    {
        {
            object obj;
            obj["tuple"] = std::tuple<i32, std::string, bool> {123, "ok", true};
            REQUIRE(obj["tuple"].is<std::tuple<i32, std::string, bool>>());
            auto tup0 = obj["tuple"].as<std::tuple<i32, std::string, bool>>();
            REQUIRE(tup0 == std::tuple<i32, std::string, bool> {123, "ok", true});

            obj["tuple2"] = std::tuple<std::string, bool, f32> {"a", false, 3.5f};
            REQUIRE(obj["tuple2"].is<std::tuple<std::string, bool, f32>>());
            auto tup1 = obj["tuple2"].as<std::tuple<std::string, bool, f32>>();
            REQUIRE(tup1 == std::tuple<std::string, bool, f32> {"a", false, 3.5f});

            REQUIRE_FALSE(obj["tuple"].is<std::tuple<i32, std::string>>());
            REQUIRE_FALSE(obj["tuple"].is<std::tuple<std::string, i32, bool>>());
            REQUIRE_FALSE(obj["tuple"].is<std::tuple<i32, f32, bool>>());
        }
    }

    SUBCASE("pair")
    {
        {
            object obj;
            obj["pair"] = std::pair<std::string, i32> {"ok", 100};
            auto pair0  = obj["pair"].as<std::pair<std::string, i32>>();
            REQUIRE(pair0 == std::pair<std::string, i32> {"ok", 100});
        }
    }

    SUBCASE("variant")
    {
        {
            object obj;
            obj["duration"] = 100;
            auto var        = obj["duration"].as<std::variant<std::string, i32>>();
            REQUIRE(std::get<i32>(var) == 100);
        }
    }

    SUBCASE("optional")
    {
        {
            object obj;
            obj["optional"] = 100;
            auto var        = obj["optional"].as<std::optional<i32>>();
            REQUIRE(var);
            REQUIRE(*var == 100);

            std::optional<i32> opt {std::nullopt};
            obj["optional2"] = opt;
            var              = obj["optional2"].as<std::optional<i32>>();
            REQUIRE_FALSE(var);

            opt              = 1234;
            obj["optional3"] = opt;
            var              = obj["optional3"].as<std::optional<i32>>();
            REQUIRE(*var == 1234);

            opt              = std::nullopt;
            obj["optional3"] = opt;
            REQUIRE(obj["optional3"].is<std::monostate>());
            REQUIRE(obj["optional3"].as<std::optional<i32>>() == std::nullopt);

            obj["optional4"] = 123;
            REQUIRE(obj["optional4"].as<std::optional<i32>>() == 123);
            obj["optional4"] = std::nullopt;
            REQUIRE(obj["optional4"].is<std::monostate>());
            REQUIRE(obj["optional4"].as<std::optional<i32>>() == std::nullopt);
        }
    }

    SUBCASE("map")
    {
        {
            object obj;
            obj["stringintMap"]["a"] = 123;
            obj["stringintMap"]["b"] = 456;
            REQUIRE(obj["stringintMap"].is<std::map<std::string, i32>>());
            auto objectMap = obj["stringintMap"].as<std::map<std::string, i32>>();
            REQUIRE(objectMap.size() == 2);
            REQUIRE(objectMap["a"] == 123);
            REQUIRE(objectMap["b"] == 456);
        }

        {
            object obj;
            obj["stringintMap2"] = std::map<std::string, i32> {{"c", 555}, {"d", 666}};
            REQUIRE(obj["stringintMap2"].is<std::map<std::string, i32>>());
            auto objectMap = obj["stringintMap2"].as<std::map<std::string, i32>>();
            REQUIRE(objectMap.size() == 2);
            REQUIRE(objectMap["c"] == 555);
            REQUIRE(objectMap["d"] == 666);
        }

        {
            object obj;
            obj["variantMap"]["f"] = 1.5;
            obj["variantMap"]["b"] = true;
            obj["variantMap"]["s"] = "ok";
            REQUIRE(obj["variantMap"].is<std::unordered_map<std::string, cfg_value>>());
            auto objectMap = obj["variantMap"].as<std::map<std::string, cfg_value>>();
            REQUIRE(objectMap.size() == 3);
            REQUIRE(std::get<f64>(objectMap["f"]) == 1.5);
            REQUIRE(std::get<bool>(objectMap["b"]) == true);
            REQUIRE(std::get<std::string>(objectMap["s"]) == "ok");
        }
    }

    SUBCASE("unordered_map")
    {
        {
            object obj;
            obj["stringintMap"]["a"] = 123;
            obj["stringintMap"]["b"] = 456;
            REQUIRE(obj["stringintMap"].is<std::unordered_map<std::string, i32>>());
            auto objectMap = obj["stringintMap"].as<std::unordered_map<std::string, i32>>();
            REQUIRE(objectMap.size() == 2);
            REQUIRE(objectMap["a"] == 123);
            REQUIRE(objectMap["b"] == 456);
        }

        {
            object obj;
            obj["stringintMap2"] = std::map<std::string, i32> {{"c", 555}, {"d", 666}};
            REQUIRE(obj["stringintMap2"].is<std::unordered_map<std::string, i32>>());
            auto objectMap = obj["stringintMap2"].as<std::unordered_map<std::string, i32>>();
            REQUIRE(objectMap.size() == 2);
            REQUIRE(objectMap["c"] == 555);
            REQUIRE(objectMap["d"] == 666);
        }
    }

    SUBCASE("set")
    {
        {
            object obj;
            obj["stringArray"] = std::vector<std::string> {"One", "Two", "Three"};
            obj["intArray"]    = std::vector<i32> {1, 2, 3};
            obj["set"]         = std::vector<i32> {1, 1, 2, 2, 3, 3};

            REQUIRE(obj["stringArray"].is<std::set<std::string>>());
            auto objectarr0 = obj["stringArray"].as<std::set<std::string>>();
            REQUIRE(objectarr0 == std::set<std::string> {"One", "Two", "Three"});

            REQUIRE(obj["intArray"].is<std::set<i32>>());
            auto objectarr1 = obj["intArray"].as<std::set<i32>>();
            REQUIRE(objectarr1 == std::set<i32> {1, 2, 3});

            REQUIRE(obj["set"].is<std::set<i32>>());
            auto objectarr2 = obj["set"].as<std::set<i32>>();
            REQUIRE(objectarr2 == std::set<i32> {1, 2, 3});
        }
    }

    SUBCASE("unordered_set")
    {
        {
            object obj;
            obj["stringArray"] = std::vector<std::string> {"One", "Two", "Three"};
            obj["intArray"]    = std::vector<i32> {1, 2, 3};
            obj["set"]         = std::vector<i32> {1, 1, 2, 2, 3, 3};

            auto objectarr0 = obj["stringArray"].as<std::unordered_set<std::string>>();
            REQUIRE(objectarr0 == std::unordered_set<std::string> {"One", "Two", "Three"});

            auto objectarr1 = obj["intArray"].as<std::unordered_set<i32>>();
            REQUIRE(objectarr1 == std::unordered_set<i32> {1, 2, 3});

            auto objectarr2 = obj["set"].as<std::unordered_set<i32>>();
            REQUIRE(objectarr2 == std::unordered_set<i32> {1, 2, 3});
        }
    }

    SUBCASE("duration")
    {
        {
            object obj;
            obj["duration"] = 100;
            auto value      = obj["duration"].as<milliseconds>();
            REQUIRE(value == milliseconds {100});
        }

        {
            object obj;
            obj["duration2"] = milliseconds {360};
            auto value       = obj["duration2"].as<milliseconds>();
            REQUIRE(value == milliseconds {360});
        }
    }
}

TEST_CASE("Data.Config.ForLoop")
{
    SUBCASE("object")
    {
        object obj {};
        obj["first"]  = 0;
        obj["second"] = 2;
        obj["third"]  = 12;

        std::set<i32>         values;
        std::set<std::string> names;
        for (auto const& [k, v] : obj) {
            values.insert(v.as<i32>());
            names.insert(k);
        }

        REQUIRE(values == std::set<i32> {0, 2, 12});
        REQUIRE(names == std::set<std::string> {"first", "second", "third"});
    }

    SUBCASE("array")
    {
        array arr {};
        arr.add(0);
        arr.add(2);
        arr.add(12);

        std::vector<i32> values;
        for (auto const& value : arr) {
            values.push_back(value.as<i32>());
        }

        REQUIRE(values == std::vector<i32> {0, 2, 12});
    }
}

TEST_CASE("Data.Config.Schema")
{
    SUBCASE("AllOf")
    {
        schema s0;
        s0.AllOf = {
            schema::string_property {.Name = "string"},
            schema::float_property {.Name = "float"},
            schema::int_property {.Name = "integer"},
            schema::array_property {.Name = "array"},
            schema::object_property {.Name = "object"},
            schema::bool_property {.Name = "bool"},
        };

        object goodSection {};
        goodSection["string"]  = "ok";
        goodSection["float"]   = 2.f;
        goodSection["integer"] = 2;
        goodSection["array"]   = array {};
        goodSection["object"]  = object {};
        goodSection["bool"]    = true;
        REQUIRE(s0.validate(goodSection));

        object badSection0 {};
        badSection0["x"]   = "ok";
        badSection0["y"]   = 2;
        badSection0["z"]   = array {};
        badSection0["aaa"] = object {};
        badSection0["bbb"] = true;
        REQUIRE_FALSE(s0.validate(badSection0));

        object badSection1 {};
        badSection1["string"] = "ok";
        badSection1["float"]  = 2;
        badSection1["array"]  = array {};
        badSection1["object"] = object {};
        badSection1["bool"]   = true;
        REQUIRE_FALSE(s0.validate(badSection1));

        object badSection2 {};
        badSection2["string"]  = "ok";
        badSection2["float"]   = 2.f;
        badSection2["integer"] = "2";
        badSection2["array"]   = array {};
        badSection2["object"]  = object {};
        badSection2["bool"]    = true;
        REQUIRE_FALSE(s0.validate(badSection2));
    }

    SUBCASE("AnyOf")
    {
        schema s0;
        s0.AnyOf = {
            schema::string_property {.Name = "string"},
            schema::float_property {.Name = "float"},
            schema::int_property {.Name = "integer"},
        };

        object goodSection0 {};
        goodSection0["string"]  = "ok";
        goodSection0["float"]   = 2.f;
        goodSection0["integer"] = 2;
        REQUIRE(s0.validate(goodSection0));

        object goodSection1 {};
        goodSection1["string"] = "ok";
        REQUIRE(s0.validate(goodSection1));

        object goodSection2 {};
        goodSection2["float"]   = 2.f;
        goodSection2["integer"] = 2;
        REQUIRE(s0.validate(goodSection2));

        object badSection0 {};
        badSection0["bla"] = 2.f;
        REQUIRE_FALSE(s0.validate(badSection0));
    }

    SUBCASE("OneOf")
    {
        schema s0;
        s0.OneOf = {
            schema::string_property {.Name = "string"},
            schema::float_property {.Name = "float"},
            schema::int_property {.Name = "integer"},
        };

        object goodSection0 {};
        goodSection0["string"] = "ok";
        REQUIRE(s0.validate(goodSection0));

        object goodSection1 {};
        goodSection1["float"] = 2.f;
        REQUIRE(s0.validate(goodSection1));

        object badSection0 {};
        badSection0["float"]   = 2.f;
        badSection0["integer"] = 2;
        REQUIRE_FALSE(s0.validate(badSection0));

        object badSection1 {};
        badSection1["float"] = "ok";
        REQUIRE_FALSE(s0.validate(badSection1));

        object badSection2 {};
        badSection2["bla"] = 2.f;
        REQUIRE_FALSE(s0.validate(badSection2));
    }

    SUBCASE("NoneOf")
    {
        schema s0;
        s0.NoneOf = {
            schema::string_property {.Name = "string"},
            schema::float_property {.Name = "float"},
        };

        object obj {};
        obj["string"] = 12;
        REQUIRE(s0.validate(obj));

        obj["string"] = "ok";
        REQUIRE_FALSE(s0.validate(obj));

        obj["string"] = nullptr;
        REQUIRE(s0.validate(obj));

        obj["float"] = 2;
        REQUIRE_FALSE(s0.validate(obj));
    }

    SUBCASE("string_property")
    {
        {
            schema s0;
            s0.AllOf = {schema::string_property {
                .Name      = "string",
                .MinLength = 3,
                .MaxLength = 5,
            }};

            object obj {};
            obj["string"] = "abc";
            REQUIRE(s0.validate(obj));
            obj["string"] = "abcd";
            REQUIRE(s0.validate(obj));
            obj["string"] = "abcde";
            REQUIRE(s0.validate(obj));

            obj["string"] = "ab";
            REQUIRE_FALSE(s0.validate(obj));
            obj["string"] = "abcdef";
            REQUIRE_FALSE(s0.validate(obj));
            obj["string"] = 123;
            REQUIRE_FALSE(s0.validate(obj));
        }
        {
            schema s0;
            s0.AllOf = {schema::string_property {
                .Name    = "string",
                .Pattern = "a*e",
            }};

            object obj {};
            obj["string"] = "abe";
            REQUIRE(s0.validate(obj));
            obj["string"] = "abcde";
            REQUIRE(s0.validate(obj));

            obj["string"] = "ab";
            REQUIRE_FALSE(s0.validate(obj));
            obj["string"] = "abcdef";
            REQUIRE_FALSE(s0.validate(obj));
        }
    }

    SUBCASE("float_property")
    {
        schema s0;
        s0.AllOf = {schema::float_property {
            .Name     = "float",
            .MinValue = 3.f,
            .MaxValue = 5.f,
        }};

        object obj {};
        obj["float"] = 3.f;
        REQUIRE(s0.validate(obj));
        obj["float"] = 4.f;
        REQUIRE(s0.validate(obj));
        obj["float"] = 5.f;
        REQUIRE(s0.validate(obj));

        obj["float"] = 1.f;
        REQUIRE_FALSE(s0.validate(obj));
        obj["float"] = 2.f;
        REQUIRE_FALSE(s0.validate(obj));
        obj["float"] = 5.1f;
        REQUIRE_FALSE(s0.validate(obj));
    }

    SUBCASE("int_property")
    {
        schema s0;
        s0.AllOf = {schema::int_property {
            .Name     = "int",
            .MinValue = 3,
            .MaxValue = 5,
        }};

        object obj {};
        obj["int"] = 3;
        REQUIRE(s0.validate(obj));
        obj["int"] = 4;
        REQUIRE(s0.validate(obj));
        obj["int"] = 5;
        REQUIRE(s0.validate(obj));

        obj["int"] = 1;
        REQUIRE_FALSE(s0.validate(obj));
        obj["int"] = 2;
        REQUIRE_FALSE(s0.validate(obj));
        obj["int"] = 6;
        REQUIRE_FALSE(s0.validate(obj));
    }

    SUBCASE("array_property")
    {
        schema s0;
        s0.AllOf = {schema::array_property {
            .Name     = "array",
            .MinSize  = 3,
            .MaxSize  = 5,
            .ItemType = type::Integer,
        }};

        array arr;
        arr.add(1);
        arr.add(2);
        arr.add(3);
        arr.add(4);

        object obj {};
        obj["array"] = arr;
        REQUIRE(s0.validate(obj));

        arr.add(3);
        arr.add(4);
        REQUIRE_FALSE(s0.validate(obj));

        for (i32 i {0}; i < 3; ++i) {
            arr.pop_back();
        }
        REQUIRE(s0.validate(obj));

        arr.add("X");
        REQUIRE_FALSE(s0.validate(obj));
    }

    SUBCASE("object_property")
    {
        auto c0 {std::make_shared<schema>()};
        c0->AllOf = {schema::string_property {.Name = "string"}, schema::int_property {.Name = "int"}};

        schema s0;
        s0.AllOf = {schema::object_property {
            .Name   = "object",
            .Schema = c0,
        }};

        object csec {};
        csec["string"] = "ok";
        csec["int"]    = 42;

        object obj {};
        obj["object"] = csec;
        REQUIRE(s0.validate(obj));

        csec["string"] = nullptr;
        REQUIRE_FALSE(s0.validate(obj));
    }

    SUBCASE("Failures")
    {
        SUBCASE("AllOf")
        {
            schema s0;
            s0.AllOf = {schema::string_property {.Name = "string"}};

            object obj {};
            obj["float"] = 2.f;

            auto res {s0.validate(obj)};
            REQUIRE_FALSE(res);

            REQUIRE(res.Failures.size() == 1);
            REQUIRE(res.Failures[0].Constraint == "Name");
            REQUIRE(res.Failures[0].Name == "string");
            REQUIRE(res.Failures[0].Group == "AllOf");
        }
        SUBCASE("OneOf")
        {
            schema s0;
            s0.OneOf = {schema::string_property {.Name = "string"},
                        schema::float_property {.Name = "float"}};

            SUBCASE("more than one")
            {
                object obj {};
                obj["string"] = "ok";
                obj["float"]  = 2.f;

                auto res {s0.validate(obj)};
                REQUIRE_FALSE(res);

                REQUIRE(res.Failures.size() == 1);
                REQUIRE(res.Failures[0].Constraint == "Group");
                REQUIRE(res.Failures[0].Name == "float");
                REQUIRE(res.Failures[0].Group == "OneOf");
            }
            SUBCASE("none")
            {
                object obj {};
                obj["x"] = "ok";
                obj["y"] = 2.f;

                auto res {s0.validate(obj)};
                REQUIRE_FALSE(res);

                REQUIRE(res.Failures.size() == 2);
                REQUIRE(res.Failures[0].Constraint == "Name");
                REQUIRE(res.Failures[0].Name == "string");
                REQUIRE(res.Failures[0].Group == "OneOf");
                REQUIRE(res.Failures[1].Constraint == "Name");
                REQUIRE(res.Failures[1].Name == "float");
                REQUIRE(res.Failures[1].Group == "OneOf");
            }
        }
    }
    SUBCASE("FromSection")
    {
        std::shared_ptr<schema> s0;

        {
            object ssec;
            ssec["Properties"]["x"]["Type"]                   = "String";
            ssec["Properties"]["y"]["Type"]                   = "Float";
            ssec["Properties"]["sub"]["Type"]                 = "Object";
            ssec["Properties"]["sub"]["Schema"]               = "sub";
            ssec["AllOf"][0]                                  = "x";
            ssec["AllOf"][1]                                  = "y";
            ssec["AllOf"][2]                                  = "sub";
            ssec["Schemas"]["sub"]["Properties"]["a"]["Type"] = "String";
            ssec["Schemas"]["sub"]["Properties"]["b"]["Type"] = "Float";
            ssec["Schemas"]["sub"]["AllOf"][0]                = "a";
            ssec["Schemas"]["sub"]["AllOf"][1]                = "b";

            s0 = schema::FromObject(ssec);

            REQUIRE(s0);
            REQUIRE(s0->AllOf.size() == 3);
        }

        object sec0 {};
        sec0["x"]        = "ok";
        sec0["y"]        = 2.f;
        sec0["sub"]["a"] = "ok";
        sec0["sub"]["b"] = 2.f;
        REQUIRE(s0->validate(sec0));

        object sec1 {};
        sec1["x"] = "ok";
        sec1["y"] = 2.f;
        REQUIRE_FALSE(s0->validate(sec1));
    }
}

enum class TestEnum0 : u8 {
    True         = 0,
    False        = 1,
    FileNotFound = 2
};

enum class TestEnum1 : u8 {
    True         = 0,
    False        = 1,
    FileNotFound = 2
};

TEST_CASE("Data.Config.Enum")
{
    SUBCASE("FromString translation")
    {
        object t;
        t["valueEnum0"] = "True";
        t["valueEnum1"] = "False";
        t["valueEnum2"] = "FileNotFound";

        REQUIRE(t["valueEnum0"].is<TestEnum1>());
        REQUIRE(t["valueEnum1"].is<TestEnum1>());
        REQUIRE(t["valueEnum2"].is<TestEnum1>());

        REQUIRE(t["valueEnum0"].as<TestEnum1>() == TestEnum1::True);
        REQUIRE(t["valueEnum1"].as<TestEnum1>() == TestEnum1::False);
        REQUIRE(t["valueEnum2"].as<TestEnum1>() == TestEnum1::FileNotFound);
    }
    SUBCASE("ToString translation")
    {
        object t {};
        t["valueEnum0"] = TestEnum1::True;
        t["valueEnum1"] = TestEnum1::False;
        t["valueEnum2"] = TestEnum1::FileNotFound;

        REQUIRE(t["valueEnum0"].is<TestEnum1>());
        REQUIRE(t["valueEnum1"].is<TestEnum1>());
        REQUIRE(t["valueEnum2"].is<TestEnum1>());

        REQUIRE(t["valueEnum0"].as<TestEnum1>() == TestEnum1::True);
        REQUIRE(t["valueEnum1"].as<TestEnum1>() == TestEnum1::False);
        REQUIRE(t["valueEnum2"].as<TestEnum1>() == TestEnum1::FileNotFound);

        REQUIRE(t["valueEnum0"].as<std::string>() == "True");
        REQUIRE(t["valueEnum1"].as<std::string>() == "False");
        REQUIRE(t["valueEnum2"].as<std::string>() == "FileNotFound");
    }
}
