#pragma once

namespace chowdsp::CLAPExtensions
{
class CLAPProcessorExtensions : public clap_juce_extensions::clap_extensions
{
public:
    CLAPProcessorExtensions() = default;
    virtual ~CLAPProcessorExtensions() = default;

    void initialise (juce::AudioProcessor& processor);

    bool supportsDirectProcess() override { return true; }
    clap_process_status clap_direct_process (const clap_process* process) noexcept override;

private:
    void process_clap_event (const clap_event_header_t* event, juce::MidiBuffer& midiBuffer, int sampleOffset);

    juce::AudioProcessor* processor = nullptr;

    std::unordered_map<juce::AudioProcessorParameter*, ParamUtils::ModParameterMixin*> modulatableParameters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CLAPProcessorExtensions)
};
} // namespace chowdsp::CLAPExtensions
