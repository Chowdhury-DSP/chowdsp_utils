#include <TimedUnitTest.h>
#include <chowdsp_core/chowdsp_core.h>

class DoubleBufferTest : public TimedUnitTest
{
public:
    DoubleBufferTest() : TimedUnitTest ("Double Buffer Test")
    {
    }

    void sizeTest()
    {
        {
            chowdsp::DoubleBuffer<float> buffer;
            buffer.resize (12);
            expectEquals (buffer.size(), 12, "Buffer size set from resize() is incorrect!");
        }

        {
            chowdsp::DoubleBuffer<float> buffer (24);
            expectEquals (buffer.size(), 24, "Buffer size set from constructor is incorrect!");
        }

        {
            chowdsp::DoubleBuffer<float> buffer (24);
            expectEquals (buffer.size(), 24, "Buffer size set from constructor is incorrect!");

            buffer.resize (12);
            expectEquals (buffer.size(), 12, "Buffer size set from resize() after constructor is incorrect!");
        }
    }

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
            expectEquals (actual[i], expected[(size_t) i], "Incorrect value at index " + juce::String (i));
    }

    template <typename T>
    void wrapAndClearTest (juce::Random& r)
    {
        auto getRandomVal = [&]() {
            if constexpr (std::is_same<T, float>::value)
                return r.nextFloat() * 2.0f - 1.0f;
            else if (std::is_same<T, int>::value)
                return r.nextInt ({ -100, 100 });
        };

        chowdsp::DoubleBuffer<T> buffer (24);

        std::vector<T> data (12);
        std::vector<T> lastData (data.size());
        std::vector<T> secondToLastData (data.size());
        for (int i = 0; i < 8; ++i)
        {
            for (auto& val : data)
                val = getRandomVal();

            buffer.push (data.data(), (int) data.size());

            std::copy (lastData.begin(), lastData.end(), secondToLastData.begin());
            std::copy (data.begin(), data.end(), lastData.begin());
        }

        const auto* testData = buffer.data ((int) data.size());
        for (int i = 0; i < buffer.size() / 2; ++i)
        {
            expectEquals (testData[i], lastData[(size_t) i], "Incorrect value at index " + juce::String (i));

            auto secondIdx = i + buffer.size() / 2;
            expectEquals (testData[secondIdx], secondToLastData[(size_t) i], "Incorrect value at index " + juce::String (secondIdx));
        }

        buffer.clear();
        for (int i = 0; i < buffer.size(); ++i)
            expectEquals (testData[i], (T) 0, "Erroneous non-zero value at index " + juce::String (i));
    }

    void runTestTimed() override
    {
        beginTest ("Size Test");
        sizeTest();

        beginTest ("Wrap Test");
        wrapTest<int>();
        wrapTest<float>();

        auto rand = getRandom();

        beginTest ("Wrap and Clear Test");
        wrapAndClearTest<int> (rand);
        wrapAndClearTest<float> (rand);
    }
};

class DoubleBufferTest doubleBufferTest;
