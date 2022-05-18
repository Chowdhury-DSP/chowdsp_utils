/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_reverb
   vendor:        Chowdhury DSP
   version:       1.0.0
   name:          ChowDSP Reverb
   description:   Reverb utilities for ChowDSP plugins
   dependencies:  chowdsp_dsp_utils

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       GPLv3

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

namespace chowdsp
{
/** ChowDSP classes for creating reverb effects */
namespace Reverb
{
}
} // namespace chowdsp

#include "Reverb/chowdsp_Diffuser.h"
#include "Reverb/chowdsp_FDN.h"
