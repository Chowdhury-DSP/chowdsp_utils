#pragma once

namespace chowdsp
{
/** Foley's GUI wrapper for PresetsComp */
template <typename ProcType, typename MenuCompType = OversamplingMenu<typename std::remove_reference<decltype (std::declval<ProcType>().getOversampling())>::type>>
class OversamplingMenuItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (OversamplingMenuItem)

    OversamplingMenuItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
    {
        setColourTranslation ({
            { "combo-background", MenuCompType::backgroundColourID },
            { "combo-text", MenuCompType::textColourID },
            { "combo-outline", MenuCompType::outlineColourID },
            { "menu-accent", MenuCompType::accentColourID },
        });

        auto* proc = dynamic_cast<ProcType*> (builder.getMagicState().getProcessor());
        jassert (proc != nullptr);

        osMenuComp = std::make_unique<MenuCompType> (proc->getOversampling(), proc->getVTS());
        addAndMakeVisible (osMenuComp.get());
    }

    void update() override
    {
        osMenuComp->updateColours();
    }

    juce::Component* getWrappedComponent() override
    {
        return osMenuComp.get();
    }

private:
    std::unique_ptr<MenuCompType> osMenuComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OversamplingMenuItem)
};
} // namespace chowdsp
