#pragma once

#include <chowdsp_dsp/chowdsp_dsp.h>

namespace chowdsp
{

template <typename PrototypeEQ, int defaultFIRLength = 4096>
class LinearPhaseEQ : private juce::HighResolutionTimer
{
    using EQParams = typename PrototypeEQ::Params;

public:
    LinearPhaseEQ() = default;

    std::function<void (PrototypeEQ&, const EQParams&)> updatePrototypeEQParameters = nullptr;

    void prepare (const juce::dsp::ProcessSpec& spec, const EQParams& initialParams);

    void setParameters (const EQParams& newParams);

    template <typename ProcessContext>
    void process (const ProcessContext& context);

    int getLatencySamples() const noexcept;

private:
    static int getIRSize (double sampleRate);
    void updateParams();
    void hiResTimerCallback() override;

    PrototypeEQ prototypeEQ;
    EQParams params;

    std::vector<std::unique_ptr<chowdsp::ConvolutionEngine>> engines;
    std::unique_ptr<chowdsp::IRTransfer> irTransfer;
    juce::AudioBuffer<float> irBuffer;

    double fs = 48000.0;
    int maxBlockSize = 512;
    int irSize = 0;

    enum class IRUpdateState
    {
        Good,
        Needed,
        Ready,
    };
    std::atomic<IRUpdateState> irUpdateState;

    std::unique_ptr<juce::dsp::FFT> fft;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinearPhaseEQ)
};
} // namespace chowdsp

#include "chowdsp_LinearPhaseEQ.cpp"
