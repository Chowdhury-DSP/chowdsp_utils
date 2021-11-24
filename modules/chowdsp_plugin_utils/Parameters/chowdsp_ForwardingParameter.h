#pragma once

namespace chowdsp
{
/**
 * Class for forwarding "dynamic" parameters from one processor to another.
 *
 * This implementation was borrowed (with permission and some small
 * modifications) from Eyal Amir.
 */
class ForwardingParameter : public juce::RangedAudioParameter,
                            private juce::AudioProcessorParameter::Listener
{
public:
    /**
     * Construct a new forwarding parameter.
     *
     * @param id            Parameter ID for the <b>forwarded</b> parameter
     * @param defaultName   Name to use when this parameter is not forwarding another one
     */
    explicit ForwardingParameter (const juce::String& id, const juce::String& defaultName = "Unused");

    /** Sets a new parameter to be forwarded */
    void setParam (juce::RangedAudioParameter* paramToUse, const juce::String& newName = {});

    /** Sets a new processor to forward to */
    void setProcessor (juce::AudioProcessor* processorToUse);

private:
    float getValue() const override;
    void setValue (float newValue) override;
    float getDefaultValue() const override;
    juce::String getText (float value, int i) const override;
    float getValueForText (const juce::String& text) const override;
    juce::String getName (int i) const override;

    const juce::NormalisableRange<float>& getNormalisableRange() const override;

    void parameterValueChanged (int paramIdx, float newValue) override;
    void parameterGestureChanged (int paramIdx, bool gestureIsStarting) override;

    juce::AudioProcessor* processor = nullptr;
    juce::RangedAudioParameter* internalParam = nullptr;

    const juce::String defaultName;
    juce::String customName = {};

    bool ignoreCallbacks = false;
};
} // namespace chowdsp
