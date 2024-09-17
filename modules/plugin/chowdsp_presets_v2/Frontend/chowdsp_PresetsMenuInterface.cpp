#include "chowdsp_PresetsMenuInterface.h"

namespace chowdsp::presets::frontend
{
MenuInterface::MenuInterface (PresetManager& manager, FileInterface* fileFace)
    : presetManager (manager), fileInterface (fileFace), clipboardInterface (manager)
{
    presetManager.presetListUpdatedBroadcaster.connect ([this]
                                                        { refreshPresetsMenu(); });
    refreshPresetsMenu();
}

void MenuInterface::addPresetsToMenu (juce::PopupMenu& menu) const
{
    for (auto iter = juce::PopupMenu::MenuItemIterator { presetsMenu }; iter.next();)
        menu.addItem (iter.getItem());
}

void MenuInterface::addExtraMenuItems (juce::PopupMenu& menu, std::initializer_list<ExtraMenuItems> extraMenuItems)
{
    const auto* currentPreset = presetManager.getCurrentPreset();
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
                               { presetManager.loadPreset (*presetManager.getCurrentPreset()); },
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
                               currentPresetFileExists && ! (currentPreset->isFactoryPreset) && hasFileInterface);
        }
        else if (itemID == Delete_Preset)
        {
            addPresetMenuItem ([this]
                               { fileInterface->deleteCurrentPreset(); },
                               currentPresetFileExists && ! (currentPreset->isFactoryPreset) && hasFileInterface);
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
                               hasFileInterface && fileInterface->hasSettingsInterface());
        }
    }
}

static void loadPresetsIntoMenu (juce::PopupMenu& menu, const PresetTree::Node& root, PresetManager& presetMgr)
{
    for (auto* node = root.first_child; node != nullptr; node = node->next_sibling)
    {
        if (node->value.has_value())
        {
            juce::PopupMenu::Item menuItem;
            menuItem.itemID = -1;
            menuItem.text = node->value.leaf().getName();
            menuItem.action = [&presetMgr, &preset = std::as_const (node->value.leaf())]
            {
                presetMgr.loadPreset (preset);
            };
            menu.addItem (menuItem);
        }
        else
        {
            juce::PopupMenu subMenu {};
            loadPresetsIntoMenu (subMenu, *node, presetMgr);
            if (subMenu.containsAnyActiveItems())
                menu.addSubMenu (toString (node->value.tag()), subMenu);
        }
    }
}

void MenuInterface::refreshPresetsMenu()
{
    presetsMenu.clear();
    loadPresetsIntoMenu (presetsMenu, presetManager.getPresetTree().getRootNode(), presetManager);
}
} // namespace chowdsp::presets::frontend
