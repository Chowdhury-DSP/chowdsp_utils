#include "chowdsp_PresetsMenuInterface.h"

namespace chowdsp::PresetsFrontend
{
MenuInterface::MenuInterface (PresetManager& manager) : presetManager (manager)
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
    const auto addPresetMenuItem = [&menu] (const juce::String& itemText, auto&& action)
    {
        juce::PopupMenu::Item item { itemText };
        item.itemID = -1;
        item.action = [&, forwardedAction = std::forward<decltype (action)> (action)]
        {
            forwardedAction();
        };
        menu.addItem (item);
    };

    for (auto itemID : extraMenuItems)
    {
        if (itemID == Separator)
        {
            menu.addSeparator();
            continue;
        }

        const auto itemName = toString (magic_enum::enum_name (itemID)).replaceCharacter ('_', ' ');
        if (itemID == Resave_Preset)
        {
            if (presetManager.currentPreset != nullptr && presetManager.currentPreset->isValid())
                addPresetMenuItem (itemName, [this]
                                   { presetManager.loadPreset (*presetManager.currentPreset); });
            continue;
        }

        if (itemID == Save_Preset_As)
        {
            if (presetManager.getUserPresetPath().isDirectory())
                addPresetMenuItem (
                    itemName,
                    [this]
                    {
                        auto [preset, file] = savePresetCallback (presetManager.savePresetState());
                        presetManager.saveUserPreset (file, std::move (preset));
                    });
            continue;
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
