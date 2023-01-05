#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

TEST_CASE ("Preset Tree Test", "[presets]")
{
    SECTION ("Flat Insertion Test")
    {
        chowdsp::PresetTree presetTree;
        presetTree.insertPreset (chowdsp::Preset { "Preset1", "", {} });
        presetTree.insertPreset (chowdsp::Preset { "Preset2", "", {} });
        presetTree.insertPreset (chowdsp::Preset { "ABCD", "", {} });
        presetTree.insertPreset (chowdsp::Preset { "ZZZZ", "", {} });

        const auto treeItems = presetTree.getTreeItems();
        REQUIRE (treeItems.size() == 4);
        REQUIRE (treeItems[0].preset->getName() == "ABCD");
        REQUIRE (treeItems[1].preset->getName() == "Preset1");
        REQUIRE (treeItems[2].preset->getName() == "Preset2");
        REQUIRE (treeItems[3].preset->getName() == "ZZZZ");
    }

    SECTION ("Vendor Insertion Test")
    {
        chowdsp::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::PresetTreeInserters::vendorInserter;

        presetTree.insertPreset (chowdsp::Preset { "Preset1", "Jatin", {} });
        presetTree.insertPreset (chowdsp::Preset { "Preset2", "Bob", {} });
        presetTree.insertPreset (chowdsp::Preset { "ABCD", "Livey", {} });
        presetTree.insertPreset (chowdsp::Preset { "ZZZZ", "Jatin", {} });

        const auto treeItems = presetTree.getTreeItems();
        REQUIRE (treeItems.size() == 3);

        REQUIRE (treeItems[0].tag == "Bob");
        REQUIRE (treeItems[0].subtree.size() == 1);
        REQUIRE (treeItems[0].subtree[0].preset->getName() == "Preset2");

        REQUIRE (treeItems[1].tag == "Jatin");
        REQUIRE (treeItems[1].subtree.size() == 2);
        REQUIRE (treeItems[1].subtree[0].preset->getName() == "Preset1");
        REQUIRE (treeItems[1].subtree[1].preset->getName() == "ZZZZ");

        REQUIRE (treeItems[2].tag == "Livey");
        REQUIRE (treeItems[2].subtree.size() == 1);
        REQUIRE (treeItems[2].subtree[0].preset->getName() == "ABCD");
    }

    SECTION ("Category Insertion Test")
    {
        chowdsp::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::PresetTreeInserters::categoryInserter;

        presetTree.insertPreset (chowdsp::Preset { "Preset1", "", {} });
        presetTree.insertPreset (chowdsp::Preset { "Preset2", "", {}, "Drums" });
        presetTree.insertPreset (chowdsp::Preset { "ABCD", "", {}, "Drums" });
        presetTree.insertPreset (chowdsp::Preset { "ZZZZ", "", {}, "Bass" });

        const auto treeItems = presetTree.getTreeItems();
        REQUIRE (treeItems.size() == 3);

        REQUIRE (treeItems[0].tag == "Bass");
        REQUIRE (treeItems[0].subtree.size() == 1);
        REQUIRE (treeItems[0].subtree[0].preset->getName() == "ZZZZ");

        REQUIRE (treeItems[1].tag == "Drums");
        REQUIRE (treeItems[1].subtree.size() == 2);
        REQUIRE (treeItems[1].subtree[0].preset->getName() == "ABCD");
        REQUIRE (treeItems[1].subtree[1].preset->getName() == "Preset2");

        REQUIRE (treeItems[2].preset->getName() == "Preset1");
    }

    SECTION ("Vendor/Category Insertion Test")
    {
        chowdsp::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::PresetTreeInserters::vendorCategoryInserter;

        presetTree.insertPresets ({
            chowdsp::Preset { "Bass1", "Jatin", {}, "Bass" },
            chowdsp::Preset { "Bass2", "Jatin", {}, "Bass" },
            chowdsp::Preset { "Drums1", "Jatin", {}, "Drums" },
            chowdsp::Preset { "Blah", "Jatin", {}, "" },
            chowdsp::Preset { "Gtr1", "Steve", {}, "Gtr" },
            chowdsp::Preset { "Gtr2", "Steve", {}, "Gtr" },
        });

        const auto treeItems = presetTree.getTreeItems();
        REQUIRE (treeItems.size() == 2);

        REQUIRE (treeItems[0].tag == "Jatin");
        REQUIRE (treeItems[0].subtree.size() == 3);

        REQUIRE (treeItems[0].subtree[0].tag == "Bass");
        REQUIRE (treeItems[0].subtree[0].subtree.size() == 2);
        REQUIRE (treeItems[0].subtree[0].subtree[0].preset->getName() == "Bass1");
        REQUIRE (treeItems[0].subtree[0].subtree[1].preset->getName() == "Bass2");

        REQUIRE (treeItems[0].subtree[1].tag == "Drums");
        REQUIRE (treeItems[0].subtree[1].subtree.size() == 1);
        REQUIRE (treeItems[0].subtree[1].subtree[0].preset->getName() == "Drums1");

        REQUIRE (treeItems[0].subtree[2].preset->getName() == "Blah");

        REQUIRE (treeItems[1].tag == "Steve");
        REQUIRE (treeItems[1].subtree.size() == 1);

        REQUIRE (treeItems[1].subtree[0].tag == "Gtr");
        REQUIRE (treeItems[1].subtree[0].subtree.size() == 2);
        REQUIRE (treeItems[1].subtree[0].subtree[0].preset->getName() == "Gtr1");
        REQUIRE (treeItems[1].subtree[0].subtree[1].preset->getName() == "Gtr2");
    }

    SECTION ("Get Preset By Index")
    {
        chowdsp::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::PresetTreeInserters::vendorCategoryInserter;

        presetTree.insertPresets ({
            chowdsp::Preset { "Bass1", "Jatin", {}, "Bass" },
            chowdsp::Preset { "Bass2", "Jatin", {}, "Bass" },
            chowdsp::Preset { "Drums1", "Jatin", {}, "Drums" },
            chowdsp::Preset { "Blah", "Jatin", {}, "" },
            chowdsp::Preset { "Gtr1", "Steve", {}, "Gtr" },
            chowdsp::Preset { "Gtr2", "Steve", {}, "Gtr" },
        });

        REQUIRE (presetTree.getPresetByIndex (0)->getName() == "Bass1");
        REQUIRE (presetTree.getPresetByIndex (1)->getName() == "Bass2");
        REQUIRE (presetTree.getPresetByIndex (2)->getName() == "Drums1");
        REQUIRE (presetTree.getPresetByIndex (3)->getName() == "Blah");
        REQUIRE (presetTree.getPresetByIndex (4)->getName() == "Gtr1");
        REQUIRE (presetTree.getPresetByIndex (5)->getName() == "Gtr2");
        REQUIRE (presetTree.getPresetByIndex (6) == nullptr);
    }

    SECTION ("Get Index For Preset")
    {
        chowdsp::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::PresetTreeInserters::vendorCategoryInserter;

        presetTree.insertPresets ({
            chowdsp::Preset { "Bass1", "Jatin", { { "val1", 0.5f } }, "Bass" },
            chowdsp::Preset { "Bass2", "Jatin", { { "val1", 0.75f } }, "Bass" },
        });

        REQUIRE (presetTree.getIndexForPreset (chowdsp::Preset { "Bass1", "Jatin", { { "val1", 0.5f } }, "Bass" }) == 0);
        REQUIRE (presetTree.getIndexForPreset (chowdsp::Preset { "Bass1", "Jatin", { { "val1", 1.0f } }, "Bass" }) == -1);
    }

    SECTION ("Delete by Index")
    {
        chowdsp::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::PresetTreeInserters::vendorCategoryInserter;

        presetTree.insertPresets ({
            chowdsp::Preset { "Bass1", "Jatin", { { "val1", 0.5f } }, "Bass" },
            chowdsp::Preset { "Bass2", "Jatin", { { "val1", 0.75f } }, "Bass" },
        });

        REQUIRE (presetTree.getTotalNumberOfPresets() == 2);
        REQUIRE (presetTree.getPresetByIndex (0)->getName() == "Bass1");
        REQUIRE (presetTree.getPresetByIndex (1)->getName() == "Bass2");

        presetTree.removePreset (0);
        REQUIRE (presetTree.getTotalNumberOfPresets() == 1);
        REQUIRE (presetTree.getPresetByIndex (0)->getName() == "Bass2");
    }

    SECTION ("Delete by Preset")
    {
        chowdsp::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::PresetTreeInserters::vendorCategoryInserter;

        presetTree.insertPresets ({
            chowdsp::Preset { "Bass1", "Jatin", { { "val1", 0.5f } }, "Bass" },
            chowdsp::Preset { "Bass2", "Jatin", { { "val1", 0.75f } }, "Bass" },
        });

        REQUIRE (presetTree.getTotalNumberOfPresets() == 2);
        REQUIRE (presetTree.getPresetByIndex (0)->getName() == "Bass1");
        REQUIRE (presetTree.getPresetByIndex (1)->getName() == "Bass2");

        presetTree.removePreset (chowdsp::Preset { "Bass2", "Jatin", { { "val1", 0.75f } }, "Bass" });
        REQUIRE (presetTree.getTotalNumberOfPresets() == 1);
        REQUIRE (presetTree.getPresetByIndex (0)->getName() == "Bass1");
    }
}
