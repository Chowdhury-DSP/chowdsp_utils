#pragma once

#include "Params.h"
#include <chowdsp_compressor/chowdsp_compressor.h>
#include <chowdsp_visualizers/chowdsp_visualizers.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

namespace chow_comp = chowdsp::compressor;

// The chowdsp::compressor module has some classes which set up a generic compressor
// architecture. There's some implementations which provide basic compressor-like
// functionality, but the cool thing is how easy it is to extend the classes with
// some custom compressor algorithms.

// For example, here we define a peak detector which operates just like the provided
// PeakDetector, but with slower time constants:
struct SlowPeakDetector : chow_comp::PeakDetector
{
    float modifyAttack (float attackMs) // NOLINT
    {
        return attackMs * 4.0f;
    }

    float modifyRelease (float releaseMs) // NOLINT
    {
        return releaseMs * 4.0f;
    }
};

// Now we can define a generic LevelDetector, which can choose between several implementations...
using LevelDetector = chow_comp::CompressorLevelDetector<float,
                                                         types_list::TypesList<chow_comp::PeakDetector,
                                                                               chow_comp::RMSDetector,
                                                                               chow_comp::PeakRtTDetector,
                                                                               SlowPeakDetector>,
                                                         chow_comp::LevelDetectorVisualizer>;

static_assert (LevelDetector::HasModifyAttack<SlowPeakDetector>);
static_assert (LevelDetector::HasModifyRelease<SlowPeakDetector>);

// ... and do the same thing for our gain computer
using GainComputer = chow_comp::GainComputer<float,
                                             types_list::TypesList<chow_comp::FeedForwardCompGainComputer<float>,
                                                                   chow_comp::FeedBackCompGainComputer<float>>>;

// Finally we have a compressor DSP class that utilises the above defined types!
using CompressorDSP = chow_comp::MonoCompressor<float,
                                                LevelDetector,
                                                GainComputer,
                                                chow_comp::GainReductionMeter::BackgroundTask>;

class ExampleCompressorPlugin : public chowdsp::PluginBase<chowdsp::PluginStateImpl<Params>>
{
public:
    ExampleCompressorPlugin() = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    juce::AudioProcessorEditor* createEditor() override;

    CompressorDSP compressor;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExampleCompressorPlugin)
};
