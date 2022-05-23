#include <juce_dsp/juce_dsp.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

int main()
{
    // Five is not a multiple of the SIMD register width on most architectures, so this NOT should compile.
    chowdsp::ResamplingTypes::LanczosResampler<4096, 5> resampler;
    return 0;
}
