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
    chowdsp::presets::PresetManager presetMgr { state };
    presetMgr.addPresets ({ chowdsp::presets::Preset { "Preset0", "Vendor", { { "float", 0.1f } } },
                            chowdsp::presets::Preset { "Preset1", "Vendor", { { "float", 0.99f } } } });

    SECTION ("Preset Change")
    {
        presetMgr.loadPreset (presetMgr.getPresetTree().getRootNode().first_child->value.leaf());
        chowdsp::presets::frontend::TextInterface textInterface { presetMgr };
        REQUIRE (textInterface.getPresetText() == "Preset0");

        presetMgr.loadPreset (presetMgr.getPresetTree().getRootNode().first_child->next_sibling->value.leaf());
        REQUIRE (textInterface.getPresetText() == "Preset1");
    }

    SECTION ("Preset Dirty Change")
    {
        presetMgr.loadPreset (presetMgr.getPresetTree().getRootNode().first_child->value.leaf());
        chowdsp::presets::frontend::TextInterface textInterface { presetMgr };
        REQUIRE (textInterface.getPresetText() == "Preset0");

        chowdsp::ParameterTypeHelpers::setValue (0.25f, *state.params.floatParam);
        state.getParameterListeners().updateBroadcastersFromMessageThread();
        REQUIRE (textInterface.getPresetText() == "Preset0*");

        presetMgr.loadPreset (presetMgr.getPresetTree().getRootNode().first_child->next_sibling->value.leaf());
        REQUIRE (textInterface.getPresetText() == "Preset1");
    }
}
