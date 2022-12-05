#pragma once

namespace chowdsp
{
/** Methods for retreiving information about the target system. */
namespace SystemInfo
{
    /** Returns the current operating system and bit-depth. */
    constexpr std::string_view getOSDescription()
    {
#if JUCE_WINDOWS
#if JUCE_64BIT
        return "Win64";
#elif JUCE_32BIT
        return "Win32";
#endif
#elif JUCE_MAC
        return "Mac";
#elif JUCE_IOS
        return "IOS";
#elif JUCE_LINUX
#if JUCE_64BIT
        return "Linux64";
#elif JUCE_32BIT
        return "Linux32";
#endif
#endif
    }

    /**
     * Returns the target system architecture.
     * Currently only "Intel" or "ARM".
     */
    constexpr std::string_view getProcArch()
    {
#if JUCE_INTEL
        return "Intel";
#elif JUCE_ARM
        return "ARM";
#else
        return "N/A";
#endif
    }
} // namespace SystemInfo
} // namespace chowdsp
