#pragma once

/**
 * This file contains custom overrides of some JUCE macros.
 */

// @TODO: figure out a way to re-implement jassert...
#define jassert(expression)
#define jassertfalse

/** This macro defines the C calling convention used as the standard for JUCE calls. */
#if JUCE_WINDOWS
#define JUCE_CALLTYPE __stdcall
#define JUCE_CDECL __cdecl
#else
#define JUCE_CALLTYPE
#define JUCE_CDECL
#endif

#ifndef JUCE_API
#define JUCE_API /**< This macro is added to all JUCE public class declarations. */
#endif

// Defining some helpful macros related to processor intrinsics.

#if JUCE_MINGW && ! defined(__SSE2__)
#define JUCE_USE_SSE_INTRINSICS 0
#endif

#ifndef JUCE_USE_SSE_INTRINSICS
#define JUCE_USE_SSE_INTRINSICS 1
#endif

#if ! JUCE_INTEL
#undef JUCE_USE_SSE_INTRINSICS
#endif

#ifndef JUCE_USE_VDSP_FRAMEWORK
#define JUCE_USE_VDSP_FRAMEWORK 1
#endif

#if __ARM_NEON__ && ! (JUCE_USE_VDSP_FRAMEWORK || defined(JUCE_USE_ARM_NEON))
#define JUCE_USE_ARM_NEON 1
#endif

#if TARGET_IPHONE_SIMULATOR
#ifdef JUCE_USE_ARM_NEON
#undef JUCE_USE_ARM_NEON
#endif
#define JUCE_USE_ARM_NEON 0
#endif

#if JUCE_DEBUG || DOXYGEN
/** A platform-independent way of forcing an inline function.
      Use the syntax: @code
      forcedinline void myfunction (int x)
      @endcode
  */
#define forcedinline inline
#else
#if JUCE_MSVC
#define forcedinline __forceinline
#else
#define forcedinline inline __attribute__ ((always_inline))
#endif
#endif
