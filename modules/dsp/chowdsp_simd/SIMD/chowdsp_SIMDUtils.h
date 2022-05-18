#pragma once

#define CHOWDSP_USING_XSIMD_STD(func) \
    using std::func;                  \
    using xsimd::func;

namespace chowdsp
{
/** Useful methods for working with SIMD batches via XSIMD */
namespace SIMDUtils {}
} // namespace chowdsp
