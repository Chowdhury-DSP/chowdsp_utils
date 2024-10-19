/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "chowdsp_Panner.h"

namespace chowdsp
{
//==============================================================================
template <typename SampleType>
Panner<SampleType>::Panner()
{
    update();
    reset();
}

//==============================================================================
template <typename SampleType>
void Panner<SampleType>::setRule (Rule newRule)
{
    currentRule = newRule;
    update();
}

template <typename SampleType>
void Panner<SampleType>::setPan (SampleType newPan)
{
    jassert (newPan >= -1.0 && newPan <= 1.0);

    pan = juce::jlimit (static_cast<SampleType> (-1.0), static_cast<SampleType> (1.0), newPan);
    update();
}

//==============================================================================
template <typename SampleType>
void Panner<SampleType>::prepare (const juce::dsp::ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    reset();
}

template <typename SampleType>
void Panner<SampleType>::reset()
{
    leftVolume.reset (sampleRate, 0.05);
    rightVolume.reset (sampleRate, 0.05);
}

//==============================================================================
template <typename SampleType>
void Panner<SampleType>::update()
{
    SampleType leftValue, rightValue;

    auto normalisedPan = static_cast<SampleType> (0.5) * (pan + static_cast<SampleType> (1.0));

    switch (currentRule)
    {
        case Rule::balanced:
            leftValue = juce::jmin (static_cast<SampleType> (0.5), static_cast<SampleType> (1.0) - normalisedPan);
            rightValue = juce::jmin (static_cast<SampleType> (0.5), normalisedPan);
            break;

        case Rule::linear:
            leftValue = static_cast<SampleType> (1.0) - normalisedPan;
            rightValue = normalisedPan;
            break;

        case Rule::sin3dB:
            leftValue = static_cast<SampleType> (std::sin (0.5 * juce::MathConstants<double>::pi * (1.0 - normalisedPan)));
            rightValue = static_cast<SampleType> (std::sin (0.5 * juce::MathConstants<double>::pi * normalisedPan));
            break;

        case Rule::sin4p5dB:
            leftValue = static_cast<SampleType> (std::pow (std::sin (0.5 * juce::MathConstants<double>::pi * (1.0 - normalisedPan)), 1.5));
            rightValue = static_cast<SampleType> (std::pow (std::sin (0.5 * juce::MathConstants<double>::pi * normalisedPan), 1.5));
            break;

        case Rule::sin6dB:
            leftValue = static_cast<SampleType> (std::pow (std::sin (0.5 * juce::MathConstants<double>::pi * (1.0 - normalisedPan)), 2.0));
            rightValue = static_cast<SampleType> (std::pow (std::sin (0.5 * juce::MathConstants<double>::pi * normalisedPan), 2.0));
            break;

        case Rule::squareRoot3dB:
            leftValue = std::sqrt (static_cast<SampleType> (1.0) - normalisedPan);
            rightValue = std::sqrt (normalisedPan);
            break;

        case Rule::squareRoot4p5dB:
            leftValue = static_cast<SampleType> (std::pow (std::sqrt (1.0 - normalisedPan), 1.5));
            rightValue = static_cast<SampleType> (std::pow (std::sqrt (normalisedPan), 1.5));
            break;

        default:
            leftValue = juce::jmin (static_cast<SampleType> (0.5), static_cast<SampleType> (1.0) - normalisedPan);
            rightValue = juce::jmin (static_cast<SampleType> (0.5), normalisedPan);
            break;
    }

    const auto boostValue = getBoostForRule (currentRule);
    leftVolume.setTargetValue (leftValue * boostValue);
    rightVolume.setTargetValue (rightValue * boostValue);
}

template <typename SampleType>
SampleType Panner<SampleType>::getBoostForRule (Rule rule)
{
    switch (rule)
    {
        case Rule::balanced:
        case Rule::linear:
        case Rule::sin6dB:
            return static_cast<SampleType> (2.0);

        case Rule::sin3dB:
        case Rule::squareRoot3dB:
            return std::sqrt (static_cast<SampleType> (2.0));

        case Rule::sin4p5dB:
        case Rule::squareRoot4p5dB:
            return static_cast<SampleType> (std::pow (2.0, 3.0 / 4.0));

        default:
            return static_cast<SampleType> (2.0);
    }
}

//==============================================================================
#if CHOWDSP_ALLOW_TEMPLATE_INSTANTIATIONS
template class Panner<float>;
template class Panner<double>;
#endif
} // namespace chowdsp
