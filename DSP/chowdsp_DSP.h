#pragma once

// SIMD
#include "SIMD/chowdsp_SIMDUtils.h"
#include "SIMD/chowdsp_SIMDSmoothedValue.h"
#include "SIMD/chowdsp_SIMDComplex.h"

template <typename T>
// template <typename C = T>
inline typename std::enable_if<std::is_floating_point<T>::value, void>::type
    copyBlocks (juce::dsp::AudioBlock<T>& dest, const juce::dsp::AudioBlock<const T>& src)
{
    dest.copyFrom (src);
}

template <typename T>
// template <typename C = T>
inline typename std::enable_if<! std::is_floating_point<T>::value, void>::type
    copyBlocks (juce::dsp::AudioBlock<T>& dest, const juce::dsp::AudioBlock<const T>& src)
{
    for (size_t ch = 0; ch < dest.getNumChannels(); ++ch)
    {
        auto* srcPtr = src.getChannelPointer (ch);
        auto* destPtr = dest.getChannelPointer (ch);
        for (size_t i = 0; i < dest.getNumSamples(); ++i)
            destPtr[i] = srcPtr[i];
    }
}

// WDF tools
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wshadow-field", // Ignore Clang warnings for WDFs
                                     "-Wshadow-field-in-constructor",
                                     "-Winconsistent-missing-destructor-override")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4324)
#include "WDF/wdf.h"
#include "WDF/wdf_t.h"
#include "WDF/r_type.h"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

// delay
#include "Delay/chowdsp_DelayInterpolation.h"
#include "Delay/chowdsp_DelayLine.h"
#include "Delay/chowdsp_BBDFilterBank.h"
#include "Delay/chowdsp_BBDDelayLine.h"
#include "Delay/chowdsp_BBDDelayWrapper.h"
#include "Delay/chowdsp_PitchShift.h"

// convolution
#include "Convolution/ConvolutionEngine.h"
#include "Convolution/IRTransfer.h"

// filter tools
#include "Filters/chowdsp_BilinearUtils.h"
#include "Filters/chowdsp_IIRFilter.h"
#include "Filters/chowdsp_QValCalcs.h"
#include "Filters/chowdsp_Shelf.h"
#include "Filters/chowdsp_StateVariableFilter.h"
#include "Filters/chowdsp_NthOrderFilter.h"
#include "Filters/chowdsp_juce_IIRFilter.h"

// modal tools
#include "Modal/chowdsp_ModalFilter.h"

// resamplers
#include "Resampling/chowdsp_BaseResampler.h"
#include "Resampling/chowdsp_LanczosResampler.h"
#include "Resampling/chowdsp_ResamplingProcessor.h"
#include "Resampling/chowdsp_SRCResampler.h"

// some other useful processors
#include "chowdsp_BypassProcessor.h"
#include "chowdsp_GainProcessor.h"
#include "chowdsp_LevelDetector.h"
#include "chowdsp_Panner.h"

// sound sources
#include "Sources/chowdsp_Noise.h"
#include "Sources/chowdsp_SineWave.h"
