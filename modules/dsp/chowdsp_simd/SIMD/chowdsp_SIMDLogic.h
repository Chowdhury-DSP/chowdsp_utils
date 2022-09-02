#pragma once

namespace chowdsp::SIMDUtils
{
#if ! CHOWDSP_NO_XSIMD
/** Returns true if any of the registers in the mask are true */
template <typename VecBoolType>
inline bool any (VecBoolType b)
{
    return xsimd::any (b);
}

/** Returns true if any of the registers in the mask are true */
template <>
inline bool any (bool b)
{
    return b;
}

/** Returns true if all of the registers in the mask are true */
template <typename VecBoolType>
inline bool all (VecBoolType b)
{
    return xsimd::all (b);
}

/** Returns true if all of the registers in the mask are true */
template <>
inline bool all (bool b)
{
    return b;
}

/** Ternary select operation */
template <typename T>
inline T select (bool b, const T& t, const T& f)
{
    return b ? t : f;
}

/** Ternary select operation */
template <typename T>
inline xsimd::batch<T> select (const xsimd::batch_bool<T>& b, const xsimd::batch<T>& t, const xsimd::batch<T>& f)
{
    return xsimd::select (b, t, f);
}
#else
/** Returns true if any of the registers in the mask are true */
inline bool any (bool b)
{
    return b;
}

/** Returns true if all of the registers in the mask are true */
inline bool all (bool b)
{
    return b;
}

/** Ternary select operation */
template <typename T>
inline T select (bool b, const T& t, const T& f)
{
    return b ? t : f;
}

#endif
} // namespace chowdsp::SIMDUtils
