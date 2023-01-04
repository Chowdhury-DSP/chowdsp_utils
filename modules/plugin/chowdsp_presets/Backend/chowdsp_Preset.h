#pragma once

#include <chowdsp_version/chowdsp_version.h>

namespace chowdsp
{
/** Information for storing a plugin preset */
struct Preset
{
    /** Create a preset from the preset information */
    Preset (const juce::String& name, const juce::String& vendor, const juce::XmlElement& stateXml, const juce::String& category = {}, const juce::File& file = {});

    /** Create a preset from a file */
    explicit Preset (const juce::File& presetFile);

    /** Create a preset from xml */
    explicit Preset (const juce::XmlElement* presetXml);

    /** Create a preset from BinaryData */
    Preset (const void* presetData, size_t presetDataSize);

    /** Move constructor */
    Preset (Preset&&) noexcept = default;

    /** Move assignment operator */
    Preset& operator= (Preset&&) noexcept = default;

    /** Saves this preset to a file */
    void toFile (const juce::File& presetFile);

    /** Saves this preset to xml */
    [[nodiscard]] std::unique_ptr<juce::XmlElement> toXml() const;

    /** Initialise the preset from XML data */
    void initialise (const juce::XmlElement* xml);

    /** Returns true if this preset is valid */
    [[nodiscard]] bool isValid() const;

    /** Returns the preset's name */
    [[nodiscard]] const juce::String& getName() const noexcept { return name; }

    /** Returns the name of the vendor that created this preset */
    [[nodiscard]] const juce::String& getVendor() const noexcept { return vendor; }

    /** Changes the name of the vendor that created this preset */
    void setVendor (const juce::String& newVendor) { vendor = newVendor; }

    /** Returns the name of the preset category */
    [[nodiscard]] const juce::String& getCategory() const noexcept { return category; }

    /** Returns the version of the plugin that was used to create this preset */
    [[nodiscard]] const Version& getVersion() const noexcept { return version; }

    /** Returns the preset's state */
    [[nodiscard]] const juce::XmlElement* getState() const noexcept { return state.get(); }

    /**
     * Returns the file path where this preset was loaded from.
     * If the preset was not loaded from a file, this will return an empty path.
     */
    [[nodiscard]] const juce::File& getPresetFile() const noexcept { return file; }

    /** Returns true if the two presets are equivalent. */
    bool operator== (const Preset& other) const noexcept;

    /** Returns true if the two presets are NOT equivalent. */
    bool operator!= (const Preset& other) const noexcept;

    /** XML Element to store any extra information (e.g. a preset description) */
    juce::XmlElement extraInfo { extraInfoTag };

    static const juce::Identifier presetTag;
    static const juce::Identifier nameTag;
    static const juce::Identifier pluginTag;
    static const juce::Identifier vendorTag;
    static const juce::Identifier categoryTag;
    static const juce::Identifier versionTag;
    static const juce::Identifier fileTag;
    static const juce::Identifier extraInfoTag;
    [[maybe_unused]] static const juce::Identifier stateTag;

private:
    juce::String name;
    juce::String vendor;
    juce::String category;

#if defined JucePlugin_VersionString
    static constexpr std::string_view versionString = JucePlugin_VersionString;
#else
    static constexpr std::string_view versionString = "0.0.0";
#endif
    Version version { versionString };

    std::unique_ptr<juce::XmlElement> state;

    juce::File file;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Preset)
};

} // namespace chowdsp
