#include "chowdsp_ModalFilter.h"

namespace chowdsp
{
template <typename T>
void ModalFilter<T>::prepare (T sampleRate)
{
    fs = sampleRate;

    decayFactor = calcDecayFactor();
    oscCoef = calcOscCoef();

    updateParams();
    reset();
}

template <typename T>
void ModalFilter<T>::processBlock (T* buffer, const int numSamples)
{
    for (int n = 0; n < numSamples; ++n)
        buffer[n] = processSample (buffer[n]);
}

//============================================================
template <typename FloatType>
void ModalFilter<xsimd::batch<FloatType>>::prepare (FloatType sampleRate)
{
    fs = sampleRate;

    decayFactor = calcDecayFactor();
    oscCoef = calcOscCoef();

    updateParams();
    reset();
}

template <typename FloatType>
void ModalFilter<xsimd::batch<FloatType>>::processBlock (VType* buffer, const int numSamples)
{
    for (int n = 0; n < numSamples; ++n)
        buffer[n] = processSample (buffer[n]);
}

template class ModalFilter<float>;
template class ModalFilter<double>;
template class ModalFilter<xsimd::batch<float>>;
#if ! JUCE_TEENSY
template class ModalFilter<xsimd::batch<double>>;
#endif
} // namespace chowdsp
