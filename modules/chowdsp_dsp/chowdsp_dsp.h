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

/** Config: CHOWDSP_USE_XSIMD
            Enable XSIMD library for accelerated SIMD functions.

            You must ensure that xsimd headers are in the include paths.
            You must respect the xsimd license when enabling this option.
  */
#ifndef CHOWDSP_USE_XSIMD
#define CHOWDSP_USE_XSIMD 0
#endif

/** Config: CHOWDSP_USE_CUSTOM_JUCE_DSP
            Use chowdsp_juce_dsp instead of juce_dsp.

            You must ensure that chowdsp_juce_dsp headers are in the include paths.
  */
#ifndef CHOWDSP_USE_CUSTOM_JUCE_DSP
#define CHOWDSP_USE_CUSTOM_JUCE_DSP 0
#endif

// JUCE includes
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

#if CHOWDSP_USE_CUSTOM_JUCE_DSP
#include <chowdsp_juce_dsp/chowdsp_juce_dsp.h>
#else
#include <juce_dsp/juce_dsp.h>
#endif

#if CHOWDSP_USE_LIBSAMPLERATE
#include <samplerate.h>
#endif

#if CHOWDSP_USE_XSIMD
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-align",
                                     "-Wimplicit-int-conversion",
                                     "-Wshadow",
                                     "-Wsign-conversion",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wc++98-compat-extra-semi")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4244)
#include <cassert>
#include <xsimd/xsimd.hpp>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC
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

// math helpers
#include "Math/chowdsp_FloatVectorOperations.h"
#include "Math/chowdsp_Polynomials.h"

// some useful data structures
#include "DataStructures/chowdsp_AudioBlockHelpers.h"
#include "DataStructures/chowdsp_DoubleBuffer.h"
#include "DataStructures/chowdsp_RebufferedProcessor.h"
#include "DataStructures/chowdsp_SmoothedBufferValue.h"

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
#include "Convolution/chowdsp_ConvolutionEngine.h"
#include "Convolution/chowdsp_IRTransfer.h"

// filter tools
#include "Filters/chowdsp_BilinearUtils.h"
#include "Filters/chowdsp_IIRFilter.h"
#include "Filters/chowdsp_FirstOrderFilters.h"
#include "Filters/chowdsp_SecondOrderFilters.h"
#include "Filters/chowdsp_QValCalcs.h"
#include "Filters/chowdsp_Shelf.h"
#include "Filters/chowdsp_StateVariableFilter.h"
#include "Filters/chowdsp_NthOrderFilter.h"
#include "Filters/chowdsp_juce_IIRFilter.h"

// modal tools
#include "Modal/chowdsp_ModalFilter.h"

// resamplers
#include "Resampling/chowdsp_Upsampler.h"
#include "Resampling/chowdsp_Downsampler.h"
#include "Resampling/chowdsp_BaseResampler.h"
#include "Resampling/chowdsp_LanczosResampler.h"
#include "Resampling/chowdsp_ResamplingProcessor.h"
#include "Resampling/chowdsp_SRCResampler.h"
#include "Resampling/chowdsp_ResampledProcess.h"

// some other useful processors
#include "Processors/chowdsp_BypassProcessor.h"
#include "Processors/chowdsp_GainProcessor.h"
#include "Processors/chowdsp_LevelDetector.h"
#include "Processors/chowdsp_Panner.h"
#include "Processors/chowdsp_TunerProcessor.h"

// sound sources
#include "Sources/chowdsp_Noise.h"
#include "Sources/chowdsp_SineWave.h"
