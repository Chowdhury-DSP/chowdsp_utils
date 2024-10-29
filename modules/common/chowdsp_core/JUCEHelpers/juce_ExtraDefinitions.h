#pragma once

#include <cassert>

/**
 * This file contains custom overrides of some JUCE macros.
 */

// @TODO: figure out a way to re-implement jassert...
#if CHOWDSP_JASSERT_IS_CASSERT
#define jassert(expression) assert (expression)
#else
#define jassert(expression)
#endif
#define jassertfalse
#define jassertquiet(expression)

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

//==============================================================================
/** This is a shorthand macro for deleting a class's copy constructor and
    copy assignment operator.

For example, instead of
    @code
    class MyClass
{
    etc..

        private:
        MyClass (const MyClass&);
    MyClass& operator= (const MyClass&);
};@endcode

    ..you can just write:

    @code
    class MyClass
{
    etc..

        private:
        JUCE_DECLARE_NON_COPYABLE (MyClass)
};@endcode
        */
#define JUCE_DECLARE_NON_COPYABLE(className) \
    className (const className&) = delete;   \
    className& operator= (const className&) = delete;

/** This is a shorthand macro for deleting a class's move constructor and
    move assignment operator.
*/
#define JUCE_DECLARE_NON_MOVEABLE(className) \
    className (className&&) = delete;        \
    className& operator= (className&&) = delete;

/** This is a shorthand way of writing both a JUCE_DECLARE_NON_COPYABLE and
    JUCE_LEAK_DETECTOR macro for a class. @TODO: implement leak detector?
*/
#define JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(className) \
    JUCE_DECLARE_NON_COPYABLE (className)

/** This macro can be added to class definitions to disable the use of new/delete to
    allocate the object on the heap, forcing it to only be used as a stack or member variable.
*/
#define JUCE_PREVENT_HEAP_ALLOCATION             \
private:                                         \
    static void* operator new (size_t) = delete; \
    static void operator delete (void*) = delete;

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
