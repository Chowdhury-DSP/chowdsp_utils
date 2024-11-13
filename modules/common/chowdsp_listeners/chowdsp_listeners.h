/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_listeners
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Listener/Broadcaster Utilities
   description:   Wrapper on the single-header rocket library
   dependencies:  chowdsp_core

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-Clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// STL includes
#include <thread>

// JUCE includes
#include <chowdsp_core/chowdsp_core.h>

// third party includes
#define ROCKET_NO_EXCEPTIONS 1
#define ROCKET_NO_TIMERS 1
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4100)
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-parameter",
                                     "-Wunused-but-set-parameter",
                                     "-Wshadow",
                                     "-Wattributes",
                                     "-Wshadow-field-in-constructor",
                                     "-Wshadow-uncaptured-local",
                                     "-Wc++20-extensions")
#include "third_party/rocket.hpp"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

namespace chowdsp
{
// rocket uses names like "signal" and "connection" which are unfortunately easy
// to confuse with other terms in audio programming. So let's set up some aliases
// using the following translations:
// signal = broadcaster
// connection = callback

/** Alias for rocket::signal */
template <typename Signature>
using Broadcaster = rocket::signal<Signature>;

/** Alias for rocket::thread_safe_signal */
template <typename Signature>
using ThreadSafeBroadcaster = rocket::thread_safe_signal<Signature>;

/** Alias for rocket::trackable */
using TrackedByBroadcasters = rocket::trackable;

/** Alias for rocket::scoped_connection */
using Callback = rocket::connection;

/** Alias for rocket::scoped_connection */
using ScopedCallback = rocket::scoped_connection;

/** Alias for rocket::scoped_connection_container */
using ScopedCallbackList = rocket::scoped_connection_container;

/** When accessed from inside a callback, this will return the same callback object. */
inline Callback getCurrentCallback() noexcept
{
    return rocket::current_connection();
}
} // namespace chowdsp
