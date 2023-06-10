#pragma once

#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

/** Namespace containing version hints for plugin parameters */
namespace VersionHints
{
using namespace chowdsp::version_literals;

/** Version Hint 1.0.0 */
static constexpr auto v1_0_0 = "1.0.0"_v.getVersionHint();
} // namespace VersionHints

enum class LevelDetectorMode
{
    Peak = 1,
    RMS = 2,
    PeakRtT = 4,
    SlowPeak = 8,
};

enum class Architecture
{
    Feed_Forward = 1,
    Feed_Back = 2,
};

struct Params : chowdsp::ParamHolder
{
    Params()
    {
        add (threshold,
             ratio,
             attack,
             release,
             knee,
             autoMakeup,
             levelDetectorMode,
             architecture);
    }

    chowdsp::GainDBParameter::Ptr threshold {
        juce::ParameterID { "threshold", VersionHints::v1_0_0 },
        "Threshold",
        juce::NormalisableRange { -30.0f, 6.0f },
        0.0f
    };

    chowdsp::RatioParameter::Ptr ratio {
        juce::ParameterID { "ratio", VersionHints::v1_0_0 },
        "Ratio",
        chowdsp::ParamUtils::createNormalisableRange (1.0f, 20.0f, 2.5f),
        2.0f
    };

    chowdsp::TimeMsParameter::Ptr attack {
        juce::ParameterID { "attack", VersionHints::v1_0_0 },
        "Attack",
        chowdsp::ParamUtils::createNormalisableRange (0.5f, 100.0f, 10.0f),
        10.0f
    };

    chowdsp::TimeMsParameter::Ptr release {
        juce::ParameterID { "release", VersionHints::v1_0_0 },
        "Release",
        chowdsp::ParamUtils::createNormalisableRange (10.0f, 1000.0f, 100.0f),
        100.0f
    };

    chowdsp::GainDBParameter::Ptr knee {
        juce::ParameterID { "knee", VersionHints::v1_0_0 },
        "Knee",
        juce::NormalisableRange { 0.0f, 24.0f },
        6.0f
    };

    chowdsp::BoolParameter::Ptr autoMakeup {
        juce::ParameterID { "auto_makeup", VersionHints::v1_0_0 },
        "Auto-Makeup",
        false
    };

    chowdsp::EnumChoiceParameter<LevelDetectorMode>::Ptr levelDetectorMode {
        juce::ParameterID { "ld_mode", VersionHints::v1_0_0 },
        "Level Detector Mode",
        LevelDetectorMode::Peak
    };

    chowdsp::EnumChoiceParameter<Architecture>::Ptr architecture {
        juce::ParameterID { "architecture", VersionHints::v1_0_0 },
        "Architecture",
        Architecture::Feed_Forward,
        std::initializer_list<std::pair<char, char>> { { '_', '-' } }
    };
};
