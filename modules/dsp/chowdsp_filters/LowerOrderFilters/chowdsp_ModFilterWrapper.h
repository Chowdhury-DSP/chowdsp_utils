#pragma once

#include "../LowerOrderFilters/chowdsp_SecondOrderFilters.h"

namespace chowdsp
{
/**
 * chowdsp::IIRFilter is not good for doing audio-rate modulation.
 * So if you need a filter than _can_ do audio-rate modulation, just
 * wrap a first or second order IIRFilter in a ModFilterWrapper, and
 * voila, now you're good to go!
 *
 * The way it works is by constructing a State Variable Filter from the
 * coefficients of the biquad filter. See the reference code on page 18:
 * http://www.solostuff.net/wp-content/uploads/2019/05/Fast-Modulation-of-Filter-Parameters-v1.1.1.pdf
 */
template <typename PrototypeFilter>
class ModFilterWrapper
{
public:
    using SampleType = typename PrototypeFilter::SampleType;
    using NumericType = SampleTypeHelpers::ProcessorNumericType<PrototypeFilter>;
    static_assert (PrototypeFilter::Order == 1 || PrototypeFilter::Order == 2, "This method only works for first or second order filters!");

    ModFilterWrapper() = default;

    /** Prepares the file to process a new block of samples */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Reset's the filter state */
    void reset();

    /** Forwards arguments to the calcCoefs method of the prototype filter */
    template <typename... Args>
    void calcCoefs (Args&&... args);

    /** Process block of samples */
    void processBlock (const BufferView<SampleType>& block) noexcept;

    /** Process a block  */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept;

    /**
     * Process a block with a modulation callback which will be called every sample.
     *
     * The modulation callback should be a lambda of the form [](size_t sampleIndex) {}.
     */
    template <typename ProcessContext, typename Modulator>
    void process (const ProcessContext& context, Modulator&& mod) noexcept;

private:
    inline SampleType processSample (SampleType x, SampleType& m1, SampleType& m2) noexcept
    {
        auto hp = (x - (g + R2) * m1 - m2) * h;

        auto y = c0 * hp + c1 * m1 + c2 * m2;
        m2 = two_gSqr * hp + g2 * m1 + m2;
        m1 = g2 * hp + m1;

        return y;
    }

    void update();
    void snapToZero() noexcept;

    PrototypeFilter prototypeFilter;

    SampleType g, g2, gSqr, two_gSqr, h, R2, c0, c1, c2;
    std::vector<SampleType> s1 { 2 }, s2 { 2 };

    std::vector<SampleType*> outChannelPointers {};
    std::vector<const SampleType*> inChannelPointers {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModFilterWrapper)
};
} // namespace chowdsp

#include "chowdsp_ModFilterWrapper.cpp"
