#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

struct Envelope_Params
{
    double attack {};
    double decay {};
    double sustain {};
    double release {};
};
static_assert (! std::atomic<Envelope_Params>::is_always_lock_free, "We can' use std::atomic for this, since it may lock!");

struct Filter_Params
{
    Filter_Params() = default;
    explicit Filter_Params (float c)
    {
        coeffs.fill (c);
    }

    std::array<float, 100> coeffs {};
};
static_assert (! std::atomic<Filter_Params>::is_always_lock_free, "We can' use std::atomic for this, since it may lock!");

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")

TEST_CASE ("Realtime Latest Object Test", "[dsp][data-structures]")
{
    SECTION ("Aggregate Type")
    {
        chowdsp::RealtimeLatestObject<Envelope_Params> envelope_params {};
        envelope_params.init (1.0, 1.0, 1.0, 1.0);

        // audio thread
        std::thread audio_thread {
            [&envelope_params]
            {
                for (int n = 0; n < 100; ++n)
                {
                    [[maybe_unused]] const auto& latest_envelope_params = envelope_params.read();
                    std::this_thread::sleep_for (std::chrono::milliseconds { 10 });
                }
            }
        };

        // ui thread
        Envelope_Params latest_envelope {};
        std::mt19937 rng { 0x1234 };
        std::uniform_real_distribution dist { 0.0, 1.0 };
        for (int n = 0; n < 10; ++n)
        {
            const auto attack = dist (rng);
            const auto decay = dist (rng);
            const auto sustain = dist (rng);
            const auto release = dist (rng);
            latest_envelope = { attack, decay, sustain, release };
            envelope_params.write (attack, decay, sustain, release);
            std::this_thread::sleep_for (std::chrono::milliseconds { 100 });
        }

        {
            const auto& actual_latest = envelope_params.read();
            REQUIRE (actual_latest.attack == latest_envelope.attack);
            REQUIRE (actual_latest.decay == latest_envelope.decay);
            REQUIRE (actual_latest.sustain == latest_envelope.sustain);
            REQUIRE (actual_latest.release == latest_envelope.release);
        }

        audio_thread.join();
        envelope_params.deinit();
    }

    SECTION ("Non-Aggregate Type")
    {
        chowdsp::RealtimeLatestObject<Filter_Params> filter_params {};
        filter_params.init (Filter_Params { 1.0f });

        // audio thread
        std::thread audio_thread {
            [&filter_params]
            {
                for (int n = 0; n < 100; ++n)
                {
                    [[maybe_unused]] const auto& latest_envelope_params = filter_params.read();
                    std::this_thread::sleep_for (std::chrono::milliseconds { 10 });
                }
            }
        };

        // ui thread
        Filter_Params latest_filter { 0.0f };
        std::mt19937 rng { 0x1234 };
        std::uniform_real_distribution<float> dist { 10.0f, 12.0 };
        for (int n = 0; n < 10; ++n)
        {
            const auto c = dist (rng);
            latest_filter = Filter_Params { c };
            filter_params.write (Filter_Params { c });
            std::this_thread::sleep_for (std::chrono::milliseconds { 100 });
        }

        {
            const auto& actual_latest = filter_params.read();
            REQUIRE (actual_latest.coeffs.front() == latest_filter.coeffs.front());
        }

        audio_thread.join();
        filter_params.deinit();
    }
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
