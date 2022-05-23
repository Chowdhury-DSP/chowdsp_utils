#include <juce_dsp/juce_dsp.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

int main()
{
    // Four is a multiple of the SIMD register width on most architectures, so this should compile.
    chowdsp::ResamplingTypes::LanczosResampler<4096, 4> resampler;
    return 0;
}
