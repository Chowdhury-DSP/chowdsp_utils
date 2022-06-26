/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_simd
   vendor:        Chowdhury DSP
   version:       1.0.0
   name:          ChowDSP DSP SIMD
   description:   JUCE interface for XSIMD, with a few extra useful methods
   dependencies:  juce_core, juce_audio_basics

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// STL includes
#include <cassert>

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

// Third-party includes
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-align",
                                     "-Wimplicit-int-conversion",
                                     "-Wshadow",
                                     "-Wshadow-field",
                                     "-Wsign-conversion",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wsign-compare",
                                     "-Wc++98-compat-extra-semi",
                                     "-Wshorten-64-to-32")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4244)
#include "third_party/xsimd/include/xsimd/xsimd.hpp"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

#include "SIMD/chowdsp_SampleTypeHelpers.h"
#include "SIMD/chowdsp_SIMDUtils.h"
#include "SIMD/chowdsp_SIMDLogic.h"
#include "SIMD/chowdsp_SIMDSpecialMath.h"
#include "SIMD/chowdsp_SIMDDecibels.h"
#include "SIMD/chowdsp_SIMDSmoothedValue.h"
#include "SIMD/chowdsp_SIMDComplexMathOps.h"
