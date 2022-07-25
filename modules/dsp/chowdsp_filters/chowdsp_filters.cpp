#if defined(__APPLE_CPP__) || defined(__APPLE_CC__)
// include <Accelerate> on Apple devices so we can use vDSP_dotpr
#define Point CarbonDummyPointName
#define Component CarbonDummyCompName
#include <Accelerate/Accelerate.h>
#undef Component
#undef Point
#endif

#include "chowdsp_filters.h"

#include "Other/chowdsp_FIRFilter.cpp"
