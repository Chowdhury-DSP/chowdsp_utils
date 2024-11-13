/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_units
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Units
   description:   Unit types and conversion utilities
   dependencies:

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-Clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <array>
#include <ostream>
#include <string_view>
#include <type_traits>

namespace chowdsp
{
/** Namespacing containing useful types and conversion methods for working with units */
namespace Units
{
}
} // namespace chowdsp

#include "Units/chowdsp_UnitsBase.h"
#include "Units/chowdsp_TimeUnits.h"
