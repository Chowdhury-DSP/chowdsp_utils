/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_plugin_base
   vendor:        Chowdhury DSP
   version:       0.0.1
   name:          ChowDSP Plugin Base
   description:   Base classes for ChowDSP plugins
   dependencies:  juce_core, juce_audio_basics, juce_audio_devices, juce_audio_formats,
                  juce_audio_utils, juce_audio_processors, juce_gui_basics,
                  chowdsp_parameters

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// JUCE includes
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <chowdsp_parameters/chowdsp_parameters.h>

#if JUCE_MODULE_AVAILABLE_chowdsp_presets
#include <chowdsp_presets/chowdsp_presets.h>
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_foleys
#include <chowdsp_foleys/chowdsp_foleys.h>
#endif

#include "PluginBase/chowdsp_ProgramAdapter.h"

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant", // Clang doesn't like HasAddParameters checker
                                     "-Winconsistent-missing-destructor-override")
#include "PluginBase/chowdsp_PluginBase.h"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#include "PluginBase/chowdsp_DummySynthSound.h"
#include "PluginBase/chowdsp_SynthBase.h"
