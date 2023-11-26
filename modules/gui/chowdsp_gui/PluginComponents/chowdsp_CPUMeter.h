#pragma once

#if JUCE_MODULE_AVAILABLE_juce_dsp
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wimplicit-const-int-float-conversion")
#include <juce_dsp/juce_dsp.h>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

namespace chowdsp
{
/** Simple progress bar to visualize CPU usage */
class CPUMeter : public juce::Component,
                 private juce::Timer
{
public:
    explicit CPUMeter (const juce::AudioProcessLoadMeasurer& loadMeasurer);
    ~CPUMeter() override;

    void colourChanged() override;
    void resized() override;

private:
    void timerCallback() override;

    double loadProportion = 0.0;
    juce::ProgressBar progress { loadProportion };
    juce::dsp::BallisticsFilter<double> smoother;

    const juce::AudioProcessLoadMeasurer& loadMeasurer;

    std::unique_ptr<juce::LookAndFeel> cpuMeterLNF;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CPUMeter)
};
} // namespace chowdsp

#endif
