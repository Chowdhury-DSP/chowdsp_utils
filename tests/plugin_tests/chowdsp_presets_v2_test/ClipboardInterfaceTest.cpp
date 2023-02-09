#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

#if ! JUCE_LINUX

struct CIParams : chowdsp::ParamHolder
{
    CIParams()
    {
        add (floatParam);
    }
    chowdsp::PercentParameter::Ptr floatParam { "float", "Param", 0.5f };
};

TEST_CASE ("Clipboard Interface Test", "[plugin][presets]")
{
    static constexpr auto val1 = 0.1f;
    static constexpr auto val2 = 0.99f;

    chowdsp::PluginStateImpl<CIParams> state {};
    chowdsp::presets::PresetManager presetMgr { state };
    presetMgr.addPresets ({ chowdsp::presets::Preset { "Preset0", "Vendor", { { "float", val1 } } },
                            chowdsp::presets::Preset { "Preset1", "Vendor", { { "float", val2 } } } });

    const auto loadPreset = [&presetMgr] (int index)
    {
        presetMgr.loadPreset (*presetMgr.getPresetTree().getPresetByIndex (index));
    };

    SECTION ("Copy/Paste")
    {
        loadPreset (0);
        REQUIRE (state.params.floatParam->get() == val1);

        chowdsp::presets::frontend::ClipboardInterface clipInterface { presetMgr };
        clipInterface.copyCurrentPreset();

        loadPreset (1);
        REQUIRE (state.params.floatParam->get() == val2);

        REQUIRE (clipInterface.tryToPastePreset());
        REQUIRE (state.params.floatParam->get() == val1);
    }

    SECTION ("Empty Paste")
    {
        chowdsp::presets::frontend::ClipboardInterface clipInterface { presetMgr };
        loadPreset (0);

        juce::SystemClipboard::copyTextToClipboard ({});

        REQUIRE (! clipInterface.tryToPastePreset());
        REQUIRE (presetMgr.getCurrentPreset()->getName() == "Preset0");
        REQUIRE (state.params.floatParam->get() == val1);
    }

    SECTION ("Invalid Preset Paste")
    {
        chowdsp::presets::frontend::ClipboardInterface clipInterface { presetMgr };
        loadPreset (0);

        chowdsp::presets::Preset invalid { "Preset1", "", { { "float", val2 } } };
        juce::SystemClipboard::copyTextToClipboard (invalid.toJson().dump());

        REQUIRE (! clipInterface.tryToPastePreset());
        REQUIRE (presetMgr.getCurrentPreset()->getName() == "Preset0");
        REQUIRE (state.params.floatParam->get() == val1);
    }
}

#endif
