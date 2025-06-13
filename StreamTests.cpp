#include "tests.hpp"

#include <string>

TEST_CASE("IO.Stream.ReadAll")
{
    SUBCASE("file_sink")
    {
        std::string const file {"test.ReadAll"};
        PrepareFile(file);

        SUBCASE("char")
        {
            {
                io::ofstream        fs {file};
                std::array<char, 3> data0 {'1', '2', '3'};
                fs.write<char>(data0);
                std::vector<char> data1 {'4', '5'};
                fs.write<char>(data1);
            }
            REQUIRE(io::get_file_size(file) == 5 * sizeof(char));
            {
                io::ifstream      fs {file};
                std::vector<char> data = fs.read_all<char>();
                REQUIRE(data == (std::vector<char> {'1', '2', '3', '4', '5'}));
            }
        }
        SUBCASE("i32")
        {
            {
                io::ofstream           fs {file};
                std::vector<i32> const data {10, 20, 33, 45, 59};
                fs.write<i32>(data);
            }
            REQUIRE(io::get_file_size(file) == 5 * sizeof(i32));
            {
                io::ifstream     fs {file};
                std::vector<i32> data = fs.read_all<i32>();
                REQUIRE(data == (std::vector<i32> {10, 20, 33, 45, 59}));
            }
        }
        SUBCASE("f32")
        {
            {
                io::ofstream fs {file};
                fs << 1.0f << 2.0f << 4.0f;
            }
            REQUIRE(io::get_file_size(file) == 3 * sizeof(f32));
            {
                io::ifstream fs {file};

                f32 a {0}, b {0}, c {0};
                fs >> a >> b >> c;

                REQUIRE(a == 1.0f);
                REQUIRE(b == 2.0);
                REQUIRE(c == 4.0f);
            }
        }
    }
    SUBCASE("span_sink")
    {
        SUBCASE("char")
        {
            std::vector<byte> out;
            out.resize(5);

            {
                io::osstream        fs {out};
                std::array<byte, 3> data0 {'1', '2', '3'};
                fs.write<byte>(data0);
                std::vector<byte> data1 {'4', '5'};
                fs.write<byte>(data1);
            }

            {
                io::isstream      fs {out};
                std::vector<byte> in = fs.read_all<byte>();
                REQUIRE(in == (std::vector<byte> {'1', '2', '3', '4', '5'}));
                REQUIRE(out == in);
            }
        }
    }
    SUBCASE("memory_sink")
    {
        SUBCASE("char")
        {
            io::iomstream fs {};

            {
                std::array<char, 3> data0 {'1', '2', '3'};
                fs.write<char>(data0);
                std::vector<char> data1 {'4', '5'};
                fs.write<char>(data1);
            }

            {
                fs.seek(0, io::seek_dir::Begin);
                std::vector<char> in = fs.read_all<char>();
                REQUIRE(in == (std::vector<char> {'1', '2', '3', '4', '5'}));
            }
        }
    }
}

TEST_CASE("IO.Stream.ReadString")
{
    SUBCASE("file_sink")
    {
        std::string const file {"test.ReadString"};
        PrepareFile(file);

        std::string line0 {"test"};
        std::string line1 {"123"};
        std::string line2 {"abc"};

        {
            io::ofstream fs {file};
            fs.write(line0);
            fs.write("\n");
            fs.write(line1);
            fs.write("\n");
            fs.write(line2);
        }
        REQUIRE(io::get_file_size(file) == 12);
        {
            io::ifstream fs {file};
            REQUIRE(fs.read_string_until('\n') == line0);
            REQUIRE(fs.read_string_until('\n') == line1);
            REQUIRE(fs.read_string_until('\n') == line2);
        }
        {
            io::ifstream fs {file};
            REQUIRE(fs.read_string(static_cast<std::streamsize>(line0.size() + 1)) == line0 + "\n");
            REQUIRE(fs.read_string(static_cast<std::streamsize>(line1.size() + 1)) == line1 + "\n");
            REQUIRE(fs.read_string(static_cast<std::streamsize>(line2.size())) == line2);
        }
    }
    SUBCASE("span_sink")
    {
        std::string line0 {"test"};
        std::string line1 {"123"};
        std::string line2 {"abc"};

        std::vector<byte> out;
        out.resize(50);

        {
            io::osstream fs {out};
            fs.write(line0);
            fs.write("\n");
            fs.write(line1);
            fs.write("\n");
            fs.write(line2);
            fs.write("\n");
        }

        {
            io::isstream fs {out};
            REQUIRE(fs.read_string_until('\n') == line0);
            REQUIRE(fs.read_string_until('\n') == line1);
            REQUIRE(fs.read_string_until('\n') == line2);
        }
        {
            io::isstream fs {out};
            REQUIRE(fs.read_string(static_cast<std::streamsize>(line0.size() + 1)) == line0 + "\n");
            REQUIRE(fs.read_string(static_cast<std::streamsize>(line1.size() + 1)) == line1 + "\n");
            REQUIRE(fs.read_string(static_cast<std::streamsize>(line2.size())) == line2);
        }
    }
    SUBCASE("memory_sink")
    {
        std::string line0 {"test"};
        std::string line1 {"123"};
        std::string line2 {"abc"};

        io::iomstream fs {};

        {

            fs.write(line0);
            fs.write("\n");
            fs.write(line1);
            fs.write("\n");
            fs.write(line2);
            fs.write("\n");
        }

        {
            fs.seek(0, io::seek_dir::Begin);
            REQUIRE(fs.read_string_until('\n') == line0);
            REQUIRE(fs.read_string_until('\n') == line1);
            REQUIRE(fs.read_string_until('\n') == line2);
        }
        {
            fs.seek(0, io::seek_dir::Begin);
            REQUIRE(fs.read_string(static_cast<std::streamsize>(line0.size() + 1)) == line0 + "\n");
            REQUIRE(fs.read_string(static_cast<std::streamsize>(line1.size() + 1)) == line1 + "\n");
            REQUIRE(fs.read_string(static_cast<std::streamsize>(line2.size())) == line2);
        }
    }
}

TEST_CASE("IO.Stream.Open")
{
    SUBCASE("ifstream::Open")
    {
        std::string const file {"test.Open"};

        io::delete_file(file);
        REQUIRE_FALSE(io::exists(file));

        auto stream0 {io::ifstream::Open(file)};
        REQUIRE_FALSE(stream0);

        PrepareFile(file);

        auto stream1 {io::ifstream::Open(file)};
        REQUIRE(stream1);
        REQUIRE(stream1->is_valid());
    }
    SUBCASE("ctor")
    {
        std::string const file {"test.OpenBlob"};

        io::delete_file(file);
        REQUIRE_FALSE(io::exists(file));

        io::ifstream stream0 {file};
        REQUIRE_FALSE(stream0);
        REQUIRE_FALSE(stream0.is_valid());
    }
}

TEST_CASE("IO.Stream.Appending")
{
    SUBCASE("file_sink")
    {
        std::string const file {"test.Append"};
        PrepareFile(file);

        {
            io::ofstream      fs {file};
            std::vector<char> data {'1', '2', '3', '4', '5'};
            fs.write<char>(data);
        }

        {
            io::ofstream      fs {file, 0, true};
            std::vector<char> data {'5', '4', '3', '2', '1'};
            fs.write<char>(data);
        }

        REQUIRE(io::get_file_size(file) == 10);
        {
            io::ifstream      fs {file};
            std::vector<char> data = fs.read_all<char>();
            REQUIRE(data == (std::vector<char> {'1', '2', '3', '4', '5', '5', '4', '3', '2', '1'}));
        }
    }
}

TEST_CASE("IO.Stream.Seeking")
{
    std::vector<char> data {'1', '2', '3', '4', '5'};

    auto seekCurrent {[](io::istream& str) {
        REQUIRE(str.read<char>() == '1');

        REQUIRE(str.seek(1, io::seek_dir::Current));
        REQUIRE(str.read<char>() == '3');

        REQUIRE(str.seek(-2, io::seek_dir::Current));
        REQUIRE(str.read<char>() == '2');
    }};
    auto seekBegin {[](io::istream& str) {
        REQUIRE(str.seek(0, io::seek_dir::Begin));
        REQUIRE(str.read<char>() == '1');

        REQUIRE(str.seek(2, io::seek_dir::Begin));
        REQUIRE(str.read<char>() == '3');

        REQUIRE(str.seek(4, io::seek_dir::Begin));
        REQUIRE(str.read<char>() == '5');

        REQUIRE_FALSE(str.seek(-1, io::seek_dir::Begin));
    }};
    auto seekEnd {[](io::istream& str) {
        REQUIRE(str.seek(-1, io::seek_dir::End));
        REQUIRE(str.read<char>() == '5');

        REQUIRE(str.seek(-3, io::seek_dir::End));
        REQUIRE(str.read<char>() == '3');

        REQUIRE(str.seek(-5, io::seek_dir::End));
        REQUIRE(str.read<char>() == '1');
    }};

    SUBCASE("file_sink")
    {
        std::string const file {"test.Seeking"};
        PrepareFile(file);
        {
            io::ofstream fs {file};
            fs.write<char>(data);
        }
        REQUIRE(io::get_file_size(file) == 5);
        {
            io::ifstream fs {file};
            seekCurrent(fs);
        }
        {
            io::ifstream fs {file};
            seekBegin(fs);
        }
        {
            io::ifstream fs {file};
            seekEnd(fs);
        }
    }
    SUBCASE("span_sink")
    {
        std::vector<byte> out;
        out.resize(5);
        {
            io::osstream fs {out};
            fs.write<char>(data);
        }
        {
            io::isstream fs {out};
            seekCurrent(fs);
        }
        {
            io::isstream fs {out};
            seekBegin(fs);
        }
        {
            io::isstream fs {out};
            seekEnd(fs);
        }
    }
    SUBCASE("memory_sink")
    {
        io::iomstream fs {};
        fs.write<char>(data);

        fs.seek(0, io::seek_dir::Begin);
        seekCurrent(fs);
        fs.seek(0, io::seek_dir::Begin);
        seekBegin(fs);
        fs.seek(0, io::seek_dir::Begin);
        seekEnd(fs);
    }
}

struct doubler {
    auto to(std::span<byte const> bytes) const -> std::vector<byte>
    {
        std::vector<byte> retValue;
        retValue.reserve(bytes.size() * 2);
        for (auto b : bytes) {
            retValue.push_back(b);
            retValue.push_back(b);
        }

        return retValue;
    }
};
struct halfer {
    auto from(std::span<byte const> bytes) const -> std::vector<byte>
    {
        std::vector<byte> retValue;
        retValue.reserve(bytes.size() * 2);
        for (usize i {0}; i < bytes.size(); i += 2) {
            retValue.push_back(bytes[i]);
        }

        return retValue;
    }
};

TEST_CASE("IO.Stream.Filter")
{
    SUBCASE("zlib")
    {
        io::iomstream stream {};

        std::vector<byte> compress;
        compress.reserve(100000);

        for (int x = 0; x < 100000; x++) {
            compress.push_back('a');
        }
        REQUIRE(stream.write_filtered<byte>(compress, io::zlib_filter {}) != -1);

        stream.seek(0, io::seek_dir::Begin);
        auto data {stream.read_filtered<byte>(stream.size_in_bytes(), io::zlib_filter {})};
        REQUIRE(data == compress);
    }

    SUBCASE("base64")
    {
        io::iomstream stream {};

        std::string       s {"hello world"};
        std::vector<byte> input(s.begin(), s.end());

        stream.write_filtered<byte>(input, io::base64_filter {});

        std::vector<byte> expected {'a', 'G', 'V', 's', 'b', 'G', '8', 'g', 'd', '2', '9', 'y', 'b', 'G', 'Q', '='};
        REQUIRE(stream.size_in_bytes() == expected.size());
        stream.seek(0, io::seek_dir::Begin);
        REQUIRE(stream.read_all<byte>() == expected);

        stream.seek(0, io::seek_dir::Begin);
        auto data {stream.read_filtered<byte>(stream.size_in_bytes(), io::base64_filter {})};
        REQUIRE(data == input);
    }

    SUBCASE("z85")
    {
        io::iomstream stream {};

        std::string       s {"hello world1"};
        std::vector<byte> input(s.begin(), s.end());

        stream.write_filtered<byte>(input, io::z85_filter {});

        std::vector<byte> expected {'x', 'K', '#', '0', '@', 'z', 'Y', '<', 'm', 'x', 'A', '+', ']', 'n', 'v'};
        REQUIRE(stream.size_in_bytes() == expected.size());
        stream.seek(0, io::seek_dir::Begin);
        REQUIRE(stream.read_all<byte>() == expected);

        stream.seek(0, io::seek_dir::Begin);
        auto data {stream.read_filtered<byte>(stream.size_in_bytes(), io::z85_filter {})};
        REQUIRE(data == input);
    }

    SUBCASE("reverser")
    {
        io::iomstream stream {};

        std::string       s {"hello world"};
        std::vector<byte> input(s.begin(), s.end());

        stream.write_filtered<byte>(input, io::reverser_filter {});

        std::vector<byte> expected {'d', 'l', 'r', 'o', 'w', ' ', 'o', 'l', 'l', 'e', 'h'};
        REQUIRE(stream.size_in_bytes() == expected.size());
        stream.seek(0, io::seek_dir::Begin);
        REQUIRE(stream.read_all<byte>() == expected);

        stream.seek(0, io::seek_dir::Begin);
        auto data {stream.read_filtered<byte>(stream.size_in_bytes(), io::reverser_filter {})};
        REQUIRE(data == input);
    }

    SUBCASE("chain")
    {
        io::iomstream stream {};

        std::string       s {"hello"};
        std::vector<byte> input(s.begin(), s.end());

        stream.write_filtered<byte>(input, io::reverser_filter {}, doubler {});

        std::vector<byte> expected {'o', 'o', 'l', 'l', 'l', 'l', 'e', 'e', 'h', 'h'};
        REQUIRE(stream.size_in_bytes() == expected.size());
        stream.seek(0, io::seek_dir::Begin);
        REQUIRE(stream.read_all<byte>() == expected);

        stream.seek(0, io::seek_dir::Begin);
        auto data {stream.read_filtered<byte>(stream.size_in_bytes(), io::reverser_filter {}, halfer {})};
        REQUIRE(data == input);
    }
}
