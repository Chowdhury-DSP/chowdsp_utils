#pragma once

namespace chowdsp
{
/**
 * Modified version of juce::dsp::LookupTableTransform with improved performance
 * when processing vectors.
 */
template <typename FloatType>
class LookupTableTransform
{
public:
    //==============================================================================
    /** Creates an uninitialised LookupTableTransform object.
        You need to call initialise() before using the object. Prefer using the
        non-default constructor instead.
        @see initialise
    */
    LookupTableTransform() = default;

    //==============================================================================
    /** Creates and initialises a LookupTableTransform object.
        @param functionToApproximate The function to be approximated. This should be a
                                     mapping from a FloatType to FloatType.
        @param minInputValueToUse    The lowest input value used. The approximation will
                                     fail for values lower than this.
        @param maxInputValueToUse    The highest input value used. The approximation will
                                     fail for values higher than this.
        @param numPoints             The number of pre-calculated values stored.
    */
    LookupTableTransform (const std::function<FloatType (FloatType)>& functionToApproximate,
                          FloatType minInputValueToUse,
                          FloatType maxInputValueToUse,
                          size_t numPoints)
    {
        initialise (functionToApproximate, minInputValueToUse, maxInputValueToUse, numPoints);
    }

    //==============================================================================
    /** Initialises or changes the parameters of a LookupTableTransform object.
        @param functionToApproximate The function to be approximated. This should be a
                                     mapping from a FloatType to FloatType.
        @param minInputValueToUse    The lowest input value used. The approximation will
                                     fail for values lower than this.
        @param maxInputValueToUse    The highest input value used. The approximation will
                                     fail for values higher than this.
        @param numPoints             The number of pre-calculated values stored.
    */
    void initialise (const std::function<FloatType (FloatType)>& functionToApproximate,
                     FloatType minInputValueToUse,
                     FloatType maxInputValueToUse,
                     size_t numPoints);

    /** Returns true if the lookup table has been initialised. */
    [[nodiscard]] bool hasBeenInitialised() const noexcept { return isInitialised; }

    /**
     * If you'd like to initialize this lookup table, you should call this method first!
     *
     * If the table is already initialized, this will return false. Otherwise, this will
     * return true, and all future calls to `hasBeenInitialized()` will return true, so
     * the class expects that the user will take care of initializing the table ASAP after
     * this method returns true.
     */
    [[nodiscard]] bool initialiseIfNotAlreadyInitialised() noexcept;

    //==============================================================================
    /** Calculates the approximated value for the given input value without range checking.
        Use this if you can guarantee that the input value is within the range specified
        in the constructor or initialise(), otherwise use processSample().
        @param value The approximation is calculated for this input value.
        @return      The approximated value for the provided input value.
        @see processSample, operator(), operator[]
    */
    [[nodiscard]] FloatType processSampleUnchecked (FloatType value) const noexcept
    {
        jassert (value >= minInputValue && value <= maxInputValue);
        return lookupTable[scaler * value + offset];
    }

    //==============================================================================
    /** Calculates the approximated value for the given input value with range checking.
        This can be called with any input values. Out-of-range input values will be
        clipped to the specified input range.
        If the index is guaranteed to be in range use the faster processSampleUnchecked()
        instead.
        @param value The approximation is calculated for this input value.
        @return      The approximated value for the provided input value.
        @see processSampleUnchecked, operator(), operator[]
    */
    [[nodiscard]] FloatType processSample (FloatType value) const noexcept
    {
        auto index = scaler * juce::jlimit (minInputValue, maxInputValue, value) + offset;
        jassert (juce::isPositiveAndBelow (index, FloatType (lookupTable.getNumPoints())));

        return lookupTable[index];
    }

    //==============================================================================
    /** @see processSampleUnchecked */
    FloatType operator[] (FloatType index) const noexcept { return processSampleUnchecked (index); }

    /** @see processSample */
    FloatType operator() (FloatType index) const noexcept { return processSample (index); }

    //==============================================================================
    /** Processes an array of input values without range checking
        @see process
    */
    void processUnchecked (const FloatType* input, FloatType* output, int numSamples) const noexcept
    {
        juce::FloatVectorOperations::multiply (output, input, scaler, numSamples);
        juce::FloatVectorOperations::add (output, output, offset, numSamples);

        for (int i = 0; i < numSamples; ++i)
            output[i] = lookupTable[output[i]];
    }

    //==============================================================================
    /** Processes an array of input values with range checking
        @see processUnchecked
    */
    void process (const FloatType* input, FloatType* output, int numSamples) const noexcept
    {
        juce::FloatVectorOperations::clip (output, input, minInputValue, maxInputValue, numSamples);
        processUnchecked (output, output, numSamples);
    }

private:
    //==============================================================================
    juce::dsp::LookupTable<FloatType> lookupTable;

    FloatType minInputValue, maxInputValue;
    FloatType scaler, offset;

    std::atomic_bool isInitialised { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookupTableTransform)
};
} // namespace chowdsp
