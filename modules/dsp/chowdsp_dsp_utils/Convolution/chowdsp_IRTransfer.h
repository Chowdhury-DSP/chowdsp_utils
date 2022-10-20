#pragma once

#include "chowdsp_ConvolutionEngine.h"

namespace chowdsp
{
/** A utility class to help smoothly transfer a new IR
 *  into a ConvolutionEngine in a thread-safe manner.
 * 
 *  Note that the the size of every IR transferred via
 *  via this class MUST be the same size as the IR in
 *  ConvolutionEngine that was passed to the constructor.
 */
struct IRTransfer
{
    // create a new IRTransfer object for a given convolution engine
    explicit IRTransfer (const ConvolutionEngine<>& eng) : fftSize (eng.fftSize),
                                                           blockSize (eng.blockSize),
                                                           irNumSamples (eng.irNumSamples),
                                                           irFFT (std::make_unique<juce::dsp::FFT> (Math::log2 (fftSize)))
    {
        ConvolutionEngine<>::updateSegmentsIfNecessary (eng.numSegments, buffersImpulseSegments, fftSize);
    }

    // loads a new IR to be ready for transferring
    void setNewIR (const float* newIR)
    {
        juce::SpinLock::ScopedLockType lock (mutex);

        size_t currentPtr = 0;
        for (auto& buf : buffersImpulseSegments)
        {
            buf.clear();
            auto* impulseResponse = buf.getWritePointer (0);

            juce::FloatVectorOperations::copy (impulseResponse,
                                               newIR + currentPtr,
                                               static_cast<int> (juce::jmin (fftSize - blockSize, irNumSamples - currentPtr)));

            irFFT->performRealOnlyForwardTransform (impulseResponse);
            ConvolutionEngine<>::prepareForConvolution (impulseResponse, fftSize);

            currentPtr += (fftSize - blockSize);
        }
    }

    // transfers the loaded IR to a convolution engine
    void transferIR (ConvolutionEngine<>& engine) const
    {
        for (size_t i = 0; i < buffersImpulseSegments.size(); ++i)
        {
            auto& sourceBuffer = buffersImpulseSegments[i];
            auto& destBuffer = engine.buffersImpulseSegments[i];

            juce::FloatVectorOperations::copy (destBuffer.getWritePointer (0),
                                               sourceBuffer.getReadPointer (0),
                                               sourceBuffer.getNumSamples());
        }
    }

    const size_t fftSize;
    const size_t blockSize;
    const size_t irNumSamples;
    const std::unique_ptr<juce::dsp::FFT> irFFT;
    std::vector<juce::AudioBuffer<float>> buffersImpulseSegments;
    juce::SpinLock mutex;
};

} // namespace chowdsp
