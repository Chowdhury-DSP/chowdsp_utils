#include <juce_dsp/juce_dsp.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

int main()
{
    // ModFilterWrapper for filter order higher than 2 should fail!
    chowdsp::ModFilterWrapper<chowdsp::ButterworthFilter<8>> modButter;

    return 0;
}
