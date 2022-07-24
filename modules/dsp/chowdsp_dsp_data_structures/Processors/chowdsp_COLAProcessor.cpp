#include "chowdsp_COLAProcessor.h"

namespace chowdsp
{
template <typename FloatType, typename juce::dsp::WindowingFunction<FloatType>::WindowingMethod Window>
COLAProcessor<FloatType, Window>::COLAProcessor (const int fftSizeAsPowerOf2, const int hopSizeDividerAsPowerOf2) : fftSize (1 << fftSizeAsPowerOf2),
                                                                                                                    hopSize (fftSize >> hopSizeDividerAsPowerOf2)
{
}

template <typename FloatType, typename juce::dsp::WindowingFunction<FloatType>::WindowingMethod Window>
void COLAProcessor<FloatType, Window>::prepare (const juce::dsp::ProcessSpec& spec)
{
    // create window
    window = std::vector<FloatType> ((size_t) fftSize, (FloatType) 0);
    createWindow();

    const auto bufferSize = (int) spec.maximumBlockSize;

    notYetUsedAudioData.setSize ((int) spec.numChannels, fftSize - 1);
    frameBuffer.setSize ((int) spec.numChannels, fftSize);

    const auto k = (int) std::floor (1.0f + (FloatType (bufferSize - 1)) / (FloatType) hopSize);
    const int M = k * hopSize + (fftSize - hopSize);

    outputBuffer.setSize ((int) spec.numChannels, M + bufferSize - 1);
    outputOffset = fftSize - 1;
    notYetUsedAudioDataCount = 0;

    prepareProcessor ({ spec.sampleRate, (juce::uint32) fftSize, spec.numChannels });
}

template <typename FloatType, typename juce::dsp::WindowingFunction<FloatType>::WindowingMethod Window>
void COLAProcessor<FloatType, Window>::reset()
{
    notYetUsedAudioData.clear();
    frameBuffer.clear();
    outputBuffer.clear();

    outputOffset = fftSize - 1;
    notYetUsedAudioDataCount = 0;

    resetProcessor();
}

template <typename FloatType, typename juce::dsp::WindowingFunction<FloatType>::WindowingMethod Window>
void COLAProcessor<FloatType, Window>::processBlock (juce::AudioBuffer<FloatType>& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();
    const auto L = numSamples;

    const int initialNotYetUsedAudioDataCount = notYetUsedAudioDataCount;
    int notYetUsedAudioDataOffset = 0;
    int usedSamples = 0;

    // we've got some left overs, so let's use them together with the new samples
    while (notYetUsedAudioDataCount > 0 && notYetUsedAudioDataCount + L >= fftSize)
    {
        // copy not yet used data into fftInOut buffer (with hann windowing)
        for (int ch = 0; ch < numChannels; ++ch)
        {
            juce::FloatVectorOperations::multiply (frameBuffer.getWritePointer (ch),
                                                   notYetUsedAudioData.getReadPointer (ch, notYetUsedAudioDataOffset),
                                                   window.data(),
                                                   notYetUsedAudioDataCount);

            // fill up fftInOut buffer with new data (with hann windowing)
            juce::FloatVectorOperations::multiply (frameBuffer.getWritePointer (ch, notYetUsedAudioDataCount),
                                                   buffer.getReadPointer (ch),
                                                   window.data() + notYetUsedAudioDataCount,
                                                   fftSize - notYetUsedAudioDataCount);
        }

        // process frame and buffer output
        processFrame (frameBuffer);
        writeBackFrame (numChannels);

        notYetUsedAudioDataOffset += hopSize;
        notYetUsedAudioDataCount -= hopSize;
    }

    if (notYetUsedAudioDataCount > 0) // not enough new input samples to use all of the previous data
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            juce::FloatVectorOperations::copy (notYetUsedAudioData.getWritePointer (ch),
                                               notYetUsedAudioData.getReadPointer (ch, initialNotYetUsedAudioDataCount - notYetUsedAudioDataCount),
                                               notYetUsedAudioDataCount);
            juce::FloatVectorOperations::copy (notYetUsedAudioData.getWritePointer (ch, notYetUsedAudioDataCount),
                                               buffer.getReadPointer (ch, usedSamples),
                                               L);
        }
        notYetUsedAudioDataCount += L;
    }
    else // all of the previous data used
    {
        int dataOffset = -notYetUsedAudioDataCount;

        while (L - dataOffset >= fftSize)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                juce::FloatVectorOperations::multiply (frameBuffer.getWritePointer (ch),
                                                       buffer.getReadPointer (ch, dataOffset),
                                                       window.data(),
                                                       fftSize);
            }
            processFrame (frameBuffer);
            writeBackFrame (numChannels);

            dataOffset += hopSize;
        }

        int remainingSamples = L - dataOffset;
        if (remainingSamples > 0)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                juce::FloatVectorOperations::copy (notYetUsedAudioData.getWritePointer (ch),
                                                   buffer.getReadPointer (ch, dataOffset),
                                                   L - dataOffset);
            }
        }
        notYetUsedAudioDataCount = remainingSamples;
    }

    // return processed samples from outputBuffer
    const int shiftStart = L;
    int shiftL = outputOffset + fftSize - hopSize - L;

    const int tooMuch = shiftStart + shiftL - outputBuffer.getNumSamples();
    if (tooMuch > 0)
        shiftL -= tooMuch;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        juce::FloatVectorOperations::copy (buffer.getWritePointer (ch), outputBuffer.getReadPointer (ch), L);
        juce::FloatVectorOperations::copy (outputBuffer.getWritePointer (ch), outputBuffer.getReadPointer (ch, shiftStart), shiftL);
    }

    outputOffset -= L;
}

template <typename FloatType, typename juce::dsp::WindowingFunction<FloatType>::WindowingMethod Window>
void COLAProcessor<FloatType, Window>::createWindow()
{
    juce::dsp::WindowingFunction<FloatType>::fillWindowingTables (window.data(), (size_t) fftSize, Window, false);

    const auto hopSizeCompensateFactor = (FloatType) 1 / ((FloatType) fftSize / (FloatType) hopSize / (FloatType) 2);
    juce::FloatVectorOperations::multiply (window.data(), hopSizeCompensateFactor, fftSize);
}

template <typename FloatType, typename juce::dsp::WindowingFunction<FloatType>::WindowingMethod Window>
void COLAProcessor<FloatType, Window>::writeBackFrame (int numChannels)
{
    for (int ch = 0; ch < numChannels; ++ch)
    {
        outputBuffer.addFrom (ch, outputOffset, frameBuffer, ch, 0, fftSize - hopSize);
        outputBuffer.copyFrom (ch, outputOffset + fftSize - hopSize, frameBuffer, ch, fftSize - hopSize, hopSize);
    }
    outputOffset += hopSize;
}

//==============================================================================
template class COLAProcessor<float>;
template class COLAProcessor<float, juce::dsp::WindowingFunction<float>::triangular>;
template class COLAProcessor<float, juce::dsp::WindowingFunction<float>::hamming>;
template class COLAProcessor<float, juce::dsp::WindowingFunction<float>::blackman>;
template class COLAProcessor<float, juce::dsp::WindowingFunction<float>::blackmanHarris>;
template class COLAProcessor<double>;
template class COLAProcessor<double, juce::dsp::WindowingFunction<double>::triangular>;
template class COLAProcessor<double, juce::dsp::WindowingFunction<double>::hamming>;
template class COLAProcessor<double, juce::dsp::WindowingFunction<double>::blackman>;
template class COLAProcessor<double, juce::dsp::WindowingFunction<double>::blackmanHarris>;

} // namespace chowdsp
