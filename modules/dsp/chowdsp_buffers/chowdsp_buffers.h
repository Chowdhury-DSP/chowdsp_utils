/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_buffers
   vendor:        Chowdhury DSP
   version:       2.1.0
   name:          ChowDSP DSP Audio Buffers
   description:   Audio buffers for ChowDSP plugins
   dependencies:  chowdsp_simd

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       GPLv3

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

#include "Buffers/chowdsp_Buffer.h"
#include "Buffers/chowdsp_StaticBuffer.h"
#include "Buffers/chowdsp_BufferView.h"
#include "Buffers/chowdsp_BufferHelpers.h"
#include "Buffers/chowdsp_SIMDBufferHelpers.h"
#include "Buffers/chowdsp_BufferIterators.h"
