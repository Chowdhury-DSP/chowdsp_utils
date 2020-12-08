#include <algorithm>
#include <JuceHeader.h>

namespace
{
    // processing constants
    constexpr size_t fftOrder = 20;
    constexpr size_t blockSize = 1 << fftOrder;
    constexpr size_t nCh = 1;
    constexpr double fs = 48000.0;

    // measuring constants
    constexpr float startFreq = 100.0f;
    constexpr size_t avgNum = 101;
    constexpr size_t negDiff = avgNum / 2;
    constexpr size_t posDiff = negDiff + 1;
}

/** Unit tests for chowdsp::Noise. Testing frequency domain accuracy for:
 *   - Uniform white noise (flat frequency spectrum)
 *   - Gaussian (Normal) white noise (flat frequency spectrum)
 *   - Pink noise (-3dB / Oct)
 */
class NoiseTest : public UnitTest
{
public:
    NoiseTest() : UnitTest ("Noise Test") {}

    /** Check the frequency slope of a noise buffer, slope measured in dB/Oct */
    void checkNoiseSlope (const AudioBuffer<float>& buffer, String noiseType, float noiseSlope = 0.0f)
    {
        dsp::FFT fft (fftOrder);

        std::vector<float> fftData (blockSize * 2, 0.0f);
        FloatVectorOperations::copy (fftData.data(), buffer.getReadPointer (0), blockSize);
        fft.performFrequencyOnlyForwardTransform (fftData.data());

        std::vector<float> magnitudes (blockSize);
        auto dBNorm = 3.0f * fftOrder - 6.0f;
        for (size_t i = 0; i < blockSize; ++i)
            magnitudes[i] = Decibels::gainToDecibels<float> (std::abs (fftData[i])) - dBNorm;

        auto getMagForFreq = [=] (float freq) -> float
        {
            auto idx = size_t ((blockSize / 2) * freq / (fs / 2.0f));
            // average over many bins to smooth
            return std::accumulate (&magnitudes[idx-negDiff], &magnitudes[idx + posDiff], 0.0f) / (float) avgNum;
        };

        auto freq = startFreq;
        auto expMag = 0.0f;
        while (freq < fs / 2)
        {
            auto mag = getMagForFreq (freq);
            String errorMsg = noiseType + " error at frequency " + String (freq, 2) + " Hz";
            expectWithinAbsoluteError (mag, expMag, 3.0f, errorMsg);
            
            expMag += noiseSlope;
            freq *= 2;
        }
    }

    String getNoiseNameForType (chowdsp::Noise<float>::NoiseType type)
    {
        if (type == chowdsp::Noise<float>::NoiseType::Uniform)
            return "Uniform White Noise";

        if (type == chowdsp::Noise<float>::NoiseType::Normal)
            return "Normal White Noise";

        if (type == chowdsp::Noise<float>::NoiseType::Pink)
            return "Pink Noise";

        jassertfalse;
        return String();
    }

    void runNoiseTest (chowdsp::Noise<float>::NoiseType type, float noiseSlope = 0.0f)
    {
        chowdsp::Noise<float> noise;

        dsp::ProcessSpec spec { fs, blockSize, nCh };
        noise.prepare (spec);

        noise.setGainLinear (1.0f);
        noise.setNoiseType (type);

        AudioBuffer<float> noiseBuff (nCh, blockSize);
        dsp::AudioBlock<float> block (noiseBuff);
        block.clear();
        dsp::ProcessContextReplacing<float> ctx (block);
        noise.process (ctx);

        checkNoiseSlope (noiseBuff, getNoiseNameForType (type), noiseSlope);
    }

    void runTest() override
    {
        beginTest ("Uniform White Noise Test");
        runNoiseTest (chowdsp::Noise<float>::NoiseType::Uniform);

        beginTest ("Normal White Noise Test");
        runNoiseTest (chowdsp::Noise<float>::NoiseType::Normal);

        beginTest ("Pink Noise Test");
        runNoiseTest (chowdsp::Noise<float>::NoiseType::Pink, -3.0f);
    }
};

static NoiseTest noiseTest;
