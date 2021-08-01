/*
 ==============================================================================

    BEGIN_JUCE_MODULE_DECLARATION

    ID:            chowdsp_utils
    vendor:        Chowdhury DSP
    version:       1.0.0
    name:          ChowDSP Shared Code
    description:   Shared code for ChowDSP plugins and applications
    dependencies:  juce_core, juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_utils, juce_audio_processors, juce_gui_basics, juce_dsp

    website:       https://ccrma.stanford.edu/~jatin/chowdsp
    license:       Dual license: non commercial under BSD V2 3-clause

    END_JUCE_MODULE_DECLARATION

 ==============================================================================
 */

#pragma once

/** Config: CHOWDSP_USE_FOLEYS_CLASSES
 *          Enables the module to use foleys_gui_magic classes. Set this to 0 if you're not using foleys_gui_magic.
 */
#ifndef CHOWDSP_USE_FOLEYS_CLASSES
#define CHOWDSP_USE_FOLEYS_CLASSES 1
#endif

/** Config: CHOWDSP_USE_LIBSAMPLERATE
            Enable libsamplerate wrappers for resampling processors.

            You must link against libsamplerate, and make sure that libsamplerate headers
            are in the include paths. You must respect the libsamplerate license when enabling this option.
  */
#ifndef CHOWDSP_USE_LIBSAMPLERATE
#define CHOWDSP_USE_LIBSAMPLERATE 0
#endif

/** Config: CHOWDSP_USE_XSIMD
            Enable XSIMD library for accelerated SIMD functions.

            You must ensure that xsimd headers are in the include paths.
            You must respect the xsimd license when enabling this option.
  */
#ifndef CHOWDSP_USE_XSIMD
#define CHOWDSP_USE_XSIMD 0
#endif

// JUCE includes
#include <juce_core/juce_core.h>

// custom behaviour for SIMD on ARM processors
#if defined(_M_ARM) || defined (__arm64__) || defined (__aarch64__)
#ifndef JUCE_VECTOR_CALLTYPE
 // __vectorcall does not work on 64-bit due to internal compiler error in
 // release mode in both VS2015 and VS2017. Re-enable when Microsoft fixes this
 #if _MSC_VER && JUCE_USE_SIMD && ! (defined(_M_X64) || defined(__amd64__))
  #define JUCE_VECTOR_CALLTYPE __vectorcall
 #else
  #define JUCE_VECTOR_CALLTYPE
 #endif
#endif

#define JUCE_SIMD_TMP JUCE_USE_SIMD
#undef JUCE_USE_SIMD

#include <arm_neon.h>
#include <complex>
#include "DSP/SIMD/fallback_SIMD_Native_Ops.h"
#include "DSP/SIMD/neon_SIMD_Native_Ops.h"

#define JUCE_USE_SIMD JUCE_SIMD_TMP
#undef JUCE_SIMD_TMP

#define SIMDInternal chowdsp::SIMDInternal
#include <juce_dsp/containers/juce_SIMDRegister.h>
#undef SIMDInternal

#include <juce_dsp/juce_dsp.h>

#else
#include <juce_dsp/juce_dsp.h>
#endif

#if CHOWDSP_USE_FOLEYS_CLASSES
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-function") // GCC doesn't like Foley's static functions
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4458) // MSVC doesn't like Foley's hiding class members
#include <foleys_gui_magic/foleys_gui_magic.h>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC
#endif

#if CHOWDSP_USE_LIBSAMPLERATE
#include <samplerate.h>
#endif

#if CHOWDSP_USE_XSIMD
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-align",
                                     "-Wimplicit-int-conversion",
                                     "-Wshadow",
                                     "-Wsign-conversion",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wc++98-compat-extra-semi")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4244)
#include <cassert>
#include <xsimd/xsimd.hpp>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC
#endif

// Plugin utils
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant", // Clang doesn't like HasAddParameters checker
                                     "-Winconsistent-missing-destructor-override")
#include "PluginUtils/chowdsp_PluginBase.h"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#include "PluginUtils/chowdsp_DummySynthSound.h"
#include "PluginUtils/chowdsp_ParamUtils.h"
#include "PluginUtils/chowdsp_SynthBase.h"
#include "PluginUtils/chowdsp_VersionUtils.h"

#include "DSP/chowdsp_DSP.h"
#include "GUI/chowdsp_GUI.h"
