#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

TEST_CASE ("Width Panner Test", "[dsp][misc]")
{
    const auto spec = juce::dsp::ProcessSpec { 48000.0, 4, 2 };

    chowdsp::Buffer<float> buffer { 2, 4 };
    for (auto [ch, data] : chowdsp::buffer_iters::channels (buffer))
        std::fill (data.begin(), data.end(), ch == 0 ? -1.0f : 1.0f);

    const auto testPanner = [spec, &buffer] (chowdsp::PanningRule rule, float pan, auto&& getExpected)
    {
        chowdsp::WidthPanner<float> panner;
        panner.setRule (rule);
        panner.setPan (pan);
        panner.prepare (spec);
        panner.processBlock (buffer);
        for (auto [ch, data] : chowdsp::buffer_iters::channels (std::as_const (buffer)))
        {
            const auto expected = getExpected (ch);
            for (auto& x : data)
                REQUIRE (x == Catch::Approx { expected }.margin (1.0e-3));
        }
    };

    SECTION ("Stereo -> Mono")
    {
        testPanner (chowdsp::PanningRule::linear, 0.0f, [] (int)
                    { return 0.0f; });
        testPanner (chowdsp::PanningRule::sin3dB, 0.0f, [] (int)
                    { return 0.0f; });
        testPanner (chowdsp::PanningRule::sin6dB, 0.0f, [] (int)
                    { return 0.0f; });
    }

    SECTION ("Stereo -> Stereo")
    {
        testPanner (chowdsp::PanningRule::linear, 1.0f, [] (int ch)
                    { return ch == 0 ? -1.0f : 1.0f; });
        testPanner (chowdsp::PanningRule::sin3dB, 1.0f, [] (int ch)
                    { return ch == 0 ? -1.0f : 1.0f; });
        testPanner (chowdsp::PanningRule::sin6dB, 1.0f, [] (int ch)
                    { return ch == 0 ? -1.0f : 1.0f; });
    }

    SECTION ("Stereo -> Inverse Stereo")
    {
        testPanner (chowdsp::PanningRule::sin4p5dB, -1.0f, [] (int ch)
                    { return ch == 0 ? 1.0f : -1.0f; });
        testPanner (chowdsp::PanningRule::squareRoot3dB, -1.0f, [] (int ch)
                    { return ch == 0 ? -1.0f : 1.0f; });
        testPanner (chowdsp::PanningRule::squareRoot4p5dB, -1.0f, [] (int ch)
                    { return ch == 0 ? 1.0f : -1.0f; });
    }

    SECTION ("Stereo -> Half-Mid (Linear)")
    {
        testPanner (chowdsp::PanningRule::linear, 0.5f, [] (int ch)
                    { return ch == 0 ? -0.5f : 0.5f; });
    }

    SECTION ("Stereo -> Half-Mid (Sin 6dB)")
    {
        testPanner (chowdsp::PanningRule::sin6dB, 0.5f, [] (int ch)
                    { return ch == 0 ? -0.7071f : 0.7071f; });
    }
}
