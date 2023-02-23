namespace chowdsp
{
template <size_t maxNumHarmonics, typename SampleType>
void AdditiveOscillator<maxNumHarmonics, SampleType>::setHarmonicAmplitudes (const SampleType (&amps)[maxNumHarmonics])
{
    size_t remainingHarmonics = maxNumHarmonics;
    size_t count = 0;
    while (remainingHarmonics >= vecSize)
    {
        amplitudes[count] = xsimd::load_unaligned (amps + count * vecSize);
        ++count;
        remainingHarmonics -= vecSize;
    }

    if (remainingHarmonics > 0)
    {
        alignas (xsimd::default_arch::alignment()) SampleType arr[vecSize] {};
        std::copy (std::end (amps) - remainingHarmonics, std::end (amps), std::begin (arr));
        amplitudes.back() = xsimd::load_aligned (arr);
    }

    setFrequency (oscFrequency, true);
}

template <size_t maxNumHarmonics, typename SampleType>
void AdditiveOscillator<maxNumHarmonics, SampleType>::setFrequency (SampleType frequencyHz, bool force)
{
    if (oscFrequency == frequencyHz && ! force)
        return;

    oscFrequency = frequencyHz;
    phaseEps = twoPiOverFs * oscFrequency;

    auto vecFreqMult = iota;
    for (auto [i, amps] : enumerate (amplitudes))
    {
        const auto sineFreq = vecFreqMult * frequencyHz;
        vecFreqMult += (SampleType) vecSize;
        amplitudesInternal[i] = xsimd::select (sineFreq < nyquistFreq, amps, Vec {});
    }
}

template <size_t maxNumHarmonics, typename SampleType>
void AdditiveOscillator<maxNumHarmonics, SampleType>::prepare (double sampleRate, [[maybe_unused]] int samplesPerBlock)
{
    twoPiOverFs = juce::MathConstants<SampleType>::twoPi / (SampleType) sampleRate;
    nyquistFreq = (SampleType) sampleRate * (SampleType) 0.495;

    setFrequency (oscFrequency, true);
}

template <size_t maxNumHarmonics, typename SampleType>
void AdditiveOscillator<maxNumHarmonics, SampleType>::reset (SampleType phase)
{
    oscPhase.reset (phase);
}

template <size_t maxNumHarmonics, typename SampleType>
void AdditiveOscillator<maxNumHarmonics, SampleType>::processBlock (const BufferView<SampleType>& buffer) noexcept
{
    for (auto& sample : buffer.getWriteSpan (0))
    {
        auto vecFreqMult = iota;

        Vec result {};
        for (auto& amp : amplitudesInternal)
        {
            result += amp * xsimd::sin (vecFreqMult * oscPhase.phase);
            vecFreqMult += (SampleType) vecSize;
        }
        sample = xsimd::reduce_add (result);

        oscPhase.advance (phaseEps);
    }

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        BufferMath::copyBufferChannels (buffer, buffer, 0, ch);
}
} // namespace chowdsp
