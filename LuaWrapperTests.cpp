#include "WrapperTestsClass.hpp"
#include "tests.hpp"

class LuaWrapperTests : public script {
public:
    LuaWrapperTests()
        : global(global_table())
    {
        open_libraries();
    }

    table global;
};

TEST_CASE_FIXTURE(LuaWrapperTests, "Script.LuaWrapper.MapWrapper")
{
    create_wrapper<std::map<std::string, i32>>("map");
    {
        std::map<std::string, i32> map = {{"a", 0}, {"b", 1}};
        global["wrap"]                 = &map;
        i32 x                          = *run<i32>("return wrap.b");
        REQUIRE(x == 1);
    }
    {
        std::map<std::string, i32> map = {{"a", 0}, {"b", 1}};
        global["wrap"]                 = &map;
        auto res                       = run("wrap.b = 100");
        REQUIRE(map["b"] == 100);
        res = run("wrap.c = 42");
        REQUIRE(map["c"] == 42);
    }
    {
        std::map<std::string, i32> map = {{"a", 0}, {"b", 1}};
        global["wrap"]                 = &map;
        auto res                       = run("b = wrap.b");
        REQUIRE(res);
        REQUIRE(global["b"].as<i32>() == map["b"]);
    }

    create_wrapper<std::unordered_map<std::string, i32>>("unmap");
    {
        std::unordered_map<std::string, i32> map = {{"a", 0}, {"b", 1}};
        global["wrap"]                           = &map;
        i32 x                                    = *run<i32>("return wrap.b");
        REQUIRE(x == 1);
    }
    {
        std::unordered_map<std::string, i32> map = {{"a", 0}, {"b", 1}};
        global["wrap"]                           = &map;
        auto res                                 = run("wrap.b = 100");
        REQUIRE(map["b"] == 100);
        res = run("wrap.c = 42");
        REQUIRE(map["c"] == 42);
    }
    {
        std::unordered_map<std::string, i32> map1 = {{"a", 0}, {"b", 1}};
        global["wrap1"]                           = &map1;
        std::unordered_map<std::string, i32> map2 = {{"b", 1}, {"a", 0}};
        global["wrap2"]                           = &map2;
        std::unordered_map<std::string, i32> map3 = {{"a", 1}, {"b", 0}};
        global["wrap3"]                           = &map3;
        bool res                                  = *run<bool>("return wrap1 == wrap2");
        REQUIRE(res);
        res = *run<bool>("return wrap3 == wrap2");
        REQUIRE_FALSE(res);
    }
}

TEST_CASE_FIXTURE(LuaWrapperTests, "Script.LuaWrapper.VectorWrapper")
{
    create_wrapper<std::vector<i32>>("vec");
    SUBCASE("set indices")
    {
        std::vector<i32> vec = {0, 1, 2, 3, 4, 5};
        global["wrap"]       = &vec;
        i32 x                = *run<i32>("return wrap[2]");
        REQUIRE(x == vec[1]);
        auto res = run("wrap[4] = 100");
        REQUIRE(100 == vec[3]);
        res = run("wrap[7] = 100");
        REQUIRE(100 == vec[6]);
    }
    SUBCASE("length and adding")
    {
        std::vector<i32> vec = {0, 1, 2, 3, 4, 5};
        global["wrap"]       = &vec;
        i32 x                = *run<i32>("return #wrap");
        REQUIRE(x == vec.size());
        auto res = run("wrap[#wrap + 1] = 6");
        REQUIRE(res.has_value());
        REQUIRE(6 == vec[6]);
    }
    SUBCASE("iterate")
    {
        std::vector<i32> vec = {0, 1, 2, 3, 4, 5};
        global["wrap"]       = &vec;

        i32 x = *run<i32>(
            "result = 0 "
            "for i, v in ipairs(wrap) do "
            "result = result + v "
            "end "
            "return result");
        REQUIRE(x == 15);
    }
}

TEST_CASE_FIXTURE(LuaWrapperTests, "Script.LuaWrapper.TypeWrapper")
{
    TestScriptClass earlyt;
    global["earlywrap"] = &earlyt;

    i32 lambdaValue {0};

    auto wrapper = create_wrapper<TestScriptClass>("TSC");

    wrapper->method<&TestScriptClass::foo>("foo");
    wrapper->method<&TestScriptClass::bar>("bar");
    wrapper->method<[] -> i32 { return 40; }>("lambda_func");
    wrapper->method<&TestScriptClass::add_value>("add");
    wrapper->method<&TestScriptClass::abstractMethod>("abstract");
    wrapper->method<&TestScriptClass::virtualMethod>("virtual");
    wrapper->method<&TestScriptClass::baseMethod>("base");
    wrapper->method<&TestScriptClass::ptr>("ptr");

    wrapper->property<&TestScriptClass::FieldValue>("field");
    wrapper->property<&TestScriptClass::PropertyValue>("prop");

    wrapper->property<&TestScriptClass::get_value, &TestScriptClass::set_value>("value");
    wrapper->property(
        "lambda_prop",
        [&lambdaValue] { return lambdaValue; },
        [&lambdaValue](TestScriptClass*, i32 val) { lambdaValue = val; });
    wrapper->getter<&TestScriptClass::get_value>("readonly_value");
    wrapper->setter<&TestScriptClass::set_value>("writeonly_value");
    wrapper->getter<&TestScriptClass::get_map>("map");

    wrapper->constructors<TestScriptClass(i32), TestScriptClass(i32, f32), TestScriptClass()>();

    auto f1 = resolve_overload<f32(i32, f32)>(&TestScriptClass::overload);
    auto f2 = resolve_overload<f32(f32, i32)>(&TestScriptClass::overload);
    auto f3 = resolve_overload<f32(std::vector<f32> const&)>(&TestScriptClass::overload);
    auto f4 = resolve_overload<f32(i32, f32, f32)>(&TestScriptClass::overload);
    auto f5 = resolve_overload<f32(f32, i32, f32)>(&TestScriptClass::overload);
    auto f6 = [](f32 x) -> f32 { return x + 40.0f; };
    wrapper->overload("overload", f1, f2, f3, f4, f5, f6);

    SUBCASE("early wrap")
    {
        i32 x = *run<i32>("return earlywrap:foo('test', 2, true)");
        REQUIRE(x == 2 * 4);
    }
    SUBCASE("index access")
    {
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(100);
        i32 i = *run<i32>("return wrap1[1]");
        REQUIRE(i == 100);
        auto res = run("wrap1[1] = 400");
        REQUIRE(res.has_value());
        REQUIRE(t1.get_value() == 400);
    }
    SUBCASE("pointer as parameter")
    {
        global["test"]["WrapperObj"] = testFuncWrapperObj;
        TestScriptClass t1;
        global["wrap"] = &t1;
        t1.set_value(100);
        i32 i = *run<i32>("return test.WrapperObj(wrap)");
        REQUIRE(i == 100);
    }
    SUBCASE("constructor")
    {
        TestScriptClass* t = *run<TestScriptClass*>("return TSC.new(20)");
        REQUIRE(t->get_value() == 20);
        t = *run<TestScriptClass*>("return TSC.new(20, 3.5)");
        REQUIRE(t->get_value() == 20 * (i32)3.5f);
        t = *run<TestScriptClass*>("return TSC.new()");
        REQUIRE(t->get_value() == 0);
    }
    SUBCASE("pointer from lua")
    {
        TestScriptClass t;
        global["wrap"] = &t;
        auto* tp       = global["wrap"].as<TestScriptClass*>();
        REQUIRE(tp == &t);
    }
    SUBCASE("properties")
    {
        {
            TestScriptClass t;
            global["wrap"] = &t;
            t.set_value(42);
            i32 value = *run<i32>("return wrap.readonly_value");
            REQUIRE(value == 42);

            auto res = run("wrap.writeonly_value = 21");
            REQUIRE(res.has_value());
            REQUIRE(t.get_value() == 21);
        }
        {
            lambdaValue = 42;
            i32 value   = *run<i32>("return wrap.lambda_prop");
            REQUIRE(value == 42);
            auto res = run("wrap.lambda_prop = 21");
            REQUIRE(res.has_value());
            REQUIRE(lambdaValue == 21);
        }
        {
            TestScriptClass t;
            global["wrap"]       = &earlyt;
            earlyt.PropertyValue = 42;

            bool changed {false};
            earlyt.PropertyValue.Changed.connect([&changed](i32 const&) { changed = true; });

            i32 value = *run<i32>("return wrap.prop");
            REQUIRE(value == 42);

            REQUIRE_FALSE(changed);

            auto res = run("wrap.prop = 21");
            REQUIRE(res.has_value());
            REQUIRE(earlyt.PropertyValue == 21);

            REQUIRE(changed);
        }
        {
            TestScriptClass t;
            global["wrap"] = &t;
            t.set_value(350);
            auto res = run("function foo(x) return x.value end ");
            REQUIRE(res.has_value());

            auto func = global["foo"].as<function<i32>>();
            i32  x    = func(&t);
            REQUIRE(x == 350);
        }
        {
            TestScriptClass t1;
            global["wrap"] = &t1;

            t1.set_value(100);
            i32 x = *run<i32>("return wrap.value");
            REQUIRE(x == 100);
            x = *run<i32>("return wrap:add(20)");
            REQUIRE(x == 120);

            TestScriptClass t2;
            t2.set_value(250);

            global["wrap"] = &t2;
            x              = *run<i32>("return wrap.value");
            REQUIRE(x == 250);

            global["wrap"] = &t1;
            x              = *run<i32>("return wrap:add(20)");
            REQUIRE(x == 120);
        }
    }
    SUBCASE("field")
    {
        TestScriptClass t;
        global["wrap"] = &t;
        t.FieldValue   = 42;
        i32 value      = *run<i32>("return wrap.field");
        REQUIRE(value == 42);

        auto res = run("wrap.field = 21");
        REQUIRE(res.has_value());
        REQUIRE(t.FieldValue == 21);
    }
    SUBCASE("overloads")
    {
        TestScriptClass t;
        global["wrap"] = &t;
        f32 x          = *run<f32>("return wrap:overload({0.2,0.4})");
        REQUIRE(x == t.overload(std::vector<f32> {0.2f, 0.4f}));

        x = *run<f32>("return wrap:overload(4, 2.1)");
        REQUIRE(x == t.overload(4, 2.1f));

        x = *run<f32>("return wrap:overload(2.1, 12)");
        REQUIRE(x == t.overload(2.1f, 12));

        x = *run<f32>("return wrap:overload(15, 2.1, 99.9)");
        REQUIRE(x == t.overload(15, 2.1f, 99.9f));

        x = *run<f32>("return wrap:overload(2.1, 15, 1.5)");
        REQUIRE(x == t.overload(2.1f, 15, 1.5f));

        x = *run<f32>("return wrap.overload(20)");
        REQUIRE(x == 60.f);
    }
    SUBCASE("functions")
    {
        TestScriptClass t;
        global["wrap"] = &t;
        i32  x         = *run<i32>("return wrap:foo('test', 4, true)");
        auto res       = run("wrap:bar(true, 'test', 4)");
        REQUIRE(res.has_value());
        REQUIRE(x == 4 * 4);
    }
    SUBCASE("inheritance")
    {
        TestScriptClass t;
        t.set_value(42);
        global["wrap"] = &t;
        i32 x          = *run<i32>("return wrap:virtual()");
        REQUIRE(x == 42 * 2);
        i32 y = *run<i32>("return wrap:abstract()");
        REQUIRE(y == 42 * 4);
        i32 z = *run<i32>("return wrap:base()");
        REQUIRE(z == 21);
    }
    SUBCASE("wrapped member")
    {
        create_wrapper<std::map<std::string, i32>>("map");
        TestScriptClass t;
        global["wrap"]                  = &t;
        std::map<std::string, i32>& map = *t.get_map();
        map["x"]                        = 100;
        auto res                        = run("wrap.map.x = 300 ");
        REQUIRE(res.has_value());
        map = *t.get_map();
        REQUIRE(map["x"] == 300);
    }
}

TEST_CASE_FIXTURE(LuaWrapperTests, "Script.LuaWrapper.TypeWrapper2")
{
    std::string name {"Dave"};

    auto& wrapper {*create_wrapper<Player>("Player")};
    wrapper.constructors<Player(std::string, i32, i32)>();

    wrapper["get_health"]      = &Player::get_health;
    wrapper["take_damage"]     = &Player::take_damage;
    wrapper["get_mana"]        = &Player::get_mana;
    wrapper["set_mana"]        = &Player::set_mana;
    wrapper["use_mana"]        = &Player::use_mana;
    wrapper["gain_experience"] = &Player::gain_experience;
    wrapper["mana"]            = getter {&Player::get_mana};
    wrapper["mana"]            = setter {&Player::set_mana};
    wrapper["health"]          = property {&Player::get_health, &Player::set_health};
    wrapper["name"]            = getter {[&name] { return name; }};

    auto f1                     = resolve_overload<void(std::string const&)>(&Player::add_to_inventory);
    auto f2                     = resolve_overload<void(int)>(&Player::add_to_inventory);
    wrapper["add_to_inventory"] = overload {f1, f2};

    SUBCASE("constructor")
    {
        Player* pl = *run<Player*>("return Player.new('bob', 100, 50)");
        REQUIRE(pl->get_mana() == 50);
        REQUIRE(pl->get_health() == 100);
        REQUIRE(pl->name() == "bob");
    }
    SUBCASE("functions")
    {
        Player pl {name, 100, 50};
        global["dave"] = &pl;
        auto res {run("dave:use_mana(20) "
                      "dave:take_damage(80) "
                      "local h = dave:get_health() "
                      "if h == 20 then dave:gain_experience(200) end ")};
        REQUIRE(res.has_value());
        REQUIRE(pl.get_mana() == 30);
        REQUIRE(pl.get_health() == 20);
        REQUIRE(pl.get_experience() == 200);
    }
    SUBCASE("getter/setter")
    {
        Player pl {name, 100, 50};
        global["dave"] = &pl;
        auto res0 {run("mana = dave.mana "
                       "dave.mana = mana/2 ")};
        REQUIRE(res0.has_value());
        REQUIRE(pl.get_mana() == 25);

        auto res1 {run<std::string>("return dave.name")};
        REQUIRE(res1.has_value());
        REQUIRE(res1.value() == name);

        auto res2 {run<std::string>("dave.name = 'Bill'")};
        REQUIRE_FALSE(res2.has_value());
    }
    SUBCASE("property")
    {
        Player pl {name, 100, 50};
        global["dave"] = &pl;
        auto res {run("health = dave.health "
                      "dave.health = health/4 ")};
        REQUIRE(res.has_value());
        REQUIRE(pl.get_health() == 25);
    }

    SUBCASE("overload")
    {
        Player pl {name, 100, 50};
        global["dave"] = &pl;
        auto res {run("dave:add_to_inventory('rope') "
                      "dave:add_to_inventory(42) ")};
        REQUIRE(res.has_value());
        REQUIRE(pl.get_inventory_size() == 2);
    }
}

TEST_CASE_FIXTURE(LuaWrapperTests, "Script.LuaWrapper.Metamethods")
{
    auto wrapper = create_wrapper<TestScriptClass>("TSCB");
    wrapper->metamethod(
        metamethod_type::Add, [](TestScriptClass* instance1, i32 x) { return scripting::managed_ptr(new TestScriptClass(instance1->get_value() + x)); });
    wrapper->metamethod(
        metamethod_type::Subtract, [](TestScriptClass* instance1, i32 x) { return scripting::managed_ptr(new TestScriptClass(instance1->get_value() - x)); });
    wrapper->metamethod(
        metamethod_type::Divide, [](TestScriptClass* instance1, i32 x) { return scripting::managed_ptr(new TestScriptClass(instance1->get_value() / x)); });
    wrapper->metamethod(
        metamethod_type::Multiply, [](TestScriptClass* instance1, i32 x) { return scripting::managed_ptr(new TestScriptClass(instance1->get_value() * x)); });
    wrapper->metamethod(
        metamethod_type::LessThan,
        [](std::variant<TestScriptClass*, i32> left, std::variant<TestScriptClass*, i32> right) {
            i32 const leftValue {std::holds_alternative<i32>(left) ? std::get<i32>(left) : std::get<TestScriptClass*>(left)->get_value()};
            i32 const rightValue {std::holds_alternative<i32>(right) ? std::get<i32>(right) : std::get<TestScriptClass*>(right)->get_value()};

            return leftValue < rightValue;
        });
    wrapper->metamethod(
        metamethod_type::LessOrEqualThan,
        [](std::variant<TestScriptClass*, i32> left, std::variant<TestScriptClass*, i32> right) {
            i32 const leftValue {std::holds_alternative<i32>(left) ? std::get<i32>(left) : std::get<TestScriptClass*>(left)->get_value()};
            i32 const rightValue {std::holds_alternative<i32>(right) ? std::get<i32>(right) : std::get<TestScriptClass*>(right)->get_value()};

            return leftValue <= rightValue;
        });
    wrapper->metamethod(
        metamethod_type::UnaryMinus, [](TestScriptClass* instance1) { return -instance1->get_value(); });
    wrapper->metamethod(
        metamethod_type::Length, [](TestScriptClass* instance1) { return instance1->get_value(); });
    wrapper->metamethod(
        metamethod_type::ToString, [](TestScriptClass* instance1) { return std::to_string(instance1->get_value()); });
    wrapper->metamethod(
        metamethod_type::Concat, [](TestScriptClass* instance1, i32 x) { return std::stoi(std::to_string(instance1->get_value()) + std::to_string(x)); });
    wrapper->metamethod(
        metamethod_type::Call, [](TestScriptClass* instance1, i32 x) { return x * instance1->get_value(); });
    wrapper->metamethod(
        metamethod_type::Close, [](TestScriptClass* instance1) { return instance1->Closed = true; });

    SUBCASE("Metatable")
    {
        TestScriptClass t1;
        global["wrap1"] = &t1;

        auto r0 = run<std::optional<table>>("return getmetatable(wrap1)").value();
        REQUIRE(r0.has_value());

        wrapper->hide_metatable("nope");
        auto r1 = run<std::optional<table>>("return getmetatable(wrap1)").value();
        REQUIRE_FALSE(r1.has_value());

        auto r2 = run<std::optional<std::string>>("return getmetatable(wrap1)").value();
        REQUIRE(r2.has_value());
        REQUIRE(r2 == "nope");

        wrapper->hide_metatable(nullptr);
        auto r3 = run<std::optional<table>>("return getmetatable(wrap1)").value();
        REQUIRE(r3.has_value());
    }
    SUBCASE("Call")
    {
        i32             value1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(value1);

        i32 b = *run<i32>("return wrap1(100)");
        REQUIRE(b == value1 * 100);
    }
    SUBCASE("Length")
    {
        i32             value1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(value1);

        i32 b = *run<i32>("return #wrap1");
        REQUIRE(b == value1);
    }
    SUBCASE("ToString")
    {
        i32             value1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(value1);

        std::string b = *run<std::string>("return tostring(wrap1)");
        REQUIRE(b == std::to_string(t1.get_value()));
    }
    SUBCASE("Concat")
    {
        i32             value1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(value1);

        i32 b = *run<i32>("return wrap1 .. 10");
        REQUIRE(b == 400010);
    }
    SUBCASE("LessOrEqualThan")
    {
        i32             value1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(value1);

        i32 value2       = 4000;
        global["value2"] = value2;

        REQUIRE(*run<bool>("return wrap1 <= value2"));
        REQUIRE(*run<bool>("return wrap1 >= value2"));
        t1.set_value(200);
        REQUIRE(*run<bool>("return wrap1 <= value2"));
        REQUIRE_FALSE(*run<bool>("return wrap1 >= value2"));
    }
    SUBCASE("LessThan")
    {
        i32             value1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(value1);

        i32 value2       = 8000;
        global["value2"] = value2;

        REQUIRE(*run<bool>("return wrap1 < value2"));
        REQUIRE_FALSE(*run<bool>("return wrap1 > value2"));
        t1.set_value(16000);
        REQUIRE_FALSE(*run<bool>("return wrap1 < value2"));
        REQUIRE(*run<bool>("return wrap1 > value2"));
    }
    SUBCASE("autogenerated equal")
    {
        i32             value1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(value1);

        i32             value2 = 4000;
        TestScriptClass t2;
        global["wrap2"] = &t2;
        t2.set_value(value2);

        REQUIRE(*run<bool>("return wrap1 == wrap2"));
        REQUIRE_FALSE(*run<bool>("return wrap1 ~= wrap2"));
        t1.set_value(8000);
        REQUIRE_FALSE(*run<bool>("return wrap1 == wrap2"));
        REQUIRE(*run<bool>("return wrap1 ~= wrap2"));
    }
    SUBCASE("Add,Subtract,Divide,Multiply")
    {
        i32             value1 = 4000;
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(value1);

        i32 value2       = 200;
        global["value2"] = value2;

        TestScriptClass* b = *run<TestScriptClass*>("return wrap1 + value2");
        REQUIRE(b->get_value() == value1 + value2);

        b = *run<TestScriptClass*>("return wrap1 - value2");
        REQUIRE(b->get_value() == value1 - value2);

        b = *run<TestScriptClass*>("return wrap1 / value2");
        REQUIRE(b->get_value() == value1 / value2);

        b = *run<TestScriptClass*>("return wrap1 * value2");
        REQUIRE(b->get_value() == value1 * value2);
    }
    SUBCASE("UnaryMinus")
    {
        TestScriptClass t1;
        global["wrap1"] = &t1;
        t1.set_value(100);

        i32 b = *run<i32>("return -wrap1");
        REQUIRE(b == -100);
    }
    SUBCASE("Close")
    {
        TestScriptClass t1;
        global["wrap1"] = &t1;

        REQUIRE(t1.Closed == false);
        auto res = run("local wrap <close> = wrap1");
        REQUIRE(res.has_value());
        REQUIRE(t1.Closed == true);
    }
    SUBCASE("GC")
    {
        REQUIRE(gc().is_running());

        auto* t1        = new TestScriptClass; // NOLINT
        global["wrap1"] = scripting::managed_ptr {t1};
        REQUIRE(TestScriptClass::ObjCount == 1);

        global["wrap1"] = nullptr;
        gc().collect();
        REQUIRE(TestScriptClass::ObjCount == 0);

        REQUIRE(gc().is_running());
        gc().stop();
        REQUIRE_FALSE(gc().is_running());
        gc().restart();
        REQUIRE(gc().is_running());
    }
}

TEST_CASE_FIXTURE(LuaWrapperTests, "Script.LuaWrapper.ChainFunctions")
{
    {
        native_closure_shared_ptr l;

        auto foo = [&l](TestScriptClass* instance1, i32 x) mutable {
            instance1->set_value(x * 10);
            auto lambda = [instance1](i32 y) {
                return instance1->get_value() + y;
            };

            l = make_shared_closure(std::function(lambda));
            return l.get();
        };

        auto wrapper = create_wrapper<TestScriptClass>("TSCB");
        wrapper->method("foo", foo);

        {
            TestScriptClass t;
            global["wrap"] = &t;
            auto ret       = run<i32>("return wrap:foo(4)(2)");
            REQUIRE(ret.has_value());
            REQUIRE(ret.value() == 42);
        }
    }
    {
        std::string text {};

        native_closure_shared_ptr l;

        auto text_adder = [&l, &text](std::string& y) {
            text += y;
            return l.get();
        };
        auto text_setter = [&l, &text](std::string& x) mutable {
            text = x;
            return l.get();
        };

        l = make_shared_closure(std::function(text_adder));

        auto wrapper = create_wrapper<TestScriptClass>("TSCB");
        wrapper->method("foo", text_setter);
        {
            TestScriptClass t;
            global["wrap"] = &t;
            auto ret       = run("wrap.foo 'hello' ' ' 'world' '!'");
            REQUIRE(ret.has_value());
            REQUIRE(text == "hello world!");
        }
    }
}

TEST_CASE_FIXTURE(LuaWrapperTests, "Script.LuaWrapper.UnknownHandler")
{
    struct foo {
        int  x {0};
        auto y() const -> int { return x * 100; }
        int  z {0};
    };

    auto wrap = create_wrapper<foo>("foo");

    SUBCASE("unhandled setter (newindex)")
    {
        auto res = run("function foo(p) p.unhandled_newindex=400 end");
        REQUIRE(res.has_value());
        auto f = global["foo"].as<function<void>>();
        foo  test {};
        REQUIRE_FALSE(f.protected_call(&test).has_value());
        REQUIRE_FALSE(test.z == 400);
    }
    SUBCASE("handled setter (newindex)")
    {
        auto res = run("function foo(p) p.z=400 end");
        REQUIRE(res.has_value());
        auto f = global["foo"].as<function<void>>();

        wrap->UnknownSet.connect([](auto&& ev) {
            if (ev.Name == "z") {
                ev.get_value(ev.Instance->z);
            }
        });

        foo test {};
        REQUIRE(f.protected_call(&test).has_value());
        REQUIRE(test.z == 400);
    }
    SUBCASE("unhandled getter (index)")
    {
        auto res = run("function foo(p) return p.unhandled_index end");
        REQUIRE(res.has_value());
        foo        test {};
        auto const funcres = global["foo"].as<function<i32>>().protected_call(&test);
        REQUIRE_FALSE(funcres.has_value());
    }
    SUBCASE("handled getter (index)")
    {
        auto res = run("function foo(p) return p.x end");
        REQUIRE(res.has_value());
        auto f = global["foo"].as<function<i32>>();

        wrap->UnknownGet.connect([](auto&& ev) {
            if (ev.Name == "x") {
                ev.return_value(ev.Instance->x);
            }
        });

        foo test {};
        test.x       = 420;
        auto funcres = f.protected_call(&test);
        REQUIRE(funcres.has_value());
        REQUIRE(funcres.value() == test.x);
    }
    SUBCASE("unhandled function (index)")
    {
        auto res = run("function foo(p) p.unhandled_index() end");
        REQUIRE(res.has_value());
        foo        test {};
        auto const funcres = global["foo"].as<function<i32>>().protected_call(&test);
        REQUIRE_FALSE(funcres.has_value());
    }
    SUBCASE("handled function (index)")
    {
        auto res = run("function foo(p) return p:y() end");
        REQUIRE(res.has_value());
        auto f = global["foo"].as<function<i32>>();

        auto yfunc = make_shared_closure(std::function([](foo* fx) { return fx->y(); }));
        wrap->UnknownGet.connect([yfunc](auto&& ev) {
            if (ev.Name == "y") {
                ev.return_value(yfunc.get());
            }
        });

        foo test {};
        test.x       = 40;
        auto funcres = f.protected_call(&test);
        REQUIRE(funcres.has_value());
        REQUIRE(funcres.value() == test.y());
    }
}

TEST_CASE_FIXTURE(LuaWrapperTests, "Script.LuaWrapper.DataObject")
{
    data::object obj;
    obj["a"] = 123.;
    obj["b"] = true;
    obj["c"] = "test123";
    obj["d"] = 456;

    auto wrap = create_wrapper<data::object>("object");

    SUBCASE("get")
    {
        wrap->UnknownGet.connect([](auto&& ev) {
            if (!ev.Instance) { return; }
            auto const proxy {(*ev.Instance)[ev.Name]};

            if (i64 ival {0}; proxy.try_get(ival)) {
                ev.return_value(ival);
            } else if (f64 fval {0}; proxy.try_get(fval)) {
                ev.return_value(fval);
            } else if (bool bval {}; proxy.try_get(bval)) {
                ev.return_value(bval);
            } else if (std::string sval {}; proxy.try_get(sval)) {
                ev.return_value(sval);
            }
        });

        global["wrap"] = &obj;

        auto const a = *run<f64>("return wrap.a");
        REQUIRE(a == obj["a"].as<f64>());

        auto const b = *run<bool>("return wrap.b");
        REQUIRE(b == obj["b"].as<bool>());

        auto const c = *run<std::string>("return wrap.c");
        REQUIRE(c == obj["c"].as<std::string>());

        auto const d = *run<i64>("return wrap.d");
        REQUIRE(d == obj["d"].as<i64>());
    }

    SUBCASE("set")
    {
        wrap->UnknownSet.connect([](auto&& ev) {
            if (!ev.Instance) { return; }

            if (i64 ival {0}; ev.get_value(ival)) {
                (*ev.Instance)[ev.Name] = ival;
            } else if (f64 fval {0}; ev.get_value(fval)) {
                (*ev.Instance)[ev.Name] = fval;
            } else if (bool bval {}; ev.get_value(bval)) {
                (*ev.Instance)[ev.Name] = bval;
            } else if (std::string sval {}; ev.get_value(sval)) {
                (*ev.Instance)[ev.Name] = sval;
            }
        });

        global["wrap"] = &obj;

        auto const res = run(
            R"( wrap.a = 20.5 
                wrap.b = 'ok'  
                wrap.c = false 
                wrap.x = 12345)");
        REQUIRE(res);

        REQUIRE(obj["a"].as<f64>() == 20.5);
        REQUIRE(obj["b"].as<std::string>() == "ok");
        REQUIRE(obj["c"].as<bool>() == false);
        REQUIRE(obj["x"].as<i64>() == 12345);
    }
}

TEST_CASE_FIXTURE(LuaWrapperTests, "Script.LuaWrapper.Null")
{
    std::function is_null = [](void* ptr) { return ptr == nullptr; };
    global["is_null"]     = &is_null;

    Player* p {nullptr};
    global["dave"] = p;
    auto res {run<bool>("return is_null(dave)")};
    REQUIRE(res.has_value());
    REQUIRE(res.value());
}
