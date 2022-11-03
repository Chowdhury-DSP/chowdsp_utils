/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_version
   vendor:        Chowdhury DSP
   version:       1.3.0
   name:          ChowDSP Plugin Versioning
   description:   Versioning system for ChowDSP plugins
   dependencies:  juce_core

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>

namespace chowdsp
{
/** Tools for working with software versioning. */
namespace VersionUtils
{
    /** Utility class to manage version strings. */
    class Version
    {
    public:
        explicit Version (const juce::String& versionStr = "0.0.0");

        Version (const Version&) = default;
        Version& operator= (const Version&) = default;

        [[nodiscard]] juce::String getVersionString() const;

        friend bool operator== (const Version& v1, const Version& v2) noexcept;
        friend bool operator!= (const Version& v1, const Version& v2) noexcept;
        friend bool operator> (const Version& v1, const Version& v2) noexcept;
        friend bool operator<(const Version& v1, const Version& v2) noexcept;
        friend bool operator>= (const Version& v1, const Version& v2) noexcept;
        friend bool operator<= (const Version& v1, const Version& v2) noexcept;

    private:
        int major = 0;
        int minor = 0;
        int patch = 0;

        JUCE_LEAK_DETECTOR (Version)
    };
} // namespace VersionUtils
} // namespace chowdsp
