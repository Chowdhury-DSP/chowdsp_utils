#include "chowdsp_Preset.h"

namespace chowdsp
{
Preset::Preset (const juce::String& thisName,
                const juce::String& thisVendor,
                const juce::XmlElement& stateXml,
                const juce::String& thisCategory) : name (thisName),
                                                    vendor (thisVendor),
                                                    category (thisCategory),
#if defined JucePlugin_VersionString
                                                    version (std::make_unique<VersionUtils::Version> (JucePlugin_VersionString)),
#else
                                                    version (std::make_unique<VersionUtils::Version> ("0.0.0")),
#endif
                                                    state (std::make_unique<juce::XmlElement> (stateXml))
{
}

Preset::Preset (const juce::File& presetFile)
{
    file = presetFile;
    auto xml = juce::XmlDocument::parse (presetFile);
    initialise (xml.get());
}

Preset::Preset (const juce::XmlElement* presetXml)
{
    initialise (presetXml);
}

Preset::Preset (const void* presetData, size_t presetDataSize)
{
    auto xmlText = juce::String::createStringFromData (presetData, (int) presetDataSize);

    jassert (xmlText.isNotEmpty()); // preset does not exist!!
    auto xml = juce::XmlDocument::parse (xmlText);

    initialise (xml.get());
}

void Preset::initialise (const juce::XmlElement* xml)
{
    if (xml == nullptr)
        return;

    if (xml->getTagName() != presetTag.toString())
        return;

    name = xml->getStringAttribute (nameTag);
    if (name.isEmpty())
        return;

#if defined JucePlugin_Name
    if (xml->getStringAttribute (pluginTag) != JucePlugin_Name)
        return;
#endif

    vendor = xml->getStringAttribute (vendorTag);
    if (vendor.isEmpty())
        return;

    category = xml->getStringAttribute (categoryTag);

    auto versionStr = xml->getStringAttribute (versionTag);
    if (versionStr.isEmpty())
        return;

    version = std::make_unique<VersionUtils::Version> (versionStr);

    if (auto* xmlExtraInfo = xml->getChildByName (extraInfoTag))
        extraInfo = std::move (*xmlExtraInfo);

    auto* xmlState = xml->getChildElement (0);
    if (xmlState == nullptr)
        return;

    state = std::make_unique<juce::XmlElement> (*xmlState);
}

void Preset::toFile (const juce::File& presetFile)
{
    auto presetXml = toXml();

    if (presetXml == nullptr)
        return;

    presetFile.deleteRecursively();
    presetFile.create();
    presetXml->writeTo (presetFile);

    file = presetFile;
}

std::unique_ptr<juce::XmlElement> Preset::toXml() const
{
    if (! isValid())
        return {};

    auto presetXml = std::make_unique<juce::XmlElement> (presetTag);

    presetXml->setAttribute (nameTag, name);
#if defined JucePlugin_Name
    presetXml->setAttribute (pluginTag, JucePlugin_Name);
#else
    presetXml->setAttribute (pluginTag, juce::String());
#endif
    presetXml->setAttribute (vendorTag, vendor);
    presetXml->setAttribute (categoryTag, category);
    presetXml->setAttribute (versionTag, version->getVersionString());

    presetXml->addChildElement (new juce::XmlElement (*state));
    presetXml->addChildElement (new juce::XmlElement (extraInfo));

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpessimizing-move")
    return std::move (presetXml);
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

bool Preset::isValid() const
{
    return state != nullptr;
}

bool Preset::operator== (const Preset& other) const noexcept
{
    if (version == nullptr || other.version == nullptr)
        return false;

    if (state == nullptr)
        return false;

    const auto infoEqual = name == other.name && vendor == other.vendor && category == other.category && *version == *other.version;
    const auto stateEqual = state->isEquivalentTo (other.state.get(), true);
    const auto extraInfoEqual = extraInfo.isEquivalentTo (&other.extraInfo, true);

    return infoEqual && stateEqual && extraInfoEqual;
}

bool Preset::operator!= (const Preset& other) const noexcept
{
    return ! (*this == other);
}

const juce::Identifier Preset::presetTag { "Preset" };
const juce::Identifier Preset::nameTag { "name" };
const juce::Identifier Preset::pluginTag { "plugin" };
const juce::Identifier Preset::vendorTag { "vendor" };
const juce::Identifier Preset::categoryTag { "category" };
const juce::Identifier Preset::versionTag { "version" };
const juce::Identifier Preset::extraInfoTag { "extra_info" };
[[maybe_unused]] const juce::Identifier Preset::stateTag { "Parameters" };

} // namespace chowdsp
