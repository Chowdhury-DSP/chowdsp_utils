#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

struct NPParams : chowdsp::ParamHolder
{
};

using State = chowdsp::PluginStateImpl<NPParams>;

TEST_CASE ("Next/Previous Test", "[plugin][presets]")
{
    State state {};

    SECTION ("No Presets")
    {
        chowdsp::presets::PresetManager presetMgr { state };
        chowdsp::presets::frontend::NextPrevious nextPrev { presetMgr };

        REQUIRE (nextPrev.goToNextPreset() == false);
        REQUIRE (nextPrev.goToPreviousPreset() == false);
    }

    SECTION ("No Current Preset")
    {
        chowdsp::presets::PresetManager presetMgr { state };
        presetMgr.addPresets ({ chowdsp::presets::Preset { "Name", "Vendor", { { "dummy", 0.0f } } } });
        chowdsp::presets::frontend::NextPrevious nextPrev { presetMgr };

        REQUIRE (nextPrev.goToNextPreset() == false);
        REQUIRE (nextPrev.goToPreviousPreset() == false);
    }

    SECTION ("With External Preset")
    {
        chowdsp::presets::PresetManager presetMgr { state };
        presetMgr.addPresets ({ chowdsp::presets::Preset { "A", "Vendor", { { "value", 0 } } },
                                chowdsp::presets::Preset { "B", "Vendor", { { "value", 1 } } },
                                chowdsp::presets::Preset { "C", "Vendor", { { "value", 2 } } } });
        presetMgr.loadPreset ({ "Blah", "Blah Vendor", { { "value", 1000 } } });

        chowdsp::presets::frontend::NextPrevious nextPrev { presetMgr };
        REQUIRE (nextPrev.goToNextPreset() == false);
        REQUIRE (nextPrev.goToPreviousPreset() == false);
    }

    const auto checkPresetIndex = [] (chowdsp::presets::PresetManager& mgr, int expectedIndex)
    {
        REQUIRE (mgr.getCurrentPreset()->getState()["value"] == expectedIndex);
    };

    SECTION ("No Wrapping")
    {
        chowdsp::presets::PresetManager presetMgr { state };
        presetMgr.addPresets ({ chowdsp::presets::Preset { "A", "Vendor", { { "value", 0 } } },
                                chowdsp::presets::Preset { "B", "Vendor", { { "value", 1 } } },
                                chowdsp::presets::Preset { "C", "Vendor", { { "value", 2 } } } });

        chowdsp::presets::frontend::NextPrevious nextPrev { presetMgr };
        nextPrev.setShouldWrapAtEndOfList (false);
        REQUIRE (nextPrev.willWrapAtEndOFList() == false);

        presetMgr.loadPreset (presetMgr.getPresetTree().getRootNode().first_child->value.leaf());
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

    SECTION ("With Wrapping and Nesting")
    {
        chowdsp::presets::PresetManager presetMgr { state };
        presetMgr.getPresetTree().treeInserter = &chowdsp::presets::PresetTreeInserters::vendorInserter;

        presetMgr.addPresets ({
            chowdsp::presets::Preset { "A", "A Vendor", { { "value", 0 } } },
            chowdsp::presets::Preset { "B", "B Vendor", { { "value", 1 } } },
            chowdsp::presets::Preset { "C", "C Vendor", { { "value", 2 } } },
            chowdsp::presets::Preset { "D", "", { { "value", 3 } } },
        });

        chowdsp::presets::frontend::NextPrevious nextPrev { presetMgr };
        nextPrev.setShouldWrapAtEndOfList (true);
        REQUIRE (nextPrev.willWrapAtEndOFList() == true);

        presetMgr.loadPreset (presetMgr.getPresetTree().getRootNode().first_child->first_child->value.leaf());
        checkPresetIndex (presetMgr, 0);

        REQUIRE (nextPrev.goToNextPreset() == true);
        checkPresetIndex (presetMgr, 1);

        REQUIRE (nextPrev.goToNextPreset() == true);
        checkPresetIndex (presetMgr, 2);

        REQUIRE (nextPrev.goToNextPreset() == true);
        checkPresetIndex (presetMgr, 3);

        REQUIRE (nextPrev.goToNextPreset() == true);
        checkPresetIndex (presetMgr, 0);

        REQUIRE (nextPrev.goToPreviousPreset() == true);
        checkPresetIndex (presetMgr, 3);

        REQUIRE (nextPrev.goToPreviousPreset() == true);
        checkPresetIndex (presetMgr, 2);

        REQUIRE (nextPrev.goToPreviousPreset() == true);
        checkPresetIndex (presetMgr, 1);

        REQUIRE (nextPrev.goToPreviousPreset() == true);
        checkPresetIndex (presetMgr, 0);
    }
}
