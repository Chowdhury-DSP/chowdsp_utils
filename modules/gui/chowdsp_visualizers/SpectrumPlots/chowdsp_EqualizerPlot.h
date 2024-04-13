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
template <size_t numBands>
class EqualizerPlot : public SpectrumPlotBase
{
public:
    explicit EqualizerPlot (SpectrumPlotParams&& plotParams);

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
        LocalPointer<EQFilterPlot, 512> plot {};
        std::optional<EQPlotFilterType> type {};
        juce::Path plotPath {};
        std::vector<float> plotData {};
    };

    std::array<BandPlotInfo, numBands> filterPlots {};
    juce::Path masterFilterPlotPath {};

    std::array<bool, numBands> filtersActiveFlags {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerPlot)
};

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state && JUCE_MODULE_AVAILABLE_chowdsp_eq
template <size_t numBands>
class EqualizerPlotWithParameters : public EqualizerPlot<numBands>
{
public:
    EqualizerPlotWithParameters (ParameterListeners& listeners,
                                 EQ::StandardEQParameters<numBands>& eqParameters,
                                 const std::function<EQPlotFilterType (int)>& filterTypeMap,
                                 SpectrumPlotParams&& plotParams = {});

private:
    void updateFreqParameter (int bandIndex);
    void updateQParameter (int bandIndex);
    void updateGainDBParameter (int bandIndex);
    void updateFilterType (int bandIndex, const std::function<EQPlotFilterType (int)>& filterTypeMap); // NOSONAR
    void updateFilterOnOff (int bandIndex);

    EQ::StandardEQParameters<numBands>& eqParams;

    ScopedCallbackList callbacks {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqualizerPlotWithParameters)
};
#endif
} // namespace chowdsp::EQ

#include "chowdsp_EqualizerPlot.cpp"
