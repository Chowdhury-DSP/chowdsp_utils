#include "chowdsp_core.h"

#if ! CHOWDSP_USING_JUCE
#include "JUCEHelpers/juce_FloatVectorOperations.cpp"
#endif

#if ! CHOWDSP_USING_JUCE || ! JUCE_MODULE_AVAILABLE_juce_dsp
#include "JUCEHelpers/dsp/juce_LookupTable.cpp"
#endif
