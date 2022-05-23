#include <juce_dsp/juce_dsp.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

int main()
{
    chowdsp::BypassProcessor<float, chowdsp::DelayLineInterpolationTypes::None> bypass;

    // Setting a floating point value for lateny should fail when not using delay-line interp.
    bypass.setLatencySamples (0.5f);

    return 0;
}
