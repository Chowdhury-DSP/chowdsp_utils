#pragma once

/** Parameters to determine the filter behaviour */
struct EQParams
{
    // parameters
    float lowCutFreqHz, lowCutQ, peakingFilterFreqHz, peakingFilterQ, peakingFilterGainDB, highCutFreqHz, highCutQ;

    // we need this method so we can know if two parameter sets are equivalent.
    bool operator== (const EQParams& other) const
    {
        return lowCutFreqHz == other.lowCutFreqHz
               && lowCutQ == other.lowCutQ
               && peakingFilterFreqHz == other.peakingFilterFreqHz
               && peakingFilterQ == other.peakingFilterQ
               && peakingFilterGainDB == other.peakingFilterGainDB
               && highCutFreqHz == other.highCutFreqHz
               && highCutFreqHz == other.highCutFreqHz
               && highCutQ == other.highCutQ;
    }
};
