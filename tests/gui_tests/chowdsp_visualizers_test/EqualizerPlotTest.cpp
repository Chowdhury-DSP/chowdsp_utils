#include "VizTestUtils.h"
#include <chowdsp_visualizers/chowdsp_visualizers.h>

struct TestEQParams : chowdsp::ParamHolder
{
    inline static const juce::StringArray bandTypeChoices {
        "1-Pole HPF",
        "2-Pole HPF",
        "Low-Shelf",
        "Bell",
        "Notch",
        "High-Shelf",
        "1-Pole LPF",
        "2-Pole LPF",
    };

    TestEQParams()
    {
        add (testEQParams);
    }

    chowdsp::EQ::StandardEQParameters<4> testEQParams {
        {
            chowdsp::EQ::StandardEQParameters<4>::EQBandParams { 0, "eq_band_0", "Band ", 100, bandTypeChoices, 3, 250.0f },
            chowdsp::EQ::StandardEQParameters<4>::EQBandParams { 1, "eq_band_1", "Band ", 100, bandTypeChoices, 3, 1000.0f },
            chowdsp::EQ::StandardEQParameters<4>::EQBandParams { 2, "eq_band_2", "Band ", 100, bandTypeChoices, 3, 4000.0f },
            chowdsp::EQ::StandardEQParameters<4>::EQBandParams { 3, "eq_band_3", "Band ", 100, bandTypeChoices, 3, 10000.0f },
        }
    };
};

struct TestEQComp : chowdsp::EQ::EqualizerPlotWithParameters<4>
{
    explicit TestEQComp (chowdsp::PluginStateImpl<TestEQParams>& state)
        : chowdsp::EQ::EqualizerPlotWithParameters<4> (
            state.getParameterListeners(),
            state.params.testEQParams,
            [] (int typeIndex) -> chowdsp::EQ::EQPlotFilterType
            {
                using Type = chowdsp::EQ::EQPlotFilterType;
                switch (typeIndex)
                {
                    case 0:
                        return Type::HPF1;
                    case 1:
                        return Type::HPF2;
                    case 2:
                        return Type::LowShelf;
                    case 3:
                        return Type::Bell;
                    case 4:
                        return Type::Notch;
                    case 5:
                        return Type::HighShelf;
                    case 6:
                        return Type::LPF1;
                    case 7:
                    default:
                        return Type::LPF2;
                }
            })
    {
        setSize (500, 300);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);

        g.setColour (juce::Colours::red);
        g.strokePath (getPath (0), juce::PathStrokeType { 1.0f });

        g.setColour (juce::Colours::green);
        g.strokePath (getPath (1), juce::PathStrokeType { 1.0f });

        g.setColour (juce::Colours::dodgerblue);
        g.strokePath (getPath (2), juce::PathStrokeType { 1.0f });

        g.setColour (getFilterActive (3) ? juce::Colours::white : juce::Colours::grey);
        g.strokePath (getPath (3), juce::PathStrokeType { 2.5f });

        g.setColour (juce::Colours::yellow);
        g.strokePath (getMasterFilterPath(), juce::PathStrokeType { 2.5f });
    }
};

TEST_CASE ("Equalizer Plot Test", "[visualizers][EQ]")
{
    SECTION ("EQ Plot")
    {
        chowdsp::PluginStateImpl<TestEQParams> state {};

        TestEQComp plotComp { state };

        chowdsp::ParameterTypeHelpers::setValue (100.0f, *state.params.testEQParams.eqParams[0].freqParam);
        chowdsp::ParameterTypeHelpers::setValue (1000.0f, *state.params.testEQParams.eqParams[1].freqParam);
        chowdsp::ParameterTypeHelpers::setValue (1000.0f, *state.params.testEQParams.eqParams[2].freqParam);
        chowdsp::ParameterTypeHelpers::setValue (8000.0f, *state.params.testEQParams.eqParams[3].freqParam);

        chowdsp::ParameterTypeHelpers::setValue (1.5f, *state.params.testEQParams.eqParams[1].qParam);
        chowdsp::ParameterTypeHelpers::setValue (2.5f, *state.params.testEQParams.eqParams[2].qParam);

        chowdsp::ParameterTypeHelpers::setValue (1, *state.params.testEQParams.eqParams[0].typeParam);
        chowdsp::ParameterTypeHelpers::setValue (3, *state.params.testEQParams.eqParams[1].typeParam);
        chowdsp::ParameterTypeHelpers::setValue (5, *state.params.testEQParams.eqParams[2].typeParam);
        chowdsp::ParameterTypeHelpers::setValue (7, *state.params.testEQParams.eqParams[3].typeParam);

        chowdsp::ParameterTypeHelpers::setValue (15.0f, *state.params.testEQParams.eqParams[1].gainParam);
        chowdsp::ParameterTypeHelpers::setValue (-10.0f, *state.params.testEQParams.eqParams[2].gainParam);

        chowdsp::ParameterTypeHelpers::setValue (true, *state.params.testEQParams.eqParams[0].onOffParam);
        chowdsp::ParameterTypeHelpers::setValue (true, *state.params.testEQParams.eqParams[1].onOffParam);
        chowdsp::ParameterTypeHelpers::setValue (true, *state.params.testEQParams.eqParams[2].onOffParam);
        chowdsp::ParameterTypeHelpers::setValue (false, *state.params.testEQParams.eqParams[3].onOffParam);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (100);

        const auto testScreenshot = plotComp.createComponentSnapshot ({ 500, 300 });
        // VizTestUtils::saveImage (testScreenshot, "eq_response_plot.png");

        const auto refScreenshot = VizTestUtils::loadImage ("eq_response_plot.png");
        VizTestUtils::compareImages (testScreenshot, refScreenshot);
    }
}
