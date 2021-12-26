#include "chowdsp_dsp.h"

// Copy of JUCE macros for Apple vDSP library
#ifndef JUCE_USE_VDSP_FRAMEWORK
#define JUCE_USE_VDSP_FRAMEWORK 1
#endif

#if (JUCE_MAC || JUCE_IOS) && JUCE_USE_VDSP_FRAMEWORK
#include <Accelerate/Accelerate.h>
#else
#undef JUCE_USE_VDSP_FRAMEWORK
#endif

// math helpers
#include "Math/chowdsp_FloatVectorOperations.cpp"

// data structures
#include "DataStructures/chowdsp_SmoothedBufferValue.cpp"

// filters
#include "Filters/chowdsp_StateVariableFilter.cpp"

// modal tools
#include "Modal/chowdsp_ModalFilter.cpp"

// resamplers
#include "Resampling/chowdsp_SRCResampler.cpp"

// convolution
#include "Convolution/chowdsp_ConvolutionEngine.cpp"

// processors
#include "Processors/chowdsp_Panner.cpp"
