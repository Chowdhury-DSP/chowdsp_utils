#include "chowdsp_EqualizerPlot.h"

namespace chowdsp::EQ
{
template <size_t numBands>
EqualizerPlot<numBands>::EqualizerPlot (SpectrumPlotParams&& plotParams)
    : SpectrumPlotBase (std::move (plotParams))
{
    std::fill (filtersActiveFlags.begin(), filtersActiveFlags.end(), true);
    for (auto& plot : filterPlots)
        plot.plot.template emplace<EQFilterPlot>();
}

template <size_t numBands>
void EqualizerPlot<numBands>::setFilterType (int bandIndex, EQPlotFilterType type)
{
    auto& plot = filterPlots[(size_t) bandIndex].plot;
    auto& plotType = filterPlots[(size_t) bandIndex].type;
    if (plotType.has_value() && *plotType == type)
        return;

    plotType = type;
    switch (type)
    {
        case EQPlotFilterType::LPF1:
            plot.template emplace<LPF1Plot>();
            break;
        case EQPlotFilterType::LPF2:
            plot.template emplace<LPF2Plot>();
            break;
        case EQPlotFilterType::LPF3:
            plot.template emplace<HigherOrderLPFPlot<3>>();
            break;
        case EQPlotFilterType::LPF4:
            plot.template emplace<HigherOrderLPFPlot<4>>();
            break;
        case EQPlotFilterType::LPF5:
            plot.template emplace<HigherOrderLPFPlot<5>>();
            break;
        case EQPlotFilterType::LPF6:
            plot.template emplace<HigherOrderLPFPlot<6>>();
            break;
        case EQPlotFilterType::LPF7:
            plot.template emplace<HigherOrderLPFPlot<7>>();
            break;
        case EQPlotFilterType::LPF8:
            plot.template emplace<HigherOrderLPFPlot<8>>();
            break;
        case EQPlotFilterType::LPF9:
            plot.template emplace<HigherOrderLPFPlot<9>>();
            break;
        case EQPlotFilterType::LPF10:
            plot.template emplace<HigherOrderLPFPlot<10>>();
            break;
        case EQPlotFilterType::LPF11:
            plot.template emplace<HigherOrderLPFPlot<11>>();
            break;
        case EQPlotFilterType::LPF12:
            plot.template emplace<HigherOrderLPFPlot<12>>();
            break;

        case EQPlotFilterType::HPF1:
            plot.template emplace<HPF1Plot>();
            break;
        case EQPlotFilterType::HPF2:
            plot.template emplace<HPF2Plot>();
            break;
        case EQPlotFilterType::HPF3:
            plot.template emplace<HigherOrderHPFPlot<3>>();
            break;
        case EQPlotFilterType::HPF4:
            plot.template emplace<HigherOrderHPFPlot<4>>();
            break;
        case EQPlotFilterType::HPF5:
            plot.template emplace<HigherOrderHPFPlot<5>>();
            break;
        case EQPlotFilterType::HPF6:
            plot.template emplace<HigherOrderHPFPlot<6>>();
            break;
        case EQPlotFilterType::HPF7:
            plot.template emplace<HigherOrderHPFPlot<7>>();
            break;
        case EQPlotFilterType::HPF8:
            plot.template emplace<HigherOrderHPFPlot<8>>();
            break;
        case EQPlotFilterType::HPF9:
            plot.template emplace<HigherOrderHPFPlot<9>>();
            break;
        case EQPlotFilterType::HPF10:
            plot.template emplace<HigherOrderHPFPlot<10>>();
            break;
        case EQPlotFilterType::HPF11:
            plot.template emplace<HigherOrderHPFPlot<11>>();
            break;
        case EQPlotFilterType::HPF12:
            plot.template emplace<HigherOrderHPFPlot<12>>();
            break;

        case EQPlotFilterType::BPF2:
            plot.template emplace<BPF2Plot>();
            break;
        case EQPlotFilterType::Bell:
            plot.template emplace<BellPlot>();
            break;
        case EQPlotFilterType::Notch:
            plot.template emplace<NotchPlot>();
            break;
        case EQPlotFilterType::LowShelf:
            plot.template emplace<LowShelfPlot>();
            break;
        case EQPlotFilterType::HighShelf:
            plot.template emplace<HighShelfPlot>();
            break;

        default:
            plot.template emplace<EQFilterPlot>();
            break;
    }
}

template <size_t numBands>
void EqualizerPlot<numBands>::setFilterActive (int bandIndex, bool isActive)
{
    filtersActiveFlags[(size_t) bandIndex] = isActive;
}

template <size_t numBands>
bool EqualizerPlot<numBands>::getFilterActive (int bandIndex) const
{
    return filtersActiveFlags[(size_t) bandIndex];
}

template <size_t numBands>
void EqualizerPlot<numBands>::setCutoffParameter (int bandIndex, float cutoffHz)
{
    filterPlots[(size_t) bandIndex].plot->setCutoffFrequency (cutoffHz);
}

template <size_t numBands>
void EqualizerPlot<numBands>::setQParameter (int bandIndex, float qValue)
{
    filterPlots[(size_t) bandIndex].plot->setQValue (qValue);
}

template <size_t numBands>
void EqualizerPlot<numBands>::setGainDBParameter (int bandIndex, float gainDB)
{
    filterPlots[(size_t) bandIndex].plot->setGainDecibels (gainDB);
}

template <size_t numBands>
void EqualizerPlot<numBands>::updateFilterPlotPath (int bandIndex)
{
    const auto width = getWidth();
    if (width == 0 || getHeight() == 0)
        return;

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

    path.startNewSubPath (getPointForXCoord (0));
    for (int x = 1; x < width; ++x)
        path.lineTo (getPointForXCoord (x));

    updateMasterFilterPlotPath();

    repaint();
}

template <size_t numBands>
void EqualizerPlot<numBands>::updateMasterFilterPlotPath()
{
    const auto width = getWidth();
    if (width == 0 || getHeight() == 0)
        return;

    masterFilterPlotPath.clear();
    masterFilterPlotPath.preallocateSpace (width * 3);

    std::vector<std::vector<float>*> activePlots;
    for (auto [index, filterPlot] : enumerate (filterPlots))
    {
        if (filtersActiveFlags[index])
            activePlots.push_back (&filterPlot.plotData);
    }

    const auto getPointForXCoord = [this, &currentActivePlots = std::as_const (activePlots)] (int x) -> juce::Point<float>
    {
        float magLinear = 1.0f;
        for (auto* plotData : currentActivePlots)
        {
            if (plotData->size() <= (size_t) x)
                break;

            magLinear *= plotData->operator[] ((size_t) x);
        }

        const auto magDB = juce::Decibels::gainToDecibels (magLinear);
        return { (float) x, getYCoordinateForDecibels (magDB) };
    };

    masterFilterPlotPath.startNewSubPath (getPointForXCoord (0));
    for (int x = 1; x < width; ++x)
        masterFilterPlotPath.lineTo (getPointForXCoord (x));
}

template <size_t numBands>
void EqualizerPlot<numBands>::resized()
{
    for (int i = 0; i < (int) filterPlots.size(); ++i)
        updateFilterPlotPath (i);
}

template <size_t numBands>
const juce::Path& EqualizerPlot<numBands>::getPath (int bandIndex) const
{
    jassert (juce::isPositiveAndBelow (bandIndex, (int) filterPlots.size()));
    return filterPlots[(size_t) bandIndex].plotPath;
}

//=================================================================================================
#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state && JUCE_MODULE_AVAILABLE_chowdsp_eq
template <size_t numBands>
EqualizerPlotWithParameters<numBands>::EqualizerPlotWithParameters (ParameterListeners& listeners,
                                                                    EQ::StandardEQParameters<numBands>& eqParameters,
                                                                    const std::function<EQPlotFilterType (int)>& filterTypeMap,
                                                                    SpectrumPlotParams&& plotParams)
    : EqualizerPlot<numBands> (std::move (plotParams)),
      eqParams (eqParameters)
{
    for (const auto [index, bandParams] : enumerate (eqParams.eqParams))
    {
        const auto bandIndex = (int) index;
        callbacks +=
            {
                listeners.addParameterListener (
                    bandParams.freqParam,
                    ParameterListenerThread::MessageThread,
                    [this, bandIndex]
                    {
                        updateFreqParameter (bandIndex);
                        EqualizerPlot<numBands>::updateFilterPlotPath (bandIndex);
                    }),
                listeners.addParameterListener (
                    bandParams.qParam,
                    ParameterListenerThread::MessageThread,
                    [this, bandIndex]
                    {
                        updateQParameter (bandIndex);
                        EqualizerPlot<numBands>::updateFilterPlotPath (bandIndex);
                    }),
                listeners.addParameterListener (
                    bandParams.gainParam,
                    ParameterListenerThread::MessageThread,
                    [this, bandIndex]
                    {
                        updateGainDBParameter (bandIndex);
                        EqualizerPlot<numBands>::updateFilterPlotPath (bandIndex);
                    }),
                listeners.addParameterListener (
                    bandParams.typeParam,
                    ParameterListenerThread::MessageThread,
                    [this, bandIndex, filterTypeMap]
                    {
                        updateFilterType (bandIndex, filterTypeMap);
                        EqualizerPlot<numBands>::updateFilterPlotPath (bandIndex);
                    }),
                listeners.addParameterListener (
                    bandParams.onOffParam,
                    ParameterListenerThread::MessageThread,
                    [this, bandIndex]
                    {
                        updateFilterOnOff (bandIndex);
                        EqualizerPlot<numBands>::updateFilterPlotPath (bandIndex);
                    })
            };

        updateFilterType (bandIndex, filterTypeMap);
        updateFilterOnOff (bandIndex);
    }
}

template <size_t numBands>
void EqualizerPlotWithParameters<numBands>::updateFreqParameter (int bandIndex)
{
    EqualizerPlot<numBands>::setCutoffParameter (bandIndex, eqParams.eqParams[(size_t) bandIndex].freqParam->get());
}

template <size_t numBands>
void EqualizerPlotWithParameters<numBands>::updateQParameter (int bandIndex)
{
    EqualizerPlot<numBands>::setQParameter (bandIndex, eqParams.eqParams[(size_t) bandIndex].qParam->get());
}

template <size_t numBands>
void EqualizerPlotWithParameters<numBands>::updateGainDBParameter (int bandIndex)
{
    EqualizerPlot<numBands>::setGainDBParameter (bandIndex, eqParams.eqParams[(size_t) bandIndex].gainParam->get());
}

template <size_t numBands>
void EqualizerPlotWithParameters<numBands>::updateFilterType (int bandIndex, const std::function<EQPlotFilterType (int)>& filterTypeMap) // NOSONAR
{
    const auto& bandTypeParam = eqParams.eqParams[(size_t) bandIndex].typeParam;
    EqualizerPlot<numBands>::setFilterType (bandIndex, filterTypeMap (bandTypeParam->getIndex()));
    updateFreqParameter (bandIndex);
    updateQParameter (bandIndex);
    updateGainDBParameter (bandIndex);
}

template <size_t numBands>
void EqualizerPlotWithParameters<numBands>::updateFilterOnOff (int bandIndex)
{
    EqualizerPlot<numBands>::setFilterActive (bandIndex, eqParams.eqParams[(size_t) bandIndex].onOffParam->get());
}
#endif
} // namespace chowdsp::EQ
