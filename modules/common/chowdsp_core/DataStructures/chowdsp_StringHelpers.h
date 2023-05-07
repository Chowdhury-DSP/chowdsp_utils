#pragma once

namespace chowdsp
{
#if CHOWDSP_USING_JUCE
/** Converts a std::string_view to a juce::String */
inline juce::String toString (const std::string_view& sv) noexcept
{
    return juce::String::fromUTF8 (sv.data(), (int) sv.size());
}
#endif
} // namespace chowdsp
