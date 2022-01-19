#pragma once

#if JUCE_MODULE_AVAILABLE_chowdsp_dsp

namespace chowdsp
{

template <typename OSType>
class OversamplingMenu : public juce::Component
{
public:
    enum ColourIDs
    {
        backgroundColourID = 0x2340002, /**< Colour used for the background of the oversampling menu */
        textColourID, /**< Colour used for the oversampling menu text */
        outlineColourID, /**< Colour used for the oversampling menu outline */
        accentColourID, /**< Colour to use for menu accents */
    };

    OversamplingMenu (OSType& osManager, juce::AudioProcessorValueTreeState& vts);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void generateComboBoxMenu();

    juce::ComboBox comboBox;
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

#if CHOWDSP_USE_FOLEYS_CLASSES
// LCOV_EXCL_START
/** Foley's GUI wrapper for PresetsComp */
template <typename ProcType>
class OversamplingMenuItem : public foleys::GuiItem
{
    using MenuCompType = OversamplingMenu<typename std::remove_reference<decltype (std::declval<ProcType>().getOversampling())>::type>;

public:
    FOLEYS_DECLARE_GUI_FACTORY (OversamplingMenuItem)

    OversamplingMenuItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
    {
        setColourTranslation ({
            { "combo-background", OversamplingMenu<MenuCompType>::backgroundColourID },
            { "combo-text", OversamplingMenu<MenuCompType>::textColourID },
            { "combo-outline", OversamplingMenu<MenuCompType>::outlineColourID },
            { "menu-accent", OversamplingMenu<MenuCompType>::accentColourID },
        });

        auto* proc = dynamic_cast<ProcType*> (builder.getMagicState().getProcessor());
        jassert (proc != nullptr);

        osMenuComp = std::make_unique<MenuCompType> (proc->getOversampling(), proc->getVTS());
        addAndMakeVisible (osMenuComp.get());
    }

    void update() override
    {
        osMenuComp->repaint();
    }

    juce::Component* getWrappedComponent() override
    {
        return osMenuComp.get();
    }

private:
    std::unique_ptr<MenuCompType> osMenuComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OversamplingMenuItem)
};
// LCOV_EXCL_STOP
#endif // CHOWDSP_USE_FOLEYS_CLASSES
} // namespace chowdsp

#endif // JUCE_MODULE_AVAILABLE_chowdsp_dsp
