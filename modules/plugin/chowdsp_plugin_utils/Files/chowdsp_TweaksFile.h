#pragma once

#include "../../../common/chowdsp_json/chowdsp_json.h"
#include "chowdsp_FileListener.h"

namespace chowdsp
{
/**
 * A file to hold some configuration that can be tweaked
 * without needing to re-compile the plugin.
 */
template <bool isBaked>
class GenericTweaksFile;

/** Specialization of GenericTweaksFile that is not baked into the plugin at compile-time. */
template <>
class GenericTweaksFile<false>
{
public:
    GenericTweaksFile() = default;

    /** Type alias for setting ID */
    using PropertyID = std::string_view;

    /** Type alias for setting property with ID */
    using Property = std::pair<PropertyID, json>;

    /** Initialises the file. If the file has already been initialised, this will do nothing. */
    void initialise (const juce::File& file, int timerSeconds);

    /** Adds a batch of properties to the file. */
    void addProperties (std::initializer_list<Property> properties);

    /** Returns the value of a property. */
    template <typename T>
    T getProperty (PropertyID id, T&& defaultValue = {});

    /** Adds a property listener to the tweaks file */
    template <typename... ListenerArgs>
    [[nodiscard]] ScopedCallback addListener (ListenerArgs&&... args)
    {
        return changeBroadcaster.connect (std::forward<ListenerArgs...> (args...));
    }

private:
    bool reloadFromFile();
    void writeToFile() const;

    struct TweaksFileListener : public FileListener
    {
        TweaksFileListener (const juce::File& file, int timerSeconds, GenericTweaksFile& tweaksFile);
        void listenerFileChanged() override;

        GenericTweaksFile& tweaksFile;
    };

    std::unique_ptr<TweaksFileListener> fileListener;

    json configProperties {};

    Broadcaster<void (PropertyID)> changeBroadcaster;

    juce::CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericTweaksFile)
};

/** Specialization of GenericTweaksFile that is baked into the plugin at compile-time. */
template <>
class GenericTweaksFile<true>
{
public:
    GenericTweaksFile() = default;

    /** Type alias for setting ID */
    using PropertyID = std::string_view;

    /** Type alias for setting property with ID */
    using Property = std::pair<PropertyID, json>;

    /** Initialises the file from a BinaryData object. If the file has already been initialised, this will do nothing. */
    void initialise (const char* tweaksFileData, int tweaksFileDataSize);

    /**
     * Adds a batch of properties to the file.
     * For the "baked" tweaks file, this function is a no-op.
     */
    static void addProperties (std::initializer_list<Property> /*properties*/) {}

    /** Returns the value of a property. */
    template <typename T>
    T getProperty (PropertyID id, T&& defaultValue = {}) const;

private:
    bool isInitialized = false;

    json configProperties {};

    juce::CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericTweaksFile)
};

#if CHOWDSP_BAKE_TWEAKS
using TweaksFile = GenericTweaksFile<true>;
#else
using TweaksFile = GenericTweaksFile<false>;
#endif
} // namespace chowdsp
