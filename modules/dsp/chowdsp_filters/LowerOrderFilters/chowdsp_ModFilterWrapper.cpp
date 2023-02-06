#include "chowdsp_ModFilterWrapper.h"

namespace chowdsp
{
template <typename PrototypeFilter>
void ModFilterWrapper<PrototypeFilter>::prepare (const juce::dsp::ProcessSpec& spec)
{
    s1.resize (spec.numChannels);
    s2.resize (spec.numChannels);

    inChannelPointers.resize (spec.numChannels);
    outChannelPointers.resize (spec.numChannels);

    reset();
}

template <typename PrototypeFilter>
void ModFilterWrapper<PrototypeFilter>::reset()
{
    for (auto v : { &s1, &s2 })
        std::fill (v->begin(), v->end(), SampleType (0));
}

template <typename PrototypeFilter>
void ModFilterWrapper<PrototypeFilter>::snapToZero() noexcept // NOSONAR (cannot be const)
{
#if JUCE_SNAP_TO_ZERO
    for (auto v : { &s1, &s2 })
        for (auto& element : *v)
            juce::dsp::util::snapToZero (element);
#endif
}

template <typename PrototypeFilter>
template <typename... Args>
void ModFilterWrapper<PrototypeFilter>::calcCoefs (Args&&... args)
{
    prototypeFilter.calcCoefs (std::forward<Args> (args)...);
    update();
}

template <typename PrototypeFilter>
void ModFilterWrapper<PrototypeFilter>::processBlock (const BufferView<SampleType>& block) noexcept
{
    for (auto [channel, sampleData] : buffer_iters::channels (block))
    {
        ScopedValue<SampleType> m1 { s1[(size_t) channel] };
        ScopedValue<SampleType> m2 { s2[(size_t) channel] };

        for (auto& sample : sampleData)
            sample = processSample (sample, m1.get(), m2.get());
    }

#if JUCE_SNAP_TO_ZERO
    snapToZero();
#endif
}

template <typename PrototypeFilter>
template <typename ProcessContext>
void ModFilterWrapper<PrototypeFilter>::process (const ProcessContext& context) noexcept
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
        outputBlock.copyFrom (inputBlock);
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

template <typename PrototypeFilter>
template <typename ProcessContext, typename Modulator>
void ModFilterWrapper<PrototypeFilter>::process (const ProcessContext& context, Modulator&& mod) noexcept
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
        outputBlock.copyFrom (inputBlock);
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

template <typename PrototypeFilter>
void ModFilterWrapper<PrototypeFilter>::update()
{
    using namespace SIMDUtils;
    const auto& a = prototypeFilter.a;
    const auto& b = prototypeFilter.b;

    // do calculation from biquad coefficients
    SampleType four_gSqr;
    if constexpr (PrototypeFilter::Order == 1)
    {
        const auto tau = (SampleType) 1 - a[1];
        four_gSqr = ((SampleType) 4 / tau) * (1 + a[1]);
        if constexpr (std::is_floating_point_v<SampleType>)
            g2 = std::sqrt (four_gSqr);
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
            g2 = sqrtSIMD (four_gSqr);

        R2 = (SampleType) 4 / (g2 * tau);
        c0 = (b[0] - b[1]) / tau;
        c1 = (SampleType) 4 * b[0] / (g2 * tau);
        c2 = (SampleType) 4 * (b[0] + b[1]) / (four_gSqr * tau);
    }
    else if constexpr (PrototypeFilter::Order == 2)
    {
        const auto tau = (SampleType) 1 - a[1] + a[2];
        four_gSqr = ((SampleType) 4 / tau) * (1 + a[1] + a[2]);
        if constexpr (std::is_floating_point_v<SampleType>)
            g2 = std::sqrt (four_gSqr);
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
            g2 = sqrtSIMD (four_gSqr);

        R2 = (SampleType) 4 * (1 - a[2]) / (g2 * tau);
        c0 = (b[0] - b[1] + b[2]) / tau;
        c1 = (SampleType) 4 * (b[0] - b[2]) / (g2 * tau);
        c2 = (SampleType) 4 * (b[0] + b[1] + b[2]) / (four_gSqr * tau);
    }

    // update coefficients for SVF
    g = (SampleType) 0.5 * g2;
    two_gSqr = (SampleType) 0.5 * four_gSqr;
    gSqr = (SampleType) 0.25 * four_gSqr;
    h = ((NumericType) 1.0 / ((NumericType) 1.0 + R2 * g + gSqr));

    c1 = c1 + c2 * g;
    c0 = c0 + c1 * g;
}
} // namespace chowdsp
