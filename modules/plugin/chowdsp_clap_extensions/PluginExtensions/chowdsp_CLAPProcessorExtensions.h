#pragma once

namespace chowdsp::CLAPExtensions
{
class CLAPProcessorExtensions : public clap_juce_extensions::clap_juce_audio_processor_capabilities
{
public:
    CLAPProcessorExtensions() = default;
    virtual ~CLAPProcessorExtensions() = default;

    void initialise (juce::AudioProcessor& processor);

    bool supportsDirectEvent (uint16_t space_id, uint16_t type) override;
    void handleDirectEvent (const clap_event_header_t* event, int sampleOffset) override;

private:
    juce::AudioProcessor* processor = nullptr;

    std::unordered_map<juce::AudioProcessorParameter*, ParamUtils::ModParameterMixin*> modulatableParameters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CLAPProcessorExtensions)
};
} // namespace chowdsp::CLAPExtensions
