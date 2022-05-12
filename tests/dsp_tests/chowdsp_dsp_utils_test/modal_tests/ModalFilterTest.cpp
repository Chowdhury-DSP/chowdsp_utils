#include <test_utils.h>
#include <TimedUnitTest.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

namespace
{
constexpr float fs = 44100.0f;
constexpr float modeDecay = 0.01f;
constexpr float modeAmp = 0.1f;
} // namespace

/** Unit tests for chowdsp::ModalFilter. Tests include:
 *   - Check that filter resonates at center frequency
 *   - Check that filter damps frequencies other than center frequency
 *   - Check that filter has correct decay time
 */
template <typename T>
class ModalFilterTest : public TimedUnitTest
{
public:
    using NumericType = typename juce::dsp::SampleTypeHelpers::ElementType<T>::Type;

    ModalFilterTest() : TimedUnitTest ("Modal Filter Test " + getSampleType(), "Modal") {}

    static juce::String getSampleType()
    {
        if (std::is_same_v<T, float>)
            return "Float";
        if (std::is_same_v<T, double>)
            return "Double";
        if (std::is_same_v<T, juce::dsp::SIMDRegister<float>>)
            return "SIMD Float";
        if (std::is_same_v<T, juce::dsp::SIMDRegister<double>>)
            return "SIMD Double";

        return "Unknown";
    }

    static std::vector<T> getTestVector (juce::AudioBuffer<float> buffer)
    {
        std::vector<T> vector ((size_t) buffer.getNumSamples(), (T) 0);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            vector[(size_t) i] = (T) (NumericType) buffer.getSample (0, i);

        return vector;
    }

    template <typename C = T>
    static typename std::enable_if<std::is_same<float, C>::value || std::is_same<double, C>::value, T>::type
        getMagnitude (const std::vector<T>& buffer, int start = 0, int num = -1)
    {
        num = num >= 0 ? num : (int) buffer.size();

        auto max = (T) 0;
        for (int i = start; i < start + num; ++i)
            max = juce::jmax (max, buffer[(size_t) i]);

        return max;
    }

    template <typename C = T>
    static typename std::enable_if<std::is_same<juce::dsp::SIMDRegister<float>, C>::value || std::is_same<juce::dsp::SIMDRegister<double>, C>::value, NumericType>::type
        getMagnitude (const std::vector<T>& buffer, int start = 0, int num = -1)
    {
        num = num >= 0 ? num : (int) buffer.size();

        auto max = (T) 0;
        for (int i = start; i < start + num; ++i)
            max = juce::jmax (max, buffer[(size_t) i]);

        return max.get (0);
    }

    void onFreqSineTest()
    {
        constexpr float testFreq = 100.0f;
        auto buffer = getTestVector (test_utils::makeSineWave (testFreq, fs, 1.0f));
        auto refMag = juce::Decibels::gainToDecibels (getMagnitude (buffer));

        chowdsp::ModalFilter<T> filter;
        filter.prepare ((NumericType) fs);
        filter.setAmp ((T) modeAmp);
        filter.setDecay ((T) modeDecay);
        filter.setFreq ((T) testFreq);
        filter.processBlock (buffer.data(), (int) buffer.size());

        auto mag = juce::Decibels::gainToDecibels (getMagnitude (buffer));
        expectGreaterThan (mag - refMag, (NumericType) 6.0f, "Modal filter is not resonating at correct frequency.");
    }

    void offFreqSineTest()
    {
        constexpr float testFreq1 = 100.0f;
        constexpr float testFreq2 = 10000.0f;
        auto buffer = getTestVector (test_utils::makeSineWave (testFreq1, fs, 1.0f));
        auto refMag = juce::Decibels::gainToDecibels (getMagnitude (buffer));

        chowdsp::ModalFilter<T> filter;
        filter.prepare ((NumericType) fs);
        filter.setAmp ((T) modeAmp);
        filter.setDecay ((T) modeDecay);
        filter.setFreq ((T) testFreq2);
        filter.processBlock (buffer.data(), (int) buffer.size());

        if constexpr (std::is_floating_point_v<T>)
            expectEquals (filter.getFreq(), (NumericType) testFreq2, "Modal filter frequency is incorrect");
        else
            expectEquals (filter.getFreq().get (0), (NumericType) testFreq2, "Modal filter frequency is incorrect");

        auto mag = juce::Decibels::gainToDecibels (getMagnitude (buffer));
        expectLessThan (mag - refMag, (NumericType) -24.0f, "Modal filter is resonating at an incorrect frequency.");
    }

    void decayTimeTest()
    {
        auto buffer = getTestVector (test_utils::makeImpulse (1.0f, fs, 1.0f));

        chowdsp::ModalFilter<T> filter;
        filter.prepare ((NumericType) fs);
        filter.setAmp ((T) 1.0f);
        filter.setDecay ((T) 0.5f);
        filter.setFreq ((T) 100.0f);
        filter.processBlock (buffer.data(), (int) buffer.size());

        auto mag = juce::Decibels::gainToDecibels (getMagnitude (buffer, int (fs * 0.5f), int (fs * 0.1f)));
        expectWithinAbsoluteError (mag, (NumericType) -60.0f, (NumericType) 1.0f, "Incorrect decay time.");
    }

    void initialPhaseTest()
    {
        static constexpr auto initialPhase = (NumericType) 0.5f;

        chowdsp::ModalFilter<T> filter;
        filter.prepare ((NumericType) fs);
        filter.setAmp ((T) 1.0f, initialPhase);
        filter.setDecay ((T) 0.5f);
        filter.setFreq ((T) 100.0f);

        const auto actual = filter.processSample ((T) 1);
        const auto expected = std::sin (initialPhase);

        if constexpr (chowdsp::SIMDUtils::IsSIMDRegister<T>)
            expect (chowdsp::SIMDUtils::all (T::equal (actual, expected)), "Incorrect initial phase!");
        else
            expectEquals (actual, expected, "Incorrect initial phase");
    }

    void runTestTimed() override
    {
        beginTest ("Resonant Frequency Sine Test");
        onFreqSineTest();

        beginTest ("Damped Frequency Sine Test");
        offFreqSineTest();

        beginTest ("Decay Time Test");
        decayTimeTest();

        beginTest ("Initial Phase Test");
        initialPhaseTest();
    }
};

static ModalFilterTest<float> mfTestFloat;
static ModalFilterTest<double> mfTestDouble;
static ModalFilterTest<juce::dsp::SIMDRegister<float>> mfTestSimdFloat;
static ModalFilterTest<juce::dsp::SIMDRegister<double>> mfTestSimdDouble;
