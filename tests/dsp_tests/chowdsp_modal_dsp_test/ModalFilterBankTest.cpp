#include <CatchUtils.h>
#include <chowdsp_modal_dsp/chowdsp_modal_dsp.h>

namespace
{
constexpr double modalSampleRate = 48000.0;
constexpr int modalBlockSize = 20;
constexpr float analysisFs = 48000.0f;

float tau2T60 (float tau)
{
    return 1.0f / (std::log (std::exp (analysisFs / tau)) / std::log (1000.0f));
}

template <typename FilterBank, typename RefModes>
void testImpulse (FilterBank& filterbank, RefModes& refModes)
{
    chowdsp::Buffer<float> buffer (1, modalBlockSize);
    buffer.getWritePointer (0)[0] = 1.0f;

    filterbank.process (buffer);
    auto actualOut = filterbank.getRenderBuffer().getReadPointer (0);

    const auto* bufferPtr = buffer.getReadPointer (0);
    for (int n = 0; n < modalBlockSize; ++n)
    {
        float y = 0.0f;
        for (auto& mode : refModes)
            y += mode.processSample (bufferPtr[n]);

        REQUIRE_MESSAGE (actualOut[n] == Catch::Approx (y).margin (1.0e-6f), "Modal filterbank output is incorrect!");
    }
}
} // namespace

TEST_CASE ("Modal FilterBank Test", "[dsp][modal]")
{
    SECTION ("Real/Imag-Tau Test")
    {
        constexpr float freqs[] = { 50.0f, 110.0f, 210.0f, 390.0f };
        constexpr float taus[] = { 2000.0f, 1000.0f, 500.0f, 60.0f };
        constexpr float ampsReal[] = { 0.05f, 0.03f, 0.02f, 0.01f };
        constexpr float ampsImag[] = { 0.05f, 0.0f, 0.02f, -0.01f };

        chowdsp::ModalFilter<float> refModes[4];
        for (int i = 0; i < 4; ++i)
        {
            refModes[i].prepare ((float) modalSampleRate);
            refModes[i].setFreq (freqs[i]);
            refModes[i].setDecay (tau2T60 (taus[i]));
            refModes[i].setAmp (std::complex<float> (ampsReal[i], ampsImag[i]));
        }

        chowdsp::ModalFilterBank<4> filterbank;
        filterbank.prepare (modalSampleRate, modalBlockSize);
        filterbank.setModeAmplitudes (ampsReal, ampsImag, -1.0f);
        filterbank.setModeFrequencies (freqs);
        filterbank.setModeDecays (taus, analysisFs);

        testImpulse (filterbank, refModes);
        filterbank.reset();
        for (auto& mode : refModes)
            mode.reset();
        testImpulse (filterbank, refModes);
    }

    SECTION ("Complex-T60 Test")
    {
        constexpr float freqs[] = { 500.0f, 1005.0f, 2010.0f, 3700.0f, 16004.0f };
        constexpr float t60s[] = { 1.0f, 0.5f, 0.4f, 0.3f, 0.2f };
        const std::complex<float> amps[] = { { 1.0f, 0.0f }, { 0.5f, 0.5f }, { 0.3f, -0.6f }, { 0.5f, -0.25f }, { 0.35f, -0.1f } };

        chowdsp::ModalFilter<float> refModes[4];
        for (int i = 0; i < 4; ++i)
        {
            refModes[i].prepare ((float) modalSampleRate);
            refModes[i].setFreq (freqs[i]);
            refModes[i].setDecay (t60s[i]);
            refModes[i].setAmp (amps[i]);
        }

        chowdsp::ModalFilterBank<5> filterbank;
        filterbank.prepare (modalSampleRate, modalBlockSize);
        filterbank.setNumModesToProcess (4);
        filterbank.setModeAmplitudes (amps);
        filterbank.setModeFrequencies (freqs);
        filterbank.setModeDecays (t60s);

        testImpulse (filterbank, refModes);
    }
}
