/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_reflection
   vendor:        Chowdhury DSP
   version:       1.0.0
   name:          ChowDSP Reflection Utilities
   description:   JUCE interface for boost::pfr
   dependencies:  juce_core

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-Clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// JUCE includes
#include <juce_core/juce_core.h>

// third party includes
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")
#include "third_party/pfr/include/pfr.hpp"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
