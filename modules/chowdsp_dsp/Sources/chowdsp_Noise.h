#pragma once

namespace chowdsp
{
/** Helper functions for generating noise signals.
 *  NOT desgined to be used directly.
 */
namespace NoiseHelpers
{
    // forward declare uniform01
    template <typename T>
    T uniform01 (juce::Random&) noexcept;
} // namespace NoiseHelpers

/** Audio processor that adds noise to an audio buffer.
 *  Currently support white noise with a uniform or normal
 *  distribution, or pink noise (-3dB / Oct).
*/
template <typename T>
class Noise : public juce::dsp::Gain<typename SampleTypeHelpers::ElementType<T>::Type>
{
    using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

public:
    enum NoiseType
    {
        Uniform, /**< Uniform white noise [-1, 1] */
        Normal, /**< White noise with a normal/Gaussian distribution, generated using the Box-Muller Transform */
        Pink, /**< Pink noise (-3dB / Oct), generated using the Voss algorithm */
    };

    Noise() = default;

    /** Selects a new noise profile */
    void setNoiseType (NoiseType newType) noexcept { type = newType; }

    /** Returns the current noise profile */
    NoiseType getNoiseType() const noexcept { return type; }

    /** Called before processing starts. */
    void prepare (const juce::dsp::ProcessSpec& spec) noexcept;

    /** Resets the internal state of the gain */
    void reset() noexcept;

    /** Sets the seed for the random number generator */
    void setSeed (juce::int64 newSeed) { rand.setSeed (newSeed); }

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept;

private:
    T processSample (T) { return (T) 0; } // hide from dsp::Gain

    template <typename C = T>
    inline typename std::enable_if<std::is_floating_point<C>::value, void>::type
        applyGain (juce::dsp::AudioBlock<T>& block)
    {
        juce::dsp::ProcessContextReplacing<T> context (block);
        juce::dsp::Gain<NumericType>::process (context);
    }

    template <typename C = T>
    inline typename std::enable_if<! std::is_floating_point<C>::value, void>::type
        applyGain (juce::dsp::AudioBlock<T>& block)
    {
        if (block.getNumChannels() == 1)
        {
            auto* data = block.getChannelPointer (0);
            for (size_t i = 0; i < block.getNumSamples(); ++i)
                data[i] *= juce::dsp::Gain<NumericType>::processSample ((NumericType) 1);
        }
        else
        {
            auto* gainData = gainBlock.getChannelPointer (0);
            for (size_t i = 0; i < block.getNumSamples(); ++i)
                gainData[i] = (T) juce::dsp::Gain<NumericType>::processSample ((NumericType) 1);

            for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
            {
                auto* data = block.getChannelPointer (ch);
                for (size_t i = 0; i < block.getNumSamples(); ++i)
                    data[i] *= gainData[i];
            }
        }
    }

    NoiseType type;
    juce::Random rand;
    juce::dsp::Gain<T> gain;

    /** Based on "The Voss algorithm"
        http://www.firstpr.com.au/dsp/pink-noise/
    */
    template <size_t QUALITY = 8>
    struct PinkNoiseGenerator
    {
        std::vector<int> frame;
        std::vector<std::array<T, QUALITY>> values;

        void reset (size_t nChannels)
        {
            frame.resize (nChannels, -1);

            values.clear();
            for (size_t ch = 0; ch < nChannels; ++ch)
            {
                std::array<T, QUALITY> v;
                v.fill ((T) 0);
                values.push_back (v);
            }
        }

        /** Generates pink noise (-3dB / octave) */
        inline T operator() (size_t ch, juce::Random& r) noexcept
        {
            using namespace SIMDUtils;

            int lastFrame = frame[ch];
            frame[ch]++;
            if (frame[ch] >= (1 << QUALITY))
                frame[ch] = 0;
            int diff = lastFrame ^ frame[ch];

            T sum = (T) 0;
            for (size_t i = 0; i < QUALITY; i++)
            {
                if (diff & (1 << i))
                {
                    values[ch][i] = NoiseHelpers::uniform01<T> (r) - (T) 0.5;
                }
                sum += values[ch][i];
            }
            return sum / static_cast<T> (8);
        }
    };

    PinkNoiseGenerator<> pink;

    juce::HeapBlock<char> randBlockData;
    juce::dsp::AudioBlock<T> randBlock;

    juce::HeapBlock<char> gainBlockData;
    juce::dsp::AudioBlock<T> gainBlock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Noise)
};

} // namespace chowdsp

#include "chowdsp_Noise.cpp"
