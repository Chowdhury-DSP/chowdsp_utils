#pragma once

namespace chowdsp
{
/** A stripped-back Convolution engine based on juce::dsp::Convolution
 *  This implementation is specifically meant to be used for linear-phase
 *  filters, where the filter might change, but the IR size remains constant.
 * 
 *  To change the filter smoothly in real-time, it is recommended to use
 *  the IRTransfer class so the IR can be computed on a separate thread:
 *  ```
 *  std::unique_ptr<ConvolutionEngine> engine[2]; // stereo convolution
 *  std::unique_ptr<IRTransfer> irTransfer;
 *
 *  // in prepareToPlay()
 *  for (auto& eng : engine)
 *      eng = std::make_unique<ConvolutionEngine> (initialIR, irSize, blockSize);
 *
 *  irTransfer = std::make_unique<IRTransfer> (*engine[0]);
 *  
 *  // when a new IR is ready (on a separate thread)
 *  irTransfer->setNewIR (newIRBuffer);
 * 
 *  // in audio callback:
 *  { // if IR update is needed...
 *      SpinLock::ScopedTryLockType lock (irTransfer->mutex);
 *      if (lock.isLocked())
 *      {
 *          for (auto& eng : engine)
 *              irTransfer->transferIR (*eng);
 *      }
 * }
 *
 *  for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
 *  {
 *      engine[ch]->processSamplesWithAddedLatency (block.getChannelPointer (ch),
 *                                                  block.getChannelPointer (ch),
 *                                                  block.getNumSamples());
 *  }
 *  ```
 */
struct ConvolutionEngine
{
    /** Creates a new convolution engine for a given IR, note that while future IRs
        may be loaded into this engine, the IR size MUST stay the same.
     */
    ConvolutionEngine (const float* samples, size_t numSamples, size_t maxBlockSize);

    // resets the state of this convolution
    void reset();

    // sets these samples as the new IR
    void setNewIR (const float* newIR);

    // process samples with zero latency
    void processSamples (const float* input, float* output, size_t numSamples);

    // processes samples with (around) a block size of latency
    void processSamplesWithAddedLatency (const float* input, float* output, size_t numSamples);

    // updates the segments of this convolution
    static void updateSegmentsIfNecessary (size_t numSegmentsToUpdate, std::vector<juce::AudioBuffer<float>>& segments, size_t fftSize);

    // After each FFT, this function is called to allow convolution to be performed with only 4 SIMD functions calls.
    static void prepareForConvolution (float* samples, size_t fftSize) noexcept;

    // Does the convolution operation itself only on half of the frequency domain samples.
    void convolutionProcessingAndAccumulate (const float* input, const float* impulse, float* output);

    // Undoes the re-organization of samples from the function prepareForConvolution.
    // Then takes the conjugate of the frequency domain first half of samples to fill the
    // second half, so that the inverse transform will return real samples in the time domain.
    void updateSymmetricFrequencyDomainData (float* samples) noexcept;

    //==============================================================================
    const size_t irNumSamples;
    const size_t blockSize;
    const size_t fftSize;
    const std::unique_ptr<juce::dsp::FFT> fftObject;
    const size_t numSegments;
    const size_t numInputSegments;
    size_t currentSegment = 0, inputDataPos = 0;

    juce::AudioBuffer<float> bufferInput, bufferOutput, bufferTempOutput, bufferOverlap;
    std::vector<juce::AudioBuffer<float>> buffersInputSegments, buffersImpulseSegments;
};

} // namespace chowdsp
