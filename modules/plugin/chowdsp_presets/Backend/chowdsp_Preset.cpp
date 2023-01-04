#include "chowdsp_Preset.h"

namespace chowdsp
{
Preset::Preset (const juce::String& thisName,
                const juce::String& thisVendor,
                const juce::XmlElement& stateXml,
                const juce::String& thisCategory,
                const juce::File& sourceFile) : name (thisName),
                                                vendor (thisVendor),
                                                category (thisCategory),
                                                state (std::make_unique<juce::XmlElement> (stateXml)),
                                                file (sourceFile)
{
}

Preset::Preset (const juce::File& presetFile) : file (presetFile)
{
    auto xml = juce::XmlDocument::parse (presetFile);
    initialise (xml.get());
}

Preset::Preset (const juce::XmlElement* presetXml)
{
    initialise (presetXml);
}

Preset::Preset (const void* presetData, size_t presetDataSize) // NOSONAR (JUCE BinaryData uses void*)
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

    version = Version { versionStr };

    const auto presetSavedFile = juce::File { xml->getStringAttribute (fileTag) };
    if (presetSavedFile.existsAsFile())
        file = presetSavedFile;

    if (auto* xmlExtraInfo = xml->getChildByName (extraInfoTag))
        extraInfo = std::move (*xmlExtraInfo);

    auto* xmlState = xml->getChildElement (0);
    if (xmlState == nullptr)
        return;

    state = std::make_unique<juce::XmlElement> (*xmlState);
}

void Preset::toFile (const juce::File& presetFile)
{
    file = presetFile;
    auto presetXml = toXml();

    if (presetXml == nullptr)
        return;

    presetFile.deleteRecursively();
    presetFile.create();
    presetXml->writeTo (presetFile);
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
    presetXml->setAttribute (versionTag, version.getVersionString());
    presetXml->setAttribute (fileTag, file.getFullPathName());

    presetXml->addChildElement (new juce::XmlElement (*state));
    presetXml->addChildElement (new juce::XmlElement (extraInfo));

    return presetXml;
}

bool Preset::isValid() const
{
    return state != nullptr;
}

bool Preset::operator== (const Preset& other) const noexcept
{
    if (state == nullptr)
        return false;

    const auto infoEqual = name == other.name && vendor == other.vendor && category == other.category && version == other.version;
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
const juce::Identifier Preset::fileTag { "preset_file" };
const juce::Identifier Preset::extraInfoTag { "extra_info" };
[[maybe_unused]] const juce::Identifier Preset::stateTag { "Parameters" };

} // namespace chowdsp
