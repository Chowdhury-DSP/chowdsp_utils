#include <TimedUnitTest.h>
#include <test_utils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

namespace
{
constexpr size_t fftOrder = 15;
constexpr size_t blockSize = 1 << fftOrder;
constexpr double _sampleRate = 48000.0;

// FFT smoothing constants
constexpr size_t avgNum = 3;
constexpr size_t negDiff = avgNum / 2;
constexpr size_t posDiff = negDiff + 1;
} // namespace

class UpsampleDownsampleTest : public TimedUnitTest
{
public:
    UpsampleDownsampleTest() : TimedUnitTest ("Upsample/Downsample Test")
    {
    }

    /** Returns frequency domain SNR in dB */
    static float calcSNR (const juce::dsp::AudioBlock<float>& buffer, float freqExpected, float fs)
    {
        juce::dsp::FFT fft (fftOrder);

        std::vector<float> fftData (blockSize * 2, 0.0f);
        juce::FloatVectorOperations::copy (fftData.data(), buffer.getChannelPointer (0), blockSize);
        fft.performFrequencyOnlyForwardTransform (fftData.data());

        std::vector<float> magnitudes (blockSize);
        auto dBNorm = 3.0f * fftOrder - 6.0f;
        auto scaleNorm = juce::Decibels::decibelsToGain<float> (dBNorm);
        for (size_t i = 0; i < blockSize; ++i)
            magnitudes[i] = std::pow (fftData[i] / scaleNorm, 2.0f);

        auto getMagForFreq = [=] (float freq) -> float {
            auto idx = size_t (((float) blockSize / 2.0f) * freq / (fs / 2.0f));
            // average over a few bins to smooth
            return std::accumulate (&magnitudes[idx - negDiff], &magnitudes[idx + posDiff], 0.0f) / (float) avgNum;
        };

        float noiseAccum = 0.0f;
        float freqInc = fs / (float) blockSize;
        for (float freq = 20.0f; freq < 0.66667f * freqExpected; freq += freqInc)
            noiseAccum += getMagForFreq (freq);

        float signalAccum = getMagForFreq (freqExpected);
        for (float freq = 1.5f * freqExpected; freq < fs * 0.48f; freq += freqInc)
            noiseAccum += getMagForFreq (freq);

        return juce::Decibels::gainToDecibels (signalAccum / noiseAccum);
    }

    template <typename ProcType>
    static juce::AudioBuffer<float> processInSubBlocks (ProcType& proc, juce::AudioBuffer<float>& inBuffer, int subBlockSize = blockSize / 8)
    {
        juce::AudioBuffer<float> outBuffer (inBuffer);
        int outBufferPtr = 0;

        juce::dsp::AudioBlock<const float> inBlock { inBuffer };
        for (int i = 0; i < inBuffer.getNumSamples(); i += subBlockSize)
        {
            auto outBlock = proc.process (chowdsp::BufferView<float> { inBuffer, i, subBlockSize });

            auto numOutSamples = (int) outBlock.getNumSamples();
            if (outBufferPtr + numOutSamples > outBuffer.getNumSamples())
                outBuffer.setSize (1, outBufferPtr + numOutSamples, true);

            outBuffer.copyFrom (0, outBufferPtr, outBlock.getReadPointer (0), numOutSamples);
            outBufferPtr += numOutSamples;
        }

        outBuffer.setSize (1, outBufferPtr, true);
        return outBuffer;
    }

    template <typename FilterType>
    void upsampleQualityTest (int upsampleRatio)
    {
        chowdsp::Upsampler<float, FilterType> upsampler;
        upsampler.prepare ({ _sampleRate, (juce::uint32) blockSize, 1 }, upsampleRatio);

        constexpr float testFreq = 10000.0f;
        auto sineBuffer = test_utils::makeSineWave (testFreq, (float) _sampleRate, (int) blockSize);
        auto upsampledBuffer = processInSubBlocks (upsampler, sineBuffer);
        auto upsampledBlock = juce::dsp::AudioBlock<float> (upsampledBuffer);
        auto snr = calcSNR (upsampledBlock, testFreq, (float) _sampleRate * (float) upsampler.getUpsamplingRatio());

        expectEquals (upsampledBuffer.getNumSamples(), (int) blockSize * upsampler.getUpsamplingRatio(), "Upsampled block size is incorrect!");
        expectGreaterThan (snr, 60.0f, "Signal to noise ratio is too low!");
    }

    template <typename FilterType>
    void downsampleQualityTest (int downsampleRatio)
    {
        chowdsp::Downsampler<float, FilterType> downsampler;
        downsampler.prepare ({ (double) downsampleRatio * _sampleRate, (juce::uint32) downsampleRatio * (juce::uint32) blockSize, 1 }, downsampleRatio);

        constexpr float testFreq = 42000.0f;
        auto thisBlockSize = downsampleRatio * (int) blockSize;
        auto sineBuffer = test_utils::makeSineWave (testFreq, (float) downsampleRatio * (float) _sampleRate, thisBlockSize);
        auto downsampledBuffer = processInSubBlocks (downsampler, sineBuffer, downsampleRatio * 256);

        float squaredSum = 0.0f;
        for (int n = 0; n < (int) blockSize / downsampleRatio; ++n)
            squaredSum += std::pow (downsampledBuffer.getSample (0, n), 2.0f);

        auto rms = juce::Decibels::gainToDecibels (std::sqrt (squaredSum / ((float) blockSize / 2.0f)));

        expectEquals (downsampledBuffer.getNumSamples(), thisBlockSize / downsampler.getDownsamplingRatio(), "Downsampled block size is incorrect!");
        expectLessThan (rms, -50.0f, "RMS level is too high!");
    }

    void runTestTimed() override
    {
        beginTest ("Upsample 2x Quality Test");
        upsampleQualityTest<chowdsp::ButterworthFilter<4>> (2);

        beginTest ("Upsample 3x Quality Test");
        upsampleQualityTest<chowdsp::ButterworthFilter<8>> (3);

        beginTest ("Upsample 3x Chebyshev Quality Test");
        upsampleQualityTest<chowdsp::ChebyshevIIFilter<12>> (3);

        beginTest ("Upsample 4x Elliptic Quality Test");
        upsampleQualityTest<chowdsp::EllipticFilter<12>> (4);

        beginTest ("Downsample 2x Quality Test");
        downsampleQualityTest<chowdsp::ButterworthFilter<4>> (2);

        beginTest ("Downsample 3x Quality Test");
        downsampleQualityTest<chowdsp::ButterworthFilter<16>> (3);

        beginTest ("Downsample 3x Chebyshev Quality Test");
        downsampleQualityTest<chowdsp::ChebyshevIIFilter<12>> (3);
        
        beginTest ("Downsample 4x Elliptic Quality Test");
        downsampleQualityTest<chowdsp::EllipticFilter<12>> (4);
    }
};

static UpsampleDownsampleTest upsampleDownsampleTest;
