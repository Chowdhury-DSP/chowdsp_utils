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

/** Basic parameters to determine the EQ behaviour */
template <size_t NumBands>
struct BasicEQParams
{
    struct BandParams
    {
        float bandFreqHz, bandQ, bandGainDB;
        int bandType;
        bool bandOnOff;
    };

    std::array<EQParams<BandParams>, NumBands> bands {};

    /** Sets the parameters of a chowdsp::EQ::EQProcessor, using the given parameters struct. */
    template <typename EQType>
    static void setEQParameters (EQType& eq, const BasicEQParams& params)
    {
        for (size_t i = 0; i < NumBands; ++i)
        {
            eq.setCutoffFrequency ((int) i, params.bands[i].params.bandFreqHz);
            eq.setQValue ((int) i, params.bands[i].params.bandQ);
            eq.setGainDB ((int) i, params.bands[i].params.bandGainDB);
            eq.setFilterType ((int) i, params.bands[i].params.bandType);
            eq.setBandOnOff ((int) i, params.bands[i].params.bandOnOff);
        }
    }
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
