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

#if CHOWDSP_USE_FOLEYS_CLASSES
#include <juce_core/juce_core.h>
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-function") // GCC doesn't like Foley's static functions
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4458) // MSVC doesn't like Foley's hiding class members
#include <foleys_gui_magic/foleys_gui_magic.h>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC
#else
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#endif

// Plugin utils
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant", // Clang doesn't like HasAddParameters checker
                                     "-Winconsistent-missing-destructor-override")
#include "PluginUtils/chowdsp_PluginBase.h"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#include "PluginUtils/chowdsp_DummySynthSound.h"
#include "PluginUtils/chowdsp_ParamUtils.h"
#include "PluginUtils/chowdsp_SynthBase.h"

#include "DSP/chowdsp_DSP.h"
#include "GUI/chowdsp_GUI.h"
