#include "chowdsp_LookupTableTransform.h"

namespace chowdsp
{
template <typename FloatType>
void LookupTableTransform<FloatType>::initialise (const std::function<FloatType (FloatType)>& functionToApproximate,
                                                  FloatType minInputValueToUse,
                                                  FloatType maxInputValueToUse,
                                                  size_t numPoints)
{
    jassert (maxInputValueToUse > minInputValueToUse);

    isInitialised.store (true);

    minInputValue = minInputValueToUse;
    maxInputValue = maxInputValueToUse;
    scaler = FloatType (numPoints - 1) / (maxInputValueToUse - minInputValueToUse);
    offset = -minInputValueToUse * scaler;

    const auto initFn = [functionToApproximate, minInputValueToUse, maxInputValueToUse, numPoints] (size_t i)
    {
        return functionToApproximate (
            juce::jlimit (
                minInputValueToUse, maxInputValueToUse, juce::jmap (FloatType (i), FloatType (0), FloatType (numPoints - 1), minInputValueToUse, maxInputValueToUse)));
    };

    lookupTable.initialise (initFn, numPoints);
}

template <typename FloatType>
bool LookupTableTransform<FloatType>::initialiseIfNotAlreadyInitialised() noexcept
{
    return AtomicHelpers::compareNegate (isInitialised, false);
}

//==============================================================================
#if CHOWDSP_ALLOW_TEMPLATE_INSTANTIATIONS
template class LookupTableTransform<float>;
template class LookupTableTransform<double>;
#endif
} // namespace chowdsp
