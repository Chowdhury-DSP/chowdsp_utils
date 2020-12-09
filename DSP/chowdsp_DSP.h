#pragma once

#include "chowdsp_GainProcessor.h"

// delay
#include "Delay/chowdsp_DelayInterpolation.h"
#include "Delay/chowdsp_DelayLine.h"
#include "Delay/chowdsp_PitchShift.h"

// filter tools
#include "Filters/chowdsp_IIRFilter.h"
#include "Filters/chowdsp_juce_IIRFilter.h"
#include "Filters/chowdsp_BilinearUtils.h"
#include "Filters/chowdsp_Shelf.h"

// modal tools
#include "Modal/chowdsp_ModalFilter.h"

// sound sources
#include "Sources/chowdsp_SineWave.h"
#include "Sources/chowdsp_Noise.h"

// WDF tools
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wshadow-field", // Ignore Clang warnings for WDFs
                                     "-Wshadow-field-in-constructor",
                                     "-Winconsistent-missing-destructor-override")
#include "WDF/wdf.h"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
