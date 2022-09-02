#pragma once

#if ! CHOWDSP_NO_XSIMD
/** Simplified way of `using` the same function from both `std::` and `xsimd::` */
#define CHOWDSP_USING_XSIMD_STD(func) \
    using std::func;                  \
    using xsimd::func
#else
#define CHOWDSP_USING_XSIMD_STD(func) \
    using std::func
#endif

namespace chowdsp
{
/** Useful methods for working with SIMD batches via XSIMD */
namespace SIMDUtils
{
}
} // namespace chowdsp
