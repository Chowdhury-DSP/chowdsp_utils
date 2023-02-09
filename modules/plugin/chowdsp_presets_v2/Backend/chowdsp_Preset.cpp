#include "chowdsp_Preset.h"

namespace chowdsp::presets
{
Preset::Preset (const juce::String& pName,
                const juce::String& pVendor,
                nlohmann::json&& pStateJson,
                const juce::String& pCategory,
                const juce::File& pFile)
    : name (pName),
      vendor (pVendor),
      category (pCategory),
      state (std::move (pStateJson)),
      file (pFile)
{
}

Preset::Preset (const juce::File& presetFile) : file (presetFile)
{
    initialiseSafe ([this]
                    { return JSONUtils::fromFile (file); },
                    "file (" + file.getFullPathName() + ")");
}

Preset::Preset (const nlohmann::json& presetJson)
{
    initialiseSafe (presetJson, "json");
}

Preset::Preset (const void* presetData, size_t presetDataSize) // NOSONAR
{
    initialiseSafe ([&presetData, &presetDataSize]
                    { return JSONUtils::fromBinaryData (presetData, (int) presetDataSize); },
                    "binary data");
}

template <typename JSONGetType>
void Preset::initialiseSafe (JSONGetType jsonGetter, const juce::String& source)
{
    try
    {
        if constexpr (std::is_same_v<std::decay_t<JSONGetType>, nlohmann::json>)
            initialise (jsonGetter);
        else
            initialise (jsonGetter());
    }
    catch (const std::exception& exception) // NOSONAR
    {
        juce::Logger::writeToLog ("Error loading preset from " + source + ": " + juce::String { exception.what() });
        jassertfalse;
        state = {};
        file = juce::File {};
    }
}

void Preset::initialise (const nlohmann::json& presetJson)
{
    if (presetJson.is_null())
        return;

    name = presetJson.at (nameTag);
    vendor = presetJson.at (vendorTag);
    category = presetJson.at (categoryTag);
    if (name.isEmpty() || vendor.isEmpty())
        throw std::runtime_error ("Preset does not contain name or vendor!"); // NOSONAR

#if defined JucePlugin_Name
    if (presetJson.at (pluginTag) != JucePlugin_Name)
        throw std::runtime_error ("Preset was saved from a different plugin!"); // NOSONAR
#endif

    const juce::String versionStr = presetJson.at (versionTag);
    if (versionStr.isEmpty())
        throw std::runtime_error ("Preset does not contain a valid version!"); // NOSONAR
    version = Version { versionStr };

    file = presetJson.value (fileTag, file.getFullPathName());
    state = presetJson.at (stateTag);
}

nlohmann::json Preset::toJson() const
{
    if (! isValid())
        return {};

    return
    {
        { nameTag, name },
#if defined JucePlugin_Name
            { pluginTag, JucePlugin_Name },
#endif
            { vendorTag, vendor },
            { categoryTag, category },
            { versionTag, version.getVersionString() },
            { fileTag, file.getFullPathName() },
            { stateTag, state },
    };
}

void Preset::toFile (const juce::File& presetFile)
{
    file = presetFile;
    const auto presetJson = toJson();
    if (presetJson.is_null())
    {
        file = juce::File {};
        return;
    }

    JSONUtils::toFile (presetJson, presetFile, 4);
}

bool Preset::isValid() const
{
    return ! state.is_null();
}

bool Preset::operator== (const Preset& other) const noexcept
{
    if (state.is_null() || other.state.is_null())
        return false;

    return name == other.name
           && vendor == other.vendor
           && category == other.category
           && version == other.version
           && state == other.state;
}

bool Preset::operator!= (const Preset& other) const noexcept
{
    return ! (*this == other);
}
} // namespace chowdsp::presets
