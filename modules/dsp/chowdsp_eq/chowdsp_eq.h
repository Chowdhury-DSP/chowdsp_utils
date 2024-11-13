/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_eq
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP EQ
   description:   EQ utilities for ChowDSP plugins
   dependencies:  chowdsp_reflection, chowdsp_dsp_utils

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       GPLv3

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <chowdsp_reflection/chowdsp_reflection.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

namespace chowdsp
{
/** ChowDSP EQ utilities */
namespace EQ
{
}
} // namespace chowdsp

#include "EQ/chowdsp_EQBand.h"
#include "EQ/chowdsp_EQProcessor.h"
#include "EQ/chowdsp_EQParams.h"

#if CHOWDSP_USING_JUCE
#include "EQ/chowdsp_LinearPhaseEQ.h"
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_parameters
#include "EQ/chowdsp_StandardEQParameters.h"
#endif
