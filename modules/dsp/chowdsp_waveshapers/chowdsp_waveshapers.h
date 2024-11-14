/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_waveshapers
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Waveshapers
   description:   Waveshaping functions for ChowDSP plugins
   dependencies:  chowdsp_math, chowdsp_dsp_data_structures, chowdsp_filters

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       GPLv3

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// STL includes
#include <future>

#include <chowdsp_math/chowdsp_math.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>
#include <chowdsp_filters/chowdsp_filters.h>

#include "Waveshapers/chowdsp_ADAAWaveshaper.h"
#include "Waveshapers/chowdsp_ADAAHardClipper.h"
#include "Waveshapers/chowdsp_ADAATanhClipper.h"
#include "Waveshapers/chowdsp_ADAASoftClipper.h"
#include "Waveshapers/chowdsp_ADAAFullWaveRectifier.h"
#include "Waveshapers/chowdsp_WestCoastWavefolder.h"
#include "Waveshapers/chowdsp_WaveMultiplier.h"
#include "Waveshapers/chowdsp_SoftClipper.h"
#include "Waveshapers/chowdsp_ADAASineClipper.h"
