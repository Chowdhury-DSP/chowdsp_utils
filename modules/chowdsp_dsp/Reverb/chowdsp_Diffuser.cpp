//#include "chowdsp_Diffuser.h"

namespace chowdsp::Reverb
{
#ifndef DOXYGEN
namespace detail
{
    template <typename T, std::size_t N, std::size_t Idx = N>
    struct array_maker
    {
        template <typename... Ts>
        static std::array<T, N> make_array (const T& v, Ts... tail)
        {
            return array_maker<T, N, Idx - 1>::make_array (v, v, tail...);
        }
    };

    template <typename T, std::size_t N>
    struct array_maker<T, N, 1>
    {
        template <typename... Ts>
        static std::array<T, N> make_array (const T& v, Ts... tail)
        {
            return std::array<T, N> { v, tail... };
        }
    };
} // namespace detail
#endif // DOXYGEN

//======================================================================
double DefaultDiffuserConfig::getDelaySamples (int channelIndex, int nChannels, double sampleRate)
{
    const auto delaySamplesRange = (double) delayRangeMs * 0.001 * sampleRate;
    const auto rangeLow = delaySamplesRange * channelIndex / nChannels;
    const auto rangeHigh = delaySamplesRange * (channelIndex + 1) / nChannels;

    return rangeLow + rand.nextDouble() * (rangeHigh - rangeLow);
}

double DefaultDiffuserConfig::getPolarityMultiplier (int /*channelIndex*/, int /*nChannels*/)
{
    return rand.nextBool() ? 1.0 : -1.0;
}

//======================================================================
template <typename FloatType, int nChannels, typename DelayInterpType>
Diffuser<FloatType, nChannels, DelayInterpType>::Diffuser() : delays (detail::array_maker<DelayType, nChannels> (DelayType { 1 << 18 }))
{
}

template <typename FloatType, int nChannels, typename DelayInterpType>
template <typename DiffuserConfig>
void Diffuser<FloatType, nChannels, DelayInterpType>::prepare (double sampleRate, const DiffuserConfig& config)
{
    for (int i = 0; i < nChannels; ++i)
    {
        delays[i].prepare ({ sampleRate, 128, 1 });
        delays[i].setDelay ((FloatType) config.getDelaySamples (i, nChannels, sampleRate));
        polarityMultipliers[i] = (FloatType) config.getPolarityMultiplier (i, nChannels);
    }
}
} // namespace chowdsp::Reverb
