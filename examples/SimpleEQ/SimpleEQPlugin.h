#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

#include "PrototypeEQ.h"

struct Params
{
    using BandParams = PrototypeEQ::EQParams::EQBandParams;

    static constexpr std::string_view bandTypeChoices =
        "1-Pole HPF,"
        "2-Pole HPF,"
        "2-Pole HPF SVF,"
        "3-Pole HPF,"
        "4-Pole HPF,"
        "8-Pole HPF,"
        "8-Pole Cheby. HPF,"
        "12-Pole Elliptic HPF,"
        "Low-Shelf,"
        "Low-Shelf SVF,"
        "Bell,"
        "Bell SVF,"
        "Notch,"
        "Notch SVF,"
        "High-Shelf,"
        "High-Shelf SVF,"
        "2-Pole BPF,"
        "2-Pole BPF SVF,"
        "1-Pole LPF,"
        "2-Pole LPF,"
        "2-Pole LPF SVF,"
        "3-Pole LPF,"
        "4-Pole LPF,"
        "8-Pole LPF,"
        "8-Pole Cheby. LPF,"
        "12-Pole Elliptic LPF";

    PrototypeEQ::EQParams eqParams {
        "chowdsp_eq",
        PrototypeEQ::EQParams::EQParameterHandles {
            BandParams { 0, "eq_band_0", "Band ", 100, 250.0f, bandTypeChoices, 10 },
            BandParams { 1, "eq_band_1", "Band ", 100, 1000.0f, bandTypeChoices, 10 },
            BandParams { 2, "eq_band_2", "Band ", 100, 4000.0f, bandTypeChoices, 10 },
        }
    };

    //    PrototypeEQ::EQParams::addEQParameters (params, PrototypeEQ::EQParams::defaultEQParamPrefix, eqTypeChoices, 7);
    chowdsp::BoolParameter::Ptr linPhaseMode {
        chowdsp::ParameterID { "linear_phase_mode", 100 },
        "Linear Phase",
        false
    };
};

using State = chowdsp::PluginState<Params>;

/** Example plugin to demonstrate the use of chowdsp::EQProcessor and chowdsp::LinearPhaseEQ */
class SimpleEQPlugin : public chowdsp::PluginBase<State>
{
public:
    SimpleEQPlugin();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    juce::AudioProcessorEditor* createEditor() override;

    PrototypeEQ::Params getEQParams() const;
    void loadEQParams (const PrototypeEQ::Params& params);

private:
    void setEQParams();

    PrototypeEQ::EQParams::EQParameterHandles& eqParamsHandles;

    PrototypeEQ protoEQ; // the regular EQ
    chowdsp::EQ::LinearPhaseEQ<PrototypeEQ> linPhaseEQ; // the linear phase EQ

    chowdsp::ScopedCallback linPhaseModeChangeCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQPlugin)
};
