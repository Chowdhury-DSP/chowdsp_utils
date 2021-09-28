#pragma once

namespace chowdsp
{
/** Information for storing a plugin preset */
struct Preset
{
    /** Create a preset from the preset information */
    Preset (const juce::String& name, const juce::String& vendor, const juce::XmlElement& stateXml, const juce::String& category = {});

    /** Create a preset from a file */
    Preset (const juce::File& presetFile);

    /** Create a preset from xml */
    Preset (const juce::XmlElement* presetXml);

    /** Create a preset from BinaryData */
    Preset (const void* presetData, size_t presetDataSize);

    /** Move constructor */
    Preset (Preset&&) = default;

    /** Saves this preset to a file */
    void toFile (const juce::File& presetFile) const;

    /** Saves this preset to xml */
    std::unique_ptr<juce::XmlElement> toXml() const;

    /** Initialise the preset from XML data */
    void initialise (const juce::XmlElement* xml);

    /** Returns true if this preset is valid */
    bool isValid() const;

    /** Returns the preset's name */
    const juce::String& getName() const noexcept { return name; }

    /** Returns the name of the vendor that created this preset */
    const juce::String& getVendor() const noexcept { return vendor; }

    /** Returns the name of the preset category */
    const juce::String& getCategory() const noexcept { return category; }

    /** Returns the version of the plugin that was used to create this preset */
    const VersionUtils::Version& getVersion() const noexcept { return *version; }

    /** Returns the preset's state */
    const juce::XmlElement* getState() const noexcept { return state.get(); }

    friend bool operator== (const Preset& p1, const Preset& p2);

    static const juce::Identifier presetTag;
    static const juce::Identifier nameTag;
    static const juce::Identifier pluginTag;
    static const juce::Identifier vendorTag;
    static const juce::Identifier categoryTag;
    static const juce::Identifier versionTag;
    static const juce::Identifier stateTag;

private:
    juce::String name;
    juce::String vendor;
    juce::String category;
    std::unique_ptr<VersionUtils::Version> version;
    std::unique_ptr<juce::XmlElement> state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Preset)
};

} // namespace chowdsp
