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

    minInputValue = minInputValueToUse;
    maxInputValue = maxInputValueToUse;
    scaler = FloatType (numPoints - 1) / (maxInputValueToUse - minInputValueToUse);
    offset = -minInputValueToUse * scaler;

    const auto initFn = [functionToApproximate, minInputValueToUse, maxInputValueToUse, numPoints] (size_t i) {
        return functionToApproximate (
            juce::jlimit (
                minInputValueToUse, maxInputValueToUse, juce::jmap (FloatType (i), FloatType (0), FloatType (numPoints - 1), minInputValueToUse, maxInputValueToUse)));
    };

    lookupTable.initialise (initFn, numPoints);
}

//==============================================================================
template class LookupTableTransform<float>;
template class LookupTableTransform<double>;
} // namespace chowdsp
