#pragma once

namespace chowdsp
{
/** A processor similarto chowdsp::ThreeWayCrossoverFilter, but linear phase */
template <int Order>
class LinearPhase3WayCrossover : private juce::HighResolutionTimer
{
public:
    static_assert (Order > 2 && Order % 2 == 0, "This implementation only works with even filter orders greater than 2");

    LinearPhase3WayCrossover() = default;

    ~LinearPhase3WayCrossover() override
    {
        stopTimer();
    }

    /** Prepares the crossover filter with an IR length and initial crossover frequencies */
    void prepare (const juce::dsp::ProcessSpec& spec, int irLength, float lowBandCrossoverHz, float highBandCrossoverHz)
    {
        fft = std::make_unique<juce::dsp::FFT> (Math::log2 (irLength));
        irSize = irLength;

        const auto protoSpec = juce::dsp::ProcessSpec { spec.sampleRate, (uint32_t) irLength, 1 };
        prototypeFilters.prepare (protoSpec);

        for (auto& singleBandConv : convolutions)
        {
            singleBandConv.irBuffer.setMaxSize (1, irSize);
            singleBandConv.irTransferData.resize ((size_t) irSize, 0.0f);
        }

        params = { lowBandCrossoverHz, highBandCrossoverHz };
        updateParams();

        for (auto& singleBandConv : convolutions)
        {
            singleBandConv.engines.clear();
            for (size_t ch = 0; ch < spec.numChannels; ++ch)
                singleBandConv.engines.push_back (std::make_unique<ConvolutionEngine<>> ((size_t) irSize,
                                                                                         spec.maximumBlockSize,
                                                                                         singleBandConv.irTransferData.data()));
            singleBandConv.irTransfer = std::make_unique<IRTransfer> (*singleBandConv.engines[0]);
        }
        irUpdateState.store (IRUpdateState::Good);

        startTimer (10);
    }

    /** Sets the crossover frequency between the low/mid/high bands. */
    void setParameters (float lowBandCrossoverHz, float highBandCrossoverHz)
    {
        // no update needed
        if (lowBandCrossoverHz == params.lowBandCrossover && highBandCrossoverHz == params.highBandCrossover)
            return;

        if (irUpdateState != IRUpdateState::Good) // already waiting on an update
            return;

        // refresh params
        params = { lowBandCrossoverHz, highBandCrossoverHz };
        irUpdateState.store (IRUpdateState::Needed);
    }

    /** Resets the processor state */
    void reset()
    {
        for (auto& singleBandConv : convolutions)
        {
            for (auto& engine : singleBandConv.engines)
                engine->reset();
        }
    }

    /**
     * Processes the all bands of the crossover filter.
     *
     * The low-band processing may happen in-place, but the mid and
     * high bands been to be processed out-of-place.
     */
    [[maybe_unused]] void processBlock (const BufferView<const float>& bufferIn,
                                        const BufferView<float>& bufferLow,
                                        const BufferView<float>& bufferMid,
                                        const BufferView<float>& bufferHigh) noexcept
    {
        if (irUpdateState == IRUpdateState::Ready)
        {
            bool irTransferSuccess = true;
            for (int i = 0; i < 3; ++i)
                irTransferSuccess &= attemptIRTransfer (i);
            if (irTransferSuccess)
                irUpdateState.store (IRUpdateState::Good);
        }

        const auto numChannels = bufferIn.getNumChannels();
        const auto numSamples = bufferIn.getNumSamples();

        // make sure all the buffers are the same size
        jassert (bufferLow.getNumChannels() == numChannels);
        jassert (bufferLow.getNumSamples() == numSamples);
        jassert (bufferMid.getNumChannels() == numChannels);
        jassert (bufferMid.getNumSamples() == numSamples);
        jassert (bufferHigh.getNumChannels() == numChannels);
        jassert (bufferHigh.getNumSamples() == numSamples);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            convolutions[2].engines[(size_t) ch]->processSamples (bufferIn.getReadPointer (ch),
                                                                  bufferHigh.getWritePointer (ch),
                                                                  (size_t) numSamples);
            convolutions[1].engines[(size_t) ch]->processSamples (bufferIn.getReadPointer (ch),
                                                                  bufferMid.getWritePointer (ch),
                                                                  (size_t) numSamples);
            convolutions[0].engines[(size_t) ch]->processSamples (bufferIn.getReadPointer (ch),
                                                                  bufferLow.getWritePointer (ch),
                                                                  (size_t) numSamples);
        }
    }

private:
    bool attemptIRTransfer (int bandIndex)
    {
        juce::SpinLock::ScopedTryLockType lock (convolutions[(size_t) bandIndex].irTransfer->mutex);
        if (! lock.isLocked())
            return false; // we weren't able to grab the irTransfer lock, so let's skip and  try again later!

        // Lock acquired! Let's do the swap
        for (const auto& eng : convolutions[(size_t) bandIndex].engines)
            convolutions[(size_t) bandIndex].irTransfer->transferIR (*eng);

        return true;
    }

    void hiResTimerCallback() override
    {
        if (irUpdateState == IRUpdateState::Needed)
        {
            updateParams();
            for (auto& singleBandConv : convolutions)
                singleBandConv.irTransfer->setNewIR (singleBandConv.irTransferData.data());
            irUpdateState.store (IRUpdateState::Ready);
        }
    }

    void updateParams()
    {
        prototypeFilters.setLowCrossoverFrequency (params.lowBandCrossover);
        prototypeFilters.setHighCrossoverFrequency (params.highBandCrossover);

        const auto processFilters = [this] (int bandIndex, const BufferView<float>& buffer)
        {
            if (bandIndex == 0) // low band
            {
                for (auto& filt : prototypeFilters.lowBandHighCut)
                {
                    filt.reset();
                    filt.processBlock (buffer);
                }
            }
            else if (bandIndex == 1) // mid band
            {
                for (auto& filt : prototypeFilters.midBandLowCut)
                {
                    filt.reset();
                    filt.processBlock (buffer);
                }
                for (auto& filt : prototypeFilters.midBandHighCut)
                {
                    filt.reset();
                    filt.processBlock (buffer);
                }
            }
            else if (bandIndex == 2) // high band
            {
                for (auto& filt : prototypeFilters.midBandLowCut)
                {
                    filt.reset();
                    filt.processBlock (buffer);
                }
                for (auto& filt : prototypeFilters.highBandLowCut)
                {
                    filt.reset();
                    filt.processBlock (buffer);
                }
            }
        };

        for (auto [i, singleBandConv] : enumerate (convolutions))
        {
            // set up IR
            singleBandConv.irBuffer.clear();
            auto* irData = singleBandConv.irBuffer.getWritePointer (0);
            irData[irSize / 2 - 1] = 1.0f;

            // process forwards
            processFilters ((int) i, singleBandConv.irBuffer);

            // process backwards
            std::reverse (irData, irData + irSize);
            processFilters ((int) i, singleBandConv.irBuffer);

            // halve the IR magnitude since we processed it twice
            IRHelpers::makeHalfMagnitude (singleBandConv.irTransferData.data(), irData, irSize, *fft);
        }
    }

    struct Params
    {
        float lowBandCrossover;
        float highBandCrossover;
    } params;

    struct PrototypeFilters
    {
        std::array<ButterworthFilter<Order / 2, ButterworthFilterType::Lowpass>, 2> lowBandHighCut {};
        std::array<ButterworthFilter<Order / 2, ButterworthFilterType::Highpass>, 2> midBandLowCut {};
        std::array<ButterworthFilter<Order / 2, ButterworthFilterType::Lowpass>, 2> midBandHighCut {};
        std::array<ButterworthFilter<Order / 2, ButterworthFilterType::Highpass>, 2> highBandLowCut {};

        void prepare (const juce::dsp::ProcessSpec& spec)
        {
            fs = (float) spec.sampleRate;
            for (auto& filter : lowBandHighCut)
                filter.prepare ((int) spec.numChannels);
            for (auto& filter : midBandLowCut)
                filter.prepare ((int) spec.numChannels);
            for (auto& filter : midBandHighCut)
                filter.prepare ((int) spec.numChannels);
            for (auto& filter : highBandLowCut)
                filter.prepare ((int) spec.numChannels);
        }

        void setLowCrossoverFrequency (float freqHz)
        {
            for (auto& filter : lowBandHighCut)
                filter.calcCoefs (freqHz, CoefficientCalculators::butterworthQ<float>, fs);
            for (auto& filter : midBandLowCut)
                filter.calcCoefs (freqHz, CoefficientCalculators::butterworthQ<float>, fs);
        }

        void setHighCrossoverFrequency (float freqHz)
        {
            for (auto& filter : midBandHighCut)
                filter.calcCoefs (freqHz, CoefficientCalculators::butterworthQ<float>, fs);
            for (auto& filter : highBandLowCut)
                filter.calcCoefs (freqHz, CoefficientCalculators::butterworthQ<float>, fs);
        }

    private:
        float fs = 48000.0f;
    } prototypeFilters;

    enum class IRUpdateState
    {
        Good,
        Needed,
        Ready,
    };
    std::atomic<IRUpdateState> irUpdateState { IRUpdateState::Good };

    struct SingleBandConvolution
    {
        std::vector<std::unique_ptr<ConvolutionEngine<>>> engines {};
        std::unique_ptr<IRTransfer> irTransfer {};
        Buffer<float> irBuffer {};
        std::vector<float> irTransferData {};
    };
    std::array<SingleBandConvolution, 3> convolutions {};

    std::unique_ptr<juce::dsp::FFT> fft {};
    int irSize = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinearPhase3WayCrossover)
};
} // namespace chowdsp
