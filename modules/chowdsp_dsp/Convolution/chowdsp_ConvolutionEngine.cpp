#include "chowdsp_ConvolutionEngine.h"

namespace chowdsp
{
ConvolutionEngine::ConvolutionEngine (const float* samples,
                                      size_t numSamples,
                                      size_t maxBlockSize)
    : irNumSamples (numSamples),
      blockSize ((size_t) juce::nextPowerOfTwo ((int) maxBlockSize)),
      fftSize (blockSize > 128 ? 2 * blockSize : 4 * blockSize),
      fftObject (std::make_unique<juce::dsp::FFT> (juce::roundToInt (std::log2 (fftSize)))),
      numSegments (numSamples / (fftSize - blockSize) + 1u),
      numInputSegments ((blockSize > 128 ? numSegments : 3 * numSegments)),
      bufferInput (1, static_cast<int> (fftSize)),
      bufferOutput (1, static_cast<int> (fftSize * 2)),
      bufferTempOutput (1, static_cast<int> (fftSize * 2)),
      bufferOverlap (1, static_cast<int> (fftSize))
{
    bufferOutput.clear();

    updateSegmentsIfNecessary (numInputSegments, buffersInputSegments, fftSize);
    updateSegmentsIfNecessary (numSegments, buffersImpulseSegments, fftSize);

    setNewIR (samples);

    reset();
}

void ConvolutionEngine::updateSegmentsIfNecessary (size_t numSegmentsToUpdate, std::vector<juce::AudioBuffer<float>>& segments, size_t fftSize)
{
    if (numSegmentsToUpdate == 0
        || numSegmentsToUpdate != (size_t) segments.size()
        || (size_t) segments[0].getNumSamples() != fftSize * 2)
    {
        segments.clear();

        for (size_t i = 0; i < numSegmentsToUpdate; ++i)
            segments.emplace_back (1, static_cast<int> (fftSize * 2));
    }
}

void ConvolutionEngine::reset()
{
    bufferInput.clear();
    bufferOverlap.clear();
    bufferTempOutput.clear();
    bufferOutput.clear();

    for (auto& buf : buffersInputSegments)
        buf.clear();

    currentSegment = 0;
    inputDataPos = 0;
}

void ConvolutionEngine::setNewIR (const float* newIR)
{
    size_t currentPtr = 0;
    for (auto& buf : buffersImpulseSegments)
    {
        buf.clear();
        auto* impulseResponse = buf.getWritePointer (0);

        if (&buf == &buffersImpulseSegments.front())
            impulseResponse[0] = 1.0f;

        juce::FloatVectorOperations::copy (impulseResponse,
                                           newIR + currentPtr,
                                           static_cast<int> (juce::jmin (fftSize - blockSize, irNumSamples - currentPtr)));

        fftObject->performRealOnlyForwardTransform (impulseResponse);
        prepareForConvolution (impulseResponse, fftSize);

        currentPtr += (fftSize - blockSize);
    }
}

void ConvolutionEngine::processSamples (const float* input, float* output, size_t numSamples)
{
    // Overlap-add, zero latency convolution algorithm with uniform partitioning
    size_t numSamplesProcessed = 0;

    auto indexStep = numInputSegments / numSegments;

    auto* inputData = bufferInput.getWritePointer (0);
    auto* outputTempData = bufferTempOutput.getWritePointer (0);
    auto* outputData = bufferOutput.getWritePointer (0);
    auto* overlapData = bufferOverlap.getWritePointer (0);

    while (numSamplesProcessed < numSamples)
    {
        const bool inputDataWasEmpty = (inputDataPos == 0);
        auto numSamplesToProcess = juce::jmin (numSamples - numSamplesProcessed, blockSize - inputDataPos);

        juce::FloatVectorOperations::copy (inputData + inputDataPos, input + numSamplesProcessed, static_cast<int> (numSamplesToProcess));

        auto* inputSegmentData = buffersInputSegments[currentSegment].getWritePointer (0);
        juce::FloatVectorOperations::copy (inputSegmentData, inputData, static_cast<int> (fftSize));

        fftObject->performRealOnlyForwardTransform (inputSegmentData);
        prepareForConvolution (inputSegmentData, fftSize);

        // Complex multiplication
        if (inputDataWasEmpty)
        {
            juce::FloatVectorOperations::fill (outputTempData, 0, static_cast<int> (fftSize + 1));

            auto index = currentSegment;

            for (size_t i = 1; i < numSegments; ++i)
            {
                index += indexStep;

                if (index >= numInputSegments)
                    index -= numInputSegments;

                convolutionProcessingAndAccumulate (buffersInputSegments[index].getWritePointer (0),
                                                    buffersImpulseSegments[i].getWritePointer (0),
                                                    outputTempData);
            }
        }

        juce::FloatVectorOperations::copy (outputData, outputTempData, static_cast<int> (fftSize + 1));

        convolutionProcessingAndAccumulate (inputSegmentData,
                                            buffersImpulseSegments.front().getWritePointer (0),
                                            outputData);

        updateSymmetricFrequencyDomainData (outputData);
        fftObject->performRealOnlyInverseTransform (outputData);

        // Add overlap
        juce::FloatVectorOperations::add (&output[numSamplesProcessed], &outputData[inputDataPos], &overlapData[inputDataPos], (int) numSamplesToProcess);

        // Input buffer full => Next block
        inputDataPos += numSamplesToProcess;

        if (inputDataPos == blockSize)
        {
            // Input buffer is empty again now
            juce::FloatVectorOperations::fill (inputData, 0.0f, static_cast<int> (fftSize));

            inputDataPos = 0;

            // Extra step for segSize > blockSize
            juce::FloatVectorOperations::add (&(outputData[blockSize]), &(overlapData[blockSize]), static_cast<int> (fftSize - 2 * blockSize));

            // Save the overlap
            juce::FloatVectorOperations::copy (overlapData, &(outputData[blockSize]), static_cast<int> (fftSize - blockSize));

            currentSegment = (currentSegment > 0) ? (currentSegment - 1) : (numInputSegments - 1);
        }

        numSamplesProcessed += numSamplesToProcess;
    }
}

void ConvolutionEngine::processSamplesWithAddedLatency (const float* input, float* output, size_t numSamples)
{
    // Overlap-add, zero latency convolution algorithm with uniform partitioning
    size_t numSamplesProcessed = 0;

    auto indexStep = numInputSegments / numSegments;

    auto* inputData = bufferInput.getWritePointer (0);
    auto* outputTempData = bufferTempOutput.getWritePointer (0);
    auto* outputData = bufferOutput.getWritePointer (0);
    auto* overlapData = bufferOverlap.getWritePointer (0);

    while (numSamplesProcessed < numSamples)
    {
        auto numSamplesToProcess = juce::jmin (numSamples - numSamplesProcessed, blockSize - inputDataPos);

        juce::FloatVectorOperations::copy (inputData + inputDataPos, input + numSamplesProcessed, static_cast<int> (numSamplesToProcess));

        juce::FloatVectorOperations::copy (output + numSamplesProcessed, outputData + inputDataPos, static_cast<int> (numSamplesToProcess));

        numSamplesProcessed += numSamplesToProcess;
        inputDataPos += numSamplesToProcess;

        // processing itself when needed (with latency)
        if (inputDataPos == blockSize)
        {
            // Copy input data in input segment
            auto* inputSegmentData = buffersInputSegments[currentSegment].getWritePointer (0);
            juce::FloatVectorOperations::copy (inputSegmentData, inputData, static_cast<int> (fftSize));

            fftObject->performRealOnlyForwardTransform (inputSegmentData);
            prepareForConvolution (inputSegmentData, fftSize);

            // Complex multiplication
            juce::FloatVectorOperations::fill (outputTempData, 0, static_cast<int> (fftSize + 1));

            auto index = currentSegment;

            for (size_t i = 1; i < numSegments; ++i)
            {
                index += indexStep;

                if (index >= numInputSegments)
                    index -= numInputSegments;

                convolutionProcessingAndAccumulate (buffersInputSegments[index].getWritePointer (0),
                                                    buffersImpulseSegments[i].getWritePointer (0),
                                                    outputTempData);
            }

            juce::FloatVectorOperations::copy (outputData, outputTempData, static_cast<int> (fftSize + 1));

            convolutionProcessingAndAccumulate (inputSegmentData,
                                                buffersImpulseSegments.front().getWritePointer (0),
                                                outputData);

            updateSymmetricFrequencyDomainData (outputData);
            fftObject->performRealOnlyInverseTransform (outputData);

            // Add overlap
            juce::FloatVectorOperations::add (outputData, overlapData, static_cast<int> (blockSize));

            // Input buffer is empty again now
            juce::FloatVectorOperations::fill (inputData, 0.0f, static_cast<int> (fftSize));

            // Extra step for segSize > blockSize
            juce::FloatVectorOperations::add (&(outputData[blockSize]), &(overlapData[blockSize]), static_cast<int> (fftSize - 2 * blockSize));

            // Save the overlap
            juce::FloatVectorOperations::copy (overlapData, &(outputData[blockSize]), static_cast<int> (fftSize - blockSize));

            currentSegment = (currentSegment > 0) ? (currentSegment - 1) : (numInputSegments - 1);

            inputDataPos = 0;
        }
    }
}

void ConvolutionEngine::prepareForConvolution (float* samples, size_t fftSize) noexcept
{
    auto FFTSizeDiv2 = fftSize / 2;

    for (size_t i = 0; i < FFTSizeDiv2; i++)
        samples[i] = samples[i << 1];

    samples[FFTSizeDiv2] = 0;

    for (size_t i = 1; i < FFTSizeDiv2; i++)
        samples[i + FFTSizeDiv2] = -samples[((fftSize - i) << 1) + 1];
}

void ConvolutionEngine::convolutionProcessingAndAccumulate (const float* input, const float* impulse, float* output) const
{
    auto FFTSizeDiv2 = fftSize / 2;

    juce::FloatVectorOperations::addWithMultiply (output, input, impulse, static_cast<int> (FFTSizeDiv2));
    juce::FloatVectorOperations::subtractWithMultiply (output, &(input[FFTSizeDiv2]), &(impulse[FFTSizeDiv2]), static_cast<int> (FFTSizeDiv2));

    juce::FloatVectorOperations::addWithMultiply (&(output[FFTSizeDiv2]), input, &(impulse[FFTSizeDiv2]), static_cast<int> (FFTSizeDiv2));
    juce::FloatVectorOperations::addWithMultiply (&(output[FFTSizeDiv2]), &(input[FFTSizeDiv2]), impulse, static_cast<int> (FFTSizeDiv2));

    output[fftSize] += input[fftSize] * impulse[fftSize];
}

void ConvolutionEngine::updateSymmetricFrequencyDomainData (float* samples) const noexcept
{
    auto FFTSizeDiv2 = fftSize / 2;

    for (size_t i = 1; i < FFTSizeDiv2; i++)
    {
        samples[(fftSize - i) << 1] = samples[i];
        samples[((fftSize - i) << 1) + 1] = -samples[FFTSizeDiv2 + i];
    }

    samples[1] = 0.f;

    for (size_t i = 1; i < FFTSizeDiv2; i++)
    {
        samples[i << 1] = samples[(fftSize - i) << 1];
        samples[(i << 1) + 1] = -samples[((fftSize - i) << 1) + 1];
    }
}

} // namespace chowdsp
