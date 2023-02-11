#pragma once

namespace chowdsp::EQ
{
/** EQ filter types that can be plotted */
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

/** UI component for plotting EQ filter frequency responses. */
class EqualizerPlot : public SpectrumPlotBase
{
public:
    explicit EqualizerPlot (int numBands, SpectrumPlotParams&& params = {});

    /** Sets the filter type for a given band. */
    void setFilterType (int bandIndex, EQPlotFilterType type);

    /** Sets the given filter band to be active or inactive. */
    void setFilterActive (int bandIndex, bool isActive);

    /** Returns true if the given filter band is active. */
    [[nodiscard]] bool getFilterActive (int bandIndex) const;

    /** Sets the cutoff frequency for a given band. */
    void setCutoffParameter (int bandIndex, float cutoffHz);

    /** Sets the Q value for a given band. */
    void setQParameter (int bandIndex, float qValue);

    /** Sets the gain parameter for a given band. */
    void setGainDBParameter (int bandIndex, float gainDB);

    /** Forcibly updates the path for a given filter plot. */
    void updateFilterPlotPath (int bandIndex);

    /** Returns the path for a given filter plot. */
    [[nodiscard]] const juce::Path& getPath (int bandIndex) const;

    /** Returns the path for a "master" filter plot, comprised of the combination of all the filter responses. */
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

    std::vector<bool> filtersActiveFlags;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerPlot)
};

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state && JUCE_MODULE_AVAILABLE_chowdsp_eq
template <size_t numBands>
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
                    listeners.addParameterListener (
                        bandParams.typeParam,
                        chowdsp::ParameterListenerThread::MessageThread,
                        [this, bandIndex, filterTypeMap]
                        {
                            updateFilterType (bandIndex, filterTypeMap);
                            updateFilterPlotPath (bandIndex);
                        }),
                    listeners.addParameterListener (
                        bandParams.onOffParam,
                        chowdsp::ParameterListenerThread::MessageThread,
                        [this, bandIndex]
                        {
                            updateFilterOnOff (bandIndex);
                            updateFilterPlotPath (bandIndex);
                        })
                };

            updateFilterType (bandIndex, filterTypeMap);
            updateFilterOnOff (bandIndex);
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

    void updateFilterType (int bandIndex, const std::function<EQPlotFilterType (int)>& filterTypeMap) // NOSONAR
    {
        const auto& bandTypeParam = eqParams.eqParams[(size_t) bandIndex].typeParam;
        setFilterType (bandIndex, filterTypeMap (bandTypeParam->getIndex()));
        updateFreqParameter (bandIndex);
        updateQParameter (bandIndex);
        updateGainDBParameter (bandIndex);
    }

    void updateFilterOnOff (int bandIndex)
    {
        setFilterActive (bandIndex, eqParams.eqParams[(size_t) bandIndex].onOffParam->get());
    }

    chowdsp::EQ::StandardEQParameters<numBands>& eqParams;

    chowdsp::ScopedCallbackList callbacks;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerPlotWithParameters)
};
#endif
} // namespace chowdsp::EQ
