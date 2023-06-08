#pragma once

#include <future>

namespace chowdsp
{
/** Mode to use for evaluating ADAA */
enum class ADAAWaveshaperMode
{
    Direct = 0, /**< Uses ADAA to evaluate f(x) */
    MinusX, /**< Uses ADAA to evaluate f(x) - x, and then adds x back in afterwards. */
};

/**
 * Waveshaper using second-order ADAA, with lookup-tables for speed.
 *
 * Note that this processor will always add exactly one sample of latency
 * to the signal, _even_ when bypassed.
 */
template <typename T, ADAAWaveshaperMode mode = ADAAWaveshaperMode::MinusX, typename illConditionTolerance = ScientificRatio<1, -2>, bool compensateHighFreqs = false>
class ADAAWaveshaper
{
public:
    /** Constructs the waveshaper optionally using a lookup table cache */
    explicit ADAAWaveshaper (LookupTableCache* lutCache = nullptr, const std::string& lutBaseID = {})
    {
        setupLookupTableCache (lutCache, lutBaseID);
    }

    /**
     * Set up the waveshaper to use a lookup table cache.
     *
     * If you don't want the waveshaper to use a lookup table cache, you
     * can pass nullptr for the first argument.
     *
     * If the cache in non-null, then you must provide a base ID for
     * the lookup table cache to use when storing the lookup tables.
     *
     * N.B: This function must be called _before_ `initialise()`!
     */
    void setupLookupTableCache (LookupTableCache* lutCache, const std::string& lutBaseID = {})
    {
        lookupTableCache = lutCache;

        if (lookupTableCache != nullptr)
        {
            jassert (! lutBaseID.empty()); // lookup table ID must not be empty when using the lookup table cache!
            lut.setNonOwning (&lookupTableCache->addLookupTable<double> (lutBaseID + "_lut"));
            lut_AD1.setNonOwning (&lookupTableCache->addLookupTable<double> (lutBaseID + "_lut_ad1"));
            lut_AD2.setNonOwning (&lookupTableCache->addLookupTable<double> (lutBaseID + "_lut_ad2"));
        }
        else
        {
            lut.setOwning (std::make_unique<LookupTableTransform<double>>());
            lut_AD1.setOwning (std::make_unique<LookupTableTransform<double>>());
            lut_AD2.setOwning (std::make_unique<LookupTableTransform<double>>());
        }
    }

    /**
     * Initialises the waveshaper with a given waveshaping function,
     * along with the first two anti-derivatives of that function.
     * There are also optional arguments which can be used to provide
     * a range and size to use for the internal lookup tables.
     */
    template <typename FuncType, typename FuncTypeD1, typename FuncTypeD2>
    void initialise (FuncType&& nlFunc, FuncTypeD1&& nlFuncD1, FuncTypeD2&& nlFuncD2, T minVal = (T) -10, T maxVal = (T) 10, int N = 1 << 18)
    {
        // load lookup tables asynchronously
        if (lut->initialiseIfNotAlreadyInitialised())
            lutLoadingFutures.push_back (std::async (std::launch::async, [&, func = std::forward<FuncType> (nlFunc), minVal, maxVal, N]
                                                     { lut->initialise (
                                                           [&func] (auto x)
                                                           {
                                                               if constexpr (mode == ADAAWaveshaperMode::Direct)
                                                                   return func ((double) x);
                                                               else if constexpr (mode == ADAAWaveshaperMode::MinusX)
                                                                   return func ((double) x) - (double) x;
                                                           },
                                                           minVal,
                                                           maxVal,
                                                           (size_t) N); }));

        if (lut_AD1->initialiseIfNotAlreadyInitialised())
            lutLoadingFutures.push_back (std::async (std::launch::async, [&, funcD1 = std::forward<FuncTypeD1> (nlFuncD1), minVal, maxVal, N]
                                                     { lut_AD1->initialise (
                                                           [&funcD1] (auto x)
                                                           {
                                                               if constexpr (mode == ADAAWaveshaperMode::Direct)
                                                                   return funcD1 ((double) x);
                                                               else if constexpr (mode == ADAAWaveshaperMode::MinusX)
                                                                   return funcD1 ((double) x) - 0.5 * Power::ipow<2> ((double) x); // + 0.5;
                                                           },
                                                           minVal,
                                                           maxVal,
                                                           2 * (size_t) N); }));

        if (lut_AD2->initialiseIfNotAlreadyInitialised())
            lutLoadingFutures.push_back (std::async (std::launch::async, [&, funcD2 = std::forward<FuncTypeD2> (nlFuncD2), minVal, maxVal, N]
                                                     { lut_AD2->initialise (
                                                           [&funcD2] (auto x)
                                                           {
                                                               if constexpr (mode == ADAAWaveshaperMode::Direct)
                                                                   return funcD2 ((double) x);
                                                               else if constexpr (mode == ADAAWaveshaperMode::MinusX)
                                                                   return funcD2 ((double) x) - (1.0 / 6.0) * Power::ipow<3> ((double) x); //+ 0.5 * (double) x;
                                                           },
                                                           minVal,
                                                           maxVal,
                                                           4 * (size_t) N); }));
    }

    /** Prepares the waveshaper for a given number of channels. */
    void prepare (int numChannels)
    {
        x1.resize ((size_t) numChannels, 0.0);
        x2.resize ((size_t) numChannels, 0.0);
        ad2_x0.resize ((size_t) numChannels, 0.0);
        ad2_x1.resize ((size_t) numChannels, 0.0);
        d2.resize ((size_t) numChannels, 0.0);

        if constexpr (compensateHighFreqs)
        {
            compFilter.prepare (numChannels);
            compFilter.calcCoefsDB ((1.0 / 3.0), 24.0, 32.0, 1.0);
        }

        lutLoadingFutures.clear();
    }

    /** Resets the waveshaper state. */
    void reset()
    {
        std::fill (x1.begin(), x1.end(), 0.0);
        std::fill (x2.begin(), x2.end(), 0.0);
        std::fill (ad2_x0.begin(), ad2_x0.end(), 0.0);
        std::fill (ad2_x1.begin(), ad2_x1.end(), 0.0);
        std::fill (d2.begin(), d2.end(), 0.0);

        if constexpr (compensateHighFreqs)
            compFilter.reset();
    }

    /** Process a single sample */
    inline T processSample (T input, int channel = 0) noexcept
    {
        const auto ch = (size_t) channel;
        const auto x = [&]
        {
            if constexpr (compensateHighFreqs)
                return compFilter.processSample ((double) input, channel);
            else
                return (double) input;
        }();

        bool illCondition = std::abs (x - x2[ch]) < TOL;
        const auto d1 = calcD1 (x, x1[ch], ad2_x0[ch], ad2_x1[ch]);
        auto y = T (illCondition ? fallback (x, x1[ch], x2[ch], ad2_x1[ch]) : (2.0 / (x - x2[ch])) * (d1 - d2[ch]));

        if constexpr (mode == ADAAWaveshaperMode::MinusX)
            y += (T) x1[ch];

        // update state
        d2[ch] = d1;
        x2[ch] = x1[ch];
        x1[ch] = x;
        ad2_x1[ch] = ad2_x0[ch];

        return y;
    }

    /** Processes a block of samples. */
    void process (T* output, const T* input, int numSamples, int channel = 0) noexcept
    {
        ScopedValue<double> _x1 { x1[(size_t) channel] };
        ScopedValue<double> _x2 { x2[(size_t) channel] };
        ScopedValue<double> _ad2_x0 { ad2_x0[(size_t) channel] };
        ScopedValue<double> _ad2_x1 { ad2_x1[(size_t) channel] };
        ScopedValue<double> _d2 { d2[(size_t) channel] };

        for (int n = 0; n < numSamples; ++n)
        {
            const auto x = [&]
            {
                if constexpr (compensateHighFreqs)
                    return compFilter.processSample ((double) input[n], channel);
                else
                    return (double) input[n];
            }();

            bool illCondition = std::abs (x - _x2.get()) < TOL;
            const auto d1 = calcD1 (x, _x1.get(), _ad2_x0.get(), _ad2_x1.get());
            output[n] = T (illCondition ? fallback (x, _x1.get(), _x2.get(), _ad2_x1.get()) : (2.0 / (x - _x2.get())) * (d1 - _d2.get()));

            if constexpr (mode == ADAAWaveshaperMode::MinusX)
                output[n] += (T) _x1.get();

            // update state
            _d2.get() = d1;
            _x2.get() = _x1.get();
            _x1.get() = x;
            _ad2_x1.get() = _ad2_x0.get();
        }
    }

    /** Processes a block of samples in bypassed mode. */
    void processBypassed (T* output, const T* input, int numSamples, int channel = 0) noexcept
    {
        ScopedValue<double> _x1 { x1[(size_t) channel] };
        ScopedValue<double> _x2 { x2[(size_t) channel] };
        ad2_x0[(size_t) channel] = 0.0;
        ad2_x1[(size_t) channel] = 0.0;
        d2[(size_t) channel] = 0.0;

        for (int n = 0; n < numSamples; ++n)
        {
            // add a one-sample delay to the bypassed signal so that everything matches up!
            const auto x = [&]
            {
                if constexpr (compensateHighFreqs)
                    return compFilter.processSample ((double) input[n], channel);
                else
                    return (double) input[n];
            }();
            output[n] = T (_x1.get());

            // update state
            _x2.get() = _x1.get();
            _x1.get() = x;
        }
    }

    /** Process a block of samples. */
    void processBlock (const BufferView<T>& buffer) noexcept
    {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();
        jassert ((int) x1.size() <= numChannels);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const auto* inputData = buffer.getReadPointer (ch);
            auto* outputData = buffer.getWritePointer (ch);

            process (outputData, inputData, numSamples, ch);
        }
    }

    /** Process a block of samples (bypassed). */
    void processBlockBypassed (const BufferView<T>& buffer) noexcept
    {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();
        jassert ((int) x1.size() <= numChannels);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const auto* inputData = buffer.getReadPointer (ch);
            auto* outputData = buffer.getWritePointer (ch);

            processBypassed (outputData, inputData, numSamples, ch);
        }
    }

    /** Processes the given processing context */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();

        const auto numSamples = (int) inputBlock.getNumSamples();
        const auto numChannels = inputBlock.getNumChannels();
        jassert (outputBlock.getNumChannels() == numChannels);
        jassert (x1.size() == numChannels);

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            const auto* inputData = inputBlock.getChannelPointer (ch);
            auto* outputData = outputBlock.getChannelPointer (ch);

            if (context.isBypassed)
                processBypassed (outputData, inputData, numSamples, (int) ch);
            else
                process (outputData, inputData, numSamples, (int) ch);
        }
    }

private:
    // @TODO: are there situations when we can safely use processSampleUnchecked()?
    [[nodiscard]] inline double nlFunc (double x) const noexcept { return lut->processSample (x); }
    [[nodiscard]] inline double nlFunc_AD1 (double x) const noexcept { return lut_AD1->processSample (x); }
    [[nodiscard]] inline double nlFunc_AD2 (double x) const noexcept { return lut_AD2->processSample (x); }

    inline double calcD1 (double x0, const double& _x1, double& _ad2_x0, const double& _ad2_x1) noexcept
    {
        bool illCondition = std::abs (x0 - _x1) < TOL;
        _ad2_x0 = nlFunc_AD2 (x0);
        return illCondition ? nlFunc_AD1 (0.5 * (x0 + _x1)) : (_ad2_x0 - _ad2_x1) / (x0 - _x1);
    }

    inline double fallback (double x, const double& _x1, const double& _x2, const double& _ad2_x1) noexcept
    {
        const auto xBar = 0.5 * (x + _x2);
        const auto delta = xBar - _x1;
        bool illCondition = std::abs (delta) < TOL;
        return illCondition ? nlFunc (0.5 * (xBar + _x1)) : (2.0 / delta) * (nlFunc_AD1 (xBar) + (_ad2_x1 - nlFunc_AD2 (xBar)) / delta);
    }

    OptionalPointer<LookupTableTransform<double>> lut {};
    OptionalPointer<LookupTableTransform<double>> lut_AD1 {};
    OptionalPointer<LookupTableTransform<double>> lut_AD2 {};

    // state
    std::vector<double> x1;
    std::vector<double> x2;
    std::vector<double> ad2_x0;
    std::vector<double> ad2_x1;
    std::vector<double> d2;

    using CompensationFilter = std::conditional_t<compensateHighFreqs, PeakingFilter<double, CoefficientCalculators::CoefficientCalculationMode::Decramped>, NullType>;
    CompensationFilter compFilter {};

    static constexpr auto TOL = illConditionTolerance::template value<double>;

    std::vector<std::future<void>> lutLoadingFutures;

    LookupTableCache* lookupTableCache = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADAAWaveshaper)
};

} // namespace chowdsp
