/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_plugin_state
   vendor:        Chowdhury DSP
   version:       1.3.0
   name:          ChowDSP Plugin State
   description:   Utilities for managing state in a ChowDSP plugin
   dependencies:  chowdsp_core, chowdsp_serialization, chowdsp_listeners, chowdsp_parameters

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       @TODO: figure out which license we want to use here!

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

/**
 * This module is still a work-in-progress. The API is _unstable_!
 *
 * Jatin's TODO list:
 * - ParameterAttachments
 * - Listeners: think about threading situation, and test to see how it scales for large numbers of params
 * - UndoManager integration
 * - Update the rest of chowdsp_utils to use this module over APVTS when it's available
 */

#pragma once

#include <chowdsp_core/chowdsp_core.h>
#include <chowdsp_serialization/chowdsp_serialization.h>
#include <chowdsp_listeners/chowdsp_listeners.h>
#include <chowdsp_parameters/chowdsp_parameters.h>

#include "Backend/chowdsp_ParameterTypeHelpers.h"
#include "Backend/chowdsp_ParameterStateSerializer.h"
#include "Backend/chowdsp_PluginState.h"

#include "Frontend/chowdsp_ParameterAttachments.h"
