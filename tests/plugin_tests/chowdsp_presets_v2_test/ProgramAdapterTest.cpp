#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

struct Params : chowdsp::ParamHolder
{
};

using State = chowdsp::PluginStateImpl<Params>;

TEST_CASE ("Program Adapter Test", "[presets]")
{
    State state {};

    SECTION ("No Preset Manager")
    {
        std::unique_ptr<chowdsp::PresetManager> presetMgr {};
        chowdsp::PresetsProgramAdapter adapter { presetMgr };

        REQUIRE (adapter.getNumPrograms() == 1);
        REQUIRE (adapter.getCurrentProgram() == 0);
        REQUIRE (adapter.getProgramName (0).isEmpty());
        adapter.setCurrentProgram (0);
    }

    SECTION ("No Presets")
    {
        auto presetMgr = std::make_unique<chowdsp::PresetManager> (state);
        chowdsp::PresetsProgramAdapter adapter { presetMgr };

        REQUIRE (adapter.getNumPrograms() == 0);
        REQUIRE (adapter.getCurrentProgram() == 0);
        REQUIRE (adapter.getProgramName (0).isEmpty());
        adapter.setCurrentProgram (0);
        REQUIRE (presetMgr->currentPreset == nullptr);
    }

    SECTION ("Get Program Info")
    {
        auto presetMgr = std::make_unique<chowdsp::PresetManager> (state);
        chowdsp::PresetsProgramAdapter adapter { presetMgr };
        presetMgr->addPresets ({ chowdsp::Preset { "A", "Vendor", { { "value", 0 } } },
                                 chowdsp::Preset { "B", "Vendor", { { "value", 1 } } },
                                 chowdsp::Preset { "C", "Vendor", { { "value", 2 } } } });

        REQUIRE (adapter.getNumPrograms() == 3);
        REQUIRE (adapter.getProgramName (0) == "A");
        REQUIRE (adapter.getProgramName (1) == "B");
        REQUIRE (adapter.getProgramName (2) == "C");
        REQUIRE (adapter.getProgramName (3) == "");
    }

    const auto loadPreset = [] (chowdsp::PresetManager& mgr, int index)
    {
        mgr.loadPreset (*mgr.getPresetTree().getPresetByIndex (index));
    };

    const auto checkPresetIndex = [] (chowdsp::PresetManager& mgr, int expectedIndex)
    {
        REQUIRE (mgr.currentPreset->getState()["value"] == expectedIndex);
    };

    SECTION ("Get Current Program")
    {
        auto presetMgr = std::make_unique<chowdsp::PresetManager> (state);
        chowdsp::PresetsProgramAdapter adapter { presetMgr };
        presetMgr->addPresets ({ chowdsp::Preset { "A", "Vendor", { { "value", 0 } } },
                                 chowdsp::Preset { "B", "Vendor", { { "value", 1 } } },
                                 chowdsp::Preset { "C", "Vendor", { { "value", 2 } } } });

        loadPreset (*presetMgr, 0);
        REQUIRE (adapter.getCurrentProgram() == 0);

        loadPreset (*presetMgr, 1);
        REQUIRE (adapter.getCurrentProgram() == 1);

        loadPreset (*presetMgr, 2);
        REQUIRE (adapter.getCurrentProgram() == 2);
    }

    SECTION ("Set Current Program")
    {
        auto presetMgr = std::make_unique<chowdsp::PresetManager> (state);
        chowdsp::PresetsProgramAdapter adapter { presetMgr };
        presetMgr->addPresets ({ chowdsp::Preset { "A", "Vendor", { { "value", 0 } } },
                                 chowdsp::Preset { "B", "Vendor", { { "value", 1 } } },
                                 chowdsp::Preset { "C", "Vendor", { { "value", 2 } } } });

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
