#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

template <typename T>
void wrapTest()
{
    chowdsp::DoubleBuffer<T> buffer (8);
    std::vector<T> data1 { 0, 1, 2, 3, 4, 5 };
    std::vector<T> data2 { 0, -1, -2, -3, -4, -5 };

    buffer.push (data1.data(), 6);
    buffer.push (data2.data(), 6);

    std::vector<T> expected { -2, -3, -4, -5, 4, 5, 0, -1 };
    const auto* actual = buffer.data();
    for (int i = 0; i < buffer.size(); ++i)
        REQUIRE_MESSAGE (juce::exactlyEqual (actual[i], expected[(size_t) i]), "Incorrect value at index " << std::to_string (i));
}

template <typename T>
void wrapAndClearTest()
{
    std::mt19937 mt (Catch::Generators::Detail::getSeed());

    auto randomDist = []()
    {
        if constexpr (std::is_same<T, float>::value)
            return std::uniform_real_distribution<float> (-1.0f, 1.0f);
        else if (std::is_same<T, int>::value)
            return std::uniform_int_distribution<int> (-100, 100);
    }();

    chowdsp::DoubleBuffer<T> buffer (24);

    std::vector<T> data (12);
    std::vector<T> lastData (data.size());
    std::vector<T> secondToLastData (data.size());
    for (int i = 0; i < 8; ++i)
    {
        for (auto& val : data)
            val = randomDist (mt);

        buffer.push (data.data(), (int) data.size());

        std::copy (lastData.begin(), lastData.end(), secondToLastData.begin());
        std::copy (data.begin(), data.end(), lastData.begin());
    }

    const auto* testData = buffer.data ((int) data.size());
    for (int i = 0; i < buffer.size() / 2; ++i)
    {
        REQUIRE_MESSAGE (juce::exactlyEqual (testData[i], lastData[(size_t) i]), "Incorrect value at index " << std::to_string (i));

        auto secondIdx = i + buffer.size() / 2;
        REQUIRE_MESSAGE (juce::exactlyEqual (testData[secondIdx], secondToLastData[(size_t) i]), "Incorrect value at index " << std::to_string (secondIdx));
    }

    buffer.clear();
    for (int i = 0; i < buffer.size(); ++i)
        REQUIRE_MESSAGE (juce::exactlyEqual (testData[i], (T) 0), "Erroneous non-zero value at index " << std::to_string (i));
}

TEST_CASE ("Double Buffer Test", "[common][data-structures]")
{
    SECTION ("Size Test")
    {
        {
            chowdsp::DoubleBuffer<float> buffer;
            buffer.resize (12);
            REQUIRE_MESSAGE (buffer.size() == 12, "Buffer size set from resize() is incorrect!");
        }

        {
            chowdsp::DoubleBuffer<float> buffer (24);
            REQUIRE_MESSAGE (buffer.size() == 24, "Buffer size set from constructor is incorrect!");
        }

        {
            chowdsp::DoubleBuffer<float> buffer (24);
            REQUIRE_MESSAGE (buffer.size() == 24, "Buffer size set from constructor is incorrect!");

            buffer.resize (12);
            REQUIRE_MESSAGE (buffer.size() == 12, "Buffer size set from resize() after construction is incorrect!");
        }
    }

    SECTION ("Wrap Test")
    {
        wrapTest<int>();
        wrapTest<float>();
    }

    SECTION ("Wrap And Clear Test")
    {
        wrapAndClearTest<int>();
        wrapAndClearTest<float>();
    }
}
