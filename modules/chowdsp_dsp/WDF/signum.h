#pragma once

namespace chowdsp
{
/** Signum function to determine the sign of the input. */
template <typename T>
inline int signum (T val)
{
    return (T (0) < val) - (val < T (0));
}

#if WDF_USING_JUCE
/** Signum function to determine the sign of the input. */
template <typename T>
inline juce::dsp::SIMDRegister<T> signumSIMD (juce::dsp::SIMDRegister<T> val)
{
    auto positive = juce::dsp::SIMDRegister<T> ((T) 1) & juce::dsp::SIMDRegister<T>::lessThan (juce::dsp::SIMDRegister<T> ((T) 0), val);
    auto negative = juce::dsp::SIMDRegister<T> ((T) 1) & juce::dsp::SIMDRegister<T>::lessThan (val, juce::dsp::SIMDRegister<T> ((T) 0));
    return positive - negative;
}
#endif

} // namespace chowdsp
