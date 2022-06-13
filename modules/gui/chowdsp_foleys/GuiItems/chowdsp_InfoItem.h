#pragma once

namespace chowdsp
{
/** Foley's GUI wrapper for InfoComp */
template <typename ProcType = juce::AudioProcessor, typename InfoProvider = StandardInfoProvider>
class InfoItem : public foleys::GuiItem
{
    using ItemType = InfoItem<ProcType, InfoProvider>;

public:
    FOLEYS_DECLARE_GUI_FACTORY (ItemType)

    InfoItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
    {
        setColourTranslation ({
            { "text1", ItemType::text1ColourID },
            { "text2", ItemType::text2ColourID },
        });

        auto* proc = dynamic_cast<ProcType*> (builder.getMagicState().getProcessor());
        jassert (proc != nullptr); // wrong type of processor!
        infoComp = std::make_unique<ItemType> (*proc);
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
    std::unique_ptr<ItemType> infoComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoItem)
};
} // namespace chowdsp
