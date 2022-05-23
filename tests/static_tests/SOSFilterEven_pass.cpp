#include <juce_dsp/juce_dsp.h>
#include <chowdsp_filters/chowdsp_filters.h>

int main()
{
    // Even-order SOS filters should compile fine
    chowdsp::ButterworthFilter<2> filter2;
    chowdsp::ButterworthFilter<4> filter4;
    chowdsp::ButterworthFilter<8> filter8;
    chowdsp::ButterworthFilter<10> filter10;

    return 0;
}
