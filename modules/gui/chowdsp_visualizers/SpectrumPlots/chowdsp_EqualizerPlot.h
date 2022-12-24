#pragma once

namespace chowdsp::EQ
{
enum class EQPlotFilterType
{
    LPF1,
    LPF2,
    LPF3,
    LPF4,
    LPF5,
    LPF6,
    LPF7,
    LPF8,
    LPF9,
    LPF10,
    LPF11,
    LPF12,

    HPF1,
    HPF2,
    HPF3,
    HPF4,
    HPF5,
    HPF6,
    HPF7,
    HPF8,
    HPF9,
    HPF10,
    HPF11,
    HPF12,

    BPF2,
    Bell,
    Notch,
    LowShelf,
    HighShelf,
};

class EqualizerPlot : public SpectrumPlotBase
{
public:
    explicit EqualizerPlot (int numBands, SpectrumPlotParams&& params = {});

    void setFilterType (int bandIndex, EQPlotFilterType type);

    void setCutoffParameter (int bandIndex, float cutoffHz);

    void setQParameter (int bandIndex, float qValue);

    void setGainDBParameter (int bandIndex, float gainDB);

    void updateFilterPlotPath (int bandIndex);

    [[nodiscard]] const juce::Path& getPath (int bandIndex) const;
    [[nodiscard]] const juce::Path& getMasterFilterPath() const { return masterFilterPlotPath; }

    void resized() override;

private:
    void updateMasterFilterPlotPath();

    struct BandPlotInfo
    {
        std::unique_ptr<EQFilterPlot> plot;
        juce::Path plotPath;
        std::vector<float> plotData {};
    };

    std::vector<BandPlotInfo> filterPlots;
    juce::Path masterFilterPlotPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerPlot)
};

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state && JUCE_MODULE_AVAILABLE_chowdsp_eq
template <int numBands>
class EqualizerPlotWithParameters : public EqualizerPlot
{
public:
    EqualizerPlotWithParameters (chowdsp::ParameterListeners& listeners,
                                 chowdsp::EQ::StandardEQParameters<numBands>& eqParameters,
                                 std::function<EQPlotFilterType (int)>&& filterTypeMap,
                                 SpectrumPlotParams&& plotParams = {})
        : EqualizerPlot (numBands, std::move (plotParams)),
          eqParams (eqParameters)
    {
        for (const auto [index, bandParams] : chowdsp::enumerate (eqParams.eqParams))
        {
            const auto bandIndex = (int) index;
            callbacks +=
                {
                    listeners.addParameterListener (
                        bandParams.freqParam,
                        chowdsp::ParameterListenerThread::MessageThread,
                        [this, bandIndex]
                        {
                            updateFreqParameter (bandIndex);
                            updateFilterPlotPath (bandIndex);
                        }),
                    listeners.addParameterListener (
                        bandParams.qParam,
                        chowdsp::ParameterListenerThread::MessageThread,
                        [this, bandIndex]
                        {
                            updateQParameter (bandIndex);
                            updateFilterPlotPath (bandIndex);
                        }),
                    listeners.addParameterListener (
                        bandParams.gainParam,
                        chowdsp::ParameterListenerThread::MessageThread,
                        [this, bandIndex]
                        {
                            updateGainDBParameter (bandIndex);
                            updateFilterPlotPath (bandIndex);
                        }),
                    listeners.template addParameterListener (
                        bandParams.typeParam,
                        chowdsp::ParameterListenerThread::MessageThread,
                        [this, bandIndex, filterTypeMap = std::move (filterTypeMap)]
                        {
                            updateFilterType (bandIndex, filterTypeMap);
                            updateFilterPlotPath (bandIndex);
                        })
                };

            updateFilterType (bandIndex, filterTypeMap);
        }
    }

private:
    void updateFreqParameter (int bandIndex)
    {
        setCutoffParameter (bandIndex, eqParams.eqParams[(size_t) bandIndex].freqParam->get());
    }

    void updateQParameter (int bandIndex)
    {
        setQParameter (bandIndex, eqParams.eqParams[(size_t) bandIndex].qParam->get());
    }

    void updateGainDBParameter (int bandIndex)
    {
        setGainDBParameter (bandIndex, eqParams.eqParams[(size_t) bandIndex].gainParam->get());
    }

    void updateFilterType (int bandIndex, const std::function<EQPlotFilterType (int)>& filterTypeMap)
    {
        setFilterType (bandIndex, filterTypeMap (eqParams.eqParams[(size_t) bandIndex].typeParam->getIndex()));
        updateFreqParameter (bandIndex);
        updateQParameter (bandIndex);
        updateGainDBParameter (bandIndex);
    }

    chowdsp::EQ::StandardEQParameters<numBands>& eqParams;

    chowdsp::ScopedCallbackList callbacks;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerPlotWithParameters)
};
#endif
} // namespace chowdsp::EQ
