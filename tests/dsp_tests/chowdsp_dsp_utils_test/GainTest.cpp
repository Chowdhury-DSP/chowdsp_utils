#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>
#include <algorithm>

namespace
{
constexpr float fs = 44100.0f;
constexpr int blockSize = 512;
constexpr auto maxErr = 0.01;
} // namespace

TEMPLATE_TEST_CASE ("Gain Test", "[dsp][misc]", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using T = TestType;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<T>;

    SECTION ("Parameter Checks")
    {
        chowdsp::Gain<T> gain;

        gain.setGainLinear ((NumericType) 2);
        REQUIRE_MESSAGE (juce::exactlyEqual (gain.getGainLinear(), (NumericType) 2), "Set linear gain is incorrect!");
        REQUIRE_MESSAGE (gain.getGainDecibels() == Catch::Approx ((NumericType) 6).margin (0.03), "Get Decibels gain is incorrect!");

        gain.setGainDecibels ((NumericType) -6);
        REQUIRE_MESSAGE (gain.getGainDecibels() == Catch::Approx ((NumericType) -6).margin (1.0e-6), "Set Decibels gain is incorrect!");
        REQUIRE_MESSAGE (gain.getGainLinear() == Catch::Approx ((NumericType) 0.5).margin (0.02), "Get linear gain is incorrect!");

        gain.setRampDurationSeconds (0.05);
        REQUIRE_MESSAGE (juce::exactlyEqual (gain.getRampDurationSeconds(), 0.05), "Set ramp duration is incorrect!");
    }

    SECTION ("Smooth Gain Test")
    {
        chowdsp::Gain<T> gain;
        gain.setGainLinear ((NumericType) 0);
        gain.prepare ({ (double) fs, (uint32_t) blockSize, 1 });
        gain.setRampDurationSeconds (0.1);

        juce::SmoothedValue<NumericType, juce::ValueSmoothingTypes::Linear> refGain;
        refGain.setCurrentAndTargetValue ((NumericType) 0);
        refGain.reset (0.1, (double) fs);

        chowdsp::Buffer<T> buffer (1, blockSize);
        auto* bufferData = buffer.getWritePointer (0);

        std::fill (bufferData, bufferData + blockSize, (T) 1);
        gain.process (buffer);
        for (int i = 0; i < blockSize; ++i)
            REQUIRE_MESSAGE (bufferData[i] == SIMDApprox<T> ((T) refGain.getNextValue()).margin ((NumericType) maxErr), "Unsmoothed gain is incorrect!");

        gain.setGainLinear ((NumericType) 2);
        refGain.setTargetValue ((NumericType) 2);
        REQUIRE_MESSAGE (gain.isSmoothing() == refGain.isSmoothing(), "isSmoothing() is incorrect!");
        std::fill (bufferData, bufferData + blockSize, (T) 1);
        gain.process (buffer);
        for (int i = 0; i < blockSize; ++i)
            REQUIRE_MESSAGE (bufferData[i] == SIMDApprox<T> ((T) refGain.getNextValue()).margin ((NumericType) maxErr), "Smoothed gain is incorrect!");
        REQUIRE_MESSAGE (gain.isSmoothing() == refGain.isSmoothing(), "isSmoothing() is incorrect!");

        gain.reset();
        refGain.reset (0.1, (double) fs);
        std::fill (bufferData, bufferData + blockSize, (T) 1);
        gain.process (buffer);
        for (int i = 0; i < blockSize; ++i)
            REQUIRE_MESSAGE (bufferData[i] == SIMDApprox<T> ((T) refGain.getNextValue()).margin ((NumericType) maxErr), "Gain after reset is incorrect!");
    }

    SECTION ("Reset Test")
    {
        chowdsp::Gain<T> gain;
        gain.setGainLinear ((NumericType) 0);
        gain.prepare ({ (double) fs, (uint32_t) blockSize, 1 });
        gain.setRampDurationSeconds (0.1);

        chowdsp::Buffer<T> buffer (1, blockSize);
        auto* bufferData = buffer.getWritePointer (0);

        std::fill (bufferData, bufferData + blockSize, (T) 1);
        gain.process (buffer);
        REQUIRE_MESSAGE (bufferData[0] == SIMDApprox<T> ((T) 0).margin ((NumericType) 1.0e-6), "Gain after prepare() is incorrect!");

        gain.setGainDecibels ((NumericType) 0);
        gain.reset();
        std::fill (bufferData, bufferData + blockSize, (T) 1);
        gain.process (buffer);
        REQUIRE_MESSAGE (bufferData[0] == SIMDApprox<T> ((T) 1).margin ((NumericType) 1.0e-6), "Gain after reset() is incorrect!");
    }
}
