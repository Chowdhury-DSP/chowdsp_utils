#pragma once

namespace chowdsp
{
/**
 * Struct containing static methods for retrieving information about the plugin/app.
 * To see how it may be used as a template "mixin", see chowdsp::InfoComp.
 */
struct StandardInfoProvider
{
    /** Returns the name of the plugin/app manufacturer */
    static juce::String getManufacturerString();

    /** Returns the URL of the manufacturer website */
    static juce::URL getManufacturerWebsiteURL();

    /** Returns a string containing the current platform info */
    static juce::String getPlatformString();

    /** Returns the current plugin/app version */
    static juce::String getVersionString();

    /** Returns the plugin wrapper type */
    template <typename ProcType>
    static juce::String getWrapperTypeString (const ProcType& proc)
    {
        if constexpr (std::is_same_v<ProcType, juce::AudioProcessor>)
            return juce::String { juce::AudioProcessor::getWrapperTypeDescription (proc.wrapperType) };
        else
            return proc.getWrapperTypeString();
    }

    /** Returns true if a "debug" flag should be shown */
    static constexpr bool showDebugFlag()
    {
#if JUCE_DEBUG
        return true;
#else
        return false;
#endif
    }
};
} // namespace chowdsp
