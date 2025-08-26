#include "chowdsp_Version.h"

namespace chowdsp
{
Version::Version (const juce::String& versionStr)
{
    [[maybe_unused]] int numDots = 0;
    for (auto ch : versionStr)
        if (ch == '.')
            numDots++;

    // Valid version strings must have one or two dots!
    jassert (numDots == 1 || numDots == 2);

    auto trimmedStr = versionStr.retainCharacters ("1234567890.");

    juce::StringArray tokens;
    [[maybe_unused]] int numTokens = tokens.addTokens (trimmedStr, ".", "");
    jassert (numTokens == 2 || numTokens == 3);

    major = tokens[0].getIntValue();
    minor = tokens[1].getIntValue();
    patch = numTokens == 3 ? tokens[2].getIntValue() : 0;
}

juce::String Version::getVersionString() const
{
    return juce::String (major) + "." + juce::String (minor) + "." + juce::String (patch);
}
} // namespace chowdsp
