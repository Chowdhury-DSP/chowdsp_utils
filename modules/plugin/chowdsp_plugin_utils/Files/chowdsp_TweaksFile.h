#pragma once

#include "../../../common/chowdsp_json/chowdsp_json.h"
#include "chowdsp_FileListener.h"

namespace chowdsp::experimental
{
/**
 * A file to hold some configuration that can be tweaked
 * without needing to re-compile the plugin.
 */
class TweaksFile
{
public:
    TweaksFile() = default;

    /** Type alias for setting ID */
    using PropertyID = std::string_view;

    /** Type alias for setting property with ID */
    using Property = std::pair<PropertyID, json>;

#if CHOWDSP_BAKE_TWEAKS
    /** Initialises the file from a BinaryData object. If the file has already been initialised, this will do nothing. */
    void initialise (const char* tweaksFileData, int tweaksFileDataSize);
#else
    /** Initialises the file. If the file has already been initialised, this will do nothing. */
    void initialise (const juce::File& file, int timerSeconds);
#endif

    /** Adds a batch of properties to the file. */
    void addProperties (std::initializer_list<Property> properties);

#if CHOWDSP_BAKE_TWEAKS
    /** Returns the value of a property. */
    template <typename T>
    T getProperty (PropertyID id, T&& defaultValue = {}) const;
#else
    /** Returns the value of a property. */
    template <typename T>
    T getProperty (PropertyID id, T&& defaultValue = {});
#endif

    // @TODO: add interface for listening to changes? (should be excluded from release builds)

private:
#if ! CHOWDSP_BAKE_TWEAKS
    bool reloadFromFile();
    void writeToFile();

    struct TweaksFileListener : public FileListener
    {
        TweaksFileListener (const juce::File& file, int timerSeconds, TweaksFile& configFile);
        void listenerFileChanged() override;

        TweaksFile& configFile;
    };

    std::unique_ptr<TweaksFileListener> fileListener;
#else
    bool isInitialized = false;
#endif

    json configProperties {};

    juce::CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TweaksFile)
};
} // namespace chowdsp::experimental
