#include "VizTestUtils.h"
#include <chowdsp_visualizers/chowdsp_visualizers.h>

TEST_CASE ("Waveform View Test", "[visualizers]")
{
    static constexpr int width = 500;
    static constexpr int height = 300;
    static constexpr int block_size = 512;

    chowdsp::WaveformView<1> waveform;
    waveform.setBufferSize (100);
    waveform.setSamplesPerBlock (block_size);

    float phase = 0.0f;
    for (int i = 0; i < 100; ++i)
    {
        std::array<float, block_size> buffer {};
        for (auto& x : buffer)
        {
            x = 0.5f * std::sin (phase);
            phase += 0.08f;
        }
        waveform.pushBuffer (chowdsp::BufferView<const float> { buffer.data(), block_size });
    }

    waveform.setSize (width, height);
    const auto testScreenshot = waveform.createComponentSnapshot ({ width, height });
    // VizTestUtils::saveImage (testScreenshot, "waveform_view.png");

    const auto refScreenshot = VizTestUtils::loadImage ("waveform_view.png");
    VizTestUtils::compareImages (testScreenshot, refScreenshot);
}
