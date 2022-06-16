#pragma once

namespace chowdsp
{
/** Foley's GUI wrapper for InfoComp */
template <typename ProcType = juce::AudioProcessor, typename InfoProvider = StandardInfoProvider>
class InfoItem : public foleys::GuiItem
{
    using ItemType = InfoItem<ProcType, InfoProvider>;
    using ComponentType = InfoComp<ProcType, InfoProvider>;

public:
    FOLEYS_DECLARE_GUI_FACTORY (ItemType)

    InfoItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
    {
        setColourTranslation ({
            { "text1", ComponentType::text1ColourID },
            { "text2", ComponentType::text2ColourID },
        });

        auto* proc = dynamic_cast<ProcType*> (builder.getMagicState().getProcessor());
        jassert (proc != nullptr); // wrong type of processor!
        infoComp = std::make_unique<ComponentType> (*proc);
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
    std::unique_ptr<ComponentType> infoComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoItem)
};
} // namespace chowdsp
