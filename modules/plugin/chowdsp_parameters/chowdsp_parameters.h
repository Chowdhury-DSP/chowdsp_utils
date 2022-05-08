/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_parameters
   vendor:        Chowdhury DSP
   version:       0.0.1
   name:          ChowDSP Parameters
   description:   Parameter utilities for ChowDSP plugins
   dependencies:  juce_core, juce_audio_basics, juce_audio_devices, juce_audio_formats,
                  juce_audio_utils, juce_audio_processors, juce_gui_basics

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

#include "ParamUtils/chowdsp_ParamUtils.h"
#include "ParamUtils/chowdsp_ForwardingParameter.h"

#if JUCE_MODULE_AVAILABLE_chowdsp_rhythms
#include "ParamUtils/chowdsp_RhythmParameter.h"
#endif
