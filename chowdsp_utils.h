/*
 ==============================================================================

    BEGIN_JUCE_MODULE_DECLARATION

    ID:            chowdsp_utils
    vendor:        Chowdhury DSP
    version:       1.0.0
    name:          ChowDSP Shared Code
    description:   Shared code for ChowDSP plugins and applications
    dependencies:  juce_core, juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_utils, juce_audio_processors, juce_gui_basics

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

/** Config: CHOWDSP_USE_CUSTOM_JUCE_DSP
            Use chowdsp_juce_dsp instead of juce_dsp.

            You must ensure that chowdsp_juce_dsp headers are in the include paths.
  */
#ifndef CHOWDSP_USE_CUSTOM_JUCE_DSP
#define CHOWDSP_USE_CUSTOM_JUCE_DSP 0
#endif

// STL includes
#include <unordered_map>

// JUCE includes
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

#if CHOWDSP_USE_CUSTOM_JUCE_DSP
#include <chowdsp_juce_dsp/chowdsp_juce_dsp.h>
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

#include "PluginUtils/chowdsp_PluginUtils.h"
#include "DSP/chowdsp_DSP.h"
#include "GUI/chowdsp_GUI.h"
