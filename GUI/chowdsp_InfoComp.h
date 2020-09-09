#pragma once

namespace chowdsp
{

class InfoComp : public juce::Component
{
public:
    InfoComp (const juce::AudioProcessor::WrapperType wrapperType);

    enum ColourIDs
    {
        text1ColourID,
        text2ColourID,
    };

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    const juce::AudioProcessor::WrapperType wrapperType;
    juce::HyperlinkButton linkButton { JucePlugin_Manufacturer, juce::URL ("https://ccrma.stanford.edu/~jatin/chowdsp") };
    
    int linkX = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InfoComp)
};

class InfoItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (InfoItem)

    InfoItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) :
        foleys::GuiItem (builder, node)
    {
        setColourTranslation ({
            {"text1", InfoComp::text1ColourID},
            {"text2", InfoComp::text2ColourID},
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InfoItem)
};

} // chowdsp
