#pragma once

namespace chowdsp
{
/** log2 for integer values */
template <typename IntType>
constexpr int log2 (IntType n)
{
    return ((n == 1) ? 0 : 1 + (int) std::log2 (n / 2));
}
} // namespace chowdsp
