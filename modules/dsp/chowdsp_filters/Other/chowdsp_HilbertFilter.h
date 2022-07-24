#pragma once

namespace chowdsp
{
/**
 * Filter cascades to implement an approximate Hilbert transform
 * (90 degree phase shift). This implementation is based on the
 * approach described in this article: http://yehar.com/blog/?p=368.
 */
template <typename T>
class HilbertFilter
{
public:
    HilbertFilter() = default;

    /** Resets the processor state */
    void reset()
    {
        for (auto& f : filter1)
            f.reset();

        for (auto& f : filter2)
            f.reset();

        oneSampleDelay = (T) 0;
    }

    /**
     * Processes a single sample.
     *
     * @param x     The input sample.
     * @return      A pair containing the output sample in-phase, and the output sample +90 degrees.
     */
    inline auto process (T x) noexcept
    {
        // First APF chain, plus 1-sample delay
        T h1 = x;
        for (auto& f : filter1)
            h1 = f.process (h1);
        auto h1Delayed = oneSampleDelay;
        oneSampleDelay = h1;

        // Second APF chain
        T h2 = x;
        for (auto& f : filter2)
            h2 = f.process (h2);

        return std::make_pair (h1Delayed, h2);
    }

private:
    /** An optimized one-multiply second-order allpass filter, */
    struct AllpassStage
    {
        AllpassStage (T alpha) : alphaSq (alpha * alpha) {} // NOLINT NOSONAR (don't want this to be explicit so we can construct the array below)

        void reset()
        {
            std::fill (in, in + 3, (T) 0);
            std::fill (out, out + 3, (T) 0);
        }

        inline T process (T x) noexcept
        {
            auto y = alphaSq * (x + out[2]) - in[2];

            in[2] = in[1];
            in[1] = x;
            out[2] = out[1];
            out[1] = y;

            return y;
        }

        const T alphaSq;
        T in[3] {};
        T out[3] {};
    };

    // The coefficients here are from the article linked above.
    std::array<AllpassStage, 4> filter1 { (T) 0.6923878, (T) 0.9360654322959, (T) 0.9882295226860, (T) 0.9987488452737 };
    std::array<AllpassStage, 4> filter2 { (T) 0.4021921162426, (T) 0.8561710882420, (T) 0.9722909545651, (T) 0.9952884791278 };
    T oneSampleDelay = (T) 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HilbertFilter)
};
} // namespace chowdsp
