#pragma once

// LCOV_EXCL_START

namespace chowdsp::CLAPExtensions
{
/** Interface for clap_juce_extensions::clap_properties */
struct CLAPInfoExtensions : public clap_juce_extensions::clap_properties
{
    /**
     * Returns juce::AudioProcessor::getWrapperTypeDescription(), unless
     * the plugin is a CLAP, in which case the method will return "CLAP".
     */
    [[nodiscard]] juce::String getPluginTypeString (juce::AudioProcessor::WrapperType wrapperType) const
    {
        if (wrapperType == juce::AudioProcessor::wrapperType_Undefined && is_clap)
            return "CLAP";

        return juce::AudioProcessor::getWrapperTypeDescription (wrapperType);
    }
};
} // namespace chowdsp::CLAPExtensions

// LCOV_EXCL_END
