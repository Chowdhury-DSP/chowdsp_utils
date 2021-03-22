/*
 ==============================================================================

    BEGIN_JUCE_MODULE_DECLARATION

    ID:            chowdsp_utils
    vendor:        Chowdhury DSP
    version:       1.0.0
    name:          ChowDSP Shared Code
    description:   Shared code for ChowDSP plugins and applications
    dependencies:  juce_core, juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_utils, juce_audio_processors, juce_gui_basics, juce_dsp, foleys_gui_magic

    website:       https://ccrma.stanford.edu/~jatin/chowdsp
    license:       Dual license: non commercial under BSD V2 3-clause

    END_JUCE_MODULE_DECLARATION

 ==============================================================================
 */

#pragma once

#include <juce_core/juce_core.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-function") // GCC doesn't like Foley's static functions
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4458) // MSVC doesn't like Foley's hiding class members
#include <foleys_gui_magic/foleys_gui_magic.h>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

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
