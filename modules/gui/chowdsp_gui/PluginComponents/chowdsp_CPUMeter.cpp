#include "chowdsp_CPUMeter.h"

namespace chowdsp
{
#ifndef DOXYGEN
class CPUMeterLNF : public juce::LookAndFeel_V4
{
public:
    CPUMeterLNF() = default;
    ~CPUMeterLNF() override = default;

    void drawProgressBar (juce::Graphics& g, juce::ProgressBar& progressBar, int width, int height, double progress, const juce::String& textToShow) override
    {
        auto background = progressBar.findColour (juce::ProgressBar::backgroundColourId);
        auto foreground = progressBar.findColour (juce::ProgressBar::foregroundColourId);

        auto barBounds = progressBar.getLocalBounds().toFloat();
        const auto cornerSize = (float) progressBar.getHeight() * 0.1f;

        g.setColour (background);
        g.fillRoundedRectangle (barBounds, cornerSize);

        {
            juce::Path p;
            p.addRoundedRectangle (barBounds, cornerSize);
            g.reduceClipRegion (p);

            barBounds.setWidth (barBounds.getWidth() * (float) progress);
            g.setColour (foreground);
            g.fillRoundedRectangle (barBounds, cornerSize);
        }

        if (textToShow.isNotEmpty())
        {
            g.setColour (juce::Colours::white);
            g.setFont ((float) height * 0.6f);

            g.drawText (textToShow, 0, 0, width, height, juce::Justification::centred, false);
        }
    }
};
#endif // DOXYGEN

CPUMeter::CPUMeter (const juce::AudioProcessLoadMeasurer& lm) : loadMeasurer (lm)
{
    cpuMeterLNF = std::make_unique<CPUMeterLNF>();
    setLookAndFeel (cpuMeterLNF.get());

    addAndMakeVisible (progress);

    constexpr double sampleRate = 20.0;
    startTimerHz ((int) sampleRate);

    smoother.prepare ({ sampleRate, 128, 1 });
    smoother.setAttackTime (500.0);
    smoother.setReleaseTime (1000.0);
}

CPUMeter::~CPUMeter()
{
    setLookAndFeel (nullptr);
    cpuMeterLNF.reset();
}

void CPUMeter::colourChanged()
{
    progress.setColour (juce::ProgressBar::backgroundColourId, findColour (juce::ProgressBar::backgroundColourId));
    progress.setColour (juce::ProgressBar::foregroundColourId, findColour (juce::ProgressBar::foregroundColourId));
}

void CPUMeter::timerCallback()
{
    const auto newLoadProportion = smoother.processSample (0, loadMeasurer.getLoadAsProportion());
    loadProportion = std::round (newLoadProportion * 100.0) * 0.01;
}

void CPUMeter::resized()
{
    progress.setBounds (getLocalBounds());
}
} // namespace chowdsp
