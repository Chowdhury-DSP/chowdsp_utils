#pragma once

#include <chowdsp_dsp/chowdsp_dsp.h>

namespace chowdsp
{
/**
 * http://www.solostuff.net/wp-content/uploads/2019/05/Fast-Modulation-of-Filter-Parameters-v1.1.1.pdf
 */
template <typename PrototypeFilter>
class ModFilterWrapper
{
public:
    using SampleType = typename PrototypeFilter::SampleType;
    using NumericType = SampleTypeHelpers::ProcessorNumericType<PrototypeFilter>;

    ModFilterWrapper() = default;

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        s1.resize (spec.numChannels);
        s2.resize (spec.numChannels);

        inChannelPointers.resize (spec.numChannels);
        outChannelPointers.resize (spec.numChannels);

        reset();
    }

    void reset()
    {
        for (auto v : { &s1, &s2 })
            std::fill (v->begin(), v->end(), SampleType (0));
    }

    void snapToZero() noexcept
    {
        for (auto v : { &s1, &s2 })
            for (auto& element : *v)
                juce::dsp::util::snapToZero (element);
    }

    template <typename... Args>
    void calcCoefs (Args&&... args)
    {
        prototypeFilter.calcCoefs (std::forward<Args> (args)...);
        update();
    }

    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();

        jassert (inputBlock.getNumChannels() <= s1.size());
        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumSamples() == numSamples);

        if (context.isBypassed)
        {
            AudioBlockHelpers::copyBlocks (outputBlock, inputBlock);
            return;
        }

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* inputSamples = inputBlock.getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            ScopedValue<SampleType> m1 { s1[channel] };
            ScopedValue<SampleType> m2 { s2[channel] };

            for (size_t i = 0; i < numSamples; ++i)
                outputSamples[i] = processSample (inputSamples[i], m1.get(), m2.get());
        }

#if JUCE_SNAP_TO_ZERO
        snapToZero();
#endif
    }

    template <typename ProcessContext, typename Modulator>
    void process (const ProcessContext& context, Modulator&& mod) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();

        jassert (inputBlock.getNumChannels() <= s1.size());
        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumSamples() == numSamples);

        if (context.isBypassed)
        {
            AudioBlockHelpers::copyBlocks (outputBlock, inputBlock);
            return;
        }

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            inChannelPointers[channel] = inputBlock.getChannelPointer (channel);
            outChannelPointers[channel] = outputBlock.getChannelPointer (channel);
        }

        for (size_t i = 0; i < numSamples; ++i)
        {
            mod (i);
            for (size_t channel = 0; channel < numChannels; ++channel)
                outChannelPointers[channel][i] = processSample (inChannelPointers[channel][i], s1[channel], s2[channel]);
        }

#if JUCE_SNAP_TO_ZERO
        snapToZero();
#endif
    }

    PrototypeFilter prototypeFilter;

private:
    SampleType processSample (SampleType x, SampleType& m1, SampleType& m2) noexcept
    {
        auto hp = (x - (g + R2) * m1 - m2) * h;
        auto bp = g * hp + m1;
        auto lp = g * bp + m2;
        m1 = g * hp + bp;
        m2 = g * bp + lp;
        return c0 * hp + c1 * bp + c2 * lp;
    }

    void update()
    {
        using namespace SIMDUtils;

        const auto& a = prototypeFilter.a;
        const auto& b = prototypeFilter.b;

        const auto tau = (SampleType) 1 - a[1] + a[2];
        const auto gSqr = ((SampleType) 4 / tau) * (1 + a[1] + a[2]);
        if constexpr (std::is_floating_point_v<SampleType>)
            g = std::sqrt (gSqr);
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
            g = sqrtSIMD (gSqr);

        R2 = (SampleType) 4 * (1 - a[2]) / (g * tau);
        c0 = (b[0] - b[1] + b[2]) / tau;
        c1 = (SampleType) 4 * (b[0] - b[2]) / (g * tau);
        c2 = (SampleType) 4 * (b[0] + b[1] + b[2]) / (gSqr * tau);

        g = (SampleType) 0.5 * g;
        h = ((NumericType) 1.0 / ((NumericType) 1.0 + R2 * g + gSqr));
    }

    SampleType g, h, R2, c0, c1, c2;
    std::vector<SampleType> s1 { 2 }, s2 { 2 };

    std::vector<SampleType*> outChannelPointers;
    std::vector<const SampleType*> inChannelPointers;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModFilterWrapper)
};
} // namespace chowdsp
