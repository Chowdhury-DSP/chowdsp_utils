#include "PolyphaseOversamplingPlugin.h"

namespace
{
constexpr int os_ratio = 4;

// Coefficients generated from: https://fiiir.com/
// Cutoff frequency: 0.13
// Transition bandwidth: 0.08
// Blackman window
constexpr std::array<float, 59> aa_coeffs {
    0.000000000000000000f,
    -0.000009290744190889f,
    -0.000003179945796776f,
    0.000082640816061341f,
    0.000229223614031907f,
    0.000263693640932877f,
    -0.000037675811380258f,
    -0.000683760580094780f,
    -0.001253639060684718f,
    -0.001027123632937930f,
    0.000442319424678493f,
    0.002640563958995431f,
    0.003947942580809822f,
    0.002523482958339593f,
    -0.002055129745209706f,
    -0.007561189288896839f,
    -0.009677304885113274f,
    -0.004738033395627680f,
    0.006739390459359920f,
    0.018445648506312032f,
    0.020795751620257420f,
    0.007230813110385698f,
    -0.019183700519560042f,
    -0.043745289771951856f,
    -0.045627091908622811f,
    -0.009231438741921514f,
    0.064759296816463446f,
    0.155809509825300757f,
    0.230923660241676043f,
    0.259999820916768321f,
    0.230923660241676099f,
    0.155809509825300757f,
    0.064759296816463446f,
    -0.009231438741921514f,
    -0.045627091908622811f,
    -0.043745289771951870f,
    -0.019183700519560049f,
    0.007230813110385697f,
    0.020795751620257423f,
    0.018445648506312043f,
    0.006739390459359923f,
    -0.004738033395627683f,
    -0.009677304885113274f,
    -0.007561189288896845f,
    -0.002055129745209709f,
    0.002523482958339594f,
    0.003947942580809828f,
    0.002640563958995434f,
    0.000442319424678492f,
    -0.001027123632937931f,
    -0.001253639060684717f,
    -0.000683760580094779f,
    -0.000037675811380258f,
    0.000263693640932877f,
    0.000229223614031908f,
    0.000082640816061341f,
    -0.000003179945796776f,
    -0.000009290744190889f,
    0.000000000000000000f,
};
} // namespace

void PolyphaseOversamplingPlugin::prepareToPlay (double sample_rate, int samples_per_block)
{
    const auto num_channels = getMainBusNumInputChannels();

    os_buffer.setMaxSize (num_channels, samples_per_block * os_ratio);
    gain.prepare ({
        sample_rate * static_cast<double> (os_ratio),
        static_cast<uint32_t> (samples_per_block * os_ratio),
        static_cast<uint32_t> (num_channels),
    });

    std::array<float, aa_coeffs.size()> upsample_coeffs {};
    std::copy (aa_coeffs.begin(), aa_coeffs.end(), upsample_coeffs.begin());
    for (auto& coeff : upsample_coeffs)
        coeff *= (float) os_ratio;

    upsampler.prepare (os_ratio, num_channels, samples_per_block, upsample_coeffs);
    downsampler.prepare (os_ratio, num_channels, samples_per_block * os_ratio, aa_coeffs);
}

void PolyphaseOversamplingPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    const auto num_channels = buffer.getNumChannels();
    const auto num_samples = buffer.getNumSamples();
    os_buffer.setCurrentSize (num_channels, num_samples * os_ratio);

    upsampler.processBlock (buffer, os_buffer);

    gain.setGainDecibels (state.params.gain->getCurrentValue());
    gain.process (os_buffer);

    chowdsp::BufferMath::applyFunctionSIMD (os_buffer,
                                            [] (auto x)
                                            {
                                                CHOWDSP_USING_XSIMD_STD (tanh);
                                                return tanh (x);
                                            });

    downsampler.processBlock (os_buffer, buffer);
}
