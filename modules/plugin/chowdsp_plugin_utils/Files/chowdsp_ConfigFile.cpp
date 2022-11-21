#include "chowdsp_ConfigFile.h"

namespace chowdsp::experimental
{
ConfigFile::ConfigFileListener::ConfigFileListener (const juce::File& file, int timerSeconds, ConfigFile& cFile)
    : FileListener (file, timerSeconds),
      configFile (cFile)
{
}

void ConfigFile::ConfigFileListener::listenerFileChanged()
{
    configFile.reloadFromFile();
}

//=================================================
void ConfigFile::initialise (const juce::File& file, int timerSeconds)
{
    if (fileListener != nullptr)
        return; // already initialised

    const juce::ScopedLock sl { lock };
    fileListener = std::make_unique<ConfigFileListener> (file, timerSeconds, *this);
    reloadFromFile();
}

bool ConfigFile::reloadFromFile()
{
    const auto configFile = fileListener->getListenerFile();
    if (! configFile.existsAsFile())
        return false;

    json newProperties;
    try
    {
        newProperties = JSONUtils::fromFile (configFile);
    }
    catch (...)
    {
        juce::Logger::writeToLog ("Unable to load config from file: " + configFile.getFullPathName());
        return false;
    }

    const juce::ScopedLock sl { lock };
    configProperties = newProperties;
    return true;
}

void ConfigFile::writeToFile()
{
    const auto configFile = fileListener->getListenerFile();
    if (! configFile.existsAsFile())
    {
        configFile.deleteRecursively();
        configFile.create();
    }

    const juce::ScopedLock sl { lock };
    JSONUtils::toFile (configProperties, configFile, 4);
}

void ConfigFile::addProperties (std::initializer_list<Property> properties)
{
    jassert (fileListener != nullptr); // Trying to add properties before initializing? Don't do that!

    const juce::ScopedLock sl { lock };
    for (const auto& [name, value] : properties)
    {
        if (! configProperties.contains (name))
            configProperties[name] = value;
    }
    writeToFile();
}

template <typename T>
T ConfigFile::getProperty (PropertyID id, T&& defaultValue) const
{
    const juce::ScopedLock sl { lock };
    jassert (configProperties.contains (id));
    return configProperties.value (id, std::forward<T> (defaultValue));
}

#ifndef DOXYGEN
template bool ConfigFile::getProperty<bool> (PropertyID, bool&&) const;
template int ConfigFile::getProperty<int> (PropertyID, int&&) const;
template double ConfigFile::getProperty<double> (PropertyID, double&&) const;
template juce::String ConfigFile::getProperty<juce::String> (PropertyID, juce::String&&) const;
template json ConfigFile::getProperty<json> (PropertyID, json&&) const;
#endif
} // namespace chowdsp::experimental
