#pragma once

namespace chowdsp::compressor
{
template <typename SampleType, typename GainComputerTypes = types_list::TypesList<FeedForwardCompGainComputer<SampleType>, FeedBackCompGainComputer<SampleType>>>
class GainComputer
{
public:
    GainComputer() = default;

    void prepare (double sampleRate, int samplesPerBlock)
    {
        threshSmooth.setRampLength (0.05);
        threshSmooth.prepare (sampleRate, samplesPerBlock);

        ratioSmooth.setRampLength (0.05);
        ratioSmooth.prepare (sampleRate, samplesPerBlock);
    }

    void reset()
    {
        threshSmooth.reset (juce::Decibels::decibelsToGain (threshDB));
        ratioSmooth.reset (ratio);
    }

    void setMode (size_t newModeIndex)
    {
        if (newModeIndex != modeIndex)
        {
            modeIndex = newModeIndex;
            recalcConstants();
        }
    }

    void setThreshold (SampleType newThreshDB)
    {
        if (threshDB != newThreshDB)
        {
            threshDB = newThreshDB;
            recalcKnees();
        }
    }

    void setRatio (SampleType newRatio)
    {
        if (ratio != newRatio)
        {
            ratio = newRatio;
            recalcConstants();
        }
    }

    void setKnee (SampleType newKneeDB)
    {
        if (kneeDB != newKneeDB)
        {
            kneeDB = newKneeDB;
            recalcKnees();
            recalcConstants();
        }
    }

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
