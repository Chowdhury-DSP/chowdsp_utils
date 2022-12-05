#include "chowdsp_InfoProvider.h"

namespace chowdsp
{
juce::String StandardInfoProvider::getManufacturerString()
{
#if defined JucePlugin_Manufacturer
    return JucePlugin_Manufacturer;
#else
    return "Manu";
#endif
}

juce::URL StandardInfoProvider::getManufacturerWebsiteURL()
{
#if defined JucePlugin_ManufacturerWebsite
    return juce::URL { JucePlugin_ManufacturerWebsite };
#else
    return juce::URL { "https://chowdsp.com" };
#endif
}

juce::String StandardInfoProvider::getPlatformString()
{
    return toString (SystemInfo::getOSDescription()) + "-" + toString (SystemInfo::getProcArch());
}

juce::String StandardInfoProvider::getVersionString()
{
#if defined JucePlugin_VersionString
    return "v" + juce::String (JucePlugin_VersionString);
#else
    return juce::String ("No Version");
#endif
}
} // namespace chowdsp
