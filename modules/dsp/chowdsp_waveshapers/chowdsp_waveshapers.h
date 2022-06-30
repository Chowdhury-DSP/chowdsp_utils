/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_waveshapers
   vendor:        Chowdhury DSP
   version:       1.0.0
   name:          ChowDSP Waveshapers
   description:   Waveshaping functions for ChowDSP plugins
   dependencies:  chowdsp_math, chowdsp_dsp_data_structures

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       GPLv3

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// STL includes
#include <future>

// JUCE includes
#include <chowdsp_math/chowdsp_math.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

#include "Waveshapers/chowdsp_ADAAWaveshaper.h"
#include "Waveshapers/chowdsp_ADAAHardClipper.h"
#include "Waveshapers/chowdsp_ADAATanhClipper.h"
#include "Waveshapers/chowdsp_SoftClipper.h"
