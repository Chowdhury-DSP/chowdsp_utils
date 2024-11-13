/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_compressor
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Compressor
   description:   Tools for building a compressor
   dependencies:  chowdsp_dsp_data_structures

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       GPLv3

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

/**
 * WARNING: This module is currently unfinished!
 * It is still in need of much testing and optimization.
 */

#pragma once

#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

namespace chowdsp
{
/** ChowDSP classes for creating compressor effects */
namespace compressor
{
} // namespace compressor
} // namespace chowdsp

#include "Compressor/chowdsp_LevelDetectorImpls.h"
#include "Compressor/chowdsp_CompressorLevelDetector.h"
#include "Compressor/chowdsp_GainComputerImpls.h"
#include "Compressor/chowdsp_CompressorGainComputer.h"
#include "Compressor/chowdsp_MonoCompressor.h"
