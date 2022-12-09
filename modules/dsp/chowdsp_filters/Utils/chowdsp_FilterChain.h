#pragma once

#include <tuple>

namespace chowdsp
{
/**
 * Wrapper for a serial chain of filters.
 *
 * This class is compatible with most of the other filters in this module.
 */
template <typename... FilterTypes>
class FilterChain
{
public:
    /** Default constructor */
    FilterChain() = default;

    /** Move constructor */
    FilterChain (FilterChain&&) noexcept = default;

    /** Move assignment operator */
    FilterChain& operator= (FilterChain&&) noexcept = default;

    /** Returns a reference to one of the filters in the chain */
    template <int Index>
    auto& get() noexcept
    {
        return std::get<Index> (filters);
    }

    /** Returns a const reference to one of the filters in the chain */
    template <int Index>
    const auto& get() const noexcept
    {
        return std::get<Index> (filters);
    }

    /** Do the same operation for each filter in the chain */
    template <typename Func>
    void doForEachFilter (Func&& func)
    {
        TupleHelpers::forEachInTuple ([&] (auto& filter, size_t)
                                      { func (filter); },
                                      filters);
    }

    /** Prepares each filter in the chain */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        doForEachFilter ([&spec] (auto& f)
                         { f.prepare (spec); });
    }

    /** Resets each filter in the chain */
    void reset()
    {
        doForEachFilter ([] (auto& f)
                         { f.reset(); });
    }

    /** Processes an individual sample through the filter chain */
    template <typename T>
    T processSample (T x)
    {
        doForEachFilter ([&] (auto& f)
                         { x = f.processSample (x); });
        return x;
    }

    /** Processes a block of samples through the filter chain */
    template <typename T>
    void processBlock (T* x, int numSamples)
    {
        doForEachFilter ([&] (auto& f)
                         { f.processBlock (x, numSamples); });
    }

    /** Processes a block of samples through the filter chain */
    template <typename T>
    void processBlock (const BufferView<T>& buffer)
    {
        doForEachFilter ([&] (auto& f)
                         { f.processBlock (buffer); });
    }

    /** Processes a buffer or processing context through the filter chain */
    template <typename... Args>
    void process (Args&&... args)
    {
        doForEachFilter ([&] (auto& f)
                         { f.process (std::forward<Args> (args)...); });
    }

private:
    std::tuple<FilterTypes...> filters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterChain)
};
} // namespace chowdsp
