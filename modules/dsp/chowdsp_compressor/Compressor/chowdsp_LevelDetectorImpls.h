#pragma once

namespace chowdsp::compressor
{
/** Simple attack/release coefficients */
template <typename T>
struct BallisticCoeffs
{
    T a1;
    T b0;
};

/** A return-to-zero peak detector */
struct PeakDetector
{
    template <typename T>
    static void process (const BufferView<T>& buffer, const BallisticCoeffs<T>& attackCoeffs, const BallisticCoeffs<T>& releaseCoeffs, T* z, T /* thresholdGain */) noexcept
    {
        for (auto [ch, data] : buffer_iters::channels (buffer))
        {
            for (auto& x : data)
            {
                const auto abs_x = std::abs (x);
                const auto b0 = abs_x > z[ch] ? attackCoeffs.b0 : releaseCoeffs.b0;
                z[ch] += b0 * (abs_x - z[ch]);
                x = z[ch];
            }
        }
    }
};

/** A return-to-threshold peak detector */
struct PeakRtTDetector
{
    template <typename T>
    static void process (const BufferView<T>& buffer, const BallisticCoeffs<T>& attackCoeffs, const BallisticCoeffs<T>& releaseCoeffs, T* z, T thresholdGain) noexcept
    {
        for (auto [ch, data] : buffer_iters::channels (buffer))
        {
            for (auto& x : data)
            {
                const auto abs_x = std::abs (x);
                const auto b0 = abs_x > z[ch] ? attackCoeffs.b0 : releaseCoeffs.b0;
                const auto x_eff = abs_x > z[ch] ? abs_x : thresholdGain;
                z[ch] += b0 * (x_eff - z[ch]);
                x = z[ch];
            }
        }
    }
};

/** An RMS level detector */
struct RMSDetector
{
    template <typename T>
    static void process (const BufferView<T>& buffer, const BallisticCoeffs<T>& attackCoeffs, const BallisticCoeffs<T>& releaseCoeffs, T* z, T /* thresholdGain */) noexcept
    {
        for (auto [ch, data] : buffer_iters::channels (buffer))
        {
            for (auto& x : data)
            {
                const auto sq_x = x * x;
                const auto& coeffs = sq_x > z[ch] ? attackCoeffs : releaseCoeffs;
                z[ch] = coeffs.a1 * z[ch] + coeffs.b0 * sq_x;
                x = z[ch];
            }
        }

        BufferMath::applyFunction (buffer,
                                   [] (auto x)
                                   {
                                       CHOWDSP_USING_XSIMD_STD (sqrt);
                                       return sqrt (x);
                                   });
    }
};
} // namespace chowdsp::compressor
