/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_sources
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Sound Sources
   description:   Sound sources (oscillators, signal generators) for ChowDSP plugins
   dependencies:  chowdsp_dsp_data_structures

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       GPLv3

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

#include "Oscillators/chowdsp_SawtoothWave.h"
#include "Oscillators/chowdsp_SineWave.h"
#include "Oscillators/chowdsp_SquareWave.h"
#include "Oscillators/chowdsp_TriangleWave.h"
#include "Oscillators/chowdsp_AdditiveOscillator.h"

#include "Oscillators/chowdsp_PolygonalOscillator.h"

#if CHOWDSP_USING_JUCE
#if JUCE_MODULE_AVAILABLE_juce_dsp
#include "Other/chowdsp_Noise.h"

#if JUCE_MODULE_AVAILABLE_chowdsp_filters
#include <chowdsp_filters/chowdsp_filters.h>
#include "Other/chowdsp_NoiseSynth.h"
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_dsp_utils
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>
#include "Other/chowdsp_RepitchedSource.h"
#endif // JUCE_MODULE_AVAILABLE_chowdsp_dsp_utils

#endif // JUCE_MODULE_AVAILABLE_juce_dsp
#endif // CHOWDSP_USING_JUCE
