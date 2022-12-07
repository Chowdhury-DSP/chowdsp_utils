/*
 ==============================================================================

    BEGIN_JUCE_MODULE_DECLARATION

    ID:            chowdsp_plugin_utils
    vendor:        Chowdhury DSP
    version:       1.3.0
    name:          ChowDSP Plugin Utilities
    description:   Utilities for creating ChowDSP plugins
    dependencies:  juce_events, juce_audio_basics, juce_audio_formats,
                   juce_gui_basics, juce_audio_processors, chowdsp_core, chowdsp_json, chowdsp_listeners

    website:       https://ccrma.stanford.edu/~jatin/chowdsp
    license:       GPLv3

    END_JUCE_MODULE_DECLARATION

 ==============================================================================
 */

#pragma once

/** Config: CHOWDSP_BAKE_TWEAKS
            When using chowdsp::TweaksFile, enabling this flag will switch to a different
            implementation of the class that uses a BinaryData version of the file,
            rather than reading/writing to a file on disk.
*/
#ifndef CHOWDSP_BAKE_TWEAKS
#define CHOWDSP_BAKE_TWEAKS 0
#endif

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
#include <chowdsp_listeners/chowdsp_listeners.h>

#include "Files/chowdsp_AudioFileSaveLoadHelper.h"
#include "Files/chowdsp_FileListener.h"
#include "Files/chowdsp_TweaksFile.h"

#include "Logging/chowdsp_PluginLogger.h"

#include "SharedUtils/chowdsp_GlobalPluginSettings.h"
#include "SharedUtils/chowdsp_LNFAllocator.h"

#include "State/chowdsp_UIState.h"

#include "Threads/chowdsp_AudioUIBackgroundTask.h"

#if JUCE_MODULE_AVAILABLE_juce_dsp
#include "Threads/chowdsp_DeferredMainThreadAction.h"
#endif
