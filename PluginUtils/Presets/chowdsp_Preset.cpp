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

    auto* xmlState = xml->getChildElement (0);
    if (xmlState == nullptr)
        return;

    state = std::make_unique<juce::XmlElement> (*xmlState);
}

void Preset::toFile (const juce::File& presetFile) const
{
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
    presetXml->setAttribute (pluginTag, {});
#endif
    presetXml->setAttribute (vendorTag, vendor);
    presetXml->setAttribute (categoryTag, category);
    presetXml->setAttribute (versionTag, version->getVersionString());

    presetXml->addChildElement (new juce::XmlElement (*state));

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpessimizing-move")
    return std::move (presetXml);
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

bool Preset::isValid() const
{
    return state != nullptr;
}

bool operator== (const Preset& p1, const Preset& p2)
{
    if (p1.version == nullptr || p2.version == nullptr)
        return false;

    if (p1.state == nullptr)
        return false;

    return p1.name == p2.name && p1.vendor == p2.vendor && p1.category == p2.category && *p1.version == *p2.version
           && p1.state->isEquivalentTo (p2.state.get(), true);
}

const juce::Identifier Preset::presetTag { "Preset" };
const juce::Identifier Preset::nameTag { "name" };
const juce::Identifier Preset::pluginTag { "plugin" };
const juce::Identifier Preset::vendorTag { "vendor" };
const juce::Identifier Preset::categoryTag { "category" };
const juce::Identifier Preset::versionTag { "version" };
const juce::Identifier Preset::stateTag { "Parameters" };

} // namespace chowdsp
