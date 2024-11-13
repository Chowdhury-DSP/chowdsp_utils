/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_math
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Math Utilities
   description:   Math utilities for ChowDSP plugins
   dependencies:  chowdsp_data_structures, chowdsp_simd

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <numeric>

#include <chowdsp_data_structures/chowdsp_data_structures.h>
#include <chowdsp_simd/chowdsp_simd.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wimplicit-int-float-conversion", "-Wfloat-equal")
#include "third_party/gcem/include/gcem.hpp"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#include "Math/chowdsp_Ratio.h"

#include "Math/chowdsp_Combinatorics.h"
#include "Math/chowdsp_FloatVectorOperations.h"
#include "Math/chowdsp_OtherMathOps.h"
#include "Math/chowdsp_MatrixOps.h"
#include "Math/chowdsp_Polynomials.h"
#include "Math/chowdsp_ChebyshevPolynomials.h"
#include "Math/chowdsp_Power.h"
#include "Math/chowdsp_RandomFloat.h"
#include "Math/chowdsp_JacobiElliptic.h"
#include "Math/chowdsp_Polylogarithm.h"
#include "Math/chowdsp_TanhIntegrals.h"
#include "Math/chowdsp_LogApprox.h"
#include "Math/chowdsp_PowApprox.h"
#include "Math/chowdsp_DecibelsApprox.h"
#include "Math/chowdsp_TrigApprox.h"

#if JUCE_MODULE_AVAILABLE_chowdsp_buffers
#include <chowdsp_buffers/chowdsp_buffers.h>
#include "Math/chowdsp_BufferMath.h"
#endif
