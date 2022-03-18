#include "chowdsp_LinearPhaseEQ.h"

namespace chowdsp
{
template <typename PrototypeEQ, int defaultFIRLength>
int LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::getIRSize (double sampleRate)
{
    int irSize = defaultFIRLength; // default IR size
    while (sampleRate > 48100.0) // 48 kHz + 100 Hz padding
    {
        irSize *= 2;
        sampleRate /= 2.0;
    }

    return irSize;
}

template <typename PrototypeEQ, int defaultFIRLength>
void LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::prepare (const juce::dsp::ProcessSpec& spec, const EQParams& initialParams)
{
    stopTimer();

    fs = spec.sampleRate;
    maxBlockSize = (int) spec.maximumBlockSize;
    irSize = getIRSize (fs);

    const auto fftOrder = chowdsp::log2 (irSize);
    fft = std::make_unique<juce::dsp::FFT> (fftOrder);

    prototypeEQ.prepare (spec);

    irBuffer = juce::AudioBuffer<float> (1, irSize);
    params = initialParams;
    updateParams();

    engines.clear();
    for (size_t ch = 0; ch < spec.numChannels; ++ch)
        engines.push_back (std::make_unique<chowdsp::ConvolutionEngine> ((size_t) irSize, spec.maximumBlockSize, irBuffer.getWritePointer (0)));

    irUpdateState.store (IRUpdateState::Good);
    irTransfer = std::make_unique<chowdsp::IRTransfer> (*engines[0]);

    startTimer (5);
}

template <typename PrototypeEQ, int defaultFIRLength>
void LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::hiResTimerCallback()
{
    if (irUpdateState == IRUpdateState::Needed)
    {
        updateParams();
        irTransfer->setNewIR (irBuffer.getReadPointer (0));
        irUpdateState.store (IRUpdateState::Ready);
    }
}

template <typename PrototypeEQ, int defaultFIRLength>
void LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::setParameters (const EQParams& newParams)
{
    // no update needed
    if (params == newParams)
        return;

    if (irUpdateState != IRUpdateState::Good) // already waiting on an update
        return;

    // refresh params
    params = newParams;
    irUpdateState.store (IRUpdateState::Needed);
}

template <typename PrototypeEQ, int defaultFIRLength>
void LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::updateParams()
{
    // update prototype EQ params...
    jassert (updatePrototypeEQParameters != nullptr); // better make sure this function is set before getting to this point!
    updatePrototypeEQParameters (prototypeEQ, params);

    // set up IR
    irBuffer.clear();
    auto* irData = irBuffer.getWritePointer (0);
    irData[irSize / 2 - 1] = 1.0f;

    // process forwards
    prototypeEQ.reset();
    prototypeEQ.processBlock (irBuffer);

    // process backwards
    irBuffer.reverse (0, irSize);
    prototypeEQ.reset();
    prototypeEQ.processBlock (irBuffer);

    chowdsp::IRHelpers::makeHalfMagnitude (irData, irData, irSize, *fft);
}

template <typename PrototypeEQ, int defaultFIRLength>
template <typename ProcessContext>
void LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::process (const ProcessContext& context)
{
    if (irUpdateState == IRUpdateState::Ready)
    {
        juce::SpinLock::ScopedTryLockType lock (irTransfer->mutex);
        if (lock.isLocked())
        {
            for (auto& eng : engines)
                irTransfer->transferIR (*eng);

            irUpdateState.store (IRUpdateState::Good);
        }
    }

    auto&& inBlock = context.getInputBlock();
    auto&& outBlock = context.getOutputBlock();

    // copy input to output if needed
    if (context.usesSeparateInputAndOutputBlocks())
        chowdsp::AudioBlockHelpers::copyBlocks (outBlock, inBlock);

    for (size_t ch = 0; ch < outBlock.getNumChannels(); ++ch)
    {
        engines[ch]->processSamples (outBlock.getChannelPointer (ch),
                                     outBlock.getChannelPointer (ch),
                                     outBlock.getNumSamples());
    }
}

template <typename PrototypeEQ, int defaultFIRLength>
int LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::getLatencySamples() const noexcept
{
    return irSize / 2;
}
} // namespace chowdsp
