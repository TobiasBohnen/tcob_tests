#include "tests.hpp"

i32 const ExpValue = 45234;

static i32 Value {0};

static void Test(i32 val)
{
    Value = val;
}

TEST_CASE("Core.Signal.Connect")
{
    SUBCASE("Member Function")
    {
        struct SignalTest {
            void test(i32 val)
            {
                Value = val;
            }
        };
        Value = 0;
        signal<i32 const> sig0;
        SignalTest        inst;
        REQUIRE(Value != ExpValue);
        sig0.connect<&SignalTest::test>(&inst);
        sig0(ExpValue);
        REQUIRE(Value == ExpValue);
        sig0(200);
        REQUIRE(Value == 200);
    }
    SUBCASE("Free Function")
    {
        Value = 0;
        signal<i32 const> sig0;
        REQUIRE(Value != ExpValue);
        sig0.connect(&Test);
        sig0(ExpValue);
        REQUIRE(Value == ExpValue);
    }
    SUBCASE("+=")
    {
        Value = 0;
        signal<i32 const> sig0;
        REQUIRE(Value != ExpValue);
        sig0 += &Test;
        sig0(ExpValue);
        REQUIRE(Value == ExpValue);
    }
    SUBCASE("Lambda")
    {
        {
            signal<i32 const> sig0;
            i32               value {0};
            REQUIRE(value != ExpValue);
            sig0.connect([&value](i32 val) { value = val; });
            sig0(ExpValue);
            REQUIRE(value == ExpValue);
        }
        {
            signal<i32 const> sig0;
            i32               value {0};
            REQUIRE(value != ExpValue);
            sig0.connect([&value] { value = ExpValue * 10; });
            sig0(ExpValue);
            REQUIRE(value == ExpValue * 10);
        }
    }

    SUBCASE("Stateful lambda")
    {
        struct Event {
            i32 Value {0};
        };

        signal<Event> sig0;
        i32           result {0};

        sig0.connect([&result, counter = 0](Event& ev) mutable {
            counter++;
            result = ev.Value * counter;
        });

        Event ev1 {10};
        sig0(ev1);
        REQUIRE(result == 10); // 10 * 1

        Event ev2 {10};
        sig0(ev2);
        REQUIRE(result == 20); // 10 * 2

        Event ev3 {10};
        sig0(ev3);
        REQUIRE(result == 30); // 10 * 3
    }
}

TEST_CASE("Core.Signal.Disconnect")
{
    {
        signal<> sig0;
        REQUIRE(sig0.slot_count() == 0);
        auto id0 = sig0.connect([] { }).id();
        auto id1 = sig0.connect([] { }).id();
        REQUIRE(sig0.slot_count() == 2);
        sig0.disconnect(id0);
        REQUIRE(sig0.slot_count() == 1);
        sig0.disconnect(id1);
        REQUIRE(sig0.slot_count() == 0);
    }
    {
        signal<> sig0;
        REQUIRE(sig0.slot_count() == 0);
        auto id0 = sig0.connect([] { }).id();
        auto id1 = sig0.connect([] { }).id();
        REQUIRE(sig0.slot_count() == 2);
        sig0.disconnect(id1);
        REQUIRE(sig0.slot_count() == 1);
        sig0.disconnect(id0);
        REQUIRE(sig0.slot_count() == 0);
    }
    {
        signal<> sig0;
        REQUIRE(sig0.slot_count() == 0);
        auto c0  = sig0 += [] { };
        auto id0 = (sig0 += [] { }).id();
        REQUIRE(sig0.slot_count() == 2);
        sig0 -= id0;
        REQUIRE(sig0.slot_count() == 1);
        sig0 -= c0;
        REQUIRE(sig0.slot_count() == 0);
    }
}

TEST_CASE("Core.Signal.Lifetime")
{
    SUBCASE("Scoped Connection")
    {
        signal<> sig0;
        REQUIRE(sig0.slot_count() == 0);
        {
            scoped_connection conn {sig0.connect([] { })};
            REQUIRE(sig0.slot_count() == 1);
        }
        REQUIRE(sig0.slot_count() == 0);
    }

    SUBCASE("Connection")
    {
        signal<> sig0;
        REQUIRE(sig0.slot_count() == 0);
        {
            std::function<void()> l {};
            connection            conn {sig0.connect(l)};
            REQUIRE(sig0.slot_count() == 1);
        }

        REQUIRE(sig0.slot_count() == 1);
        sig0();
        REQUIRE(sig0.slot_count() == 0);
    }
}

TEST_CASE("Core.Signal.Handled")
{
    struct event_base {
        bool Handled {false};
    };

    {
        signal<event_base> sig0;
        event_base         ev;
        sig0.connect([](auto&& arg) { arg.Handled = true; });
        sig0(ev);
        REQUIRE(ev.Handled);
    }
    {
        signal<event_base> sig0;
        event_base         ev;
        i32                callCount {0};
        sig0.connect([&](auto&&) { callCount++; });
        sig0.connect([&](auto&& arg) { callCount++; arg.Handled = true; });
        sig0.connect([&](auto&&) { callCount++; });
        sig0.connect([&](auto&&) { callCount++; });
        sig0.connect([&](auto&&) { callCount++; });
        sig0(ev);
        REQUIRE(ev.Handled);
        REQUIRE(callCount == 2);
    }
}

TEST_CASE("Core.Signal.ChangeEvArgs")
{
    struct event_base {
        i32 Value {0};
    };

    {
        signal<event_base> sig0;
        event_base         ev;
        sig0.connect([](auto&& arg) { arg.Value = 100; });
        REQUIRE(ev.Value == 0);
        sig0(ev);
        REQUIRE(ev.Value == 100);
    }
}

TEST_CASE("Core.Signal.NestedSubscription")
{
    SUBCASE("Connect during emission")
    {
        signal<> sig0;
        i32      callCount {0};

        sig0.connect([&sig0, &callCount] {
            callCount++;
            sig0.connect([&callCount] {
                callCount += 10;
            });
        });

        sig0.connect([&callCount] {
            callCount += 100;
        });

        sig0();
        REQUIRE(callCount > 0);
        REQUIRE(sig0.slot_count() == 3);
    }

    SUBCASE("Disconnect during emission")
    {
        signal<>          sig0;
        i32               callCount {0};
        scoped_connection c2;

        sig0.connect([&c2, &callCount] {
            callCount++;
            c2.disconnect();
        });

        c2 = sig0.connect([&callCount] {
            callCount += 10;
        });

        sig0.connect([&callCount] {
            callCount += 100;
        });

        sig0();
        callCount = 0;
        sig0();
        REQUIRE(callCount == 101);
    }

    SUBCASE("Recursive signal emission")
    {
        signal<> sig0;
        i32      callCount {0};
        i32      maxDepth {0};
        i32      currentDepth {0};

        sig0.connect([&sig0, &callCount, &currentDepth, &maxDepth] {
            callCount++;
            currentDepth++;
            maxDepth = std::max(maxDepth, currentDepth);

            if (currentDepth < 3) {
                sig0();
            }

            currentDepth--;
        });

        sig0();

        REQUIRE(callCount == 3);
        REQUIRE(maxDepth == 3);
    }
}
