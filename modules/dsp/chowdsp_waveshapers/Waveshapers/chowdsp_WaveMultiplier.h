#pragma once

namespace chowdsp
{
/**
 * Wavefolding effect based on the Serge wave multiplier,
 * with a variable number of stages.
 *
 * Note: the folding effect starts around |x| = 0.38 volts,
 * and "clips" the signal within a range of +/- 0.23 volts. So
 * until the input signal grows larger than (0.38 * nStages), the
 * output will never exceed the clipping threshold. This means that
 * the output gain might be lower than expected, especially when
 * using a larger number of stages, so pay attention to the input
 * gain level, and consider adding a makeup gain to the output if needed.
 *
 * Note: The group delay of the entire processor will be 1 sample times
 * the number of stages.
 *
 * Note: this processor should probably be used with at least 2x oversampling.
 *
 * Reference: https://electro-music.com/forum/phpbb-files/schem_cgs113_vcm2_194.gif
 */
template <typename T, size_t nStages = 6, ADAAWaveshaperMode mode = ADAAWaveshaperMode::MinusX>
class WaveMultiplier
{
public:
    explicit WaveMultiplier (LookupTableCache* lutCache = nullptr, T range = (T) 10, int N = 1 << 17)
    {
        for (auto& stage : stages)
            stage.initialise (lutCache, range, N);
    }

    /** Prepares the waveshaper for a given number of channels. */
    void prepare (int numChannels)
    {
        for (auto& stage : stages)
            stage.prepare (numChannels);
    }

    /** Resets the waveshaper state. */
    void reset()
    {
        for (auto& stage : stages)
            stage.reset();
    }

    /** Process a single sample */
    inline T processSample (T x, int channel = 0) noexcept
    {
        for (auto& stage : stages)
            x = stage.processSample (x, channel);

        return x;
    }

    /** Process a block of samples. */
    void processBlock (const BufferView<T>& buffer) noexcept
    {
        for (auto& stage : stages)
            stage.processBlock (buffer);
    }

    /** Process a block of samples (bypassed). */
    void processBlockBypassed (const BufferView<T>& buffer) noexcept
    {
        for (auto& stage : stages)
            stage.processBlockBypassed (buffer);
    }

private:
    struct FolderCell : public ADAAWaveshaper<T, mode>
    {
        FolderCell() = default;

        void initialise (LookupTableCache* lutCache, T range, int N)
        {
            ADAAWaveshaper<T>::setupLookupTableCache (lutCache, "chowdsp_wave_multiplier_folder_cell");

            // So here's the rough derivation: In the original Serge circuit, the output of one folder cell
            // can be computed as:
            // V_plus = V_in - (33 kOhms) * 2 * Is * sinh(V_plus / vt / nabla)
            // V_out = 2 * V_plus - V_in
            //
            // Where Is, vt, and nabla are the diode constants for a 1N4148 diode.
            // The first equation cannot be solved in closed-form (I think?).
            // So what we did is use a Newton-Raphson solver to figure out the
            // shape of V_plus, and then approximate it. What we got was:
            // V_plus_approx = (1 / 2.45) * tanh(V_in * 2.45) + x * 0.02
            //
            // Combining this with the second equation from above, we get:
            // V_out = (2 / 2.45) * tanh(V_in * 2.45) - x * (1 - 2 * 0.02)

            static constexpr auto D = 2.45;
            static constexpr auto B = 1.0 - 2 * 0.02;
            juce::ignoreUnused (D, B);

            using namespace TanhIntegrals;
            using Power::ipow;

            ADAAWaveshaper<T, mode>::initialise (
                [&] (auto x)
                {
                    return (2.0 / D) * std::tanh (x * D) - B * x;
                },
                [&] (auto x)
                {
                    return (2.0 / ipow<2> (D)) * tanhAD1 (x * D) - B * ipow<2> (x) / 2.0;
                },
                [&] (auto x)
                {
                    return (2.0 / ipow<3> (D)) * tanhAD2 (x * D) - B * ipow<3> (x) / 6.0;
                },
                -range,
                range,
                N);
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FolderCell)
    };

    std::array<FolderCell, nStages> stages;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveMultiplier)
};
} // namespace chowdsp
