#include <TimedUnitTest.h>
#include <test_utils.h>

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
    static float calcSNR (const dsp::AudioBlock<float>& buffer, float freqExpected, float fs)
    {
        dsp::FFT fft (fftOrder);

        std::vector<float> fftData (blockSize * 2, 0.0f);
        FloatVectorOperations::copy (fftData.data(), buffer.getChannelPointer (0), blockSize);
        fft.performFrequencyOnlyForwardTransform (fftData.data());

        std::vector<float> magnitudes (blockSize);
        auto dBNorm = 3.0f * fftOrder - 6.0f;
        auto scaleNorm = Decibels::decibelsToGain<float> (dBNorm);
        for (size_t i = 0; i < blockSize; ++i)
            magnitudes[i] = std::pow (fftData[i] / scaleNorm, 2.0f);

        auto getMagForFreq = [=] (float freq) -> float
        {
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

        return Decibels::gainToDecibels (signalAccum / noiseAccum);
    }

    void upsampleQualityTest()
    {
        chowdsp::Upsampler<float> upsampler;
        upsampler.prepare ({ _sampleRate, (uint32) blockSize, 1 }, 2);

        constexpr float testFreq = 10000.0f;
        auto sineBuffer = test_utils::makeSineWave (testFreq, (float) _sampleRate, (int) blockSize);
        auto upsampledBlock = upsampler.process (dsp::AudioBlock<float> (sineBuffer));
        auto snr = calcSNR (upsampledBlock, testFreq, (float) _sampleRate * (float) upsampler.getUpsamplingRatio());

        expectEquals (upsampledBlock.getNumSamples(), blockSize * (size_t) upsampler.getUpsamplingRatio(), "Upsampled block size is incorrect!");
        expectGreaterThan (snr, 60.0f, "Signal to noise ratio is too low!");
    }

    void downsampleQualityTest()
    {
        chowdsp::Downsampler<float, 8> downsampler;
        downsampler.prepare ({ 2.0 * _sampleRate, (uint32) blockSize, 1 }, 2);

        constexpr float testFreq = 42000.0f;
        auto sineBuffer = test_utils::makeSineWave (testFreq, 2.0f * (float) _sampleRate, (int) blockSize);
        auto downsampledBlock = downsampler.process (dsp::AudioBlock<float> (sineBuffer));

        float squaredSum = 0.0f;
        for (size_t n = 0; n < blockSize / 2; ++n)
            squaredSum += std::pow (downsampledBlock.getSample (0, (int) n), 2.0f);

        auto rms = Decibels::gainToDecibels (std::sqrt (squaredSum / ((float) blockSize / 2.0f)));

        expectEquals (downsampledBlock.getNumSamples(), blockSize / (size_t) downsampler.getDownsamplingRatio(), "Downsampled block size is incorrect!");
        expectLessThan (rms, -50.0f, "RMS level is too high!");
    }

    void runTestTimed() override
    {
        beginTest ("Upsample Quality Test");
        upsampleQualityTest();

        beginTest ("Downsample Quality Test");
        downsampleQualityTest();
    }
};

static UpsampleDownsampleTest upsampleDownsampleTest;
