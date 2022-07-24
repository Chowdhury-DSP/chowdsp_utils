#pragma once

namespace chowdsp
{
/**
 * A cache that can be used for storing lookup tables,
 * which may need to be used more than once.
 */
class LookupTableCache
{
public:
    LookupTableCache() = default;

    /**
     * Adds a lookup table to the cache, or returns the lookup table
     * if a lookup table with the given ID is already present in the cache.
     */
    template <typename FloatType>
    LookupTableTransform<FloatType>& addLookupTable (const std::string& tableID)
    {
        if constexpr (std::is_same_v<FloatType, float>)
        {
            if (auto tableIter = floatLUTs.find (tableID); tableIter != floatLUTs.end())
                return tableIter->second;
            return floatLUTs[tableID];
        }
        else if constexpr (std::is_same_v<FloatType, double>)
        {
            if (auto tableIter = doubleLUTs.find (tableID); tableIter != doubleLUTs.end())
                return tableIter->second;
            return doubleLUTs[tableID];
        }
        else
        {
            // lookup table must be either a float or double type
            jassertfalse;
        }
    }

    /** Clears any lookup tables currently stored in the cache. */
    void clearCache()
    {
        floatLUTs.clear();
        doubleLUTs.clear();
    }

private:
    std::unordered_map<std::string, LookupTableTransform<float>> floatLUTs;
    std::unordered_map<std::string, LookupTableTransform<double>> doubleLUTs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookupTableCache)
};

#if CHOWDSP_USING_JUCE
/** Helpful alias for creating a SharedResourcePointer<LookupTableCache> */
using SharedLookupTableCache = juce::SharedResourcePointer<LookupTableCache>;
#endif
} // namespace chowdsp
