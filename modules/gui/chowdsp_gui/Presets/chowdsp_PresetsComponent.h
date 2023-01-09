#pragma once

#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

namespace chowdsp
{
class PresetsComponent : public juce::Component
{
public:
    explicit PresetsComponent (PresetManager& presetManager, PresetsFrontend::FileInterface* fileInterface = nullptr);

    enum ColourIDs
    {
        backgroundColour = 0x45671002,
        bubbleColour,
        textColour,
    };

    void paint (juce::Graphics&) override;
    void resized() override;

    void setNextPrevButton (const juce::Drawable* image, bool isNext);

protected:
    void mouseDown (const juce::MouseEvent& e) override;
    void colourChanged() override;
    virtual void saveUserPreset (nlohmann::json&& presetState);

    PresetManager& presetManager;

    juce::DrawableButton prevPresetButton, nextPresetButton;
    juce::Label presetNameDisplay;
    juce::TextEditor presetNameEditor;

    OptionalPointer<PresetsFrontend::FileInterface> fileInterface {};
    std::unique_ptr<PresetsFrontend::MenuInterface> menuInterface;

private:
    PresetsFrontend::NextPrevious presetsNextPrevious { presetManager };
    PresetsFrontend::TextInterface textInterface { presetManager };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsComponent)
};
} // namespace chowdsp
