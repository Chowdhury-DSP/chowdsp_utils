/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_clap_extensions
   vendor:        Chowdhury DSP
   version:       1.3.0
   name:          ChowDSP CLAP Extensions
   description:   Extensions for building CLAP plugins
   dependencies:  juce_core, juce_audio_processors

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#if ! HAS_CLAP_JUCE_EXTENSIONS
static_assert (false, "clap-juce-extensions must be linked to be able to use this module!");
#endif

// JUCE includes
#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

// third-party includes
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-parameter", "-Wextra-semi", "-Wnon-virtual-dtor")
#include <clap-juce-extensions/clap-juce-extensions.h>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

namespace chowdsp
{
/** Extensions for creating CLAP plugins */
namespace CLAPExtensions
{
}
} // namespace chowdsp

// LCOV_EXCL_START
#include "ParameterExtensions/chowdsp_ModParamMixin.h"
#include "PluginExtensions/chowdsp_CLAPInfoExtensions.h"
// LCOV_EXCL_END
