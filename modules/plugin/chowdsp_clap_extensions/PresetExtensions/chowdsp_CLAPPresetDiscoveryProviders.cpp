#include "chowdsp_CLAPPresetDiscoveryProviders.h"

#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-parameter")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4100 4127)
#include <clap/helpers/preset-discovery-provider.hh>
#include <clap/helpers/preset-discovery-provider.hxx>
JUCE_END_IGNORE_WARNINGS_MSVC
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

namespace chowdsp::presets::discovery
{
EmbeddedPresetsProvider::EmbeddedPresetsProvider (const clap_universal_plugin_id& this_plug_id,
                                                  const clap_preset_discovery_provider_descriptor& desc,
                                                  const clap_preset_discovery_location& location,
                                                  const clap_preset_discovery_indexer* indexer)
    : CLAPPresetsProviderBase (&desc, indexer),
      this_plugin_id (this_plug_id),
      discoveryLocation (location)
{
    // CLAP requires that a location containing embedded presets must be nullptr
    jassert (discoveryLocation.location == nullptr);
}

std::vector<Preset> EmbeddedPresetsProvider::getPresets() { return {}; } // LCOV_EXCL_LINE

bool EmbeddedPresetsProvider::init() noexcept
{
    indexer()->declare_location (indexer(), &discoveryLocation);
    return true;
}

bool EmbeddedPresetsProvider::getMetadata (uint32_t location_kind,
                                           [[maybe_unused]] const char* location,
                                           const clap_preset_discovery_metadata_receiver_t* metadata_receiver) noexcept
{
    if (location_kind != CLAP_PRESET_DISCOVERY_LOCATION_PLUGIN)
        return false;

    for (const auto& factoryPreset : getPresets())
    {
        DBG ("Indexing factory preset: " + factoryPreset.getName());
        if (metadata_receiver->begin_preset (metadata_receiver, factoryPreset.getName().toRawUTF8(), factoryPreset.getName().toRawUTF8()))
        {
            metadata_receiver->add_plugin_id (metadata_receiver, &this_plugin_id);
            metadata_receiver->add_creator (metadata_receiver, factoryPreset.getVendor().toRawUTF8());

            if (factoryPreset.getCategory().isNotEmpty())
                metadata_receiver->add_feature (metadata_receiver, factoryPreset.getCategory().toRawUTF8());
        }
        else
        {
            break;
        }
    }

    return true;
}

//==============================================================================
FilePresetsProvider::FilePresetsProvider (const clap_universal_plugin_id& this_plug_id,
                                          const clap_preset_discovery_provider_descriptor& desc,
                                          const clap_preset_discovery_filetype& filetype,
                                          const clap_preset_discovery_indexer* indexer)
    : CLAPPresetsProviderBase (&desc, indexer),
      this_plugin_id (this_plug_id),
      presets_filetype (filetype)
{
}

bool FilePresetsProvider::init() noexcept
{
    indexer()->declare_filetype (indexer(), &presets_filetype);

    discoveryLocation.flags = CLAP_PRESET_DISCOVERY_IS_USER_CONTENT;
    discoveryLocation.kind = CLAP_PRESET_DISCOVERY_LOCATION_FILE;
    if (! fillInLocation (discoveryLocation))
        return false;

    indexer()->declare_location (indexer(), &discoveryLocation);

    return true;
}

bool FilePresetsProvider::getMetadata (uint32_t location_kind,
                                       const char* location,
                                       const clap_preset_discovery_metadata_receiver_t* metadata_receiver) noexcept
{
    if (location_kind != CLAP_PRESET_DISCOVERY_LOCATION_FILE || location == nullptr)
        return false;

    const auto userPresetFile = juce::File { location };
    if (! userPresetFile.existsAsFile())
        return false;

    Preset preset { userPresetFile };
    if (! preset.isValid())
        return false;

    DBG ("Indexing user preset: " + preset.getName() + ", from path: " + userPresetFile.getFullPathName());
    if (metadata_receiver->begin_preset (metadata_receiver, userPresetFile.getFullPathName().toRawUTF8(), ""))
    {
        metadata_receiver->add_plugin_id (metadata_receiver, &this_plugin_id);
        metadata_receiver->add_creator (metadata_receiver, preset.getVendor().toRawUTF8());

        if (preset.getCategory().isNotEmpty())
            metadata_receiver->add_feature (metadata_receiver, preset.getCategory().toRawUTF8());

        metadata_receiver->set_timestamps (metadata_receiver,
                                           (clap_timestamp) userPresetFile.getCreationTime().toMilliseconds() / 1000,
                                           (clap_timestamp) userPresetFile.getLastModificationTime().toMilliseconds() / 1000);
    }

    return true;
}
} // namespace chowdsp::presets::discovery
