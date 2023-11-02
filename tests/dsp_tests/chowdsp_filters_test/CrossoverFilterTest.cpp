#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>

template <typename T>
auto basic_fft (nonstd::span<const T> input)
{
    std::vector<std::complex<T>> out;
    out.resize (input.size(), {});
    for (size_t k = 0; k < out.size(); ++k)
    {
        out[k] = (T) 0;
        for (size_t n = 0; n < out.size(); ++n)
        {
            using namespace std::complex_literals;
            const auto e_k = -juce::MathConstants<T>::twoPi * std::complex { (T) 0.0, (T) 1.0 } * (T) k * (T) n / (T) out.size();
            out[k] += input[n] * std::exp (e_k);
        }
    }
    return out;
}

template <typename T>
auto basic_fft_mag (nonstd::span<const T> input)
{
    const auto out_complex = basic_fft (input);
    std::vector<T> out {};
    out.resize (input.size(), {});
    for (auto [x_c, x_mag] : chowdsp::zip (out_complex, out))
        x_mag = std::abs (x_c);
    return out;
}

template <int Order, typename T = float>
void testCrossover (float crossFreq0, float crossFreq1, float crossFreq2)
{
    static constexpr size_t N = 1024;
    chowdsp::Buffer<T> buffer { 1, (int) N };
    buffer.clear();
    buffer.getWritePointer (0)[0] = 1.0f;

    chowdsp::CrossoverFilter<T, Order, 4> crossover;
    chowdsp::Buffer<T> low_buffer { 1, (int) N };
    chowdsp::Buffer<T> low_mid_buffer { 1, (int) N };
    chowdsp::Buffer<T> mid_high_buffer { 1, (int) N };
    chowdsp::Buffer<T> high_buffer { 1, (int) N };
    crossover.prepare ({ 48000.0, (uint32_t) N, 1 });
    crossover.setCrossoverFrequency (0, (T) crossFreq0);
    crossover.setCrossoverFrequency (1, (T) crossFreq1);
    crossover.setCrossoverFrequency (2, (T) crossFreq2);

    crossover.processBlock (buffer, { low_buffer, low_mid_buffer, mid_high_buffer, high_buffer });

    chowdsp::Buffer<T> test_buffer { 1, (int) N };
    test_buffer.clear();
    chowdsp::BufferMath::addBufferData (low_buffer, test_buffer);
    chowdsp::BufferMath::addBufferData (low_mid_buffer, test_buffer);
    chowdsp::BufferMath::addBufferData (mid_high_buffer, test_buffer);
    chowdsp::BufferMath::addBufferData (high_buffer, test_buffer);

    const auto out_spectrum = basic_fft_mag (test_buffer.getReadSpan (0));
    for (auto& y_mag : out_spectrum)
        REQUIRE (y_mag == Catch::Approx { (T) 1 }.margin ((T) 0.01));
}

TEST_CASE ("Crossover Filter Test", "[dsp][filters]")
{
    SECTION ("2nd Order")
    {
        testCrossover<2> (200.0f, 1000.0f, 8000.0f);
        testCrossover<2> (200.0f, 8000.0f, 1000.0f);
        testCrossover<2> (1000.0f, 200.0f, 8000.0f);
        testCrossover<2> (1000.0f, 8000.0f, 200.0f);
        testCrossover<2> (8000.0f, 1000.0f, 200.0f);
        testCrossover<2> (8000.0f, 200.0f, 1000.0f);
    }
    SECTION ("4th Order")
    {
        testCrossover<4> (200.0f, 1000.0f, 8000.0f);
        testCrossover<4> (200.0f, 8000.0f, 1000.0f);
        testCrossover<4> (1000.0f, 200.0f, 8000.0f);
        testCrossover<4> (1000.0f, 8000.0f, 200.0f);
        testCrossover<4> (8000.0f, 1000.0f, 200.0f);
        testCrossover<4> (8000.0f, 200.0f, 1000.0f);
    }
    SECTION ("8th Order")
    {
        testCrossover<8> (200.0f, 1000.0f, 8000.0f);
        testCrossover<8> (200.0f, 8000.0f, 1000.0f);
        testCrossover<8> (1000.0f, 200.0f, 8000.0f);
        testCrossover<8> (1000.0f, 8000.0f, 200.0f);
        testCrossover<8> (8000.0f, 1000.0f, 200.0f);
        testCrossover<8> (8000.0f, 200.0f, 1000.0f);
    }
}
