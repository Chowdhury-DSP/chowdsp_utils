#pragma once

namespace chowdsp
{
/**
 * Base class for doing Constant Overlap-Add processing with a given window.
 *
 * Mostly borrowed from the implementation by Daniel Rudrich under the GPL license.
 * https://github.com/DanielRudrich/OverlappingFFTProcessor/blob/master/Source/OverlappingFFTProcessor.h
 */
template <typename FloatType, typename juce::dsp::WindowingFunction<FloatType>::WindowingMethod Window = juce::dsp::WindowingFunction<FloatType>::hann>
class COLAProcessor
{
public:
    /**
     * Constructor
     * @param fftSizeAsPowerOf2 defines the fftSize as a power of 2: fftSize = 2^fftSizeAsPowerOf2
     * @param hopSizeDividerAsPowerOf2 defines the hopSize as a fraction of fftSize: hopSize = fftSize / (2^hopSizeDivider)
     */
    explicit COLAProcessor (const int fftSizeAsPowerOf2, const int hopSizeDividerAsPowerOf2 = 1);

    virtual ~COLAProcessor() = default;

    /** Prepares the processor to process a new audio stream. */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Reset's the state of the processor. */
    void reset();

    /** Process a block of audio */
    void processBlock (juce::AudioBuffer<FloatType>& buffer);

protected:
    /** Override this method to handle internal reset behaviour. */
    virtual void resetProcessor() {}

    /** Override this method to handle internal preparation behaviour. */
    virtual void prepareProcessor (const juce::dsp::ProcessSpec&) {}

    /**
     * This method gets called each time the processor has gathered enough samples for a transformation.
     * The data in the `frameBuffer` is still in time domain.
     */
    virtual void processFrame (juce::AudioBuffer<FloatType>& /*frame*/) {}

private:
    void createWindow();
    void writeBackFrame (int numChannels);

    std::vector<FloatType> window;
    juce::AudioBuffer<FloatType> frameBuffer;
    const int fftSize;
    const int hopSize;

    juce::AudioBuffer<FloatType> notYetUsedAudioData;
    juce::AudioBuffer<FloatType> outputBuffer;

    int outputOffset = 0;
    int notYetUsedAudioDataCount = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (COLAProcessor)
};

} // namespace chowdsp
