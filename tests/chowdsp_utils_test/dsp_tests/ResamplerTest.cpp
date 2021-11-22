#include <JuceHeader.h>

namespace
{
constexpr float origSampleRate = 48000.0;
constexpr int origNumSamples = 4800;
} // namespace

class ResamplerTest : public UnitTest
{
public:
    ResamplerTest() : UnitTest ("Resampler Test") {}

    void gen_sine (std::vector<float>& audio, float freq, float fs, int num_samples)
    {
        audio.resize ((size_t) num_samples, 0.0f);
        std::generate (audio.begin(), audio.end(), [=, n = 0.0f]() mutable {
            return std::sin (MathConstants<float>::twoPi * (float) n++ * freq / fs);
        });
    }

    int calc_latency (const std::vector<float>& data, const std::vector<float>& ref_data)
    {
        auto find_first_point5 = [] (const std::vector<float>& x) -> int {
            for (size_t i = 0; i < x.size(); ++i)
            {
                if (x[i] >= 0.5f)
                    return (int) i;
            }
            return -1;
        };

        auto ref_one = find_first_point5 (ref_data);
        auto actual_one = find_first_point5 (data);

        if (actual_one < 0)
            std::cout << "WARNING: data never reached above 0.5!" << std::endl;

        return actual_one - ref_one;
    }

    std::pair<double, double> calc_error (float freq, float fs, const std::vector<float>& data, const int num_samples)
    {
        std::vector<float> compare_data;
        gen_sine (compare_data, freq, fs, num_samples);
        auto latency_samp = std::max (calc_latency (data, compare_data), 0);
        std::vector<float> error_vec ((size_t) num_samples, 0.0f);

        float error_sum = 0.0f;
        for (size_t i = 0; i < size_t (num_samples - latency_samp); ++i)
        {
            auto expected = compare_data[i];
            auto actual = data[i + (size_t) latency_samp];
            error_vec[i] = std::abs (actual - expected);
            error_sum += error_vec[i];
        }

        auto max_error = *std::max_element (error_vec.begin(), error_vec.end());
        auto avg_error = error_sum / (float) num_samples;

        return std::make_pair (avg_error, max_error);
    }

    template <typename RType>
    void testResampler (float freq, double avgErrLimit, double maxErrLimit)
    {
        std::vector<float> inData;
        gen_sine (inData, freq, origSampleRate, origNumSamples);
        auto testSampleRate = [=] (float outSampleRate) {
            const auto ratio = outSampleRate / origSampleRate;

            RType resampler;
            resampler.prepare (origSampleRate, ratio);

            constexpr size_t block_size = 256;
            std::vector<float> out (size_t (origNumSamples * ratio) + 1, 0.0f);

            size_t out_ptr = 0;
            for (size_t i = 0; i + block_size < inData.size(); i += block_size)
                out_ptr += resampler.process (&inData[i], &out[out_ptr], block_size);

            auto [avgError, maxError] = calc_error (freq, outSampleRate, out, (int) out_ptr);
            expectLessThan (avgError, avgErrLimit, "Avg. Error is too large! Sample rate: " + String (outSampleRate));
            expectLessThan (maxError, maxErrLimit, "Max Error is too large! Sample rate: " + String (outSampleRate));
        };

        testSampleRate (origSampleRate);
        testSampleRate (origSampleRate * 2.0f);
        testSampleRate (origSampleRate * 1.5f);
        testSampleRate (origSampleRate * 0.5f);
        testSampleRate (origSampleRate * 0.667f);
    }

    void runTest() override
    {
        beginTest ("Lanczos Standard");
        testResampler<chowdsp::ResamplingTypes::LanczosResampler<>> (100.0f, 0.01, 0.05);

        beginTest ("Lanczos Short");
        testResampler<chowdsp::ResamplingTypes::LanczosResampler<2048>> (100.0f, 0.01, 0.05);

        beginTest ("Lanczos Long");
        testResampler<chowdsp::ResamplingTypes::LanczosResampler<8192, 8>> (100.0f, 0.01, 0.05);

        beginTest ("Lanczos Wide");
        testResampler<chowdsp::ResamplingTypes::LanczosResampler<4096, 32>> (100.0f, 0.01, 0.05);

#if CHOWDSP_USE_LIBSAMPLERATE
        beginTest ("SRC Best");
        testResampler<chowdsp::ResamplingTypes::SRCResampler<SRC_SINC_BEST_QUALITY>> (100.0f, 0.1, 0.05);

        beginTest ("SRC Medium");
        testResampler<chowdsp::ResamplingTypes::SRCResampler<SRC_SINC_MEDIUM_QUALITY>> (100.0f, 0.02, 0.05);

        beginTest ("SRC Fastest");
        testResampler<chowdsp::ResamplingTypes::SRCResampler<SRC_SINC_FASTEST>> (100.0f, 1.0e-5, 0.01);
#endif
    }
};

static ResamplerTest resamplerTest;
