#pragma once

/** Tools for working with software versioning. */
namespace chowdsp::VersionUtils
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
    friend bool operator< (const Version& v1, const Version& v2) noexcept;
    friend bool operator>= (const Version& v1, const Version& v2) noexcept;
    friend bool operator<= (const Version& v1, const Version& v2) noexcept;

private:
    int major = 0;
    int minor = 0;
    int patch = 0;

    JUCE_LEAK_DETECTOR (Version)
};

} // namespace chowdsp::VersionUtils
