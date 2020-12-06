#pragma once

namespace chowdsp
{

/** Sinusoidal oscillator using the "magic circle" algorithm.
 * See: https://ccrma.stanford.edu/~jos/pasp/Digital_Sinusoid_Generators.html, eq. 2
 */
template<typename T>
class SineWave
{
public:
    SineWave() = default;

    /** Sets the frequency of the oscillator. */
    void setFrequency (T newFrequency) noexcept
    {
        freq = newFrequency;
        eps = 2 * std::sin (juce::MathConstants<T>::pi * newFrequency / fs);
    }

    /** Returns the current frequency of the oscillator. */
    T getFrequency() const noexcept { return freq; }

    void prepare (const juce::dsp::ProcessSpec& spec) noexcept
    {
        fs = static_cast<T> (spec.sampleRate);
        reset();
    }

    /** Resets the internal state of the oscillator */
    void reset() noexcept
    {
        // reset state to be "in phase"
        x1 = -1.0f;
        x2 = 0.0f;
    }

    /** Returns the result of processing a single sample. */
    T processSample() noexcept
    {
        auto y = x2;
        x1 += eps * x2;
        x2 -= eps * x1;
        return y;
    }

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        auto&& outBlock = context.getOutputBlock();
        auto&& inBlock  = context.getInputBlock();

        auto len           = outBlock.getNumSamples();
        auto numChannels   = outBlock.getNumChannels();
        auto inputChannels = inBlock.getNumChannels();

        if (context.isBypassed)
        {
            context.getOutputBlock().clear();

            for (size_t i = 0; i < len; ++i)
            {
                x1 += eps * x2;
                x2 -= eps * x1;
            }
        }
        else
        {
            T x1_temp = x1;
            T x2_temp = x2;
            size_t ch;

            if (context.usesSeparateInputAndOutputBlocks())
            {
                for (ch = 0; ch < juce::jmin (numChannels, inputChannels); ++ch)
                {
                    x1_temp = x1;
                    x2_temp = x2;
                    auto* dst = outBlock.getChannelPointer (ch);
                    auto* src = inBlock.getChannelPointer (ch);

                    for (size_t i = 0; i < len; ++i)
                    {
                        dst[i] = src[i] + x2_temp;
                        x1_temp += eps * x2_temp;
                        x2_temp -= eps * x1_temp;
                    }
                }
            }
            else
            {
                for (ch = 0; ch < juce::jmin (numChannels, inputChannels); ++ch)
                {
                    x1_temp = x1;
                    x2_temp = x2;
                    auto* dst = outBlock.getChannelPointer (ch);

                    for (size_t i = 0; i < len; ++i)
                    {
                        dst[i] += x2_temp;
                        x1_temp += eps * x2_temp;
                        x2_temp -= eps * x1_temp;
                    }
                }
            }
            

            for (; ch < numChannels; ++ch)
            {
                x1_temp = x1;
                x2_temp = x2;
                auto* dst = outBlock.getChannelPointer (ch);

                for (size_t i = 0; i < len; ++i)
                {
                    dst[i] = x2_temp;
                    x1_temp += eps * x2_temp;
                    x2_temp -= eps * x1_temp;
                }
            }
            
            x1 = x1_temp;
            x2 = x2_temp;
        }
    }

private:
    T x1  = static_cast<T> (0.0);
    T x2  = static_cast<T> (0.0);
    T eps = static_cast<T> (0.0);

    T freq = static_cast<T> (0.0);
    T fs   = static_cast<T> (44100.0);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SineWave)
};

} // chowdsp
