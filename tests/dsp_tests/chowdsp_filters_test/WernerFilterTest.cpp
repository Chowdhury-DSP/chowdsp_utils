#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>

TEST_CASE ("Werner Filter Test", "[dsp][filters]")
{
    const auto spec = juce::dsp::ProcessSpec { 48000.0f, 256, 1 };

    chowdsp::WernerFilter filterRef;
    chowdsp::WernerFilter filterMM;
    using FilterType = chowdsp::WernerFilterType;

    filterRef.prepare (spec);
    filterMM.prepare (spec);

    auto minus1To1 = test_utils::RandomFloatGenerator { -1.0f, 1.0f };

    SECTION ("LPF/Multi-Mode Test")
    {
        for (int i = 0; i < 20; ++i)
        {
            const auto x = minus1To1();
            const auto actual = filterMM.processSample<FilterType::MultiMode> (0, x, 0.0f);
            const auto exp = filterMM.processSample<FilterType::Lowpass2> (0, x);
            REQUIRE (actual == Catch::Approx (exp).margin (1.0e-6f));
        }
    }

    SECTION ("BPF/Multi-Mode Test")
    {
        filterRef.reset();
        filterMM.reset();

        std::vector<float> actual (spec.maximumBlockSize);
        std::vector<float> exp (spec.maximumBlockSize);

        for (size_t i = 0; i < spec.maximumBlockSize; ++i)
        {
            const auto x = minus1To1();
            actual[i] = x;
            exp[i] = x;
        }

        auto* actualData = actual.data();
        filterRef.processBlock<FilterType::Bandpass2> ({ &actualData, 1, (int) spec.maximumBlockSize });

        auto* expData = exp.data();
        filterMM.processBlock<FilterType::MultiMode> ({ &expData, 1, (int) spec.maximumBlockSize }, 0.5f);

        for (size_t i = 0; i < spec.maximumBlockSize; ++i)
            REQUIRE (actual[i] == Catch::Approx (exp[i]).margin (1.0e-6f));
    }

    SECTION ("HPF/Multi-Mode Test")
    {
        filterRef.reset();
        filterMM.reset();

        std::vector<float> actual (spec.maximumBlockSize);
        std::vector<float> exp (spec.maximumBlockSize);
        std::vector<float> mode (spec.maximumBlockSize);

        for (size_t i = 0; i < spec.maximumBlockSize; ++i)
        {
            const auto x = minus1To1();
            actual[i] = x;
            exp[i] = x;
            mode[i] = 1.0f;
        }

        auto* actualData = actual.data();
        filterRef.processBlock<FilterType::Highpass2> ({ &actualData, 1, (int) spec.maximumBlockSize });

        auto* expData = exp.data();
        filterMM.processBlock<FilterType::MultiMode> ({ &expData, 1, (int) spec.maximumBlockSize }, mode.data());

        for (size_t i = 0; i < spec.maximumBlockSize; ++i)
            REQUIRE (actual[i] == Catch::Approx (exp[i]).margin (1.0e-6f));
    }
}
