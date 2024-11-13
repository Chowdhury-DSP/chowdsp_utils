/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_core
   vendor:        Chowdhury DSP
   version:       2.3.0
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
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <new>
#include <string_view>
#include <tuple>
#include <vector>

#ifndef DOXYGEN
#if JUCE_MODULE_AVAILABLE_juce_core
#define CHOWDSP_USING_JUCE 1
#include <juce_core/juce_core.h>
#else
#define CHOWDSP_USING_JUCE 0
#endif

#ifndef CHOWDSP_ALLOW_TEMPLATE_INSTANTIATIONS
#define CHOWDSP_ALLOW_TEMPLATE_INSTANTIATIONS 1
#endif

#ifndef CHOWDSP_JASSERT_IS_CASSERT
#define CHOWDSP_JASSERT_IS_CASSERT 0
#endif

#if ! CHOWDSP_USING_JUCE
#define JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED 0
#include "JUCEHelpers/juce_TargetPlatform.h"
#include "JUCEHelpers/juce_CompilerWarnings.h"
#include "JUCEHelpers/juce_ExtraDefinitions.h"
#include "JUCEHelpers/juce_MathsFunctions.h"
#include "JUCEHelpers/juce_FloatVectorOperations.h"
#include "JUCEHelpers/juce_FixedSizeFunction.h"
#include "JUCEHelpers/juce_Decibels.h"
#include "JUCEHelpers/juce_SmoothedValue.h"

#if ! JUCE_MODULE_AVAILABLE_juce_dsp
#include "JUCEHelpers/dsp/juce_ProcessSpec.h"
#include "JUCEHelpers/dsp/juce_LookupTable.h"
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wimplicit-const-int-float-conversion")
#include "JUCEHelpers/dsp/juce_FastMathApproximations.h"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#endif
#elif JUCE_VERSION < 0x070006
namespace juce
{
/** Equivalent to operator==, but suppresses float-equality warnings.

  This allows code to be explicit about float-equality checks that are known to have the correct
  semantics.
*/
template <typename Type>
constexpr bool exactlyEqual (Type a, Type b)
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")
    return a == b;
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}
} // namespace juce
#endif // CHOWDSP_USING_JUCE
#endif // DOXYGEN

/** Namespace reserved for code written for Chowdhury DSP projects. */
namespace chowdsp
{

/** Namespace reserved for code that has an unstable API or is otherwise not yet production ready. */
namespace experimental
{
}

} // namespace chowdsp

#include "third_party/types_list/include/types_list/types_list.hpp"
#include "third_party/span-lite/include/nonstd/span.hpp"

#include "DataStructures/chowdsp_AtomicHelpers.h"
#include "DataStructures/chowdsp_StringHelpers.h"
#include "DataStructures/chowdsp_GraphicsHelpers.h"
#include "DataStructures/chowdsp_ScopedValue.h"

#include "Functional/chowdsp_Bindings.h"
#include "Functional/chowdsp_EndOfScopeAction.h"
#include "Memory/chowdsp_AlignedAlloc.h"
#include "Memory/chowdsp_MemoryAliasing.h"
#include "Types/chowdsp_TypeHasCheckers.h"
#include "Types/chowdsp_TypeTraits.h"
