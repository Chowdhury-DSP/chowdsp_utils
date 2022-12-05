/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_dsp_data_structures
   vendor:        Chowdhury DSP
   version:       1.3.0
   name:          ChowDSP DSP Data Structures
   description:   DSP data structures for ChowDSP plugins
   dependencies:  chowdsp_simd, chowdsp_math

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       GPLv3

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

//STL includes
#include <array>
#include <unordered_map>

//JUCE includes
#include <chowdsp_simd/chowdsp_simd.h>
#include <chowdsp_math/chowdsp_math.h>

#if JUCE_MODULE_AVAILABLE_juce_dsp
#include <juce_dsp/juce_dsp.h>
#include "chowdsp_dsp_data_structures/Buffers/chowdsp_SIMDAudioBlock.h"
#endif

// third-party includes
#if ! JUCE_TEENSY // readerwriterqueue does not compile with the Teensy toolchain
#include "third_party/moodycamel/readerwriterqueue.h"
#include "third_party/moodycamel/concurrentqueue.h"
#endif

#include "Other/chowdsp_ScopedValue.h"
#include "Other/chowdsp_SmoothedBufferValue.h"

#include "Buffers/chowdsp_Buffer.h"
#include "Buffers/chowdsp_StaticBuffer.h"
#include "Buffers/chowdsp_BufferView.h"
#include "Buffers/chowdsp_SIMDBufferHelpers.h"

#include "Processors/chowdsp_RebufferedProcessor.h"
#include "LookupTables/chowdsp_LookupTableTransform.h"
#include "LookupTables/chowdsp_LookupTableCache.h"

#if JUCE_MODULE_AVAILABLE_juce_dsp
#include "Processors/chowdsp_COLAProcessor.h"
#endif
