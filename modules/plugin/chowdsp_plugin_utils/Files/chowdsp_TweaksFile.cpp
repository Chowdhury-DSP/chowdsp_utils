#include "chowdsp_TweaksFile.h"

namespace chowdsp
{
GenericTweaksFile<false>::TweaksFileListener::TweaksFileListener (const juce::File& file, int timerSeconds, GenericTweaksFile& tFile)
    : FileListener (file, timerSeconds),
      tweaksFile (tFile)
{
}

void GenericTweaksFile<false>::TweaksFileListener::listenerFileChanged()
{
    tweaksFile.reloadFromFile();
}

//=================================================
void GenericTweaksFile<false>::initialise (const juce::File& file, int timerSeconds)
{
    if (fileListener != nullptr)
        return; // already initialised

    const juce::ScopedLock sl { lock };
    fileListener = std::make_unique<TweaksFileListener> (file, timerSeconds, *this);
    reloadFromFile();
}

bool GenericTweaksFile<false>::reloadFromFile()
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
    const auto oldProperties = configProperties;
    configProperties = newProperties;

    for (const auto& [name, value] : oldProperties.items())
    {
        if (! configProperties.contains (name))
            continue;

        auto& newProperty = configProperties[name];
        if (! JSONUtils::isSameType (value, newProperty))
        {
            // new property does not have the same type as the original!
            // also hit when listenerFileChanged callback is hit and our properties are different than the settings file
            jassertfalse;

            newProperty = value;
        }

        if (value != newProperty)
            changeBroadcaster (name);
    }
    return true;
}

void GenericTweaksFile<false>::writeToFile() const
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

void GenericTweaksFile<false>::addProperties (std::initializer_list<Property> properties)
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
T GenericTweaksFile<false>::getProperty (PropertyID id, T&& defaultValue)
{
    const juce::ScopedLock sl { lock };
    if (! configProperties.contains (id))
    {
        addProperties ({ { id, defaultValue } });
        return std::forward<T> (defaultValue);
    }

    return configProperties.value (id, std::forward<T> (defaultValue));
}

//=====================================================
void GenericTweaksFile<true>::initialise (const char* tweaksFileData, int tweaksFileDataSize)
{
    if (isInitialized)
        return; // already initialised

    const juce::ScopedLock sl { lock };
    isInitialized = true;
    configProperties = JSONUtils::fromBinaryData (tweaksFileData, tweaksFileDataSize);
}

template <typename T>
T GenericTweaksFile<true>::getProperty (PropertyID id, T&& defaultValue) const
{
    const juce::ScopedLock sl { lock };
    jassert (configProperties.contains (id));
    return configProperties.value (id, std::forward<T> (defaultValue));
}

#ifndef DOXYGEN
template bool GenericTweaksFile<false>::getProperty<bool> (PropertyID, bool&&);
template int GenericTweaksFile<false>::getProperty<int> (PropertyID, int&&);
template float GenericTweaksFile<false>::getProperty<float> (PropertyID, float&&);
template double GenericTweaksFile<false>::getProperty<double> (PropertyID, double&&);
template juce::String GenericTweaksFile<false>::getProperty<juce::String> (PropertyID, juce::String&&);
template json GenericTweaksFile<false>::getProperty<json> (PropertyID, json&&);
template bool GenericTweaksFile<true>::getProperty<bool> (PropertyID, bool&&) const;
template int GenericTweaksFile<true>::getProperty<int> (PropertyID, int&&) const;
template float GenericTweaksFile<true>::getProperty<float> (PropertyID, float&&) const;
template double GenericTweaksFile<true>::getProperty<double> (PropertyID, double&&) const;
template juce::String GenericTweaksFile<true>::getProperty<juce::String> (PropertyID, juce::String&&) const;
template json GenericTweaksFile<true>::getProperty<json> (PropertyID, json&&) const;
#endif
} // namespace chowdsp
