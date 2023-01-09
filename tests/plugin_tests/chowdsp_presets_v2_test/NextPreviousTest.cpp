#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

struct NPParams : chowdsp::ParamHolder
{
};

using State = chowdsp::PluginStateImpl<NPParams>;

TEST_CASE ("Next/Previous Test", "[presets]")
{
    State state {};

    SECTION ("No Presets")
    {
        chowdsp::PresetManager presetMgr { state };
        chowdsp::PresetsFrontend::NextPrevious nextPrev { presetMgr };

        REQUIRE (nextPrev.goToNextPreset() == false);
        REQUIRE (nextPrev.goToPreviousPreset() == false);
    }

    SECTION ("No Current Preset")
    {
        chowdsp::PresetManager presetMgr { state };
        presetMgr.addPresets ({ chowdsp::Preset { "Name", "Vendor", { { "dummy", 0.0f } } } });
        chowdsp::PresetsFrontend::NextPrevious nextPrev { presetMgr };
        presetMgr.currentPreset.reset();

        REQUIRE (nextPrev.goToNextPreset() == false);
        REQUIRE (nextPrev.goToPreviousPreset() == false);
    }

    const auto loadPreset = [] (chowdsp::PresetManager& mgr, int index)
    {
        mgr.loadPreset (*mgr.getPresetTree().getPresetByIndex (index));
    };

    const auto checkPresetIndex = [] (chowdsp::PresetManager& mgr, int expectedIndex)
    {
        REQUIRE (mgr.currentPreset->getState()["value"] == expectedIndex);
    };

    SECTION ("No Wrapping")
    {
        chowdsp::PresetManager presetMgr { state };
        presetMgr.addPresets ({ chowdsp::Preset { "A", "Vendor", { { "value", 0 } } },
                                chowdsp::Preset { "B", "Vendor", { { "value", 1 } } },
                                chowdsp::Preset { "C", "Vendor", { { "value", 2 } } } });

        chowdsp::PresetsFrontend::NextPrevious nextPrev { presetMgr };
        nextPrev.setShouldWrapAtEndOfList (false);
        REQUIRE (nextPrev.willWrapAtEndOFList() == false);

        loadPreset (presetMgr, 0);
        checkPresetIndex (presetMgr, 0);

        REQUIRE (nextPrev.goToPreviousPreset() == false);
        checkPresetIndex (presetMgr, 0);

        REQUIRE (nextPrev.goToNextPreset() == true);
        checkPresetIndex (presetMgr, 1);

        REQUIRE (nextPrev.goToNextPreset() == true);
        checkPresetIndex (presetMgr, 2);

        REQUIRE (nextPrev.goToNextPreset() == false);
        checkPresetIndex (presetMgr, 2);

        REQUIRE (nextPrev.goToPreviousPreset() == true);
        checkPresetIndex (presetMgr, 1);

        REQUIRE (nextPrev.goToPreviousPreset() == true);
        checkPresetIndex (presetMgr, 0);

        REQUIRE (nextPrev.goToPreviousPreset() == false);
        checkPresetIndex (presetMgr, 0);
    }

    SECTION ("With Wrapping")
    {
        chowdsp::PresetManager presetMgr { state };
        presetMgr.addPresets ({ chowdsp::Preset { "A", "Vendor", { { "value", 0 } } },
                                chowdsp::Preset { "B", "Vendor", { { "value", 1 } } },
                                chowdsp::Preset { "C", "Vendor", { { "value", 2 } } } });

        chowdsp::PresetsFrontend::NextPrevious nextPrev { presetMgr };
        nextPrev.setShouldWrapAtEndOfList (true);
        REQUIRE (nextPrev.willWrapAtEndOFList() == true);

        loadPreset (presetMgr, 0);
        checkPresetIndex (presetMgr, 0);

        REQUIRE (nextPrev.goToNextPreset() == true);
        checkPresetIndex (presetMgr, 1);

        REQUIRE (nextPrev.goToNextPreset() == true);
        checkPresetIndex (presetMgr, 2);

        REQUIRE (nextPrev.goToNextPreset() == true);
        checkPresetIndex (presetMgr, 0);

        REQUIRE (nextPrev.goToPreviousPreset() == true);
        checkPresetIndex (presetMgr, 2);

        REQUIRE (nextPrev.goToPreviousPreset() == true);
        checkPresetIndex (presetMgr, 1);

        REQUIRE (nextPrev.goToPreviousPreset() == true);
        checkPresetIndex (presetMgr, 0);
    }
}