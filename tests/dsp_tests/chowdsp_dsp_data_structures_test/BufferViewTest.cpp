#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

TEMPLATE_TEST_CASE ("Buffer View Test", "", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using chowdsp::SIMDUtils::all;
    std::random_device rd;
    std::mt19937 mt (rd());
    std::uniform_real_distribution<float> dist (-1.0f, 1.0f);

    SECTION ("Data Test")
    {
        chowdsp::Buffer<TestType> buffer { 2, 128 };
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* x = buffer.getWritePointer (ch);
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                x[n] = TestType (dist (mt));
        }

        {
            chowdsp::BufferView view { buffer };
            REQUIRE_MESSAGE (buffer.getNumChannels() == view.getNumChannels(), "View has the incorrect number of channels!");
            REQUIRE_MESSAGE (buffer.getNumSamples() == view.getNumSamples(), "View has the incorrect number of samples!");

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                const auto* x = buffer.getReadPointer (ch);
                const auto* xView = view.getReadPointer (ch);
                for (int n = 0; n < buffer.getNumSamples(); ++n)
                    REQUIRE_MESSAGE (all (x[n] == xView[n]), "Data is not correct!");
            }
        }

        {
            chowdsp::BufferView view { buffer.getArrayOfWritePointers(), buffer.getNumChannels(), buffer.getNumSamples() };
            REQUIRE_MESSAGE (buffer.getNumChannels() == view.getNumChannels(), "View has the incorrect number of channels!");
            REQUIRE_MESSAGE (buffer.getNumSamples() == view.getNumSamples(), "View has the incorrect number of samples!");

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                const auto* x = buffer.getReadPointer (ch);
                const auto* xView = view.getReadPointer (ch);
                for (int n = 0; n < buffer.getNumSamples(); ++n)
                    REQUIRE_MESSAGE (all (x[n] == xView[n]), "Data is not correct!");
            }
        }
    }

    SECTION ("Data Offset Test")
    {
        static constexpr int offset = 32;

        chowdsp::StaticBuffer<TestType, 2, 1024> buffer { 2, 128 };
        chowdsp::BufferView view { buffer, offset };
        REQUIRE_MESSAGE (buffer.getNumChannels() == view.getNumChannels(), "View has the incorrect number of channels!");
        REQUIRE_MESSAGE (view.getNumSamples() == buffer.getNumSamples() - offset, "View has the incorrect number of samples!");

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* x = view.getWritePointer (ch);
            for (int n = 0; n < view.getNumSamples(); ++n)
                x[n] = TestType (dist (mt));
        }

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            const auto* x = buffer.getReadPointer (ch);
            const auto* xView = view.getReadPointer (ch);
            for (int n = 0; n < offset; ++n)
                REQUIRE_MESSAGE (all (x[n] == TestType (0)), "Skipped data is not correct!");

            for (int n = offset; n < buffer.getNumSamples(); ++n)
                REQUIRE_MESSAGE (all (x[n] == xView[n - offset]), "Data in view not correct!");
        }
    }

    SECTION ("Clear Test")
    {
        chowdsp::Buffer<TestType> buffer { 2, 128 };
        chowdsp::BufferView view { buffer };

        auto* xWrite = view.getArrayOfWritePointers();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int n = 0; n < view.getNumSamples(); ++n)
                xWrite[ch][n] = TestType (dist (mt));
        }

        view.clear();

        auto* xRead = view.getArrayOfWritePointers();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                REQUIRE_MESSAGE (all (xRead[ch][n] == TestType (0)), "Data in view not correct!");
        }
    }
}
