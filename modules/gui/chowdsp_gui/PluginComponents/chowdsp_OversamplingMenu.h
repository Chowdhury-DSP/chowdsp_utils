#pragma once

#include <chowdsp_listeners/chowdsp_listeners.h>

// LCOV_EXCL_START

namespace chowdsp
{
/** Menu that can be used as an interface for chowdsp::VariableOversampling */
template <typename OSType, typename ComboBoxType = juce::ComboBox>
class OversamplingMenu : public ComboBoxType
{
    static_assert (std::is_base_of_v<juce::ComboBox, ComboBoxType>, "ComboBoxType must be derived from juce::ComboBox");

public:
    enum ColourIDs
    {
        backgroundColourID = 0x2340002, /**< Colour used for the background of the oversampling menu */
        textColourID, /**< Colour used for the oversampling menu text */
        outlineColourID, /**< Colour used for the oversampling menu outline */
        accentColourID, /**< Colour to use for menu accents */
    };

    /** Constructor with fields for the oversampling object, and APVTS */
    template <typename... Args>
    OversamplingMenu (OSType& oversamplingManager, juce::AudioProcessorValueTreeState& vts, Args&&... args)
        : ComboBoxType (args...), osManager (oversamplingManager)
    {
        initialise (vts);
    }

    void updateColours();

protected:
    virtual void generateComboBoxMenu();

private:
    void initialise (juce::AudioProcessorValueTreeState& vts);

    juce::Colour accentColour;

    std::unique_ptr<juce::ParameterAttachment> attachments[5] {};
    juce::RangedAudioParameter* parameters[5] {};

    enum ParamTypes
    {
        OSParam = 0,
        OSMode,
        OSOfflineParam,
        OSOfflineMode,
        OSOfflineSame,
    };

    OSType& osManager;
    bool offlineParamsAvailable = false;

    ScopedCallback sampleRateOrBlockSizeChangedCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OversamplingMenu)
};
} // namespace chowdsp

// LCOV_EXCL_END
