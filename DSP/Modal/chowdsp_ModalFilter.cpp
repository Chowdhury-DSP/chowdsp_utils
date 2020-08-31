#include "chowdsp_ModalFilter.h"

namespace chowdsp
{

template<typename T>
void ModalFilter<T>::prepare (T sampleRate) {
    fs = sampleRate;

    decayFactor = calcDecayFactor();
    oscCoef = calcOscCoef();

    updateParams();
    reset();
}

template<typename T>
T ModalFilter<T>::calcDecayFactor() noexcept
{
    return std::pow ((T) 0.001, (T) 1 / (t60 * fs));
}

template<typename T>
std::complex<T> ModalFilter<T>::calcOscCoef() noexcept
{
    constexpr std::complex<T> jImag { 0, 1 };
    return std::exp (jImag * juce::MathConstants<T>::twoPi * (freq / fs));
}

template<typename T>
void ModalFilter<T>::processBlock (T* buffer, const int numSamples)
{
    for (int n = 0; n < numSamples; ++n)
        buffer[n] = processSample (buffer[n]);
}

template class ModalFilter<float>;
template class ModalFilter<double>;

} // chowdsp
