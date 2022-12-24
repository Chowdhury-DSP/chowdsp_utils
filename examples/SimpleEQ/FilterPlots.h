#pragma once

#include <chowdsp_eq/chowdsp_eq.h>
#include <chowdsp_visualizers/chowdsp_visualizers.h>

class FilterPlots : public chowdsp::EQ::EqualizerPlotWithParameters<3>
{
public:
    FilterPlots (chowdsp::PluginState& pluginState, chowdsp::EQ::StandardEQParameters<3>& eqParameters);

    void paint (juce::Graphics& g) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterPlots)
};
