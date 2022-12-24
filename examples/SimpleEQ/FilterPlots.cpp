#include "FilterPlots.h"

namespace
{
chowdsp::EQ::EQPlotFilterType getFilterType (int typeIndex)
{
    using Type = chowdsp::EQ::EQPlotFilterType;
    switch (typeIndex)
    {
        case 0:
            return Type::HPF1;
        case 1:
        case 2:
            return Type::HPF2;
        case 3:
            return Type::HPF3;
        case 4:
            return Type::HPF4;
        case 5:
        case 6:
            return Type::HPF8;
        case 7:
            return Type::HPF12;

        case 8:
        case 9:
            return Type::LowShelf;
        case 10:
        case 11:
            return Type::Bell;
        case 12:
        case 13:
            return Type::Notch;
        case 14:
        case 15:
            return Type::HighShelf;
        case 16:
        case 17:
            return Type::BPF2;

        case 18:
            return Type::LPF1;
        case 19:
        case 20:
            return Type::LPF2;
        case 21:
            return Type::LPF3;
        case 22:
            return Type::LPF4;
        case 23:
        case 24:
            return Type::LPF8;
        case 25:
            return Type::LPF12;
        default:
            return {};
    }
}
} // namespace

FilterPlots::FilterPlots (chowdsp::PluginState& pluginState, chowdsp::EQ::StandardEQParameters<3>& eqParameters)
    : chowdsp::EQ::EqualizerPlotWithParameters<3> (pluginState.getParameterListeners(),
                                                   eqParameters,
                                                   &getFilterType)
{
}

void FilterPlots::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    // major grid lines
    g.setColour (juce::Colours::white);
    drawFrequencyLines (g, { 100.0f, 1000.0f, 10000.0f });
    drawMagnitudeLines (g, { -20.0f, -10.0f, 0.0f, 10.0f, 20.0f });

    g.setColour (juce::Colours::red);
    g.strokePath (getPath (0), juce::PathStrokeType { 1.0f });

    g.setColour (juce::Colours::green);
    g.strokePath (getPath (1), juce::PathStrokeType { 1.0f });

    g.setColour (juce::Colours::dodgerblue);
    g.strokePath (getPath (2), juce::PathStrokeType { 1.0f });

    g.setColour (juce::Colours::yellow);
    g.strokePath (getMasterFilterPath(), juce::PathStrokeType { 2.5f });
}
