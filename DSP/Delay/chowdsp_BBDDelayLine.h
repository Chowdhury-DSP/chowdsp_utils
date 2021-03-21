#pragma once

namespace chowdsp
{

template <size_t STAGES>
class BBDDelayLine
{
  public:
    BBDDelayLine() = default;

    void prepare (double sampleRate)
    {
        evenOn = true;
        FS = (float) sampleRate;
        Ts = 1.0f / FS;

        tn = 0.0f;
        evenOn = true;

        inputFilter = std::make_unique<InputFilterBank> (Ts);
        outputFilter = std::make_unique<OutputFilterBank> (Ts);
        H0 = outputFilter->calcH0();

        reset();
    }

    void reset()
    {
        bufferPtr = 0;
        std::fill (buffer.begin(), buffer.end(), 0.0f);
    }

    void setFilterFreq (float freqHz)
    {
        inputFilter->set_freq(freqHz);
        inputFilter->set_time(tn);

        outputFilter->set_freq(freqHz);
        outputFilter->set_time(tn);
    }

    inline void setDelayTime (float delaySec) noexcept
    {
        const auto clock_rate_hz = (2.0f * (float) STAGES) / delaySec;
        Ts_bbd = 1.0f / clock_rate_hz;

        const auto doubleTs = 2 * Ts_bbd;
        inputFilter->set_delta(doubleTs);
        outputFilter->set_delta(doubleTs);
    }

    inline float process (float u) noexcept
    {
        SIMDComplex<float> xOutAccum;
        while (tn < Ts)
        {
            if (evenOn)
            {
                inputFilter->calcG();
                auto sum = SIMDComplexMulReal(inputFilter->Gcalc, inputFilter->x).sum();
                buffer[bufferPtr++] = sum;
                bufferPtr = (bufferPtr < STAGES) ? bufferPtr : 0;
            }
            else
            {
                auto yBBD = buffer[bufferPtr];
                auto delta = yBBD - yBBD_old;
                yBBD_old = yBBD;
                outputFilter->calcG();
                xOutAccum += outputFilter->Gcalc * delta;
            }

            evenOn = !evenOn;
            tn += Ts_bbd;
        }
        tn -= Ts;

        inputFilter->process(u);
        outputFilter->process(xOutAccum);
        float sum = xOutAccum._r.sum();
        return H0 * yBBD_old + sum;
    }

  private:
    float FS = 48000.0f;
    float Ts = 1.0f / FS;
    float Ts_bbd = Ts;

    std::unique_ptr<InputFilterBank> inputFilter;
    std::unique_ptr<OutputFilterBank> outputFilter;
    float H0 = 1.0f;

    std::array<std::complex<float>, BBDFilterSpec::N_filt> xIn;
    std::array<std::complex<float>, BBDFilterSpec::N_filt> xOut;

    std::array<float, STAGES> buffer;
    size_t bufferPtr = 0;

    float yBBD_old = 0.0f;
    float tn = 0.0f;
    bool evenOn = true;
};

} // namespace chowdsp
