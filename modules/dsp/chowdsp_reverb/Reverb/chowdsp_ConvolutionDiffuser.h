#pragma once

namespace chowdsp::Reverb
{
/** Adds diffusion to the signal by convolving the signal with a noise kernel. */
class ConvolutionDiffuser
{
public:
    /** Initialises the processor with a max diffusion time */
    explicit ConvolutionDiffuser (double maxDiffusionLengthSeconds = 0.1) : maxDiffusionSeconds (maxDiffusionLengthSeconds)
    {
    }

    /** Prepares this class to process a new stream of data */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = (float) spec.sampleRate;

        const auto kernelSize = juce::nextPowerOfTwo (juce::roundToInt (spec.sampleRate * maxDiffusionSeconds));
        generateConvolutionKernel (spec.sampleRate, (int) spec.numChannels, kernelSize);

        convolutionEngines.clear();
        convolutionEngines.reserve (spec.numChannels);
        for (size_t ch = 0; ch < spec.numChannels; ++ch)
            convolutionEngines.emplace_back ((size_t) kernelSize, spec.maximumBlockSize);

        scratchBuffer.setMaxSize ((int) spec.numChannels, kernelSize);
        setDiffusionTime (diffusionTimeSeconds);
    }

    /** Resets the state of the processor */
    void reset()
    {
        for (auto& eng : convolutionEngines)
            eng.reset();
    }

    /** Sets the diffusion time in seconds */
    void setDiffusionTime (float newDiffusionTimeSeconds)
    {
        jassert (newDiffusionTimeSeconds <= (float) maxDiffusionSeconds);
        diffusionTimeSeconds = juce::jmin (newDiffusionTimeSeconds, (float) maxDiffusionSeconds);

        const auto diffusionTimeSamples = juce::roundToInt (fs * diffusionTimeSeconds);
        scratchBuffer.clear();
        BufferMath::copyBufferData (convolutionKernelBuffer,
                                    scratchBuffer,
                                    0,
                                    0,
                                    diffusionTimeSamples);

        const auto makeupGain = 32.0f / std::sqrt ((float) diffusionTimeSamples);
        BufferMath::applyGain (scratchBuffer, makeupGain);

        for (int ch = 0; ch < convolutionKernelBuffer.getNumChannels(); ++ch)
            convolutionEngines[(size_t) ch].setNewIR (scratchBuffer.getReadPointer (ch));
    }

    /** Processes a buffer */
    void processBlock (const BufferView<float>& buffer) noexcept
    {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            convolutionEngines[(size_t) ch].processSamples (buffer.getReadPointer (ch),
                                                            buffer.getWritePointer (ch),
                                                            (size_t) numSamples);
        }
    }

private:
    void generateConvolutionKernel (double sampleRate, int numChannels, int kernelSize)
    {
        convolutionKernelBuffer.setMaxSize (numChannels, kernelSize);
        for (auto [_, data] : buffer_iters::channels (convolutionKernelBuffer))
        {
            juce::Random rand;
            for (auto& sample : data)
                sample = rand.nextFloat() * 2.0f - 1.0f;
        }

        SmoothedBufferValue<float> kernelRamp;
        kernelRamp.prepare (sampleRate, kernelSize);
        kernelRamp.setRampLength (0.05); // Should the ramp length be a parameter in some way?
        kernelRamp.reset (0.0f);
        kernelRamp.process (1.0f, kernelSize);
        BufferMath::applyGainSmoothedBuffer (convolutionKernelBuffer, kernelRamp);
    }

    std::vector<ConvolutionEngine<>> convolutionEngines {};

    Buffer<float> convolutionKernelBuffer;
    Buffer<float> scratchBuffer;

    const double maxDiffusionSeconds;
    float fs = 48000.0f;
    float diffusionTimeSeconds = 0.1f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConvolutionDiffuser)
};
} // namespace chowdsp::Reverb
