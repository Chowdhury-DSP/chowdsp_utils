#include "chowdsp_SpectrumPlotBase.h"

namespace chowdsp
{
SpectrumPlotBase::SpectrumPlotBase (SpectrumPlotParams&& plotParams) : params (std::move (plotParams))
{
}

float SpectrumPlotBase::getXCoordinateForFrequency (float freqHz) const
{
    const auto xNorm = std::log (freqHz / params.minFrequencyHz) / params.frequencyScale;
    return xNorm * (float) getWidth();
}

float SpectrumPlotBase::getFrequencyForXCoordinate (float xCoord) const
{
    const auto xNorm = xCoord / (float) getWidth();
    return std::exp (xNorm * params.frequencyScale) * params.minFrequencyHz;
}

float SpectrumPlotBase::getYCoordinateForDecibels (float gainDB) const
{
    return (float) getHeight() * (params.maxMagnitudeDB - gainDB) / params.rangeDB;
}
} // namespace chowdsp
