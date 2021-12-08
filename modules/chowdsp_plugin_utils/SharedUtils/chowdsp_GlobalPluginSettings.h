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

    /** Initialise this settings object for a given file, and update time */
    void initialise (const juce::String& settingsFile, int timerSeconds = 5);

    /** Derive from this class to listen for changes to settings properties */
    struct Listener
    {
        virtual ~Listener() = default;

        /** This method will be called when a property has changed */
        virtual void propertyChanged (const juce::Identifier&, const juce::var&) = 0;
    };

    /** Adds a set of properties to the plugin settings, and adds a listener for those properties */
    void addProperties (std::initializer_list<juce::NamedValueSet::NamedValue> properties, Listener* listener = nullptr);

    /** Returns the settings property with a give name */
    const juce::var& getProperty (const juce::Identifier& name) const noexcept { return globalProperties[name]; }

    /** Returns true if the given setting already exists */
    bool hasProperty (const juce::Identifier& name) const noexcept { return globalProperties.contains (name); }

    /**
     * If a property with this name has been added to the plugin settings,
     * it will be set to the new value.
     */
    void setProperty (const juce::Identifier& name, juce::var&& property);

    /** Adds a listener for a given property */
    void addPropertyListener (const juce::Identifier& id, Listener* listener);

    /** Removes a listener for a given property */
    void removePropertyListener (const juce::Identifier& id, Listener* listener);

    /** Removes a listener from all its properties */
    void removePropertyListener (Listener* listener);

    /** Returns the file be used to store the global settings */
    juce::File getSettingsFile() const noexcept;

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
    juce::NamedValueSet globalProperties;

    // @TODO: figure out how to map over identifiers! Then this should be much faster.
    // Ongoing discussion here: https://forum.juce.com/t/std-hash-specialisation-for-identifier-unordered-map/25157/18
    std::unordered_map<juce::String, juce::Array<Listener*>> listeners;

    const static juce::Identifier settingsTag;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalPluginSettings)
};

/** Helpful alias for creating a SharedResourcePointer<GlobalPluginSettings> */
using SharedPluginSettings = juce::SharedResourcePointer<GlobalPluginSettings>;
} // namespace chowdsp
