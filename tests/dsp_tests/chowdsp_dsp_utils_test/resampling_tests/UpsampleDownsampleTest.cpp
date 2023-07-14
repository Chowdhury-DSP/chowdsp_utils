#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

#if JUCE_MAC
#include <Accelerate/Accelerate.h>

struct AppleFFT
{
    static constexpr int priority = 5;

    template <typename T>
    using Complex = std::complex<T>;

    static AppleFFT* create (int order)
    {
        return new AppleFFT (order);
    }

    explicit AppleFFT (int orderToUse)
        : order (static_cast<vDSP_Length> (orderToUse)),
          fftSetup (vDSP_create_fftsetup (order, 2)),
          forwardNormalisation (0.5f),
          inverseNormalisation (1.0f / static_cast<float> (1 << order))
    {
    }

    ~AppleFFT()
    {
        if (fftSetup != nullptr)
        {
            vDSP_destroy_fftsetup (fftSetup);
            fftSetup = nullptr;
        }
    }

    void perform (const Complex<float>* input, Complex<float>* output, bool inverse) const noexcept
    {
        auto size = (1 << order);

        DSPSplitComplex splitInput (toSplitComplex (const_cast<Complex<float>*> (input)));
        DSPSplitComplex splitOutput (toSplitComplex (output));

        vDSP_fft_zop (fftSetup, &splitInput, 2, &splitOutput, 2, order, inverse ? kFFTDirection_Inverse : kFFTDirection_Forward);

        float factor = (inverse ? inverseNormalisation : forwardNormalisation * 2.0f);
        vDSP_vsmul ((float*) output, 1, &factor, (float*) output, 1, static_cast<size_t> (size << 1));
    }

    void performRealOnlyForwardTransform (float* inoutData, bool ignoreNegativeFreqs) const noexcept
    {
        auto size = (1 << order);
        auto* inout = reinterpret_cast<Complex<float>*> (inoutData);
        auto splitInOut (toSplitComplex (inout));

        inoutData[size] = 0.0f;
        vDSP_fft_zrip (fftSetup, &splitInOut, 2, order, kFFTDirection_Forward);
        vDSP_vsmul (inoutData, 1, &forwardNormalisation, inoutData, 1, static_cast<size_t> (size << 1));

        mirrorResult (inout, ignoreNegativeFreqs);
    }

    void performRealOnlyInverseTransform (float* inoutData) const noexcept
    {
        auto* inout = reinterpret_cast<Complex<float>*> (inoutData);
        auto size = (1 << order);
        auto splitInOut (toSplitComplex (inout));

        // Imaginary part of nyquist and DC frequencies are always zero
        // so Apple uses the imaginary part of the DC frequency to store
        // the real part of the nyquist frequency
        if (size != 1)
            inout[0] = Complex<float> (inout[0].real(), inout[size >> 1].real());

        vDSP_fft_zrip (fftSetup, &splitInOut, 2, order, kFFTDirection_Inverse);
        vDSP_vsmul (inoutData, 1, &inverseNormalisation, inoutData, 1, static_cast<size_t> (size << 1));
        vDSP_vclr (inoutData + size, 1, static_cast<size_t> (size));
    }

    void performFrequencyOnlyForwardTransform (float* inputOutputData, bool ignoreNegativeFreqs = false) const noexcept
    {
        const auto size = 1 << order;

        performRealOnlyForwardTransform (inputOutputData, ignoreNegativeFreqs);
        auto* out = reinterpret_cast<Complex<float>*> (inputOutputData);

        const auto limit = ignoreNegativeFreqs ? (size / 2) + 1 : size;

        for (int i = 0; i < limit; ++i)
            inputOutputData[i] = std::abs (out[i]);

        juce::FloatVectorOperations::clear (inputOutputData + limit, static_cast<size_t> (size * 2 - limit));
    }

private:
    //==============================================================================
    void mirrorResult (Complex<float>* out, bool ignoreNegativeFreqs) const noexcept
    {
        auto size = (1 << order);
        auto i = size >> 1;

        // Imaginary part of nyquist and DC frequencies are always zero
        // so Apple uses the imaginary part of the DC frequency to store
        // the real part of the nyquist frequency
        out[i++] = { out[0].imag(), 0.0 };
        out[0] = { out[0].real(), 0.0 };

        if (! ignoreNegativeFreqs)
            for (; i < size; ++i)
                out[i] = std::conj (out[size - i]);
    }

    static DSPSplitComplex toSplitComplex (Complex<float>* data) noexcept
    {
        // this assumes that Complex interleaves real and imaginary parts
        // and is tightly packed.
        return { reinterpret_cast<float*> (data),
                 reinterpret_cast<float*> (data) + 1 };
    }

    //==============================================================================
    vDSP_Length order;
    FFTSetup fftSetup;
    float forwardNormalisation, inverseNormalisation;
};

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

/** Returns frequency domain SNR in dB */
static float calcSNR (const chowdsp::BufferView<float>& buffer, float freqExpected, float fs)
{
    AppleFFT fft (fftOrder);

    std::vector<float> fftData (blockSize * 2, 0.0f);
    juce::FloatVectorOperations::copy (fftData.data(), buffer.getReadPointer (0), blockSize);
    fft.performFrequencyOnlyForwardTransform (fftData.data());

    std::vector<float> magnitudes (blockSize);
    auto dBNorm = 3.0f * fftOrder - 6.0f;
    auto scaleNorm = juce::Decibels::decibelsToGain<float> (dBNorm);
    for (size_t i = 0; i < blockSize; ++i)
        magnitudes[i] = std::pow (fftData[i] / scaleNorm, 2.0f);

    auto getMagForFreq = [&magnitudes, fs] (float freq) -> float
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

    return juce::Decibels::gainToDecibels (signalAccum / noiseAccum);
}

template <typename ProcType>
static auto processInSubBlocks (ProcType& proc, chowdsp::Buffer<float>& inBuffer, int subBlockSize = blockSize / 8)
{
    chowdsp::Buffer<float> outBuffer { inBuffer.getNumChannels(), inBuffer.getNumSamples() * 10 };
    int outBufferPtr = 0;

    for (int i = 0; i < inBuffer.getNumSamples(); i += subBlockSize)
    {
        auto outBlock = proc.process (chowdsp::BufferView<float> { inBuffer, i, subBlockSize });

        auto numOutSamples = (int) outBlock.getNumSamples();
        juce::FloatVectorOperations::copy (outBuffer.getWritePointer (0) + outBufferPtr, outBlock.getReadPointer (0), numOutSamples);
        outBufferPtr += numOutSamples;
    }

    outBuffer.setCurrentSize (1, outBufferPtr);
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
    auto snr = calcSNR (upsampledBuffer, testFreq, (float) _sampleRate * (float) upsampler.getUpsamplingRatio());

    REQUIRE_MESSAGE (upsampledBuffer.getNumSamples() == (int) blockSize * upsampler.getUpsamplingRatio(), "Upsampled block size is incorrect!");
    REQUIRE_MESSAGE (snr > 60.0f, "Signal to noise ratio is too low!");
}

template <typename FilterType>
void downsampleQualityTest (int downsampleRatio)
{
    chowdsp::Downsampler<float, FilterType> downsampler;
    downsampler.prepare ({ (double) downsampleRatio * _sampleRate, (juce::uint32) downsampleRatio * (juce::uint32) blockSize, 1 }, downsampleRatio);

    static constexpr float testFreq = 42000.0f;
    auto thisBlockSize = downsampleRatio * (int) blockSize;
    auto sineBuffer = test_utils::makeSineWave (testFreq, (float) downsampleRatio * (float) _sampleRate, thisBlockSize);
    auto downsampledBuffer = processInSubBlocks (downsampler, sineBuffer, downsampleRatio * 2048);

    REQUIRE_MESSAGE (downsampledBuffer.getNumSamples() == thisBlockSize / downsampler.getDownsamplingRatio(), "Downsampled block size is incorrect!");
    if (downsampleRatio > 1)
    {
        float squaredSum = 0.0f;
        for (int n = 0; n < (int) blockSize / downsampleRatio; ++n)
            squaredSum += std::pow (downsampledBuffer.getReadPointer (0)[n], 2.0f);

        auto rms = juce::Decibels::gainToDecibels (std::sqrt (squaredSum / ((float) blockSize / 2.0f)));

        REQUIRE_MESSAGE (rms < -42.0f, "RMS level is too high!");
    }
    else
    {
        for (auto [ch, dsData] : chowdsp::buffer_iters::channels (std::as_const (downsampledBuffer)))
        {
            auto sineData = sineBuffer.getReadSpan (ch);
            for (auto [dsSample, sineSample] : chowdsp::zip (dsData, sineData))
                REQUIRE (std::abs (sineSample - dsSample) < 1.0e-6f);
        }
    }
}

TEST_CASE ("Upsample/Downsample Test", "[dsp][resampling]")
{
    SECTION ("Upsample 1x Quality Test")
    {
        upsampleQualityTest<chowdsp::ButterworthFilter<4>> (1);
    }

    SECTION ("Upsample 2x Quality Test")
    {
        upsampleQualityTest<chowdsp::ButterworthFilter<4>> (2);
    }

    SECTION ("Upsample 3x Quality Test")
    {
        upsampleQualityTest<chowdsp::ButterworthFilter<8>> (3);
    }

    SECTION ("Upsample 3x Chebyshev Quality Test")
    {
        upsampleQualityTest<chowdsp::ChebyshevIIFilter<12>> (3);
    }

    SECTION ("Upsample 4x Elliptic Quality Test")
    {
        upsampleQualityTest<chowdsp::EllipticFilter<12>> (4);
    }

    SECTION ("Downsample 1x Quality Test")
    {
        downsampleQualityTest<chowdsp::ButterworthFilter<4>> (1);
    }

    SECTION ("Downsample 2x Quality Test")
    {
        downsampleQualityTest<chowdsp::ButterworthFilter<4>> (2);
    }

    SECTION ("Downsample 3x Quality Test")
    {
        downsampleQualityTest<chowdsp::ButterworthFilter<16>> (3);
    }

    SECTION ("Downsample 3x Chebyshev Quality Test")
    {
        downsampleQualityTest<chowdsp::ChebyshevIIFilter<12>> (3);
    }

    SECTION ("Downsample 4x Elliptic Quality Test")
    {
        downsampleQualityTest<chowdsp::EllipticFilter<12>> (4);
    }
}

#endif // JUCE_MAC
