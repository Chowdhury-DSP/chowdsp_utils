#include <juce_dsp/juce_dsp.h>
#include <chowdsp_filters/chowdsp_filters.h>

int main()
{
    // Odd-order SOS filters should NOT compile
    chowdsp::ButterworthFilter<3> filter3;
    chowdsp::ButterworthFilter<5> filter5;
    chowdsp::ButterworthFilter<9> filter9;
    chowdsp::ButterworthFilter<11> filter11;

    return 0;
}
