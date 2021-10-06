namespace chowdsp
{
namespace VersionUtils
{
    Version::Version (const juce::String& versionStr)
    {
        int numDots = 0;
        for (auto ch : versionStr)
            if (ch == '.')
                numDots++;

        // Valid version strings must have two dots!
        jassert (numDots == 2);

        auto trimmedStr = versionStr.retainCharacters ("1234567890.");

        juce::StringArray tokens;
        int numTokens = tokens.addTokens (trimmedStr, ".", "");
        jassert (numTokens == 3);
        juce::ignoreUnused (numTokens);

        major = tokens[0].getIntValue();
        minor = tokens[1].getIntValue();
        patch = tokens[2].getIntValue();
    }

    juce::String Version::getVersionString() const
    {
        return juce::String (major) + "." + juce::String (minor) + "." + juce::String (patch);
    }

    bool operator== (const Version& v1, const Version& v2)
    {
        return v1.major == v2.major && v1.minor == v2.minor && v1.patch == v2.patch;
    }

    bool operator!= (const Version& v1, const Version& v2)
    {
        return ! (v1 == v2);
    }

    bool operator> (const Version& v1, const Version& v2)
    {
        return v1.major > v2.major
               || (v1.major == v2.major && v1.minor > v2.minor)
               || (v1.major == v2.major && v1.minor == v2.minor && v1.patch > v2.patch);
    }

    bool operator< (const Version& v1, const Version& v2)
    {
        return v1.major < v2.major
               || (v1.major == v2.major && v1.minor < v2.minor)
               || (v1.major == v2.major && v1.minor == v2.minor && v1.patch < v2.patch);
    }

    bool operator>= (const Version& v1, const Version& v2)
    {
        return v1 > v2 || v1 == v2;
    }

    bool operator<= (const Version& v1, const Version& v2)
    {
        return v1 < v2 || v1 == v2;
    }

} // namespace VersionUtils
} // namespace chowdsp
