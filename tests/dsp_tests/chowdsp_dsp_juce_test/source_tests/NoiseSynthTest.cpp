#include <CatchUtils.h>
#include <chowdsp_sources/chowdsp_sources.h>

TEST_CASE ("Noise Synth Test", "[dsp][sources]")
{
    static constexpr auto fs = 48000.0;
    static constexpr int blockSize = 2048;

    chowdsp::NoiseSynth<128> synth;
    synth.prepare ({ fs, (uint32_t) blockSize, 1 });

    std::array<float, 128> magnitudes {};
    std::fill (magnitudes.begin(), magnitudes.begin() + 32, 1.0f);
    chowdsp::Buffer<float> buffer { 1, blockSize };
    synth.process (buffer, magnitudes);

    juce::dsp::FFT fft { chowdsp::Math::log2 (blockSize) };
    std::vector<float> fftInOut {};
    fftInOut.resize (2 * blockSize, 0.0f);

    juce::FloatVectorOperations::copy (fftInOut.data(), buffer.getReadPointer (0), blockSize);
    fft.performFrequencyOnlyForwardTransform (fftInOut.data(), true);

    const auto lowBandMag = std::accumulate (fftInOut.begin(), fftInOut.begin() + blockSize / 4, 0.0f);
    const auto highBandMag = std::accumulate (fftInOut.begin() + blockSize / 4, fftInOut.begin() + blockSize / 2, 0.0f);
    REQUIRE (lowBandMag / highBandMag > 10.0f);
}
