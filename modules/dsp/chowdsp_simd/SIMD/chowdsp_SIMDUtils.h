#pragma once

#define CHOWDSP_USING_XSIMD_STD(func) \
    using std::func;                  \
    using xsimd::func;

/** Useful methds for working with SIMD batches via XSIMD */
namespace chowdsp::SIMDUtils
{
} // namespace chowdsp::SIMDUtils
