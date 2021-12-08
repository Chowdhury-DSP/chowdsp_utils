#include "chowdsp_GlobalPluginSettings.h"

namespace chowdsp
{
GlobalPluginSettings::SettingsFileListener::SettingsFileListener (const juce::File& file, int timerSeconds, GlobalPluginSettings& settings)
    : FileListener (file, timerSeconds),
      globalSettings (settings)
{
}

void GlobalPluginSettings::SettingsFileListener::listenerFileChanged()
{
    globalSettings.loadSettingsFromFile();
}

void GlobalPluginSettings::initialise (const juce::String& settingsFile, int timerSeconds)
{
    if (fileListener != nullptr)
        return; // already initialised!

    auto settingsDir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
    fileListener = std::make_unique<SettingsFileListener> (settingsDir.getChildFile (settingsFile), timerSeconds, *this);
    if (! loadSettingsFromFile())
        writeSettingsToFile();
}

void GlobalPluginSettings::addProperties (std::initializer_list<juce::NamedValueSet::NamedValue> properties, Listener* listener)
{
    jassert (fileListener != nullptr); // Trying to add properties before initalizing? Don't do that!

    juce::Array<juce::NamedValueSet::NamedValue> propertiesToAdd (std::move (properties));
    for (auto& prop : propertiesToAdd)
    {
        if (! globalProperties.contains (prop.name))
            globalProperties.set (prop.name, std::move (prop.value));
        addPropertyListener (prop.name, listener);
    }

    writeSettingsToFile();
}

void GlobalPluginSettings::setProperty (const juce::Identifier& name, juce::var&& property)
{
    if (! globalProperties.contains (name))
        return;

    globalProperties.set (name, std::move (property));
    writeSettingsToFile();

    for (auto* l : listeners[name.toString()])
        l->propertyChanged (name, globalProperties[name]);
}

void GlobalPluginSettings::addPropertyListener (const juce::Identifier& id, Listener* listener)
{
    if (listener == nullptr)
        return;

    listeners[id.toString()].addIfNotAlreadyThere (listener);
}

void GlobalPluginSettings::removePropertyListener (const juce::Identifier& id, Listener* listener)
{
    if (listeners.find (id.toString()) == listeners.end())
    {
        jassertfalse; // this property does not have any listeners!
        return;
    }

    listeners.at (id.toString()).removeAllInstancesOf (listener);
}

void GlobalPluginSettings::removePropertyListener (Listener* listener)
{
    for (auto& [_, propListeners] : listeners)
        propListeners.removeAllInstancesOf (listener);
}

juce::File GlobalPluginSettings::getSettingsFile() const noexcept
{
    if (fileListener == nullptr)
        return juce::File();

    return fileListener->getListenerFile();
}

bool GlobalPluginSettings::loadSettingsFromFile()
{
    // this method should not be used before initialise()
    jassert (fileListener != nullptr);

    auto& settingsFile = fileListener->getListenerFile();
    if (! settingsFile.existsAsFile())
        return false;

    auto settingsXml = juce::XmlDocument::parse (settingsFile);
    if (settingsXml == nullptr)
    {
        // invalid settings!
        settingsFile.deleteRecursively();
        return false;
    }

    if (settingsXml->getTagName() != settingsTag.toString())
    {
        // invalid settings!
        settingsFile.deleteRecursively();
        return false;
    }

    juce::NamedValueSet oldProperties = globalProperties;
    globalProperties.setFromXmlAttributes (*settingsXml);

    for (const auto& prop : oldProperties)
    {
        if (prop.value != globalProperties[prop.name])
        {
            for (auto* l : listeners[prop.name.toString()])
                l->propertyChanged (prop.name, globalProperties[prop.name]);
        }
    }

    return true;
}

void GlobalPluginSettings::writeSettingsToFile()
{
    if (fileListener == nullptr)
        return;

    auto& settingsFile = fileListener->getListenerFile();
    juce::XmlElement settingsXml { settingsTag };
    globalProperties.copyToXmlAttributes (settingsXml);
    settingsXml.writeTo (settingsFile);
}

const juce::Identifier GlobalPluginSettings::settingsTag { "plugin_settings" };
} // namespace chowdsp
