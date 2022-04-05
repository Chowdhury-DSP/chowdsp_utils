/*
 ==============================================================================

    BEGIN_JUCE_MODULE_DECLARATION

    ID:            chowdsp_plugin_utils
    vendor:        Chowdhury DSP
    version:       0.0.1
    name:          ChowDSP Plugin Utilities
    description:   Utilities for creating ChowDSP plugins
    dependencies:  juce_core, juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_utils, juce_audio_processors, juce_gui_basics

    website:       https://ccrma.stanford.edu/~jatin/chowdsp
    license:       GPLv3

    END_JUCE_MODULE_DECLARATION

 ==============================================================================
 */

#pragma once

// STL includes
#include <unordered_map>

// third party includes
#include "third_party/nlohmann/json.hpp"

// JUCE includes
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

#if CHOWDSP_USE_FOLEYS_CLASSES
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-function") // GCC doesn't like Foley's static functions
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4458) // MSVC doesn't like Foley's hiding class members
#include <foleys_gui_magic/foleys_gui_magic.h>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC
#endif

#include "Serialization/chowdsp_JSON.h"

#include "Parameters/chowdsp_ParamUtils.h"
#include "Parameters/chowdsp_ForwardingParameter.h"
#include "Version/chowdsp_VersionUtils.h"

#include "Presets/chowdsp_Preset.h"
#include "Presets/chowdsp_PresetManager.h"

#include "Files/chowdsp_AudioFileSaveLoadHelper.h"
#include "Files/chowdsp_FileListener.h"
#include "Logging/chowdsp_PluginLogger.h"

#include "SharedUtils/chowdsp_GlobalPluginSettings.h"
#include "SharedUtils/chowdsp_LNFAllocator.h"

#include "PluginBase/chowdsp_DummySynthSound.h"
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant", // Clang doesn't like HasAddParameters checker
                                     "-Winconsistent-missing-destructor-override")
#include "PluginBase/chowdsp_PluginBase.h"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#include "PluginBase/chowdsp_SynthBase.h"

#include "Threads/chowdsp_AudioUIBackgroundTask.h"
