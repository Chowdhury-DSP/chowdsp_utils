#pragma once

#include <array>
#include <cmath>
#include <vector>

namespace chowdsp
{
/** Filter type options for State Variable Filters */
enum class OnePoleSVFType
{
    Lowpass,
    Highpass,
};

/** A simple 1-pole State Variable Filter */
template <typename SampleType, OnePoleSVFType type, size_t maxChannelCount = defaultChannelCount>
struct OnePoleSVF
{
    static constexpr int Order = 1;
    static constexpr auto Type = type;

    using FilterType = StateVariableFilterType;
    using NumericType = SampleTypeHelpers::NumericType<SampleType>;

    /** Prepares the processor to process an audio stream */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        T = static_cast<NumericType> (1.0 / spec.sampleRate);

        if constexpr (maxChannelCount == dynamicChannelCount)
        {
            ic1eq.resize (spec.numChannels);
        }
        else
        {
            jassert (spec.numChannels <= maxChannelCount);
        }
    }

    /** Resets the filter state */
    void reset (SampleType resetValue = {}) noexcept
    {
        std::fill (ic1eq.begin(), ic1eq.end(), resetValue);
    }

    /** Sets the filter cutoff frequency */
    void setCutoffFrequency (SampleType cutoffHz)
    {
        CHOWDSP_USING_XSIMD_STD (tan);
        const auto w = juce::MathConstants<NumericType>::pi * cutoffHz * T;
        g = tan (w);
        g1_r = NumericType (1) / (NumericType (1) + g);
    }

    /** Processes a block of audio */
    void processBlock (const BufferView<SampleType>& block) noexcept
    {
        for (auto [channel, sampleData] : buffer_iters::channels (block))
        {
            ScopedValue s1 { ic1eq[(size_t) channel] };
            for (auto& sample : sampleData)
                sample = processSampleInternal (sample, s1.get());
        }

#if JUCE_SNAP_TO_ZERO
        snapToZero();
#endif
    }

    /** Process a single sample */
    inline SampleType processSample (int channel, SampleType x) noexcept
    {
        return processSampleInternal (x, ic1eq[(size_t) channel]);
    }

    /** Internal use only! */
    inline SampleType processSampleInternal (SampleType x, SampleType& z) const noexcept
    {
        SampleType y;
        if (type == OnePoleSVFType::Lowpass)
        {
            y = (g * x + z) * g1_r;
            z = NumericType (2) * y - z;
        }
        else if (type == OnePoleSVFType::Highpass)
        {
            y = (x - z) * g1_r;
            z = NumericType (2) * (x - y) - z;
        }

        return y;
    }

    /** Snaps the internal state variables to zero if needed */
    void snapToZero() noexcept
    {
#if JUCE_SNAP_TO_ZERO
        for (auto& element : ic1eq)
            juce::dsp::util::snapToZero (element);
#endif
    }

    SampleType g {};
    SampleType g1_r {};

    using State = std::conditional_t<maxChannelCount == dynamicChannelCount, std::vector<SampleType>, std::array<SampleType, maxChannelCount>>;
    State ic1eq {}; // state variables

    NumericType T {};
};
} // namespace chowdsp
