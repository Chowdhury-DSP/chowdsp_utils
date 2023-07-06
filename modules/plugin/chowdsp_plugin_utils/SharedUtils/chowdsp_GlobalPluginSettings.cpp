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

juce::File GlobalPluginSettings::getSettingsFile (const juce::String& settingsFilePath)
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile (settingsFilePath);
}

void GlobalPluginSettings::initialise (const juce::String& settingsFile, int timerSeconds)
{
    if (fileListener != nullptr)
        return; // already initialised!

    const juce::ScopedLock sl (lock);
    fileListener = std::make_unique<SettingsFileListener> (getSettingsFile (settingsFile), timerSeconds, *this);
    if (! loadSettingsFromFile())
        writeSettingsToFile();
}

void GlobalPluginSettings::addProperties (std::initializer_list<SettingProperty> properties)
{
    jassert (fileListener != nullptr); // Trying to add properties before initializing? Don't do that!
    const juce::ScopedLock sl (lock);

    for (const auto& [name, value] : properties)
    {
        if (! globalProperties.contains (name))
            globalProperties[name] = value; // we have to copy here because you can't "move" out of std::initializer_list
    }
    writeSettingsToFile();
}

template <typename T>
T GlobalPluginSettings::getProperty (SettingID name)
{
    const juce::ScopedLock sl (lock);
    try
    {
        return globalProperties[name].get<T>();
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
    const juce::ScopedLock sl (lock);
    if (! globalProperties.contains (name))
    {
        // property must be added before it can be set!
        jassertfalse;
        return;
    }

    if (! JSONUtils::isSameType (json (T {}), globalProperties[name]))
    {
        // new property must have the same type as the original!
        jassertfalse;
        return;
    }

    globalProperties[name] = property;
    writeSettingsToFile();

    globalSettingChangedBroadcaster (name);
}

juce::File GlobalPluginSettings::getSettingsFile() const noexcept
{
    const juce::ScopedLock sl (lock);
    if (fileListener == nullptr)
        return {};

    return fileListener->getListenerFile();
}

bool GlobalPluginSettings::loadSettingsFromFile()
{
    // this method should not be used before initialise()
    jassert (fileListener != nullptr);

    const juce::ScopedLock sl (lock);
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

    if (! settingsJson.contains (settingsTag))
    {
        // invalid settings!
        settingsFile.deleteRecursively();
        return false;
    }

    const auto oldProperties = globalProperties;
    globalProperties = settingsJson[settingsTag];

    for (const auto& [name, value] : oldProperties.items())
    {
        auto& newProperty = globalProperties[name];
        if (! JSONUtils::isSameType (value, newProperty))
        {
            // new property does not have the same type as the original!
            // also hit when listenerFileChanged callback is hit and our properties are different than the settings file
            jassertfalse;

            newProperty = value;
        }

        if (value != newProperty)
            globalSettingChangedBroadcaster (name);
    }
    return true;
}

void GlobalPluginSettings::writeSettingsToFile() const
{
    if (fileListener == nullptr)
        return;

    const juce::ScopedLock sl (lock);
    auto& settingsFile = fileListener->getListenerFile();

    json settingsJson;
    settingsJson[settingsTag] = globalProperties;

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
