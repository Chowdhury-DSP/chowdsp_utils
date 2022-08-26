#include <chowdsp_filters/chowdsp_filters.h>

int main()
{
    static constexpr auto butterQs = chowdsp::QValCalcs::butterworth_Qs<float, 6>();
    juce::ignoreUnused (butterQs);

    return 0;
}
