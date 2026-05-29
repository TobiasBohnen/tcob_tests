#include <cstddef>
#include <cstdint>
#include <print>
#include <span>
#include <tuple>

#include <tcob/tcob.hpp>

using namespace tcob;
static std::shared_ptr<tcob::platform> pl;
static std::string                     g_ext;

extern "C" auto LLVMFuzzerInitialize(int* argc, char*** argv) -> int // NOLINT
{
    pl = tcob::platform::HeadlessInit("tcob_tests.log");
    io::magic::clear_signatures();

    for (int i {1}; i < *argc; ++i) {
        std::string_view const arg {(*argv)[i]};
        if (arg.starts_with("--ext=")) {
            std::string_view val {arg.substr(6)};
            g_ext = val.starts_with('.') ? std::string {val} : std::format(".{}", val);
        }
    }

    if (g_ext.empty()) {
        std::println(stderr, "Usage: fuzz_audio --ext=<[FORMAT]>");
        pl = nullptr;
        std::exit(1);
    }
    if (locate_service<audio::decoder::factory>().create(g_ext) == nullptr) {
        std::println(stderr, "Error: no decoder registered for '{}'", g_ext);
        pl = nullptr;
        std::exit(1);
    }
    return 0;
}

extern "C" auto LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) -> int
{
    try {
        std::shared_ptr<io::isstream> stream {std::make_shared<io::isstream>(std::span<std::byte const> {
            reinterpret_cast<std::byte const*>(data), size})};
        stream->seek(0, io::seek_dir::Begin);
        audio::buffer s;
        std::ignore = s.load(stream, g_ext, {});
    } catch (...) {
    }
    return 0;
}