#include "WrapperTestsClass.hpp"
#include "tests.hpp"

#include <algorithm>
#include <cstring>
#include <numeric>

using namespace tcob::scripting;
using namespace tcob::scripting::squirrel;

auto static testfuncstr() -> std::string
{
    return "huhu";
}

auto static testfuncfloat() -> float
{
    return 4.2f;
}
auto static testfuncpair(std::pair<i32, f32> const& p) -> f32
{
    return static_cast<f32>(p.first) * p.second;
}
auto static testfuncfloat2(std::expected<f32, error_code> f, std::expected<f32, error_code> x, int i) -> f32
{
    return f.value() * x.value() * static_cast<f32>(i);
}

struct foo {
    i32 x = 0;
    i32 y = 0;
    i32 z = 0;
};

namespace tcob::scripting::squirrel {
template <>
struct converter<foo> {
    auto static IsType(vm_view ls, SQInteger idx) -> bool
    {
        table lt {table::Acquire(ls, idx)};
        return lt.has("x") && lt.has("y") && lt.has("z");
    }

    auto static From(vm_view ls, SQInteger& idx, foo& value) -> bool
    {
        if (ls.is_table(idx)) {
            table lt {table::Acquire(ls, idx++)};

            value.x = lt["x"].as<i32>();
            value.y = lt["y"].as<i32>();
            value.z = lt["z"].as<i32>();
        }
        return true;
    }

    void static To(vm_view ls, foo const& value)
    {
        ls.new_table();
        table lt {table::Acquire(ls, -1)};

        lt["x"] = value.x;
        lt["y"] = value.y;
        lt["z"] = value.z;
    }
};
}

class SquirrelScriptTests : public squirrel::script {
public:
    SquirrelScriptTests()
        : global(root_table())
    {
        open_libraries();
    }

    table global;
};

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Array")
{
    {
        array arr = *run<array>("return [4,5,2,1] ");
        REQUIRE(arr.size() == 4);
        i32 x = arr[0].as<i32>();
        REQUIRE(x == 4);
        x = arr[1].as<i32>();
        REQUIRE(x == 5);
        x = arr[2].as<i32>();
        REQUIRE(x == 2);
        x = arr[3].as<i32>();
        REQUIRE(x == 1);
    }
    {
        auto res = run("arr <- [4,5,2,1] ");
        REQUIRE(res);
        {
            array arr1 = global["arr"].as<array>();
            arr1[1]    = 100;
            REQUIRE(arr1[1].as<i32>() == 100);
        }
        {
            array arr1 = global["arr"].as<array>();
            arr1[1]    = 100;
            REQUIRE(arr1[1].as<i32>() == 100);
        }
    }
    {
        auto res = run("arr1 <- [4,5,2,1]; ");
        REQUIRE(res);
        array arr1 = global["arr1"].as<array>();
        arr1[3]    = 999;
        array arr2 = global["arr1"].as<array>();
        REQUIRE(arr2[3].as<i32>() == 999);
    }
    {
        auto res = run("arr1 <- [4,5,2,1]; arr2 <- [1,2,3,4] ");
        REQUIRE(res);
        array arr1a = global["arr1"].as<array>();
        array arr1b = global["arr1"].as<array>();
        array arr2  = global["arr2"].as<array>();
        REQUIRE(arr1a == arr1b);
        REQUIRE(arr1a != arr2);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.ClassesAndInstances")
{
    SUBCASE("fields")
    {
        clazz c {clazz::Create(get_view())};
        c["value"]    = 100;
        global["foo"] = c;

        auto res = run<i32>("inst <- foo(); return inst.value");
        REQUIRE(res);
        REQUIRE(res.value() == 100);

        auto i = global["inst"].as<instance>();
        REQUIRE(i["value"].as<i32>() == 100);
        i["value"] = 420;

        res = run<i32>("return inst.value");
        REQUIRE(res);
        REQUIRE(res.value() == 420);

        res = run<i32>("inst2 <- foo(); return inst2.value");
        REQUIRE(res);
        REQUIRE(res.value() == 100);
    }
    SUBCASE("functions")
    {
        std::function func = [](f32 x, f32 y) {
            return x * y;
        };

        clazz c {clazz::Create(get_view())};
        c["func"]     = &func;
        global["foo"] = c;

        auto res = run<f32>("inst <- foo(); return inst.func(2.5, 4.1)");
        REQUIRE(res);
        REQUIRE(res.value() == 2.5f * 4.1f);
    }
    SUBCASE("create instance")
    {
        clazz c {clazz::Create(get_view())};
        c["value"]     = 100;
        global["inst"] = c.create_instance();

        auto res = run<i32>("return inst.value");
        REQUIRE(res);
        REQUIRE(res.value() == 100);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Closures")
{
    std::function testFuncPrim = [](i32 i, f32 f, f64 d, bool b) {
        return std::to_string(i) + std::to_string(f) + std::to_string(d) + std::string(b ? "true" : "false");
    };

    i32           voidTest     = 0;
    std::function testFuncVoid = [&voidTest] {
        voidTest++;
    };

    global["test"]["Prim"] = &testFuncPrim;
    global["test"]["Void"] = &testFuncVoid;

    {
        auto res = run("str <- test.Prim(20, 4.4, 5.22, true)");
        REQUIRE(res);
        auto str = global["str"].as<std::string>();
        REQUIRE(str == testFuncPrim(20, 4.4f, 5.22, true));
    }
    {
        auto res = run("test.Void()");
        REQUIRE(res);
        REQUIRE(voidTest == 1);
        res = run("test.Void()");
        REQUIRE(res);
        REQUIRE(voidTest == 2);
    }
    {
        global["testFunc"] = testfuncstr;
        std::string x      = *run<std::string>("return testFunc()");
        REQUIRE(x == testfuncstr());
    }
    {
        global["testFunc"] = testfuncfloat;
        f32 x              = *run<f32>("return testFunc()");
        REQUIRE(x == testfuncfloat());
    }
    {
        global["testFunc"] = testfuncfloat2;
        f32 x              = *run<f32>("return testFunc(4,4.5,3)");
        REQUIRE(x == testfuncfloat2(std::expected<f32, error_code> {4.f}, std::expected<f32, error_code> {4.5f}, 3));
    }
    {
        global["testFunc"] = testfuncpair;
        f32 x              = *run<f32>("return testFunc([4, 6.5])");
        REQUIRE(x == testfuncpair({4, 6.5f}));
    }
    {
        auto l             = std::function([](i32 i) { return (f32)i * 2.5f; });
        global["testFunc"] = &l;
        f32 x              = *run<f32>("return testFunc(2)");
        REQUIRE(x == 5.0f);
    }
    {
        global["foo"]             = table::PushNew(get_view());
        auto l                    = std::function([](i32 i) { return (f32)i * 2.5f; });
        global["foo"]["testFunc"] = &l;
        f32 x                     = *run<f32>("return foo.testFunc(2)");
        REQUIRE(x == 5.0f);
    }
    {
        f32  x {0};
        auto l             = std::function([&x](i32 i) { x = static_cast<f32>(i) * 2.5f; });
        global["testFunc"] = &l;
        auto res           = run("testFunc(2)");
        REQUIRE(res);
        REQUIRE(x == 5.0f);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Container")
{
    std::function testFuncVector = [] {
        return std::vector<std::string> {"1", "2", "3", "4", "5"};
    };

    std::function testFuncArray = [] {
        return std::array<std::string, 5> {"1", "2", "3", "4", "5"};
    };

    std::function testFuncPairPara = [](std::pair<std::string, i32> const& pair) {
        return pair.second;
    };
    /*
        std::function testFuncTuple = [](double d) {
            return std::tuple(d * 5, std::to_string(d));
        };
    */
    std::function testFuncMap = [] {
        return std::map<std::string, i32> {
            {"abc", 123},
            {"def", 234}};
    };
    std::function testFuncUMap = [] {
        return std::unordered_map<std::string, i32> {
            {"abc", 123}, {"def", 234}};
    };

    // global["test"]["Tuple"]    = testFuncTuple;
    global["test"]["Map"]      = &testFuncMap;
    global["test"]["UMap"]     = &testFuncUMap;
    global["test"]["Vector"]   = &testFuncVector;
    global["test"]["Array"]    = &testFuncArray;
    global["test"]["PairPara"] = &testFuncPairPara;

    /*
        SUBCASE("tuple return from cpp")
        {
            auto res = run("a <- test.Tuple(5.22)");
            REQUIRE(res);
            double      a = global["a"];
            std::string b = global["b"];
            REQUIRE(a == 5.22 * 5);
            REQUIRE(b == std::to_string(5.22));
        }
        */
    SUBCASE("return: std::map<string, int>")
    {
        auto res = run("x <- test.Map()");
        REQUIRE(res);
        auto x = global["x"].as<std::map<std::string, i32>>();
        REQUIRE(x["abc"] == 123);
        REQUIRE(x["def"] == 234);
    }
    SUBCASE("return: std::unordered_map<string, int>")
    {
        auto res = run("x <- test.UMap()");
        REQUIRE(res);
        auto x = global["x"].as<std::unordered_map<std::string, i32>>();
        REQUIRE(x["abc"] == 123);
        REQUIRE(x["def"] == 234);
    }
    SUBCASE("return: std::vector<string>")
    {
        auto res = run("x <- test.Vector();");
        REQUIRE(res);
        auto vec = global["x"].as<std::vector<std::string>>();
        REQUIRE(vec[0] == "1");
        REQUIRE(vec[4] == "5");
    }
    SUBCASE("return: std::array<string, N>")
    {
        auto res = run("x <- test.Array()");
        REQUIRE(res);
        auto vec = global["x"].as<std::array<std::string, 5>>();
        REQUIRE(vec[0] == "1");
        REQUIRE(vec[4] == "5");
    }
    SUBCASE("param: vector and array")
    {
        auto res = run("function foo(x) {return x[1] * x[3]} ");
        REQUIRE(res);
        std::vector vec {1, 2, 3, 4, 5};
        auto        func = global["foo"].as<function<i32>>();
        i32         a    = func(vec);
        REQUIRE(a == 2 * 4);

        std::array arr {1, 2, 3, 4, 5};
        a = func(arr);
        REQUIRE(a == 2 * 4);
    }
    SUBCASE("param: std::pair<int, float>")
    {
        auto res = run("function foo(x) {return x[0] * x[1]} ");
        REQUIRE(res);
        auto tup  = std::make_pair(4, 2.4f);
        auto func = global["foo"].as<function<f32>>();
        f32  a    = func(tup);
        REQUIRE(a == 4 * 2.4f);
    }
    SUBCASE("param: std::map and std::unordered_map")
    {
        auto res = run("function foo(x) {return x.test} ");
        REQUIRE(res);
        auto map  = std::map<std::string, i32> {{"test", 123}};
        auto func = global["foo"].as<function<i32>>();
        i32  a    = func(map);
        REQUIRE(a == 123);

        std::unordered_map<std::string, i32> umap = {{"test", 245}};
        a                                         = func(umap);
        REQUIRE(a == 245);
    }
    SUBCASE("interop: get/set std::vector by index")
    {
        std::vector<std::string> vec = {"test", "123"};
        global["foo"]                = vec;
        std::string a                = *run<std::string>("return foo[0] ");
        REQUIRE(a == "test");
        std::string b = *run<std::string>("return foo[1] ");
        REQUIRE(b == "123");
    }
    SUBCASE("interop: get/set std::deque by index")
    {
        std::deque<std::string> deck = {"test", "123"};
        global["foo"]                = deck;
        std::string a                = *run<std::string>("return foo[0] ");
        REQUIRE(a == "test");
        std::string b = *run<std::string>("return foo[1] ");
        REQUIRE(b == "123");
    }
    SUBCASE("interop: get/set std::span by index")
    {
        std::vector<std::string> vec {"test", "123"};
        std::span<std::string>   s {vec.data(), vec.size()};
        global["foo"] = s;
        std::string a = *run<std::string>("return foo[0] ");
        REQUIRE(a == vec[0]);
        std::string b = *run<std::string>("return foo[1] ");
        REQUIRE(b == vec[1]);
    }
    SUBCASE("interop: convert Sq table to std::map")
    {
        auto res = run("rectF <- {x=2.7, y=3.1, width=2.3, height=55.2} ");
        REQUIRE(res);
        auto rectF = global["rectF"].as<std::map<std::string, f32>>();
        REQUIRE(rectF["x"] == 2.7f);
    }
    SUBCASE("interop: get/set std::map<string, int>")
    {
        std::map<std::string, i32> map = {{"test", 123}};
        global["foo"]                  = map;
        i32 a                          = *run<i32>("return foo.test ");
        REQUIRE(a == 123);
    }
    SUBCASE("return: multiple values as std::pair")
    {
        std::pair<std::string, i32> x = *run<std::pair<std::string, i32>>("return [\"ok\", 10]");
        REQUIRE(x.first == "ok");
        REQUIRE(x.second == 10);
    }
    SUBCASE("param: std::pair<string, int>")
    {
        auto func = global["test"]["PairPara"].as<function<i32>>();
        i32  a    = func(std::pair {"ok"s, 4});
        REQUIRE(a == 4);
    }
    SUBCASE("interop: get/set std::set<string>")
    {
        std::set<std::string> set1 {"test", "test2"};
        global["foo"]              = set1;
        std::set<std::string> set2 = *run<std::set<std::string>>("return foo ");
        REQUIRE(set1 == set2);
    }
    SUBCASE("return: std::set<int> from Sq table")
    {
        std::set<i32> set = *run<std::set<i32>>("return [1, 2, 3, 1, 2, 3, 4, 2] ");
        REQUIRE(set == std::set<i32> {1, 2, 3, 4});
    }
    SUBCASE("interop: get/set std::unordered_set<string>")
    {
        std::unordered_set<std::string> set1 {"test", "test2"};
        global["foo"]                        = set1;
        std::unordered_set<std::string> set2 = *run<std::unordered_set<std::string>>("return foo ");
        REQUIRE(set1 == set2);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Enums")
{
    enum class testEnum : u8 {
        True,
        False,
        FileNotFound
    };
    std::function testFuncEnum = [](testEnum numnum) {
        return numnum;
    };

    global["test"]["Enum"] = &testFuncEnum;
    {
        auto     func = global["test"]["Enum"].as<function<testEnum>>();
        testEnum num  = *func.call(testEnum::FileNotFound);
        REQUIRE(num == testEnum::FileNotFound);
    }
    {
        std::unordered_map<std::string, testEnum> map = {
            {"True", testEnum::True},
            {"False", testEnum::False},
            {"FileNotFound", testEnum::FileNotFound},
        };

        global["testEnum"] = map;

        testEnum num = *run<testEnum>(
            "ex <- testEnum.False; "
            "return test.Enum(ex); ");
        REQUIRE(num == testEnum::False);
        num = *run<testEnum>(
            "ex <- testEnum.True; "
            "return test.Enum(ex); ");
        REQUIRE(num == testEnum::True);
        num = *run<testEnum>(
            "ex <- testEnum.FileNotFound; "
            "return test.Enum(ex); ");
        REQUIRE(num == testEnum::FileNotFound);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Functions")
{
    SUBCASE("cpp -> sq -> cpp")
    {
        auto res = run(
            "function foo0(a) { return cppFunc0(a) }  "
            "function foo1(b) { return 3 * b }        ");
        REQUIRE(res);
        auto foo0 = global["foo0"].as<function<i32>>();
        auto foo1 = global["foo1"].as<function<i32>>();

        auto cppFunc0      = std::function([&](i32 i) {
            return foo1(20 * i);
        });
        global["cppFunc0"] = &cppFunc0;

        i32 result = foo0(10);
        REQUIRE(result == 600);
    }
    SUBCASE("sq -> cpp -> sq")
    {
        auto res = run("function testPoint(p) { return p.x * p.y }");
        REQUIRE(res);
        auto func = global["testPoint"].as<function<i32>>();
        i32  a    = *func.call(point_i {2, 4});
        REQUIRE(a == 2 * 4);
        a = func(point_i {2, 4});
        REQUIRE(a == 2 * 4);
    }
    SUBCASE("nullptr as parameter")
    {
        auto res = run(
            "function foo(a, b, c) {"
            "  if (!a) return 1; "
            "  if (!b) return 2; "
            "  if (!c) return 3; "
            "  return 0 "
            "} ");
        REQUIRE(res);
        auto func = global["foo"].as<function<i32>>();

        i32 a = func(1, 2, 3);
        REQUIRE(a == 0);
        a = func(nullptr, 2, 3);
        REQUIRE(a == 1);
        a = func(1, nullptr, 3);
        REQUIRE(a == 2);
        a = func(1, 2, nullptr);
        REQUIRE(a == 3);
    }
    SUBCASE("optional parameter")
    {
        {
            auto f = *run<function<bool>>("return function(i) { return i == null }");
            REQUIRE(f(std::nullopt) == true);
            std::optional<i32> opt;
            REQUIRE(f(opt) == true);
            opt = 10;
            REQUIRE(f(opt) == false);
        }
        {
            auto               f = *run<function<i32>>("return function(a,b,c) { return b == null ? a + c : a + b + c }");
            std::optional<i32> opt;
            REQUIRE(f(1, opt, 5) == 6);
            opt = 10;
            REQUIRE(f(1, opt, 5) == 16);
        }
    }
    SUBCASE("parameter_pack")
    {
        {
            auto res = run(
                "function foo(a, b, c) { "
                "  return a+b+c "
                "} ");
            REQUIRE(res);
            auto func = global["foo"].as<function<i32>>();

            parameter_pack<i32> pack;
            pack.Items = {1, 2, 3};
            i32 a      = func(pack);
            REQUIRE(a == 6);
        }
        {
            auto res = run(
                "function foo(a, b, c) { "
                "  return b ? a+c : a*c "
                "} ");
            REQUIRE(res);
            auto func = global["foo"].as<function<i32>>();

            parameter_pack<i32, bool> pack;
            pack.Items = {2, true, 3};
            i32 a      = func(pack);
            REQUIRE(a == 5);
            pack.Items = {2, false, 4};
            a          = func(pack);
            REQUIRE(a == 8);
        }
    }
    SUBCASE("recurse")
    {
        auto res = run("function recurse(n) { if (n == 0) { return 1 } return cppFunc(n - 1) }");
        REQUIRE(res);
        auto func         = std::function<i32(i32)>([&](i32 n) { return global["recurse"].as<function<i32>>()(n); });
        global["cppFunc"] = &func;
        auto result       = global["recurse"].as<function<i32>>()(10);
        REQUIRE(result == 1);
    }
    SUBCASE("closure capture")
    {
        auto res = run("function make_adder(x) {return function(y) {return x + y}} ");
        REQUIRE(res);
        auto f = global["make_adder"].as<function<function<i32>>>()(5);
        REQUIRE(f(3) == 8);
    }
    SUBCASE("misc")
    {
        {
            function<i32> func = *run<function<i32>>("return @(x) x*x ");
            i32           a    = func(200);
            REQUIRE(a == 200 * 200);
        }
        {
            auto func = run<function<i32>>("return function() {return 100} ").value();
            REQUIRE(func() == 100);
            REQUIRE(100 == func());
        }
        {
            function<i32> func = *run<function<i32>>("return @() 100 ");
            REQUIRE_FALSE(func() == 10);
            REQUIRE_FALSE(10 == func());
        }
        {
            function<i32> func = *run<function<i32>>("return @() 5 ");
            REQUIRE(func() * 20 == 100);
            REQUIRE(20 * func() == 100);
        }
        {
            function<i32> func = *run<function<i32>>("return @() 500 ");
            REQUIRE(func() / 5 == 100);
            REQUIRE(50000 / func() == 100);
        }
        {
            function<i32> func = *run<function<i32>>("return @() 95 ");
            REQUIRE(func() + 5 == 100);
            REQUIRE(5 + func() == 100);
        }
        {
            function<i32> func = *run<function<i32>>("return @() 105 ");
            REQUIRE(func() - 5 == 100);
            REQUIRE(205 - func() == 100);
        }
        {

            auto func = run<function<std::vector<i32>>>("return @() [5, 4, 3, 2, 1] ").value();
            auto a    = func();
            REQUIRE(a[0] == 5);
            REQUIRE(a[1] == 4);
            REQUIRE(a[2] == 3);
            REQUIRE(a[3] == 2);
            REQUIRE(a[4] == 1);
        }
        {
            auto func = run<function<std::map<std::string, i32>>>("return @() {x=5, y=4, b=3, r=2, aa=1} ").value();
            auto a    = func();
            REQUIRE(a["x"] == 5);
            REQUIRE(a["y"] == 4);
            REQUIRE(a["b"] == 3);
            REQUIRE(a["r"] == 2);
            REQUIRE(a["aa"] == 1);
        }
        {
            auto res = run("function testPoint(p) { return p.x * p.y } ");
            REQUIRE(res);
            auto    func = global["testPoint"].as<function<i32>>();
            point_i p    = point_i {2, 4};
            i32     a    = func(p);
            REQUIRE(a == 2 * 4);
            a = func(point_i {6, 4});
            REQUIRE(a == 6 * 4);
            a = func(point_i {15, 7});
            REQUIRE(a == 15 * 7);
        }
        {
            auto res = run(
                "x <- 0;"
                "function testVoid(p)  {x = p.x * p.y } ");
            REQUIRE(res);
            auto func = global["testVoid"].as<function<void>>();
            func(point_i {2, 4});
            i32 x = global["x"].as<i32>();
            REQUIRE(x == 2 * 4);
        }
        {
            auto res = run("function testMulti(f,p,r,b)  { return f * p.x * r.y } ");
            REQUIRE(res);
            auto func = global["testMulti"].as<function<f32>>();
            f32  x    = func(10.4f, point_i {2, 4}, rect_f {0, 20, 4, 5}, true);
            REQUIRE(x == 10.4f * 2 * 20);
        }
        {
            auto res = run("function testTable(x,y) { return { a = x, b = y } } ");
            REQUIRE(res);
            auto  func = global["testTable"].as<function<table>>();
            table tab  = func(10, 20);
            REQUIRE(tab["a"].as<i32>() == 10);
            REQUIRE(tab["b"].as<i32>() == 20);
        }
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Generators")
{
    {
        auto res = run(R"(
            function geny() {
                for(local i=1;i<10;i+=1) { yield i; }
                return null;
            }

            co <- geny();
        )");
        REQUIRE(res);

        REQUIRE(global.is<generator>("co"));
        auto co = global["co"].as<generator>();
        REQUIRE(co.resume<i32>().value() == 1);
        REQUIRE(co.resume<i32>().value() == 2);
        REQUIRE(co.resume<i32>().value() == 3);
    }
    {
        auto res = run(R"(
            function geny(x) {
                for(local i=1;i<10;i+=1) { yield i*x; }
                return null;
            }

            co <- geny(2);
        )");
        REQUIRE(res);

        auto co = global["co"].as<generator>();
        REQUIRE(co.resume<i32>().value() == 2);
        REQUIRE(co.resume<i32>().value() == 4);
        REQUIRE(co.resume<i32>().value() == 6);
    }
    {
        auto res = run(R"(
            function geny() {
                for(local i=1;i<=2;i+=1) { yield i; }
                return null;
            }

            co <- geny();
        )");
        REQUIRE(res);

        auto co = global["co"].as<generator>();

        using return_type = std::optional<i32>;

        auto result = co.resume<return_type>();
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 1);
        result = co.resume<return_type>();
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 2);
        result = co.resume<return_type>();
        REQUIRE(result.value() == std::nullopt);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.GetSet")
{
    SUBCASE("get/set std::string")
    {
        auto res = run("x <- \"ok\"");
        REQUIRE(res);
        std::string x = *global.get<std::string>("x");
        REQUIRE(x == "ok");

        global["x"] = "ko";
        auto y      = global["x"].as<std::string>();
        REQUIRE(y == "ko");
    }
    SUBCASE("get/set std::string_view")
    {
        using namespace std::literals;
        auto constexpr x {"ABCDEF"sv};
        global["x"] = x.substr(2, 2);
        auto y      = global["x"].as<std::string_view>();
        REQUIRE(y == "CD");
    }
    SUBCASE("get const char*")
    {
        auto res = run("x <- \"ok\"");
        REQUIRE(res);
        char const* x = *global.get<char const*>("x");
        REQUIRE(strcmp(x, "ok") == 0);
    }
    SUBCASE("get/set const char*")
    {
        char const* x {"ok"};
        global["x"]   = x;
        char const* y = global["x"].as<char const*>();
        REQUIRE(strcmp(x, y) == 0);
    }
    SUBCASE("get/set i32")
    {
        auto res = run("x <- 1337");
        REQUIRE(res);
        i32 x = global["x"].as<i32>();
        REQUIRE(x == 1337);
        global["x"] = 2000;
        x           = global["x"].as<i32>();
        REQUIRE(x == 2000);
        x = *run<i32>("return x");
        REQUIRE(x == 2000);
    }
    SUBCASE("delete table entry")
    {
        auto res = run("x <- 1337");
        REQUIRE(res);
        REQUIRE(global.has("x"));
        global["x"] = nullptr;
        REQUIRE_FALSE(global.has("x"));
    }
    SUBCASE("get/set u8")
    {
        auto res = run("x <- 30 ");
        REQUIRE(res);
        u8 x = global["x"].as<u8>();
        REQUIRE(x == 30);
    }
    SUBCASE("get/set nested i32")
    {
        auto res = run("x <- { y = { z = 30 } }");
        REQUIRE(res);
        i32 x = global["x"]["y"]["z"].as<i32>();
        REQUIRE(x == 30);
        global["x"]["y"]["z"] = 2000;
        x                     = global["x"]["y"]["z"].as<i32>();
        REQUIRE(x == 2000);
    }
    SUBCASE("create nested entries w/ subscript")
    {
        auto res = run("x <- { y = {  } }");
        REQUIRE(res);
        REQUIRE_FALSE(global.has("x", "y", "z"));
        global["x"]["y"]["z"] = 2000;
        REQUIRE(global.has("x", "y", "z"));
        i32 x = global["x"]["y"]["z"].as<i32>();
        REQUIRE(x == 2000);
    }
    SUBCASE("create nested entries w/ set")
    {
        auto res = run("x <- { y = {  } }");
        REQUIRE(res);
        REQUIRE_FALSE(global.has("x", "y", "z"));
        global.set("x", "y", "z", 2000);
        REQUIRE(global.has("x", "y", "z"));
        i32 x = global["x"]["y"]["z"].as<i32>();
        REQUIRE(x == 2000);
    }
    SUBCASE("create global var w/ subscript")
    {
        REQUIRE_FALSE(global.has("testVar1"));
        global["testVar1"] = 2000;
        REQUIRE(global.has("testVar1"));
        i32 x = global["testVar1"].as<i32>();
        REQUIRE(x == 2000);
    }
    SUBCASE("create global var w/ set")
    {
        REQUIRE_FALSE(global.has("testVar2"));
        global.set("testVar2", 2000);
        REQUIRE(global.has("testVar2"));
        i32 x = global["testVar2"].as<i32>();
        REQUIRE(x == 2000);
    }
    SUBCASE("try access undefined global")
    {
        REQUIRE_FALSE(global.has("testVar4"));
        auto f = global.get<f32>("testVar4");
        REQUIRE(f.error() == error_code::Undefined);

        global["testVar4"] = 1.5f;
        auto f1            = global.get<f32>("testVar4");
        REQUIRE(f1.value() == 1.5f);
    }
    SUBCASE("access nested table")
    {
        auto res = run("x <- { y = 100, z = { m = 75, n = 5 } }");
        REQUIRE(res);
        i32 m = global["x"]["z"]["m"].as<i32>();
        REQUIRE(m == 75);
    }
    SUBCASE("access created global from function")
    {
        REQUIRE_FALSE(global.has("testVar"));
        global["testVar"] = 400;
        auto res          = run("function foo() {return testVar * 10 }");
        REQUIRE(res);
        auto func = global["foo"].as<function<i32>>();
        i32  a    = func();
        REQUIRE(a == 400 * 10);
        global["testVar"] = 2000;
        a                 = func();
        REQUIRE(a == 2000 * 10);
    }
    SUBCASE("get/set undefined nested i32")
    {
        auto res = run("x <- { y = { } }");
        REQUIRE(res);
        REQUIRE_FALSE(global.has("x", "y", "z"));
        global["x"]["y"]["z"] = 2000;
        i32 x                 = global["x"]["y"]["z"].as<i32>();
        REQUIRE(x == 2000);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Hook")
{
    SUBCASE("locals in hook")
    {
        i32         x {0};
        i32         y {0};
        std::string z;

        auto func = [&](debug_event type, string const& sourcename, i64, string const& funcname) {
            if (sourcename == "TEST" && funcname == "foo" && type == debug_event::Return) {
                auto const view {get_view()};
                auto const guard {view.create_stack_guard()};

                for (u32 i {0};; ++i) {
                    auto val {view.get_local(0, i)};
                    if (val.empty()) { break; }

                    if (val == "x") {
                        view.pull_convert_idx(-1, x);
                    } else if (val == "y") {
                        view.pull_convert_idx(-1, y);
                    } else if (val == "z") {
                        view.pull_convert_idx(-1, z);
                    }
                }
            }
        };

        set_hook(func);
        auto res = run(R"-(
            function foo() {
                local x = 100
                local y = 400
                local z = "ok"
                return 45
            }
            foo()
        )-",
                       "TEST");

        REQUIRE(res);
        REQUIRE(x == 100);
        REQUIRE(y == 400);
        REQUIRE(z == "ok");
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.IsHas")
{
    SUBCASE("is")
    {
        auto res = run(
            "a <- 100;"
            "b <- true;"
            "c <- 10.3;"
            "d <- \"hello\";"
            "e <- { };"
            "f <- function(x) { return x };"
            "g <- { a = 1, b = 2, c = 3 };"
            "h <- [ 1, 2, 3 ];");

        REQUIRE(res);

        REQUIRE(global.is<i32>("a"));
        REQUIRE(global.is<bool>("b"));
        REQUIRE(global.is<f32>("c"));
        REQUIRE(global.is<std::string>("d"));
        REQUIRE(global.is<table>("e"));
        REQUIRE(global.is<function<void>>("f"));
        REQUIRE((global.is<std::map<std::string, i32>>("g")));
        REQUIRE((global.is<std::vector<i32>>("h")));

        REQUIRE_FALSE(global.is<bool>("a"));
        REQUIRE_FALSE(global.is<std::string>("a"));
        REQUIRE_FALSE(global.is<table>("a"));
        REQUIRE_FALSE(global.is<function<void>>("a"));
        REQUIRE_FALSE((global.is<std::vector<std::string>>("h")));
        REQUIRE_FALSE((global.is<std::map<i32, i32>>("g")));

        REQUIRE_FALSE(global.is<bool>("c"));
        REQUIRE_FALSE(global.is<i32>("c"));

        REQUIRE_FALSE(global.is<bool>("d"));
        REQUIRE_FALSE(global.is<table>("d"));
        REQUIRE_FALSE(global.is<function<void>>("d"));
    }
    SUBCASE("has")
    {
        auto res = run("x <- { y = 100, z = { m = 75, n = 5 } }");
        REQUIRE(res);
        REQUIRE(global.has("x"));
        REQUIRE(global.has("x", "y"));
        REQUIRE(global.has("x", "z"));
        REQUIRE(global.has("x", "z", "m"));

        REQUIRE_FALSE(global.has("y"));
        REQUIRE_FALSE(global.has("x", "a"));
        REQUIRE_FALSE(global.has("x", "y", "z"));
        REQUIRE_FALSE(global.has("x", "z", "m", "a"));
    }
}

TEST_CASE("Script.Squirrel.Literals")
{
    using namespace tcob::literals;
    {
        auto script = "x <- 100"_squirrel;
        i32  x      = script->root_table()["x"].as<i32>();
        REQUIRE(x == 100);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Map")
{
    SUBCASE("to")
    {
        {
            std::map<std::string, rect_f> m = {{"a", {0, 1, 2, 3}}, {"b", {4, 3, 2, 1}}};
            global["map"]                   = m;
            rect_f x                        = *run<rect_f>("return map.a");
            REQUIRE(x == m["a"]);
            rect_f y = *run<rect_f>("return map.b");
            REQUIRE(y == m["b"]);
        }
        {
            std::map<i32, rect_f> m = {{1, {0, 1, 2, 3}}, {2, {4, 3, 2, 1}}};
            global["map"]           = m;
            rect_f x                = *run<rect_f>("return map[1]");
            REQUIRE(x == m[1]);
            rect_f y = *run<rect_f>("return map[2]");
            REQUIRE(y == m[2]);
        }
        {
            std::unordered_map<std::string, rect_f> m = {{"a", {0, 1, 2, 3}}, {"b", {4, 3, 2, 1}}};
            global["map"]                             = m;
            rect_f x                                  = *run<rect_f>("return map.a");
            REQUIRE(x == m["a"]);
            rect_f y = *run<rect_f>("return map.b");
            REQUIRE(y == m["b"]);
        }
        {
            std::unordered_map<i32, rect_f> m = {{1, {0, 1, 2, 3}}, {2, {4, 3, 2, 1}}};
            global["map"]                     = m;
            rect_f x                          = *run<rect_f>("return map[1]");
            REQUIRE(x == m[1]);
            rect_f y = *run<rect_f>("return map[2]");
            REQUIRE(y == m[2]);
        }
    }
    SUBCASE("from")
    {
        {
            auto m = *run<std::map<std::string, rect_f>>("return {a={x=0,y=1,width=2,height=3},b={x=4,y=3,width=2,height=1}}");
            REQUIRE(m["a"] == (rect_f {0, 1, 2, 3}));
            REQUIRE(m["b"] == (rect_f {4, 3, 2, 1}));
        }
        {
            auto m = *run<std::map<i32, rect_f>>("return [{x=0,y=1,width=2,height=3},{x=4,y=3,width=2,height=1}]");
            REQUIRE(m[0] == (rect_f {0, 1, 2, 3}));
            REQUIRE(m[1] == (rect_f {4, 3, 2, 1}));
        }
        {
            auto m = *run<std::unordered_map<std::string, rect_f>>("return {a={x=0,y=1,width=2,height=3},b={x=4,y=3,width=2,height=1}}");
            REQUIRE(m["a"] == (rect_f {0, 1, 2, 3}));
            REQUIRE(m["b"] == (rect_f {4, 3, 2, 1}));
        }
        {
            auto m = *run<std::unordered_map<i32, rect_f>>("return [{x=0,y=1,width=2,height=3},{x=4,y=3,width=2,height=1}]");
            REQUIRE(m[0] == (rect_f {0, 1, 2, 3}));
            REQUIRE(m[1] == (rect_f {4, 3, 2, 1}));
        }
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Optional")
{
    std::function Optional = [](i32 i, std::optional<std::string> str) -> i32 {
        if (str.has_value()) {
            return static_cast<i32>(str.value().length());
        }

        return i;
    };
    std::function Optional2 = [](i32 i, std::optional<std::string> str, f32 f) -> f32 {
        if (str.has_value()) {
            return static_cast<f32>(str.value().length()) * f;
        }

        return static_cast<f32>(i) * f;
    };

    global["test"]["Optional"]  = &Optional;
    global["test"]["Optional2"] = &Optional2;

    {
        i32 i = *run<i32>("return test.Optional(100)");
        REQUIRE(i == Optional(100, std::nullopt));
    }
    {
        i32 i = *run<i32>("return test.Optional(100, \"hurray\")");
        REQUIRE(i == 6);
    }
    {
        f32 f = *run<f32>("return test.Optional2(100, 2.5)");
        REQUIRE(f == Optional2(100, std::nullopt, 2.5f));
    }
    {
        f32 f = *run<f32>("return test.Optional2(100,\"hurray\", 2.5)");
        REQUIRE(f == Optional2(100, "hurray", 2.5f));
    }
    {
        auto f = *run<std::optional<f32>>("return 10.25");
        REQUIRE(f.has_value());
        REQUIRE(f == 10.25f);
    }
    {
        auto f = run<std::optional<f32>>("return \"ok\"");
        REQUIRE(f.has_value());
        REQUIRE_FALSE(f.value().has_value());
    }
    {
        auto f = run<std::optional<f32>>("return");
        REQUIRE(f.has_value());
        REQUIRE_FALSE(f.value().has_value());
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Overloads")
{
    SUBCASE("Lambdas")
    {
        auto const overload = make_unique_overload(
            [] { return 2.5f; },
            [](f32 i) { return i * 2.5f; },
            [](f32 i0, f32 i1) { return i0 * i1 * 2.5f; },
            [](std::array<f32, 5> arr) { return std::accumulate(arr.begin(), arr.end(), 1.f, std::multiplies {}) * 2.5f; });
        global["overload"] = overload.get();

        auto res = run<f32>("return overload([1, 2, 3, 4, 5])");
        REQUIRE(res);
        REQUIRE(res.value() == 300.0f);

        res = run<f32>("return overload()");
        REQUIRE(res);
        REQUIRE(res.value() == 2.5f);

        res = run<f32>("return overload(2)");
        REQUIRE(res);
        REQUIRE(res.value() == 5.0f);

        res = run<f32>("return overload(2, 3)");
        REQUIRE(res);
        REQUIRE(res.value() == 15.0f);
    }
    SUBCASE("Member functions")
    {
        TestScriptClass t;
        auto            f1 = resolve_overload<f32(i32, f32)>(&TestScriptClass::overload);
        auto            f2 = resolve_overload<f32(f32, i32)>(&TestScriptClass::overload);
        auto            f3 = resolve_overload<f32()>(&TestScriptClass::overload);

        auto overload      = make_unique_overload(f1, f2, f3);
        global["obj"]      = &t;
        global["overload"] = overload.get();

        auto res = run<f32>("return overload(obj)");
        REQUIRE(res);
        REQUIRE(res.value() == t.overload());

        res = run<f32>("return overload(obj, 1, 2.5)");
        REQUIRE(res);
        REQUIRE(res.value() == t.overload(1, 2.5f));

        res = run<f32>("return overload(obj, 2.5, 1)");
        REQUIRE(res);
        REQUIRE(res.value() == t.overload(2.5f, 1));
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Results")
{
    {
        auto x = run("function return 1 edn", "bad script");
        REQUIRE(x.error() == error_code::Error);
    }
    {
        auto res = run("x <- \"ok\"");
        REQUIRE(res);
        auto f = global.get<f32>("x");
        REQUIRE(f.error() == error_code::TypeMismatch);
        f = global.get<f32>("testX");
        REQUIRE(f.error() == error_code::Undefined);
        auto s = global.get<std::string>("x");
        REQUIRE(s.has_value());
        REQUIRE(s.value() == "ok");
    }
    {
        auto res = run("arrayX <- [1,2,3,\"a\"]");
        REQUIRE(res);
        auto tab = global.get<std::vector<i32>>("arrayX");
        REQUIRE(tab.error() == error_code::TypeMismatch);
    }
    {
        auto res = run<i32>("return \"ok\"");
        REQUIRE(res.error() == error_code::TypeMismatch);
        REQUIRE(res.value_or(200) == 200);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Run")
{
    SUBCASE("get int")
    {
        {
            char const* source = R"-(
            return 1042
        )-";

            auto result = run<i64>(source);
            REQUIRE(result.has_value());
            REQUIRE(result.value() == 1042);
        }
        {
            char const* source = R"-(
            function f() { return 521 }
            return f()
        )-";

            auto result = run<i64>(source);
            REQUIRE(result.has_value());
            REQUIRE(result.value() == 521);
        }
    }
    SUBCASE("get double")
    {
        char const* source = R"-(
            return 1.42
        )-";

        auto result = run<f64>(source);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == Approx(1.42));
    }
    SUBCASE("get string")
    {
        char const* source = R"-(
            return "ok"
        )-";

        auto result = run<std::string>(source);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == "ok");
    }
    SUBCASE("get bool")
    {
        char const* source = R"-(
            return true
        )-";

        auto result = run<bool>(source);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == true);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Table")
{
    SUBCASE("table as parameter")
    {
        {
            i32           x {0};
            std::function func {[&](table& tab0) {
                REQUIRE(tab0.has("x"));
                x = tab0["x"].as<i32>();
            }};

            global["func"] = &func;
            auto res       = run("local tab = { x = 42 }; func(tab);");
            REQUIRE(res);
            REQUIRE(x == 42);
        }
        {
            i32           x {0};
            std::function func {[&](stack_base& root, table& tab) {
                REQUIRE(root.has("y"));
                x += root["y"].as<i32>();
                REQUIRE(tab.has("x"));
                x += tab["x"].as<i32>();
            }};

            global["func"] = &func;
            auto res       = run("y <- 100; local tab = { x = 42 }; func(tab);");
            REQUIRE(res);
            REQUIRE(x == 142);
        }
    }
    SUBCASE("basic operations")
    {
        {
            auto res                             = run("tableX <- { }");
            global["tableX"]["a"]["b"]["c"]["d"] = 100;
            i32 x                                = global["tableX"]["a"]["b"]["c"]["d"].as<i32>();
            REQUIRE(res);
            REQUIRE(x == 100);
        }
        {
            auto res = run("tableX <- {left=2.7, top={x=10,y=2} }");
            i32  x   = *global.get<i32>("tableX", "top", "x");
            REQUIRE(res);
            REQUIRE(x == 10);
        }
        {
            auto res = run("tableX <- {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);

            i32 x {0};
            REQUIRE(global["tableX"]["top"].as<table>().try_get<i32>(x, "x"));
            REQUIRE(x == 10);

            REQUIRE_FALSE(global.try_get<i32>(x, "x"));
            REQUIRE_FALSE(global.try_get<i32>(x, "tableX"));

            point_f y;
            REQUIRE(global["tableX"].as<table>().try_get(y, "top"));
            REQUIRE_FALSE(global["tableX"].as<table>().try_get(y, "x"));
        }
        {
            table tab = *run<table>("return {left=2.7, top=3.1, width=2.3, height=55.2} ");
            f32   f   = tab["top"].as<f32>();
            REQUIRE(f == 3.1f);
        }
        {
            table tab = *run<table>("return {left=2.7, top=3.1, width=2.3, height=55.2} ");
            REQUIRE(tab.has("left"));
            REQUIRE(tab.has("top"));
            REQUIRE(tab.has("width"));
            REQUIRE(tab.has("height"));
        }
        {
            table tab = *run<table>("return {a = 2.4, b = true, c = \"hello\"} ");
            REQUIRE(tab.is<f32>("a"));
            REQUIRE(tab["b"].is<bool>());
            REQUIRE(tab.is<std::string>("c"));
        }
        {
            table tab = *run<table>("return { a = 2.4, b = \"ok\" } ");
            REQUIRE(tab["a"].get<bool>().value_or(false) == false);
            REQUIRE(tab["b"].get<std::string>().value_or("default") == "ok");
        }

        {
            table                    tab = *run<table>("local tab = { a = 2.4, b = true, c = \"hello\" }; tab[1] <- 42; return tab ");
            std::vector<std::string> vect {"a", "b", "c"};
            std::vector<std::string> keys = tab.get_keys<std::string>();
            std::ranges::sort(keys);
            REQUIRE(keys == vect);
        }
        {
            table            tab = *run<table>("local tab = { a = 22 }; tab[1] <- \"a\"; tab[2] <- 3; tab[3] <- 55; return tab");
            std::vector<i32> vect {1, 2, 3};
            std::vector<i32> keys = tab.get_keys<i32>();
            std::sort(keys.begin(), keys.end());
            REQUIRE(keys == vect);
        }
        {
            using var            = std::variant<i32, std::string>;
            table            tab = *run<table>("local tab = {a = 2.4, c = \"hello\"}; tab[1] <- 42; return tab ");
            std::vector<var> vect {1, "a", "c"};
            auto             keys = tab.get_keys<var>();
            std::ranges::sort(keys);
            REQUIRE(keys == vect);
        }

        {
            auto res = run("rectF <- {x=2.7, y=3.1, width=2.3, height=55.2} ");
            REQUIRE(res);
            auto tab = global["rectF"].as<table>();
            f32  f   = tab["x"].as<f32>();
            REQUIRE(f == 2.7f);
        }
        {
            auto res = run(
                "rectF <- {x=2.7, y=3.1, width=2.3, height=55.2} "
                "function tabletest(x) { return x.y}");
            REQUIRE(res);
            auto tab  = global["rectF"].as<table>();
            tab["y"]  = 100.5f;
            auto func = global["tabletest"].as<function<f32>>();
            f32  x    = func(tab);
            REQUIRE(x == 100.5f);
            REQUIRE(tab["y"].as<f32>() == 100.5f);
        }
        {
            auto res = run("tableX <- {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);
            auto top = global["tableX"]["top"].as<table>();
            i32  x   = top["x"].as<i32>();
            REQUIRE(x == 10);
        }
        {
            auto res = run("tableX <- {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);
            auto tab = global["tableX"].as<table>();
            auto top = tab["top"].as<point_i>();
            REQUIRE(top.X == 10);
        }
        {
            auto res = run("tableX <- {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);
            auto tab = global["tableX"].as<table>();
            i32  top = tab["top"]["x"].as<i32>();
            REQUIRE(top == 10);
        }
        {
            auto res = run("tableX <- {left=2.7, top={x=10,y=2} }");
            REQUIRE(res);
            auto tab        = global["tableX"].as<table>();
            tab["top"]["x"] = 400;
            i32 top         = global["tableX"]["top"]["x"].as<i32>();
            REQUIRE(top == 400);
        }
        {
            auto res = run("tableX <- { a={ b={ c={ d=2 } } } }");
            REQUIRE(res);
            auto tab                = global["tableX"].as<table>();
            tab["a"]["b"]["c"]["d"] = 42;
            i32 top                 = global["tableX"]["a"]["b"]["c"]["d"].as<i32>();
            REQUIRE(top == 42);
        }
        {
            auto res = run("tableX <- { a={ b={ c={ d=2 } } } }");
            REQUIRE(res);
            auto tab = global["tableX"].as<table>();
            REQUIRE(tab["a"]["b"]["c"]["d"].as<i32>() == 2);
            res = run("tableX.a.b.c.d = 4");
            REQUIRE(tab["a"]["b"]["c"]["d"].as<i32>() == 4);
        }
        {
            auto res = run("tableX <- { a={ b={ c={ d=2 } } } }");
            REQUIRE(res);
            auto tab = global["tableX"]["a"]["b"]["c"].as<table>();
            REQUIRE(tab["d"].as<i32>() == 2);
            res = run("tableX.a.b.c.d = 4");
            REQUIRE(tab["d"].as<i32>() == 4);
            tab = global["tableX"].as<table>();
            REQUIRE(tab["a"]["b"]["c"]["d"].get<i32>().value() == 4);
        }
        {
            auto res = run("tableX <- { a={ b={ bb = \"ok\", c={ d=2 } } } }");
            REQUIRE(res);
            auto tab                             = global["tableX"].as<table>();
            global["tableX"]["a"]["b"]["c"]["d"] = 100;
            REQUIRE(global["tableX"]["a"]["b"]["c"]["d"].as<i32>() == 100);
            REQUIRE(tab["a"]["b"]["c"]["d"].as<i32>() == 100);
            auto x = tab["a"]["b"]["bb"].as<std::string>();
            REQUIRE(x == "ok");
        }
        {
            auto res = run("tableX <- {  }");
            REQUIRE(res);
            auto  tab = global["tableX"].as<table>();
            table subt {};
            REQUIRE_FALSE(subt.is_valid());
            tab["sub"] = subt;
            REQUIRE(subt.is_valid());
            subt["x"] = 42;

            REQUIRE(global["tableX"]["sub"]["x"].as<i32>() == 42);
        }
        {
            auto res = run("tableX <- {  }");
            REQUIRE(res);
            auto tab        = global["tableX"].as<table>();
            tab["sub"]      = table {};
            tab["sub"]["x"] = 42;

            REQUIRE(global["tableX"]["sub"]["x"].as<i32>() == 42);
        }
        {
            auto res = run("tableX <- {  }");
            REQUIRE(res);
            auto  tab = global["tableX"].as<table>();
            table subt;
            tab["sub"] = subt;
            subt["x"]  = 42;

            REQUIRE(global["tableX"]["sub"]["x"].as<i32>() == 42);
        }
        {
            table tab0 = *run<table>("tableX <- {  }; tableY <- {  }; return tableX;");
            auto  tab1 = global["tableX"].as<table>();
            REQUIRE(tab0 == tab1);
            auto tab2 = global["tableY"].as<table>();
            REQUIRE(tab0 != tab2);
        }
        {
            table tab = *run<table>("local tableX = { a = 12 , b = { c = 100 }}; return tableX;");
            REQUIRE(tab.is<table>("b"));
            REQUIRE(tab["b"]["c"].as<i32>() == 100);
        }
    }
    SUBCASE("try_make")
    {
        {
            auto s0 = create_table();
            s0["x"] = 100;
            s0["y"] = 200;
            point_f p;
            REQUIRE(s0.try_make<f32, f32>(p, "x", "y"));
            REQUIRE(p == point_f {100, 200});
        }
        {
            auto s0 = create_table();
            s0["x"] = 100;
            s0["y"] = 200;
            size_f p;
            REQUIRE(s0.try_make<f32, f32>(p, "x", "y"));
            REQUIRE(p == size_f {100, 200});
        }
        {
            auto s0 = create_table();
            s0["x"] = 100;
            s0["z"] = 200;
            point_f p;
            REQUIRE_FALSE(s0.try_make<f32, f32>(p, "x", "y"));
        }
        {
            auto s0 = create_table();
            s0["x"] = 100;
            s0["y"] = false;
            point_f p;
            REQUIRE_FALSE(s0.try_make<f32, f32>(p, "x", "y"));
        }
    }
    SUBCASE("delegate")
    {
        auto tab              = create_table();
        global["tab"]         = tab;
        auto          metatab = create_table();
        std::function func    = [](table const&) {
            return "hello world";
        };
        metatab["_tostring"] = &func;
        tab.set_delegate(metatab);

        auto res = run<std::string>("return tab.tostring()");
        REQUIRE(res.has_value());
        REQUIRE(res.value().starts_with("hello world"));
    }
    SUBCASE("proxy")
    {
        SUBCASE("int")
        {
            auto tab = table::Create(get_view());
            tab["x"] = 100;
            tab["y"] = tab["x"];
            REQUIRE(tab["x"].as<i32>() == 100);
            REQUIRE(tab["y"].as<i32>() == 100);
        }
        SUBCASE("bool")
        {
            auto tab = table::Create(get_view());
            tab["x"] = true;
            tab["y"] = tab["x"];
            REQUIRE(tab["x"].as<bool>() == true);
            REQUIRE(tab["y"].as<bool>() == true);
        }
        SUBCASE("string")
        {
            auto tab = table::Create(get_view());
            tab["x"] = "ok";
            tab["y"] = tab["x"];
            REQUIRE(tab["x"].as<string>() == "ok");
            REQUIRE(tab["y"].as<string>() == "ok");
        }
        SUBCASE("table")
        {
            auto tab1 = table::Create(get_view());
            auto tab2 = table::Create(get_view());
            tab2["a"] = 100;
            tab1["x"] = tab2;
            tab1["y"] = tab1["x"];
            REQUIRE(tab1["x"]["a"].as<i32>() == 100);
            REQUIRE(tab1["y"]["a"].as<i32>() == 100);
        }
        SUBCASE("undefined")
        {
            auto tab = table::Create(get_view());
            tab["y"] = tab["x"];
            REQUIRE_FALSE(tab.has("y"));
        }
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.TcobTypes")
{
    std::function testFuncColor = [](color c) {
        return color {static_cast<u8>(c.R * 2),
                      static_cast<u8>(c.G * 2),
                      static_cast<u8>(c.B * 2),
                      static_cast<u8>(c.A * 2)};
    };
    std::function testFuncPointF = [](point_f p) { return point_f {p.X * 2, p.Y * 2}; };
    std::function testFuncPointI = [](point_i p) { return point_i {p.X * 2, p.Y * 2}; };
    std::function testFuncSizeI  = [](size_i s) { return size_i {s.Width * 5, s.Height * 8}; };
    std::function testFuncRectF  = [](rect_f r) { return rect_f {r.left() * 2, r.top() * 2, r.width() * 2, r.height() * 2}; };
    std::function testFuncRectI  = [](rect_i r) { return rect_i {r.left() * 2, r.top() * 2, r.width() * 2, r.height() * 2}; };
    std::function testFuncMix    = [](i32 i, rect_f r, color c, std::string const& s, bool b, point_i p) {
        f32 ret = static_cast<f32>(i) + r.left() + c.A + static_cast<f32>(s.length()) + (b ? 1 : 100) + static_cast<f32>(p.X);
        return ret;
    };

    global["test"]["Color"]  = &testFuncColor;
    global["test"]["PointF"] = &testFuncPointF;
    global["test"]["PointI"] = &testFuncPointI;
    global["test"]["RectF"]  = &testFuncRectF;
    global["test"]["RectI"]  = &testFuncRectI;
    global["test"]["SizeI"]  = &testFuncSizeI;
    global["test"]["Mix"]    = &testFuncMix;
    SUBCASE("get from globals")
    {
        auto res = run(
            "rectI <- {x=3, y=6, width=10, height=20} "
            "rectF <- {x=2.7, y=3.1, width=2.3, height=55.2} "
            "color <- { r= 1, g = 2, b = 3, a = 1} "
            "pointI <- { x = 20, y = 400 } "
            "pointF <- { x = 4.5, y = 3.23 } ");
        REQUIRE(res);
        REQUIRE(global["color"].get<color>().value() == color(1, 2, 3, 1));
        REQUIRE(global["pointI"].get<point_i>().value() == point_i(20, 400));
        REQUIRE(global["pointF"].get<point_f>().value() == point_f(4.5f, 3.23f));
        REQUIRE(global["rectI"].get<rect_i>().value() == rect_i(3, 6, 10, 20));
        REQUIRE(global["rectF"].get<rect_f>().value() == rect_f(2.7f, 3.1f, 2.3f, 55.2f));
    }
    SUBCASE("is")
    {
        auto res = run(
            "rectI <- {x=3, y=6, width=10, height=20}; "
            "rectI = test.RectI(rectI); "

            "rectF <- {x=2.7, y=3.1, width=2.3, height=55.2}; "
            "rectF = test.RectF(rectF); "

            "color <- { r = 1, g = 2, b = 3, a = 1 }; "
            "color = test.Color(color);"

            "pointI <- { x = 20, y = 400 }; "
            "pointI = test.PointI(pointI); "

            "pointF <- { x = 4.5, y = 3.23 }; "
            "pointF = test.PointF(pointF); "

            "sizeI <- { width = 20, height = 400 }; "
            "sizeI = test.SizeI(sizeI); "

            "rectIS  <- { x=3,   y=6,   width=10,  height=20   }; "
            "rectFS  <- { x=2.7, y=3.1, width=2.3, height=55.2 }; "
            "pointIS <- { x=20,  y=400  }; "
            "pointFS <- { x=4.5, y=3.23 }; "

            "degree <- 160;");
        REQUIRE(res);

        auto c = global["color"].as<color>();
        REQUIRE(global.is<color>("color"));
        REQUIRE(c == color(1 * 2, 2 * 2, 3 * 2, 1 * 2));

        auto p1 = global["pointI"].as<point_i>();
        REQUIRE(global.is<point_i>("pointI"));
        REQUIRE(global.is<point_i>("pointIS"));
        REQUIRE(p1 == point_i(20 * 2, 400 * 2));

        auto p2 = global["pointF"].as<point_f>();
        REQUIRE(global.is<point_f>("pointF"));
        REQUIRE(global.is<point_f>("pointFS"));
        REQUIRE(p2 == point_f(4.5f * 2, 3.23f * 2));

        auto s1 = global["sizeI"].as<size_i>();
        REQUIRE(global.is<size_i>("sizeI"));
        REQUIRE(s1 == size_i(20 * 5, 400 * 8));

        auto r1 = global["rectI"].as<rect_i>();
        REQUIRE(global.is<rect_i>("rectI"));
        REQUIRE(global.is<rect_i>("rectIS"));
        REQUIRE(r1 == rect_i(3 * 2, 6 * 2, 10 * 2, 20 * 2));

        auto r2 = global["rectF"].as<rect_f>();
        REQUIRE(global.is<rect_f>("rectF"));
        REQUIRE(global.is<rect_f>("rectFS"));
        REQUIRE(r2 == rect_f(2.7f * 2, 3.1f * 2, 2.3f * 2, 55.2f * 2));

        auto deg = global["degree"].as<degree_f>();
        REQUIRE(global.is<degree_f>("degree"));
        REQUIRE(deg == 160);
    }
    SUBCASE("parameters")
    {
        auto res = run(
            "rectF <- { x=2.7, y=3.1, width=2.3, height=55.2} "
            "color <- { r = 1, g = 2, b = 3, a = 1} "
            "pointI <- { x = 20, y = 400 } "
            "x <- test.Mix(100, rectF, color, \"Hello\", false, pointI)");
        REQUIRE(res);
        f32 x = global["x"].as<f32>();

        REQUIRE(x == testFuncMix(100, rect_f(2.7f, 3.1f, 2.3f, 55.2f), color(1, 2, 3, 1), "Hello", false, point_i(20, 400)));
    }
    SUBCASE("error checking")
    {
        auto res = run(
            "rectF <- { x=2.7, y=3.1, width=\"hello\", height=true } "
            "color <- { r = \"red\", g = \"green\", b = \"blue\", a = \"aqua\" } "
            "pointI <- { x = \"1\", y = \"400\" } ");
        REQUIRE(res);

        auto rectF = global.get<rect_f>("rectF");
        REQUIRE(rectF.error() == error_code::TypeMismatch);

        auto col = global.get<color>("color");
        REQUIRE(col.error() == error_code::TypeMismatch);

        auto pointI = global.get<point_f>("pointI");
        REQUIRE(pointI.error() == error_code::TypeMismatch);
    }
    SUBCASE("api: color_stop")
    {
        auto res = run<color_stop>("return { pos = 150, value = { r = 10, g = 20, b = 40 } }").value();
        REQUIRE(res.Position == 150.f);
        REQUIRE(res.Value == color {10, 20, 40, 255});
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.Threads")
{
    {
        auto res = run(R"(
                function coroutine_test()
                {
                    ::suspend("1");
                    ::suspend("2");
                }

                 coro <- ::newthread(coroutine_test);
        )");

        REQUIRE(res);
        REQUIRE(global.is<thread>("coro"));
        auto t = global["coro"].as<thread>();
        REQUIRE(t.status() == vm_view::status::Idle);
        auto cres = t.call<std::string>();
        REQUIRE(t.status() == vm_view::status::Suspended);
        REQUIRE(cres);
        REQUIRE(cres.value() == "1");
        auto wres = t.wake_up<std::string>();
        REQUIRE(wres);
        REQUIRE(wres.value() == "2");
    }
    {
        auto res = run(R"(
                function coroutine_test(x)
                {
                    ::suspend(x+"1");
                    ::suspend(x+"2");
                }

                 coro <- ::newthread(coroutine_test);
        )");

        REQUIRE(res);
        REQUIRE(global.is<thread>("coro"));
        auto t = global["coro"].as<thread>();
        REQUIRE(t.status() == vm_view::status::Idle);
        auto cres = t.call<std::string>("a");
        REQUIRE(t.status() == vm_view::status::Suspended);
        REQUIRE(cres);
        REQUIRE(cres.value() == "a1");
        auto wres = t.wake_up<std::string>("a");
        REQUIRE(wres);
        REQUIRE(wres.value() == "a2");
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.TypeCoercion")
{
    SUBCASE("string from int")
    {
        auto res = run("a <- 100 ");
        REQUIRE(res);
        REQUIRE(global.is<i32>("a"));
        REQUIRE_FALSE(global.is<std::string>("a"));
        auto val = global["a"].as<std::string>();
        REQUIRE(val == "100");
    }
    SUBCASE("string from number")
    {
        auto res = run("a <- 100.5 ");
        REQUIRE(res);
        REQUIRE(global.is<f32>("a"));
        REQUIRE_FALSE(global.is<std::string>("a"));
        auto val = global["a"].as<std::string>();
        REQUIRE(val == "100.500000");
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.UserDefinedConversion")
{
    std::function Foo = [](foo const& f) {
        return f.x + f.y + f.z;
    };
    global["test"]["Foo"] = &Foo;

    {
        foo i = *run<foo>("return {x=3,y=2,z=1}");
        REQUIRE(i.x == 3);
    }
    {
        i32 i = *run<i32>("return test.Foo({x=3,y=2,z=1})");
        REQUIRE(i == Foo(foo {3, 2, 1}));
    }
    {
        auto res = run("foo <- {x=3,y=2,z=1}");
        REQUIRE(res);
        REQUIRE(global.is<foo>("foo"));
        res = run("foo = {x=3,n=2,z=1}");
        REQUIRE_FALSE(global.is<foo>("foo"));
    }
    {
        auto res = run("function bar(p) {return p.x * p.y * p.z} ");
        REQUIRE(res);
        auto func = global["bar"].as<function<i32>>();
        i32  a    = *func.call(foo {1, 2, 3});
        REQUIRE(a == 6);
    }
}

TEST_CASE_FIXTURE(SquirrelScriptTests, "Script.Squirrel.VariadicFunctions")
{
    SUBCASE("variadic Squirrel function")
    {
        auto res = run(
            R"(
            function testArg(...) {
                local retValue = 0;
                foreach(i, v in vargv) {retValue += v;}
                return retValue;
            }
        )");

        REQUIRE(res);
        auto func = global["testArg"].as<function<i32>>();
        i32  a    = func(1, 2, 3, 4, 5, 6);
        REQUIRE(a == (1 + 2 + 3 + 4 + 5 + 6));
    }
}
