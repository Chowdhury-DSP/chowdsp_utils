/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_buffers
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP DSP Audio Buffers
   description:   Audio buffers for ChowDSP plugins
   dependencies:  chowdsp_simd

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-Clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

//STL includes
#include <array>

//JUCE includes
#include <chowdsp_simd/chowdsp_simd.h>

#if JUCE_MODULE_AVAILABLE_juce_dsp
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wimplicit-const-int-float-conversion")
#include <juce_dsp/juce_dsp.h>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#include "Buffers/chowdsp_SIMDAudioBlock.h"
#endif

/** The max number of channels that can fit in a chowdsp::Buffer or chowdsp::BufferView */
#ifndef CHOWDSP_BUFFER_MAX_NUM_CHANNELS
#define CHOWDSP_BUFFER_MAX_NUM_CHANNELS 32
#else
// If we have a max number of channels, we can use that to set the default max channel count
#ifndef CHOWDSP_PROCESSOR_DEFAULT_CHANNEL_COUNT
#define CHOWDSP_PROCESSOR_DEFAULT_CHANNEL_COUNT CHOWDSP_BUFFER_MAX_NUM_CHANNELS
#endif
#endif

namespace chowdsp
{
constexpr auto dynamicChannelCount = std::numeric_limits<size_t>::max();

/** The default channel count to use for multi-channel processors */
#ifdef CHOWDSP_PROCESSOR_DEFAULT_CHANNEL_COUNT
constexpr auto defaultChannelCount = CHOWDSP_PROCESSOR_DEFAULT_CHANNEL_COUNT;
#else
constexpr auto defaultChannelCount = dynamicChannelCount;
#endif

#ifndef DOXYGEN
namespace buffers_detail
{
    /**
     * Divides two numbers and rounds up if there is a remainder.
     *
     * This is often useful for figuring out haw many SIMD registers are needed
     * to contain a given number of scalar values.
    */
    template <typename T>
    constexpr T ceiling_divide (T num, T den)
    {
        return (num + den - 1) / den;
    }
} // namespace buffers_detail
#endif
} // namespace chowdsp

#include "Buffers/chowdsp_Buffer.h"
#include "Buffers/chowdsp_StaticBuffer.h"
#include "Buffers/chowdsp_BufferView.h"
#include "Buffers/chowdsp_BufferHelpers.h"
#include "Buffers/chowdsp_SIMDBufferHelpers.h"
#include "Buffers/chowdsp_BufferIterators.h"
