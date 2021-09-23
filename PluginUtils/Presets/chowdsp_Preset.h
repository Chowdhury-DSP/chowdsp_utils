#pragma once

namespace chowdsp
{
/** Information for storing a plugin preset */
struct Preset
{
    /** Create a preset from the preset information */
    Preset (const juce::String& name, const juce::String& vendor, juce::XmlElement&& stateXml);

    /** Create a preset from a file */
    Preset (const juce::File& presetFile);

    /** Create a preset from BinaryData */
    Preset (const void* presetData, size_t presetDataSize);

    /** Saves this preset to a file */
    void toFile (const juce::File& presetFile) const;

    /** initialise the preset from XML data */
    void initialise (const juce::XmlElement* xml);

    /** returns true if this preset is valid */
    bool isValid() const;

    juce::String name;
    juce::String vendor;
    std::unique_ptr<VersionUtils::Version> version;
    std::unique_ptr<juce::XmlElement> state;

    static const juce::Identifier presetTag;
    static const juce::Identifier nameTag;
    static const juce::Identifier pluginTag;
    static const juce::Identifier vendorTag;
    static const juce::Identifier versionTag;
    static const juce::Identifier stateTag;
};

} // namespace chowdsp
