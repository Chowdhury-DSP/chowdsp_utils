#pragma once

namespace chowdsp
{
/** A simple component to display the type, version, and manufacturer of a plugin */
class InfoComp : public juce::Component
{
public:
    /** Creates an Info component for the given plugin wrapper type */
    InfoComp (const juce::AudioProcessor::WrapperType wrapperType);

    enum ColourIDs
    {
        text1ColourID, /**< Colour used for plugin type text */
        text2ColourID, /**< Colour used for web link */
    };

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    const juce::AudioProcessor::WrapperType wrapperType;
    juce::HyperlinkButton linkButton;

    int linkX = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoComp)
};

#if CHOWDSP_USE_FOLEYS_CLASSES
/** Foley's GUI wrapper for InfoComp */
class InfoItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (InfoItem)

    InfoItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
    {
        setColourTranslation ({
            { "text1", InfoComp::text1ColourID },
            { "text2", InfoComp::text2ColourID },
        });

        infoComp = std::make_unique<InfoComp> (builder.getMagicState().getProcessor()->wrapperType);
        addAndMakeVisible (infoComp.get());
    }

    void update() override
    {
    }

    juce::Component* getWrappedComponent() override
    {
        return infoComp.get();
    }

private:
    std::unique_ptr<InfoComp> infoComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoItem)
};
#endif

} // namespace chowdsp
