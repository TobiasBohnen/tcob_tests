

#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>

#include <tcob/tcob.hpp>

extern "C" auto LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) -> int
{
    using namespace tcob;
    auto pl {platform::HeadlessInit("tcob_tests.log")};

    try {
        io::isstream stream {std::span<std::byte const> {reinterpret_cast<std::byte const*>(data), size}};
        stream.seek(0, io::seek_dir::Begin);
        gfx::image s;
        std::ignore = s.load(stream, ".gif");
    } catch (...) {
        // Never allow exceptions to escape
    }

    return 0;
}