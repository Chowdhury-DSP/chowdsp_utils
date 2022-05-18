#include "chowdsp_GlobalPluginSettings.h"

// @TODO: Update this class when nlohmann::json supports std::string_view
// this should be available in version 3.11 (https://github.com/nlohmann/json/issues/1529)

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

void GlobalPluginSettings::addProperties (std::initializer_list<SettingProperty> properties, Listener* listener)
{
    jassert (fileListener != nullptr); // Trying to add properties before initalizing? Don't do that!

    for (auto& [name, value] : properties)
    {
        if (! globalProperties.contains (name))
            globalProperties[name.data()] = std::move (value);
        addPropertyListener (name, listener);
    }

    writeSettingsToFile();
}

template <typename T>
T GlobalPluginSettings::getProperty (SettingID name)
{
    try
    {
        return globalProperties[name.data()].get<T>();
    }
    catch (...)
    {
        // property was of the wrong data type!
        jassertfalse;

        return {};
    }
}

template <typename T>
void GlobalPluginSettings::setProperty (SettingID name, T property)
{
    if (! globalProperties.contains (name))
    {
        // property must be added before it can be set!
        jassertfalse;
        return;
    }

    if (! JSONUtils::isSameType (json (T {}), globalProperties[name.data()]))
    {
        // new property must have the same type as the original!
        jassertfalse;
        return;
    }

    globalProperties[name.data()] = property;
    writeSettingsToFile();

    for (auto* l : listeners[name])
        l->globalSettingChanged (name);
}

void GlobalPluginSettings::addPropertyListener (SettingID id, Listener* listener)
{
    if (listener == nullptr)
        return;

    listeners[id].addIfNotAlreadyThere (listener);
}

void GlobalPluginSettings::removePropertyListener (SettingID id, Listener* listener)
{
    const auto listenersForIDIter = listeners.find (id);
    if (listenersForIDIter == listeners.end())
    {
        jassertfalse; // this property does not have any listeners!
        return;
    }

    listenersForIDIter->second.removeAllInstancesOf (listener);
}

void GlobalPluginSettings::removePropertyListener (Listener* listener)
{
    for (auto& [_, propListeners] : listeners)
        propListeners.removeAllInstancesOf (listener);
}

juce::File GlobalPluginSettings::getSettingsFile() const noexcept
{
    if (fileListener == nullptr)
        return {};

    return fileListener->getListenerFile();
}

bool GlobalPluginSettings::loadSettingsFromFile()
{
    // this method should not be used before initialise()
    jassert (fileListener != nullptr);

    auto& settingsFile = fileListener->getListenerFile();
    if (! settingsFile.existsAsFile())
        return false;

    json settingsJson;
    try
    {
        settingsJson = JSONUtils::fromFile (settingsFile);
    }
    catch (...)
    {
        // something went wrong when trying to read the properties file...
        jassertfalse;
        return false;
    }

    if (! settingsJson.contains (settingsTag.data()))
    {
        // invalid settings!
        settingsFile.deleteRecursively();
        return false;
    }

    const auto oldProperties = globalProperties;
    globalProperties = settingsJson[settingsTag.data()];

    for (const auto& [name, value] : oldProperties.items())
    {
        auto& newProperty = globalProperties[name.data()];
        if (! JSONUtils::isSameType (value, newProperty))
        {
            // new property does not have the same type as the original!
            jassertfalse;

            newProperty = value;
        }

        if (value != newProperty)
        {
            for (auto* l : listeners[name])
                l->globalSettingChanged (name);
        }
    }

    return true;
}

void GlobalPluginSettings::writeSettingsToFile()
{
    if (fileListener == nullptr)
        return;

    auto& settingsFile = fileListener->getListenerFile();

    json settingsJson;
    settingsJson[settingsTag.data()] = globalProperties;

    if (! settingsFile.existsAsFile())
    {
        settingsFile.deleteRecursively();
        settingsFile.create();
    }

    JSONUtils::toFile (settingsJson, settingsFile);
}

#ifndef DOXYGEN
template void GlobalPluginSettings::setProperty<bool> (SettingID name, bool property);
template void GlobalPluginSettings::setProperty<int> (SettingID name, int property);
template void GlobalPluginSettings::setProperty<double> (SettingID name, double property);
template void GlobalPluginSettings::setProperty<juce::String> (SettingID name, juce::String property);
template void GlobalPluginSettings::setProperty<json> (SettingID name, json property);

template bool GlobalPluginSettings::getProperty<bool> (SettingID name);
template int GlobalPluginSettings::getProperty<int> (SettingID name);
template double GlobalPluginSettings::getProperty<double> (SettingID name);
template juce::String GlobalPluginSettings::getProperty<juce::String> (SettingID name);
template json GlobalPluginSettings::getProperty<json> (SettingID name);
#endif // DOXYGEN
} // namespace chowdsp
