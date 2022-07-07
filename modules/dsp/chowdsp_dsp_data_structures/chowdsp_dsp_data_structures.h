/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_dsp_data_structures
   vendor:        Chowdhury DSP
   version:       1.0.0
   name:          ChowDSP DSP Data Structures
   description:   DSP data structures for ChowDSP plugins
   dependencies:  chowdsp_simd

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       GPLv3

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <array>

#include <chowdsp_simd/chowdsp_simd.h>

#include "DataStructures/chowdsp_ScopedValue.h"
#include "DataStructures/chowdsp_Buffer.h"
#include "DataStructures/chowdsp_SmoothedBufferValue.h"

#if CHOWDSP_USING_JUCE
#include <juce_dsp/juce_dsp.h>
#include "DataStructures/chowdsp_SIMDAudioBlock.h"
#include "DataStructures/chowdsp_COLAProcessor.h" // JUCE only
#include "DataStructures/chowdsp_LookupTableTransform.h" // relies on JUCE lookup table...
#include "DataStructures/chowdsp_RebufferedProcessor.h" // re-write to use BufferView
#include "DataStructures/chowdsp_LookupTableCache.h" // relies on lookup table...
#endif
