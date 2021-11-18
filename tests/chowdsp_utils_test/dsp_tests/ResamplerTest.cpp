#include <JuceHeader.h>

namespace
{
constexpr float origSampleRate = 48000.0;
constexpr int origNumSamples = 4800;
constexpr int origBlockSize = 480;
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

    std::pair<int, int> calc_latency (const std::vector<float>& data, const std::vector<float>& ref_data)
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

        return std::make_pair (std::max (actual_one - ref_one, 0), std::max (ref_one - actual_one, 0));
    }

    std::pair<double, double> calc_error (float freq, float fs, const std::vector<float>& data, const int num_samples)
    {
        std::vector<float> compare_data;
        gen_sine (compare_data, freq, fs, num_samples);
        auto [data_off, comp_off] = calc_latency (data, compare_data);
        std::vector<float> error_vec ((size_t) num_samples, 0.0f);

        float error_sum = 0.0f;
        for (size_t i = 0; i < size_t (num_samples - std::max (data_off, comp_off)); ++i)
        {
            auto expected = compare_data[i + (size_t) comp_off];
            auto actual = data[i + (size_t) data_off];
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

        for (auto factor : { 1.0f, 2.0f, 1.5f, 0.5f, 0.667f })
            testSampleRate (origSampleRate * factor);
    }

    template <typename RType>
    void resamplingFactorTest()
    {
        chowdsp::ResampledProcess<RType> resampledProcess;
        resampledProcess.prepare ({ (double) origSampleRate, (uint32) origBlockSize, 1 });

        for (auto factor : { 1.0f, 2.0f, 1.5f, 0.5f, 0.667f })
        {
            resampledProcess.setResampleRatio (factor);

            auto expFs = origSampleRate * factor;
            auto actualFs = resampledProcess.getTargetSampleRate();
            expectWithinAbsoluteError (actualFs, expFs, 1.0e-6f, "Resampler target sample rate incorrect!");
        }
    }

    template <typename RType>
    void resamplingSampleRateTest()
    {
        chowdsp::ResampledProcess<RType> resampledProcess;
        resampledProcess.prepare ({ (double) origSampleRate, (uint32) origBlockSize, 1 });

        for (auto factor : { 1.0f, 2.0f, 1.5f, 0.5f, 0.667f })
        {
            resampledProcess.setTargetSampleRate (origSampleRate * factor);

            auto actualRatio = resampledProcess.getResampleRatio();
            expectWithinAbsoluteError (actualRatio, factor, 1.0e-6f, "Resampling ratio incorrect!");
        }
    }

    template <typename RType>
    void resampledProcessTest (float freq, double avgErrLimit, double maxErrLimit)
    {
        std::vector<float> inData;
        gen_sine (inData, freq, origSampleRate, origNumSamples);
        auto testSampleRate = [=] (float targetSampleRate) {
            const auto ratio = targetSampleRate / origSampleRate;
            const auto expBlockSize = int ((float) origBlockSize * ratio);

            chowdsp::ResampledProcess<RType> resampler;
            resampler.prepare ({ (double) origSampleRate, (uint32) origBlockSize, 1 });
            resampler.setTargetSampleRate (targetSampleRate);

            AudioBuffer<float> buffer (1, origNumSamples);
            buffer.copyFrom (0, 0, inData.data(), origNumSamples);
            dsp::AudioBlock<float> rBlock (buffer);

            std::vector<float> out (size_t (origNumSamples * ratio) + 1, 0.0f);

            size_t out_ptr = 0;
            for (int i = 0; i + origBlockSize < origNumSamples; i += origBlockSize)
            {
                auto block = rBlock.getSubBlock ((size_t) i, (size_t) origBlockSize);

                auto resampledBlock = resampler.processIn (block);
                auto rSize = resampledBlock.getNumSamples();
                if (out_ptr > 2 * (size_t) expBlockSize)
                    expectWithinAbsoluteError ((int) rSize, expBlockSize, 2, "Resampled block size is incorrect");

                auto rPtr = resampledBlock.getChannelPointer (0);
                std::copy (rPtr, rPtr + rSize, out.begin() + (int) out_ptr);
                out_ptr += rSize;
            }

            auto [avgError, maxError] = calc_error (freq, targetSampleRate, out, (int) out_ptr);
            expectLessThan (avgError, avgErrLimit, "Avg. Error is too large! Sample rate: " + String (targetSampleRate));
            expectLessThan (maxError, maxErrLimit, "Max Error is too large! Sample rate: " + String (targetSampleRate));
        };

        for (auto factor : { 1.0f, 2.0f, 1.5f, 0.5f, 0.667f })
            testSampleRate (origSampleRate * factor);
    }

    void runTest() override
    {
        using namespace chowdsp::ResamplingTypes;

        beginTest ("Lanczos Standard");
        testResampler<LanczosResampler<>> (100.0f, 0.01, 0.05);

        beginTest ("Lanczos Short");
        testResampler<LanczosResampler<2048>> (100.0f, 0.01, 0.05);

        beginTest ("Lanczos Long");
        testResampler<LanczosResampler<8192, 8>> (100.0f, 0.01, 0.05);

        beginTest ("Lanczos Wide");
        testResampler<LanczosResampler<4096, 32>> (100.0f, 0.01, 0.05);

        beginTest ("Resampling Ratio Test (Lanczos)");
        resamplingFactorTest<LanczosResampler<>>();

        beginTest ("Resampling Sample Rate Test (Lanczos)");
        resamplingSampleRateTest<LanczosResampler<>>();

        beginTest ("Resampled Process Test (Lanczos)");
        resampledProcessTest<LanczosResampler<>> (100.0f, 0.01, 0.05);

#if CHOWDSP_USE_LIBSAMPLERATE
        beginTest ("SRC Best");
        testResampler<SRCResampler<SRC_SINC_BEST_QUALITY>> (100.0f, 0.02, 0.05);

        beginTest ("SRC Medium");
        testResampler<SRCResampler<SRC_SINC_MEDIUM_QUALITY>> (100.0f, 0.02, 0.05);

        beginTest ("SRC Fastest");
        testResampler<SRCResampler<SRC_SINC_FASTEST>> (100.0f, 1.0e-5, 0.01);

        beginTest ("Resampling Ratio Test (SRC)");
        resamplingFactorTest<SRCResampler<>>();

        beginTest ("Resampling Sample Rate Test (SRC)");
        resamplingSampleRateTest<SRCResampler<>>();

        beginTest ("Resampled Process Test (SRC)");
        resampledProcessTest<SRCResampler<SRC_SINC_FASTEST>> (100.0f, 1.0e-5, 0.05);
#endif
    }
};

static ResamplerTest resamplerTest;
