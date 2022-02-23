#pragma once

namespace chowdsp
{
/** A simple component to display the type, version, and manufacturer of a plugin */
template <typename InfoProvider = StandardInfoProvider, typename ProcType = juce::AudioProcessor>
class InfoComp : public juce::Component
{
public:
    /** Creates an Info component for the given plugin wrapper type */
    explicit InfoComp (const ProcType& processor);

    enum ColourIDs
    {
        text1ColourID, /**< Colour used for plugin type text */
        text2ColourID, /**< Colour used for web link */
    };

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    const ProcType& proc;
    juce::HyperlinkButton linkButton;

    int linkX = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoComp)
};

#if CHOWDSP_USE_FOLEYS_CLASSES
// LCOV_EXCL_START
/** Foley's GUI wrapper for InfoComp */
template <typename InfoProvider = StandardInfoProvider, typename ProcType = juce::AudioProcessor>
class InfoItem : public foleys::GuiItem
{
    using ItemType = InfoItem<InfoProvider, ProcType>;

public:
    FOLEYS_DECLARE_GUI_FACTORY (ItemType)

    InfoItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
    {
        setColourTranslation ({
            { "text1", InfoComp<InfoProvider, ProcType>::text1ColourID },
            { "text2", InfoComp<InfoProvider, ProcType>::text2ColourID },
        });

        auto* proc = dynamic_cast<ProcType*> (builder.getMagicState().getProcessor());
        jassert (proc != nullptr); // wrong type of processor!
        infoComp = std::make_unique<InfoComp<InfoProvider, ProcType>> (*proc);
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
    std::unique_ptr<InfoComp<InfoProvider, ProcType>> infoComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoItem)
};
// LCOV_EXCL_STOP
#endif // CHOWDSP_USE_FOLEYS_CLASSES

} // namespace chowdsp

#include "chowdsp_InfoComp.cpp"
