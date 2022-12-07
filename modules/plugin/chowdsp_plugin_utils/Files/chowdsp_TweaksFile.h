#pragma once

#include "../../../common/chowdsp_json/chowdsp_json.h"
#include "chowdsp_FileListener.h"

namespace chowdsp::experimental
{
/**
 * A file to hold some configuration that can be tweaked
 * without needing to re-compile the plugin.
 *
 * @TODO: We should figure out an easy way to "bake" a config file
 * with binary data for release builds.
 */
class TweaksFile
{
public:
    TweaksFile() = default;

    /** Type alias for setting ID */
    using PropertyID = std::string_view;

    /** Type alias for setting property with ID */
    using Property = std::pair<PropertyID, json>;

    /** Initialises the file. If the file has already been initialised, this will do nothing. */
    void initialise (const juce::File& file, int timerSeconds);

    /** Adds a batch of properties to the file. */
    void addProperties (std::initializer_list<Property> properties);

    /** Returns the value of a property. */
    template <typename T>
    T getProperty (PropertyID id, T&& defaultValue = {});

    // @TODO: add interface for listening to changes? (should be excluded from release builds)

private:
    bool reloadFromFile();
    void writeToFile();

    struct TweaksFileListener : public FileListener
    {
        TweaksFileListener (const juce::File& file, int timerSeconds, TweaksFile& configFile);
        void listenerFileChanged() override;

        TweaksFile& configFile;
    };

    std::unique_ptr<TweaksFileListener> fileListener;
    json configProperties {};

    juce::CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TweaksFile)
};
} // namespace chowdsp::experimental
