#pragma once
#include <utility>

#include "tests.hpp"

using namespace tcob::scripting;

class TestScriptClassBase {
public:
    virtual ~TestScriptClassBase() = default;
    auto baseMethod() const -> i32 { return _x; }
    auto virtual virtualMethod() -> i32 { return 1; }
    auto virtual abstractMethod() -> i32 = 0;

private:
    i32 _x {21};
};
class TestScriptClass : public TestScriptClassBase {
public:
    static inline i32 ObjCount;

    using value_type = i32;
    using size_type  = i32;
    auto operator[](i32) -> i32& { return _value; }

    TestScriptClass()
    {
        ObjCount++;
    }

    TestScriptClass(i32 age)
        : _value(age)
    {
        ObjCount++;
    }

    TestScriptClass(i32 age, f32 x)
        : _value(age * (i32)x)
    {
        ObjCount++;
    }

    ~TestScriptClass() override
    {
        ObjCount--;
    }

    void set_value(i32 x)
    {
        _value = x;
    }
    auto get_value() const -> i32
    {
        return _value;
    }

    i32       FieldValue {};
    prop<i32> PropertyValue;
    bool      Closed {false};

    auto get_map() -> std::map<std::string, i32>*
    {
        return &_testMap;
    }
    void ptr(TestScriptClass* c)
    {
        _value = c->_value;
    }
    auto foo(std::string const& test, i32 x, bool) -> i32
    {
        return (i32)test.length() * x;
    }
    void bar(bool, std::string const&, i32) { }

    auto add_value(i32 x) const -> i32
    {
        return _value + x;
    }
    auto overload() -> f32
    {
        return 25;
    }
    auto overload(i32 i, f32 f) -> f32
    {
        return f * static_cast<f32>(i) * 25;
    }
    auto overload(f32 f, i32 i) -> f32
    {
        return f * static_cast<f32>(i) * 3;
    }
    auto overload(i32 i, f32 f0, f32 f1) -> f32
    {
        return static_cast<f32>(i) * f0 * f1;
    }
    auto overload(f32 f, i32 i0, f32 f1) -> f32
    {
        return f * static_cast<f32>(i0) + f1;
    }
    auto overload(std::vector<f32> const& vec) -> f32
    {
        return static_cast<f32>(vec.size());
    }

    auto virtualMethod() -> i32 override { return 84; }
    auto abstractMethod() -> i32 override { return 42; }

    std::map<std::string, i32> _testMap;
    i32                        _value = 0;
};

template <typename Signature, typename T>
auto consteval resolve_overload(Signature T::*ptr)
{
    return ptr;
}

class TestScriptClassSub final : public TestScriptClass {
};

inline auto operator==(TestScriptClass const& left, TestScriptClass const& right) -> bool
{
    return (left._value == right._value);
}
inline auto testFuncWrapperObj(TestScriptClass const* tsc) -> i32
{
    return tsc->get_value();
}

struct op_test_class {
    static inline i32 ObjCount;
    op_test_class(i32 v)
        : value(v)
    {
        ObjCount++;
    }
    ~op_test_class()
    {
        ObjCount--;
    }

    i32 value {0};
};
inline auto operator+(op_test_class const& left, op_test_class const& right) -> op_test_class
{
    return {left.value + right.value};
}
inline auto operator-(op_test_class const& left, op_test_class const& right) -> op_test_class
{
    return {left.value - right.value};
}
inline auto operator/(op_test_class const& left, op_test_class const& right) -> op_test_class
{
    return {left.value / right.value};
}
inline auto operator*(op_test_class const& left, op_test_class const& right) -> op_test_class
{
    return {left.value * right.value};
}
inline auto operator-(op_test_class const& right) -> op_test_class
{
    return {-right.value};
}
inline auto operator==(op_test_class const& left, op_test_class const& right) -> bool
{
    return left.value == right.value;
}
inline auto operator<=>(op_test_class const& left, op_test_class const& right)
{
    return left.value <=> right.value;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class Player {
public:
    // Constructor
    Player(std::string name, i32 health, i32 mana)
        : _name(std::move(name))
        , _health(health)
        , _mana(mana)
    {
    }

    auto get_name() const -> std::string const&
    {
        return _name;
    }

    auto get_health() const -> i32
    {
        return _health;
    }

    void set_health(i32 amount)
    {
        _health = amount;
    }

    void take_damage(i32 damage)
    {
        _health -= damage;
    }

    auto get_mana() const -> i32
    {
        return _mana;
    }

    void set_mana(i32 amount)
    {
        _mana = amount;
    }

    void use_mana(i32 amount)
    {
        if (_mana >= amount) {
            _mana -= amount;
        }
    }

    auto get_experience() const -> i32
    {
        return _experience;
    }

    void gain_experience(i32 points)
    {
        _experience += points;
        if (_experience >= _level * 100) {
            _level++;
        }
    }

    void add_to_inventory(std::string const& item)
    {
        _inventory.emplace_back(item);
    }

    void add_to_inventory(i32 item)
    {
        _inventory.emplace_back(item);
    }

    auto get_inventory_size() const -> i32
    {
        return static_cast<i32>(_inventory.size());
    }

private:
    std::string                                 _name;
    i32                                         _health;
    i32                                         _mana;
    i32                                         _level {1};
    i32                                         _experience {0};
    std::vector<std::variant<std::string, i32>> _inventory;
};
