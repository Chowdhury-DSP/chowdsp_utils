#include <juce_dsp/juce_dsp.h>
#include <chowdsp_filters/chowdsp_filters.h>

int main()
{
    // ModFilterWrapper for filter with order 1 or 2 should work fine!
    chowdsp::ModFilterWrapper<chowdsp::ShelfFilter<float>> modShelf;
    chowdsp::ModFilterWrapper<chowdsp::PeakingFilter<float>> modPeak;

    return 0;
}
