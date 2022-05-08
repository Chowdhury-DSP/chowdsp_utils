#pragma once

/** Parameters to determine the filter behaviour */
struct EQParams
{
    static constexpr size_t numBands = 3;
    struct BandParams
    {
        float bandFreqHz, bandQ, bandGainDB;
        int bandType;
        bool bandOnOff;
    };

    // parameters
    std::array<chowdsp::EQ::EQParams<BandParams>, numBands> bands;
};
