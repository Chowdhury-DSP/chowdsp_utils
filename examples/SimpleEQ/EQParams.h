#pragma once

/** Parameters to determine the filter behaviour */
struct EQParams
{
    static constexpr size_t numBands = 3;
    struct BandParams
    {
        float bandFreqHz, bandQ, bandGainDB;
        int bandType;

        bool operator== (const BandParams& other) const
        {
            return bandFreqHz == other.bandFreqHz
                   && bandQ == other.bandQ
                   && bandGainDB == other.bandGainDB
                   && bandType == other.bandType;
        }
    };

    // parameters
    std::array<BandParams, numBands> bands;

    // we need this method so we can know if two parameter sets are equivalent.
    bool operator== (const EQParams& other) const
    {
        bool result = true;
        for (size_t i = 0; i < numBands; ++i)
            result &= bands[i] == other.bands[i];

        return result;
    }
};
