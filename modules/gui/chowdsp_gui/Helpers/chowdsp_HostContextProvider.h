#pragma once

namespace chowdsp
{
/**
 * A class used to provide a bridge between a host/plugin format
 * and a plugin GUI.
 *
 * Currently functionality includes:
 * - Helpers for creating parameter context menus (VST3)
 * - Helpers for getting a parameter index for a component (AAX)
 * - Helpers for determining if the plugin is being used in a context that supports parameter modulation (CLAP)
 */
class HostContextProvider : private juce::ComponentListener
{
public:
    /** Constructs a HostContextProvider for a plugin and plugin editor */
    template <typename PluginType>
    HostContextProvider (const PluginType& plugin, juce::AudioProcessorEditor& pluginEditor)
        : editor (pluginEditor),
          pluginSupportsParamModulation (plugin.supportsParameterModulation())
    {
    }

    /** Shows the parameter context menu for a given parameter */
    void showParameterContextPopupMenu (const juce::RangedAudioParameter& param,
                                        const juce::PopupMenu::Options& menuOptions = {},
                                        juce::LookAndFeel* lookAndFeel = nullptr) const;

    /** Returns a context menu for a given parameter */
    [[nodiscard]] virtual std::unique_ptr<juce::HostProvidedContextMenu> getContextMenuForParameter (const juce::RangedAudioParameter& param) const;

    /** Registers a component for a parameter. (See getParameterIndexForComponent()) */
    virtual void registerParameterComponent (juce::Component& comp, const juce::RangedAudioParameter& param);

    /**
     * After a Component has been registered with registerParameterForComponent(),
     * this method can be used to get a parameter index for a given component. This
     * can be useful for implementing juce::AudioProcessorEditor::getControlParameterIndex().
     */
    int getParameterIndexForComponent (juce::Component& comp) const;

    /**
     * Returns true if the plugin supports non-destructive parameter automation.
     * Currently this will only return true for CLAP plugins.
     */
    bool supportsParameterModulation() const noexcept { return pluginSupportsParamModulation; }

private:
    void componentBeingDeleted (juce::Component& comp) override;

    juce::AudioProcessorEditor& editor;
    const bool pluginSupportsParamModulation;

    std::unordered_map<juce::Component*, int> componentToParameterIndexMap;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HostContextProvider)
};

} // namespace chowdsp
