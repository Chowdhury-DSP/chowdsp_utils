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
    juce::Array<juce::NamedValueSet::NamedValue> propertiesToAdd (std::move (properties));
    for (auto& prop : propertiesToAdd)
    {
        if (! globalProperties.contains (prop.name))
            globalProperties.set (prop.name, std::move (prop.value));
        listeners.add (std::make_pair (prop.name, listener));
    }

    writeSettingsToFile();
}

void GlobalPluginSettings::addPropertyListener (const juce::Identifier& id, Listener* listener)
{
    listeners.addIfNotAlreadyThere (std::make_pair (id, listener));
}

void GlobalPluginSettings::removePropertyListener (const juce::Identifier& id, Listener* listener)
{
    listeners.removeIf ([&id, &listener] (ListenerPair& pair) { return pair.first == id && pair.second == listener; });
}

void GlobalPluginSettings::removePropertyListener (Listener* listener)
{
    listeners.removeIf ([&listener] (ListenerPair& pair) { return pair.second == listener; });
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
            for (auto& [tag, l] : listeners)
            {
                if (tag == prop.name)
                    l->propertyChanged (tag, globalProperties[prop.name]);
            }
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
