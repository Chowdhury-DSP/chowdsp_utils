#include <CatchUtils.h>
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>
#include <chowdsp_clap_extensions/chowdsp_clap_extensions.h>

#include "TestPresetBinaryData.h"

#if JUCE_LINUX || JUCE_MAC
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-field-initializers", "-Wpedantic")

TEST_CASE ("CLAP Presets Discovery Test", "[plugin][presets][clap]")
{
    SECTION ("Embedded Presets Discovery")
    {
        struct EmbeddedPresetsProvider : chowdsp::presets::discovery::EmbeddedPresetsProvider
        {
            using chowdsp::presets::discovery::EmbeddedPresetsProvider::EmbeddedPresetsProvider;

            std::vector<chowdsp::presets::Preset> getPresets() override
            {
                return {
                    chowdsp::presets::Preset { BinaryData::test_preset_preset, BinaryData::test_preset_presetSize },
                };
            }
        };

        static constexpr clap_universal_plugin_id id {
            .abi = "clap",
            .id = "org.chowdsp.test-plugin",
        };

        static constexpr clap_preset_discovery_provider_descriptor descriptor {
            .clap_version = CLAP_VERSION_INIT,
            .id = "org.chowdsp.test-plugin.factory-presets",
            .name = "Test Factory Presets Provider",
            .vendor = "ChowDSP"
        };

        static constexpr clap_preset_discovery_location location {
            .flags = CLAP_PRESET_DISCOVERY_IS_FACTORY_CONTENT,
            .name = "Test Factory Presets",
            .kind = CLAP_PRESET_DISCOVERY_LOCATION_PLUGIN,
            .location = nullptr,
        };

        static constexpr clap_preset_discovery_indexer indexer {
            .clap_version = CLAP_VERSION,
            .name = "Test Indexer",
            .vendor = "ChowDSP",
            .url = "https://chowdsp.com",
            .version = "0.0.0",
            .declare_location = [] ([[maybe_unused]] const clap_preset_discovery_indexer* test_indexer,
                                    const clap_preset_discovery_location_t* test_location) -> bool
            {
                REQUIRE (test_location == &location);
                return true;
            },
            .declare_soundpack = nullptr,
            .get_extension = nullptr,
        };

        EmbeddedPresetsProvider provider {
            id,
            descriptor,
            location,
            &indexer,
        };

        provider.init();

        SECTION ("Normal")
        {
            static constexpr clap_preset_discovery_metadata_receiver receiver {
                .begin_preset = [] (const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                    const char* name,
                                    const char* load_key) -> bool
                {
                    REQUIRE (test_receiver == &receiver);
                    REQUIRE (std::string_view { name } == "Name");
                    REQUIRE (std::string_view { load_key } == "Name");
                    return true;
                },
                .add_plugin_id = [] (const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                     const clap_universal_plugin_id_t* plugin_id) -> void
                {
                    REQUIRE (test_receiver == &receiver);
                    REQUIRE (plugin_id == &id);
                },
                .add_creator = [] (const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                   const char* creator) -> void
                {
                    REQUIRE (test_receiver == &receiver);
                    REQUIRE (std::string_view { creator } == "Vendor");
                },
                .add_feature = [] (const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                   const char* feature) -> void
                {
                    REQUIRE (test_receiver == &receiver);
                    REQUIRE (std::string_view { feature } == "DRUM");
                },
            };

            const auto result = provider.getMetadata (CLAP_PRESET_DISCOVERY_LOCATION_PLUGIN,
                                                      nullptr,
                                                      &receiver);
            REQUIRE (result);
        }

        SECTION ("Wrong Location Kind")
        {
            static constexpr clap_preset_discovery_metadata_receiver receiver {
                .begin_preset = [] ([[maybe_unused]] const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                    [[maybe_unused]] const char* name,
                                    [[maybe_unused]] const char* load_key) -> bool
                {
                    REQUIRE (false);
                    return true;
                },
            };

            const auto result = provider.getMetadata (CLAP_PRESET_DISCOVERY_LOCATION_FILE,
                                                      nullptr,
                                                      &receiver);
            REQUIRE (! result);
        }

        SECTION ("Begin Preset Returns False")
        {
            static constexpr clap_preset_discovery_metadata_receiver receiver {
                .begin_preset = [] (const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                    const char* name,
                                    const char* load_key) -> bool
                {
                    REQUIRE (test_receiver == &receiver);
                    REQUIRE (std::string_view { name } == "Name");
                    REQUIRE (std::string_view { load_key } == "Name");
                    return false;
                },
                .add_plugin_id = [] ([[maybe_unused]] const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                     [[maybe_unused]] const clap_universal_plugin_id_t* plugin_id) -> void
                {
                    REQUIRE (false);
                },
            };

            const auto result = provider.getMetadata (CLAP_PRESET_DISCOVERY_LOCATION_PLUGIN,
                                                      nullptr,
                                                      &receiver);
            REQUIRE (result);
        }
    }

    SECTION ("File Presets Discovery")
    {
        static constexpr auto get_presets_folder = []
        {
            return juce::File::getSpecialLocation (juce::File::userDesktopDirectory).getChildFile ("test presets");
        };
        const auto folder = get_presets_folder();
        if (folder.isDirectory())
            folder.deleteRecursively();
        folder.createDirectory();
        const auto _ = chowdsp::runAtEndOfScope ([&folder]
                                                 { folder.deleteRecursively(); });
        chowdsp::presets::Preset preset { "Name", "Vendor", { { "tag", 0.0f } }, "Category" };
        const auto presetFile = folder.getChildFile ("preset.testpreset");
        preset.toFile (presetFile);
        chowdsp::JSONUtils::toFile ({}, folder.getChildFile ("badpreset.testpreset"));

        struct FilePresetsProvider : chowdsp::presets::discovery::FilePresetsProvider
        {
            using chowdsp::presets::discovery::FilePresetsProvider::FilePresetsProvider;

            juce::String path {};

            bool fillInLocation (clap_preset_discovery_location& location) override
            {
                path = get_presets_folder().getFullPathName();

                location.name = "Test User Presets Location";
                location.location = path.toRawUTF8();
                return true;
            }
        };

        static constexpr clap_universal_plugin_id id {
            .abi = "clap",
            .id = "org.chowdsp.test-plugin",
        };

        static constexpr clap_preset_discovery_provider_descriptor descriptor {
            .clap_version = CLAP_VERSION_INIT,
            .id = "org.chowdsp.test-plugin.user-presets",
            .name = "Test User Presets Provider",
            .vendor = "ChowDSP"
        };

        static constexpr clap_preset_discovery_filetype filetype {
            .name = "Test Preset Filetype",
            .description = "",
            .file_extension = "testpreset",
        };

        static constexpr clap_preset_discovery_indexer indexer {
            .clap_version = CLAP_VERSION,
            .name = "Test Indexer",
            .vendor = "ChowDSP",
            .url = "https://chowdsp.com",
            .version = "0.0.0",
            .declare_filetype = [] ([[maybe_unused]] const struct clap_preset_discovery_indexer* test_indexer,
                                    const clap_preset_discovery_filetype_t* test_filetype) -> bool
            {
                REQUIRE (test_filetype == &filetype);
                return true;
            },
            .declare_location = [] ([[maybe_unused]] const clap_preset_discovery_indexer* test_indexer,
                                    const clap_preset_discovery_location_t* test_location) -> bool
            {
                REQUIRE (juce::String { test_location->location } == get_presets_folder().getFullPathName());
                return true;
            },
        };

        FilePresetsProvider provider {
            id,
            descriptor,
            filetype,
            &indexer,
        };

        provider.init();

        SECTION ("Normal")
        {
            struct ReceiverData
            {
                chowdsp::presets::Preset* p;
                const juce::File* preset_file;
            } data;
            data.p = &preset;
            data.preset_file = &presetFile;

            clap_preset_discovery_metadata_receiver receiver {
                .receiver_data = &data,
                .begin_preset = [] (const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                    const char* name,
                                    [[maybe_unused]] const char* load_key) -> bool
                {
                    const auto* r_data = static_cast<ReceiverData*> (test_receiver->receiver_data);
                    REQUIRE (juce::String { name } == r_data->preset_file->getFullPathName());
                    return true;
                },
                .add_plugin_id = [] ([[maybe_unused]] const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                     const clap_universal_plugin_id_t* plugin_id) -> void
                {
                    REQUIRE (plugin_id == &id);
                },
                .add_creator = [] (const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                   const char* creator) -> void
                {
                    const auto* r_data = static_cast<ReceiverData*> (test_receiver->receiver_data);
                    REQUIRE (juce::String { creator } == r_data->p->getVendor());
                },
                .set_timestamps = [] (const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                      clap_timestamp creation_time,
                                      clap_timestamp modification_time) -> void
                {
                    const auto* r_data = static_cast<ReceiverData*> (test_receiver->receiver_data);
                    REQUIRE (creation_time == (clap_timestamp) r_data->preset_file->getCreationTime().toMilliseconds() / 1000);
                    REQUIRE (modification_time == (clap_timestamp) r_data->preset_file->getLastModificationTime().toMilliseconds() / 1000);
                },
                .add_feature = [] (const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                   const char* feature) -> void
                {
                    const auto* r_data = static_cast<ReceiverData*> (test_receiver->receiver_data);
                    REQUIRE (juce::String { feature } == r_data->p->getCategory());
                },
            };

            for (auto& entry : juce::RangedDirectoryIterator { folder, false, "*.testpreset" })
            {
                const auto path = entry.getFile().getFullPathName();
                const auto result = provider.getMetadata (CLAP_PRESET_DISCOVERY_LOCATION_FILE,
                                                          path.toRawUTF8(),
                                                          &receiver);
                REQUIRE (result == (path == presetFile.getFullPathName()));
            }
        }

        SECTION ("Wrong Location Kind")
        {
            static constexpr clap_preset_discovery_metadata_receiver receiver {
                .begin_preset = [] ([[maybe_unused]] const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                    [[maybe_unused]] const char* name,
                                    [[maybe_unused]] const char* load_key) -> bool
                {
                    REQUIRE (false);
                    return true;
                },
            };

            const auto result = provider.getMetadata (CLAP_PRESET_DISCOVERY_LOCATION_PLUGIN,
                                                      nullptr,
                                                      &receiver);
            REQUIRE (! result);
        }

        SECTION ("Missing Location")
        {
            static constexpr clap_preset_discovery_metadata_receiver receiver {
                .begin_preset = [] ([[maybe_unused]] const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                    [[maybe_unused]] const char* name,
                                    [[maybe_unused]] const char* load_key) -> bool
                {
                    REQUIRE (false);
                    return true;
                },
            };

            const auto result = provider.getMetadata (CLAP_PRESET_DISCOVERY_LOCATION_FILE,
                                                      nullptr,
                                                      &receiver);
            REQUIRE (! result);
        }

        SECTION ("Non-Existent Preset File")
        {
            static constexpr clap_preset_discovery_metadata_receiver receiver {
                .begin_preset = [] ([[maybe_unused]] const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                    [[maybe_unused]] const char* name,
                                    [[maybe_unused]] const char* load_key) -> bool
                {
                    REQUIRE (false);
                    return true;
                },
            };

            const auto nonexistentFile = folder.getNonexistentChildFile ("nonexistent_file", "testpreset");
            const auto result = provider.getMetadata (CLAP_PRESET_DISCOVERY_LOCATION_FILE,
                                                      nonexistentFile.getFullPathName().toRawUTF8(),
                                                      &receiver);
            REQUIRE (! result);
        }

        SECTION ("Begin Preset Returns False")
        {
            static constexpr clap_preset_discovery_metadata_receiver receiver {
                .begin_preset = [] (const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                    [[maybe_unused]] const char* name,
                                    [[maybe_unused]] const char* load_key) -> bool
                {
                    REQUIRE (test_receiver == &receiver);
                    return false;
                },
                .add_plugin_id = [] ([[maybe_unused]] const struct clap_preset_discovery_metadata_receiver* test_receiver,
                                     [[maybe_unused]] const clap_universal_plugin_id_t* plugin_id) -> void
                {
                    REQUIRE (false);
                },
            };

            const auto result = provider.getMetadata (CLAP_PRESET_DISCOVERY_LOCATION_FILE,
                                                      presetFile.getFullPathName().toRawUTF8(),
                                                      &receiver);
            REQUIRE (result);
        }
    }
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#endif // JUCE_LINUX || JUCE_MAC
