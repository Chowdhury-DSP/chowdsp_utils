#pragma once

#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

namespace chowdsp::EQ
{
/** Wraps a chowdsp::EQ::EQProcessor as a prototype EQ for a LinearPhaseEQ */
template <typename Float, typename EQParams, int numBands, typename EQBand>
struct LinearPhasePrototypeEQ
{
    static_assert (std::is_floating_point_v<Float>, "Float type must be a floating point type!");

    using Params = EQParams;
    using FloatType = Float;
    EQ::EQProcessor<FloatType, (size_t) numBands, EQBand> eq;

    LinearPhasePrototypeEQ() = default;

    void setParameters (const Params& eqParams)
    {
        Params::setEQParameters (eq, eqParams);
    }

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        eq.prepare (spec);
    }

    void reset()
    {
        eq.reset();
    }

    void processBlock (const BufferView<FloatType>& buffer)
    {
        eq.processBlock (buffer);
    }
};

/**
 * Class for creating a linear phase EQ, using a "prototype" EQ, which is not linear phase.
 *
 * The PrototypeEQ template class represents the prototype EQ. It must contain the following items:
 *   - `PrototypeEQ::Params` struct, which itself must define `operator==`
 *   - `PrototypeEQ::prepare (const juce::dsp::ProcessSpec&)`
 *   - `PrototypeEQ::reset()`
 *   - `PrototypeEQ::processBlock (juce::AudioBuffer<float>&)` (note that the buffer passed in will only contain one channel)
 *
 * The defaultFIRLength represents the FIR filter length to use at 48 kHz sampling rate.
 */
template <typename PrototypeEQ, int defaultFIRLength = 4096>
class LinearPhaseEQ : private juce::HighResolutionTimer
{
    using ProtoEQParams = typename PrototypeEQ::Params;

public:
    /** Default constructor. */
    LinearPhaseEQ() = default;
    ~LinearPhaseEQ() override;

    /** Implement this function to update the prototype EQ parameters. */
    std::function<void (PrototypeEQ&, const ProtoEQParams&)> updatePrototypeEQParameters = nullptr;

    /** Prepares the EQ to process a new stream of data. */
    void prepare (const juce::dsp::ProcessSpec& spec, const ProtoEQParams& initialParams);

    /** Sets the current EQ parameters. */
    void setParameters (const ProtoEQParams& newParams);

    /** Process a buffer of audio data. */
    void processBlock (const BufferView<float>& buffer) noexcept;

    /** Process a new block of audio data. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept;

    /** Returns the latency introduced by this processor. */
    [[nodiscard]] int getLatencySamples() const noexcept;

private:
    static int getIRSize (double sampleRate);

    void updateParams();
    void hiResTimerCallback() override;

    bool attemptIRTransfer();
    void processBlocksInternal (const AudioBlock<const float>& inputBlock, AudioBlock<float>& outputBlock) noexcept;

    PrototypeEQ prototypeEQ;
    EQParams<ProtoEQParams> params {};

    std::vector<std::unique_ptr<ConvolutionEngine<>>> engines;
    std::unique_ptr<IRTransfer> irTransfer;
    chowdsp::Buffer<typename PrototypeEQ::FloatType> irBuffer;
    std::vector<float> irTransferData;

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
} // namespace chowdsp::EQ

#include "chowdsp_LinearPhaseEQ.cpp"
