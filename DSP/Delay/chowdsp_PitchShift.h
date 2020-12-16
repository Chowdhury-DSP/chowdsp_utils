namespace chowdsp
{

template <typename SampleType, typename InterpolationType = DelayLineInterpolationTypes::Linear>
class PitchShifter
{
public:
    /** Default constructor. */
    PitchShifter();

    /** Constructor. */
    explicit PitchShifter (int maximumBufferSize, int crossfadeOverlap = 256);

    /** Initialises the processor. */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Resets the internal state variables of the processor. */
    void reset();

    /** Sets the pitch shift in semitones. */
    void setShiftSemitones (SampleType shiftSemitones);

    /** Returns the current delay in samples. */
    SampleType getShiftSemitones() const noexcept;

    /** Sets the pitch shift factor as multiplier. */
    void setShiftFactor (SampleType shiftFactor);

    /** Returns the current delay in samples. */
    SampleType getShiftFactor() const noexcept;

    /** Processes a single sample */
    inline SampleType processSample (size_t ch, SampleType x) noexcept
    {
        auto& writePtr = writePos[ch];
        auto& readPtr = readPos[ch];
        auto& chCross = crossfade[ch];

        // write to double ring buffer
        bufferPtrs[ch][writePtr] = x;

        const auto readPtr2 = readPtr >= halfSize ? readPtr - halfSize : readPtr + halfSize;
        SampleType rd0 = readSample (ch, readPtr);
        SampleType rd1 = readSample (ch, readPtr2);
        
        // crossfade
        if (overlap >= (writePtr - (int) readPtr) && (writePtr - readPtr) >= 0)
            chCross = ((SampleType) writePtr - readPtr) / (SampleType) overlap;
        else if (writePtr - (int) readPtr == 0)
            chCross = (SampleType) 0;

        if (overlap >= (writePtr - (int) readPtr2) && (writePtr - readPtr2) >= 0)
            chCross = (SampleType) 1.0 - ((SampleType) writePtr - readPtr2) / (SampleType) overlap;
        else if (writePtr - (int) readPtr2 == 0)
            chCross = (SampleType) 1;
            
        auto y = rd0 * chCross + rd1 * ((SampleType) 1.0 - chCross);

        // iterate pointers
        writePtr = writePtr + 1 >= totalSize ? 0 : writePtr + 1;
        readPtr = int (readPtr + shift) >= totalSize ? (SampleType) 0 : readPtr + shift;

        return y;
    }

    /** Processes the input and output samples supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock      = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples  = outputBlock.getNumSamples();

        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumChannels() == this->writePos.size());
        jassert (inputBlock.getNumSamples()  == numSamples);

        if (context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* inputSamples = inputBlock.getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
                outputSamples[i] = processSample (channel, inputSamples[i]);
        }
    }

private:
    inline SampleType readSample (size_t channel, SampleType readPtr) noexcept
    {
        auto delayInt = (int) readPtr;
        auto delayFrac = readPtr - delayInt;
        interpolator.updateInternalVariables (delayInt, delayFrac);
        return interpolator.call (bufferPtrs[channel], delayInt, delayFrac, v[channel]);
    }

    juce::AudioBuffer<SampleType> bufferData; // Ring buffer for storing data
    SampleType** bufferPtrs = nullptr;        // pointers to data buffer
    std::vector<SampleType> v; // State needed for Thiran interpolator
    InterpolationType interpolator;

    // Read and write ptrs
    std::vector<int> writePos;
    std::vector<SampleType> readPos;

    SampleType shift;                   // Shift factor to increment read ptr
    std::vector<SampleType> crossfade;  // crossfade gain

    int overlap;   // crossfade overlap
    int totalSize; // max buffer size
    SampleType halfSize; // half buffer size

    SampleType alpha = 0.0; // Needed for Thiran interpolation

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchShifter)
};

} // chowdsp
