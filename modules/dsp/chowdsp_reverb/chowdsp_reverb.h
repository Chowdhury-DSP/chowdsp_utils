/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_reverb
   vendor:        Chowdhury DSP
   version:       1.3.0
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
    /**
     * Internal tools for creating a Dattorr-style reverb.
     *
     * This implementation is still a bit of a work-in-progress, so
     * everything in this namespace should be considered an unstable API
     * for the moment.
     *
     * Reference: https://ccrma.stanford.edu/~dattorro/EffectDesignPart1.pdf
     */
    namespace Dattorro
    {
    }
} // namespace Reverb
} // namespace chowdsp

#include "Reverb/chowdsp_ConvolutionDiffuser.h"
#include "Reverb/chowdsp_Diffuser.h"
#include "Reverb/chowdsp_FDN.h"

#include "Reverb/chowdsp_DattorroLattice.h"
#include "Reverb/chowdsp_DattorroInputNetwork.h"
#include "Reverb/chowdsp_DattorroTankNetwork.h"
