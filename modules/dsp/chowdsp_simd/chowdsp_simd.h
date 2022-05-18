/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_simd
   vendor:        Chowdhury DSP
   version:       1.0.0
   name:          ChowDSP DSP Utilities
   description:   Commonly used DSP utilities for ChowDSP plugins
   dependencies:  juce_core

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

/** Config: CHOWDSP_USE_CUSTOM_JUCE_DSP
            Use chowdsp_juce_dsp instead of juce_dsp.

You must ensure that chowdsp_juce_dsp headers are in the include paths.
*/
#ifndef CHOWDSP_USE_CUSTOM_JUCE_DSP
#define CHOWDSP_USE_CUSTOM_JUCE_DSP 0
#endif

// STL includes
#include <cassert>

#include <juce_core/juce_core.h>

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

#if CHOWDSP_USE_CUSTOM_JUCE_DSP
#include <chowdsp_juce_dsp/chowdsp_juce_dsp.h>
#else
#include <juce_dsp/juce_dsp.h>
#endif

#include "SIMD/chowdsp_SampleTypeHelpers.h"
#include "SIMD/chowdsp_SIMDUtils.h"
#include "SIMD/chowdsp_SIMDLogic.h"
#include "SIMD/chowdsp_SIMDSpecialMath.h"
#include "SIMD/chowdsp_SIMDDecibels.h"
#include "SIMD/chowdsp_SIMDSmoothedValue.h"
#include "SIMD/chowdsp_SIMDComplexMathOps.h"
