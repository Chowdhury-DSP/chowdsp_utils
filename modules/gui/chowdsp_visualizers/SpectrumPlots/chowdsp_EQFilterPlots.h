#pragma once

namespace chowdsp::EQ
{
/** Base class for plotting EQ filters. */
struct EQFilterPlot
{
    EQFilterPlot() = default;
    virtual ~EQFilterPlot() = default;

    virtual void setCutoffFrequency ([[maybe_unused]] float cutoffFreqHz) {}
    virtual void setQValue ([[maybe_unused]] float qVal) {}
    virtual void setGainDecibels ([[maybe_unused]] float gainDB) {}
    [[nodiscard]] virtual float getMagnitudeForFrequency ([[maybe_unused]] float freqHz) const { return 1.0f; }
};

/** Plotting helper for first-order filters. */
struct FirstOrderFilterPlot : public EQFilterPlot
{
    float b_coeffs[2] {}; // s-domain numerator coefficients
    float a_coeffs[2] { 1.0f, 1.0f }; // s-domain denominator coefficients
    float freq0 = 1.0f; // resonant frequency

    void setCutoffFrequency (float cutoffFreqHz) override
    {
        freq0 = cutoffFreqHz;
    }

    [[nodiscard]] float getMagnitudeForFrequency (float freqHz) const override
    {
        const auto s = std::complex<float> { 0, freqHz / freq0 };
        const auto numerator = s * b_coeffs[1] + b_coeffs[0];
        const auto denominator = s * a_coeffs[1] + a_coeffs[0];
        return std::abs (numerator / denominator);
    }
};

/** Plotting helper for second-order filters. */
struct SecondOrderFilterPlot : public EQFilterPlot
{
    float b_coeffs[3] {}; // s-domain numerator coefficients
    float a_coeffs[3] { 1.0f, 1.0f, 1.0f }; // s-domain denominator coefficients
    float freq0 = 1.0f; // resonant frequency

    void setCutoffFrequency (float cutoffFreqHz) override
    {
        freq0 = cutoffFreqHz;
    }

    [[nodiscard]] float getMagnitudeForFrequency (float freqHz) const override
    {
        const auto s = std::complex<float> { 0, freqHz / freq0 };
        const auto sSq = s * s;
        const auto numerator = sSq * b_coeffs[2] + s * b_coeffs[1] + b_coeffs[0];
        const auto denominator = sSq * a_coeffs[2] + s * a_coeffs[1] + a_coeffs[0];
        return std::abs (numerator / denominator);
    }
};

/** Plotting helper for first-order LPF. */
struct LPF1Plot : FirstOrderFilterPlot
{
    LPF1Plot()
    {
        b_coeffs[0] = 1.0f;
    }
};

/** Plotting helper for first-order HPF. */
struct HPF1Plot : FirstOrderFilterPlot
{
    HPF1Plot()
    {
        b_coeffs[1] = 1.0f;
    }
};

/** Plotting helper for second-order LPF. */
struct LPF2Plot : SecondOrderFilterPlot
{
    LPF2Plot()
    {
        b_coeffs[0] = 1.0f;
    }

    void setQValue (float qVal) override
    {
        a_coeffs[1] = 1.0f / qVal;
    }
};

/** Plotting helper for second-order HPF. */
struct HPF2Plot : SecondOrderFilterPlot
{
    HPF2Plot()
    {
        b_coeffs[2] = 1.0f;
    }

    void setQValue (float qVal) override
    {
        a_coeffs[1] = 1.0f / qVal;
    }
};

/** Plotting helper for second-order BPF. */
struct BPF2Plot : SecondOrderFilterPlot
{
    BPF2Plot() = default;

    void setQValue (float qVal) override
    {
        b_coeffs[1] = 1.0f / qVal;
        a_coeffs[1] = 1.0f / qVal;
    }
};

/** Plotting helper for peaking filter. */
struct BellPlot : SecondOrderFilterPlot
{
    BellPlot()
    {
        b_coeffs[2] = 1.0f;
        b_coeffs[0] = 1.0f;
    }

    void setQValue (float qVal) override
    {
        qValue = qVal;
        update();
    }

    void setGainDecibels (float gainDB) override
    {
        linearGain = juce::Decibels::decibelsToGain (gainDB);
        update();
    }

private:
    void update()
    {
        if (linearGain > 1.0f)
        {
            b_coeffs[1] = linearGain / qValue;
            a_coeffs[1] = 1.0f / qValue;
        }
        else
        {
            b_coeffs[1] = 1.0f / qValue;
            a_coeffs[1] = 1.0f / (linearGain * qValue);
        }
    }

    float linearGain = 1.0f;
    float qValue = CoefficientCalculators::butterworthQ<float>;
};

/** Plotting helper for notch filter. */
struct NotchPlot : SecondOrderFilterPlot
{
    NotchPlot()
    {
        b_coeffs[2] = 1.0f;
        b_coeffs[0] = 1.0f;
    }

    void setQValue (float qVal) override
    {
        a_coeffs[1] = 1.0f / qVal;
    }
};

/** Plotting helper for low-shelf filter. */
struct LowShelfPlot : SecondOrderFilterPlot
{
    LowShelfPlot() = default;

    void setQValue (float qVal) override
    {
        qValue = qVal;
        b_coeffs[1] = A * sqrtA / qValue;
        a_coeffs[1] = sqrtA / qValue;
    }

    void setGainDecibels (float gainDB) override
    {
        A = std::sqrt (juce::Decibels::decibelsToGain (gainDB));
        sqrtA = std::sqrt (A);

        b_coeffs[2] = A;
        b_coeffs[1] = A * sqrtA / qValue;
        b_coeffs[0] = A * A;

        a_coeffs[2] = A;
        a_coeffs[1] = sqrtA / qValue;
    }

private:
    float A = 1.0f;
    float sqrtA = 1.0f;
    float qValue = CoefficientCalculators::butterworthQ<float>;
};

/** Plotting helper for high-shelf filter. */
struct HighShelfPlot : SecondOrderFilterPlot
{
    HighShelfPlot() = default;

    void setQValue (float qVal) override
    {
        qValue = qVal;
        b_coeffs[1] = A * sqrtA / qValue;
        a_coeffs[1] = sqrtA / qValue;
    }

    void setGainDecibels (float gainDB) override
    {
        A = std::sqrt (juce::Decibels::decibelsToGain (gainDB));
        sqrtA = std::sqrt (A);

        b_coeffs[2] = A * A;
        b_coeffs[1] = A * sqrtA / qValue;
        b_coeffs[0] = A;

        a_coeffs[1] = sqrtA / qValue;
        a_coeffs[0] = A;
    }

private:
    float A = 1.0f;
    float sqrtA = 1.0f;
    float qValue = CoefficientCalculators::butterworthQ<float>;
};

/** Plotting helper for higher-order LPFs. */
template <int order>
struct HigherOrderLPFPlot : EQFilterPlot
{
    HigherOrderLPFPlot()
    {
        for (auto [index, plot] : chowdsp::enumerate (plots))
        {
            plot.b_coeffs[0] = 1.0f;
            plot.a_coeffs[1] = 1.0f / butterQVals[index];
        }
    }

    void setCutoffFrequency (float cutoffFreqHz) override
    {
        for (auto& plot : plots)
            plot.freq0 = cutoffFreqHz;

        if constexpr (order % 2 == 1)
            extraPlot.setCutoffFrequency (cutoffFreqHz);
    }

    void setQValue (float qVal) override
    {
        plots[0].a_coeffs[1] = 1.0f / ((qVal / CoefficientCalculators::butterworthQ<float>) *butterQVals[0]);
    }

    [[nodiscard]] float getMagnitudeForFrequency (float freqHz) const override
    {
        float result = 1.0f;
        for (auto& plot : plots)
            result *= plot.getMagnitudeForFrequency (freqHz);

        if constexpr (order % 2 == 1)
            result *= extraPlot.getMagnitudeForFrequency (freqHz);

        return result;
    }

private:
    LPF1Plot extraPlot;
    std::array<SecondOrderFilterPlot, size_t (order / 2)> plots {};
    static constexpr auto butterQVals = QValCalcs::butterworth_Qs<float, order>();
};

/** Plotting helper for higher-order HPFs. */
template <int order>
struct HigherOrderHPFPlot : EQFilterPlot
{
    HigherOrderHPFPlot()
    {
        for (auto [index, plot] : chowdsp::enumerate (plots))
        {
            plot.b_coeffs[2] = 1.0f;
            plot.a_coeffs[1] = 1.0f / butterQVals[index];
        }
    }

    void setCutoffFrequency (float cutoffFreqHz) override
    {
        for (auto& plot : plots)
            plot.freq0 = cutoffFreqHz;

        if constexpr (order % 2 == 1)
            extraPlot.setCutoffFrequency (cutoffFreqHz);
    }

    void setQValue (float qVal) override
    {
        plots[0].a_coeffs[1] = 1.0f / ((qVal / CoefficientCalculators::butterworthQ<float>) *butterQVals[0]);
    }

    [[nodiscard]] float getMagnitudeForFrequency (float freqHz) const override
    {
        float result = 1.0f;
        for (auto& plot : plots)
            result *= plot.getMagnitudeForFrequency (freqHz);

        if constexpr (order % 2 == 1)
            result *= extraPlot.getMagnitudeForFrequency (freqHz);

        return result;
    }

private:
    HPF1Plot extraPlot;
    std::array<SecondOrderFilterPlot, size_t (order / 2)> plots {};
    static constexpr auto butterQVals = QValCalcs::butterworth_Qs<float, order>();
};
} // namespace chowdsp::EQ
