/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_parameters
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Parameters
   description:   Parameter utilities for ChowDSP plugins
   dependencies:  chowdsp_data_structures, chowdsp_reflection, juce_audio_processors

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// JUCE includes
#include <juce_audio_processors/juce_audio_processors.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>
#include <chowdsp_reflection/chowdsp_reflection.h>

namespace chowdsp
{
/** Useful methods for creating juce::AudioProcessorParameter's */
namespace ParamUtils
{
}
} // namespace chowdsp

#include "ParamUtils/chowdsp_ParameterConversions.h"
#include "ParamUtils/chowdsp_ParameterTypes.h"
#include "ParamUtils/chowdsp_ParamUtils.h"
#include "Forwarding/chowdsp_ForwardingParameter.h"
#include "Forwarding/chowdsp_ForwardingParametersManager.h"

#if JUCE_MODULE_AVAILABLE_chowdsp_rhythm
#include "ParamUtils/chowdsp_RhythmParameter.h"
#endif
