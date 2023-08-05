#pragma once

namespace chowdsp
{
/** Foley's GUI wrapper for PresetsComp */
template <typename ProcType, typename PresetCompType = PresetsComp>
class PresetsItem : public foleys::GuiItem
{
public:
    static_assert (std::is_base_of_v<PresetsComp, PresetCompType>, "PresetCompType must be derived from chowdsp::PresetsComp");

    FOLEYS_DECLARE_GUI_FACTORY (PresetsItem)

    static const juce::Identifier pNextButton;
    static const juce::Identifier pPrevButton;

    PresetsItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
    {
        setColourTranslation ({
            { "background", PresetCompType::backgroundColourID },
            { "text", PresetCompType::textColourID },
            { "text-highlight", PresetCompType::textHighlightColourID },
        });

        auto* proc = dynamic_cast<ProcType*> (builder.getMagicState().getProcessor());
        jassert (proc != nullptr);

        presetsComp = std::make_unique<PresetCompType> (proc->getPresetManager());
        addAndMakeVisible (presetsComp.get());
    }

    void update() override
    {
#if FOLEYS_ENABLE_BINARY_DATA
        auto getDrawable = [] (const juce::String& name)
        {
            int dataSize = 0;
            const char* data = BinaryData::getNamedResource (name.toRawUTF8(), dataSize);
            return juce::Drawable::createFromImageData (data, (size_t) dataSize);
        };

        auto nextButtonName = configNode.getProperty (pNextButton, juce::String()).toString();
        if (nextButtonName.isNotEmpty())
            presetsComp->setNextPrevButton (getDrawable (nextButtonName).get(), true);
        else
            presetsComp->setNextPrevButton (nullptr, true);

        auto prevButtonName = configNode.getProperty (pPrevButton, juce::String()).toString();
        if (prevButtonName.isNotEmpty())
            presetsComp->setNextPrevButton (getDrawable (prevButtonName).get(), false);
        else
            presetsComp->setNextPrevButton (nullptr, false);
#endif // FOLEYS_ENABLE_BINARY_DATA
    }

    [[nodiscard]] std::vector<foleys::SettableProperty> getSettableProperties() const override
    {
        std::function<void (juce::ComboBox&)> createAssetFilesMenuLambda = [this] (juce::ComboBox&)
        {
            magicBuilder.getMagicState().createAssetFilesMenu();
        };

        std::vector<foleys::SettableProperty> props {
            { configNode, pNextButton, foleys::SettableProperty::Choice, {}, createAssetFilesMenuLambda },
            { configNode, pPrevButton, foleys::SettableProperty::Choice, {}, createAssetFilesMenuLambda },
        };

        return props;
    }

    juce::Component* getWrappedComponent() override
    {
        return presetsComp.get();
    }

private:
    std::unique_ptr<PresetCompType> presetsComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsItem)
};

template <typename ProcType, typename PresetCompType>
const juce::Identifier PresetsItem<ProcType, PresetCompType>::pNextButton { "next-button" };

template <typename ProcType, typename PresetCompType>
const juce::Identifier PresetsItem<ProcType, PresetCompType>::pPrevButton { "prev-button" };
} // namespace chowdsp
