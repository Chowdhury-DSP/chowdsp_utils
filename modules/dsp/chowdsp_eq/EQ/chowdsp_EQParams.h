#pragma once

namespace chowdsp::EQ
{
/**
 * Useful struct for wrapping EQ params, with equality and inequality operations defined
 * for all the members.
 */
template <typename ParamsType>
struct EQParams
{
    ParamsType params {};
};

template <typename ParamsType>
inline bool operator== (const EQParams<ParamsType>& x, const EQParams<ParamsType>& y)
{
    return pfr::eq_fields (x.params, y.params);
}

template <typename ParamsType>
inline bool operator== (const EQParams<ParamsType>& x, const ParamsType& y)
{
    return pfr::eq_fields (x.params, y);
}

template <typename ParamsType>
inline bool operator== (const ParamsType& x, const EQParams<ParamsType>& y)
{
    return pfr::eq_fields (x, y.params);
}

template <typename ParamsType>
inline bool operator!= (const EQParams<ParamsType>& x, const EQParams<ParamsType>& y)
{
    return pfr::ne_fields (x.params, y.params);
}

template <typename ParamsType>
inline bool operator!= (const EQParams<ParamsType>& x, const ParamsType& y)
{
    return pfr::ne_fields (x.params, y);
}

template <typename ParamsType>
inline bool operator!= (const ParamsType& x, const EQParams<ParamsType>& y)
{
    return pfr::ne_fields (x, y.params);
}
} // namespace chowdsp::EQ
