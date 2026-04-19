#include "tests.hpp"

TEST_CASE("AI.FSM.AddState")
{
    SUBCASE("state can be added")
    {
        fsm f;
        f.add_state({.ID = 1});
        REQUIRE(f.current_state() == INVALID_ID);
    }
}

TEST_CASE("AI.FSM.StartStop")
{
    SUBCASE("start enters initial state")
    {
        fsm f;
        f.add_state({.ID = 1});
        f.start(1, {});
        REQUIRE(f.current_state() == 1);
        REQUIRE(f.is_running());
    }

    SUBCASE("start fires StateChanged")
    {
        fsm f;
        f.add_state({.ID = 1});
        fsm::transition_event ev {};
        f.StateChanged.connect([&](auto const& e) { ev = e; });
        f.start(1, {});
        REQUIRE(ev.From == INVALID_ID);
        REQUIRE(ev.To == 1);
    }

    SUBCASE("start calls OnEnter")
    {
        fsm  f;
        bool entered {false};
        f.add_state({.ID = 1, .OnEnter = [&](user_object&) { entered = true; }});
        f.start(1, {});
        REQUIRE(entered);
    }

    SUBCASE("start is no-op if already running")
    {
        fsm f;
        f.add_state({.ID = 1});
        f.add_state({.ID = 2});
        f.start(1, {});
        f.start(2, {});
        REQUIRE(f.current_state() == 1);
    }

    SUBCASE("stop exits current state")
    {
        fsm  f;
        bool exited {false};
        f.add_state({.ID = 1, .OnExit = [&](user_object&) { exited = true; }});
        f.start(1, {});
        f.stop();
        REQUIRE(exited);
        REQUIRE_FALSE(f.is_running());
        REQUIRE(f.current_state() == INVALID_ID);
    }

    SUBCASE("stop fires StateChanged")
    {
        fsm f;
        f.add_state({.ID = 1});
        f.start(1, {});
        fsm::transition_event ev {};
        f.StateChanged.connect([&](auto const& e) { ev = e; });
        f.stop();
        REQUIRE(ev.From == 1);
        REQUIRE(ev.To == INVALID_ID);
    }

    SUBCASE("stop sets previous state")
    {
        fsm f;
        f.add_state({.ID = 1});
        f.start(1, {});
        f.stop();
        REQUIRE(f.previous_state() == 1);
    }

    SUBCASE("stop is no-op if not running")
    {
        fsm f;
        f.add_state({.ID = 1});
        bool exited {false};
        f.add_state({.ID = 2, .OnExit = [&](user_object&) { exited = true; }});
        f.stop();
        REQUIRE_FALSE(exited);
    }
}

TEST_CASE("AI.FSM.Update")
{
    SUBCASE("update calls OnUpdate")
    {
        fsm  f;
        bool ticked {false};
        f.add_state({.ID = 1, .OnUpdate = [&](user_object&, milliseconds) { ticked = true; }});
        f.start(1, {});
        f.update(milliseconds {16});
        REQUIRE(ticked);
    }

    SUBCASE("update accumulates time in state")
    {
        fsm f;
        f.add_state({.ID = 1});
        f.start(1, {});
        f.update(milliseconds {100});
        f.update(milliseconds {200});
        REQUIRE(f.time_in_state() == milliseconds {300});
    }

    SUBCASE("update is no-op if not running")
    {
        fsm  f;
        bool ticked {false};
        f.add_state({.ID = 1, .OnUpdate = [&](user_object&, milliseconds) { ticked = true; }});
        f.update(milliseconds {16});
        REQUIRE_FALSE(ticked);
    }
}

TEST_CASE("AI.FSM.Transitions")
{
    SUBCASE("condition transition fires")
    {
        fsm f;
        f.add_state({.ID = 1, .Transitions = {{.TargetStateID = 2, .Condition = [](user_object const&) { return true; }}}});
        f.add_state({.ID = 2});
        f.start(1, {});
        f.update(milliseconds {16});
        REQUIRE(f.current_state() == 2);
    }

    SUBCASE("condition transition does not fire when false")
    {
        fsm f;
        f.add_state({.ID = 1, .Transitions = {{.TargetStateID = 2, .Condition = [](user_object const&) { return false; }}}});
        f.add_state({.ID = 2});
        f.start(1, {});
        f.update(milliseconds {16});
        REQUIRE(f.current_state() == 1);
    }

    SUBCASE("transition fires OnExit on source state")
    {
        fsm  f;
        bool exited {false};
        f.add_state({.ID          = 1,
                     .OnExit      = [&](user_object&) { exited = true; },
                     .Transitions = {{.TargetStateID = 2, .Condition = [](user_object const&) { return true; }}}});
        f.add_state({.ID = 2});
        f.start(1, {});
        f.update(milliseconds {16});
        REQUIRE(exited);
    }

    SUBCASE("transition fires OnEnter on target state")
    {
        fsm  f;
        bool entered {false};
        f.add_state({.ID = 1, .Transitions = {{.TargetStateID = 2, .Condition = [](user_object const&) { return true; }}}});
        f.add_state({.ID = 2, .OnEnter = [&](user_object&) { entered = true; }});
        f.start(1, {});
        f.update(milliseconds {16});
        REQUIRE(entered);
    }

    SUBCASE("transition fires OnTransition")
    {
        fsm  f;
        bool transitioned {false};
        f.add_state({.ID = 1, .Transitions = {{.TargetStateID = 2, .Condition = [](user_object const&) { return true; }, .OnTransition = [&](user_object&) { transitioned = true; }}}});
        f.add_state({.ID = 2});
        f.start(1, {});
        f.update(milliseconds {16});
        REQUIRE(transitioned);
    }

    SUBCASE("transition fires StateChanged")
    {
        fsm f;
        f.add_state({.ID = 1, .Transitions = {{.TargetStateID = 2, .Condition = [](user_object const&) { return true; }}}});
        f.add_state({.ID = 2});
        fsm::transition_event ev {};
        f.StateChanged.connect([&](auto const& e) { ev = e; });
        f.start(1, {});
        f.update(milliseconds {16});
        REQUIRE(ev.From == 1);
        REQUIRE(ev.To == 2);
    }

    SUBCASE("transition sets previous state")
    {
        fsm f;
        f.add_state({.ID = 1, .Transitions = {{.TargetStateID = 2, .Condition = [](user_object const&) { return true; }}}});
        f.add_state({.ID = 2});
        f.start(1, {});
        f.update(milliseconds {16});
        REQUIRE(f.previous_state() == 1);
    }

    SUBCASE("first matching transition wins")
    {
        fsm f;
        f.add_state({.ID = 1, .Transitions = {{.TargetStateID = 2, .Condition = [](user_object const&) { return true; }}, {.TargetStateID = 3, .Condition = [](user_object const&) { return true; }}}});
        f.add_state({.ID = 2});
        f.add_state({.ID = 3});
        f.start(1, {});
        f.update(milliseconds {16});
        REQUIRE(f.current_state() == 2);
    }

    SUBCASE("self transition is ignored")
    {
        fsm  f;
        bool exited {false};
        f.add_state({.ID          = 1,
                     .OnExit      = [&](user_object&) { exited = true; },
                     .Transitions = {{.TargetStateID = 1, .Condition = [](user_object const&) { return true; }}}});
        f.start(1, {});
        f.update(milliseconds {16});
        REQUIRE(f.current_state() == 1);
        REQUIRE_FALSE(exited);
    }

    SUBCASE("time in state resets on transition")
    {
        fsm f;
        f.add_state({.ID = 1, .Transitions = {{.TargetStateID = 2, .Condition = [](user_object const&) { return true; }}}});
        f.add_state({.ID = 2});
        f.start(1, {});
        f.update(milliseconds {100});
        f.update(milliseconds {16});
        REQUIRE(f.time_in_state() == milliseconds {16});
    }
}

TEST_CASE("AI.FSM.Timeout")
{
    SUBCASE("timeout transition fires after elapsed time")
    {
        fsm f;
        f.add_state({.ID = 1, .Transitions = {{.TargetStateID = 2, .Timeout = milliseconds {100}}}});
        f.add_state({.ID = 2});
        f.start(1, {});
        f.update(milliseconds {50});
        REQUIRE(f.current_state() == 1);
        f.update(milliseconds {60});
        REQUIRE(f.current_state() == 2);
    }

    SUBCASE("timeout does not fire before elapsed")
    {
        fsm f;
        f.add_state({.ID = 1, .Transitions = {{.TargetStateID = 2, .Timeout = milliseconds {200}}}});
        f.add_state({.ID = 2});
        f.start(1, {});
        f.update(milliseconds {100});
        REQUIRE(f.current_state() == 1);
    }

    SUBCASE("timeout resets on state entry")
    {
        fsm f;
        f.add_state({.ID = 1, .Transitions = {{.TargetStateID = 2, .Timeout = milliseconds {100}}}});
        f.add_state({.ID = 2, .Transitions = {{.TargetStateID = 1, .Condition = [](user_object const&) { return true; }}}});
        f.start(1, {});
        f.update(milliseconds {60});
        f.update(milliseconds {60}); // transitions to 2
        REQUIRE(f.current_state() == 2);
        f.update(milliseconds {16}); // back to 1
        REQUIRE(f.current_state() == 1);
        REQUIRE(f.time_in_state() == milliseconds {0});
    }
}

TEST_CASE("AI.FSM.GlobalTransitions")
{
    SUBCASE("global transition fires from any state")
    {
        fsm f;
        f.add_state({.ID = 1});
        f.add_state({.ID = 2});
        f.add_state({.ID = 3});
        f.add_global_transition({.TargetStateID = 3, .Condition = [](user_object const&) { return true; }});
        f.start(1, {});
        f.update(milliseconds {16});
        REQUIRE(f.current_state() == 3);
    }

    SUBCASE("global transition takes priority over state transition")
    {
        fsm f;
        f.add_state({.ID = 1, .Transitions = {{.TargetStateID = 2, .Condition = [](user_object const&) { return true; }}}});
        f.add_state({.ID = 2});
        f.add_state({.ID = 3});
        f.add_global_transition({.TargetStateID = 3, .Condition = [](user_object const&) { return true; }});
        f.start(1, {});
        f.update(milliseconds {16});
        REQUIRE(f.current_state() == 3);
    }

    SUBCASE("global transition does not fire when condition false")
    {
        fsm f;
        f.add_state({.ID = 1});
        f.add_state({.ID = 2});
        f.add_global_transition({.TargetStateID = 2, .Condition = [](user_object const&) { return false; }});
        f.start(1, {});
        f.update(milliseconds {16});
        REQUIRE(f.current_state() == 1);
    }
}

TEST_CASE("AI.FSM.ForceState")
{
    SUBCASE("force_state transitions immediately")
    {
        fsm f;
        f.add_state({.ID = 1});
        f.add_state({.ID = 2});
        f.start(1, {});
        f.force_state(2);
        REQUIRE(f.current_state() == 2);
    }

    SUBCASE("force_state fires OnExit and OnEnter")
    {
        fsm  f;
        bool exited {false};
        bool entered {false};
        f.add_state({.ID = 1, .OnExit = [&](user_object&) { exited = true; }});
        f.add_state({.ID = 2, .OnEnter = [&](user_object&) { entered = true; }});
        f.start(1, {});
        f.force_state(2);
        REQUIRE(exited);
        REQUIRE(entered);
    }

    SUBCASE("force_state fires StateChanged")
    {
        fsm f;
        f.add_state({.ID = 1});
        f.add_state({.ID = 2});
        fsm::transition_event ev {};
        f.StateChanged.connect([&](auto const& e) { ev = e; });
        f.start(1, {});
        f.force_state(2);
        REQUIRE(ev.From == 1);
        REQUIRE(ev.To == 2);
    }

    SUBCASE("force_state sets previous state")
    {
        fsm f;
        f.add_state({.ID = 1});
        f.add_state({.ID = 2});
        f.start(1, {});
        f.force_state(2);
        REQUIRE(f.previous_state() == 1);
    }
}

TEST_CASE("AI.FSM.Data")
{
    SUBCASE("data is accessible in OnEnter")
    {
        fsm f;
        i32 seen {0};
        f.add_state({.ID = 1, .OnEnter = [&](user_object& data) { seen = *data.get<i32>(); }});
        f.start(1, user_object {42});
        REQUIRE(seen == 42);
    }

    SUBCASE("data can be mutated in OnUpdate")
    {
        fsm f;
        f.add_state({.ID = 1, .OnUpdate = [](user_object& data, milliseconds) {
                         (*data.get<i32>())++;
                     }});
        f.start(1, user_object {0});
        f.update(milliseconds {16});
        REQUIRE(*f.data<i32>() == 1);
    }

    SUBCASE("data persists across transitions")
    {
        fsm f;
        f.add_state({.ID = 1, .Transitions = {{.TargetStateID = 2, .Condition = [](user_object const&) { return true; }}}});
        f.add_state({.ID = 2});
        f.start(1, user_object {99});
        f.update(milliseconds {16});
        REQUIRE(f.current_state() == 2);
        REQUIRE(*f.data<i32>() == 99);
    }
}
