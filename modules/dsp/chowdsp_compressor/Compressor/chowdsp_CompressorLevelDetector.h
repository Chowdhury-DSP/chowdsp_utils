#pragma once

namespace chowdsp::compressor
{
/** Variadic level detector to be used in a compressor. */
template <typename SampleType,
          typename LevelDetectorTypes = types_list::TypesList<PeakDetector, PeakRtTDetector, RMSDetector>,
          typename LevelDetectorVisualizer = NullType>
class CompressorLevelDetector
{
    static constexpr bool isMultiModal = types_list::IsTypesList<LevelDetectorTypes>;

public:
    CompressorLevelDetector() = default;

    /**
     * Sets the index of the level detector type to use,
     * based on the index in the template types_list.
     *
     * This method should be called _before_ setting the other parameters.
     */
    template <bool hasModes = isMultiModal>
    std::enable_if_t<hasModes, void> setMode (size_t newModeIndex)
    {
        jassert (juce::isPositiveAndBelow (newModeIndex, LevelDetectorTypes::count));
        modeIndex = juce::jlimit ((size_t) 0, LevelDetectorTypes::count - 1, newModeIndex);
    }

    /** Sets the attack time in milliseconds */
    void setAttackMs (SampleType newAttackMs)
    {
        if constexpr (isMultiModal)
        {
            TupleHelpers::visit_at (detectors,
                                    modeIndex,
                                    [&newAttackMs] (auto& detector)
                                    {
                                        if constexpr (HasModifyAttack<std::decay_t<decltype (detector)>>)
                                            newAttackMs = detector.modifyAttack (newAttackMs);
                                    });
        }
        else
        {
            if constexpr (HasModifyAttack<std::decay_t<decltype (detectors)>>)
                newAttackMs = detectors.modifyAttack (newAttackMs);
        }

        const auto coeffs = computeBallisticCoeffs (newAttackMs, fs);
        a1_a = coeffs.a1;
        b0_a = coeffs.b0;
    }

    /** Sets the release time in milliseconds */
    void setReleaseMs (float newReleaseMs)
    {
        if constexpr (isMultiModal)
        {
            TupleHelpers::visit_at (detectors,
                                    modeIndex,
                                    [&newReleaseMs] (auto& detector)
                                    {
                                        if constexpr (HasModifyRelease<std::decay_t<decltype (detector)>>)
                                            newReleaseMs = detector.modifyRelease (newReleaseMs);
                                    });
        }
        else
        {
            if constexpr (HasModifyRelease<std::decay_t<decltype (detectors)>>)
                newReleaseMs = detectors.modifyRelease (newReleaseMs);
        }

        const auto coeffs = computeBallisticCoeffs (newReleaseMs, fs);
        a1_r = coeffs.a1;
        b0_r = coeffs.b0;
    }

    /** Sets the threshold level in Decibels */
    void setThresholdDB (float newThresholdDB)
    {
        thresholdLinearGain = juce::Decibels::decibelsToGain (newThresholdDB);
    }

    /** Prepares the level detector to process an audio stream */
    void prepare (double sampleRate, int samplesPerBlock)
    {
        if constexpr (! std::is_same_v<LevelDetectorVisualizer, NullType>)
        {
            levelDetectorViz.setBufferSize (int (levelDetectorViz.secondsToVisualize * sampleRate / (double) samplesPerBlock));
            levelDetectorViz.setSamplesPerBlock (samplesPerBlock);
        }

        fs = (SampleType) sampleRate;
        juce::ignoreUnused (samplesPerBlock);
        reset();
    }

    /** Reset's the level detector state */
    void reset()
    {
        z1 = (SampleType) 0;
    }

    /**
     * Processes a mono buffer. If a multi-channel buffer is passed in,
     * only the first channel will be used.
     */
    void processBlock (const BufferView<float>& buffer) noexcept
    {
        if constexpr (! std::is_same_v<LevelDetectorVisualizer, NullType>)
            levelDetectorViz.pushChannel (0, buffer.getReadSpan (0));

        if constexpr (isMultiModal)
        {
            TupleHelpers::visit_at (detectors,
                                    modeIndex,
                                    [this, &buffer] (auto& detector)
                                    {
                                        detector.process (buffer,
                                                          { a1_a, b0_a },
                                                          { a1_r, b0_r },
                                                          &z1,
                                                          thresholdLinearGain);
                                    });
        }
        else
        {
            detectors.process (buffer,
                               { a1_a, b0_a },
                               { a1_r, b0_r },
                               &z1,
                               thresholdLinearGain);
        }

        if constexpr (! std::is_same_v<LevelDetectorVisualizer, NullType>)
            levelDetectorViz.pushChannel (1, buffer.getReadSpan (0));
    }

    LevelDetectorVisualizer levelDetectorViz;

    CHOWDSP_CHECK_HAS_METHOD (HasModifyAttack, modifyAttack, SampleType {})
    CHOWDSP_CHECK_HAS_METHOD (HasModifyRelease, modifyRelease, SampleType {})

private:
    // Attack coeffs
    SampleType a1_a = (SampleType) 0;
    SampleType b0_a = (SampleType) 1;

    // Release coeffs
    SampleType a1_r = (SampleType) 0;
    SampleType b0_r = (SampleType) 1;

    size_t modeIndex = 0;
    SampleType thresholdLinearGain = (SampleType) 1;
    SampleType fs = (SampleType) 48000;

    SampleType z1 = (SampleType) 0;

    types_list::TypesWrapper<LevelDetectorTypes> detectors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorLevelDetector)
};
} // namespace chowdsp::compressor
