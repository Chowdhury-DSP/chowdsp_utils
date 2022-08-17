#include <chowdsp_filters/chowdsp_filters.h>

int main()
{
    static constexpr auto tanhAD1 = chowdsp::TanhIntegrals::tanhAD1 (100.0f);
    juce::ignoreUnused (tanhAD1);

    static constexpr auto tanhAD2 = chowdsp::TanhIntegrals::tanhAD2 (100.0f);
    juce::ignoreUnused (tanhAD2);

    static constexpr auto butterQs = chowdsp::QValCalcs::butterworth_Qs<float, 6>();
    juce::ignoreUnused (butterQs);

    return 0;
}
