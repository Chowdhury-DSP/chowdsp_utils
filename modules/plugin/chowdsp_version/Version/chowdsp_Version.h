#pragma once

namespace chowdsp
{
/** Utility class to manage version strings. */
class Version
{
public:
    /** Construct a version object from a JUCE string. */
    explicit Version (const juce::String& versionStr);

    /** Construct a version object from a string_view (can be constexpr). */
    constexpr explicit Version (const std::string_view& versionStr)
    {
        int numDots = 0;
        for (auto ch : versionStr)
            if (ch == '.')
                numDots++;

        // Valid version strings must have two dots!
        if (numDots != 2)
        {
            major = 0;
            minor = 0;
            patch = 0;
            return;
        }

        const auto firstDot = versionStr.find ('.');
        const auto majorVersionStr = versionStr.substr (0, firstDot);
        major = version_detail::stoi (majorVersionStr);

        auto remainder = versionStr.substr (firstDot + 1);
        const auto secondDot = remainder.find ('.');
        const auto minorVersionStr = remainder.substr (0, secondDot);
        minor = version_detail::stoi (minorVersionStr);

        const auto patchVersionStr = remainder.substr (secondDot + 1);
        patch = version_detail::stoi (patchVersionStr);
    }

    /** Construct a version object directly. */
    constexpr Version (int majorVersion, int minorVersion, int patchVersion)
        : major (majorVersion), minor (minorVersion), patch (patchVersion)
    {
    }

    /** Construct a version object with version "0.0.0". */
    constexpr Version() : Version (0, 0, 0) {}

    constexpr Version (const Version&) = default;
    constexpr Version& operator= (const Version&) = default;
    constexpr Version (Version&&) noexcept = default;
    constexpr Version& operator= (Version&&) noexcept = default;

    /** Returns the version as a string of the form MAJOR.MINOR.PATCH. */
    [[nodiscard]] juce::String getVersionString() const;

    /** Returns an integer hint for this version value. */
    [[nodiscard]] constexpr int getVersionHint() const { return major * 10000 + minor * 100 + patch; }

    /** Custom serializer */
    template <typename Serializer>
    static typename Serializer::SerializedType serialize (const Version& object)
    {
        return Serializer::template serialize<Serializer> (object.getVersionString());
    }

    /** Custom deserializer */
    template <typename Serializer>
    static void deserialize (typename Serializer::DeserializedType serial, Version& object)
    {
        juce::String versionString;
        Serializer::template deserialize<Serializer> (serial, versionString);
        object = Version { versionString };
    }

    friend constexpr bool operator== (const Version& v1, const Version& v2) noexcept;
    friend constexpr bool operator!= (const Version& v1, const Version& v2) noexcept;
    friend constexpr bool operator> (const Version& v1, const Version& v2) noexcept;
    friend constexpr bool operator<(const Version& v1, const Version& v2) noexcept;
    friend constexpr bool operator>= (const Version& v1, const Version& v2) noexcept;
    friend constexpr bool operator<= (const Version& v1, const Version& v2) noexcept;

private:
    int major = 0;
    int minor = 0;
    int patch = 0;
};

namespace version_literals
{
    constexpr Version operator"" _v (const char* str, size_t len) noexcept
    {
        return Version { std::string_view (str, len) };
    }
} // namespace version_literals

#if defined JucePlugin_VersionString
static constexpr auto currentPluginVersion = Version { std::string_view { JucePlugin_VersionString } };
#endif
} // namespace chowdsp
