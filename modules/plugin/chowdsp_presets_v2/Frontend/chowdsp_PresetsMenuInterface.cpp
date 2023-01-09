#include "chowdsp_PresetsMenuInterface.h"

namespace chowdsp::PresetsFrontend
{
MenuInterface::MenuInterface (PresetManager& manager, FileInterface* fileFace)
    : presetManager (manager), fileInterface (fileFace), clipboardInterface (manager)
{
    presetManager.presetListUpdatedBroadcaster.connect ([this]
                                                        { refreshPresetsMenu(); });
    refreshPresetsMenu();
}

void MenuInterface::addPresetsToMenu (juce::PopupMenu& menu)
{
    for (auto iter = juce::PopupMenu::MenuItemIterator { presetsMenu }; iter.next();)
        menu.addItem (iter.getItem());
}

void MenuInterface::addExtraMenuItems (juce::PopupMenu& menu, std::initializer_list<ExtraMenuItems> extraMenuItems)
{
    const auto* currentPreset = presetManager.currentPreset.get();
    const auto currentPresetExists = currentPreset != nullptr && currentPreset->isValid();
    const auto currentPresetFileExists = currentPresetExists && currentPreset->getPresetFile().existsAsFile();
    const auto userPresetDirExists = presetManager.getUserPresetPath().isDirectory();
    const auto hasFileInterface = fileInterface != nullptr;

    for (auto itemID : extraMenuItems)
    {
        if (itemID == Separator)
        {
            menu.addSeparator();
            continue;
        }

        const auto itemName = toString (magic_enum::enum_name (itemID)).replaceCharacter ('_', ' ');
        const auto addPresetMenuItem = [&menu, &itemName] (auto&& action, bool shouldShow = true)
        {
            if (shouldShow)
            {
                juce::PopupMenu::Item item { itemName };
                item.itemID = -1;
                item.action = [&, forwardedAction = std::forward<decltype (action)> (action)]
                {
                    forwardedAction();
                };
                menu.addItem (item);
            }
        };

        if (itemID == Reset)
        {
            addPresetMenuItem ([this]
                               { presetManager.loadPreset (*presetManager.currentPreset); },
                               currentPresetExists);
        }
        else if (itemID == Save_Preset_As)
        {
            addPresetMenuItem ([this]
                               { fileInterface->savePreset(); },
                               userPresetDirExists && hasFileInterface);
        }
        else if (itemID == Resave_Preset)
        {
            addPresetMenuItem ([this]
                               { fileInterface->resaveCurrentPreset(); },
                               currentPresetFileExists && ! presetManager.isFactoryPreset (*currentPreset) && hasFileInterface);
        }
        else if (itemID == Delete_Preset)
        {
            addPresetMenuItem ([this]
                               { fileInterface->deleteCurrentPreset(); },
                               currentPresetFileExists && ! presetManager.isFactoryPreset (*currentPreset) && hasFileInterface);
        }
        else if (itemID == Copy_Current_Preset)
        {
            addPresetMenuItem ([this]
                               { clipboardInterface.copyCurrentPreset(); },
                               currentPresetExists);
        }
        else if (itemID == Paste_Preset)
        {
            addPresetMenuItem ([this]
                               { clipboardInterface.tryToPastePreset(); });
        }
        else if (itemID == Load_Preset_From_File)
        {
            addPresetMenuItem ([this]
                               { fileInterface->loadPresetFromFile(); },
                               hasFileInterface);
        }
        else if (itemID == Go_to_User_Presets_Folder)
        {
            addPresetMenuItem ([this]
                               { fileInterface->goToUserPresetsFolder(); },
                               userPresetDirExists && hasFileInterface);
        }
        else if (itemID == Choose_User_Presets_Folder)
        {
            addPresetMenuItem ([this]
                               { fileInterface->chooseUserPresetsFolder(); },
                               hasFileInterface);
        }
    }
}

static void loadPresetsIntoMenu (juce::PopupMenu& menu, const std::vector<PresetTree::Item>& presetTree, PresetManager& presetMgr)
{
    for (const auto& item : presetTree)
    {
        if (item.preset.has_value())
        {
            juce::PopupMenu::Item menuItem;
            menuItem.itemID = -1;
            menuItem.text = item.preset->getName();
            menuItem.action = [&presetMgr, &preset = std::as_const (*item.preset)]
            {
                presetMgr.loadPreset (preset);
            };
            menu.addItem (menuItem);

            // @TODO: figure out how to mark a preset as selected?
        }
        else
        {
            juce::PopupMenu subMenu {};
            loadPresetsIntoMenu (subMenu, item.subtree, presetMgr);

            if (subMenu.containsAnyActiveItems())
                menu.addSubMenu (item.tag, subMenu);
        }
    }
}

void MenuInterface::refreshPresetsMenu()
{
    presetsMenu.clear();
    loadPresetsIntoMenu (presetsMenu, presetManager.getPresetTree().getTreeItems(), presetManager);
}
} // namespace chowdsp::PresetsFrontend
