#pragma once

/** Helper methods for juce::dsp::AudioBlock */
namespace chowdsp::AudioBlockHelpers
{
/** Copy the information from one block to another (float/double) */
template <typename T, typename OtherT>
inline typename std::enable_if<std::is_floating_point<T>::value, void>::type
    copyBlocks (juce::dsp::AudioBlock<T>& dest, const juce::dsp::AudioBlock<OtherT>& src)
{
    dest.copyFrom (src);
}

/** Copy the information from one block to another (SIMD types) */
template <typename T, typename OtherT>
inline typename std::enable_if<SampleTypeHelpers::IsSIMDRegister<T>, void>::type
    copyBlocks (juce::dsp::AudioBlock<T>& dest, const juce::dsp::AudioBlock<OtherT>& src)
{
    for (size_t ch = 0; ch < dest.getNumChannels(); ++ch)
    {
        auto* srcPtr = src.getChannelPointer (ch);
        auto* destPtr = dest.getChannelPointer (ch);
        for (size_t i = 0; i < dest.getNumSamples(); ++i)
            destPtr[i] = srcPtr[i];
    }
}

/** Fades between two source blocks, and stores the result in a destination block */
//template <typename T>
//inline void fadeBlocks (juce::dsp::AudioBlock<T>& dest, const juce::dsp::AudioBlock<T>& src0to1, const juce::dsp::AudioBlock<T>& src1to0)
//{
//    const auto numSamples = dest.getNumSamples();
//    const auto increment = (T) 1 / (T) numSamples;
//
//    for (size_t ch = 0; ch < dest.getNumChannels(); ++ch)
//    {
//        auto* src01Ptr = src0to1.getChannelPointer (ch);
//        auto* src10Ptr = src1to0.getChannelPointer (ch);
//        auto* destPtr = dest.getChannelPointer (ch);
//
//        auto gain01 = (T) 0;
//        auto gain10 = (T) 1;
//
//        // @TODO: maybe we can vectorize this in some way...
//        for (size_t i = 0; i < numSamples; ++i)
//        {
//            destPtr[i] = src01Ptr[i] * gain01 + src10Ptr[i] * gain10;
//
//            gain01 += increment;
//            gain10 -= increment;
//        }
//    }
//}

} // namespace chowdsp::AudioBlockHelpers
