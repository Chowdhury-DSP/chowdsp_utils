#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

TEMPLATE_TEST_CASE ("Buffer Test", "", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using chowdsp::SIMDUtils::all;
    std::random_device rd;
    std::mt19937 mt (rd());
    std::uniform_real_distribution<float> dist (-1.0f, 1.0f);

    SECTION ("Default Construction Test")
    {
        chowdsp::Buffer<TestType> buffer;
        REQUIRE_MESSAGE (buffer.getNumChannels() == 0, "Default constructed # channels is incorrect");
        REQUIRE_MESSAGE (buffer.getNumSamples() == 0, "Default constructed # samples is incorrect");
    }

    SECTION ("Construct with size Test")
    {
        chowdsp::Buffer<TestType> buffer { 2, 128 };
        REQUIRE_MESSAGE (buffer.getNumChannels() == 2, "Constructed buffer with 2 channels is incorrect");
        REQUIRE_MESSAGE (buffer.getNumSamples() == 128, "Constructed buffer with 128 samples is incorrect");
    }

    SECTION ("Resize Test")
    {
        chowdsp::Buffer<TestType> buffer;

        buffer.setMaxSize (2, 128);
        REQUIRE_MESSAGE (buffer.getNumChannels() == 2, "Resized # channels is incorrect");
        REQUIRE_MESSAGE (buffer.getNumSamples() == 128, "Resized # samples is incorrect");
    }

    SECTION ("Move Construct Test")
    {
        chowdsp::Buffer<TestType> buffer { 2, 128 };
        buffer.getWritePointer (0)[100] = TestType (1);
        buffer.getWritePointer (1)[110] = TestType (-1);

        auto&& movedBuffer = std::move (buffer);
        REQUIRE_MESSAGE (movedBuffer.getNumChannels() == 2, "Moved buffer with 2 channels is incorrect");
        REQUIRE_MESSAGE (movedBuffer.getNumSamples() == 128, "Moved buffer with 128 samples is incorrect");
        REQUIRE_MESSAGE (all (buffer.getReadPointer (0)[0] == TestType (0)), "Cleared sample in channel 0 is incorrect");
        REQUIRE_MESSAGE (all (buffer.getReadPointer (0)[100] == TestType (1)), "Set sample in channel 0 is incorrect");
        REQUIRE_MESSAGE (all (buffer.getReadPointer (1)[0] == TestType (0)), "Cleared sample in channel 1 is incorrect");
        REQUIRE_MESSAGE (all (buffer.getReadPointer (1)[110] == TestType (-1)), "Set sample in channel 1 is incorrect");
    }

    SECTION ("Clear Test")
    {
        chowdsp::Buffer<TestType> buffer { 2, 128 };
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* x = buffer.getWritePointer (ch);
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                x[n] = TestType (dist (mt));
        }

        for (int i = 0; i < 2; ++i)
        {
            buffer.clear();
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                const auto* x = buffer.getReadPointer (ch);
                for (int n = 0; n < buffer.getNumSamples(); ++n)
                    REQUIRE_MESSAGE (all (x[n] == TestType (0)), "Buffer was not cleared!");
            }
        }
    }

    SECTION ("Resize Test")
    {
        chowdsp::Buffer<TestType> buffer { 2, 128 };

        auto** x = buffer.getArrayOfWritePointers();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                x[ch][n] = TestType (dist (mt));
        }

        buffer.setCurrentSize (1, 32);
        buffer.clear();

        buffer.setCurrentSize (2, 64);
        auto** xConst = buffer.getArrayOfReadPointers();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                REQUIRE_MESSAGE (all (xConst[ch][n] == TestType (0)), "Buffer was not cleared! " << ch << ", " << n);
        }
    }
}
