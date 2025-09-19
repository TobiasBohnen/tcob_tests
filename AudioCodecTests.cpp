#include "tests.hpp"

TEST_CASE("Audio.Decoder.BSA")
{
    using namespace audio;

    specification info;
    info.SampleRate = 44100;
    info.Channels   = 2;
    usize const frameCount {100};

    std::vector<f32> samples;
    samples.resize(frameCount * info.Channels);
    for (usize i {0}; i < samples.size(); ++i) {
        samples[i] = std::sin(static_cast<f32>(i) * 0.1f);
    }

    auto stream {std::make_shared<io::iomstream>()};

    {
        REQUIRE(buffer::Create(info, samples).save(*stream, ".bsa"));
    }

    stream->seek(0, io::seek_dir::Begin);

    auto buffer {buffer::Load(stream, ".bsa")};
    REQUIRE(buffer);
    REQUIRE(buffer->info().Specs.Channels == info.Channels);
    REQUIRE(buffer->info().Specs.SampleRate == info.SampleRate);
    REQUIRE(buffer->info().FrameCount == frameCount);

    REQUIRE(buffer->data().size() == static_cast<isize>(samples.size()));

    for (usize i {0}; i < samples.size(); ++i) {
        REQUIRE(buffer->data()[i] == doctest::Approx(samples[i]).epsilon(0.001f));
    }
}
