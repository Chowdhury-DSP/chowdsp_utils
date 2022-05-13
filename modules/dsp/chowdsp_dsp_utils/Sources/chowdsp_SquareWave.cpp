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
    interMediateData = chowdsp::AudioBlock<T> (dataBlock, spec.numChannels, spec.maximumBlockSize);

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
    saw2.template process<chowdsp::ProcessContextNonReplacing<T>> (chowdsp::ProcessContextNonReplacing<T> { inBlock, intermediateBlock });

    if (context.usesSeparateInputAndOutputBlocks())
        outBlock += intermediateBlock;
    else
        outBlock.copyFrom (intermediateBlock);

    intermediateBlock.clear();
    saw1.template process<chowdsp::ProcessContextReplacing<T>> (chowdsp::ProcessContextReplacing<T> { intermediateBlock });
    outBlock -= intermediateBlock;
}
} // namespace chowdsp
