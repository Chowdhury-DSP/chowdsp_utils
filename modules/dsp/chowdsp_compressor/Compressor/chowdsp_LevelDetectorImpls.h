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

/** Computes ballistic coefficients for a given attack/release time */
template <typename T>
BallisticCoeffs<T> computeBallisticCoeffs (T timeMs, T fs)
{
    BallisticCoeffs<T> coeffs {};
    coeffs.a1 = std::exp ((T) -1 / (fs * timeMs * (T) 0.001));
    coeffs.b0 = (T) 1 - coeffs.a1;
    return coeffs;
}

/** A return-to-zero peak detector */
struct PeakDetector
{
    template <typename T>
    static void process (const BufferView<T>& buffer, const BallisticCoeffs<T>& attackCoeffs, const BallisticCoeffs<T>& releaseCoeffs, T* z, T /* thresholdGain */) noexcept
    {
        for (auto [ch, data] : buffer_iters::channels (buffer))
        {
            ScopedValue _z { z[ch] };
            for (auto& x : data)
            {
                const auto abs_x = std::abs (x);
                const auto b0 = abs_x > _z.get() ? attackCoeffs.b0 : releaseCoeffs.b0;
                _z.get() += b0 * (abs_x - _z.get());
                x = _z.get();
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
            ScopedValue _z { z[ch] };
            for (auto& x : data)
            {
                const auto abs_x = std::abs (x);
                const auto b0 = abs_x > _z.get() ? attackCoeffs.b0 : releaseCoeffs.b0;
                const auto x_eff = abs_x > _z.get() ? abs_x : thresholdGain;
                _z.get() += b0 * (x_eff - _z.get());
                x = _z.get();
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
            ScopedValue _z { z[ch] };
            for (auto& x : data)
            {
                const auto sq_x = x * x;
                const auto& coeffs = sq_x > _z.get() ? attackCoeffs : releaseCoeffs;
                _z.get() = coeffs.a1 * _z.get() + coeffs.b0 * sq_x;
                x = _z.get();
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
