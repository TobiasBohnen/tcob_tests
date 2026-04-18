#include "tests.hpp"

TEST_CASE_TEMPLATE("Core.OrderedMap.Basic", map_t, // NOLINT
                   ordered_map<i32, i32>,
                   ordered_map<i32, string>,
                   ordered_map<string, i32>,
                   ordered_map<string, string>)
{
    map_t m;

    auto make_key = [](i32 i) {
        if constexpr (std::is_same_v<typename map_t::container::value_type::first_type, i32>) {
            return i;
        } else {
            return std::to_string(i);
        }
    };
    auto make_val = [](i32 i) {
        if constexpr (std::is_same_v<typename map_t::container::value_type::second_type, i32>) {
            return i;
        } else {
            return std::to_string(i);
        }
    };

    SUBCASE("empty on construction")
    {
        REQUIRE(m.empty());
        REQUIRE(m.size() == 0);
    }

    SUBCASE("set and get")
    {
        m.set(make_key(1), make_val(10));
        REQUIRE(m.get(make_key(1)) != nullptr);
        REQUIRE(*m.get(make_key(1)) == make_val(10));
        REQUIRE(m.get(make_key(99)) == nullptr);
    }

    SUBCASE("set overwrites existing key")
    {
        m.set(make_key(1), make_val(10));
        m.set(make_key(1), make_val(20));
        REQUIRE(m.size() == 1);
        REQUIRE(*m.get(make_key(1)) == make_val(20));
    }

    SUBCASE("contains")
    {
        m.set(make_key(1), make_val(10));
        REQUIRE(m.contains(make_key(1)));
        REQUIRE_FALSE(m.contains(make_key(99)));
    }

    SUBCASE("insertion order preserved")
    {
        m.set(make_key(3), make_val(30));
        m.set(make_key(1), make_val(10));
        m.set(make_key(2), make_val(20));

        auto it = m.begin();
        REQUIRE(it->first == make_key(3));
        ++it;
        REQUIRE(it->first == make_key(1));
        ++it;
        REQUIRE(it->first == make_key(2));
    }

    SUBCASE("erase removes key")
    {
        m.set(make_key(1), make_val(10));
        m.set(make_key(2), make_val(20));
        m.erase(make_key(1));
        REQUIRE_FALSE(m.contains(make_key(1)));
        REQUIRE(m.size() == 1);
    }

    SUBCASE("erase preserves insertion order of remaining keys")
    {
        m.set(make_key(1), make_val(10));
        m.set(make_key(2), make_val(20));
        m.set(make_key(3), make_val(30));
        m.erase(make_key(2));

        auto it = m.begin();
        REQUIRE(it->first == make_key(1));
        ++it;
        REQUIRE(it->first == make_key(3));
    }

    SUBCASE("erase first key")
    {
        m.set(make_key(1), make_val(10));
        m.set(make_key(2), make_val(20));
        m.set(make_key(3), make_val(30));
        m.erase(make_key(1));

        auto it = m.begin();
        REQUIRE(it->first == make_key(2));
        ++it;
        REQUIRE(it->first == make_key(3));
    }

    SUBCASE("erase last key")
    {
        m.set(make_key(1), make_val(10));
        m.set(make_key(2), make_val(20));
        m.set(make_key(3), make_val(30));
        m.erase(make_key(3));

        auto it = m.begin();
        REQUIRE(it->first == make_key(1));
        ++it;
        REQUIRE(it->first == make_key(2));
        REQUIRE(m.size() == 2);
    }

    SUBCASE("erase non-existent key is no-op")
    {
        m.set(make_key(1), make_val(10));
        m.erase(make_key(99));
        REQUIRE(m.size() == 1);
    }

    SUBCASE("find returns correct iterator")
    {
        m.set(make_key(1), make_val(10));
        m.set(make_key(2), make_val(20));
        auto it = m.find(make_key(1));
        REQUIRE(it != m.end());
        REQUIRE(it->second == make_val(10));
    }

    SUBCASE("find returns end for missing key")
    {
        REQUIRE(m.find(make_key(99)) == m.end());
    }

    SUBCASE("clear empties the map")
    {
        m.set(make_key(1), make_val(10));
        m.set(make_key(2), make_val(20));
        m.clear();
        REQUIRE(m.empty());
        REQUIRE(m.size() == 0);
    }

    SUBCASE("re-insert after erase restores key at end")
    {
        m.set(make_key(1), make_val(10));
        m.set(make_key(2), make_val(20));
        m.erase(make_key(1));
        m.set(make_key(1), make_val(99));

        auto it = m.begin();
        REQUIRE(it->first == make_key(2));
        ++it;
        REQUIRE(it->first == make_key(1));
        REQUIRE(it->second == make_val(99));
    }

    SUBCASE("reserve does not affect behaviour")
    {
        m.reserve(64);
        m.set(make_key(1), make_val(10));
        REQUIRE(m.size() == 1);
        REQUIRE(*m.get(make_key(1)) == make_val(10));
    }
}