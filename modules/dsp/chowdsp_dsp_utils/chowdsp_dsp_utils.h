/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_dsp_utils
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP DSP Utilities
   description:   Commonly used DSP utilities for ChowDSP plugins
   dependencies:  chowdsp_filters

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

// STL includes
#include <random>

// JUCE includes
#include <chowdsp_filters/chowdsp_filters.h>

#if CHOWDSP_USE_LIBSAMPLERATE
#include <samplerate.h>
#endif

// delay
#include "Delay/chowdsp_DelayInterpolation.h"
#include "Delay/chowdsp_DelayLine.h"
#include "Delay/chowdsp_StaticDelayBuffer.h"
#include "Delay/chowdsp_PitchShift.h"

#if ! CHOWDSP_NO_XSIMD
#include "Delay/BBD/chowdsp_BBDFilterBank.h"
#include "Delay/BBD/chowdsp_BBDDelayLine.h"
#include "Delay/BBD/chowdsp_BBDDelayWrapper.h"
#endif

// resamplers
#include "Resampling/chowdsp_Upsampler.h"
#include "Resampling/chowdsp_Downsampler.h"
#include "Resampling/chowdsp_BaseResampler.h"
#if ! CHOWDSP_NO_XSIMD
#include "Resampling/chowdsp_LanczosResampler.h"
#endif
#include "Resampling/chowdsp_ResamplingProcessor.h"
#include "Resampling/chowdsp_SRCResampler.h"
#include "Resampling/chowdsp_ResampledProcess.h"

// some other useful processors
#include "Processors/chowdsp_AudioTimer.h"
#include "Processors/chowdsp_BypassProcessor.h"
#include "Processors/chowdsp_Gain.h"
#include "Processors/chowdsp_LevelDetector.h"
#include "Processors/chowdsp_Panner.h"
#include "Processors/chowdsp_WidthPanner.h"
#include "Processors/chowdsp_TunerProcessor.h"
#include "Processors/chowdsp_OvershootLimiter.h"

#if CHOWDSP_USING_JUCE
#include <juce_audio_processors/juce_audio_processors.h>

#if JUCE_MODULE_AVAILABLE_juce_dsp
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wimplicit-const-int-float-conversion")
#include <juce_dsp/juce_dsp.h>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

// convolution
#include "Convolution/chowdsp_ConvolutionEngine.h"
#include "Convolution/chowdsp_IRTransfer.h"
#include "Convolution/chowdsp_IRHelpers.h"
#include "Processors/chowdsp_LinearPhase3WayCrossover.h"

// resamplers
#include "Resampling/chowdsp_VariableOversampling.h"
#endif
#endif
