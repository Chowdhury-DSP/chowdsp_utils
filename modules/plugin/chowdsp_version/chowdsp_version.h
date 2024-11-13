/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_version
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Plugin Versioning
   description:   Versioning system for ChowDSP plugins
   dependencies:  juce_core

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>

#include "Version/chowdsp_VersionDetail.h"
#include "Version/chowdsp_Version.h"
#include "Version/chowdsp_VersionComparisons.h"

namespace chowdsp
{

/** Tools for working with software versioning. */
namespace VersionUtils
{
    /** Utility class to manage version strings. */
    using Version = ::chowdsp::Version;
} // namespace VersionUtils
} // namespace chowdsp
