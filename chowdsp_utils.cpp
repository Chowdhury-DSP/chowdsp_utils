#include "chowdsp_utils.h"

#if defined(_M_ARM) || defined (__arm64__) || defined (__aarch64__)
#include "DSP/SIMD/neon_SIMD_Native_Ops.cpp"
#endif

#include "DSP/chowdsp_DSP.cpp"
#include "GUI/chowdsp_GUI.cpp"
#include "PluginUtils/chowdsp_ParamUtils.cpp"
