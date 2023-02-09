#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

struct MenuIParams : chowdsp::ParamHolder
{
};

TEST_CASE ("Menu Interface Test", "[plugin][presets]")
{
    chowdsp::PluginStateImpl<MenuIParams> state {};
    chowdsp::presets::PresetManager presetMgr { state };

    SECTION ("Presets Tree Menu")
    {
        presetMgr.getPresetTree().treeInserter = &chowdsp::presets::PresetTreeInserters::vendorInserter;

        presetMgr.addPresets ({
            chowdsp::presets::Preset { "Preset4", "Vendor C", { { "param", 0.0f } } },
            chowdsp::presets::Preset { "Preset2", "Vendor A", { { "param", 0.0f } } },
            chowdsp::presets::Preset { "Preset3", "Vendor B", { { "param", 0.0f } } },
            chowdsp::presets::Preset { "Preset1", "Vendor A", { { "param", 0.0f } } },
        });

        chowdsp::presets::frontend::MenuInterface menuInterface { presetMgr };
        {
            juce::PopupMenu menu;
            menuInterface.addPresetsToMenu (menu);
            juce::String sortedNames[] { "Preset1", "Preset2", "Preset3", "Preset4" };
            juce::PopupMenu::MenuItemIterator iter { menu, true };
            int count = 0;
            while (iter.next())
            {
                if (iter.getItem().subMenu != nullptr)
                    continue;

                REQUIRE (iter.getItem().text == sortedNames[count]);
                count++;
            }
            REQUIRE (count == 4);
        }

        presetMgr.addPresets ({
            chowdsp::presets::Preset { "Preset0", "Alpha Vendor", { { "param", 0.0f } } },
        });
        {
            juce::PopupMenu menu;
            menuInterface.addPresetsToMenu (menu);
            juce::String sortedNames[] { "Preset0", "Preset1", "Preset2", "Preset3", "Preset4" };
            juce::PopupMenu::MenuItemIterator iter { menu, true };
            int count = 0;
            while (iter.next())
            {
                if (iter.getItem().subMenu != nullptr)
                    continue;

                REQUIRE (iter.getItem().text == sortedNames[count]);
                count++;
            }
            REQUIRE (count == 5);
        }

        juce::PopupMenu menu;
        menuInterface.addPresetsToMenu (menu);
        juce::PopupMenu::MenuItemIterator iter { menu, true };
        while (iter.next())
        {
            if (iter.getItem().subMenu != nullptr)
                continue;

            iter.getItem().action();
            break;
        }
        REQUIRE (presetMgr.getCurrentPreset()->getName() == "Preset0");
    }

    SECTION ("Extra Menu Items")
    {
        using Items = chowdsp::presets::frontend::MenuInterface::ExtraMenuItems;

        // with nothing
        {
            chowdsp::presets::frontend::MenuInterface menuInterface { presetMgr };

            juce::PopupMenu menu;
            menuInterface.addExtraMenuItems (menu, {
                                                       Items::Separator,
                                                       Items::Reset,
                                                       Items::Save_Preset_As,
                                                       Items::Resave_Preset,
                                                       Items::Delete_Preset,
                                                       Items::Copy_Current_Preset,
                                                       Items::Paste_Preset,
                                                       Items::Load_Preset_From_File,
                                                       Items::Go_to_User_Presets_Folder,
                                                       Items::Choose_User_Presets_Folder,
                                                   });
            REQUIRE (menu.getNumItems() == 1);
        }

        // with current preset
        {
            chowdsp::presets::frontend::MenuInterface menuInterface { presetMgr };
            presetMgr.setDefaultPreset (chowdsp::presets::Preset { "Preset4", "Vendor C", { { "param", 0.0f } } });
            presetMgr.loadDefaultPreset();

            juce::PopupMenu menu;
            menuInterface.addExtraMenuItems (menu, {
                                                       Items::Separator,
                                                       Items::Reset,
                                                       Items::Save_Preset_As,
                                                       Items::Resave_Preset,
                                                       Items::Delete_Preset,
                                                       Items::Copy_Current_Preset,
                                                       Items::Paste_Preset,
                                                       Items::Load_Preset_From_File,
                                                       Items::Go_to_User_Presets_Folder,
                                                       Items::Choose_User_Presets_Folder,
                                                   });
            REQUIRE (menu.getNumItems() == 3);
        }
    }
}
