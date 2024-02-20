/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_dsp_data_structures
   vendor:        Chowdhury DSP
   version:       2.2.0
   name:          ChowDSP DSP Data Structures
   description:   DSP data structures for ChowDSP plugins
   dependencies:  chowdsp_simd, chowdsp_buffers, chowdsp_math

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
#include <chowdsp_buffers/chowdsp_buffers.h>
#include <chowdsp_math/chowdsp_math.h>

// third-party includes
#if ! JUCE_TEENSY // readerwriterqueue does not compile with the Teensy toolchain
#include "third_party/moodycamel/readerwriterqueue.h"
#include "third_party/moodycamel/concurrentqueue.h"
#endif

#include "Other/chowdsp_SmoothedBufferValue.h"

#include "Processors/chowdsp_RebufferedProcessor.h"
#include "LookupTables/chowdsp_LookupTableTransform.h"
#include "LookupTables/chowdsp_LookupTableCache.h"

#if ! JUCE_TEENSY // needs moodycamel dependency
#include "Other/chowdsp_UIToAudioPipeline.h"
#endif

#if JUCE_MODULE_AVAILABLE_juce_dsp
#include "Processors/chowdsp_COLAProcessor.h"
#endif
