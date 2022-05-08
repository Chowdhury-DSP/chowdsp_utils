#pragma once

namespace chowdsp
{
/** Menu that can be used as an interface for chowdsp::VariableOversampling */
template <typename OSType>
class OversamplingMenu : public juce::ComboBox,
                         private OSType::Listener
{
public:
    enum ColourIDs
    {
        backgroundColourID = 0x2340002, /**< Colour used for the background of the oversampling menu */
        textColourID, /**< Colour used for the oversampling menu text */
        outlineColourID, /**< Colour used for the oversampling menu outline */
        accentColourID, /**< Colour to use for menu accents */
    };

    /** Constructor with fields for the oversampling object, and APVTS */
    OversamplingMenu (OSType& osManager, juce::AudioProcessorValueTreeState& vts);
    ~OversamplingMenu() override;

    void updateColours();

protected:
    virtual void generateComboBoxMenu();

private:
    void sampleRateOrBlockSizeChanged() override { generateComboBoxMenu(); }

    juce::Colour accentColour;

    std::unique_ptr<juce::ParameterAttachment> attachments[5];
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OversamplingMenu)
};
} // namespace chowdsp
