/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_plugin_base
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Plugin Base
   description:   Base classes for ChowDSP plugins
   dependencies:  chowdsp_core, chowdsp_parameters

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       GPLv3

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// JUCE includes
#include <chowdsp_core/chowdsp_core.h>
#include <chowdsp_parameters/chowdsp_parameters.h>

#if JUCE_MODULE_AVAILABLE_chowdsp_foleys
#include <chowdsp_foleys/chowdsp_foleys.h>
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>
#endif

#include "PluginBase/chowdsp_ProgramAdapter.h"

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant", // Clang doesn't like HasAddParameters checker
                                     "-Winconsistent-missing-destructor-override")
#include "PluginBase/chowdsp_PluginBase.h"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#include "PluginBase/chowdsp_DummySynthSound.h"
#include "PluginBase/chowdsp_SynthBase.h"
#include "PluginBase/chowdsp_PluginDiagnosticInfo.h"
