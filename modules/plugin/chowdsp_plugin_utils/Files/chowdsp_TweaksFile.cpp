#include "chowdsp_TweaksFile.h"

namespace chowdsp::experimental
{
TweaksFile::TweaksFileListener::TweaksFileListener (const juce::File& file, int timerSeconds, TweaksFile& cFile)
    : FileListener (file, timerSeconds),
      configFile (cFile)
{
}

void TweaksFile::TweaksFileListener::listenerFileChanged()
{
    configFile.reloadFromFile();
}

//=================================================
void TweaksFile::initialise (const juce::File& file, int timerSeconds)
{
    if (fileListener != nullptr)
        return; // already initialised

    const juce::ScopedLock sl { lock };
    fileListener = std::make_unique<TweaksFileListener> (file, timerSeconds, *this);
    reloadFromFile();
}

bool TweaksFile::reloadFromFile()
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

void TweaksFile::writeToFile()
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

void TweaksFile::addProperties (std::initializer_list<Property> properties)
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
T TweaksFile::getProperty (PropertyID id, T&& defaultValue)
{
    const juce::ScopedLock sl { lock };
    if (! configProperties.contains (id))
    {
        addProperties ({ { id, defaultValue } });
        return defaultValue;
    }

    return configProperties.value (id, std::forward<T> (defaultValue));
}

#ifndef DOXYGEN
template bool TweaksFile::getProperty<bool> (PropertyID, bool&&);
template int TweaksFile::getProperty<int> (PropertyID, int&&);
template float TweaksFile::getProperty<float> (PropertyID, float&&);
template double TweaksFile::getProperty<double> (PropertyID, double&&);
template juce::String TweaksFile::getProperty<juce::String> (PropertyID, juce::String&&);
template json TweaksFile::getProperty<json> (PropertyID, json&&);
#endif
} // namespace chowdsp::experimental
