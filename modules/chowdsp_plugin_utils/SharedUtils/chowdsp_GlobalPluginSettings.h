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

    using ListenerPair = std::pair<juce::Identifier, Listener*>;
    juce::Array<ListenerPair> listeners;

    const static juce::Identifier settingsTag;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalPluginSettings)
};

/** Helpful alias for creating a SharedResourcePointer<GlobalPluginSettings> */
using SharedPluginSettings = juce::SharedResourcePointer<GlobalPluginSettings>;
} // namespace chowdsp
