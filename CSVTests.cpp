#include "tests.hpp"

using namespace tcob::data;

TEST_CASE("Data.CSV.Parse")
{
    SUBCASE("default")
    {
        {
            std::string csvString {"A, B, C, D, E, F \n"
                                   "1, 2, 3, 4, 5, 6 \n"
                                   "7, 8, 9, 10, 11, 12 "};

            csv_table tab;
            REQUIRE(tab.parse(csvString, {}));
            REQUIRE(tab.Rows.height() == 2);
            REQUIRE(tab.Rows.width() == 6);
            REQUIRE(tab.Header == std::vector<std::string> {"A", "B", "C", "D", "E", "F"});
            REQUIRE(tab.Rows[0, 0] == "1");
            REQUIRE(tab.Rows[1, 0] == "2");
            REQUIRE(tab.Rows[2, 0] == "3");
            REQUIRE(tab.Rows[3, 0] == "4");
            REQUIRE(tab.Rows[4, 0] == "5");
            REQUIRE(tab.Rows[5, 0] == "6");
            REQUIRE(tab.Rows[0, 1] == "7");
            REQUIRE(tab.Rows[1, 1] == "8");
            REQUIRE(tab.Rows[2, 1] == "9");
            REQUIRE(tab.Rows[3, 1] == "10");
            REQUIRE(tab.Rows[4, 1] == "11");
            REQUIRE(tab.Rows[5, 1] == "12");
        }
        {
            std::string csvString {"a,b,c,d,e,f,g\n 1,2,3,4,5,6,7\n 8,9,10,11,12,13,14\n 15,16,17,18,19,20,21\n 22,23,24,25,26,27,28\n 29,30,31,32,33,34,35\n 36,37,38,39,40,41,42\n 43,44,45,46,47,48,49\n 50,51,52,53,54,55,56\n 57,58,59,60,61,62,63\n 64,65,66,67,68,69,70\n 71,72,73,74,75,76,77\n 78,79,80,81,82,83,84\n 85,86,87,88,89,90,91\n 92,93,94,95,96,97,98\n 99,100,101,102,103,104,105\n 106,107,108,109,110,111,112\n 113,114,115,116,117,118,119\n 120,121,122,123,124,125,126\n 127,128,129,130,131,132,133\n 134,135,136,137,138,139,140\n 141,142,143,144,145,146,147\n 148,149,150,151,152,153,154\n 155,156,157,158,159,160,161\n 162,163,164,165,166,167,168\n 169,170,171,172,173,174,175\n 176,177,178,179,180,181,182\n 183,184,185,186,187,188,189\n 190,191,192,193,194,195,196\n 197,198,199,200,201,202,203\n 204,205,206,207,208,209,210\n 211,212,213,214,215,216,217\n 218,219,220,221,222,223,224\n 225,226,227,228,229,230,231\n 232,233,234,235,236,237,238"};
            csv_table   tab;
            REQUIRE(tab.parse(csvString, {}));
            REQUIRE(tab.Rows.height() == 34);
            REQUIRE(tab.Rows.width() == 7);
            for (isize i {0}; i < tab.Rows.count(); ++i) {
                REQUIRE(tab.Rows[i] == std::to_string(i + 1));
            }
        }
    }
    SUBCASE("without header")
    {
        {
            std::string csvString {"1,2,3,4,5,6,7\n 8,9,10,11,12,13,14\n 15,16,17,18,19,20,21\n"};

            csv_table           tab;
            csv_table::settings set {
                .HasHeader = false,
                .Separator = ',',
                .Quote     = '"',
            };
            REQUIRE(tab.parse(csvString, set));
            REQUIRE(tab.Rows.height() == 3);
            REQUIRE(tab.Rows.width() == 7);
            REQUIRE(tab.Header.empty());
            for (isize i {0}; i < tab.Rows.count(); ++i) {
                REQUIRE(tab.Rows[i] == std::to_string(i + 1));
            }
        }
    }
    SUBCASE("quotes")
    {
        {
            std::string csvString {"A,B,\"C \" \n 1, 2, \"OK\""};

            csv_table tab;
            REQUIRE(tab.parse(csvString, {}));
            REQUIRE(tab.Rows.height() == 1);
            REQUIRE(tab.Header[2] == "C ");
            REQUIRE(tab.Rows[2, 0] == "OK");
        }
    }
}

TEST_CASE("Data.CSV.Save")
{
    SUBCASE("file")
    {

        std::string csvString {"A, B, C, D, E, F \n"
                               "1, 2, 3, 4, 5, 6 "};
        csv_table   tab0;
        REQUIRE(tab0.parse(csvString, {}));
        REQUIRE(tab0.save("test0.csv", {}));

        csv_table tab1;
        REQUIRE(tab1.load("test0.csv", {}));
        REQUIRE(tab1.Rows.height() == 1);
        REQUIRE(tab1.Rows.width() == 6);
        REQUIRE(tab1.Header == std::vector<std::string> {"A", "B", "C", "D", "E", "F"});
        REQUIRE(std::vector<std::string>(tab1.Rows.row(0).begin(), tab1.Rows.row(0).end()) == std::vector<std::string> {"1", "2", "3", "4", "5", "6"});
    }
    SUBCASE("memory")
    {
        io::iomstream stream {};

        std::string csvString {"A, B, C, D, E, F \n"
                               "1, 2, 3, 4, 5, 6 "};
        csv_table   tab0;
        REQUIRE(tab0.parse(csvString, {}));
        REQUIRE(tab0.save(stream, {}));

        stream.seek(0, io::seek_dir::Begin);

        csv_table tab1;
        REQUIRE(tab1.load(stream, {}));
        REQUIRE(tab1.Rows.height() == 1);
        REQUIRE(tab1.Rows.width() == 6);
        REQUIRE(tab1.Header == std::vector<std::string> {"A", "B", "C", "D", "E", "F"});
        REQUIRE(std::vector<std::string>(tab1.Rows.row(0).begin(), tab1.Rows.row(0).end()) == std::vector<std::string> {"1", "2", "3", "4", "5", "6"});
    }
}
