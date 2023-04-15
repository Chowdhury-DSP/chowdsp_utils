#pragma once

namespace chowdsp
{
/**
 * An emulation of the wavefolding "timbre" circuit from the Buchla 259,
 * using integrated waveshaping (ADAA). The wavefolder is designed so
 * that signals below -6 dB will not be affected, and the majority of
 * the wavefolding will happen for signals in the range [-6 dB, +12 db].
 * Note that the original circuit also contains a first-order lowpass
 * filter at the output, which is not included in this processor, but
 * it should be straightforward to implement this yourself, using
 * something like chowdsp::FirstOrderLPF.
 *
 * Reference paper: http://www.dafx17.eca.ed.ac.uk/papers/DAFx17_paper_82.pdf
 * Derivation: https://www.desmos.com/calculator/o4j8apf9t5
 */
template <typename T, ADAAWaveshaperMode mode = ADAAWaveshaperMode::MinusX>
class WestCoastWavefolder : public ADAAWaveshaper<T, mode>
{
public:
    explicit WestCoastWavefolder (LookupTableCache* lutCache = nullptr, T range = (T) 10, int N = 1 << 17) : ADAAWaveshaper<T, mode> (lutCache, "chowdsp_west_coast_folder")
    {
        static constexpr std::array<FolderCell, 5> cells {
            FolderCell { 0.8333, 0.5, 0.6, -12.0 },
            FolderCell { 0.3768, 1.1281, 2.994, -27.777 },
            FolderCell { 0.2829, 1.5446, 5.46, -21.428 },
            FolderCell { 0.5743, 1.0338, 1.8, 17.647 },
            FolderCell { 0.2673, 1.0907, 4.08, 36.363 },
        };
        juce::ignoreUnused (cells);

        static constexpr auto xMix = 5.0;

        this->initialise (
            [&] (auto x)
            {
                double y = xMix * x;
                for (auto& cell : cells)
                    y += cell.mix * cell.func (x);
                return y;
            },
            [&] (auto x)
            {
                double y = 0.5 * xMix * Power::ipow<2> (x);
                for (auto& cell : cells)
                    y += cell.mix * cell.funcDeriv (x);
                return y;
            },
            [&] (auto x)
            {
                double y = (xMix / 6.0) * Power::ipow<3> (x);
                for (auto& cell : cells)
                    y += cell.mix * cell.funcDeriv2 (x);
                return y;
            },
            -range,
            range,
            N);
    }

private:
    struct FolderCell
    {
        const double G = 0.0; // folder cell "gain"
        const double B = 0.0; // folder cell "bias"
        const double thresh = 0.0; // folder cell "threshold"
        const double mix = 0.0; // folder cell mixing factor

        const double Bp = 0.5 * G * Power::ipow<2> (thresh) - B * thresh;
        static constexpr auto oneSixth = 1.0 / 6.0;
        const double Bpp = oneSixth * G * Power::ipow<3> (thresh) - 0.5 * B * Power::ipow<2> (thresh) - thresh * Bp;

        [[nodiscard]] inline double func (double x) const noexcept
        {
            return std::abs (x) > thresh ? (G * x - B * Math::sign (x)) : 0.0;
        }

        [[nodiscard]] inline double funcDeriv (double x) const noexcept
        {
            return std::abs (x) > thresh
                       ? 0.5 * G * Power::ipow<2> (x) - B * x * Math::sign (x) - Bp
                       : 0.0;
        }

        [[nodiscard]] inline double funcDeriv2 (double x) const noexcept
        {
            const auto sgn = Math::sign (x);
            return std::abs (x) > thresh
                       ? oneSixth * G * Power::ipow<3> (x) - 0.5 * B * Power::ipow<2> (x) * sgn - Bp * x - Bpp * sgn
                       : 0.0;
        }
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WestCoastWavefolder)
};
} // namespace chowdsp
