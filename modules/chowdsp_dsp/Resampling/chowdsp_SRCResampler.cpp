#include "chowdsp_SRCResampler.h"

#if CHOWDSP_USE_LIBSAMPLERATE

namespace chowdsp::ResamplingTypes
{
template <size_t QUALITY>
void SRCResampler<QUALITY>::prepare (double sampleRate, double startRatio)
{
    fs = sampleRate;

    int error;
    src_state.reset (src_new (QUALITY, 1, &error));
    src_set_ratio (src_state.get(), startRatio);
    ratio = startRatio;
}

template <size_t QUALITY>
void SRCResampler<QUALITY>::reset()
{
    src_reset (src_state.get());
    src_set_ratio (src_state.get(), ratio);
}

template <size_t QUALITY>
size_t SRCResampler<QUALITY>::process (const float* input, float* output, size_t numSamples) noexcept
{
    SRC_DATA src_data {
        input, // data_in
        output, // data_out
        (int) numSamples, // input_frames
        int ((double) numSamples * ratio) + 1, // output_frames
        0, // input_frames_used
        0, // output_frames_gen
        0, // end_of_input
        ratio // src_ratio
    };

    src_process (src_state.get(), &src_data);

    return (size_t) src_data.output_frames_gen;
}

template class SRCResampler<SRC_SINC_BEST_QUALITY>;
template class SRCResampler<SRC_SINC_MEDIUM_QUALITY>;
template class SRCResampler<SRC_SINC_FASTEST>;
template class SRCResampler<SRC_ZERO_ORDER_HOLD>;
template class SRCResampler<SRC_LINEAR>;

} // namespace chowdsp::ResamplingTypes

#endif // CHOWDSP_USE_LIBSAMPLERATE
