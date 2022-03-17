#pragma once

struct EQParams
{
    float lowCutFreqHz, lowCutQ, peakingFilterFreqHz, peakingFilterQ, peakingFilterGainDB, highCutFreqHz, highCutQ;

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
