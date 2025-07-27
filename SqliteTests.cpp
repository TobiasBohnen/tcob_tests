#include "tests.hpp"

using namespace tcob::db;

struct foo {
    i32    ID;
    string Name;
    i32    Age;
    float  Height;
    bool   Alive;

    auto operator==(foo const& lhs) const -> bool
    {
        return ID == lhs.ID && Name == lhs.Name && Age == lhs.Age && Height == lhs.Height && Alive == lhs.Alive;
    }
};

TEST_CASE("Data.Sqlite.Select")
{
    string tableName {"testTable"};

    database db {database::OpenMemory()};
    auto     dbTable {db.create_table(tableName,
                                      int_column<primary_key> {.Name = "ID", .NotNull = true},
                                      text_column {.Name = "Name", .NotNull = true},
                                      int_column {.Name = "Age"},
                                      real_column {.Name = "Height"},
                                      int_column {.Name = "Alive"})};
    REQUIRE(dbTable);
    std::vector<std::tuple<i32, string, i32, f32, bool>> vec;
    for (i32 i {1}; i <= 100; ++i) {
        vec.emplace_back(i, std::to_string(i), i * 100, static_cast<f32>(i) * 1.5f, i % 2 == 0);
    }

    REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height", "Alive")(vec));

    SUBCASE("select")
    {
        {
            auto const rows {dbTable->select_from<i32, string, i32, f32, bool>()()};
            REQUIRE(rows.size() == 100);
            REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
            REQUIRE(rows[1] == std::tuple {2, "2", 200, 3.0f, true});
        }
        {
            auto const rows {dbTable->select_from<string, i32, f32>("Name", "Age", "Height")()};
            REQUIRE(rows.size() == 100);
            REQUIRE(rows[0] == std::tuple {"1", 100, 1.5f});
            REQUIRE(rows[1] == std::tuple {"2", 200, 3.0f});
        }
    }
    SUBCASE("where")
    {
        SUBCASE("=")
        {
            {
                auto const rows {dbTable->select_from<i32, string, i32, f32, bool>().where(equal {"Age", 100})()};
                REQUIRE(rows.size() == 1);
                REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
            }
            {
                auto const rows {dbTable->select_from<i32, string, i32, f32, bool>().where(equal {"Age"})(500)};
                REQUIRE(rows.size() == 1);
                REQUIRE(rows[0] == std::tuple {5, "5", 500, 7.5f, false});
            }
        }
        SUBCASE("<>")
        {
            auto const rows {dbTable->select_from<i32>("ID").where(not_equal {"ID", 1})()};
            REQUIRE(rows.size() == 99);
        }
        SUBCASE(">")
        {
            auto const rows {dbTable->select_from<i32>("ID").where(greater {"ID", 50})()};
            REQUIRE(rows.size() == 50);
        }
        SUBCASE(">=")
        {
            auto const rows {dbTable->select_from<i32>("ID").where(greater_equal {"ID", 50})()};
            REQUIRE(rows.size() == 51);
        }
        SUBCASE("<")
        {
            auto const rows {dbTable->select_from<i32>("ID").where(less {"ID", 50})()};
            REQUIRE(rows.size() == 49);
        }
        SUBCASE("<=")
        {
            auto const rows {dbTable->select_from<i32>("ID").where(less_equal {"ID", 50})()};
            REQUIRE(rows.size() == 50);
        }
        SUBCASE("AND/OR")
        {
            {
                auto const rows {dbTable->select_from<i32, bool>("Age", "Alive").where(greater {"ID", 5} && equal("Alive", true))()};
                REQUIRE(rows.size() == 48);
                for (auto const& row : rows) {
                    auto const [age, alive] {row};
                    REQUIRE(age > 500);
                    REQUIRE(alive);
                }
            }
            {
                auto const rows {dbTable->select_from<i32, bool>("Age", "Alive").where(greater {"ID", 5} || equal("Alive", true))()};
                REQUIRE(rows.size() == 97);
                for (auto const& row : rows) {
                    auto const [age, alive] {row};
                    REQUIRE((age > 500 || alive));
                }
            }
            {
                auto const rows {dbTable->select_from<i32, bool>("Age", "Alive").where((greater {"ID", 5} && equal("Alive", true)) || equal {"ID", 2})()};
                REQUIRE(rows.size() == 49);
                for (auto const& row : rows) {
                    auto const [age, alive] {row};
                    REQUIRE((age > 500 || age == 200));
                    REQUIRE(alive);
                }
            }
            {
                auto const rows {dbTable->select_from<i32, bool>("Age", "Alive").where(equal("Alive", true) && (greater {"ID", 5} || equal {"ID", 2}))()};
                REQUIRE(rows.size() == 49);
                for (auto const& row : rows) {
                    auto const [age, alive] {row};
                    REQUIRE((age > 500 || age == 200));
                    REQUIRE(alive);
                }
            }
            {
                auto const rows {dbTable->select_from<i32, bool>("Age", "Alive").where(equal("Alive", true) && !(greater {"ID", 5} || equal {"ID", 2}))()};
                REQUIRE(rows.size() == 1);
                REQUIRE(rows[0] == std::tuple {400, true});
            }
        }
        SUBCASE("LIKE")
        {
            {
                auto const rows {dbTable->select_from<string>("Name").where(like {"Name", "%0"})()};
                REQUIRE(rows.size() == 10);
                for (i32 i {1}; i <= 10; ++i) {
                    REQUIRE(rows[i - 1] == std::to_string(i * 10));
                }
            }
            {
                auto const rows {dbTable->select_from<string>("Name").where(like {"Name"})("%0")};
                REQUIRE(rows.size() == 10);
                for (i32 i {1}; i <= 10; ++i) {
                    REQUIRE(rows[i - 1] == std::to_string(i * 10));
                }
            }
        }
        SUBCASE("GLOB")
        {
            auto const rows {dbTable->select_from<i32, string, i32, f32, bool>().where(glob {"Name", "*0"})()};
            REQUIRE(rows.size() == 10);
            for (i32 i {1}; i <= 10; ++i) {
                auto const tup {std::tuple {i * 10, std::to_string(i * 10), i * 1000, static_cast<f32>(i) * 15.0f, true}};
                REQUIRE_MESSAGE(rows[i - 1] == tup, i);
            }
        }
        SUBCASE("IN")
        {
            std::set<i32> idsToMatch {3, 7, 42};
            {
                auto ids {dbTable->select_from<i32>("ID").where(in {"ID", 3, 7, 42})()};
                REQUIRE(ids.size() == idsToMatch.size());
                for (auto const& id : ids) {
                    REQUIRE(idsToMatch.contains(id));
                }
            }
            {
                auto ids {dbTable->select_from<i32>("ID").where(!in {"ID", 3, 7, 42})()};
                REQUIRE(ids.size() == 100 - idsToMatch.size());
                for (auto const& id : ids) {
                    REQUIRE_FALSE(idsToMatch.contains(id));
                }
            }
        }
        SUBCASE("BETWEEN")
        {
            {
                auto ids {dbTable->select_from<i32>("ID").where(between {"ID", 1, 5})()};
                REQUIRE(ids.size() == 5);
                for (i32 i {1}; i <= 5; ++i) { REQUIRE(ids[i - 1] == i); }
            }
            {
                auto ids {dbTable->select_from<i32>("ID").where(between {"ID"})(12, 16)};
                REQUIRE(ids.size() == 5);
                for (i32 i {0}; i < 5; ++i) { REQUIRE(ids[i] == i + 12); }
            }
            {
                auto ids {dbTable->select_from<i32>("ID").where(!between {"ID", 12, 16})()};
                REQUIRE(ids.size() == 95);
            }
        }
        SUBCASE("IS NULL")
        {
            REQUIRE(dbTable->insert_into("Name", "Age", "Height", "Alive")(std::tuple {"NULL", nullptr, nullptr, nullptr}));
            {
                auto ids {dbTable->select_from<i32>("ID").where(is_null {"Height"})()};
                REQUIRE(ids.size() == 1);
            }
            {
                auto ids {dbTable->select_from<i32>("ID").where(!is_null {"Height"})()};
                REQUIRE(ids.size() == 100);
            }
        }
    }
    SUBCASE("order_by")
    {
        SUBCASE("descending by Name")
        {
            auto const rows {dbTable->select_from<i32, string, i32, f32, bool>()
                                 .order_by(desc {"Name"})()};
            REQUIRE(rows.size() == 100);
            REQUIRE(rows[0] == std::tuple {99, "99", 9900, 148.5f, false});
            REQUIRE(rows[1] == std::tuple {98, "98", 9800, 147.0f, true});
            REQUIRE(rows[2] == std::tuple {97, "97", 9700, 145.5f, false});
        }
        SUBCASE("ascending by Name")
        {
            auto const rows {dbTable->select_from<i32, string, i32, f32, bool>()
                                 .order_by(asc {"Name"})()};
            REQUIRE(rows.size() == 100);
            REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
            REQUIRE(rows[1] == std::tuple {10, "10", 1000, 15.0f, true});
            REQUIRE(rows[2] == std::tuple {100, "100", 10000, 150.0f, true});
        }
        SUBCASE("ascending by Age")
        {
            auto const rows {dbTable->select_from<i32, string, i32, f32, bool>()
                                 .order_by(asc {"Age"})()};
            REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
            REQUIRE(rows[1] == std::tuple {2, "2", 200, 3.0f, true});
        }
        SUBCASE("ascending by 3")
        {
            auto const rows {dbTable->select_from<i32, string, i32, f32, bool>()
                                 .order_by(asc {3})()};
            REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
            REQUIRE(rows[1] == std::tuple {2, "2", 200, 3.0f, true});
        }
        SUBCASE("descending by Height")
        {
            auto const rows {dbTable->select_from<i32, string, i32, f32, bool>()
                                 .order_by(desc {"Height"})()};
            REQUIRE(rows[0] == std::tuple {100, "100", 10000, 150.0f, true});
            REQUIRE(rows[1] == std::tuple {99, "99", 9900, 148.5f, false});
        }
        SUBCASE("ascending by Alive")
        {
            auto const rows {dbTable->select_from<i32, string, i32, f32, bool>()
                                 .order_by(asc {"Alive"})()};
            for (usize i {0}; i < 50; ++i) {
                REQUIRE(std::get<4>(rows[i]) == false);
            }
            for (usize i {50}; i < 100; ++i) {
                REQUIRE(std::get<4>(rows[i]) == true);
            }
        }
        SUBCASE("multi-column: Alive asc, Age desc")
        {
            auto const rows {dbTable->select_from<i32, string, i32, f32, bool>()
                                 .order_by(asc {"Alive"},
                                           desc {"Age"})()};
            for (usize i {1}; i < rows.size(); ++i) {
                auto const& [idPrev, namePrev, agePrev, hPrev, alivePrev] {rows[i - 1]};
                auto const& [idCurr, nameCurr, ageCurr, hCurr, aliveCurr] {rows[i]};

                if (alivePrev == aliveCurr) {
                    REQUIRE(agePrev >= ageCurr);
                } else {
                    REQUIRE(alivePrev == false);
                    REQUIRE(aliveCurr == true);
                }
            }
        }
        SUBCASE("multi-column: 5 asc, 3 desc")
        {
            auto const rows {dbTable->select_from<i32, string, i32, f32, bool>()
                                 .order_by(asc {5}, desc {3})()};
            for (usize i {1}; i < rows.size(); ++i) {
                auto const& [idPrev, namePrev, agePrev, hPrev, alivePrev] {rows[i - 1]};
                auto const& [idCurr, nameCurr, ageCurr, hCurr, aliveCurr] {rows[i]};

                if (alivePrev == aliveCurr) {
                    REQUIRE(agePrev >= ageCurr);
                } else {
                    REQUIRE(alivePrev == false);
                    REQUIRE(aliveCurr == true);
                }
            }
        }
        SUBCASE("multi-column by string: Alive asc, Age desc")
        {
            auto const rows {dbTable->select_from<i32, string, i32, f32, bool>()
                                 .order_by("Alive ASC", "Age DESC")()};
            for (usize i {1}; i < rows.size(); ++i) {
                auto const& [idPrev, namePrev, agePrev, hPrev, alivePrev] {rows[i - 1]};
                auto const& [idCurr, nameCurr, ageCurr, hCurr, aliveCurr] {rows[i]};

                if (alivePrev == aliveCurr) {
                    REQUIRE(agePrev >= ageCurr);
                } else {
                    REQUIRE(alivePrev == false);
                    REQUIRE(aliveCurr == true);
                }
            }
        }
    }
    SUBCASE("limit")
    {
        auto const rows {dbTable->select_from<i32, string, i32, f32, bool>().limit(1)()};
        REQUIRE(rows.size() == 1);
        REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
    }
    SUBCASE("limit offset")
    {
        auto const rows {dbTable->select_from<i32, string, i32, f32, bool>("ID", "Name", "Age", "Height", "Alive").limit(1, 1)()};
        REQUIRE(rows.size() == 1);
        REQUIRE(rows[0] == std::tuple {2, "2", 200, 3.0f, true});
    }
    SUBCASE("distinct")
    {
        auto const rows {dbTable->select_from<bool>(distinct, "Alive")()};
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == false);
        REQUIRE(rows[1] == true);
    }
    SUBCASE("tuple to type")
    {
        auto const rows {dbTable->select_from<i32, string, i32, f32, bool>().exec<foo>()};
        REQUIRE(rows.size() == 100);
        REQUIRE(rows[0] == foo {1, "1", 100, 1.5f, false});
        REQUIRE(rows[1] == foo {2, "2", 200, 3.0f, true});
    }
}

TEST_CASE("Data.Sqlite.Aggregate")
{
    string tableName {"testTable"};

    database db {database::OpenMemory()};
    auto     dbTable {db.create_table(tableName,
                                      int_column<primary_key> {.Name = "ID", .NotNull = true},
                                      text_column {.Name = "Name", .NotNull = true},
                                      int_column {.Name = "Age"},
                                      text_column {.Name = "Category"})};
    REQUIRE(dbTable);

    std::vector<std::tuple<string, i32, string>> vec {
        {"A", 100, "X"},
        {"A", 110, "X"},
        {"A", 120, "Y"},
        {"A", 130, "Y"},
        {"B", 200, "X"},
        {"B", 210, "X"},
        {"B", 220, "Y"},
        {"B", 230, "Y"},
        {"B", 240, "Y"},
    };

    REQUIRE(dbTable->insert_into("Name", "Age", "Category")(vec));

    SUBCASE("group_by single column: Name")
    {
        auto const rows {dbTable->select_from<i32>(sum("Age")).group_by("Name")()};

        REQUIRE(rows.size() == 2);

        std::vector<i32> expectedSums {100 + 110 + 120 + 130, 200 + 210 + 220 + 230 + 240};
        REQUIRE(((rows[0] == expectedSums[0] && rows[1] == expectedSums[1]) || (rows[1] == expectedSums[0] && rows[0] == expectedSums[1])));
    }
    SUBCASE("count grouped by Name")
    {
        auto const counts {dbTable->select_from<i32>(count("*")).group_by("Name")()};

        REQUIRE(counts.size() == 2);

        std::vector<i32> expectedCounts {4, 5};
        REQUIRE(((counts[0] == expectedCounts[0] && counts[1] == expectedCounts[1]) || (counts[1] == expectedCounts[0] && counts[0] == expectedCounts[1])));
    }
    SUBCASE("avg Age grouped by Name")
    {
        auto const avgs {dbTable->select_from<double>(avg("Age")).group_by("Name")()};

        REQUIRE(avgs.size() == 2);

        f64 const expectedAvgA {(100 + 110 + 120 + 130) / 4.0};
        f64 const expectedAvgB {(200 + 210 + 220 + 230 + 240) / 5.0};

        bool order1 {(std::abs(avgs[0] - expectedAvgA) < 1e-6 && std::abs(avgs[1] - expectedAvgB) < 1e-6)};
        bool order2 {(std::abs(avgs[1] - expectedAvgA) < 1e-6 && std::abs(avgs[0] - expectedAvgB) < 1e-6)};
        REQUIRE((order1 || order2));
    }
    SUBCASE("min Age grouped by Name")
    {
        auto const mins {dbTable->select_from<i32>(min("Age")).group_by("Name")()};

        REQUIRE(mins.size() == 2);

        std::vector<i32> expectedMins {100, 200};
        REQUIRE(((mins[0] == expectedMins[0] && mins[1] == expectedMins[1]) || (mins[1] == expectedMins[0] && mins[0] == expectedMins[1])));
    }
    SUBCASE("max Age grouped by Name")
    {
        auto const maxs {dbTable->select_from<i32>(max("Age")).group_by("Name")()};

        REQUIRE(maxs.size() == 2);

        std::vector<i32> expectedMaxs {130, 240};
        REQUIRE(((maxs[0] == expectedMaxs[0] && maxs[1] == expectedMaxs[1]) || (maxs[1] == expectedMaxs[0] && maxs[0] == expectedMaxs[1])));
    }
    SUBCASE("group_by multiple columns: Name and Category")
    {
        auto const rows {dbTable->select_from<i32>(sum("Age")).group_by("Name", "Category")()};

        REQUIRE(rows.size() == 4);

        std::vector<i32> expectedSums {100 + 110, 120 + 130, 200 + 210, 220 + 230 + 240};

        for (auto val : expectedSums) {
            REQUIRE(std::ranges::find(rows, val) != rows.end());
        }
    }
    SUBCASE("having")
    {
        auto const rows {dbTable->select_from<string, string, i32>("Name", "Category", count("*"))
                             .group_by("Name", "Category")
                             .having(greater {count("*"), 2})()};

        REQUIRE(rows.size() == 1);

        auto const& [name, category, count] = rows[0];
        REQUIRE(name == "B");
        REQUIRE(category == "Y");
        REQUIRE(count == 3);
    }
}

TEST_CASE("Data.Sqlite.Insert")
{
    string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName,
                                  int_column {.Name = "ID"},
                                  text_column {.Name = "Name"},
                                  int_column {.Name = "Age"},
                                  real_column {.Name = "Height"})};
    REQUIRE(dbTable);

    SUBCASE("tuples")
    {
        std::tuple tup0 {1, "A", 100, 1.5f};
        std::tuple tup1 {2, "B", 200, 4.5f};

        REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup0, tup1));
        auto const rows {dbTable->select_from<i32, string, i32, f32>("ID", "Name", "Age", "Height")()};
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == tup0);
        REQUIRE(rows[1] == tup1);
    }
    SUBCASE("vector of tuples")
    {
        auto vec {std::vector {std::tuple {1, "A", 100, 1.5f}, std::tuple {2, "B", 200, 4.5f}}};
        REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(vec));

        auto const rows {dbTable->select_from<i32, string, i32, f32>("ID", "Name", "Age", "Height")()};
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == vec[0]);
        REQUIRE(rows[1] == vec[1]);
    }
    SUBCASE("values")
    {
        REQUIRE(dbTable->insert_into("Age")(100, 200));
        auto const rows {dbTable->select_from<i32>("Age")()};
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == 100);
        REQUIRE(rows[1] == 200);
    }
}

TEST_CASE("Data.Sqlite.Update")
{
    string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName,
                                  int_column<primary_key> {.Name = "ID", .NotNull = true},
                                  text_column {.Name = "Name", .NotNull = true},
                                  int_column {.Name = "Age"},
                                  real_column {.Name = "Height"})};
    REQUIRE(dbTable);

    SUBCASE("WHERE value in conditional")
    {
        std::tuple tup0 {1, "A", 100, 1.5f};
        std::tuple tup1 {2, "B", 200, 4.5f};

        REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup0, tup1));
        REQUIRE(dbTable->update("Age", "Height").where(equal {"ID", 2})(100, 4.2));

        auto const rows {dbTable->select_from<i32, string, i32, f32>("ID", "Name", "Age", "Height")()};
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == tup0);
        REQUIRE(rows[1] == std::tuple {2, "B", 100, 4.2f});
    }
    SUBCASE("WHERE value as parameter")
    {
        std::tuple tup0 {1, "A", 100, 1.5f};
        std::tuple tup1 {2, "B", 200, 4.5f};

        REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup0, tup1));
        REQUIRE(dbTable->update("Age", "Height").where(equal {"ID"})(100, 4.2, 2));

        auto const rows {dbTable->select_from<i32, string, i32, f32>("ID", "Name", "Age", "Height")()};
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == tup0);
        REQUIRE(rows[1] == std::tuple {2, "B", 100, 4.2f});
    }
}

TEST_CASE("Data.Sqlite.Delete")
{
    string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName,
                                  int_column<primary_key> {.Name = "ID", .NotNull = true},
                                  text_column {.Name = "Name", .NotNull = true},
                                  int_column {.Name = "Age"},
                                  real_column {.Name = "Height"})};
    REQUIRE(dbTable);

    std::tuple tup0 {1, "A", 100, 1.5f};
    std::tuple tup1 {2, "B", 200, 4.5f};

    REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup0, tup1));
    REQUIRE(dbTable->row_count() == 2);
    REQUIRE(dbTable->delete_from().where(equal("ID", 2))());
    REQUIRE(dbTable->row_count() == 1);
}

TEST_CASE("Data.Sqlite.CreateTable")
{
    string tableName0 {"testTable0"};
    string tableName1 {"testTable1"};

    database db {database::OpenMemory()};

    REQUIRE(db.create_table(tableName0, int_column<primary_key> {.Name = "ID", .NotNull = true, .Constraint = {}}));

    REQUIRE(db.table_names().size() == 1);
    REQUIRE(db.table_names().contains(tableName0));

    REQUIRE(db.create_table(tableName1,
                            int_column<primary_key> {.Name = "ID", .NotNull = true},
                            text_column {.Name = "Name", .NotNull = true},
                            int_column {.Name = "Age"},
                            real_column {.Name = "Height"}));

    auto nameSet {db.get_table(tableName1)->column_names()};
    for (auto const& col : std::array {"ID", "Name", "Age", "Height"}) {
        REQUIRE(nameSet.contains(col));
    }

    REQUIRE(db.table_names().size() == 2);
    REQUIRE(db.table_names().contains(tableName0));
    REQUIRE(db.table_names().contains(tableName1));

    REQUIRE(db.table_exists(tableName0));
    REQUIRE(db.table_exists(tableName1));
    REQUIRE_FALSE(db.table_exists("foobar"));
}

TEST_CASE("Data.Sqlite.DropTable")
{
    string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName, int_column<primary_key> {.Name = "ID", .NotNull = true})};
    REQUIRE(db.table_exists(tableName));

    REQUIRE(db.table_names().size() == 1);
    REQUIRE(db.table_names().contains(tableName));

    REQUIRE(db.drop_table(tableName));

    REQUIRE_FALSE(db.table_exists(tableName));
    REQUIRE(db.table_names().size() == 0);
}

TEST_CASE("Data.Sqlite.Constraints")
{
    database db {database::OpenMemory()};

    SUBCASE("no_constraint")
    {
        auto table {db.create_table("TestNone",
                                    int_column<no_constraint> {.Name = "ID"})};
        REQUIRE(table);
        REQUIRE(table->insert_into("ID")(42));
        auto const rows {table->select_from<i32>("ID")()};
        REQUIRE(rows[0] == 42);
    }

    SUBCASE("default_value")
    {
        auto table {db.create_table("TestDefault",
                                    int_column<no_constraint> {.Name = "ID"},
                                    text_column<default_value<string>> {.Name = "Name", .Constraint = {"DefaultName"}})};
        REQUIRE(table);
        REQUIRE(table->insert_into("ID")(0));
        auto const rows {table->select_from<string>("Name")()};
        REQUIRE(rows[0] == "DefaultName");
    }

    SUBCASE("unique")
    {
        auto table {db.create_table("TestUnique",
                                    int_column {.Name = "ID"},
                                    unique {"ID"})};
        REQUIRE(table);
        REQUIRE(table->insert_into("ID")(1));
        REQUIRE_FALSE(table->insert_into("ID")(1)); // duplicate
    }

    SUBCASE("primary_key")
    {
        auto table {db.create_table("TestPrimaryKey",
                                    int_column<primary_key> {.Name = "ID"})};
        REQUIRE(table);
        REQUIRE(table->insert_into("ID")(123));
        REQUIRE_FALSE(table->insert_into("ID")(123)); // primary key violation
    }

    SUBCASE("foreign_key")
    {
        REQUIRE(db.create_table("Parent",
                                int_column<primary_key> {.Name = "ID"}));
        REQUIRE(db.create_table("Child",
                                int_column<foreign_key> {.Name = "ParentID", .Constraint = {.ForeignTable = "Parent", .ForeignColumn = "ID"}}));
        REQUIRE(db.get_table("Parent")->insert_into("ID")(1));
        REQUIRE(db.get_table("Child")->insert_into("ParentID")(1));
        REQUIRE_FALSE(db.get_table("Child")->insert_into("ParentID")(999)); // invalid FK
    }

    SUBCASE("check")
    {
        auto table {db.create_table("TestCheck",
                                    int_column<check> {.Name = "Value", .Constraint = {"Value > 0"}})};
        REQUIRE(table);
        REQUIRE(table->insert_into("Value")(10));
        REQUIRE_FALSE(table->insert_into("Value")(0)); // check failed
    }
}

TEST_CASE("Data.Sqlite.VacuumInto")
{
    string tableName {"testTable"};
    string fileName {"test0.db"};
    {
        database db {database::OpenMemory()};

        auto dbTable {db.create_table(tableName, int_column<primary_key> {.Name = "ID", .NotNull = true})};

        io::delete_file(fileName);
        REQUIRE_FALSE(io::exists(fileName));
        REQUIRE(db.vacuum_into(fileName));
    }
    {
        REQUIRE(io::exists(fileName));
        auto db {database::Open(fileName)};
        REQUIRE(db);
        REQUIRE(db->table_exists(tableName));
    }
}

TEST_CASE("Data.Sqlite.Attach")
{
    SUBCASE("file")
    {
        string const mainTable {"mainTable"};
        string const attachedTable {"attachedTable"};
        string const attachedFile {"attached.db"};
        string const aliasSchema {"other"};

        {
            database   attached {database::OpenMemory()};
            auto const table {attached.create_table(attachedTable,
                                                    int_column<primary_key> {.Name = "ID", .NotNull = true},
                                                    text_column {.Name = "Name", .NotNull = true})};
            REQUIRE(table);

            std::vector<std::tuple<i32, string>> const data {
                {1, "One"},
                {2, "Two"},
                {3, "Three"}};
            REQUIRE(table->insert_into("ID", "Name")(data));

            io::delete_file(attachedFile);
            REQUIRE_FALSE(io::exists(attachedFile));
            REQUIRE(attached.vacuum_into(attachedFile));
        }

        {
            REQUIRE(io::exists(attachedFile));
            database db {database::OpenMemory()};

            auto attached {db.attach(attachedFile, aliasSchema)};
            REQUIRE(attached);
            REQUIRE(attached->table_exists(attachedTable));

            auto const table {attached->get_table(attachedTable)};
            REQUIRE(table);

            auto const rows {table->select_from<string>("Name").where(equal {"ID", 2})()};
            REQUIRE(rows.size() == 1);
            REQUIRE(rows[0] == "Two");
            REQUIRE(db.schema_exists(aliasSchema));
            REQUIRE(attached->detach());
            REQUIRE_FALSE(db.schema_exists(aliasSchema));
        }
    }
    SUBCASE("memory")
    {
        string const attachedTable {"attachedTable"};
        string const aliasSchema {"other"};

        database db {database::OpenMemory()};

        auto attached {db.attach_memory(aliasSchema)};
        REQUIRE(attached);

        auto const table {attached->create_table(attachedTable,
                                                 int_column<primary_key> {.Name = "ID", .NotNull = true},
                                                 text_column {.Name = "Name", .NotNull = true})};

        REQUIRE(table);
        REQUIRE(attached->table_exists(attachedTable));
        REQUIRE(db.schema_exists(aliasSchema));
        REQUIRE(attached->detach());
        REQUIRE_FALSE(db.schema_exists(aliasSchema));
    }
}

TEST_CASE("Data.Sqlite.Blobs")
{
    string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName,
                                  int_column<primary_key> {.Name = "ID", .NotNull = true},
                                  blob_column {.Name = "Point"},
                                  blob_column {.Name = "Size"},
                                  blob_column {.Name = "Rect"},
                                  blob_column {.Name = "Color"})};
    REQUIRE(db.table_exists(tableName));
    std::tuple tup0 {1, point_f {4.2f, 2.4f}, size_u {420, 69}, rect_i {-5, 5, 20, 9}, colors::Aqua};
    std::tuple tup1 {2, point_f {6.9f, 9.6f}, size_u {123, 456}, rect_i {5, -5, -20, 9}, colors::RebeccaPurple};

    REQUIRE(dbTable->insert_into("ID", "Point", "Size", "Rect", "Color")(tup0, tup1));
    {
        auto const rows {dbTable->select_from<i32, point_f, size_u, rect_i, color>()()};
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == tup0);
        REQUIRE(rows[1] == tup1);
    }
}

TEST_CASE("Data.Sqlite.NullAndOptional")
{
    string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName,
                                  int_column<primary_key> {.Name = "ID", .NotNull = true},
                                  int_column {.Name = "Age", .NotNull = false})};
    REQUIRE(db.table_exists(tableName));

    std::tuple tup0 {100};
    std::tuple tup1 {nullptr};
    std::tuple tup2 {std::nullopt};

    REQUIRE(dbTable->insert_into("Age")(tup0, tup1, tup2));
    {
        auto const rows {dbTable->select_from<std::optional<i32>>("Age")()};
        REQUIRE(rows.size() == 3);
        REQUIRE(rows[0] == 100);
        REQUIRE(rows[1] == std::nullopt);
        REQUIRE(rows[2] == std::nullopt);
    }
}

TEST_CASE("Data.Sqlite.Views")
{
    string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName,
                                  int_column<primary_key> {.Name = "ID", .NotNull = true},
                                  int_column {.Name = "A"},
                                  int_column {.Name = "B"},
                                  int_column {.Name = "C"},
                                  int_column {.Name = "D"})};
    REQUIRE(db.table_exists(tableName));

    std::vector<std::tuple<i32, i32, i32, i32>> values;
    rng                                         r {54321};
    for (i32 i {0}; i < 650; i++) {
        values.emplace_back(r(0, 100), r(0, 100), r(0, 100), r(0, 100));
    }

    REQUIRE(dbTable->insert_into("A", "B", "C", "D")(values));
    {
        auto       select {std::move(dbTable->select_from<i32, i32, i32, i32>("A", "B", "C", "D").where("A > 50"))};
        auto const rows {select()};

        auto dbView {db.create_view("testView", select)};
        REQUIRE(dbView.has_value());
        REQUIRE(db.view_exists("testView"));
        if (dbView) {
            auto viewrows {dbView->select_from<i32, i32, i32, i32>()()};
            REQUIRE(viewrows.size() == rows.size());
        }
    }
}

TEST_CASE("Data.Sqlite.Join")
{
    string tableName0 {"People"};
    string tableName1 {"Countries"};

    database db {database::OpenMemory()};

    SUBCASE("LeftJoin")
    {
        {
            auto dbTable {db.create_table(tableName0,
                                          int_column<primary_key> {.Name = "ID", .NotNull = true},
                                          text_column {.Name = "Name"},
                                          int_column {.Name = "CountryID"})};
            REQUIRE(db.table_exists(tableName0));

            std::tuple tup0 {"Peter", 1};
            std::tuple tup1 {"Paul", 1};
            std::tuple tup2 {"Marie", 2};

            REQUIRE(dbTable->insert_into("Name", "CountryID")(tup0, tup1, tup2));
        }

        auto dbTable {db.create_table(tableName1,
                                      int_column<primary_key> {.Name = "ID", .NotNull = true},
                                      text_column {.Name = "Code"})};
        REQUIRE(db.table_exists(tableName1));

        std::tuple tup0 {1, "UK"};
        std::tuple tup1 {2, "USA"};

        REQUIRE(dbTable->insert_into("ID", "Code")(tup0, tup1));

        auto const rows {db.get_table(tableName0)
                             ->select_from<string, string>("Name", "Code")
                             .left_join(*dbTable, on {.LeftColumn = "CountryID", .RightColumn = "ID"})()};
        REQUIRE(rows.size() == 3);
        REQUIRE(rows[0] == std::tuple {"Peter", "UK"});
        REQUIRE(rows[1] == std::tuple {"Paul", "UK"});
        REQUIRE(rows[2] == std::tuple {"Marie", "USA"});
    }
    SUBCASE("InnerJoin")
    {
        {
            auto dbTable {db.create_table(tableName0,
                                          int_column<primary_key> {.Name = "ID", .NotNull = true},
                                          text_column {.Name = "Name"},
                                          int_column {.Name = "CountryID"})};
            REQUIRE(db.table_exists(tableName0));

            std::tuple tup0 {"Peter", 1};
            std::tuple tup1 {"Paul", 1};
            std::tuple tup2 {"Marie", 2};

            REQUIRE(dbTable->insert_into("Name", "CountryID")(tup0, tup1, tup2));
        }

        auto dbTable {db.create_table(tableName1,
                                      int_column<primary_key> {.Name = "ID", .NotNull = true},
                                      text_column {.Name = "Code"})};
        REQUIRE(db.table_exists(tableName1));

        std::tuple tup0 {1, "UK"};

        REQUIRE(dbTable->insert_into("ID", "Code")(tup0));

        auto const rows {db.get_table(tableName0)
                             ->select_from<string, string>("Name", "Code")
                             .inner_join(*dbTable, on {.LeftColumn = "CountryID", .RightColumn = "ID"})()};
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == std::tuple {"Peter", "UK"});
        REQUIRE(rows[1] == std::tuple {"Paul", "UK"});
    }
    SUBCASE("CrossJoin")
    {
        {
            REQUIRE(db.create_table(tableName0, text_column {"Color"}));
            REQUIRE(db.get_table(tableName0)->insert_into("Color")("Green", "Blue", "Yellow"));
        }
        {
            REQUIRE(db.create_table(tableName1, int_column {"Number"}));
            REQUIRE(db.get_table(tableName1)->insert_into("Number")(1, 2));
        }

        auto const rows {db.get_table(tableName0)->select_from<string, i32>("Color", "Number").cross_join(tableName1)()};
        REQUIRE(rows.size() == 6);
        REQUIRE(rows[0] == std::tuple {"Green", 1});
        REQUIRE(rows[1] == std::tuple {"Green", 2});
        REQUIRE(rows[2] == std::tuple {"Blue", 1});
        REQUIRE(rows[3] == std::tuple {"Blue", 2});
        REQUIRE(rows[4] == std::tuple {"Yellow", 1});
        REQUIRE(rows[5] == std::tuple {"Yellow", 2});
    }
}

TEST_CASE("Data.Sqlite.SavePoint")
{
    string tableName {"testTable"};

    database db {database::OpenMemory()};

    auto dbTable {db.create_table(tableName,
                                  int_column {.Name = "ID"},
                                  text_column {.Name = "Name"},
                                  int_column {.Name = "Age"},
                                  real_column {.Name = "Height"})};
    REQUIRE(dbTable);

    auto tup0 {std::tuple {1, "A", 100, 1.5f}};
    REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup0));
    auto rows {dbTable->select_from<i32, string, i32, f32>("ID", "Name", "Age", "Height")()};
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0] == tup0);

    {
        auto sp {db.create_savepoint("t1")};

        auto tup1 {std::tuple {2, "B", 200, 4.5f}};
        REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup1));
        rows = dbTable->select_from<i32, string, i32, f32>("ID", "Name", "Age", "Height")();
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == tup0);
        REQUIRE(rows[1] == tup1);
    }

    rows = dbTable->select_from<i32, string, i32, f32>("ID", "Name", "Age", "Height")();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0] == tup0);
}

TEST_CASE("Data.Sqlite.TableInfo")
{
    string tableName {"testTable"};

    database db {database::OpenMemory()};
    auto     dbTable {db.create_table(tableName,
                                      int_column<primary_key> {.Name = "ID", .NotNull = true},
                                      text_column {.Name = "Name", .NotNull = true},
                                      int_column {.Name = "Age"},
                                      real_column {.Name = "Height"},
                                      blob_column {.Name = "Alive"})};

    auto info {dbTable->info()};

    REQUIRE(info.size() == 5);

    REQUIRE(info[0].Name == "ID");
    REQUIRE(info[0].Type == "INTEGER");
    REQUIRE(info[0].NotNull == true);
    REQUIRE(info[0].IsPrimaryKey == true);

    REQUIRE(info[1].Name == "Name");
    REQUIRE(info[1].Type == "TEXT");
    REQUIRE(info[1].NotNull == true);
    REQUIRE(info[1].IsPrimaryKey == false);

    REQUIRE(info[2].Name == "Age");
    REQUIRE(info[2].Type == "INTEGER");
    REQUIRE_FALSE(info[2].NotNull);
    REQUIRE_FALSE(info[2].IsPrimaryKey);

    REQUIRE(info[3].Name == "Height");
    REQUIRE(info[3].Type == "REAL");
    REQUIRE_FALSE(info[3].NotNull);
    REQUIRE_FALSE(info[3].IsPrimaryKey);

    REQUIRE(info[4].Name == "Alive");
    REQUIRE(info[4].Type == "BLOB");
    REQUIRE_FALSE(info[4].NotNull);
    REQUIRE_FALSE(info[4].IsPrimaryKey);
}
