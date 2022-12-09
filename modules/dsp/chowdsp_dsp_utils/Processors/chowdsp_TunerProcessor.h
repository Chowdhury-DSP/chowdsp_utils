#pragma once

namespace chowdsp
{
/** Helpful methods for workign with tuning information */
namespace TuningHelpers
{
    [[maybe_unused]] inline double getMidiNoteInHertz (const int noteNumber, const double frequencyOfA = 440.0) noexcept
    {
        return frequencyOfA * std::pow (2.0, (noteNumber - 69) / 12.0);
    }

    /** Converts a frequency in Hz to the corresponding MIDI note number, and cents away from that note's correct frequency */
    [[maybe_unused]] inline std::pair<int, double> frequencyHzToNoteAndCents (double freq) noexcept
    {
        const int noteNum = juce::roundToIntAccurate (12.0 * std::log2 (freq / 440.0) + 69.0);

        const auto idealFreq = getMidiNoteInHertz (noteNum);
        double cents = 1200.0 * std::log2 (freq / idealFreq);

        return std::make_pair (noteNum, cents);
    }
} // namespace TuningHelpers

/**
 * Simple monphonic auto-correlation based frequency
 * detection that can be used for a tuner.
 */
template <typename T>
class TunerProcessor
{
public:
    TunerProcessor() = default;

    /** Returns the size of the buffer needed to compute auto-correlation-based frequency detection */
    [[nodiscard]] int getAutocorrelationSize() const noexcept { return autocorrelationSize; }

    /** Returns the current frequency detected by the tuner */
    T getCurrentFrequencyHz() const noexcept { return curFreqHz; }

    /** Prepares the tuner to process data at a given sample rate */
    void prepare (double sampleRate)
    {
        constexpr double lowestFreqHz = 10.0;
        autocorrelationSize = juce::nextPowerOfTwo (int (sampleRate / lowestFreqHz) + 1);

        fs = (T) sampleRate;
        curFreqHz = 1.0;
    }

    /**
     * Performs the frequency detection calculation.
     * Note that this method is not designed to be called every block,
     * but only when the latest frequency is needed.
     *
     * The length of the data vector must be the same as the requested autocorrelation size.
     */
    void process (const T* data)
    {
        // exit early if buffer is silent!
        if (FloatVectorOperations::findAbsoluteMaximum (data, autocorrelationSize) < (T) 1.0e-2)
        {
            curFreqHz = (T) 1;
            return;
        }

        // simple autocorrelation-based frequency detection
        auto sumOld = (T) 0;
        auto sum = (T) 0;
        int period = 0;
        auto thresh = (T) 0;
        bool firstPeakFound = false;

        for (int i = 0; i < autocorrelationSize; ++i)
        {
            sumOld = sum;
            sum = FloatVectorOperations::innerProduct (data, &data[i], autocorrelationSize - i);

            // set threshold on first pass
            if (i == 0)
            {
                thresh = sum * (T) 0.5;
                continue;
            }

            // check if we've found the first peak
            if (! firstPeakFound && (sum > thresh) && (sum - sumOld) > 0)
            {
                firstPeakFound = true;
                continue;
            }

            // check if we've started decreasing after the first peak
            if (firstPeakFound && (sum - sumOld) <= 0)
            {
                period = i;
                break;
            }
        }

        if (period > 0)
            curFreqHz = fs / (T) period;
    }

private:
    T curFreqHz = (T) 1;

    T fs = (T) 48000;
    int autocorrelationSize = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TunerProcessor)
};
} // namespace chowdsp
