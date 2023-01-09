#pragma once

#include <chowdsp_presets_v2/chowdsp_presets_v2.h>

namespace chowdsp
{
class PresetsComponent : public juce::Component
{
public:
    explicit PresetsComponent (PresetManager& presetManaager);

    void paint (juce::Graphics&) override;
    void resized() override;

    void setNextPrevButton (const juce::Drawable* image, bool isNext);

protected:
    PresetManager& presetManager;

    juce::DrawableButton prevPresetButton, nextPresetButton;

private:
    PresetsFrontend::NextPrevious presetsNextPrevious { presetManager };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsComponent)
};
} // namespace chowdsp
