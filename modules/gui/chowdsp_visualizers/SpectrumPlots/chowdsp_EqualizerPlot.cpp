#include "chowdsp_EqualizerPlot.h"

namespace chowdsp::EQ
{
EqualizerPlot::EqualizerPlot (int numBands, SpectrumPlotParams&& plotParams)
    : SpectrumPlotBase (std::move (plotParams)),
      filterPlots ((size_t) numBands)
{
    for (auto& plot : filterPlots)
        plot.plot = std::make_unique<EQFilterPlot>();
}

void EqualizerPlot::setFilterType (int bandIndex, EQPlotFilterType type)
{
    auto& plot = filterPlots[(size_t) bandIndex].plot;
    switch (type)
    {
        case EQPlotFilterType::LPF1:
            plot = std::make_unique<LPF1Plot>();
            break;
        case EQPlotFilterType::LPF2:
            plot = std::make_unique<LPF2Plot>();
            break;
        case EQPlotFilterType::LPF3:
            plot = std::make_unique<HigherOrderLPFPlot<3>>();
            break;
        case EQPlotFilterType::LPF4:
            plot = std::make_unique<HigherOrderLPFPlot<4>>();
            break;
        case EQPlotFilterType::LPF5:
            plot = std::make_unique<HigherOrderLPFPlot<5>>();
            break;
        case EQPlotFilterType::LPF6:
            plot = std::make_unique<HigherOrderLPFPlot<6>>();
            break;
        case EQPlotFilterType::LPF7:
            plot = std::make_unique<HigherOrderLPFPlot<7>>();
            break;
        case EQPlotFilterType::LPF8:
            plot = std::make_unique<HigherOrderLPFPlot<8>>();
            break;
        case EQPlotFilterType::LPF9:
            plot = std::make_unique<HigherOrderLPFPlot<9>>();
            break;
        case EQPlotFilterType::LPF10:
            plot = std::make_unique<HigherOrderLPFPlot<10>>();
            break;
        case EQPlotFilterType::LPF11:
            plot = std::make_unique<HigherOrderLPFPlot<11>>();
            break;
        case EQPlotFilterType::LPF12:
            plot = std::make_unique<HigherOrderLPFPlot<12>>();
            break;

        case EQPlotFilterType::HPF1:
            plot = std::make_unique<HPF1Plot>();
            break;
        case EQPlotFilterType::HPF2:
            plot = std::make_unique<HPF2Plot>();
            break;
        case EQPlotFilterType::HPF3:
            plot = std::make_unique<HigherOrderHPFPlot<3>>();
            break;
        case EQPlotFilterType::HPF4:
            plot = std::make_unique<HigherOrderHPFPlot<4>>();
            break;
        case EQPlotFilterType::HPF5:
            plot = std::make_unique<HigherOrderHPFPlot<5>>();
            break;
        case EQPlotFilterType::HPF6:
            plot = std::make_unique<HigherOrderHPFPlot<6>>();
            break;
        case EQPlotFilterType::HPF7:
            plot = std::make_unique<HigherOrderHPFPlot<7>>();
            break;
        case EQPlotFilterType::HPF8:
            plot = std::make_unique<HigherOrderHPFPlot<8>>();
            break;
        case EQPlotFilterType::HPF9:
            plot = std::make_unique<HigherOrderHPFPlot<9>>();
            break;
        case EQPlotFilterType::HPF10:
            plot = std::make_unique<HigherOrderHPFPlot<10>>();
            break;
        case EQPlotFilterType::HPF11:
            plot = std::make_unique<HigherOrderHPFPlot<11>>();
            break;
        case EQPlotFilterType::HPF12:
            plot = std::make_unique<HigherOrderHPFPlot<12>>();
            break;

        case EQPlotFilterType::BPF2:
            plot = std::make_unique<BPF2Plot>();
            break;
        case EQPlotFilterType::Bell:
            plot = std::make_unique<BellPlot>();
            break;
        case EQPlotFilterType::Notch:
            plot = std::make_unique<NotchPlot>();
            break;
        case EQPlotFilterType::LowShelf:
            plot = std::make_unique<LowShelfPlot>();
            break;
        case EQPlotFilterType::HighShelf:
            plot = std::make_unique<HighShelfPlot>();
            break;

        default:
            plot = std::make_unique<EQFilterPlot>();
            break;
    }
}

void EqualizerPlot::setCutoffParameter (int bandIndex, float cutoffHz)
{
    filterPlots[(size_t) bandIndex].plot->setCutoffFrequency (cutoffHz);
}

void EqualizerPlot::setQParameter (int bandIndex, float qValue)
{
    filterPlots[(size_t) bandIndex].plot->setQValue (qValue);
}

void EqualizerPlot::setGainDBParameter (int bandIndex, float gainDB)
{
    filterPlots[(size_t) bandIndex].plot->setGainDecibels (gainDB);
}

void EqualizerPlot::updateFilterPlotPath (int bandIndex)
{
    const auto width = getWidth();

    const auto& plot = *filterPlots[(size_t) bandIndex].plot;
    auto& plotData = filterPlots[(size_t) bandIndex].plotData;
    plotData.resize ((size_t) width);

    auto& path = filterPlots[(size_t) bandIndex].plotPath;
    path.clear();
    path.preallocateSpace (width * 3);

    const auto getPointForXCoord = [this, &plot, &plotData] (int x) -> juce::Point<float>
    {
        const auto frequency = getFrequencyForXCoordinate ((float) x);
        plotData[(size_t) x] = plot.getMagnitudeForFrequency (frequency);
        const auto magDB = juce::Decibels::gainToDecibels (plotData[(size_t) x]);
        return { (float) x, getYCoordinateForDecibels (magDB) };
    };

    path.startNewSubPath (getPointForXCoord (0.0f));
    for (int x = 1; x < width; ++x)
        path.lineTo (getPointForXCoord (x));

    updateMasterFilterPlotPath();

    repaint();
}

void EqualizerPlot::updateMasterFilterPlotPath()
{
    const auto width = getWidth();

    masterFilterPlotPath.clear();
    masterFilterPlotPath.preallocateSpace (width * 3);

    const auto getPointForXCoord = [this] (int x) -> juce::Point<float>
    {
        float magLinear = 1.0f;
        for (auto& filterPlot : filterPlots)
        {
            if (filterPlot.plotData.size() <= (size_t) x)
                break;

            magLinear *= filterPlot.plotData[(size_t) x];
        }

        const auto magDB = juce::Decibels::gainToDecibels (magLinear);
        return { (float) x, getYCoordinateForDecibels (magDB) };
    };

    masterFilterPlotPath.startNewSubPath (getPointForXCoord (0.0f));
    for (int x = 1; x < width; ++x)
        masterFilterPlotPath.lineTo (getPointForXCoord (x));
}

void EqualizerPlot::resized()
{
    for (int i = 0; i < (int) filterPlots.size(); ++i)
        updateFilterPlotPath (i);
}

const juce::Path& EqualizerPlot::getPath (int bandIndex) const
{
    jassert (juce::isPositiveAndBelow (bandIndex, (int) filterPlots.size()));
    return filterPlots[(size_t) bandIndex].plotPath;
}
} // namespace chowdsp::EQ
