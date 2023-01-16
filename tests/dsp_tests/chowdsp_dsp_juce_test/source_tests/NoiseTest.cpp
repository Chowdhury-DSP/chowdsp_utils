#include <algorithm>
#include <CatchUtils.h>
#include <chowdsp_sources/chowdsp_sources.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpessimizing-move")

namespace
{
// processing constants
constexpr size_t fftOrder = 20;
constexpr size_t nSamples = 1 << fftOrder;
constexpr size_t blockSize = 256;
constexpr double fs = 48000.0;

// measuring constants
constexpr float startFreq = 100.0f;
constexpr size_t avgNum = 101;
constexpr size_t negDiff = avgNum / 2;
constexpr size_t posDiff = negDiff + 1;

using vec4 = xsimd::batch<float>;
} // namespace

/** Check the frequency slope of a noise buffer, slope measured in dB/Oct */
static void checkNoiseSlope (const float* buffer, const juce::String& noiseType, float noiseSlope = 0.0f, float maxErr = 3.0f)
{
    juce::dsp::FFT fft (fftOrder);

    std::vector<float> fftData (nSamples * 2, 0.0f);
    juce::FloatVectorOperations::copy (fftData.data(), buffer, nSamples);
    fft.performFrequencyOnlyForwardTransform (fftData.data());

    std::vector<float> magnitudes (nSamples);
    auto dBNorm = 3.0f * fftOrder - 6.0f;
    for (size_t i = 0; i < nSamples; ++i)
        magnitudes[i] = juce::Decibels::gainToDecibels<float> (std::abs (fftData[i])) - dBNorm;

    auto getMagForFreq = [=] (float freq) -> float
    {
        auto idx = size_t (((float) nSamples / 2.0f) * freq / (fs / 2.0f));
        // average over many bins to smooth
        return std::accumulate (&magnitudes[idx - negDiff], &magnitudes[idx + posDiff], 0.0f) / (float) avgNum;
    };

    auto freq = startFreq;
    auto expMag = 0.0f;
    while (freq < fs / 2)
    {
        auto mag = getMagForFreq (freq);
        juce::String errorMsg = noiseType + " error at frequency " + juce::String (freq, 2) + " Hz";
        REQUIRE_MESSAGE (mag == Catch::Approx { expMag }.margin (maxErr), errorMsg);

        expMag += noiseSlope;
        freq *= 2;
    }
}

template <typename FloatType>
juce::String getNoiseNameForType (typename chowdsp::Noise<FloatType>::NoiseType type)
{
    if (type == chowdsp::Noise<FloatType>::NoiseType::Uniform)
        return "Uniform White Noise";

    if (type == chowdsp::Noise<FloatType>::NoiseType::Normal)
        return "Normal White Noise";

    if (type == chowdsp::Noise<FloatType>::NoiseType::Pink)
        return "Pink Noise";

    jassertfalse;
    return {};
}

[[maybe_unused]] static std::vector<std::vector<float>> getTestVector (const juce::dsp::AudioBlock<float> data)
{
    const auto numChannels = data.getNumChannels();
    const auto numSamples = data.getNumSamples();

    std::vector<std::vector<float>> vecs;
    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto* dataPtr = data.getChannelPointer (ch);
        vecs.emplace_back (dataPtr, dataPtr + numSamples);
    }

    return std::move (vecs);
}

static std::vector<std::vector<float>> getTestVector (const chowdsp::AudioBlock<vec4> data)
{
    const auto numChannels = data.getNumChannels();
    const auto numSamples = data.getNumSamples();
    std::vector<std::vector<float>> vecs (4 * numChannels, std::vector<float> (numSamples, 0.0f));

    for (size_t ch = 0; ch < numChannels; ++ch)
        for (size_t n = 0; n < numSamples; ++n)
            for (size_t k = 0; k < 4; ++k)
                vecs[4 * ch + k][n] = data.getSample ((int) ch, (int) n).get (k);

    return std::move (vecs);
}

template <typename FloatType>
void runNoiseTest (typename chowdsp::Noise<FloatType>::NoiseType type, float noiseSlope = 0.0f, float maxErr = 4.0f, size_t nCh = 1, bool replacing = true)
{
    chowdsp::Noise<FloatType> noise;

    juce::dsp::ProcessSpec spec { fs, blockSize, (juce::uint32) nCh };
    noise.prepare (spec);

    noise.setGainLinear (1.0f);
    noise.setNoiseType (type);

    juce::HeapBlock<char> inBlockData;
    auto inBlock = chowdsp::AudioBlock<FloatType> (inBlockData, (size_t) nCh, (size_t) nSamples);
    inBlock.clear();

    juce::HeapBlock<char> outBlockData;
    auto outBlock = chowdsp::AudioBlock<FloatType> (outBlockData, (size_t) nCh, (size_t) nSamples);
    outBlock.clear();

    if (replacing)
    {
        for (int n = 0; n < (int) nSamples; n += (int) blockSize)
        {
            auto block = outBlock.getSubBlock ((size_t) n, (size_t) blockSize);
            chowdsp::ProcessContextReplacing<FloatType> ctx (block);
            noise.process (ctx);
        }
    }
    else
    {
        for (int n = 0; n < (int) nSamples; n += (int) blockSize)
        {
            auto inSubBlock = inBlock.getSubBlock ((size_t) n, (size_t) blockSize);
            auto outSubBlock = outBlock.getSubBlock ((size_t) n, (size_t) blockSize);
            chowdsp::ProcessContextNonReplacing<FloatType> ctx { inSubBlock, outSubBlock };
            noise.process (ctx);
        }
    }

    auto testVectors = getTestVector (outBlock);
    for (const auto& vec : testVectors)
        checkNoiseSlope (vec.data(), getNoiseNameForType<FloatType> (type), noiseSlope, maxErr);
}

/** Unit tests for chowdsp::Noise. Testing frequency domain accuracy for:
 *   - Uniform white noise (flat frequency spectrum)
 *   - Gaussian (Normal) white noise (flat frequency spectrum)
 *   - Pink noise (-3dB / Oct)
 */
TEST_CASE ("Noise Test", "[dsp][sources]")
{
    SECTION ("Uniform White Noise Test")
    {
        runNoiseTest<float> (chowdsp::Noise<float>::NoiseType::Uniform);
    }

    SECTION ("Uniform White Noise Test (SIMD)")
    {
        runNoiseTest<vec4> (chowdsp::Noise<vec4>::NoiseType::Uniform);
    }

    SECTION ("Normal White Noise Test")
    {
        runNoiseTest<float> (chowdsp::Noise<float>::NoiseType::Normal);
    }

    SECTION ("Normal White Noise Test (SIMD)")
    {
        runNoiseTest<vec4> (chowdsp::Noise<vec4>::NoiseType::Normal);
    }

    SECTION ("Pink Noise Test")
    {
        runNoiseTest<float> (chowdsp::Noise<float>::NoiseType::Pink, -3.0f, 4.0f, 2, false);
    }

    SECTION ("Pink Noise Test (SIMD)")
    {
        runNoiseTest<vec4> (chowdsp::Noise<vec4>::NoiseType::Pink, -3.0f, 4.0f, 2, false);
    }
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
