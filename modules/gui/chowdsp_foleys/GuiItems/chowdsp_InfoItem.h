#pragma once

namespace chowdsp
{
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
} // namespace chowdsp
