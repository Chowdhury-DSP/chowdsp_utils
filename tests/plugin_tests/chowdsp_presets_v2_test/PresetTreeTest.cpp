#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

TEST_CASE ("Preset Tree Test", "[plugin][presets]")
{
    SECTION ("Flat Insertion Test")
    {
        chowdsp::presets::PresetTree preset_tree;
        std::array<juce::String, 4> preset_names {
            "Preset1",
            "Preset2",
            "ABCD",
            "ZZZZ",
        };

        for (const auto& name : preset_names)
            preset_tree.insertElement (chowdsp::presets::Preset { name, "", {} });

        std::array<juce::String, 4> preset_names_sorted {
            "ABCD",
            "Preset1",
            "Preset2",
            "ZZZZ",
        };

        REQUIRE (preset_tree.size() == 4);
        const auto& root = std::as_const (preset_tree).getRootNode();
        const auto* iter = root.first_child;
        for (const auto& name : preset_names_sorted)
        {
            REQUIRE (iter->value.leaf().getName() == name);
            iter = iter->next_sibling;
        }
        REQUIRE (iter == nullptr);
    }

    SECTION ("Vendor Insertion Test")
    {
        chowdsp::presets::PresetTree preset_tree;
        preset_tree.treeInserter = &chowdsp::presets::PresetTreeInserters::vendorInserter;

        preset_tree.insertElement (chowdsp::presets::Preset { "Preset1", "Jatin", {} });
        preset_tree.insertElement (chowdsp::presets::Preset { "Preset2", "Bob", {} });
        preset_tree.insertElement (chowdsp::presets::Preset { "ABCD", "Livey", {} });
        preset_tree.insertElement (chowdsp::presets::Preset { "ZZZZ", "Jatin", {} });
        REQUIRE (preset_tree.size() == 4);

        const auto& root = preset_tree.getRootNode();

        const auto* bob_node = root.first_child;
        REQUIRE (bob_node->value.tag() == "Bob");
        REQUIRE (bob_node->first_child->value.leaf().getName() == "Preset2");
        REQUIRE (bob_node->first_child->next_sibling == nullptr);

        const auto* jatin_node = bob_node->next_sibling;
        REQUIRE (jatin_node->value.tag() == "Jatin");
        REQUIRE (jatin_node->first_child->value.leaf().getName() == "Preset1");
        REQUIRE (jatin_node->first_child->next_sibling->value.leaf().getName() == "ZZZZ");
        REQUIRE (jatin_node->first_child->next_sibling->next_sibling == nullptr);

        const auto* livey_node = jatin_node->next_sibling;
        REQUIRE (livey_node->value.tag() == "Livey");
        REQUIRE (livey_node->first_child->value.leaf().getName() == "ABCD");
        REQUIRE (livey_node->first_child->next_sibling == nullptr);
    }

    SECTION ("Category Insertion Test")
    {
        chowdsp::presets::PresetTree preset_tree;
        preset_tree.treeInserter = &chowdsp::presets::PresetTreeInserters::categoryInserter;

        preset_tree.insertElement (chowdsp::presets::Preset { "Preset1", "", {} });
        preset_tree.insertElement (chowdsp::presets::Preset { "Preset2", "", {}, "Drums" });
        preset_tree.insertElement (chowdsp::presets::Preset { "ABCD", "", {}, "Drums" });
        preset_tree.insertElement (chowdsp::presets::Preset { "ZZZZ", "", {}, "Bass" });
        REQUIRE (preset_tree.size() == 4);

        const auto& root = preset_tree.getRootNode();

        const auto* bass_node = root.first_child;
        REQUIRE (bass_node->value.tag() == "Bass");
        REQUIRE (bass_node->first_child->value.leaf().getName() == "ZZZZ");
        REQUIRE (bass_node->first_child->next_sibling == nullptr);

        const auto* drums_node = bass_node->next_sibling;
        REQUIRE (drums_node->value.tag() == "Drums");
        REQUIRE (drums_node->first_child->value.leaf().getName() == "ABCD");
        REQUIRE (drums_node->first_child->next_sibling->value.leaf().getName() == "Preset2");
        REQUIRE (drums_node->first_child->next_sibling->next_sibling == nullptr);

        const auto* loose_preset = drums_node->next_sibling;
        REQUIRE (loose_preset->next_sibling == nullptr);
        REQUIRE (loose_preset->value.leaf().getName() == "Preset1");
    }

    SECTION ("Vendor/Category Insertion Test")
    {
        chowdsp::presets::PresetTree preset_tree;
        preset_tree.treeInserter = &chowdsp::presets::PresetTreeInserters::vendorCategoryInserter;

        preset_tree.insertElements ({
            chowdsp::presets::Preset { "Bass1", "Jatin", {}, "Bass" },
            { "Bass2", "Jatin", {}, "Bass" },
            { "Drums1", "Jatin", {}, "Drums" },
            { "Blah", "Jatin", {}, "" },
            { "Gtr1", "Steve", {}, "Gtr" },
            { "Gtr2", "Steve", {}, "Gtr" },
        });

        REQUIRE (preset_tree.size() == 6);

        const auto& root = preset_tree.getRootNode();

        const auto* jatin_node = root.first_child;
        REQUIRE (jatin_node->value.tag() == "Jatin");

        const auto* jatin_bass_node = jatin_node->first_child;
        REQUIRE (jatin_bass_node->value.tag() == "Bass");
        REQUIRE (jatin_bass_node->first_child->value.leaf().getName() == "Bass1");
        REQUIRE (jatin_bass_node->first_child->next_sibling->value.leaf().getName() == "Bass2");
        REQUIRE (jatin_bass_node->first_child->next_sibling->next_sibling == nullptr);

        const auto* jatin_drums_node = jatin_bass_node->next_sibling;
        REQUIRE (jatin_drums_node->value.tag() == "Drums");
        REQUIRE (jatin_drums_node->first_child->value.leaf().getName() == "Drums1");
        REQUIRE (jatin_drums_node->first_child->next_sibling == nullptr);

        const auto* jatin_loose_preset = jatin_drums_node->next_sibling;
        REQUIRE (jatin_loose_preset->next_sibling == nullptr);
        REQUIRE (jatin_loose_preset->value.leaf().getName() == "Blah");

        const auto* steve_node = jatin_node->next_sibling;
        REQUIRE (steve_node->value.tag() == "Steve");

        const auto* steve_gtr_node = steve_node->first_child;
        REQUIRE (steve_gtr_node->value.tag() == "Gtr");
        REQUIRE (steve_gtr_node->first_child->value.leaf().getName() == "Gtr1");
        REQUIRE (steve_gtr_node->first_child->next_sibling->value.leaf().getName() == "Gtr2");
        REQUIRE (steve_gtr_node->first_child->next_sibling->next_sibling == nullptr);
    }

    // SECTION ("Get Preset By Index")
    // {
    //     chowdsp::presets::PresetTree presetTree;
    //     presetTree.treeInserter = &chowdsp::presets::PresetTreeInserters::vendorCategoryInserter;
    //
    //     presetTree.insertElements ({
    //         chowdsp::presets::Preset { "Bass1", "Jatin", {}, "Bass" },
    //         chowdsp::presets::Preset { "Bass2", "Jatin", {}, "Bass" },
    //         chowdsp::presets::Preset { "Drums1", "Jatin", {}, "Drums" },
    //         chowdsp::presets::Preset { "Blah", "Jatin", {}, "" },
    //         chowdsp::presets::Preset { "Gtr1", "Steve", {}, "Gtr" },
    //         chowdsp::presets::Preset { "Gtr2", "Steve", {}, "Gtr" },
    //     });
    //
    //     REQUIRE (presetTree.getElementByIndex (0)->getName() == "Bass1");
    //     REQUIRE (presetTree.getElementByIndex (1)->getName() == "Bass2");
    //     REQUIRE (presetTree.getElementByIndex (2)->getName() == "Drums1");
    //     REQUIRE (presetTree.getElementByIndex (3)->getName() == "Blah");
    //     REQUIRE (presetTree.getElementByIndex (4)->getName() == "Gtr1");
    //     REQUIRE (presetTree.getElementByIndex (5)->getName() == "Gtr2");
    //     REQUIRE (presetTree.getElementByIndex (6) == nullptr);
    // }

    // SECTION ("Get Index For Preset")
    // {
    //     chowdsp::presets::PresetTree presetTree;
    //     presetTree.treeInserter = &chowdsp::presets::PresetTreeInserters::vendorCategoryInserter;
    //
    //     presetTree.insertElements ({
    //         chowdsp::presets::Preset { "Bass1", "Jatin", { { "val1", 0.5f } }, "Bass" },
    //         chowdsp::presets::Preset { "Bass2", "Jatin", { { "val1", 0.75f } }, "Bass" },
    //     });
    //
    //     REQUIRE (presetTree.getIndexForElement (chowdsp::presets::Preset { "Bass1", "Jatin", { { "val1", 0.5f } }, "Bass" }) == 0);
    //     REQUIRE (presetTree.getIndexForElement (chowdsp::presets::Preset { "Bass1", "Jatin", { { "val1", 1.0f } }, "Bass" }) == -1);
    // }

    SECTION ("Delete by Preset")
    {
        chowdsp::presets::PresetTree preset_tree;
        preset_tree.treeInserter = &chowdsp::presets::PresetTreeInserters::vendorCategoryInserter;

        const auto preset1 = chowdsp::presets::Preset { "Bass1", "Jatin", { { "val1", 0.5f } }, "Bass" };
        const auto preset2 = chowdsp::presets::Preset { "Bass2", "Jatin", { { "val1", 0.75f } }, "Bass" };
        preset_tree.insertElements ({ preset1, preset2 });

        REQUIRE (preset_tree.size() == 2);
        const auto* bass_node = preset_tree.getRootNode().first_child->first_child;
        REQUIRE (bass_node->value.tag() == "Bass");

        REQUIRE (bass_node->first_child->value.leaf().getName() == "Bass1");
        REQUIRE (bass_node->first_child->next_sibling->value.leaf().getName() == "Bass2");

        preset_tree.removeElement (preset2);
        REQUIRE (preset_tree.size() == 1);
        REQUIRE (bass_node->first_child->value.leaf().getName() == "Bass1");
        REQUIRE (bass_node->first_child->next_sibling == nullptr);
    }

    SECTION ("Delete by Expression")
    {
        chowdsp::presets::PresetTree preset_tree;
        preset_tree.treeInserter = &chowdsp::presets::PresetTreeInserters::vendorCategoryInserter;

        preset_tree.insertElements ({
            chowdsp::presets::Preset { "Bass1", "Jatin", {}, "Bass" },
            { "Bass2", "Jatin", {}, "Bass" },
            { "Drums1", "Jatin", {}, "Drums" },
            { "Blah", "Jatin", {}, "" },
            { "Gtr1", "Steve", {}, "Gtr" },
            { "Gtr2", "Steve", {}, "Gtr" },
        });
        REQUIRE (preset_tree.size() == 6);

        preset_tree.removeElements (
            [] (const chowdsp::presets::Preset& preset)
            {
                return preset.getVendor() == "Jatin";
            });

        REQUIRE (preset_tree.size() == 2);
        preset_tree.doForAllElements ([] (const chowdsp::presets::Preset& preset)
                                      { REQUIRE (preset.getVendor() == "Steve"); });
    }

    SECTION ("Find Preset")
    {
        chowdsp::presets::PresetTree preset_tree;
        const auto& preset = preset_tree.insertElement (chowdsp::presets::Preset { "Blah", "Jatin", { { "tag", 1.0f } }, "Cat1" });

        const auto foundPreset = preset_tree.findElement (preset);
        REQUIRE (*foundPreset == preset);

        REQUIRE (! preset_tree.findElement (chowdsp::presets::Preset { "Blah", "Jatin", { { "tag", 100.0f } } }));
    }

    SECTION ("With Preset State")
    {
        const auto preset = chowdsp::presets::Preset { "Blah", "Jatin", { { "tag", 1.0f } }, "Cat1" };

        chowdsp::presets::PresetState state;
        chowdsp::presets::PresetTree preset_tree { &state };
        preset_tree.insertElement (chowdsp::presets::Preset { preset });

        state = preset_tree.getRootNode().first_child->value.leaf();
        REQUIRE (*state.get() == preset);

        preset_tree.removeElement (preset);
        REQUIRE (*state.get() == preset);
    }
}
