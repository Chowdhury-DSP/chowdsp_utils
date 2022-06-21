/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_parameters
   vendor:        Chowdhury DSP
   version:       1.0.0
   name:          ChowDSP Parameters
   description:   Parameter utilities for ChowDSP plugins
   dependencies:  juce_core, juce_audio_processors

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// JUCE includes
#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace chowdsp
{
/** Useful methods for creating juce::AudioProcessorParameter's */
namespace ParamUtils
{
}
} // namespace chowdsp

#include "ParamUtils/chowdsp_ParameterTypes.h"
#include "ParamUtils/chowdsp_ParamUtils.h"
#include "ParamUtils/chowdsp_ForwardingParameter.h"

#if JUCE_MODULE_AVAILABLE_chowdsp_rhythm
#include "ParamUtils/chowdsp_RhythmParameter.h"
#endif
