#include "VizTestUtils.h"
#include <chowdsp_visualizers/chowdsp_visualizers.h>

TEST_CASE ("Level Detector Test", "[visualizers][compressor]")
{
    static constexpr int width = 500;
    static constexpr int height = 300;
    static constexpr int block_size = 512;

    chowdsp::compressor::LevelDetectorVisualizer visualizer;
    visualizer.setBufferSize (100);
    visualizer.setSamplesPerBlock (block_size);

    float phase = 0.0f;
    for (int i = 0; i < 100; ++i)
    {
        std::array<float, block_size> buffer {};
        for (auto& x : buffer)
        {
            x = 0.5f * std::sin (phase);
            phase += 0.08f;
        }
        visualizer.pushChannel (0, nonstd::span<const float> { buffer });

        buffer.fill (0.5f);
        visualizer.pushChannel (1, nonstd::span<const float> { buffer });
    }

    visualizer.setSize (width, height);
    const auto testScreenshot = visualizer.createComponentSnapshot ({ width, height });
    // VizTestUtils::saveImage (testScreenshot, "level_detector.png");

    const auto refScreenshot = VizTestUtils::loadImage ("level_detector.png");
    VizTestUtils::compareImages (testScreenshot, refScreenshot);
}
