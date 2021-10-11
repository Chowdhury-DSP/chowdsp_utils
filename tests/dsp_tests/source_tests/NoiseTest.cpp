#include <JuceHeader.h>
#include <algorithm>

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

using vec4 = dsp::SIMDRegister<float>;
} // namespace

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
    void checkNoiseSlope (const float* buffer, String noiseType, float noiseSlope = 0.0f, float maxErr = 3.0f)
    {
        dsp::FFT fft (fftOrder);

        std::vector<float> fftData (blockSize * 2, 0.0f);
        FloatVectorOperations::copy (fftData.data(), buffer, blockSize);
        fft.performFrequencyOnlyForwardTransform (fftData.data());

        std::vector<float> magnitudes (blockSize);
        auto dBNorm = 3.0f * fftOrder - 6.0f;
        for (size_t i = 0; i < blockSize; ++i)
            magnitudes[i] = Decibels::gainToDecibels<float> (std::abs (fftData[i])) - dBNorm;

        auto getMagForFreq = [=] (float freq) -> float
        {
            auto idx = size_t ((blockSize / 2) * freq / (fs / 2.0f));
            // average over many bins to smooth
            return std::accumulate (&magnitudes[idx - negDiff], &magnitudes[idx + posDiff], 0.0f) / (float) avgNum;
        };

        auto freq = startFreq;
        auto expMag = 0.0f;
        while (freq < fs / 2)
        {
            auto mag = getMagForFreq (freq);
            String errorMsg = noiseType + " error at frequency " + String (freq, 2) + " Hz";
            expectWithinAbsoluteError (mag, expMag, maxErr, errorMsg);

            expMag += noiseSlope;
            freq *= 2;
        }
    }

    template <typename FloatType>
    String getNoiseNameForType (typename chowdsp::Noise<FloatType>::NoiseType type)
    {
        if (type == chowdsp::Noise<FloatType>::NoiseType::Uniform)
            return "Uniform White Noise";

        if (type == chowdsp::Noise<FloatType>::NoiseType::Normal)
            return "Normal White Noise";

        if (type == chowdsp::Noise<FloatType>::NoiseType::Pink)
            return "Pink Noise";

        jassertfalse;
        return String();
    }

    std::vector<std::vector<float>> getTestVector (const dsp::AudioBlock<float> data)
    {
        auto* dataPtr = data.getChannelPointer (0);
        return { { dataPtr, dataPtr + data.getNumSamples() } };
    }

    std::vector<std::vector<float>> getTestVector (const dsp::AudioBlock<vec4> data)
    {
        const auto numSamples = data.getNumSamples();
        std::vector<std::vector<float>> vecs (4, std::vector<float> (numSamples, 0.0f));

        for (size_t n = 0; n < numSamples; ++n)
            for (size_t ch = 0; ch < 4; ++ch)
                vecs[ch][n] = data.getSample (0, (int) n).get (ch);

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpessimizing-move")
        return std::move (vecs);
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    template <typename FloatType>
    void runNoiseTest (typename chowdsp::Noise<FloatType>::NoiseType type, float noiseSlope = 0.0f, float maxErr = 3.0f)
    {
        chowdsp::Noise<FloatType> noise;

        dsp::ProcessSpec spec { fs, blockSize, nCh };
        noise.prepare (spec);

        noise.setGainLinear (1.0f);
        noise.setNoiseType (type);

        HeapBlock<char> blockData;
        auto block = dsp::AudioBlock<FloatType> (blockData, (size_t) nCh, (size_t) blockSize);
        block.clear();
        dsp::ProcessContextReplacing<FloatType> ctx (block);
        noise.process (ctx);

        auto testVectors = getTestVector (block);
        for (const auto& vec : testVectors)
            checkNoiseSlope (vec.data(), getNoiseNameForType<FloatType> (type), noiseSlope, maxErr);
    }

    void runTest() override
    {
        beginTest ("Uniform White Noise Test");
        runNoiseTest<float> (chowdsp::Noise<float>::NoiseType::Uniform);

        beginTest ("Uniform White Noise Test (SIMD)");
        runNoiseTest<vec4> (chowdsp::Noise<vec4>::NoiseType::Uniform);

        beginTest ("Normal White Noise Test");
        runNoiseTest<float> (chowdsp::Noise<float>::NoiseType::Normal);

        beginTest ("Normal White Noise Test (SIMD)");
        runNoiseTest<vec4> (chowdsp::Noise<vec4>::NoiseType::Normal);

        beginTest ("Pink Noise Test");
        runNoiseTest<float> (chowdsp::Noise<float>::NoiseType::Pink, -3.0f, 3.5f);

        beginTest ("Pink Noise Test (SIMD)");
        runNoiseTest<vec4> (chowdsp::Noise<vec4>::NoiseType::Pink, -3.0f, 3.5f);
    }
};

static NoiseTest noiseTest;
