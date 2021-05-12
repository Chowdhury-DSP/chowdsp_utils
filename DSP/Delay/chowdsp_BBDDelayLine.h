#pragma once

namespace chowdsp
{
namespace BBD
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

        /** Prepares the delay line for processing */
        void prepare (double sampleRate)
        {
            FS = (float) sampleRate;
            Ts = 1.0f / FS;

            tn = 0.0f;
            evenOn = 1;

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
     * Sets the cutoff frequency of the anti-alisaing
     * filter used by the bucket-brigade device
     */
        void setFilterFreq (float freqHz)
        {
            inputFilter->set_freq (ALIEN ? freqHz * 0.2f : freqHz);
            inputFilter->set_time (tn);

            outputFilter->set_freq (ALIEN ? freqHz * 0.2f : freqHz);
            outputFilter->set_time (tn);
        }

        /**
     * Sets the delay time of the delay line.
     * Internally this changed the "clock rate"
     * of the bucket-brigade device
     */
        template <bool A = ALIEN>
        inline typename std::enable_if<A == true, void>::type
            setDelayTime (float delaySec) noexcept
        {
            const auto clock_rate_hz = (2.0f * (float) STAGES) / delaySec;
            Ts_bbd = 1.0f / clock_rate_hz;

            const auto doubleTs = 2 * Ts_bbd / Ts;
            inputFilter->set_delta (doubleTs);
            outputFilter->set_delta (doubleTs);
        }

        template <bool A = ALIEN>
        inline typename std::enable_if<A == false, void>::type
            setDelayTime (float delaySec) noexcept
        {
            const auto clock_rate_hz = (2.0f * (float) STAGES) / delaySec;
            Ts_bbd = 1.0f / clock_rate_hz;

            const auto doubleTs = 2 * Ts_bbd;
            inputFilter->set_delta (doubleTs);
            outputFilter->set_delta (doubleTs);
        }

        /** Processes a sample with the delay line */
        template <bool A = ALIEN>
        inline typename std::enable_if<A == true, float>::type
            process (float u) noexcept
        {
            SIMDComplex<float> xOutAccum;
            float sum = 0.0f;
            float yBBD = 0.0f;
            float delta = 0.0f;
            while (tn < 1.0f)
            {
                switch (evenOn)
                {
                    case 1:
                        inputFilter->calcG();
                        sum = SIMDComplexMulReal (inputFilter->Gcalc, inputFilter->x).sum();
                        buffer[bufferPtr++] = sum;
                        bufferPtr = (bufferPtr < STAGES) ? bufferPtr : 0;
                        break;

                    case 0:
                        yBBD = buffer[bufferPtr];
                        delta = yBBD - yBBD_old;
                        yBBD_old = yBBD;
                        outputFilter->calcG();
                        xOutAccum += outputFilter->Gcalc * delta;
                        break;
                }

                evenOn = 1 - evenOn;
                tn += Ts_bbd / Ts;
            }
            tn -= 1.0f;

            inputFilter->process (u);
            outputFilter->process (xOutAccum);
            float sumOut = xOutAccum._r.sum();
            return H0 * yBBD_old + sumOut;
        }

        template <bool A = ALIEN>
        inline typename std::enable_if<A == false, float>::type
            process (float u) noexcept
        {
            SIMDComplex<float> xOutAccum;
            float sum = 0.0f;
            float yBBD = 0.0f;
            float delta = 0.0f;
            while (tn < Ts)
            {
                switch (evenOn)
                {
                    case 1:
                        inputFilter->calcG();
                        sum = SIMDComplexMulReal (inputFilter->Gcalc, inputFilter->x).sum();
                        buffer[bufferPtr++] = sum;
                        bufferPtr = (bufferPtr < STAGES) ? bufferPtr : 0;

                    case 0:
                        yBBD = buffer[bufferPtr];
                        delta = yBBD - yBBD_old;
                        yBBD_old = yBBD;
                        outputFilter->calcG();
                        xOutAccum += outputFilter->Gcalc * delta;
                }

                evenOn = 1 - evenOn;
                tn += Ts_bbd;
            }
            tn -= Ts;

            inputFilter->process (u);
            outputFilter->process (xOutAccum);
            float sumOut = xOutAccum._r.sum();
            return H0 * yBBD_old + sumOut;
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
        int evenOn = 1;
    };

} // namespace BBD

} // namespace chowdsp
