/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_dsp_utils
   vendor:        Chowdhury DSP
   version:       1.0.0
   name:          ChowDSP DSP Utilities
   description:   Commonly used DSP utilities for ChowDSP plugins
   dependencies:  juce_audio_processors, chowdsp_filters

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
#include <juce_audio_processors/juce_audio_processors.h>
#include <chowdsp_filters/chowdsp_filters.h>

#if CHOWDSP_USE_LIBSAMPLERATE
#include <samplerate.h>
#endif

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

// sound sources
#include "Sources/chowdsp_Noise.h"
#include "Sources/chowdsp_SawtoothWave.h"
#include "Sources/chowdsp_SineWave.h"
#include "Sources/chowdsp_SquareWave.h"
#include "Sources/chowdsp_RepitchedSource.h"
