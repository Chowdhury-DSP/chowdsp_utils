#include "chowdsp_LevelDetectorVisualizer.h"

namespace chowdsp::compressor
{
void LevelDetectorVisualizer::paintChannel (int channelIndex, juce::Graphics& g, juce::Rectangle<float> area, const juce::Range<float>* levels, int numLevels, int nextSample)
{
    if (channelIndex == 0)
        g.setColour (audioColour);
    else if (channelIndex == 1)
        g.setColour (levelColour);

    juce::Path path;
    path.preallocateSpace (4 * numLevels + 8);

    for (int i = 0; i < numLevels; ++i)
    {
        auto level = -(levels[(nextSample + i) % numLevels].getEnd());

        if (i == 0)
            path.startNewSubPath (0.0f, level);
        else
            path.lineTo ((float) i, level);
    }

    for (int i = numLevels; --i >= 0;)
        path.lineTo ((float) i, -(levels[(nextSample + i) % numLevels].getStart()));

    path.applyTransform (juce::AffineTransform::fromTargetPoints (0.0f,
                                                                  -params.yMax,
                                                                  area.getX(),
                                                                  area.getY(),
                                                                  0.0f,
                                                                  -params.yMin,
                                                                  area.getX(),
                                                                  area.getBottom(),
                                                                  (float) numLevels,
                                                                  -params.yMax,
                                                                  area.getRight(),
                                                                  area.getY()));

    if (channelIndex == 0)
    {
        path.closeSubPath();
        g.fillPath (path);
    }
    else
    {
        g.strokePath (path, juce::PathStrokeType { 2.5f });
    }
}
} // namespace chowdsp::compressor
