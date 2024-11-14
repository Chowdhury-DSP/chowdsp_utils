/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_reflection
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Reflection Utilities
   description:   JUCE interface for boost::pfr
   dependencies:  chowdsp_core

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-Clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// JUCE includes
#include <chowdsp_core/chowdsp_core.h>

// third party includes
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant",
                                     "-Wshadow-field-in-constructor",
                                     "-Wfloat-equal",
                                     "-Wc++20-compat")
#include "third_party/pfr/include/pfr.hpp"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#include "third_party/nameof/include/nameof.hpp"
#include "third_party/magic_enum/include/magic_enum.hpp"

#include "ReflectionHelpers/chowdsp_EnumHelpers.h"
