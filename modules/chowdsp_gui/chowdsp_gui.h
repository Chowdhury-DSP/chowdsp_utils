/*
 ==============================================================================

    BEGIN_JUCE_MODULE_DECLARATION

    ID:            chowdsp_gui
    vendor:        Chowdhury DSP
    version:       0.0.1
    name:          ChowDSP GUI Utilities
    description:   Commonly used GUI utilities for ChowDSP plugins
    dependencies:  juce_core, juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_utils, juce_audio_processors, juce_gui_basics

    website:       https://ccrma.stanford.edu/~jatin/chowdsp
    license:       GPLv3

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

/** Config: CHOWDSP_NEEDS_PRESETS_COMPONENT
 *          Disable this flag to skip compiling the presets component
 */
#ifndef CHOWDSP_NEEDS_PRESETS_COMPONENT
#define CHOWDSP_NEEDS_PRESETS_COMPONENT 1
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

#if JUCE_MODULE_AVAILABLE_juce_opengl
#include <juce_opengl/juce_opengl.h>
#endif

#if CHOWDSP_USE_FOLEYS_CLASSES
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-function") // GCC doesn't like Foley's static functions
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4458) // MSVC doesn't like Foley's hiding class members
#include <foleys_gui_magic/foleys_gui_magic.h>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC
#endif

#if FOLEYS_ENABLE_BINARY_DATA
#include "BinaryData.h"
#endif

#include "Assets/chowdsp_BinaryData.h"

#include "LookAndFeel/chowdsp_ChowLNF.h"

#include "Helpers/chowdsp_LongPressActionHelper.h"
#include "Helpers/chowdsp_PopupMenuHelper.h"
#include "Helpers/chowdsp_OpenGLHelper.h"

#include "PluginComponents/chowdsp_SystemInfo.h"
#include "PluginComponents/chowdsp_InfoComp.h"
#include "PluginComponents/chowdsp_TitleComp.h"
#include "PluginComponents/chowdsp_TooltipComp.h"
#include "PluginComponents/chowdsp_WindowInPlugin.h"

#if CHOWDSP_NEEDS_PRESETS_COMPONENT
#include "Presets/chowdsp_PresetsComp.h"
#endif

#include "MagicGUI/chowdsp_MagicGUI.h"
