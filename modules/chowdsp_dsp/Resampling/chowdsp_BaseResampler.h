#pragma once

namespace chowdsp
{
namespace ResamplingTypes
{
    class BaseResampler
    {
    public:
        /** Default constructor */
        BaseResampler() = default;

        /** Prepares the resampler for a given input sample rate */
        virtual void prepare (double sampleRate, double startRatio = 1.0) = 0;

        /** Resets the state of the resampler */
        virtual void reset() = 0;

        /** Sets the ratio of the output sample rate over input sample rate */
        virtual void setResampleRatio (float newRatio) = 0;
        
        /** Returns the ratio of the output sample rate over input sample rate */
        virtual float getResampleRatio() const noexcept = 0;

        /** Processes a buffer of samples
     * 
     *  @return the number of output samples generated by the resampler
     */
        virtual size_t process (const float* input, float* output, size_t numSamples) noexcept = 0;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BaseResampler)
    };

} // namespace ResamplingTypes

} // namespace chowdsp
