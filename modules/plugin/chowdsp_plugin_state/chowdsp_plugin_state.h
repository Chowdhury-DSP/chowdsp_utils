/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_plugin_state
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Plugin State
   description:   Utilities for managing state in a ChowDSP plugin
   dependencies:  chowdsp_core, chowdsp_serialization, chowdsp_listeners, chowdsp_version, chowdsp_parameters

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-Clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <chowdsp_core/chowdsp_core.h>
#include <chowdsp_serialization/chowdsp_serialization.h>
#include <chowdsp_listeners/chowdsp_listeners.h>
#include <chowdsp_version/chowdsp_version.h>
#include <chowdsp_parameters/chowdsp_parameters.h>
#include <variant>

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4324) // struct was padded warning

#include "Backend/chowdsp_StateValue.h"
#include "Backend/chowdsp_NonParamState.h"

#include "Backend/chowdsp_ParameterTypeHelpers.h"
#include "Backend/chowdsp_ParamHolder.h"
#include "Backend/chowdsp_ParameterListeners.h"
#include "Backend/chowdsp_PluginState.h"
#include "Backend/chowdsp_PluginStateImpl.h"

#include "Frontend/chowdsp_ParameterAttachment.h"
#include "Frontend/chowdsp_SliderAttachment.h"
#include "Frontend/chowdsp_SliderChoiceAttachment.h"
#include "Frontend/chowdsp_ComboBoxAttachment.h"
#include "Frontend/chowdsp_ButtonAttachment.h"

JUCE_END_IGNORE_WARNINGS_MSVC
