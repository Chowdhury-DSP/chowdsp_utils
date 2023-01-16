#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

struct TIParams : chowdsp::ParamHolder
{
    TIParams()
    {
        add (floatParam);
    }
    chowdsp::PercentParameter::Ptr floatParam { "float", "Param", 0.5f };
};

TEST_CASE ("Text Interface Test", "[plugin][presets]")
{
    chowdsp::PluginStateImpl<TIParams> state {};
    chowdsp::PresetManager presetMgr { state };
    presetMgr.addPresets ({ chowdsp::Preset { "Preset0", "Vendor", { { "float", 0.1f } } },
                            chowdsp::Preset { "Preset1", "Vendor", { { "float", 0.99f } } } });

    const auto loadPreset = [&presetMgr] (int index)
    {
        presetMgr.loadPreset (*presetMgr.getPresetTree().getPresetByIndex (index));
    };

    SECTION ("Preset Change")
    {
        loadPreset (0);
        chowdsp::PresetsFrontend::TextInterface textInterface { presetMgr };
        REQUIRE (textInterface.getPresetText() == "Preset0");

        loadPreset (1);
        REQUIRE (textInterface.getPresetText() == "Preset1");
    }

    SECTION ("Preset Dirty Change")
    {
        loadPreset (0);
        chowdsp::PresetsFrontend::TextInterface textInterface { presetMgr };
        REQUIRE (textInterface.getPresetText() == "Preset0");

        chowdsp::ParameterTypeHelpers::setValue (0.25f, *state.params.floatParam);
        state.getParameterListeners().updateBroadcastersFromMessageThread();
        REQUIRE (textInterface.getPresetText() == "Preset0*");

        loadPreset (1);
        REQUIRE (textInterface.getPresetText() == "Preset1");
    }
}
