#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

TEST_CASE ("UI-to-Audio Pipeline Test", "[dsp][data-structures]")
{
    using TestData = std::array<bool, 10>;
    chowdsp::UIToAudioPipeline<TestData> pipeline;

    auto testPipeline = [&] (int numWrites)
    {
        std::atomic_bool finished { false };
        TestData mostRecentWriteData {};
        std::thread audioThread (
            [&]
            {
                TestData mostRecentReadData {};
                while (! finished.load())
                {
                    const auto* object = pipeline.read();
                    if (object != nullptr)
                        mostRecentReadData = *object;

                    std::this_thread::sleep_for (std::chrono::milliseconds (3));
                }
                const auto* object = pipeline.read();
                REQUIRE (object != nullptr);
                mostRecentReadData = *object;
                REQUIRE (mostRecentReadData == mostRecentWriteData);
            });

        auto randomiser = [dist = std::uniform_int_distribution<> (0, 1),
                           mt = std::mt19937 { Catch::Generators::Detail::getSeed() }]() mutable -> bool
        { return dist (mt); };

        // UI thread
        for (int i = 0; i < numWrites; ++i)
        {
            auto uiObject = std::make_unique<TestData>();
            for (auto& el : *uiObject)
                el = randomiser();
            mostRecentWriteData = *uiObject;
            pipeline.write (std::move (uiObject));
            std::this_thread::sleep_for (std::chrono::milliseconds (16)); // roughly 60 FPS
        }

        finished.store (true);
        audioThread.join();
    };

    SECTION ("Single write")
    {
        testPipeline (1);
    }

    SECTION ("Ten writes")
    {
        testPipeline (10);
    }

    SECTION ("Five + Five writes")
    {
        testPipeline (5);
        testPipeline (5);
    }
}
