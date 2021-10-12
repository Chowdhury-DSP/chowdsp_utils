#pragma once

namespace chowdsp
{
/** A component to display a presets menu */
class PresetsComp : public juce::Component,
                    private PresetManager::Listener
{
public:
    enum ColourIDs
    {
        backgroundColourID, /**< Colour used for the background of the preset menu */
        textColourID, /**< Colour used for the preset menu text */
        textHighlightColourID, /**< Colour used for the preset menu text highlighting */
    };

    /** Creates an Presets component for the given preset manager */
    PresetsComp (PresetManager& presetManager);
    ~PresetsComp() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    juce::String getPresetMenuText() const noexcept { return presetBox.getText(); }
    juce::ComboBox& getPresetMenuBox() { return presetBox; }
    juce::TextEditor& getPresetNameEditor() { return presetNameEditor; }

    void presetListUpdated() override;
    void presetDirtyStatusChanged() override { updatePresetBoxText(); }
    void selectedPresetChanged() override { updatePresetBoxText(); }

    void goToNextPreset (bool forward);
    void setNextPrevButton (const juce::Drawable* image, bool isNext);

protected:
    virtual void chooseUserPresetFolder (std::function<void()> onFinish = {});
    virtual int addPresetOptions (int optionID);
    virtual void saveUserPreset();
    virtual void updatePresetBoxText();

    PresetManager& manager;

    juce::ComboBox presetBox;
    juce::TextEditor presetNameEditor;
    juce::DrawableButton prevPresetButton, nextPresetButton;

private:
    void savePresetFile (const juce::String& fileName);

    std::shared_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsComp)
};

#if CHOWDSP_USE_FOLEYS_CLASSES
/** Foley's GUI wrapper for PresetsComp */
template <typename ProcType>
class PresetsItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (PresetsItem)

    static const juce::Identifier pNextButton;
    static const juce::Identifier pPrevButton;

    PresetsItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
    {
        setColourTranslation ({
            { "background", PresetsComp::backgroundColourID },
            { "text", PresetsComp::textColourID },
            { "text-highlight", PresetsComp::textHighlightColourID },
        });

        auto* proc = dynamic_cast<ProcType*> (builder.getMagicState().getProcessor());
        jassert (proc != nullptr);

        presetsComp = std::make_unique<PresetsComp> (proc->getPresetManager());
        addAndMakeVisible (presetsComp.get());
    }

    void update() override
    {
#if FOLEYS_ENABLE_BINARY_DATA
        auto getDrawable = [] (const juce::String& name) {
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
#endif
    }

    std::vector<foleys::SettableProperty> getSettableProperties() const override
    {
        std::function<void (juce::ComboBox&)> createAssetFilesMenuLambda = [=] (juce::ComboBox&) {
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
    std::unique_ptr<PresetsComp> presetsComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsItem)
};

template <typename ProcType>
const juce::Identifier PresetsItem<ProcType>::pNextButton { "next-button" };

template <typename ProcType>
const juce::Identifier PresetsItem<ProcType>::pPrevButton { "prev-button" };

#endif

} // namespace chowdsp
