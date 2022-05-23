#include <juce_dsp/juce_dsp.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

int main()
{
    // SoftClipper order must be an odd number, larger than 2
    chowdsp::SoftClipper<-1> clipperm1;
    chowdsp::SoftClipper<0> clipper0;
    chowdsp::SoftClipper<2> clipper2;
    chowdsp::SoftClipper<6> clipper6;

    return 0;
}
