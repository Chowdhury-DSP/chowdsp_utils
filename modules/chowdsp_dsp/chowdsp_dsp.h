/*
 ==============================================================================

    BEGIN_JUCE_MODULE_DECLARATION

    ID:            chowdsp_dsp
    vendor:        Chowdhury DSP
    version:       0.0.1
    name:          ChowDSP DSP Utilities
    description:   Commonly used DSP utilities for ChowDSP plugins
    dependencies:  juce_core, juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_utils, juce_audio_processors

    website:       https://ccrma.stanford.edu/~jatin/chowdsp
    license:       GPLv3

    END_JUCE_MODULE_DECLARATION

 ==============================================================================
 */

#pragma once

/** Config: CHOWDSP_USE_LIBSAMPLERATE
            Enable libsamplerate wrappers for resampling processors.

            You must link against libsamplerate, and make sure that libsamplerate headers
            are in the include paths. You must respect the libsamplerate license when enabling this option.
  */
#ifndef CHOWDSP_USE_LIBSAMPLERATE
#define CHOWDSP_USE_LIBSAMPLERATE 0
#endif

/** Config: CHOWDSP_USE_CUSTOM_JUCE_DSP
            Use chowdsp_juce_dsp instead of juce_dsp.

            You must ensure that chowdsp_juce_dsp headers are in the include paths.
  */
#ifndef CHOWDSP_USE_CUSTOM_JUCE_DSP
#define CHOWDSP_USE_CUSTOM_JUCE_DSP 0
#endif

// STL includes
#include <cassert>
#include <random>

// JUCE includes
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

// Third-party includes
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-align",
                                     "-Wimplicit-int-conversion",
                                     "-Wshadow",
                                     "-Wshadow-field",
                                     "-Wsign-conversion",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wsign-compare",
                                     "-Wc++98-compat-extra-semi")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4244)
#include "third_party/xsimd/include/xsimd/xsimd.hpp"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

#if CHOWDSP_USE_CUSTOM_JUCE_DSP
#include <chowdsp_juce_dsp/chowdsp_juce_dsp.h>
#else
#include <juce_dsp/juce_dsp.h>
#endif

#if CHOWDSP_USE_LIBSAMPLERATE
#include <samplerate.h>
#endif

// SIMD
#include "SIMD/chowdsp_SampleTypeHelpers.h"
#include "SIMD/chowdsp_SIMDUtils.h"
#include "SIMD/chowdsp_SIMDLogic.h"
#include "SIMD/chowdsp_SIMDArithmetic.h"
#include "SIMD/chowdsp_SIMDSpecialMath.h"
#include "SIMD/chowdsp_SIMDFastMath.h"
#include "SIMD/chowdsp_SIMDDecibels.h"
#include "SIMD/chowdsp_SIMDSmoothedValue.h"
#include "SIMD/chowdsp_SIMDComplex.h"
#include "SIMD/chowdsp_SIMDComplexMathOps.h"

// math helpers
#include "Math/chowdsp_Combinatorics.h"
#include "Math/chowdsp_FloatVectorOperations.h"
#include "Math/chowdsp_MatrixOps.h"
#include "Math/chowdsp_Polynomials.h"
#include "Math/chowdsp_Power.h"
#include "Math/chowdsp_OtherMathOps.h"

// some useful data structures
#include "DataStructures/chowdsp_AudioBlockHelpers.h"
#include "DataStructures/chowdsp_COLAProcessor.h"
#include "DataStructures/chowdsp_DoubleBuffer.h"
#include "DataStructures/chowdsp_LookupTableTransform.h"
#include "DataStructures/chowdsp_RebufferedProcessor.h"
#include "DataStructures/chowdsp_SmoothedBufferValue.h"
#include "DataStructures/chowdsp_ScopedValue.h"

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
#include "Delay/chowdsp_PitchShift.h"
#include "Delay/BBD/chowdsp_BBDFilterBank.h"
#include "Delay/BBD/chowdsp_BBDDelayLine.h"
#include "Delay/BBD/chowdsp_BBDDelayWrapper.h"

// convolution
#include "Convolution/chowdsp_ConvolutionEngine.h"
#include "Convolution/chowdsp_IRTransfer.h"
#include "Convolution/chowdsp_IRHelpers.h"

// filter tools
#include "Filters/chowdsp_BilinearUtils.h"
#include "Filters/chowdsp_CoefficientCalculators.h"
#include "Filters/chowdsp_IIRFilter.h"
#include "Filters/chowdsp_FilterChain.h"
#include "Filters/chowdsp_FirstOrderFilters.h"
#include "Filters/chowdsp_SecondOrderFilters.h"
#include "Filters/chowdsp_QValCalcs.h"
#include "Filters/chowdsp_StateVariableFilter.h"
#include "Filters/chowdsp_NthOrderFilter.h"
#include "Filters/chowdsp_juce_IIRFilter.h"
#include "Filters/chowdsp_HilbertFilter.h"
#include "Filters/chowdsp_ModFilterWrapper.h"
#include "Filters/chowdsp_SOSFilter.h"
#include "Filters/chowdsp_ButterworthFilter.h"
#include "Filters/chowdsp_ChebyshevIIFilter.h"

// modal tools
#include "Modal/chowdsp_ModalFilter.h"
#include "Modal/chowdsp_ModalFilterBank.h"

// resamplers
#include "Resampling/chowdsp_Upsampler.h"
#include "Resampling/chowdsp_Downsampler.h"
#include "Resampling/chowdsp_BaseResampler.h"
#include "Resampling/chowdsp_LanczosResampler.h"
#include "Resampling/chowdsp_ResamplingProcessor.h"
#include "Resampling/chowdsp_SRCResampler.h"
#include "Resampling/chowdsp_ResampledProcess.h"
#include "Resampling/chowdsp_VariableOversampling.h"

// some other useful processors
#include "Processors/chowdsp_AudioTimer.h"
#include "Processors/chowdsp_BypassProcessor.h"
#include "Processors/chowdsp_GainProcessor.h"
#include "Processors/chowdsp_LevelDetector.h"
#include "Processors/chowdsp_Panner.h"
#include "Processors/chowdsp_SoftClipper.h"
#include "Processors/chowdsp_TunerProcessor.h"

// EQ tools
#include "EQ/chowdsp_EQBand.h"
#include "EQ/chowdsp_EQProcessor.h"
#include "EQ/chowdsp_LinearPhaseEQ.h"

// sound sources
#include "Sources/chowdsp_Noise.h"
#include "Sources/chowdsp_SawtoothWave.h"
#include "Sources/chowdsp_SineWave.h"
#include "Sources/chowdsp_SquareWave.h"
#include "Sources/chowdsp_RepitchedSource.h"

// some reverb utils
#include "Reverb/chowdsp_Diffuser.h"
#include "Reverb/chowdsp_FDN.h"
