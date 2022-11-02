namespace chowdsp
{
template <typename SampleType, typename InterpolationType>
PitchShifter<SampleType, InterpolationType>::PitchShifter()
    : PitchShifter (4096)
{
}

template <typename SampleType, typename InterpolationType>
PitchShifter<SampleType, InterpolationType>::PitchShifter (int maximumBufferSize, int crossfadeOverlap)
{
    // buffer needs size!
    jassert (maximumBufferSize > 0);

    // overlap needs space in between overlaps!
    jassert (crossfadeOverlap > 0 && crossfadeOverlap < maximumBufferSize / 4);

    overlap = crossfadeOverlap;
    totalSize = juce::jmax (32, maximumBufferSize);
    halfSize = (SampleType) totalSize / 2;
}

template <typename SampleType, typename InterpolationType>
void PitchShifter<SampleType, InterpolationType>::prepare (const juce::dsp::ProcessSpec& spec)
{
    jassert (spec.numChannels > 0);

    bufferData.setMaxSize ((int) spec.numChannels, 2 * totalSize);
    bufferPtrs = bufferData.getArrayOfWritePointers();
    jassert (bufferPtrs != nullptr);

    writePos.resize (spec.numChannels);
    readPos.resize (spec.numChannels);
    crossfade.resize (spec.numChannels);

    v.resize (spec.numChannels);

    reset();
}

template <typename SampleType, typename InterpolationType>
void PitchShifter<SampleType, InterpolationType>::reset()
{
    std::fill (writePos.begin(), writePos.end(), 0);
    std::fill (readPos.begin(), readPos.end(), (SampleType) 0);
    std::fill (crossfade.begin(), crossfade.end(), (SampleType) 0);
    std::fill (v.begin(), v.end(), (SampleType) 0);

    bufferData.clear();
}

template <typename SampleType, typename InterpolationType>
void PitchShifter<SampleType, InterpolationType>::setShiftSemitones (SampleType shiftSemitones)
{
    shift = std::pow ((SampleType) 2, shiftSemitones / (SampleType) 12.0);
}

template <typename SampleType, typename InterpolationType>
SampleType PitchShifter<SampleType, InterpolationType>::getShiftSemitones() const noexcept
{
    return std::log2 (shift);
}

template <typename SampleType, typename InterpolationType>
void PitchShifter<SampleType, InterpolationType>::setShiftFactor (SampleType shiftFactor)
{
    shift = shiftFactor;
}

template <typename SampleType, typename InterpolationType>
SampleType PitchShifter<SampleType, InterpolationType>::getShiftFactor() const noexcept
{
    return shift;
}

} // namespace chowdsp
