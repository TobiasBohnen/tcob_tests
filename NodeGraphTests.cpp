#include "tests.hpp"

TEST_CASE("Core.NodeGraph.CreateNode")
{
    SUBCASE("returns valid id")
    {
        node_graph g;
        uid const  id {g.create_node({.Title = "A"})};
        REQUIRE(id != uid {});
    }

    SUBCASE("node appears in nodes()")
    {
        node_graph g;
        uid const  id {g.create_node({.Title = "A"})};
        auto const nodes {g.nodes()};
        REQUIRE(std::ranges::any_of(nodes, [id](auto const& n) { return n.ID == id; }));
    }

    SUBCASE("multiple nodes have unique ids")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A"})};
        uid const  b {g.create_node({.Title = "B"})};
        uid const  c {g.create_node({.Title = "C"})};
        REQUIRE(a != b);
        REQUIRE(b != c);
        REQUIRE(a != c);
    }
}

TEST_CASE("Core.NodeGraph.RemoveNode")
{
    SUBCASE("returns true on valid id")
    {
        node_graph g;
        uid const  id {g.create_node({.Title = "A"})};
        REQUIRE(g.remove_node(id));
    }

    SUBCASE("returns false on invalid id")
    {
        node_graph g;
        REQUIRE_FALSE(g.remove_node(uid {9999}));
    }

    SUBCASE("node no longer in nodes()")
    {
        node_graph g;
        uid const  id {g.create_node({.Title = "A"})};
        g.remove_node(id);
        REQUIRE(std::ranges::none_of(g.nodes(), [id](auto const& n) { return n.ID == id; }));
    }

    SUBCASE("removes connected connections")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        g.create_connection(a, 1, b, 1);
        REQUIRE(g.connections().size() == 1);
        g.remove_node(a);
        REQUIRE(g.connections().empty());
    }

    SUBCASE("removing input node removes connection")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        g.create_connection(a, 1, b, 1);
        g.remove_node(b);
        REQUIRE(g.connections().empty());
    }
}

TEST_CASE("Core.NodeGraph.CanConnect")
{
    SUBCASE("valid connection")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        REQUIRE(g.can_connect(a, 1, b, 1));
    }

    SUBCASE("self connection rejected")
    {
        node_graph g;
        uid const  a {g.create_node({.Title   = "A",
                                     .Inputs  = {{.ID = 1, .Name = "In"}},
                                     .Outputs = {{.ID = 2, .Name = "Out"}}})};
        REQUIRE_FALSE(g.can_connect(a, 2, a, 1));
    }

    SUBCASE("invalid node id rejected")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        REQUIRE_FALSE(g.can_connect(a, 1, uid {9999}, 1));
    }

    SUBCASE("invalid port id rejected")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        REQUIRE_FALSE(g.can_connect(a, 99, b, 1));
        REQUIRE_FALSE(g.can_connect(a, 1, b, 99));
    }

    SUBCASE("type mismatch rejected")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out", .Type = 0b0001}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In", .Type = 0b0010}}})};
        REQUIRE_FALSE(g.can_connect(a, 1, b, 1));
    }

    SUBCASE("type overlap accepted")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out", .Type = 0b0011}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In", .Type = 0b0001}}})};
        REQUIRE(g.can_connect(a, 1, b, 1));
    }

    SUBCASE("duplicate input port rejected")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  c {g.create_node({.Title = "C", .Inputs = {{.ID = 1, .Name = "In"}}})};
        g.create_connection(a, 1, c, 1);
        REQUIRE_FALSE(g.can_connect(b, 1, c, 1));
    }

    SUBCASE("cycle rejected")
    {
        node_graph g;
        uid const  a {g.create_node({.Title   = "A",
                                     .Inputs  = {{.ID = 1, .Name = "In"}},
                                     .Outputs = {{.ID = 2, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title   = "B",
                                     .Inputs  = {{.ID = 1, .Name = "In"}},
                                     .Outputs = {{.ID = 2, .Name = "Out"}}})};
        g.create_connection(a, 2, b, 1);
        REQUIRE_FALSE(g.can_connect(b, 2, a, 1));
    }

    SUBCASE("longer cycle rejected")
    {
        node_graph g;
        uid const  a {g.create_node({.Title   = "A",
                                     .Inputs  = {{.ID = 1, .Name = "In"}},
                                     .Outputs = {{.ID = 2, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title   = "B",
                                     .Inputs  = {{.ID = 1, .Name = "In"}},
                                     .Outputs = {{.ID = 2, .Name = "Out"}}})};
        uid const  c {g.create_node({.Title   = "C",
                                     .Inputs  = {{.ID = 1, .Name = "In"}},
                                     .Outputs = {{.ID = 2, .Name = "Out"}}})};
        g.create_connection(a, 2, b, 1);
        g.create_connection(b, 2, c, 1);
        REQUIRE_FALSE(g.can_connect(c, 2, a, 1));
    }
}

TEST_CASE("Core.NodeGraph.CreateConnection")
{
    SUBCASE("returns valid id")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        auto const id {g.create_connection(a, 1, b, 1)};
        REQUIRE(id.has_value());
    }

    SUBCASE("returns nullopt on invalid connection")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out", .Type = 0b0001}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In", .Type = 0b0010}}})};
        REQUIRE_FALSE(g.create_connection(a, 1, b, 1).has_value());
    }

    SUBCASE("connection appears in connections()")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        auto const id {g.create_connection(a, 1, b, 1)};
        REQUIRE(std::ranges::any_of(g.connections(), [&](auto const& c) { return c.ID == *id; }));
    }
}

TEST_CASE("Core.NodeGraph.RemoveConnection")
{
    SUBCASE("returns true on valid id")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        auto const id {g.create_connection(a, 1, b, 1)};
        REQUIRE(g.remove_connection(*id));
    }

    SUBCASE("returns false on invalid id")
    {
        node_graph g;
        REQUIRE_FALSE(g.remove_connection(uid {9999}));
    }

    SUBCASE("connection no longer in connections()")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        auto const id {g.create_connection(a, 1, b, 1)};
        g.remove_connection(*id);
        REQUIRE(g.connections().empty());
    }
}

TEST_CASE("Core.NodeGraph.Evaluate")
{
    using out_map = std::unordered_map<uid, node_value_types>;

    SUBCASE("single source node")
    {
        node_graph g;
        uid const  src {g.create_node({.Title      = "Float",
                                       .Outputs    = {{.ID = 1, .Name = "Value"}},
                                       .Parameters = {node_param_float {.Name = "Value", .Value = 2.5f}},
                                       .Compute    = [](auto const& /*in*/, auto const& vals) -> out_map {
                                          return {{1, std::get<f32>(vals[0])}};
                                       }})};
        uid const  sink {g.create_node({.Title  = "Sink",
                                        .Inputs = {{.ID = 1, .Name = "In"}}})};

        g.create_connection(src, 1, sink, 1);

        node_value_types result {};
        g.evaluate(sink, [&](auto const& in, auto const& /*vals*/) -> out_map {
            result = in[0];
            return {};
        });
        REQUIRE(std::get<f32>(result) == 2.5f);
    }

    SUBCASE("chained nodes")
    {
        node_graph g;
        uid const  src {g.create_node({.Title      = "Float",
                                       .Outputs    = {{.ID = 1, .Name = "Value"}},
                                       .Parameters = {node_param_float {.Name = "Value", .Value = 3.0f}},
                                       .Compute    = [](auto const& /*in*/, auto const& vals) -> out_map {
                                          return {{1, std::get<f32>(vals[0])}};
                                       }})};
        uid const  mul {g.create_node({.Title   = "Double",
                                       .Inputs  = {{.ID = 1, .Name = "In"}},
                                       .Outputs = {{.ID = 2, .Name = "Out"}},
                                       .Compute = [](auto const& in, auto const& /*vals*/) -> out_map {
                                          f32 const v {std::get_if<f32>(&in[0]) ? std::get<f32>(in[0]) : 0.0f};
                                          return {{2, v * 2.0f}};
                                       }})};
        uid const  sink {g.create_node({.Title  = "Sink",
                                        .Inputs = {{.ID = 1, .Name = "In"}}})};

        g.create_connection(src, 1, mul, 1);
        g.create_connection(mul, 2, sink, 1);

        node_value_types result {};
        g.evaluate(sink, [&](auto const& in, auto const& /*vals*/) -> out_map {
            result = in[0];
            return {};
        });
        REQUIRE(std::get<f32>(result) == 6.0f);
    }

    SUBCASE("unconnected input is default")
    {
        node_graph g;
        uid const  sink {g.create_node({.Title  = "Sink",
                                        .Inputs = {{.ID = 1, .Name = "In"}}})};

        node_value_types result {1.0f};
        g.evaluate(sink, [&](auto const& in, auto const& /*vals*/) -> out_map {
            result = in[0];
            return {};
        });
        REQUIRE(std::get<f32>(result) == 0.0f);
    }

    SUBCASE("multiple outputs evaluated once")
    {
        i32        computeCount {0};
        node_graph g;
        uid const  src {g.create_node({.Title   = "Src",
                                       .Outputs = {{.ID = 1, .Name = "A"},
                                                   {.ID = 2, .Name = "B"}},
                                       .Compute = [&](auto const& /*in*/, auto const& /*vals*/) -> out_map {
                                          ++computeCount;
                                          return {{1, 1.0f}, {2, 2.0f}};
                                       }})};
        uid const  sink {g.create_node({.Title  = "Sink",
                                        .Inputs = {{.ID = 1, .Name = "A"},
                                                   {.ID = 2, .Name = "B"}}})};

        g.create_connection(src, 1, sink, 1);
        g.create_connection(src, 2, sink, 2);

        g.evaluate(sink, [](auto const& /*in*/, auto const& /*vals*/) -> out_map { return {}; });
        REQUIRE(computeCount == 1);
    }

    SUBCASE("multiple outputs correct values")
    {
        node_graph g;
        uid const  src {g.create_node({.Title   = "Src",
                                       .Outputs = {{.ID = 1, .Name = "A"},
                                                   {.ID = 2, .Name = "B"}},
                                       .Compute = [](auto const& /*in*/, auto const& /*vals*/) -> out_map {
                                          return {{1, 10.0f}, {2, 20.0f}};
                                       }})};
        uid const  sink {g.create_node({.Title  = "Sink",
                                        .Inputs = {{.ID = 1, .Name = "A"},
                                                   {.ID = 2, .Name = "B"}}})};

        g.create_connection(src, 1, sink, 1);
        g.create_connection(src, 2, sink, 2);

        f32 a {}, b {};
        g.evaluate(sink, [&](auto const& in, auto const& /*vals*/) -> out_map {
            a = std::get<f32>(in[0]);
            b = std::get<f32>(in[1]);
            return {};
        });
        REQUIRE(a == 10.0f);
        REQUIRE(b == 20.0f);
    }

    SUBCASE("invalid node id is no-op")
    {
        node_graph g;
        bool       called {false};
        g.evaluate(uid {9999}, [&](auto const& /*in*/, auto const& /*vals*/) -> out_map {
            called = true;
            return {};
        });
        REQUIRE_FALSE(called);
    }

    SUBCASE("sink params passed to eval fn")
    {
        node_graph g;
        uid const  sink {g.create_node({.Title      = "Sink",
                                        .Parameters = {node_param_float {.Name = "Threshold", .Value = 0.75f}}})};

        f32 param {};
        g.evaluate(sink, [&](auto const& /*in*/, auto const& vals) -> out_map {
            param = std::get<f32>(vals[0]);
            return {};
        });
        REQUIRE(param == 0.75f);
    }
}

TEST_CASE("Core.NodeGraph.MutateParam")
{
    using out_map = std::unordered_map<uid, node_value_types>;

    SUBCASE("mutate float value")
    {
        node_graph g;
        uid const  id {g.create_node({.Title      = "A",
                                      .Parameters = {node_param_float {.Name = "V", .Value = 1.0f, .Min = 0.0f, .Max = 5.0f, .Step = 1.0f}}})};

        g.mutate_param(id, 0, [](auto& p) {
            std::get<node_param_float>(p).Value = 3.0f;
            return true;
        });

        node_value_types result {};
        g.evaluate(id, [&](auto const& /*in*/, auto const& vals) -> out_map {
            result = vals[0];
            return {};
        });
        REQUIRE(std::get<f32>(result) == 3.0f);
    }

    SUBCASE("returns false on invalid node")
    {
        node_graph g;
        REQUIRE_FALSE(g.mutate_param(uid {9999}, 0, [](auto&) { return true; }));
    }

    SUBCASE("returns false on invalid param index")
    {
        node_graph g;
        uid const  id {g.create_node({.Title = "A"})};
        REQUIRE_FALSE(g.mutate_param(id, 0, [](auto&) { return true; }));
    }
}

TEST_CASE("Core.NodeGraph.Signals")
{
    SUBCASE("NodeAdded fires on create_node")
    {
        node_graph g;
        uid        firedID {};
        g.NodeAdded.connect([&](uid id) { firedID = id; });
        uid const id {g.create_node({.Title = "A"})};
        REQUIRE(firedID == id);
    }

    SUBCASE("NodeRemoved fires on remove_node")
    {
        node_graph g;
        uid        firedID {};
        g.NodeRemoved.connect([&](uid id) { firedID = id; });
        uid const id {g.create_node({.Title = "A"})};
        g.remove_node(id);
        REQUIRE(firedID == id);
    }

    SUBCASE("ConnectionAdded fires on create_connection")
    {
        node_graph g;
        uid        firedID {};
        g.ConnectionAdded.connect([&](uid id) { firedID = id; });
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        auto const id {g.create_connection(a, 1, b, 1)};
        REQUIRE(firedID == *id);
    }

    SUBCASE("ConnectionRemoved fires on remove_connection")
    {
        node_graph g;
        uid        firedID {};
        g.ConnectionRemoved.connect([&](uid id) { firedID = id; });
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        auto const id {g.create_connection(a, 1, b, 1)};
        g.remove_connection(*id);
        REQUIRE(firedID == *id);
    }

    SUBCASE("Changed fires on create_node")
    {
        node_graph g;
        i32        count {0};
        g.Changed.connect([&] { ++count; });
        g.create_node({.Title = "A"});
        REQUIRE(count == 1);
    }

    SUBCASE("Changed fires on remove_node")
    {
        node_graph g;
        uid const  id {g.create_node({.Title = "A"})};
        i32        count {0};
        g.Changed.connect([&] { ++count; });
        g.remove_node(id);
        REQUIRE(count == 1);
    }

    SUBCASE("Changed does not fire on failed remove_node")
    {
        node_graph g;
        i32        count {0};
        g.Changed.connect([&] { ++count; });
        g.remove_node(uid {9999});
        REQUIRE(count == 0);
    }
}
