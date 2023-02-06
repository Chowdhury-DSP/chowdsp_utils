namespace chowdsp
{
template <typename T>
void SquareWave<T>::setFrequency (T newFrequency) noexcept
{
    saw1.setFrequency (newFrequency);
    saw2.setFrequency (newFrequency);
}

template <typename T>
void SquareWave<T>::prepare (const juce::dsp::ProcessSpec& spec) noexcept
{
#if JUCE_MODULE_AVAILABLE_juce_dsp
    interMediateData = AudioBlock<T> (dataBlock, spec.numChannels, spec.maximumBlockSize);
#endif
    intermediateBuffer.setMaxSize ((int) spec.numChannels, (int) spec.maximumBlockSize);

    saw1.prepare (spec);
    saw2.prepare (spec);
    reset();
}

template <typename T>
void SquareWave<T>::reset (T phase) noexcept
{
    saw1.reset (phase);
    saw2.reset (phase + (T) 1);
}

template <typename T>
void SquareWave<T>::processBlock (const BufferView<T>& buffer) noexcept
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    intermediateBuffer.setCurrentSize (numChannels, numSamples);
    intermediateBuffer.clear();

    saw2.processBlock (intermediateBuffer);

    for (auto [ch, outputData] : buffer_iters::channels (buffer))
        juce::FloatVectorOperations::add (outputData.data(), intermediateBuffer.getReadPointer (ch), numSamples);

    intermediateBuffer.clear();
    saw1.processBlock (intermediateBuffer);

    for (auto [ch, outputData] : buffer_iters::channels (buffer))
        juce::FloatVectorOperations::subtract (outputData.data(), intermediateBuffer.getReadPointer (ch), numSamples);
}

#if JUCE_MODULE_AVAILABLE_juce_dsp
template <typename T>
template <typename ProcessContext>
void SquareWave<T>::process (const ProcessContext& context) noexcept
{
    auto&& outBlock = context.getOutputBlock();
    auto&& inBlock = context.getInputBlock();

    if (context.usesSeparateInputAndOutputBlocks())
        context.getOutputBlock().clear();

    if (context.isBypassed)
    {
        auto len = outBlock.getNumSamples();
        for (size_t i = 0; i < len; ++i)
            processSample();

        return;
    }

    auto&& intermediateBlock = interMediateData.getSubBlock (0, outBlock.getNumSamples());
    saw2.template process<ProcessContextNonReplacing<T>> (ProcessContextNonReplacing<T> { inBlock, intermediateBlock });

    if (context.usesSeparateInputAndOutputBlocks())
        outBlock += intermediateBlock;
    else
        outBlock.copyFrom (intermediateBlock);

    intermediateBlock.clear();
    saw1.template process<ProcessContextReplacing<T>> (ProcessContextReplacing<T> { intermediateBlock });
    outBlock -= intermediateBlock;
}
#endif
} // namespace chowdsp
