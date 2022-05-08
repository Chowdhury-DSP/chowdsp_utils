/*
 ==============================================================================

    BEGIN_JUCE_MODULE_DECLARATION

    ID:            chowdsp_plugin_utils
    vendor:        Chowdhury DSP
    version:       1.0.0
    name:          ChowDSP Plugin Utilities
    description:   Utilities for creating ChowDSP plugins
    dependencies:  juce_events, juce_audio_basics, juce_audio_formats,
                   juce_gui_basics, juce_audio_processors, chowdsp_core, chowdsp_json

    website:       https://ccrma.stanford.edu/~jatin/chowdsp
    license:       GPLv3

    END_JUCE_MODULE_DECLARATION

 ==============================================================================
 */

#pragma once

// STL includes
#include <unordered_map>

// JUCE includes
#include <juce_events/juce_events.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <chowdsp_core/chowdsp_core.h>
#include <chowdsp_json/chowdsp_json.h>

#include "Files/chowdsp_AudioFileSaveLoadHelper.h"
#include "Files/chowdsp_FileListener.h"

#include "Logging/chowdsp_PluginLogger.h"

#include "SharedUtils/chowdsp_GlobalPluginSettings.h"
#include "SharedUtils/chowdsp_LNFAllocator.h"

#include "State/chowdsp_UIState.h"

#include "Threads/chowdsp_AudioUIBackgroundTask.h"
