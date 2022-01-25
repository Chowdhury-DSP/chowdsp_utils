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
    interMediateData = juce::dsp::AudioBlock<T> (dataBlock, spec.numChannels, spec.maximumBlockSize);

    saw1.prepare (spec);
    saw2.prepare (spec);
}

template <typename T>
void SquareWave<T>::reset (T phase) noexcept
{
    saw1.reset (phase);
    saw2.reset (phase + (T) 0.5);
}

template <typename T>
template <typename ProcessContext>
void SquareWave<T>::process (const ProcessContext& context) noexcept
{
    auto&& outBlock = context.getOutputBlock();
    auto&& inBlock = context.getInputBlock();

    auto&& intBlock = interMediateData.getSubBlock (0, outBlock.getNumSamples());
    saw1.template process (juce::dsp::ProcessContextNonReplacing<T> { inBlock, intBlock });

    if (context.usesSeparateInputAndOutputBlocks())
        outBlock += intBlock;
    else
        AudioBlockHelpers::copyBlocks (outBlock, intBlock);

    saw2.template process (juce::dsp::ProcessContextNonReplacing<T> { inBlock, intBlock });
    outBlock -= intBlock;
}
} // namespace chowdsp
