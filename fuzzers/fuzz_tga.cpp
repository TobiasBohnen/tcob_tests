

#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>

#include <tcob/tcob.hpp>

using namespace tcob;
static std::shared_ptr<tcob::platform> pl;

extern "C" auto LLVMFuzzerInitialize(int* /* argc */, char*** /* argv */) -> int
{
    pl = tcob::platform::HeadlessInit("tcob_tests.log");
    return 0;
}

extern "C" auto LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) -> int
{
    try {
        io::isstream stream {std::span<std::byte const> {reinterpret_cast<std::byte const*>(data), size}};
        stream.seek(0, io::seek_dir::Begin);
        gfx::image s;
        std::ignore = s.load(stream, ".tga");
    } catch (...) {
        // Never allow exceptions to escape
    }

    return 0;
}