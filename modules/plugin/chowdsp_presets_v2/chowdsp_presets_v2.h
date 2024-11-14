/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_presets_v2
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Presets Utilities Version 2
   description:   Presets management system for ChowDSP plugins with chowdsp::PluginState
   dependencies:  chowdsp_core, chowdsp_json, chowdsp_version, chowdsp_plugin_state

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// JUCE includes
#include <chowdsp_core/chowdsp_core.h>
#include <chowdsp_json/chowdsp_json.h>
#include <chowdsp_version/chowdsp_version.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

#include "Backend/chowdsp_Preset.h"
#include "Backend/chowdsp_PresetState.h"
#include "Backend/chowdsp_PresetTree.h"
#include "Backend/chowdsp_PresetSaverLoader.h"
#include "Backend/chowdsp_PresetManager.h"

namespace chowdsp
{
/** Namespace for chowdsp Presets system */
namespace presets
{
    /** Interfaces for working with the chowdsp::presets::PresetManager */
    namespace frontend
    {
    }
} // namespace presets
} // namespace chowdsp

#include "Frontend/chowdsp_PresetsProgramAdapter.h"
#include "Frontend/chowdsp_PresetsFileInterface.h"
#include "Frontend/chowdsp_PresetsClipboardInterface.h"
#include "Frontend/chowdsp_PresetsMenuInterface.h"
#include "Frontend/chowdsp_PresetsTextInterface.h"
#include "Frontend/chowdsp_PresetsNextPreviousInterface.h"
#include "Frontend/chowdsp_PresetsSettingsInterface.h"
