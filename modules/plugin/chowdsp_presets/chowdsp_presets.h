/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_presets
   vendor:        Chowdhury DSP
   version:       1.3.0
   name:          ChowDSP Presets Utilities
   description:   Presets management system for ChowDSP plugins
   dependencies:  juce_core, juce_audio_utils, chowdsp_version

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// STL includes
#include <unordered_map>

// JUCE includes
#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <chowdsp_version/chowdsp_version.h>

#include "Backend/chowdsp_Preset.h"
#include "Backend/chowdsp_PresetManager.h"
