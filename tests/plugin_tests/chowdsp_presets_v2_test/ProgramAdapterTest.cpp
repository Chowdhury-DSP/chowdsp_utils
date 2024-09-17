#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

struct Params : chowdsp::ParamHolder
{
};

using State = chowdsp::PluginStateImpl<Params>;

TEST_CASE ("Program Adapter Test", "[plugin][presets]")
{
    State state {};

    SECTION ("No Preset Manager")
    {
        std::unique_ptr<chowdsp::presets::PresetManager> presetMgr {};
        chowdsp::presets::frontend::PresetsProgramAdapter adapter { presetMgr };

        REQUIRE (adapter.getNumPrograms() == 1);
        REQUIRE (adapter.getCurrentProgram() == 0);
        REQUIRE (adapter.getProgramName (0).isEmpty());
        adapter.setCurrentProgram (0);
    }

    SECTION ("No Presets")
    {
        auto presetMgr = std::make_unique<chowdsp::presets::PresetManager> (state);
        chowdsp::presets::frontend::PresetsProgramAdapter adapter { presetMgr };

        REQUIRE (adapter.getNumPrograms() == 1);
        REQUIRE (adapter.getCurrentProgram() == 0);
        REQUIRE (adapter.getProgramName (0) == "User Preset");
        adapter.setCurrentProgram (0);
        REQUIRE (presetMgr->getCurrentPreset() == nullptr);
    }

    SECTION ("Get Program Info")
    {
        auto presetMgr = std::make_unique<chowdsp::presets::PresetManager> (state);
        chowdsp::presets::frontend::PresetsProgramAdapter adapter { presetMgr };
        presetMgr->addPresets ({ chowdsp::presets::Preset { "A", "Vendor", { { "value", 0 } } },
                                 chowdsp::presets::Preset { "B", "Vendor", { { "value", 1 } } },
                                 chowdsp::presets::Preset { "C", "Vendor", { { "value", 2 } } } });

        REQUIRE (adapter.getNumPrograms() == 4);
        REQUIRE (adapter.getProgramName (0) == "A");
        REQUIRE (adapter.getProgramName (1) == "B");
        REQUIRE (adapter.getProgramName (2) == "C");
        REQUIRE (adapter.getProgramName (3) == "User Preset");
        REQUIRE (adapter.getProgramName (4) == "");
    }

    const auto checkPresetIndex = [] (chowdsp::presets::PresetManager& mgr, int expectedIndex)
    {
        REQUIRE (mgr.getCurrentPreset()->getState()["value"] == expectedIndex);
    };

    SECTION ("Get Current Program")
    {
        auto presetMgr = std::make_unique<chowdsp::presets::PresetManager> (state);
        chowdsp::presets::frontend::PresetsProgramAdapter adapter { presetMgr };
        presetMgr->addPresets ({ chowdsp::presets::Preset { "A", "Vendor", { { "value", 0 } } },
                                 chowdsp::presets::Preset { "B", "Vendor", { { "value", 1 } } },
                                 chowdsp::presets::Preset { "C", "Vendor", { { "value", 2 } } } });

        presetMgr->loadPreset (presetMgr->getPresetTree().getRootNode().first_child->value.leaf());
        REQUIRE (adapter.getCurrentProgram() == 0);

        presetMgr->loadPreset (presetMgr->getPresetTree().getRootNode().first_child->next_sibling->value.leaf());
        REQUIRE (adapter.getCurrentProgram() == 1);

        presetMgr->loadPreset (presetMgr->getPresetTree().getRootNode().first_child->next_sibling->next_sibling->value.leaf());
        REQUIRE (adapter.getCurrentProgram() == 2);
    }

    SECTION ("Set Current Program")
    {
        auto presetMgr = std::make_unique<chowdsp::presets::PresetManager> (state);
        chowdsp::presets::frontend::PresetsProgramAdapter adapter { presetMgr };
        presetMgr->addPresets ({ chowdsp::presets::Preset { "A", "Vendor", { { "value", 0 } } },
                                 chowdsp::presets::Preset { "B", "Vendor", { { "value", 1 } } },
                                 chowdsp::presets::Preset { "C", "Vendor", { { "value", 2 } } } });

        adapter.setCurrentProgram (0);
        checkPresetIndex (*presetMgr, 0);

        adapter.setCurrentProgram (1);
        checkPresetIndex (*presetMgr, 1);

        adapter.setCurrentProgram (2);
        checkPresetIndex (*presetMgr, 2);

        adapter.setCurrentProgram (3);
        checkPresetIndex (*presetMgr, 2);
    }
}
