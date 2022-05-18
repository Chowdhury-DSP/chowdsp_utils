#pragma once

namespace chowdsp
{
/**
 * Utility class to hold plugin settings that should be shared between
 * plugin instances. It should typically be used as a SharedResourcePointer.
 */
class GlobalPluginSettings
{
public:
    /** Default constructor */
    GlobalPluginSettings() = default;

    /** Type alias for setting ID */
    using SettingID = std::string_view;

    /** Type alias for setting property with ID */
    using SettingProperty = std::pair<SettingID, json>;

    /** Initialise this settings object for a given file, and update time */
    void initialise (const juce::String& settingsFile, int timerSeconds = 5);

    /** Derive from this class to listen for changes to settings properties */
    struct Listener
    {
        virtual ~Listener() = default;

        /** This method will be called when a property has changed */
        virtual void globalSettingChanged (SettingID) = 0;
    };

    /** Adds a set of properties to the plugin settings, and adds a listener for those properties */
    void addProperties (std::initializer_list<SettingProperty> properties, Listener* listener = nullptr);

    /** Returns the settings property with a give name */
    template <typename T>
    T getProperty (SettingID name);

    /** Returns true if the given setting already exists */
    [[nodiscard]] bool hasProperty (SettingID name) const noexcept { return globalProperties.contains (name); }

    /**
     * If a property with this name has been added to the plugin settings,
     * it will be set to the new value. Note that the type of the property
     * must be the same as the type that was originally set for that property.
     */
    template <typename T>
    void setProperty (SettingID name, T property);

    /** Adds a listener for a given property */
    void addPropertyListener (SettingID id, Listener* listener);

    /** Removes a listener for a given property */
    void removePropertyListener (SettingID id, Listener* listener);

    /** Removes a listener from all its properties */
    void removePropertyListener (Listener* listener);

    /** Returns the file be used to store the global settings */
    [[nodiscard]] juce::File getSettingsFile() const noexcept;

private:
    bool loadSettingsFromFile();
    void writeSettingsToFile();

    struct SettingsFileListener : public FileListener
    {
        SettingsFileListener (const juce::File& file, int timerSeconds, GlobalPluginSettings& settings);
        void listenerFileChanged() override;

        GlobalPluginSettings& globalSettings;
    };

    std::unique_ptr<SettingsFileListener> fileListener;
    json globalProperties;

    std::unordered_map<SettingID, juce::Array<Listener*>> listeners;

    static constexpr SettingID settingsTag = "plugin_settings";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalPluginSettings)
};

/** Helpful alias for creating a SharedResourcePointer<GlobalPluginSettings> */
using SharedPluginSettings = juce::SharedResourcePointer<GlobalPluginSettings>;
} // namespace chowdsp
