/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_dsp_data_structures
   vendor:        Chowdhury DSP
   version:       1.1.0
   name:          ChowDSP DSP Data Structures
   description:   DSP data structures for ChowDSP plugins
   dependencies:  chowdsp_simd, chowdsp_math

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       GPLv3

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <array>
#include <unordered_map>

#include <chowdsp_simd/chowdsp_simd.h>
#include <chowdsp_math/chowdsp_math.h>

#if JUCE_MODULE_AVAILABLE_juce_dsp
#include <juce_dsp/juce_dsp.h>
#include "DataStructures/chowdsp_SIMDAudioBlock.h"
#endif

#include "DataStructures/chowdsp_ScopedValue.h"
#include "DataStructures/chowdsp_Buffer.h"
#include "DataStructures/chowdsp_BufferView.h"
#include "DataStructures/chowdsp_SmoothedBufferValue.h"
#include "DataStructures/chowdsp_RebufferedProcessor.h"
#include "DataStructures/chowdsp_LookupTableTransform.h"
#include "DataStructures/chowdsp_LookupTableCache.h"

#if JUCE_MODULE_AVAILABLE_juce_dsp
#include "DataStructures/chowdsp_COLAProcessor.h"
#endif
