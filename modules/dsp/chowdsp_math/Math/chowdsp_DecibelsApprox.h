namespace chowdsp
{
namespace DecibelsApprox
{
    template <typename T>
    inline T gainToDecibels (T gain, T minusInfinityDb = SampleTypeHelpers::NumericType<T> (-100))
    {
        CHOWDSP_USING_XSIMD_STD (max);
        return SIMDUtils::select (gain > T{},
                                  max (LogApprox::log10 (gain) * (SampleTypeHelpers::NumericType<T>) 20, minusInfinityDb),
                                  minusInfinityDb);
    }

    template <typename T>
    inline T decibelsToGain (T decibels, T minusInfinityDb = SampleTypeHelpers::NumericType<T> (-100))
    {
        return SIMDUtils::select (decibels > minusInfinityDb,
                                  PowApprox::pow10 (decibels * (SampleTypeHelpers::NumericType<T>) 0.05),
                                  T {});
    }
} // namespace DecibelsApprox
} // namespace chowdsp
