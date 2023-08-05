namespace chowdsp
{
template <size_t maxNumHarmonics, AdditiveOscSineApprox sineApprox, typename SampleType>
void AdditiveOscillator<maxNumHarmonics, sineApprox, SampleType>::setHarmonicAmplitudes (const SampleType (&amps)[maxNumHarmonics])
{
    setHarmonicAmplitudes (nonstd::span<const SampleType> { amps });
}

template <size_t maxNumHarmonics, AdditiveOscSineApprox sineApprox, typename SampleType>
void AdditiveOscillator<maxNumHarmonics, sineApprox, SampleType>::setHarmonicAmplitudes (nonstd::span<const SampleType> amps)
{
    jassert (amps.size() == maxNumHarmonics);

    size_t remainingHarmonics = maxNumHarmonics;
    size_t count = 0;
    while (remainingHarmonics >= vecSize)
    {
        amplitudes[count] = xsimd::load_unaligned (amps.data() + count * vecSize);
        ++count;
        remainingHarmonics -= vecSize;
    }

    if (remainingHarmonics > 0)
    {
        alignas (xsimd::default_arch::alignment()) SampleType arr[vecSize] {};
        std::copy (std::end (amps) - (int) remainingHarmonics, std::end (amps), std::begin (arr));
        amplitudes.back() = xsimd::load_aligned (arr);
    }

    setFrequency (oscFrequency, true);
}

template <size_t maxNumHarmonics, AdditiveOscSineApprox sineApprox, typename SampleType>
void AdditiveOscillator<maxNumHarmonics, sineApprox, SampleType>::setFrequency (SampleType frequencyHz, bool force)
{
    if (juce::approximatelyEqual (oscFrequency, frequencyHz) && ! force)
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

template <size_t maxNumHarmonics, AdditiveOscSineApprox sineApprox, typename SampleType>
void AdditiveOscillator<maxNumHarmonics, sineApprox, SampleType>::prepare (double sampleRate)
{
    twoPiOverFs = juce::MathConstants<SampleType>::twoPi / (SampleType) sampleRate;
    nyquistFreq = (SampleType) sampleRate * (SampleType) 0.495;

    setFrequency (oscFrequency, true);
}

template <size_t maxNumHarmonics, AdditiveOscSineApprox sineApprox, typename SampleType>
void AdditiveOscillator<maxNumHarmonics, sineApprox, SampleType>::reset (SampleType phase)
{
    oscPhase = phase - phaseEps;
    incrementPhase (oscPhase, phaseEps);
}

template <size_t maxNumHarmonics, AdditiveOscSineApprox sineApprox, typename SampleType>
void AdditiveOscillator<maxNumHarmonics, sineApprox, SampleType>::processBlock (const BufferView<SampleType>& buffer) noexcept
{
    ScopedValue scopedPhi { oscPhase };
    for (auto& sample : buffer.getWriteSpan (0))
    {
        sample += generateSample (iota, scopedPhi.get(), amplitudesInternal);
        incrementPhase (scopedPhi.get(), phaseEps);
    }

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        BufferMath::addBufferChannels (buffer, buffer, 0, ch);
}
} // namespace chowdsp
