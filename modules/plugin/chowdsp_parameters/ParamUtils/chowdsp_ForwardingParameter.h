#pragma once

namespace chowdsp
{
/**
 * Class for forwarding "dynamic" parameters from one processor to another.
 *
 * The original implementation for this class was borrowed
 * (with permission) from Eyal Amir, and then modified a
 * little bit from there.
 */
class ForwardingParameter : public juce::RangedAudioParameter
{
public:
    /**
     * Construct a new forwarding parameter.
     *
     * @param id            Parameter ID for the <b>forwarded</b> parameter
     * @param um            UndoManager to use with this parameter
     * @param defaultName   Name to use when this parameter is not forwarding another one
     */
    explicit ForwardingParameter (const juce::String& id, juce::UndoManager* um = nullptr, const juce::String& defaultName = "Unused");

    /** Sets a new parameter to be forwarded */
    void setParam (juce::RangedAudioParameter* paramToUse, const juce::String& newName = {});

    /** Returns the parameter currently being forwarded */
    const auto* getParam() const noexcept { return internalParam; }

    /** Sets a new processor to forward to */
    void setProcessor (juce::AudioProcessor* processorToUse);

private:
    struct ForwardingAttachment : private juce::AudioProcessorParameter::Listener,
                                  private juce::AsyncUpdater
    {
        ForwardingAttachment (juce::RangedAudioParameter& internal, juce::RangedAudioParameter& forwarding, juce::UndoManager* um);
        ~ForwardingAttachment() override;

        void setNewValue (float paramVal);

    private:
        void beginGesture();
        void endGesture();

        void handleAsyncUpdate() override;
        void parameterValueChanged (int paramIdx, float newValue) override;
        void parameterGestureChanged (int paramIdx, bool gestureIsStarting) override;

        juce::RangedAudioParameter& internalParam;
        juce::RangedAudioParameter& forwardingParam;
        juce::UndoManager* undoManager;

        float newValue = 0.0f;
        bool ignoreCallbacks = false;
    };

    float getValue() const override;
    void setValue (float newValue) override;
    float getDefaultValue() const override;
    juce::String getText (float value, int i) const override;
    float getValueForText (const juce::String& text) const override;
    juce::String getName (int i) const override;

    const juce::NormalisableRange<float>& getNormalisableRange() const override;

    juce::AudioProcessor* processor = nullptr;
    juce::RangedAudioParameter* internalParam = nullptr;

    std::unique_ptr<ForwardingAttachment> attachment;
    juce::UndoManager* undoManager;
    juce::SpinLock paramLock;

    const juce::String defaultName;
    juce::String customName = {};
};
} // namespace chowdsp
