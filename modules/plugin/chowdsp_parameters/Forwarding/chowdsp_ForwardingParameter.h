#pragma once

namespace chowdsp
{
#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
class PluginState;
#endif

/**
 * Class for forwarding "dynamic" parameters from one processor to another.
 *
 * The original implementation for this class was borrowed
 * (with permission) from Eyal Amir, and then modified a
 * little bit from there.
 */
class ForwardingParameter : public juce::RangedAudioParameter,
                            public ParamUtils::ModParameterMixin
{
public:
#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
    /**
     * Construct a new forwarding parameter.
     *
     * @param id            Parameter ID for the <b>forwarded</b> parameter
     * @param um            UndoManager to use with this parameter
     * @param defaultName   Name to use when this parameter is not forwarding another one
     */
    explicit ForwardingParameter (const ParameterID& id, PluginState& pluginState, const juce::String& defaultName = "Unused");
#else
    /**
     * Construct a new forwarding parameter.
     *
     * @param id            Parameter ID for the <b>forwarded</b> parameter
     * @param um            UndoManager to use with this parameter
     * @param defaultName   Name to use when this parameter is not forwarding another one
     */
    explicit ForwardingParameter (const ParameterID& id, juce::UndoManager* um = nullptr, const juce::String& defaultName = "Unused");
#endif
    ~ForwardingParameter() override;

    /**
     * Sets a new parameter to be forwarded.
     * 
     * Note that the parameter must not be deleted while
     * it is being held as the forwarding parameter.
     */
    void setParam (juce::RangedAudioParameter* paramToUse, const juce::String& newName = {}, bool deferHostNotification = false);

    /** Returns the parameter currently being forwarded */
    const auto* getParam() const noexcept { return internalParam; }

    /** Sets a new processor to forward to */
    void setProcessor (juce::AudioProcessor* processorToUse);

    /** Reports to the host that the processor's parameter info has changed */
    static void reportParameterInfoChange (juce::AudioProcessor* processor);

private:
#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
    struct ForwardingAttachment;
#else
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
#endif

    float getValue() const override;
    void setValue (float newValue) override;
    float getDefaultValue() const override;
    juce::String getText (float value, int i) const override;
    float getValueForText (const juce::String& text) const override;
    juce::String getName (int i) const override;

    const juce::NormalisableRange<float>& getNormalisableRange() const override;

    bool supportsMonophonicModulation() override;
    bool supportsPolyphonicModulation() override;
    void applyMonophonicModulation (double value) override;
    void applyPolyphonicModulation (int32_t note_id, int16_t port_index, int16_t channel, int16_t key, double value) override;

    juce::AudioProcessor* processor = nullptr;
    juce::RangedAudioParameter* internalParam = nullptr;
    ParamUtils::ModParameterMixin* internalParamAsModulatable = nullptr;

    std::unique_ptr<ForwardingAttachment> attachment;
#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
    PluginState& pluginState;
#else
    juce::UndoManager* undoManager;
    juce::SpinLock paramLock;
#endif

    const juce::String defaultName;
    juce::String customName = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ForwardingParameter)
};
} // namespace chowdsp
