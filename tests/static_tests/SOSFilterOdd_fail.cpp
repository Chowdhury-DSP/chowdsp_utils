#include <juce_dsp/juce_dsp.h>
#include <chowdsp_filters/chowdsp_filters.h>

int main()
{
    // Odd-order SOS filters should NOT compile
    chowdsp::EllipticFilter<3> filter3;
    chowdsp::EllipticFilter<5> filter5;
    chowdsp::EllipticFilter<9> filter9;
    chowdsp::EllipticFilter<11> filter11;

    chowdsp::NthOrderFilter<float, 5> nthOrderFilter5;

    return 0;
}
