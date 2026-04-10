#include "tests.hpp"

TEST_CASE("Core.NodeGraph.Empty")
{
    node_graph g;
    REQUIRE(g.nodes().empty());
    REQUIRE(g.connections().empty());
}

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
        REQUIRE(std::ranges::any_of(g.nodes(), [id](auto const& n) { return n.ID == id; }));
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

    SUBCASE("removes all connections from output node")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        uid const  c {g.create_node({.Title = "C", .Inputs = {{.ID = 1, .Name = "In"}}})};
        g.create_connection(a, 1, b, 1);
        g.create_connection(a, 1, c, 1);
        REQUIRE(g.connections().size() == 2);
        g.remove_node(a);
        REQUIRE(g.connections().empty());
    }

    SUBCASE("removes all connections from input node")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  c {g.create_node({.Title  = "C",
                                     .Inputs = {{.ID = 1, .Name = "In1"}, {.ID = 2, .Name = "In2"}}})};
        g.create_connection(a, 1, c, 1);
        g.create_connection(b, 1, c, 2);
        REQUIRE(g.connections().size() == 2);
        g.remove_node(c);
        REQUIRE(g.connections().empty());
    }

    SUBCASE("only removes connections of removed node")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title   = "B",
                                     .Inputs  = {{.ID = 1, .Name = "In"}},
                                     .Outputs = {{.ID = 2, .Name = "Out"}}})};
        uid const  c {g.create_node({.Title = "C", .Inputs = {{.ID = 1, .Name = "In"}}})};
        g.create_connection(a, 1, b, 1);
        g.create_connection(b, 2, c, 1);
        g.remove_node(a);
        REQUIRE(g.connections().size() == 1);
        REQUIRE(g.connections()[0].OutputNodeID == b);
        REQUIRE(g.connections()[0].InputNodeID == c);
    }

    SUBCASE("NodeRemoved does not fire on invalid id")
    {
        node_graph g;
        bool       fired {false};
        g.NodeRemoved.connect([&](uid) { fired = true; });
        g.remove_node(uid {9999});
        REQUIRE_FALSE(fired);
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

    SUBCASE("invalid output node rejected")
    {
        node_graph g;
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        REQUIRE_FALSE(g.can_connect(uid {9999}, 1, b, 1));
    }

    SUBCASE("invalid input node rejected")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        REQUIRE_FALSE(g.can_connect(a, 1, uid {9999}, 1));
    }

    SUBCASE("invalid output port rejected")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        REQUIRE_FALSE(g.can_connect(a, 99, b, 1));
    }

    SUBCASE("invalid input port rejected")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
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

    SUBCASE("default type accepts all")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out", .Type = 0b0001}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        REQUIRE(g.can_connect(a, 1, b, 1));
    }

    SUBCASE("duplicate input port rejected")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  c {g.create_node({.Title = "C", .Inputs = {{.ID = 1, .Name = "In"}}})};
        REQUIRE(g.create_connection(a, 1, c, 1));
        REQUIRE_FALSE(g.create_connection(b, 1, c, 1));
    }

    SUBCASE("fan-out accepted")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        uid const  c {g.create_node({.Title = "C", .Inputs = {{.ID = 1, .Name = "In"}}})};
        REQUIRE(g.create_connection(a, 1, b, 1));
        REQUIRE(g.can_connect(a, 1, c, 1));
    }

    SUBCASE("direct cycle rejected")
    {
        node_graph g;
        uid const  a {g.create_node({.Title   = "A",
                                     .Inputs  = {{.ID = 1, .Name = "In"}},
                                     .Outputs = {{.ID = 2, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title   = "B",
                                     .Inputs  = {{.ID = 1, .Name = "In"}},
                                     .Outputs = {{.ID = 2, .Name = "Out"}}})};
        REQUIRE(g.create_connection(a, 2, b, 1));
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
        REQUIRE(g.create_connection(a, 2, b, 1));
        REQUIRE(g.create_connection(b, 2, c, 1));
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

    SUBCASE("returns nullopt on type mismatch")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out", .Type = 0b0001}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In", .Type = 0b0010}}})};
        REQUIRE_FALSE(g.create_connection(a, 1, b, 1).has_value());
    }

    SUBCASE("returns nullopt on duplicate input")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  c {g.create_node({.Title = "C", .Inputs = {{.ID = 1, .Name = "In"}}})};
        REQUIRE(g.create_connection(a, 1, c, 1));
        REQUIRE_FALSE(g.create_connection(b, 1, c, 1).has_value());
    }

    SUBCASE("connection appears in connections()")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        auto const id {g.create_connection(a, 1, b, 1)};
        REQUIRE(std::ranges::any_of(g.connections(), [&](auto const& c) { return c.ID == *id; }));
    }

    SUBCASE("fan-out creates two connections")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        uid const  c {g.create_node({.Title = "C", .Inputs = {{.ID = 1, .Name = "In"}}})};
        g.create_connection(a, 1, b, 1);
        g.create_connection(a, 1, c, 1);
        REQUIRE(g.connections().size() == 2);
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

    SUBCASE("port can be reconnected after removal")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  c {g.create_node({.Title = "C", .Inputs = {{.ID = 1, .Name = "In"}}})};
        auto const id {g.create_connection(a, 1, c, 1)};
        g.remove_connection(*id);
        REQUIRE(g.create_connection(b, 1, c, 1).has_value());
    }

    SUBCASE("only removes specified connection")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        uid const  c {g.create_node({.Title = "C", .Inputs = {{.ID = 1, .Name = "In"}}})};
        auto const id0 {g.create_connection(a, 1, b, 1)};
        auto const id1 {g.create_connection(a, 1, c, 1)};
        g.remove_connection(*id0);
        REQUIRE(g.connections().size() == 1);
        REQUIRE(g.connections()[0].ID == *id1);
    }
}

TEST_CASE("Core.NodeGraph.Evaluate")
{
    SUBCASE("single source node")
    {
        node_graph g;
        uid const  src {g.create_node({.Title      = "Float",
                                       .Outputs    = {{.ID = 1, .Name = "Value"}},
                                       .Parameters = {node_param_float {.Name = "Value", .Value = 2.5f}},
                                       .Compute    = [](auto const& /*in*/, auto const& vals) -> node_compute_result {
                                          return {{1, std::get<f32>(vals[0])}};
                                       }})};
        uid const  sink {g.create_node({.Title = "Sink", .Inputs = {{.ID = 1, .Name = "In"}}})};
        REQUIRE(g.create_connection(src, 1, sink, 1));

        node_value_types result {};
        g.evaluate(sink, [&](auto const& in, auto const& /*vals*/) -> node_compute_result {
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
                                       .Compute    = [](auto const& /*in*/, auto const& vals) -> node_compute_result {
                                          return {{1, std::get<f32>(vals[0])}};
                                       }})};
        uid const  mul {g.create_node({.Title   = "Double",
                                       .Inputs  = {{.ID = 1, .Name = "In"}},
                                       .Outputs = {{.ID = 2, .Name = "Out"}},
                                       .Compute = [](auto const& in, auto const& /*vals*/) -> node_compute_result {
                                          auto const* v {std::get_if<f32>(&in[0])};
                                          return {{2, v ? *v * 2.0f : 0.0f}};
                                       }})};
        uid const  sink {g.create_node({.Title = "Sink", .Inputs = {{.ID = 1, .Name = "In"}}})};
        REQUIRE(g.create_connection(src, 1, mul, 1));
        REQUIRE(g.create_connection(mul, 2, sink, 1));

        node_value_types result {};
        g.evaluate(sink, [&](auto const& in, auto const& /*vals*/) -> node_compute_result {
            result = in[0];
            return {};
        });
        REQUIRE(std::get<f32>(result) == 6.0f);
    }

    SUBCASE("unconnected input is default")
    {
        node_graph g;
        uid const  sink {g.create_node({.Title = "Sink", .Inputs = {{.ID = 1, .Name = "In"}}})};

        node_value_types result {1.0f};
        g.evaluate(sink, [&](auto const& in, auto const& /*vals*/) -> node_compute_result {
            result = in[0];
            return {};
        });
        REQUIRE(std::get<f32>(result) == 0.0f);
    }

    SUBCASE("node with no Compute returns 0 for outputs")
    {
        node_graph g;
        uid const  src {g.create_node({.Title = "Src", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  sink {g.create_node({.Title = "Sink", .Inputs = {{.ID = 1, .Name = "In"}}})};
        REQUIRE(g.create_connection(src, 1, sink, 1));

        node_value_types result {1.0f};
        g.evaluate(sink, [&](auto const& in, auto const& /*vals*/) -> node_compute_result {
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
                                       .Outputs = {{.ID = 1, .Name = "A"}, {.ID = 2, .Name = "B"}},
                                       .Compute = [&](auto const& /*in*/, auto const& /*vals*/) -> node_compute_result {
                                          ++computeCount;
                                          return {{1, 1.0f}, {2, 2.0f}};
                                       }})};
        uid const  sink {g.create_node({.Title  = "Sink",
                                        .Inputs = {{.ID = 1, .Name = "A"}, {.ID = 2, .Name = "B"}}})};
        REQUIRE(g.create_connection(src, 1, sink, 1));
        REQUIRE(g.create_connection(src, 2, sink, 2));

        g.evaluate(sink, [](auto const& /*in*/, auto const& /*vals*/) -> node_compute_result { return {}; });
        REQUIRE(computeCount == 1);
    }

    SUBCASE("multiple outputs correct values")
    {
        node_graph g;
        uid const  src {g.create_node({.Title   = "Src",
                                       .Outputs = {{.ID = 1, .Name = "A"}, {.ID = 2, .Name = "B"}},
                                       .Compute = [](auto const& /*in*/, auto const& /*vals*/) -> node_compute_result {
                                          return {{1, 10.0f}, {2, 20.0f}};
                                       }})};
        uid const  sink {g.create_node({.Title  = "Sink",
                                        .Inputs = {{.ID = 1, .Name = "A"}, {.ID = 2, .Name = "B"}}})};
        REQUIRE(g.create_connection(src, 1, sink, 1));
        REQUIRE(g.create_connection(src, 2, sink, 2));

        f32 a {}, b {};
        g.evaluate(sink, [&](auto const& in, auto const& /*vals*/) -> node_compute_result {
            a = std::get<f32>(in[0]);
            b = std::get<f32>(in[1]);
            return {};
        });
        REQUIRE(a == 10.0f);
        REQUIRE(b == 20.0f);
    }

    SUBCASE("diamond dependency - upstream node computed once")
    {
        i32        computeCount {0};
        node_graph g;
        uid const  src {g.create_node({.Title   = "Src",
                                       .Outputs = {{.ID = 1, .Name = "Out"}},
                                       .Compute = [&](auto const& /*in*/, auto const& /*vals*/) -> node_compute_result {
                                          ++computeCount;
                                          return {{1, 5.0f}};
                                       }})};
        uid const  left {g.create_node({.Title   = "Left",
                                        .Inputs  = {{.ID = 1, .Name = "In"}},
                                        .Outputs = {{.ID = 2, .Name = "Out"}},
                                        .Compute = [](auto const& in, auto const& /*vals*/) -> node_compute_result {
                                           auto const* v {std::get_if<f32>(&in[0])};
                                           return {{2, v ? *v + 1.0f : 0.0f}};
                                        }})};
        uid const  right {g.create_node({.Title   = "Right",
                                         .Inputs  = {{.ID = 1, .Name = "In"}},
                                         .Outputs = {{.ID = 2, .Name = "Out"}},
                                         .Compute = [](auto const& in, auto const& /*vals*/) -> node_compute_result {
                                            auto const* v {std::get_if<f32>(&in[0])};
                                            return {{2, v ? *v + 2.0f : 0.0f}};
                                         }})};
        uid const  sink {g.create_node({.Title  = "Sink",
                                        .Inputs = {{.ID = 1, .Name = "L"}, {.ID = 2, .Name = "R"}}})};
        REQUIRE(g.create_connection(src, 1, left, 1));
        REQUIRE(g.create_connection(src, 1, right, 1));
        REQUIRE(g.create_connection(left, 2, sink, 1));
        REQUIRE(g.create_connection(right, 2, sink, 2));

        f32 l {}, r {};
        g.evaluate(sink, [&](auto const& in, auto const& /*vals*/) -> node_compute_result {
            l = std::get<f32>(in[0]);
            r = std::get<f32>(in[1]);
            return {};
        });
        REQUIRE(computeCount == 1);
        REQUIRE(l == 6.0f);
        REQUIRE(r == 7.0f);
    }

    SUBCASE("large fan-in correct values")
    {
        node_graph g;
        auto const makeSrc {[&](f32 val, uid outPort) -> uid {
            return g.create_node({.Title      = "Src",
                                  .Outputs    = {{.ID = outPort, .Name = "Out"}},
                                  .Parameters = {node_param_float {.Name = "V", .Value = val}},
                                  .Compute    = [outPort](auto const& /*in*/, auto const& vals) -> node_compute_result {
                                      return {{outPort, std::get<f32>(vals[0])}};
                                  }});
        }};

        uid const s0 {makeSrc(1.0f, 1)};
        uid const s1 {makeSrc(2.0f, 1)};
        uid const s2 {makeSrc(3.0f, 1)};
        uid const sink {g.create_node({.Title  = "Sink",
                                       .Inputs = {{.ID = 1, .Name = "A"}, {.ID = 2, .Name = "B"}, {.ID = 3, .Name = "C"}}})};
        REQUIRE(g.create_connection(s0, 1, sink, 1));
        REQUIRE(g.create_connection(s1, 1, sink, 2));
        REQUIRE(g.create_connection(s2, 1, sink, 3));

        f32 a {}, b {}, c {};
        g.evaluate(sink, [&](auto const& in, auto const& /*vals*/) -> node_compute_result {
            a = std::get<f32>(in[0]);
            b = std::get<f32>(in[1]);
            c = std::get<f32>(in[2]);
            return {};
        });
        REQUIRE(a == 1.0f);
        REQUIRE(b == 2.0f);
        REQUIRE(c == 3.0f);
    }

    SUBCASE("sink params passed to eval fn")
    {
        node_graph g;
        uid const  sink {g.create_node({.Title      = "Sink",
                                        .Parameters = {node_param_float {.Name = "Threshold", .Value = 0.75f}}})};

        f32 param {};
        g.evaluate(sink, [&](auto const& /*in*/, auto const& vals) -> node_compute_result {
            param = std::get<f32>(vals[0]);
            return {};
        });
        REQUIRE(param == 0.75f);
    }

    SUBCASE("multiple params correct indexing")
    {
        node_graph g;
        uid const  sink {g.create_node({.Title      = "Sink",
                                        .Parameters = {
                                            node_param_float {.Name = "A", .Value = 1.0f},
                                            node_param_float {.Name = "B", .Value = 2.0f},
                                            node_param_float {.Name = "C", .Value = 3.0f}}})};

        std::vector<f32> params;
        g.evaluate(sink, [&](auto const& /*in*/, auto const& vals) -> node_compute_result {
            for (auto const& v : vals) { params.push_back(std::get<f32>(v)); }
            return {};
        });
        REQUIRE(params.size() == 3);
        REQUIRE(params[0] == 1.0f);
        REQUIRE(params[1] == 2.0f);
        REQUIRE(params[2] == 3.0f);
    }

    SUBCASE("invalid node id is no-op")
    {
        node_graph g;
        bool       called {false};
        g.evaluate(uid {9999}, [&](auto const& /*in*/, auto const& /*vals*/) -> node_compute_result {
            called = true;
            return {};
        });
        REQUIRE_FALSE(called);
    }
}

TEST_CASE("Core.NodeGraph.MutateParam")
{
    SUBCASE("mutate float value")
    {
        node_graph g;
        uid const  id {g.create_node({.Title      = "A",
                                      .Parameters = {node_param_float {.Name = "V", .Value = 1.0f, .Min = 0.0f, .Max = 5.0f, .Step = 1.0f}}})};
        g.mutate_param(id, 0, [](auto& p) {
            std::get<node_param_float>(p).Value = 3.0f;
            return true;
        });

        f32 result {};
        g.evaluate(id, [&](auto const& /*in*/, auto const& vals) -> node_compute_result {
            result = std::get<f32>(vals[0]);
            return {};
        });
        REQUIRE(result == 3.0f);
    }

    SUBCASE("mutate int value")
    {
        node_graph g;
        uid const  id {g.create_node({.Title      = "A",
                                      .Parameters = {node_param_int {.Name = "V", .Value = 0, .Min = -10, .Max = 10, .Step = 1}}})};
        g.mutate_param(id, 0, [](auto& p) {
            std::get<node_param_int>(p).Value = 7;
            return true;
        });

        i32 result {};
        g.evaluate(id, [&](auto const& /*in*/, auto const& vals) -> node_compute_result {
            result = std::get<i32>(vals[0]);
            return {};
        });
        REQUIRE(result == 7);
    }

    SUBCASE("mutate bool value")
    {
        node_graph g;
        uid const  id {g.create_node({.Title      = "A",
                                      .Parameters = {node_param_bool {.Name = "V", .Value = false}}})};
        g.mutate_param(id, 0, [](auto& p) {
            std::get<node_param_bool>(p).Value = true;
            return true;
        });

        bool result {};
        g.evaluate(id, [&](auto const& /*in*/, auto const& vals) -> node_compute_result {
            result = std::get<bool>(vals[0]);
            return {};
        });
        REQUIRE(result == true);
    }

    SUBCASE("fn returning false does not fire Changed")
    {
        node_graph g;
        uid const  id {g.create_node({.Title      = "A",
                                      .Parameters = {node_param_float {.Name = "V", .Value = 1.0f}}})};
        i32        count {0};
        g.Changed.connect([&] { ++count; });
        g.mutate_param(id, 0, [](auto&) { return false; });
        REQUIRE(count == 0);
    }

    SUBCASE("fn returning false leaves value unchanged")
    {
        node_graph g;
        uid const  id {g.create_node({.Title      = "A",
                                      .Parameters = {node_param_float {.Name = "V", .Value = 1.0f}}})};
        g.mutate_param(id, 0, [](auto& p) {
            std::get<node_param_float>(p).Value = 99.0f;
            return false;
        });

        f32 result {};
        g.evaluate(id, [&](auto const& /*in*/, auto const& vals) -> node_compute_result {
            result = std::get<f32>(vals[0]);
            return {};
        });
        REQUIRE(result == 1.0f);
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

    SUBCASE("NodeRemoved does not fire on invalid id")
    {
        node_graph g;
        bool       fired {false};
        g.NodeRemoved.connect([&](uid) { fired = true; });
        g.remove_node(uid {9999});
        REQUIRE_FALSE(fired);
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

    SUBCASE("ConnectionAdded does not fire on invalid connection")
    {
        node_graph g;
        bool       fired {false};
        g.ConnectionAdded.connect([&](uid) { fired = true; });
        uid const a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out", .Type = 0b0001}}})};
        uid const b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In", .Type = 0b0010}}})};
        REQUIRE_FALSE(g.create_connection(a, 1, b, 1));
        REQUIRE_FALSE(fired);
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

    SUBCASE("Changed fires on create_connection")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        i32        count {0};
        g.Changed.connect([&] { ++count; });
        g.create_connection(a, 1, b, 1);
        REQUIRE(count == 1);
    }

    SUBCASE("Changed does not fire on failed create_connection")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out", .Type = 0b0001}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In", .Type = 0b0010}}})};
        i32        count {0};
        g.Changed.connect([&] { ++count; });
        g.create_connection(a, 1, b, 1);
        REQUIRE(count == 0);
    }

    SUBCASE("Changed fires on remove_connection")
    {
        node_graph g;
        uid const  a {g.create_node({.Title = "A", .Outputs = {{.ID = 1, .Name = "Out"}}})};
        uid const  b {g.create_node({.Title = "B", .Inputs = {{.ID = 1, .Name = "In"}}})};
        auto const id {g.create_connection(a, 1, b, 1)};
        i32        count {0};
        g.Changed.connect([&] { ++count; });
        g.remove_connection(*id);
        REQUIRE(count == 1);
    }

    SUBCASE("Changed fires on mutate_param when fn returns true")
    {
        node_graph g;
        uid const  id {g.create_node({.Title      = "A",
                                      .Parameters = {node_param_float {.Name = "V", .Value = 1.0f}}})};
        i32        count {0};
        g.Changed.connect([&] { ++count; });
        g.mutate_param(id, 0, [](auto&) { return true; });
        REQUIRE(count == 1);
    }

    SUBCASE("Changed does not fire on mutate_param when fn returns false")
    {
        node_graph g;
        uid const  id {g.create_node({.Title      = "A",
                                      .Parameters = {node_param_float {.Name = "V", .Value = 1.0f}}})};
        i32        count {0};
        g.Changed.connect([&] { ++count; });
        g.mutate_param(id, 0, [](auto&) { return false; });
        REQUIRE(count == 0);
    }
}
