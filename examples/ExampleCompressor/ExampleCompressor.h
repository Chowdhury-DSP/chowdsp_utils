#pragma once

#include "Params.h"
#include <chowdsp_compressor/chowdsp_compressor.h>
#include <chowdsp_visualizers/chowdsp_visualizers.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

namespace chow_comp = chowdsp::compressor;
using LevelDetector = chow_comp::CompressorLevelDetector<float,
                                                         types_list::TypesList<chow_comp::PeakDetector,
                                                                               chow_comp::RMSDetector,
                                                                               chow_comp::PeakRtTDetector>,
                                                         chow_comp::LevelDetectorVisualizer>;
using GainComputer = chow_comp::GainComputer<float,
                                             types_list::TypesList<chow_comp::FeedForwardCompGainComputer<float>,
                                                                   chow_comp::FeedBackCompGainComputer<float>>>;

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
