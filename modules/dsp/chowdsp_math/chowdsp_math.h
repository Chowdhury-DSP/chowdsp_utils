/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_math
   vendor:        Chowdhury DSP
   version:       1.3.0
   name:          ChowDSP Math Utilities
   description:   Math utilities for ChowDSP plugins
   dependencies:  chowdsp_simd

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <numeric>

#include <chowdsp_simd/chowdsp_simd.h>

#include "third_party/gcem/include/gcem.hpp"

#include "Math/chowdsp_Ratio.h"

#include "Math/chowdsp_Combinatorics.h"
#include "Math/chowdsp_FloatVectorOperations.h"
#include "Math/chowdsp_MatrixOps.h"
#include "Math/chowdsp_Polynomials.h"
#include "Math/chowdsp_Power.h"
#include "Math/chowdsp_OtherMathOps.h"
#include "Math/chowdsp_JacobiElliptic.h"
#include "Math/chowdsp_Polylogarithm.h"
#include "Math/chowdsp_BufferMath.h"
#include "Math/chowdsp_TanhIntegrals.h"
