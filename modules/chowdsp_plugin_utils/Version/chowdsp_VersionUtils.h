#pragma once

namespace chowdsp
{
namespace VersionUtils
{
    /** Utility class to manage version strings. */
    class Version
    {
    public:
        Version (const juce::String& versionStr = "0.0.0");

        Version (const Version&) = default;
        Version& operator= (const Version&) = default;

        juce::String getVersionString() const;

        friend bool operator== (const Version& v1, const Version& v2);
        friend bool operator!= (const Version& v1, const Version& v2);
        friend bool operator> (const Version& v1, const Version& v2);
        friend bool operator< (const Version& v1, const Version& v2);
        friend bool operator>= (const Version& v1, const Version& v2);
        friend bool operator<= (const Version& v1, const Version& v2);

    private:
        int major = 0;
        int minor = 0;
        int patch = 0;

        JUCE_LEAK_DETECTOR (Version)
    };

} // namespace VersionUtils

} // namespace chowdsp
