namespace chowdsp
{
/**
 * Approximate versions of juce::Decibels.
 *
 * Note that these approximations rely on an IEEE Standard 754 floating-point representation.
 */
namespace DecibelsApprox
{
    /** Approximate version of juce::decibelsToGain */
    template <typename T, int order = 3>
    inline T gainToDecibels (T gain, T minusInfinityDb = SampleTypeHelpers::NumericType<T> (-100))
    {
        CHOWDSP_USING_XSIMD_STD (max);
        return SIMDUtils::select (gain > T {},
                                  max (LogApprox::log10<T, order> (gain) * (SampleTypeHelpers::NumericType<T>) 20, minusInfinityDb),
                                  minusInfinityDb);
    }

    /** Approximate version of juce::gainToDecibels */
    template <typename T, int order = 3>
    inline T decibelsToGain (T decibels, T minusInfinityDb = SampleTypeHelpers::NumericType<T> (-100))
    {
        return SIMDUtils::select (decibels > minusInfinityDb,
                                  PowApprox::pow10<T, order> (decibels * (SampleTypeHelpers::NumericType<T>) 0.05),
                                  T {});
    }
} // namespace DecibelsApprox
} // namespace chowdsp
