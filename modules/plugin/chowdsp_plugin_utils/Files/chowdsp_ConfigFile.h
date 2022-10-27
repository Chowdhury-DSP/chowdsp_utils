#pragma once

namespace chowdsp::experimental
{
/**
 * A file to hold some configuration that can be tweaked
 * without needing to re-compile the plugin.
 *
 * @TODO: We should figure out an easy way to "bake" a config file
 * with binary data for release builds.
 */
class ConfigFile
{
public:
    ConfigFile() = default;

    /** Type alias for setting ID */
    using PropertyID = std::string_view;

    /** Type alias for setting property with ID */
    using Property = std::pair<PropertyID, json>;

    void initialise (const juce::File& file, int timerSeconds);

    void addProperties (std::initializer_list<Property> properties);

    template <typename T>
    T getProperty (PropertyID id, T&& defaultValue = {}) const;

    // @TODO: add interface for listening to changes? (should be excluded from release builds)

private:
    bool reloadFromFile();
    void writeToFile();

    struct ConfigFileListener : public FileListener
    {
        ConfigFileListener (const juce::File& file, int timerSeconds, ConfigFile& configFile);
        void listenerFileChanged() override;

        ConfigFile& configFile;
    };

    std::unique_ptr<ConfigFileListener> fileListener;
    json configProperties {};

    juce::CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigFile)
};
} // namespace chowdsp::experimental
