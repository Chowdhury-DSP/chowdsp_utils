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
#include <algorithm>
#include <atomic>
#include <cmath>
#include <tuple>
#include <cstring>

#ifndef DOXYGEN
#if JUCE_MODULE_AVAILABLE_juce_core
#define CHOWDSP_USING_JUCE 1
#include <juce_core/juce_core.h>
#else
#define CHOWDSP_USING_JUCE 0
#endif

#if ! CHOWDSP_USING_JUCE
#define JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED 0
#include "JUCEHelpers/juce_TargetPlatform.h"
#include "JUCEHelpers/juce_CompilerWarnings.h"
#include "JUCEHelpers/juce_ExtraDefinitions.h"
#include "JUCEHelpers/juce_MathsFunctions.h"
#include "JUCEHelpers/juce_FloatVectorOperations.h"
#include "JUCEHelpers/juce_Decibels.h"
#include "JUCEHelpers/juce_SmoothedValue.h"
#include "JUCEHelpers/dsp/juce_ProcessSpec.h"
#endif // CHOWDSP_USING_JUCE
#endif // DOXYGEN

#include "DataStructures/chowdsp_AtomicHelpers.h"
#include "DataStructures/chowdsp_DoubleBuffer.h"
#include "DataStructures/chowdsp_TupleHelpers.h"
#include "Memory/chowdsp_MemoryAliasing.h"
#include "Types/chowdsp_TypeTraits.h"
