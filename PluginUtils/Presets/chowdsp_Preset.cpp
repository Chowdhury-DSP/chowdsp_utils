namespace chowdsp
{
Preset::Preset (const juce::String& thisName,
                const juce::String& thisVendor,
                juce::XmlElement&& stateXml) : name (thisName),
                                               vendor (thisVendor),
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
    auto xml = juce::XmlDocument::parse (presetFile);
    initialise (xml.get());
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

    auto versionStr = xml->getStringAttribute (versionTag);
    if (versionStr.isEmpty())
        return;

    version = std::make_unique<VersionUtils::Version> (versionStr);
    auto* xmlState = xml->getChildElement (0);
    if (xmlState == nullptr)
        return;

    state = std::make_unique<juce::XmlElement> (*xmlState);
}

void Preset::toFile (const juce::File& presetFile) const
{
    if (! isValid())
        return;

    auto presetXml = std::make_unique<juce::XmlElement> (presetTag);

    presetXml->setAttribute (nameTag, name);
#if defined JucePlugin_Name
    presetXml->setAttribute (pluginTag, JucePlugin_Name);
#else
    presetXml->setAttribute (pluginTag, {});
#endif
    presetXml->setAttribute (vendorTag, vendor);
    presetXml->setAttribute (versionTag, version->getVersionString());

    presetXml->addChildElement (new juce::XmlElement (*state));

    presetFile.deleteRecursively();
    presetFile.create();
    presetXml->writeTo (presetFile);
}

bool Preset::isValid() const
{
    return state != nullptr;
}

const juce::Identifier Preset::presetTag { "Preset" };
const juce::Identifier Preset::nameTag { "name" };
const juce::Identifier Preset::pluginTag { "plugin" };
const juce::Identifier Preset::vendorTag { "vendor" };
const juce::Identifier Preset::versionTag { "version" };
const juce::Identifier Preset::stateTag { "Parameters" };

} // namespace chowdsp
