#pragma once

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-parameter")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4100)
#include <clap/helpers/preset-discovery-provider.hh>
JUCE_END_IGNORE_WARNINGS_MSVC
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

namespace chowdsp::presets
{
class Preset;
}

namespace chowdsp::presets::discovery
{
using CLAPPresetsProviderBase =
#if JUCE_DEBUG
    clap::helpers::PresetDiscoveryProvider<clap::helpers::MisbehaviourHandler::Terminate, clap::helpers::CheckingLevel::Maximal>;
#else
    clap::helpers::PresetDiscoveryProvider<clap::helpers::MisbehaviourHandler::Ignore, clap::helpers::CheckingLevel::Minimal>;
#endif

/** A CLAP preset provider for presets that are embedded in the plugin's binary data. */
struct EmbeddedPresetsProvider : CLAPPresetsProviderBase
{
    const clap_universal_plugin_id& this_plugin_id;
    const clap_preset_discovery_location& discoveryLocation {};

    EmbeddedPresetsProvider (const clap_universal_plugin_id& this_plug_id,
                             const clap_preset_discovery_provider_descriptor& desc,
                             const clap_preset_discovery_location& location,
                             const clap_preset_discovery_indexer* indexer);

    /** Users are expected to override this method to provide the relevant presets. */
    virtual std::vector<Preset> getPresets();

    bool init() noexcept override;
    bool getMetadata (uint32_t location_kind,
                      const char* location,
                      const clap_preset_discovery_metadata_receiver_t* metadata_receiver) noexcept override;
};

/** A CLAP preset provider for presets that are stored in the user's filesystem. */
struct FilePresetsProvider : CLAPPresetsProviderBase
{
    const clap_universal_plugin_id& this_plugin_id;
    const clap_preset_discovery_filetype& presets_filetype;
    clap_preset_discovery_location discoveryLocation {};

    FilePresetsProvider (const clap_universal_plugin_id& this_plug_id,
                         const clap_preset_discovery_provider_descriptor& desc,
                         const clap_preset_discovery_filetype& filetype,
                         const clap_preset_discovery_indexer* indexer);

    /** Users are expected to override this method to fill in the location name and path. */
    virtual bool fillInLocation (clap_preset_discovery_location&) = 0;

    bool init() noexcept override;
    bool getMetadata (uint32_t location_kind,
                      const char* location,
                      const clap_preset_discovery_metadata_receiver_t* metadata_receiver) noexcept override;
};
} // namespace chowdsp::presets::discovery
