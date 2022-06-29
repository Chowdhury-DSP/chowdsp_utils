/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_filters
   vendor:        Chowdhury DSP
   version:       1.0.0
   name:          ChowDSP Filters
   description:   Filter classes for ChowDSP plugins
   dependencies:  chowdsp_core, chowdsp_dsp_data_structures, chowdsp_math

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       GPLv3

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <chowdsp_core/chowdsp_core.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>
#include <chowdsp_math/chowdsp_math.h>

#include "Utils/chowdsp_ConformalMaps.h"
#include "Utils/chowdsp_CoefficientCalculators.h"
#include "Utils/chowdsp_FilterChain.h"
#include "Utils/chowdsp_QValCalcs.h"

#include "LowerOrderFilters/chowdsp_IIRFilter.h"
#include "LowerOrderFilters/chowdsp_FirstOrderFilters.h"
#include "LowerOrderFilters/chowdsp_SecondOrderFilters.h"
#include "LowerOrderFilters/chowdsp_StateVariableFilter.h"
#include "LowerOrderFilters/chowdsp_ModFilterWrapper.h"

#include "HigherOrderFilters/chowdsp_NthOrderFilter.h"
#include "HigherOrderFilters/chowdsp_SOSFilter.h"
#include "HigherOrderFilters/chowdsp_ButterworthFilter.h"
#include "HigherOrderFilters/chowdsp_ChebyshevIIFilter.h"
#include "HigherOrderFilters/chowdsp_EllipticFilter.h"

#include "Other/chowdsp_FractionalOrderFilter.h"
#include "Other/chowdsp_HilbertFilter.h"
#include "Other/chowdsp_FIRFilter.h"
