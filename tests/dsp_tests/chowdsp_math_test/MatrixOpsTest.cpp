#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>
#include <array>

TEST_CASE ("Matrix Ops Test", "[dsp][math][simd]")
{
    SECTION ("Householder Scalar Test")
    {
        static constexpr size_t size = 8;

        std::array<float, size> data {};
        std::array<float, size> data2 {};
        std::fill (data.begin(), data.end(), 1.0f);

        chowdsp::MatrixOps::HouseHolder<float, size>::outOfPlace (data2.data(), data.data());
        chowdsp::MatrixOps::HouseHolder<float, size>::inPlace (data.data());

        for (auto& x : data)
            REQUIRE_MESSAGE (juce::approximatelyEqual (x, -1.0f), "Householder ouput is incorrect!");

        for (auto& x : data2)
            REQUIRE_MESSAGE (juce::approximatelyEqual (x, -1.0f), "Householder out-of-place ouput is incorrect!");
    }

    SECTION ("Householder Scalar Odd Test")
    {
        static constexpr size_t size = 7;

        std::array<float, size> data {};
        std::array<float, size> data2 {};
        std::fill (data.begin(), data.end(), 1.0f);

        chowdsp::MatrixOps::HouseHolder<float, size>::outOfPlace (data2.data(), data.data());
        chowdsp::MatrixOps::HouseHolder<float, size>::inPlace (data.data());

        for (auto& x : data)
            REQUIRE_MESSAGE (juce::approximatelyEqual (x, -1.0f), "Householder ouput is incorrect!");

        for (auto& x : data2)
            REQUIRE_MESSAGE (juce::approximatelyEqual (x, -1.0f), "Householder out-of-place ouput is incorrect!");
    }

    SECTION ("Householder Vector Test")
    {
        using VecType = xsimd::batch<float>;
        std::array<VecType, 2> data {};
        std::fill (data.begin(), data.end(), 1.0f);
        chowdsp::MatrixOps::HouseHolder<VecType, 2>::inPlace (data.data());

        for (auto& x : data)
            REQUIRE_MESSAGE (xsimd::all (x == -1.0f), "Householder ouput is incorrect!");
    }

    SECTION ("Hadamard Scalar Test")
    {
        static constexpr size_t size = 8;

        std::array<float, size> data {};
        std::fill (data.begin(), data.end(), 1.0f);

        chowdsp::MatrixOps::Hadamard<float, size>::inPlace (data.data());

        REQUIRE_MESSAGE (juce::approximatelyEqual (data[0], (float) size / (float) sqrt (size)), "Hadamard value 0 is incorrect!");
        for (size_t i = 1; i < size; ++i)
            REQUIRE_MESSAGE (juce::approximatelyEqual (data[i], 0.0f), "Hadamard output is incorrect!");
    }

    SECTION ("Hadamard Scalar Test Small")
    {
        static constexpr size_t size = 4;

        std::array<float, size> data {};
        std::fill (data.begin(), data.end(), 1.0f);

        chowdsp::MatrixOps::Hadamard<float, size>::inPlace (data.data());

        REQUIRE_MESSAGE (juce::approximatelyEqual (data[0], (float) size / (float) sqrt (size)), "Hadamard value 0 is incorrect!");
        for (size_t i = 1; i < size; ++i)
            REQUIRE_MESSAGE (juce::approximatelyEqual (data[i], 0.0f), "Hadamard output is incorrect!");
    }

    SECTION ("Hadamard Scalar Test Large")
    {
        static constexpr size_t size = 16;

        std::array<float, size> data {};
        std::fill (data.begin(), data.end(), 1.0f);

        chowdsp::MatrixOps::Hadamard<float, size>::inPlace (data.data());

        REQUIRE_MESSAGE (juce::approximatelyEqual (data[0], (float) size / (float) sqrt (size)), "Hadamard value 0 is incorrect!");
        for (size_t i = 1; i < size; ++i)
            REQUIRE_MESSAGE (juce::approximatelyEqual (data[i], 0.0f), "Hadamard output is incorrect!");
    }

    SECTION ("Hadamard Vector Test")
    {
        using VecType = xsimd::batch<float>;
        static constexpr size_t size = 2;
        std::array<VecType, size> data {};
        std::fill (data.begin(), data.end(), 1.0f);
        chowdsp::MatrixOps::Hadamard<VecType, size>::inPlace (data.data());

        REQUIRE_MESSAGE (juce::approximatelyEqual (data[0].get (0), float (size * VecType ::size) / (float) sqrt (size * VecType::size)), "Hadamard value 0 is incorrect!");
        REQUIRE_MESSAGE (juce::approximatelyEqual (data[0].get (1), 0.0f), "Hadamard output is incorrect!");
        REQUIRE_MESSAGE (juce::approximatelyEqual (data[0].get (2), 0.0f), "Hadamard output is incorrect!");
        REQUIRE_MESSAGE (juce::approximatelyEqual (data[0].get (3), 0.0f), "Hadamard output is incorrect!");
        for (size_t i = 1; i < size; ++i)
            for (size_t j = 0; j < VecType::size; ++j)
                REQUIRE_MESSAGE (juce::approximatelyEqual (data[i].get (j), 0.0f), "Hadamard output is incorrect!");
    }
}
