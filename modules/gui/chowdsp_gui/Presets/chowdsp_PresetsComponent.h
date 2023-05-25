#pragma once

#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

namespace chowdsp::presets
{
class PresetsComponent : public juce::Component
{
public:
    explicit PresetsComponent (PresetManager& presetManager, frontend::FileInterface* fileInterface = nullptr);

    enum ColourIDs
    {
        backgroundColour = 0x45671002,
        bubbleColour,
        textColour,
    };

    void paint (juce::Graphics&) override;
    void resized() override;

    void setNextPrevButton (const juce::Drawable* image, bool isNext);

    void hidePresetNameEditor();

protected:
    void mouseDown (const juce::MouseEvent& e) override;
    void colourChanged() override;
    virtual void saveUserPreset (nlohmann::json&& presetState);

    virtual void confirmAndDeletePreset (const Preset& presetToDelete, std::function<void (const Preset&)>&& presetDeleter);
    virtual void confirmAndOverwritePresetFile (const juce::File&, Preset&&, std::function<void (const juce::File&, Preset&&)>&& presetSaver);
    virtual void showFailedToLoadPresetMessage (const Preset&);

    PresetManager& presetManager;

    juce::DrawableButton prevPresetButton { "", juce::DrawableButton::ImageOnButtonBackground };
    juce::DrawableButton nextPresetButton { "", juce::DrawableButton::ImageOnButtonBackground };
    juce::Label presetNameDisplay;
    juce::TextEditor presetNameEditor;

    OptionalPointer<frontend::FileInterface> fileInterface {};
    std::unique_ptr<frontend::MenuInterface> menuInterface;

    frontend::NextPrevious presetsNextPrevious { presetManager };
    frontend::TextInterface textInterface { presetManager };

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsComponent)
};
} // namespace chowdsp::presets
