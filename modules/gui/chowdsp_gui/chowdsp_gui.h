/*
 ==============================================================================

    BEGIN_JUCE_MODULE_DECLARATION

    ID:            chowdsp_gui
    vendor:        Chowdhury DSP
    version:       2.1.0
    name:          ChowDSP GUI Utilities
    description:   Commonly used GUI utilities for ChowDSP plugins
    dependencies:  juce_core, juce_audio_processors, juce_gui_basics, chowdsp_core, chowdsp_data_structures

    website:       https://ccrma.stanford.edu/~jatin/chowdsp
    license:       GPLv3

    END_JUCE_MODULE_DECLARATION

 ==============================================================================
 */

#pragma once

/** Config: CHOWDSP_ENABLE_OPEN_GL_CONTEXT
            If selected an juce OpenGLCOntext is attached. Not a big difference on OSX, but vital on Windows.
  */
#ifndef CHOWDSP_ENABLE_OPEN_GL_CONTEXT
#define CHOWDSP_ENABLE_OPEN_GL_CONTEXT 1
#endif

// JUCE includes
#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <chowdsp_core/chowdsp_core.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

#if JUCE_MODULE_AVAILABLE_juce_opengl && CHOWDSP_ENABLE_OPEN_GL_CONTEXT
#define CHOWDSP_OPENGL_IS_AVAILABLE 1
#include <juce_opengl/juce_opengl.h>
#else
#define CHOWDSP_OPENGL_IS_AVAILABLE 0
#endif

#include "Assets/chowdsp_BinaryData.h"

#include "LookAndFeel/chowdsp_ChowLNF.h"

#include "Helpers/chowdsp_ComponentArena.h"
#include "Helpers/chowdsp_LongPressActionHelper.h"
#include "Helpers/chowdsp_PopupMenuHelper.h"
#include "Helpers/chowdsp_OpenGLHelper.h"
#include "Helpers/chowdsp_HostContextProvider.h"

#include "InfoUtils/chowdsp_InfoProvider.h"
#include "InfoUtils/chowdsp_SystemInfo.h"

#include "PluginComponents/chowdsp_CPUMeter.h"
#include "PluginComponents/chowdsp_InfoComp.h"
#include "PluginComponents/chowdsp_TitleComp.h"
#include "PluginComponents/chowdsp_TooltipComp.h"
#include "PluginComponents/chowdsp_WindowInPlugin.h"

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
#include "PluginComponents/chowdsp_ParametersView.h"
#endif

#if CHOWDSP_USING_JUCE && JUCE_MODULE_AVAILABLE_chowdsp_dsp_utils
#include "PluginComponents/chowdsp_OversamplingMenu.h"
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_presets
#include "Presets/chowdsp_PresetsComp.h"
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_presets_v2
#include "Presets/chowdsp_PresetsComponent.h"
#endif
