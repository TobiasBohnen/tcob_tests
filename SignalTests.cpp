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
        signal<i32 const, void> sig0;
        SignalTest              inst;
        REQUIRE(Value != ExpValue);
        sig0.connect<&SignalTest::test>(&inst);
        emit_signal(sig0, ExpValue);
        REQUIRE(Value == ExpValue);
        emit_signal(sig0, 200);
        REQUIRE(Value == 200);
    }
    SUBCASE("Free Function")
    {
        Value = 0;
        signal<i32 const, void> sig0;
        REQUIRE(Value != ExpValue);
        sig0.connect(&Test);
        emit_signal(sig0, ExpValue);
        REQUIRE(Value == ExpValue);
    }
    SUBCASE("+=")
    {
        Value = 0;
        signal<i32 const, void> sig0;
        REQUIRE(Value != ExpValue);
        sig0 += &Test;
        emit_signal(sig0, ExpValue);
        REQUIRE(Value == ExpValue);
    }
    SUBCASE("Lambda")
    {
        {
            signal<i32 const, void> sig0;
            i32                     value {0};
            REQUIRE(value != ExpValue);
            sig0.connect([&value](i32 val) { value = val; });
            emit_signal(sig0, ExpValue);
            REQUIRE(value == ExpValue);
        }
        {
            signal<i32 const, void> sig0;
            i32                     value {0};
            REQUIRE(value != ExpValue);
            sig0.connect([&value] { value = ExpValue * 10; });
            emit_signal(sig0, ExpValue);
            REQUIRE(value == ExpValue * 10);
        }
    }

    SUBCASE("Stateful lambda")
    {
        struct Event {
            i32 Value {0};
        };

        signal<Event, void> sig0;
        i32                 result {0};

        sig0.connect([&result, counter = 0](Event& ev) mutable {
            counter++;
            result = ev.Value * counter;
        });

        Event ev1 {10};
        emit_signal(sig0, ev1);
        REQUIRE(result == 10); // 10 * 1

        Event ev2 {10};
        emit_signal(sig0, ev2);
        REQUIRE(result == 20); // 10 * 2

        Event ev3 {10};
        emit_signal(sig0, ev3);
        REQUIRE(result == 30); // 10 * 3
    }
}

TEST_CASE("Core.Signal.Disconnect")
{
    {
        signal<void, void> sig0;
        auto               id0 = sig0.connect([] { }).id();
        auto               id1 = sig0.connect([] { }).id();
        REQUIRE(sig0.slot_count() == 2);
        sig0.disconnect(id0);
        REQUIRE(sig0.slot_count() == 1);
        sig0.disconnect(id1);
        REQUIRE(sig0.slot_count() == 0);
    }
    {
        signal<void, void> sig0;
        auto               id0 = sig0.connect([] { }).id();
        auto               id1 = sig0.connect([] { }).id();
        REQUIRE(sig0.slot_count() == 2);
        sig0.disconnect(id1);
        REQUIRE(sig0.slot_count() == 1);
        sig0.disconnect(id0);
        REQUIRE(sig0.slot_count() == 0);
    }
    {
        signal<void, void> sig0;
        auto               c0  = sig0 += [] { };
        auto               id0 = (sig0 += [] { }).id();
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
        signal<void, void> sig0;
        REQUIRE(sig0.slot_count() == 0);
        {
            scoped_connection conn {sig0.connect([] { })};
            REQUIRE(sig0.slot_count() == 1);
        }
        REQUIRE(sig0.slot_count() == 0);
    }
}

TEST_CASE("Core.Signal.Handled")
{
    struct event_base {
        bool Handled {false};
    };

    {
        signal<event_base, void> sig0;
        event_base               ev;
        sig0.connect([](auto&& arg) { arg.Handled = true; });
        emit_signal(sig0, ev);
        REQUIRE(ev.Handled);
    }
    {
        signal<event_base, void> sig0;
        event_base               ev;
        i32                      callCount {0};
        sig0.connect([&](auto&&) { callCount++; });
        sig0.connect([&](auto&& arg) { callCount++; arg.Handled = true; });
        sig0.connect([&](auto&&) { callCount++; });
        sig0.connect([&](auto&&) { callCount++; });
        sig0.connect([&](auto&&) { callCount++; });
        emit_signal(sig0, ev);
        REQUIRE(ev.Handled);
        REQUIRE(callCount == 2);
    }
}

TEST_CASE("Core.Signal.ChangeEvArgs")
{
    struct event_base {
        i32 Value {0};
    };

    signal<event_base, void> sig0;
    event_base               ev;
    sig0.connect([](auto&& arg) { arg.Value = 100; });
    REQUIRE(ev.Value == 0);
    emit_signal(sig0, ev);
    REQUIRE(ev.Value == 100);
}

TEST_CASE("Core.Signal.NestedSubscription")
{
    SUBCASE("Connect during emission")
    {
        signal<void, void> sig0;
        i32                callCount {0};

        sig0.connect([&] {
            callCount++;
            sig0.connect([&] {
                callCount += 10;
            });
        });

        sig0.connect([&] {
            callCount += 100;
        });

        emit_signal(sig0);
        REQUIRE(callCount > 0);
        REQUIRE(sig0.slot_count() >= 2);
    }

    SUBCASE("Disconnect during emission")
    {
        signal<void, void> sig0;
        i32                callCount {0};
        scoped_connection  c2;

        sig0.connect([&] {
            callCount++;
            c2.disconnect();
        });

        c2 = sig0.connect([&] {
            callCount += 10;
        });

        sig0.connect([&] {
            callCount += 100;
        });

        emit_signal(sig0);
        callCount = 0;
        emit_signal(sig0);
        REQUIRE(callCount == 101);
    }

    SUBCASE("Recursive signal emission")
    {
        signal<void, void> sig0;
        i32                callCount {0};

        sig0.connect([&] {
            callCount++;
            if (callCount < 3) { emit_signal(sig0); }
        });

        emit_signal(sig0);

        REQUIRE(callCount == 3);
    }
}
