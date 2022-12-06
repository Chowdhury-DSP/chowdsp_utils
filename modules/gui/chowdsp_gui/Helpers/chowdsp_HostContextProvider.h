#pragma once

#include "chowdsp_plugin_base/chowdsp_plugin_base.h"
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

namespace chowdsp
{
template <class Processor>
class HostContextProvider : private juce::ComponentListener
{
public:
    HostContextProvider (const PluginBase<Processor>& p, juce::AudioProcessorEditor& ed)
        : supportsParameterModulation (p.supportsParameterModulation()),
          plugin (p),
          editor (ed)
    {
    }

    // For VST3 parameter context menus
    void showParameterContextPopupMenu (const juce::RangedAudioParameter& param) const
    {
        if (const auto contextMenu = getContextMenuForParameter (param))
        {
            auto popupMenu = contextMenu->getEquivalentPopupMenu();
            if (popupMenu.containsAnyActiveItems())
            {
                const auto options = juce::PopupMenu::Options()
                                         .withParentComponent (&editor)
                                         .withPreferredPopupDirection (juce::PopupMenu::Options::PopupDirection::downwards)
                                         .withStandardItemHeight (27);

                popupMenu.showMenuAsync (juce::PopupMenu::Options());
            }
        }
    }

    std::unique_ptr<juce::HostProvidedContextMenu> getContextMenuForParameter (const juce::RangedAudioParameter& param) const
    {
        if (const auto* editorContext = editor.getHostContext())
            return editorContext->getContextMenuForParameter (&param);

        return {};
    }

    // For AAX automation menus
    void registerParameterComponent (juce::Component& comp, const juce::RangedAudioParameter& param)
    {
        componentToParameterIndexMap.insert_or_assign (&comp, param.getParameterIndex());
    }
    int getParameterIndexForComponent (juce::Component& comp) const
    {
        if (const auto iter = componentToParameterIndexMap.find (&comp); iter != componentToParameterIndexMap.end())
            return iter->second;
        return -1;
    }

    void componentBeingDeleted (juce::Component& comp)
    {
        componentToParameterIndexMap.erase (&comp);
    }

    const bool supportsParameterModulation;

private:
    const PluginBase<Processor>& plugin;
    juce::AudioProcessorEditor& editor;

    std::unordered_map<juce::Component*, int> componentToParameterIndexMap;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HostContextProvider)
};

} // namespace chowdsp
