#pragma once

namespace chowdsp::CLAPExtensions
{
struct CLAPInfoExtensions : protected clap_juce_extensions::clap_properties
{
    [[nodiscard]] juce::String getPluginTypeString (juce::AudioProcessor::WrapperType wrapperType) const
    {
        if (wrapperType == juce::AudioProcessor::wrapperType_Undefined && is_clap)
            return "CLAP";

        return juce::AudioProcessor::getWrapperTypeDescription (wrapperType);
    }
};
} // namespace chowdsp::CLAPExtensions
