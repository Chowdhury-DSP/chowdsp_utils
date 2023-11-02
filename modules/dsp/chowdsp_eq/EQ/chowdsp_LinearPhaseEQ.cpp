namespace chowdsp::EQ
{
template <typename PrototypeEQ, int defaultFIRLength>
LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::~LinearPhaseEQ()
{
    stopTimer();
}

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
void LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::prepare (const juce::dsp::ProcessSpec& spec, const ProtoEQParams& initialParams)
{
    stopTimer();

    fs = spec.sampleRate;
    maxBlockSize = (int) spec.maximumBlockSize;
    irSize = getIRSize (fs);

    const auto fftOrder = Math::log2 (irSize);
    fft = std::make_unique<juce::dsp::FFT> (fftOrder);

    prototypeEQ.prepare ({ spec.sampleRate, (juce::uint32) irSize, 1 });

    irBuffer.setMaxSize (1, irSize);
    irTransferData.resize ((size_t) irSize, 0.0f);
    params = { initialParams };
    updateParams();

    engines.clear();
    for (size_t ch = 0; ch < spec.numChannels; ++ch)
        engines.push_back (std::make_unique<ConvolutionEngine<>> ((size_t) irSize, spec.maximumBlockSize, irTransferData.data()));

    irUpdateState.store (IRUpdateState::Good);
    irTransfer = std::make_unique<IRTransfer> (*engines[0]);

    startTimer (10); // @TODO: should we expose the timer frequency to the user?
}

template <typename PrototypeEQ, int defaultFIRLength>
void LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::hiResTimerCallback()
{
    if (irUpdateState == IRUpdateState::Needed)
    {
        updateParams();
        irTransfer->setNewIR (irTransferData.data());
        irUpdateState.store (IRUpdateState::Ready);
    }
}

template <typename PrototypeEQ, int defaultFIRLength>
void LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::setParameters (const ProtoEQParams& newParams)
{
    // no update needed
    if (params == newParams)
        return;

    if (irUpdateState != IRUpdateState::Good) // already waiting on an update
        return;

    // refresh params
    params = { newParams };
    irUpdateState.store (IRUpdateState::Needed);
}

template <typename PrototypeEQ, int defaultFIRLength>
void LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::updateParams()
{
    // update prototype EQ params...
    jassert (updatePrototypeEQParameters != nullptr); // better make sure this function is set before getting to this point!
    updatePrototypeEQParameters (prototypeEQ, params.params);

    // set up IR
    irBuffer.clear();
    auto* irData = irBuffer.getWritePointer (0);
    irData[irSize / 2 - 1] = (typename PrototypeEQ::FloatType) 1;

    // process forwards
    prototypeEQ.reset();
    prototypeEQ.processBlock (irBuffer);

    // process backwards
    std::reverse (irData, irData + irSize);
    prototypeEQ.reset();
    prototypeEQ.processBlock (irBuffer);

    // transfer from whatever floating-point precision the prototype EQ needs to single-precision
    std::copy (irData, irData + irSize, irTransferData.begin());

    // halve the IR magnitude since we processed it twice
    IRHelpers::makeHalfMagnitude (irTransferData.data(), irTransferData.data(), irSize, *fft);
}

template <typename PrototypeEQ, int defaultFIRLength>
bool LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::attemptIRTransfer()
{
    juce::SpinLock::ScopedTryLockType lock (irTransfer->mutex);
    if (! lock.isLocked())
        return false; // we weren't able to grab the irTransfer lock, so let's skip and  try again later!

    // Lock acquired! Let's do the swap
    for (const auto& eng : engines)
        irTransfer->transferIR (*eng);

    irUpdateState.store (IRUpdateState::Good);
    return true;
}

template <typename PrototypeEQ, int defaultFIRLength>
void LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::processBlock (const BufferView<float>& buffer) noexcept
{
    if (irUpdateState == IRUpdateState::Ready)
        attemptIRTransfer();

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        engines[(size_t) ch]->processSamples (buffer.getReadPointer (ch),
                                              buffer.getWritePointer (ch),
                                              (size_t) numSamples);
    }
}

template <typename PrototypeEQ, int defaultFIRLength>
void LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::processBlocksInternal (const chowdsp::AudioBlock<const float>& inBlock, chowdsp::AudioBlock<float>& outBlock) noexcept
{
    const auto numChannels = outBlock.getNumChannels();
    const auto numSamples = outBlock.getNumSamples();

    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        engines[ch]->processSamples (inBlock.getChannelPointer (ch),
                                     outBlock.getChannelPointer (ch),
                                     numSamples);
    }
}

template <typename PrototypeEQ, int defaultFIRLength>
template <typename ProcessContext>
void LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::process (const ProcessContext& context) noexcept
{
    if (irUpdateState == IRUpdateState::Ready)
        attemptIRTransfer();

    auto&& inBlock = context.getInputBlock();
    auto&& outBlock = context.getOutputBlock();

    // input and output blocks must be the same size!
    jassert (outBlock.getNumChannels() == inBlock.getNumChannels());
    jassert (outBlock.getNumSamples() == inBlock.getNumSamples());

    // Do regular processing
    processBlocksInternal (inBlock, outBlock);
}

template <typename PrototypeEQ, int defaultFIRLength>
int LinearPhaseEQ<PrototypeEQ, defaultFIRLength>::getLatencySamples() const noexcept
{
    return irSize / 2;
}
} // namespace chowdsp::EQ
