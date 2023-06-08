#pragma once

namespace chowdsp::compressor
{
/** Variadic gain computer intended to be used in a compressor */
template <typename SampleType,
          typename GainComputerTypes = types_list::TypesList<FeedForwardCompGainComputer<SampleType>,
                                                             FeedBackCompGainComputer<SampleType>>>
class GainComputer
{
public:
    GainComputer() = default;

    /** Prepares the gain computer to process and audio stream. */
    void prepare (double sampleRate, int samplesPerBlock)
    {
        threshSmooth.setRampLength (0.05);
        threshSmooth.prepare (sampleRate, samplesPerBlock);

        ratioSmooth.setRampLength (0.05);
        ratioSmooth.prepare (sampleRate, samplesPerBlock);
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
    void setMode (size_t newModeIndex)
    {
        if (newModeIndex != modeIndex)
        {
            jassert (juce::isPositiveAndBelow (newModeIndex, GainComputerTypes::count));
            modeIndex = juce::jlimit (0, GainComputerTypes::count - 1, newModeIndex);
            recalcConstants();
        }
    }

    /** Sets the gain computer threshold in Decibels */
    void setThreshold (SampleType newThreshDB)
    {
        if (threshDB != newThreshDB)
        {
            threshDB = newThreshDB;
            recalcKnees();
        }
    }

    /** Sets the gain computer ratio */
    void setRatio (SampleType newRatio)
    {
        if (ratio != newRatio)
        {
            ratio = newRatio;
            recalcConstants();
        }
    }

    /** Sets the gain computer knee in Decibels */
    void setKnee (SampleType newKneeDB)
    {
        if (kneeDB != newKneeDB)
        {
            kneeDB = newKneeDB;
            recalcKnees();
            recalcConstants();
        }
    }

    /**
     * Processes a stream of audio data.
     * The levelBuffer should contain a level signal that is always greater than zero.
     * Calling this method will fill the gainBuffer with a set ig "gain" values,
     * which can be multiplied by the audio signal to apply compression.
     */
    void processBlock (const BufferView<const SampleType>& levelBuffer, const BufferView<SampleType>& gainBuffer, bool applyAutoMakeup) noexcept
    {
        jassert (levelBuffer.getNumSamples() == gainBuffer.getNumSamples());

        const auto numSamples = gainBuffer.getNumSamples();
        threshSmooth.process (juce::Decibels::decibelsToGain (threshDB), numSamples);
        ratioSmooth.process (ratio, numSamples);

        TupleHelpers::visit_at (computers,
                                modeIndex,
                                [this, &levelBuffer, &gainBuffer, applyAutoMakeup] (auto& computer)
                                {
                                    computer.process (levelBuffer,
                                                      gainBuffer,
                                                      {
                                                          threshSmooth.getSmoothedBuffer(),
                                                          ratioSmooth.getSmoothedBuffer(),
                                                          kneeDB,
                                                          kneeLower,
                                                          kneeUpper,
                                                          applyAutoMakeup,
                                                      });
                                });
    }

private:
    void recalcKnees() // recalculate knee values for a new threshold or knee width
    {
        kneeLower = juce::Decibels::decibelsToGain (threshDB - kneeDB * 0.5f);
        kneeUpper = juce::Decibels::decibelsToGain (threshDB + kneeDB * 0.5f);
    }

    void recalcConstants()
    {
        TupleHelpers::visit_at (computers,
                                modeIndex,
                                [this] (auto& computer)
                                {
                                    computer.recalcConstants (ratio, kneeDB);
                                });
    }

    SampleType threshDB = (SampleType) 0;
    SampleType ratio = (SampleType) 1;
    SmoothedBufferValue<SampleType, juce::ValueSmoothingTypes::Multiplicative> threshSmooth;
    SmoothedBufferValue<SampleType, juce::ValueSmoothingTypes::Multiplicative> ratioSmooth;
    SampleType kneeDB = (SampleType) 1;

    SampleType kneeUpper = (SampleType) 1;
    SampleType kneeLower = (SampleType) 1;

    size_t modeIndex = 0;
    typename GainComputerTypes::Types computers;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainComputer)
};
} // namespace chowdsp::compressor
