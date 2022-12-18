#include <TimedUnitTest.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

struct LevelParams : chowdsp::ParamHolder
{
    LevelParams()
    {
        add (percent, gain);
    }

    chowdsp::PercentParameter::Ptr percent { juce::ParameterID { "percent", 100 }, "Percent" };
    chowdsp::GainDBParameter::Ptr gain { juce::ParameterID { "gain", 100 }, "Gain", juce::NormalisableRange { -30.0f, 0.0f }, 0.0f };
};

struct PluginParameterState : chowdsp::ParamHolder
{
    PluginParameterState()
    {
        add (levelParams, mode, onOff);
    }

    LevelParams levelParams;
    chowdsp::ChoiceParameter::Ptr mode { juce::ParameterID { "mode", 100 }, "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { juce::ParameterID { "on_off", 100 }, "On/Off", true };
};

struct Plugin : chowdsp::PluginBase<chowdsp::PluginStateImpl<PluginParameterState>>
{
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>&) override {}
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

};

class StatePluginInterfaceTest : public TimedUnitTest
{
public:
    StatePluginInterfaceTest() : TimedUnitTest ("State/Plugin Interface Test", "ChowDSP State")
    {
    }

    void parameterInfoTest()
    {
        Plugin plugin {};
        auto& params = plugin.getState().params;

        const auto pluginParams = plugin.getParameters();
        expectEquals (pluginParams.size(), params.count(), "Plugin has the incorrect number of parameters!");

        juce::StringArray expectedParamIDs;
        params.doForAllParameters ([&expectedParamIDs] (auto& param, size_t)
                                   {
                                      expectedParamIDs.add (param.paramID);
                                   });

        juce::StringArray actualParamIDs;
        for (const auto* param : pluginParams)
            actualParamIDs.add (dynamic_cast<const juce::RangedAudioParameter*> (param)->paramID);

        for (auto& paramID : expectedParamIDs)
        {
            expect (actualParamIDs.contains (paramID), "Plugin does not contain parameter ID!");
            actualParamIDs.removeString (paramID);
        }

        expect (actualParamIDs.isEmpty(), "Plugin has extra parameter IDs!");
    }

    void runTestTimed() override
    {
        beginTest ("Parameter Info Test");
        parameterInfoTest();
    }
};

static StatePluginInterfaceTest statePluginInterfaceTest;
