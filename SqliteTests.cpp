#include "tests.hpp"

using namespace tcob::data::sqlite;

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
                                      int_column {.Name = "Height"},
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
            auto rows = dbTable->select_from<i32, string, i32, f32, bool>()();
            REQUIRE(rows.size() == 100);
            REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
            REQUIRE(rows[1] == std::tuple {2, "2", 200, 3.0f, true});
        }
        {
            auto rows = dbTable->select_from<string, i32, f32>("Name", "Age", "Height")();
            REQUIRE(rows.size() == 100);
            REQUIRE(rows[0] == std::tuple {"1", 100, 1.5f});
            REQUIRE(rows[1] == std::tuple {"2", 200, 3.0f});
        }
    }
    SUBCASE("where")
    {
        {
            auto rows = dbTable->select_from<i32, string, i32, f32, bool>().where(equal {.Column = "Age", .Value = 100})();
            REQUIRE(rows.size() == 1);
            REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
        }
        {
            auto rows = dbTable->select_from<i32, string, i32, f32, bool>().where(equal {.Column = "Age"})(500);
            REQUIRE(rows.size() == 1);
            REQUIRE(rows[0] == std::tuple {5, "5", 500, 7.5f, false});
        }
        {
            auto rows = dbTable->select_from<i32, string, i32, f32, bool>().where(like {.Column = "Name", .Value = "%0"})();
            REQUIRE(rows.size() == 10);
            for (i32 i {1}; i <= 10; ++i) {
                auto const tup {std::tuple {i * 10, std::to_string(i * 10), i * 1000, static_cast<f32>(i) * 15.0f, true}};
                REQUIRE_MESSAGE(rows[i - 1] == tup, i);
            }
        }
    }
    SUBCASE("order_by")
    {
        SUBCASE("descending by Name")
        {
            auto rows = dbTable->select_from<i32, string, i32, f32, bool>()
                            .order_by(desc {"Name"})();
            REQUIRE(rows.size() == 100);
            REQUIRE(rows[0] == std::tuple {99, "99", 9900, 148.5f, false});
            REQUIRE(rows[1] == std::tuple {98, "98", 9800, 147.0f, true});
            REQUIRE(rows[2] == std::tuple {97, "97", 9700, 145.5f, false});
        }

        SUBCASE("ascending by Name")
        {
            auto rows = dbTable->select_from<i32, string, i32, f32, bool>()
                            .order_by(asc {"Name"})();
            REQUIRE(rows.size() == 100);
            REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
            REQUIRE(rows[1] == std::tuple {10, "10", 1000, 15.0f, true});
            REQUIRE(rows[2] == std::tuple {100, "100", 10000, 150.0f, true});
        }
        SUBCASE("ascending by Age")
        {
            auto rows = dbTable->select_from<i32, string, i32, f32, bool>()
                            .order_by(asc {"Age"})();
            REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
            REQUIRE(rows[1] == std::tuple {2, "2", 200, 3.0f, true});
        }
        SUBCASE("ascending by 3")
        {
            auto rows = dbTable->select_from<i32, string, i32, f32, bool>()
                            .order_by(asc {3})();
            REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
            REQUIRE(rows[1] == std::tuple {2, "2", 200, 3.0f, true});
        }
        SUBCASE("descending by Height")
        {
            auto rows = dbTable->select_from<i32, string, i32, f32, bool>()
                            .order_by(desc {"Height"})();
            REQUIRE(rows[0] == std::tuple {100, "100", 10000, 150.0f, true});
            REQUIRE(rows[1] == std::tuple {99, "99", 9900, 148.5f, false});
        }

        SUBCASE("ascending by Alive")
        {
            auto rows = dbTable->select_from<i32, string, i32, f32, bool>()
                            .order_by(asc {"Alive"})();
            for (usize i = 0; i < 50; ++i) {
                REQUIRE(std::get<4>(rows[i]) == false);
            }
            for (usize i = 50; i < 100; ++i) {
                REQUIRE(std::get<4>(rows[i]) == true);
            }
        }

        SUBCASE("multi-column: Alive asc, Age desc")
        {
            auto rows = dbTable->select_from<i32, string, i32, f32, bool>()
                            .order_by(asc {"Alive"},
                                      desc {"Age"})();
            for (usize i = 1; i < rows.size(); ++i) {
                auto const& [idPrev, namePrev, agePrev, hPrev, alivePrev] = rows[i - 1];
                auto const& [idCurr, nameCurr, ageCurr, hCurr, aliveCurr] = rows[i];

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
            auto rows = dbTable->select_from<i32, string, i32, f32, bool>()
                            .order_by(asc {5}, desc {3})();
            for (usize i = 1; i < rows.size(); ++i) {
                auto const& [idPrev, namePrev, agePrev, hPrev, alivePrev] = rows[i - 1];
                auto const& [idCurr, nameCurr, ageCurr, hCurr, aliveCurr] = rows[i];

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
            auto rows = dbTable->select_from<i32, string, i32, f32, bool>()
                            .order_by("Alive ASC", "Age DESC")();
            for (usize i = 1; i < rows.size(); ++i) {
                auto const& [idPrev, namePrev, agePrev, hPrev, alivePrev] = rows[i - 1];
                auto const& [idCurr, nameCurr, ageCurr, hCurr, aliveCurr] = rows[i];

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
        auto rows = dbTable->select_from<i32, string, i32, f32, bool>().limit(1)();
        REQUIRE(rows.size() == 1);
        REQUIRE(rows[0] == std::tuple {1, "1", 100, 1.5f, false});
    }
    SUBCASE("limit offset")
    {
        auto rows = dbTable->select_from<i32, string, i32, f32, bool>("ID", "Name", "Age", "Height", "Alive").limit(1, 1)();
        REQUIRE(rows.size() == 1);
        REQUIRE(rows[0] == std::tuple {2, "2", 200, 3.0f, true});
    }
    SUBCASE("distinct")
    {
        auto rows = dbTable->select_from<bool>(distinct, "Alive")();
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == false);
        REQUIRE(rows[1] == true);
    }
    SUBCASE("tuple to type")
    {
        std::vector<foo> rows {dbTable->select_from<i32, string, i32, f32, bool>().exec<foo>()};
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
        auto rows = dbTable->select_from<i32>(sum("Age")).group_by("Name")();

        REQUIRE(rows.size() == 2);

        std::vector<i32> expectedSums {100 + 110 + 120 + 130, 200 + 210 + 220 + 230 + 240};
        REQUIRE(((rows[0] == expectedSums[0] && rows[1] == expectedSums[1]) || (rows[1] == expectedSums[0] && rows[0] == expectedSums[1])));
    }

    SUBCASE("count grouped by Name")
    {
        auto counts = dbTable->select_from<i32>(count("*")).group_by("Name")();

        REQUIRE(counts.size() == 2);

        std::vector<i32> expectedCounts {4, 5};
        REQUIRE(((counts[0] == expectedCounts[0] && counts[1] == expectedCounts[1]) || (counts[1] == expectedCounts[0] && counts[0] == expectedCounts[1])));
    }

    SUBCASE("avg Age grouped by Name")
    {
        auto avgs = dbTable->select_from<double>(avg("Age")).group_by("Name")();

        REQUIRE(avgs.size() == 2);

        double expectedAvgA = (100 + 110 + 120 + 130) / 4.0;
        double expectedAvgB = (200 + 210 + 220 + 230 + 240) / 5.0;

        bool order1 = (std::abs(avgs[0] - expectedAvgA) < 1e-6 && std::abs(avgs[1] - expectedAvgB) < 1e-6);
        bool order2 = (std::abs(avgs[1] - expectedAvgA) < 1e-6 && std::abs(avgs[0] - expectedAvgB) < 1e-6);
        REQUIRE((order1 || order2));
    }

    SUBCASE("min Age grouped by Name")
    {
        auto mins = dbTable->select_from<i32>(min("Age")).group_by("Name")();

        REQUIRE(mins.size() == 2);

        std::vector<i32> expectedMins {100, 200};
        REQUIRE(((mins[0] == expectedMins[0] && mins[1] == expectedMins[1]) || (mins[1] == expectedMins[0] && mins[0] == expectedMins[1])));
    }

    SUBCASE("max Age grouped by Name")
    {
        auto maxs = dbTable->select_from<i32>(max("Age")).group_by("Name")();

        REQUIRE(maxs.size() == 2);

        std::vector<i32> expectedMaxs {130, 240};
        REQUIRE(((maxs[0] == expectedMaxs[0] && maxs[1] == expectedMaxs[1]) || (maxs[1] == expectedMaxs[0] && maxs[0] == expectedMaxs[1])));
    }

    SUBCASE("group_by multiple columns: Name and Category")
    {
        auto rows = dbTable->select_from<i32>(sum("Age")).group_by("Name", "Category")();

        REQUIRE(rows.size() == 4);

        std::vector<i32> expectedSums {100 + 110, 120 + 130, 200 + 210, 220 + 230 + 240};

        for (auto val : expectedSums) {
            REQUIRE(std::ranges::find(rows, val) != rows.end());
        }
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
                                  int_column {.Name = "Height"})};
    REQUIRE(dbTable);

    SUBCASE("tuples")
    {
        auto tup0 = std::tuple {1, "A", 100, 1.5f};
        auto tup1 = std::tuple {2, "B", 200, 4.5f};

        REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup0, tup1));
        auto rows = dbTable->select_from<i32, string, i32, f32>("ID", "Name", "Age", "Height")();
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == tup0);
        REQUIRE(rows[1] == tup1);
    }
    SUBCASE("vector of tuples")
    {
        auto vec = std::vector {std::tuple {1, "A", 100, 1.5f}, std::tuple {2, "B", 200, 4.5f}};
        REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(vec));

        auto rows = dbTable->select_from<i32, string, i32, f32>("ID", "Name", "Age", "Height")();
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0] == vec[0]);
        REQUIRE(rows[1] == vec[1]);
    }
    SUBCASE("values")
    {
        REQUIRE(dbTable->insert_into("Age")(100, 200));
        auto rows = dbTable->select_from<i32>("Age")();
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
                                  int_column {.Name = "Height"})};
    REQUIRE(dbTable);

    auto tup0 = std::tuple {1, "A", 100, 1.5f};
    auto tup1 = std::tuple {2, "B", 200, 4.5f};

    REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup0, tup1));
    REQUIRE(dbTable->update("Age", "Height").where(equal {"ID", 2})(100, 4.2));
    {
        auto rows = dbTable->select_from<i32, string, i32, f32>("ID", "Name", "Age", "Height")();
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
                                  int_column {.Name = "Height"})};
    REQUIRE(dbTable);

    auto tup0 = std::tuple {1, "A", 100, 1.5f};
    auto tup1 = std::tuple {2, "B", 200, 4.5f};

    REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup0, tup1));
    REQUIRE(dbTable->row_count() == 2);
    REQUIRE(dbTable->delete_from().where(equal("ID", 2))());
    REQUIRE(dbTable->row_count() == 1);
}

TEST_CASE("Data.Sqlite.CreateTable")
{
    string tableName0 = "testTable0";
    string tableName1 = "testTable1";

    database db {database::OpenMemory()};

    REQUIRE(db.create_table(tableName0, int_column<primary_key> {.Name = "ID", .NotNull = true, .Constraint = {}}));

    REQUIRE(db.table_names().size() == 1);
    REQUIRE(db.table_names().contains(tableName0));

    REQUIRE(db.create_table(tableName1,
                            int_column<primary_key> {.Name = "ID", .NotNull = true},
                            text_column {.Name = "Name", .NotNull = true},
                            int_column {.Name = "Age"},
                            int_column {.Name = "Height"}));

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
        auto table = db.create_table("TestNone",
                                     int_column<no_constraint> {.Name = "ID"});
        REQUIRE(table);
        REQUIRE(table->insert_into("ID")(42));
        auto rows = table->select_from<i32>("ID")();
        REQUIRE(rows[0] == 42);
    }

    SUBCASE("default_value")
    {
        auto table = db.create_table("TestDefault",
                                     int_column<no_constraint> {.Name = "ID"},
                                     text_column<default_value<string>> {.Name = "Name", .Constraint = {"DefaultName"}});
        REQUIRE(table);
        REQUIRE(table->insert_into("ID")(0));
        auto rows = table->select_from<string>("Name")();
        REQUIRE(rows[0] == "DefaultName");
    }

    SUBCASE("unique")
    {
        auto table = db.create_table("TestUnique",
                                     int_column {.Name = "ID"},
                                     unique {"ID"});
        REQUIRE(table);
        REQUIRE(table->insert_into("ID")(1));
        REQUIRE_FALSE(table->insert_into("ID")(1)); // duplicate
    }

    SUBCASE("primary_key")
    {
        auto table = db.create_table("TestPrimaryKey",
                                     int_column<primary_key> {.Name = "ID"});
        REQUIRE(table);
        REQUIRE(table->insert_into("ID")(123));
        REQUIRE_FALSE(table->insert_into("ID")(123)); // primary key violation
    }

    SUBCASE("foreign_key")
    {
        REQUIRE(db.create_table("Parent",
                                int_column<primary_key> {.Name = "ID"}));
        REQUIRE(db.create_table("Child",
                                int_column<foreign_key> {.Name = "ParentID", .Constraint = {"Parent", "ID"}}));
        REQUIRE(db.get_table("Parent")->insert_into("ID")(1));
        REQUIRE(db.get_table("Child")->insert_into("ParentID")(1));
        REQUIRE_FALSE(db.get_table("Child")->insert_into("ParentID")(999)); // invalid FK
    }

    SUBCASE("check")
    {
        auto table = db.create_table("TestCheck",
                                     int_column<check> {.Name = "Value", .Constraint = {"Value > 0"}});
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
    auto tup0 = std::tuple {1, point_f {4.2f, 2.4f}, size_u {420, 69}, rect_i {-5, 5, 20, 9}, colors::Aqua};
    auto tup1 = std::tuple {2, point_f {6.9f, 9.6f}, size_u {123, 456}, rect_i {5, -5, -20, 9}, colors::RebeccaPurple};

    REQUIRE(dbTable->insert_into("ID", "Point", "Size", "Rect", "Color")(tup0, tup1));
    {
        auto rows = dbTable->select_from<i32, point_f, size_u, rect_i, color>()();
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

    auto tup0 = std::tuple {100};
    auto tup1 = std::tuple {nullptr};
    auto tup2 = std::tuple {std::nullopt};

    REQUIRE(dbTable->insert_into("Age")(tup0, tup1, tup2));
    {
        auto rows = dbTable->select_from<std::optional<i32>>("Age")();
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
        auto select {std::move(dbTable->select_from<i32, i32, i32, i32>("A", "B", "C", "D").where("A > 50"))};
        auto rows {select()};

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
    string tableName0 = "People";
    string tableName1 = "Countries";

    database db {database::OpenMemory()};

    SUBCASE("LeftJoin")
    {
        {
            auto dbTable {db.create_table(tableName0,
                                          int_column<primary_key> {.Name = "ID", .NotNull = true},
                                          text_column {.Name = "Name"},
                                          int_column {.Name = "CountryID"})};
            REQUIRE(db.table_exists(tableName0));

            auto tup0 = std::tuple {"Peter", 1};
            auto tup1 = std::tuple {"Paul", 1};
            auto tup2 = std::tuple {"Marie", 2};

            REQUIRE(dbTable->insert_into("Name", "CountryID")(tup0, tup1, tup2));
        }
        {
            auto dbTable {db.create_table(tableName1,
                                          int_column<primary_key> {.Name = "ID", .NotNull = true},
                                          text_column {.Name = "Code"})};
            REQUIRE(db.table_exists(tableName1));

            auto tup0 = std::tuple {1, "UK"};
            auto tup1 = std::tuple {2, "USA"};

            REQUIRE(dbTable->insert_into("ID", "Code")(tup0, tup1));
        }

        auto rows = db.get_table(tableName0)->select_from<string, string>("Name", "Code").left_join(tableName1, "People.CountryID = Countries.ID")();
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

            auto tup0 = std::tuple {"Peter", 1};
            auto tup1 = std::tuple {"Paul", 1};
            auto tup2 = std::tuple {"Marie", 2};

            REQUIRE(dbTable->insert_into("Name", "CountryID")(tup0, tup1, tup2));
        }
        {
            auto dbTable {db.create_table(tableName1,
                                          int_column<primary_key> {.Name = "ID", .NotNull = true},
                                          text_column {.Name = "Code"})};
            REQUIRE(db.table_exists(tableName1));

            auto tup0 = std::tuple {1, "UK"};

            REQUIRE(dbTable->insert_into("ID", "Code")(tup0));
        }

        auto rows = db.get_table(tableName0)
                        ->select_from<string, string>("Name", "Code")
                        .inner_join(tableName1, "People.CountryID = Countries.ID")();
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

        auto rows = db.get_table(tableName0)->select_from<string, i32>("Color", "Number").cross_join(tableName1)();
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
                                  int_column {.Name = "Height"})};
    REQUIRE(dbTable);

    auto tup0 = std::tuple {1, "A", 100, 1.5f};
    REQUIRE(dbTable->insert_into("ID", "Name", "Age", "Height")(tup0));
    auto rows = dbTable->select_from<i32, string, i32, f32>("ID", "Name", "Age", "Height")();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0] == tup0);

    {
        auto sp = db.create_savepoint("t1");

        auto tup1 = std::tuple {2, "B", 200, 4.5f};
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
