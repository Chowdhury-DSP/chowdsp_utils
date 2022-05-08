#include "chowdsp_math.h"

// Copy of JUCE macros for Apple vDSP library
#ifndef JUCE_USE_VDSP_FRAMEWORK
#define JUCE_USE_VDSP_FRAMEWORK 1
#endif

#if (JUCE_MAC || JUCE_IOS) && JUCE_USE_VDSP_FRAMEWORK
#include <Accelerate/Accelerate.h>
#else
#undef JUCE_USE_VDSP_FRAMEWORK
#endif

#include "Math/chowdsp_FloatVectorOperations.cpp"
