#pragma once

namespace chowdsp::compressor
{
/** Variadic gain computer intended to be used in a compressor */
template <typename SampleType,
          typename GainComputerTypes = types_list::TypesList<FeedForwardCompGainComputer<SampleType>,
                                                             FeedBackCompGainComputer<SampleType>>>
class GainComputer
{
    static constexpr bool isMultiModal = types_list::IsTypesList<GainComputerTypes>;

public:
    GainComputer() = default;

    /** Prepares the gain computer to process and audio stream. */
    void prepare (double sampleRate, int samplesPerBlock, bool useArenaAllocator = false)
    {
        threshSmooth.setRampLength (0.05);
        threshSmooth.prepare (sampleRate, samplesPerBlock, ! useArenaAllocator);

        ratioSmooth.setRampLength (0.05);
        ratioSmooth.prepare (sampleRate, samplesPerBlock, ! useArenaAllocator);
    }

    /** Reset's the processor state. */
    void reset()
    {
        threshSmooth.reset (juce::Decibels::decibelsToGain (threshDB));
        ratioSmooth.reset (ratio);
    }

    /**
     * Sets the index of the gain computer type to use,
     * based on the index in the template types_list.
     */
    template <bool hasModes = isMultiModal>
    std::enable_if_t<hasModes, void> setMode (size_t newModeIndex)
    {
        if (newModeIndex != modeIndex)
        {
            jassert (juce::isPositiveAndBelow (newModeIndex, GainComputerTypes::count));
            modeIndex = juce::jlimit ((size_t) 0, GainComputerTypes::count - 1, newModeIndex);
            recalcConstants();
        }
    }

    /** Sets the gain computer threshold in Decibels */
    void setThreshold (SampleType newThreshDB)
    {
        if (! juce::approximatelyEqual (threshDB, newThreshDB))
        {
            threshDB = newThreshDB;
            recalcKnees();
        }
    }

    /** Sets the gain computer ratio */
    void setRatio (SampleType newRatio)
    {
        if (! juce::approximatelyEqual (ratio, newRatio))
        {
            ratio = newRatio;
            recalcConstants();
        }
    }

    /** Sets the gain computer knee in Decibels */
    void setKnee (SampleType newKneeDB)
    {
        if (! juce::approximatelyEqual (kneeDB, newKneeDB))
        {
            kneeDB = newKneeDB;
            recalcKnees();
            recalcConstants();
        }
    }

    /**
     * Processes a stream of audio data.
     * The levelBuffer should contain a level signal that is always greater than zero.
     * Calling this method will fill the gainBuffer with a set of "gain" values,
     * which can be multiplied by the audio signal to apply compression.
     */
    void processBlock (const BufferView<const SampleType>& levelBuffer,
                       const BufferView<SampleType>& gainBuffer,
                       ArenaAllocator<>* arena = nullptr) noexcept
    {
        jassert (levelBuffer.getNumSamples() == gainBuffer.getNumSamples());

        const auto numSamples = gainBuffer.getNumSamples();
        if (arena == nullptr)
        {
            threshSmooth.process (juce::Decibels::decibelsToGain (threshDB), numSamples);
            ratioSmooth.process (ratio, numSamples);
        }
        else
        {
            threshSmooth.process (juce::Decibels::decibelsToGain (threshDB), numSamples, *arena);
            ratioSmooth.process (ratio, numSamples, *arena);
        }

        if constexpr (isMultiModal)
        {
            TupleHelpers::visit_at (computers,
                                    modeIndex,
                                    [this, &levelBuffer, &gainBuffer] (auto& computer)
                                    {
                                        computer.process (levelBuffer,
                                                          gainBuffer,
                                                          {
                                                              threshSmooth,
                                                              ratioSmooth,
                                                              kneeDB,
                                                              kneeLower,
                                                              kneeUpper,
                                                          });
                                    });
        }
        else
        {
            computers.process (levelBuffer,
                               gainBuffer,
                               {
                                   threshSmooth,
                                   ratioSmooth,
                                   kneeDB,
                                   kneeLower,
                                   kneeUpper,
                               });
        }
    }

    /** Applies the gain computer's auto-makeup gain to the buffer */
    void applyAutoMakeup (const BufferView<SampleType>& buffer, SampleType autoMakeupTargetDB = (SampleType) 0)
    {
        const auto targetGainLinear = juce::Decibels::decibelsToGain (autoMakeupTargetDB);
        if constexpr (isMultiModal)
        {
            TupleHelpers::visit_at (computers,
                                    modeIndex,
                                    [this, &buffer, targetGainLinear] (auto& computer)
                                    {
                                        computer.applyAutoMakeup (buffer,
                                                                  {
                                                                      threshSmooth,
                                                                      ratioSmooth,
                                                                      kneeDB,
                                                                      kneeLower,
                                                                      kneeUpper,
                                                                      targetGainLinear,
                                                                  });
                                    });
        }
        else
        {
            computers.applyAutoMakeup (buffer,
                                       {
                                           threshSmooth,
                                           ratioSmooth,
                                           kneeDB,
                                           kneeLower,
                                           kneeUpper,
                                           targetGainLinear,
                                       });
        }
    }

private:
    void recalcKnees() // recalculate knee values for a new threshold or knee width
    {
        kneeLower = juce::Decibels::decibelsToGain (threshDB - kneeDB * 0.5f);
        kneeUpper = juce::Decibels::decibelsToGain (threshDB + kneeDB * 0.5f);
    }

    void recalcConstants()
    {
        if constexpr (isMultiModal)
        {
            TupleHelpers::visit_at (computers,
                                    modeIndex,
                                    [this] (auto& computer)
                                    {
                                        if constexpr (HasRecalcConstants<std::decay_t<decltype (computer)>>)
                                            computer.recalcConstants (ratio, kneeDB);
                                    });
        }
        else
        {
            if constexpr (HasRecalcConstants<GainComputerTypes>)
                computers.recalcConstants (ratio, kneeDB);
        }
    }

    SampleType threshDB = (SampleType) 0;
    SampleType ratio = (SampleType) 1;
    SmoothedBufferValue<SampleType, juce::ValueSmoothingTypes::Multiplicative> threshSmooth;
    SmoothedBufferValue<SampleType, juce::ValueSmoothingTypes::Multiplicative> ratioSmooth;
    SampleType kneeDB = (SampleType) 1;

    SampleType kneeUpper = (SampleType) 1;
    SampleType kneeLower = (SampleType) 1;

    std::conditional_t<isMultiModal, size_t, NullType> modeIndex { 0 };
    types_list::TypesWrapper<GainComputerTypes> computers;

    CHOWDSP_CHECK_HAS_METHOD (HasRecalcConstants, recalcConstants, SampleType {}, SampleType {})

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainComputer)
};
} // namespace chowdsp::compressor
