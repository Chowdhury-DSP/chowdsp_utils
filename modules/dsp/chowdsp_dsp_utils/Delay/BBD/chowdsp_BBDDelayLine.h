#pragma once

namespace chowdsp::BBD
{
/**
 * A class to emulate an analog delay line
 * made using a bucket-brigade device
 */
template <size_t STAGES, bool ALIEN = false>
class BBDDelayLine
{
public:
    BBDDelayLine() = default;
    BBDDelayLine (BBDDelayLine&&) noexcept = default;
    BBDDelayLine& operator= (BBDDelayLine&&) noexcept = default;

    /** Prepares the delay line for processing */
    void prepare (double sampleRate)
    {
        FS = (float) sampleRate;
        Ts = 1.0f / FS;

        tn = 0.0f;
        evenOn = true;

        inputFilter = std::make_unique<InputFilterBank> (Ts);
        outputFilter = std::make_unique<OutputFilterBank> (Ts);
        H0 = outputFilter->calcH0();

        reset();
    }

    /** Resets the state of the delay */
    void reset()
    {
        bufferPtr = 0;
        std::fill (buffer.begin(), buffer.end(), 0.0f);
    }

    /**
     * Sets the cutoff frequency of the input anti-imaging
     * filter used by the bucket-brigade device
     */
    void setInputFilterFreq (float freqHz = BBDFilterSpec::inputFilterOriginalCutoff) const
    {
        inputFilter->set_freq (ALIEN ? freqHz * 0.2f : freqHz);
        inputFilter->set_time (tn);
    }

    /**
     * Sets the cutoff frequency of the output anti-aliasing
     * filter used by the bucket-brigade device
     */
    void setOutputFilterFreq (float freqHz = BBDFilterSpec::outputFilterOriginalCutoff) const
    {
        outputFilter->set_freq (ALIEN ? freqHz * 0.2f : freqHz);
        outputFilter->set_time (tn);
    }

    /**
     * Sets the delay time of the delay line.
     * Internally this changed the "clock rate"
     * of the bucket-brigade device
     */
    void setDelayTime (float delaySec) noexcept
    {
        delaySec = juce::jmax (Ts, delaySec - Ts); // don't divide by zero!!

        const auto clock_rate_hz = (2.0f * (float) STAGES) / delaySec;
        Ts_bbd = 1.0f / clock_rate_hz;

        // if Ts_bbd == 0, then we get an infinite loop, so limit the min. delay
        Ts_bbd = juce::jmax (Ts * 0.01f, Ts_bbd);

        const auto doubleTs = 2 * Ts_bbd;
        inputFilter->set_delta (doubleTs);
        outputFilter->set_delta (doubleTs);
    }

    /** Processes a sample with the delay line (ALIEN MODE) */
    template <bool A = ALIEN>
    inline std::enable_if_t<A, float>
        process (float u) noexcept
    {
        SIMDComplex<float> xOutAccum;
        float yBBD, delta;
        while (tn < 1.0f)
        {
            if (evenOn)
            {
                inputFilter->calcG();
                buffer[bufferPtr++] = xsimd::reduce_add (SIMDUtils::SIMDComplexMulReal (inputFilter->Gcalc, inputFilter->x));
                bufferPtr = (bufferPtr <= STAGES) ? bufferPtr : 0;
            }
            else
            {
                yBBD = buffer[bufferPtr];
                delta = yBBD - yBBD_old;
                yBBD_old = yBBD;
                outputFilter->calcG();
                xOutAccum += outputFilter->Gcalc * delta;
            }

            evenOn = ! evenOn;
            tn += Ts_bbd / Ts;
        }
        tn -= 1.0f;

        inputFilter->process (u);
        outputFilter->process (xOutAccum);
        float sumOut = xsimd::reduce_add (xOutAccum.real());
        return H0 * yBBD_old + sumOut;
    }

    /** Processes a sample with the delay line (BBD MODE) */
    template <bool A = ALIEN>
    inline std::enable_if_t<! A, float>
        process (float u) noexcept
    {
        SIMDComplex<float> xOutAccum {};
        float yBBD, delta;
        while (tn < Ts)
        {
            if (evenOn)
            {
                inputFilter->calcG();
                buffer[bufferPtr++] = xsimd::reduce_add (SIMDUtils::SIMDComplexMulReal (inputFilter->Gcalc, inputFilter->x));
                bufferPtr = (bufferPtr <= STAGES) ? bufferPtr : 0;
            }
            else
            {
                yBBD = buffer[bufferPtr];
                delta = yBBD - yBBD_old;
                yBBD_old = yBBD;
                outputFilter->calcG();
                xOutAccum += outputFilter->Gcalc * delta;
            }

            evenOn = ! evenOn;
            tn += Ts_bbd;
        }
        tn -= Ts;

        inputFilter->process (u);
        outputFilter->process (xOutAccum);
        float sumOut = xsimd::reduce_add (xOutAccum.real());
        return H0 * yBBD_old + sumOut;
    }

private:
    float FS = 48000.0f;
    float Ts = 1.0f / FS;
    float Ts_bbd = Ts;

    std::unique_ptr<InputFilterBank> inputFilter;
    std::unique_ptr<OutputFilterBank> outputFilter;
    float H0 = 1.0f;

    std::array<float, STAGES + 1> buffer;
    size_t bufferPtr = 0;

    float yBBD_old = 0.0f;
    float tn = 0.0f;
    bool evenOn = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BBDDelayLine)
};

} // namespace chowdsp::BBD
