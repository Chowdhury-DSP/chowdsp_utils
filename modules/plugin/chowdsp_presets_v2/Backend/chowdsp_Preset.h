#pragma once

#include <chowdsp_version/chowdsp_version.h>

namespace chowdsp::presets
{
/** Information for storing a plugin preset */
class Preset
{
public:
    /** Create a preset from the preset info. */
    Preset (const juce::String& name,
            const juce::String& vendor,
            nlohmann::json&& stateJson,
            const juce::String& category = {},
            const juce::File& file = {});

    /** Create a preset from a file */
    explicit Preset (const juce::File& presetFile);

    /** Create a preset from json */
    explicit Preset (const nlohmann::json& presetJson);

    /** Create a preset from BinaryData */
    Preset (const void* presetData, size_t presetDataSize);

    Preset (const Preset&) = default;
    Preset& operator= (const Preset&) = default;
    Preset (Preset&&) noexcept = default;
    Preset& operator= (Preset&&) noexcept = default;

    /** Serializes this preset to JSON */
    [[nodiscard]] nlohmann::json toJson() const;

    /** Saves this preset to a file */
    void toFile (const juce::File& presetFile);

    /** Returns true if this preset is valid */
    [[nodiscard]] bool isValid() const;

    /** Returns the name of the preset */
    [[nodiscard]] const juce::String& getName() const noexcept { return name; }

    /** Returns the name of the vendor that created this preset */
    [[nodiscard]] const juce::String& getVendor() const noexcept { return vendor; }

    /** Returns the name of the preset category */
    [[nodiscard]] const juce::String& getCategory() const noexcept { return category; }

    /** Returns the version of the plugin that was used to create this preset */
    [[nodiscard]] Version getVersion() const noexcept { return version; }

    /**
     * Returns the file path where this preset was loaded from.
     * If the preset was not loaded from a file, this will return an empty path.
     */
    [[nodiscard]] const juce::File& getPresetFile() const noexcept { return file; }

    /** Returns the preset state */
    [[nodiscard]] const auto& getState() const noexcept { return state; }

    /** Field to mark whether or not this is a factory preset. */
    bool isFactoryPreset = false;

    /**
     * Returns true if the two presets are equivalent.
     * Note that this compares the entire preset state.
     */
    bool operator== (const Preset& other) const noexcept;

    /** Returns true if the two presets are NOT equivalent. */
    bool operator!= (const Preset& other) const noexcept;

    static constexpr std::string_view presetTag { "preset" };
    static constexpr std::string_view nameTag { "name" };
    static constexpr std::string_view pluginTag { "plugin" };
    static constexpr std::string_view vendorTag { "vendor" };
    static constexpr std::string_view categoryTag { "category" };
    static constexpr std::string_view versionTag { "version" };
    static constexpr std::string_view fileTag { "preset_file" };
    static constexpr std::string_view stateTag { "preset_state" };

private:
    template <typename JSONGetType>
    void initialiseSafe (JSONGetType jsonGetter, const juce::String& source);
    void initialise (const nlohmann::json& presetJson);

    juce::String name;
    juce::String vendor;
    juce::String category;

#if defined JucePlugin_VersionString
    static constexpr std::string_view versionString = JucePlugin_VersionString;
#else
    static constexpr std::string_view versionString = "0.0.0";
#endif
    Version version { versionString };

    nlohmann::json state {};

    juce::File file {};

    JUCE_LEAK_DETECTOR (Preset)
};
} // namespace chowdsp::presets
