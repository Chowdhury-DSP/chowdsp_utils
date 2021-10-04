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

    void presetListUpdated() override;
    void presetDirtyStatusChanged() override { updatePresetBoxText(); }
    void selectedPresetChanged() override { updatePresetBoxText(); }

    // @TODO methods to set images for next/prev buttons

protected:
    virtual void chooseUserPresetFolder (std::function<void()> onFinish = {});
    virtual int addPresetOptions (int optionID);
    virtual void saveUserPreset();
    virtual void updatePresetBoxText();

    PresetManager& manager;

    juce::ComboBox presetBox;
    juce::TextEditor presetNameEditor;

private:
    void savePresetFile (const juce::String& fileName);

    juce::DrawableButton presetsLeft, presetsRight;

    juce::WaitableEvent waiter;
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
    }

    juce::Component* getWrappedComponent() override
    {
        return presetsComp.get();
    }

private:
    std::unique_ptr<PresetsComp> presetsComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsItem)
};
#endif

} // namespace chowdsp
