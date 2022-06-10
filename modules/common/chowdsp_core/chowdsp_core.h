/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_core
   vendor:        Chowdhury DSP
   version:       1.0.0
   name:          ChowDSP Core
   description:   Core code for other ChowDSP modules
   dependencies:

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-Clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// STL includes
#include <atomic>
#include <algorithm>
#include <tuple>

#include "DataStructures/chowdsp_AtomicHelpers.h"
#include "DataStructures/chowdsp_DoubleBuffer.h"
#include "DataStructures/chowdsp_TupleHelpers.h"
#include "Memory/chowdsp_MemoryAliasing.h"
#include "Types/chowdsp_TypeTraits.h"
