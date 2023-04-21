#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

TEST_CASE ("Preset Tree Test", "[plugin][presets]")
{
    SECTION ("Flat Insertion Test")
    {
        chowdsp::presets::PresetTree presetTree;
        presetTree.insertElement (chowdsp::presets::Preset { "Preset1", "", {} });
        presetTree.insertElement (chowdsp::presets::Preset { "Preset2", "", {} });
        presetTree.insertElement (chowdsp::presets::Preset { "ABCD", "", {} });
        presetTree.insertElement (chowdsp::presets::Preset { "ZZZZ", "", {} });

        const auto treeItems = presetTree.getNodes();
        REQUIRE (treeItems.size() == 4);
        REQUIRE (treeItems[0].leaf->getName() == "ABCD");
        REQUIRE (treeItems[1].leaf->getName() == "Preset1");
        REQUIRE (treeItems[2].leaf->getName() == "Preset2");
        REQUIRE (treeItems[3].leaf->getName() == "ZZZZ");
    }

    SECTION ("Vendor Insertion Test")
    {
        chowdsp::presets::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::presets::PresetTreeInserters::vendorInserter;

        presetTree.insertElement (chowdsp::presets::Preset { "Preset1", "Jatin", {} });
        presetTree.insertElement (chowdsp::presets::Preset { "Preset2", "Bob", {} });
        presetTree.insertElement (chowdsp::presets::Preset { "ABCD", "Livey", {} });
        presetTree.insertElement (chowdsp::presets::Preset { "ZZZZ", "Jatin", {} });

        const auto treeItems = presetTree.getNodes();
        REQUIRE (treeItems.size() == 3);

        REQUIRE (treeItems[0].tag == "Bob");
        REQUIRE (treeItems[0].subtree.size() == 1);
        REQUIRE (treeItems[0].subtree[0].leaf->getName() == "Preset2");

        REQUIRE (treeItems[1].tag == "Jatin");
        REQUIRE (treeItems[1].subtree.size() == 2);
        REQUIRE (treeItems[1].subtree[0].leaf->getName() == "Preset1");
        REQUIRE (treeItems[1].subtree[1].leaf->getName() == "ZZZZ");

        REQUIRE (treeItems[2].tag == "Livey");
        REQUIRE (treeItems[2].subtree.size() == 1);
        REQUIRE (treeItems[2].subtree[0].leaf->getName() == "ABCD");
    }

    SECTION ("Category Insertion Test")
    {
        chowdsp::presets::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::presets::PresetTreeInserters::categoryInserter;

        presetTree.insertElement (chowdsp::presets::Preset { "Preset1", "", {} });
        presetTree.insertElement (chowdsp::presets::Preset { "Preset2", "", {}, "Drums" });
        presetTree.insertElement (chowdsp::presets::Preset { "ABCD", "", {}, "Drums" });
        presetTree.insertElement (chowdsp::presets::Preset { "ZZZZ", "", {}, "Bass" });

        const auto treeItems = presetTree.getNodes();
        REQUIRE (treeItems.size() == 3);

        REQUIRE (treeItems[0].tag == "Bass");
        REQUIRE (treeItems[0].subtree.size() == 1);
        REQUIRE (treeItems[0].subtree[0].leaf->getName() == "ZZZZ");

        REQUIRE (treeItems[1].tag == "Drums");
        REQUIRE (treeItems[1].subtree.size() == 2);
        REQUIRE (treeItems[1].subtree[0].leaf->getName() == "ABCD");
        REQUIRE (treeItems[1].subtree[1].leaf->getName() == "Preset2");

        REQUIRE (treeItems[2].leaf->getName() == "Preset1");
    }

    SECTION ("Vendor/Category Insertion Test")
    {
        chowdsp::presets::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::presets::PresetTreeInserters::vendorCategoryInserter;

        presetTree.insertElements ({
            chowdsp::presets::Preset { "Bass1", "Jatin", {}, "Bass" },
            chowdsp::presets::Preset { "Bass2", "Jatin", {}, "Bass" },
            chowdsp::presets::Preset { "Drums1", "Jatin", {}, "Drums" },
            chowdsp::presets::Preset { "Blah", "Jatin", {}, "" },
            chowdsp::presets::Preset { "Gtr1", "Steve", {}, "Gtr" },
            chowdsp::presets::Preset { "Gtr2", "Steve", {}, "Gtr" },
        });

        const auto treeItems = presetTree.getNodes();
        REQUIRE (treeItems.size() == 2);

        REQUIRE (treeItems[0].tag == "Jatin");
        REQUIRE (treeItems[0].subtree.size() == 3);

        REQUIRE (treeItems[0].subtree[0].tag == "Bass");
        REQUIRE (treeItems[0].subtree[0].subtree.size() == 2);
        REQUIRE (treeItems[0].subtree[0].subtree[0].leaf->getName() == "Bass1");
        REQUIRE (treeItems[0].subtree[0].subtree[1].leaf->getName() == "Bass2");

        REQUIRE (treeItems[0].subtree[1].tag == "Drums");
        REQUIRE (treeItems[0].subtree[1].subtree.size() == 1);
        REQUIRE (treeItems[0].subtree[1].subtree[0].leaf->getName() == "Drums1");

        REQUIRE (treeItems[0].subtree[2].leaf->getName() == "Blah");

        REQUIRE (treeItems[1].tag == "Steve");
        REQUIRE (treeItems[1].subtree.size() == 1);

        REQUIRE (treeItems[1].subtree[0].tag == "Gtr");
        REQUIRE (treeItems[1].subtree[0].subtree.size() == 2);
        REQUIRE (treeItems[1].subtree[0].subtree[0].leaf->getName() == "Gtr1");
        REQUIRE (treeItems[1].subtree[0].subtree[1].leaf->getName() == "Gtr2");
    }

    SECTION ("Get Preset By Index")
    {
        chowdsp::presets::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::presets::PresetTreeInserters::vendorCategoryInserter;

        presetTree.insertElements ({
            chowdsp::presets::Preset { "Bass1", "Jatin", {}, "Bass" },
            chowdsp::presets::Preset { "Bass2", "Jatin", {}, "Bass" },
            chowdsp::presets::Preset { "Drums1", "Jatin", {}, "Drums" },
            chowdsp::presets::Preset { "Blah", "Jatin", {}, "" },
            chowdsp::presets::Preset { "Gtr1", "Steve", {}, "Gtr" },
            chowdsp::presets::Preset { "Gtr2", "Steve", {}, "Gtr" },
        });

        REQUIRE (presetTree.getElementByIndex (0)->getName() == "Bass1");
        REQUIRE (presetTree.getElementByIndex (1)->getName() == "Bass2");
        REQUIRE (presetTree.getElementByIndex (2)->getName() == "Drums1");
        REQUIRE (presetTree.getElementByIndex (3)->getName() == "Blah");
        REQUIRE (presetTree.getElementByIndex (4)->getName() == "Gtr1");
        REQUIRE (presetTree.getElementByIndex (5)->getName() == "Gtr2");
        REQUIRE (presetTree.getElementByIndex (6) == nullptr);
    }

    SECTION ("Get Index For Preset")
    {
        chowdsp::presets::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::presets::PresetTreeInserters::vendorCategoryInserter;

        presetTree.insertElements ({
            chowdsp::presets::Preset { "Bass1", "Jatin", { { "val1", 0.5f } }, "Bass" },
            chowdsp::presets::Preset { "Bass2", "Jatin", { { "val1", 0.75f } }, "Bass" },
        });

        REQUIRE (presetTree.getIndexForElement (chowdsp::presets::Preset { "Bass1", "Jatin", { { "val1", 0.5f } }, "Bass" }) == 0);
        REQUIRE (presetTree.getIndexForElement (chowdsp::presets::Preset { "Bass1", "Jatin", { { "val1", 1.0f } }, "Bass" }) == -1);
    }

    SECTION ("Delete by Index")
    {
        chowdsp::presets::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::presets::PresetTreeInserters::vendorCategoryInserter;

        presetTree.insertElements ({
            chowdsp::presets::Preset { "Bass1", "Jatin", { { "val1", 0.5f } }, "Bass" },
            chowdsp::presets::Preset { "Bass2", "Jatin", { { "val1", 0.75f } }, "Bass" },
        });

        REQUIRE (presetTree.size() == 2);
        REQUIRE (presetTree.getElementByIndex (0)->getName() == "Bass1");
        REQUIRE (presetTree.getElementByIndex (1)->getName() == "Bass2");

        presetTree.removeElement (0);
        REQUIRE (presetTree.size() == 1);
        REQUIRE (presetTree.getElementByIndex (0)->getName() == "Bass2");
    }

    SECTION ("Delete by Preset")
    {
        chowdsp::presets::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::presets::PresetTreeInserters::vendorCategoryInserter;

        presetTree.insertElements ({
            chowdsp::presets::Preset { "Bass1", "Jatin", { { "val1", 0.5f } }, "Bass" },
            chowdsp::presets::Preset { "Bass2", "Jatin", { { "val1", 0.75f } }, "Bass" },
        });

        REQUIRE (presetTree.size() == 2);
        REQUIRE (presetTree.getElementByIndex (0)->getName() == "Bass1");
        REQUIRE (presetTree.getElementByIndex (1)->getName() == "Bass2");

        presetTree.removeElement (chowdsp::presets::Preset { "Bass2", "Jatin", { { "val1", 0.75f } }, "Bass" });
        REQUIRE (presetTree.size() == 1);
        REQUIRE (presetTree.getElementByIndex (0)->getName() == "Bass1");
    }

    SECTION ("Delete by Expression")
    {
        chowdsp::presets::PresetTree presetTree;
        presetTree.treeInserter = &chowdsp::presets::PresetTreeInserters::vendorCategoryInserter;

        presetTree.insertElements ({
            chowdsp::presets::Preset { "Bass1", "Jatin", {}, "Bass" },
            chowdsp::presets::Preset { "Bass2", "Jatin", {}, "Bass" },
            chowdsp::presets::Preset { "Drums1", "Jatin", {}, "Drums" },
            chowdsp::presets::Preset { "Blah", "Jatin", {}, "" },
            chowdsp::presets::Preset { "Gtr1", "Steve", {}, "Gtr" },
            chowdsp::presets::Preset { "Gtr2", "Steve", {}, "Gtr" },
        });
        REQUIRE (presetTree.getNodes().size() == 2);

        presetTree.removeElements ([] (const chowdsp::presets::Preset& preset)
                                   { return preset.getVendor() == "Jatin"; });

        REQUIRE (presetTree.getNodes().size() == 1);
        REQUIRE (presetTree.size() == 2);
        REQUIRE (presetTree.getElementByIndex (0)->getVendor() == "Steve");
        REQUIRE (presetTree.getElementByIndex (1)->getVendor() == "Steve");
    }

    SECTION ("Find Preset")
    {
        chowdsp::presets::PresetTree presetTree;
        const auto& preset = presetTree.insertElement (chowdsp::presets::Preset { "Blah", "Jatin", { { "tag", 1.0f } }, "Cat1" });

        const auto* foundPreset = presetTree.findElement (preset);
        REQUIRE (*foundPreset == preset);

        REQUIRE (presetTree.findElement (chowdsp::presets::Preset { "Blah", "Jatin", { { "tag", 100.0f } } }) == nullptr);
    }

    SECTION ("With Preset State")
    {
        const auto preset = chowdsp::presets::Preset { "Blah", "Jatin", { { "tag", 1.0f } }, "Cat1" };

        chowdsp::presets::PresetState state;
        chowdsp::presets::PresetTree presetTree { &state };
        presetTree.insertElement (chowdsp::presets::Preset { preset });

        state = *presetTree.getElementByIndex (0);
        REQUIRE (*state.get() == preset);

        presetTree.removeElement (0);
        REQUIRE (*state.get() == preset);
    }
}
