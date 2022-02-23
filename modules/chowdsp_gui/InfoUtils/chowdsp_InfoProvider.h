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
    static juce::String getWrapperTypeString (const juce::AudioProcessor& proc);

    /** Returns true if a "debug" flag should be shown */
    static constexpr bool showDebugFlag();
};
} // namespace chowdsp
