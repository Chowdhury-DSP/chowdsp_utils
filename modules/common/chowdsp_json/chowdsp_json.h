/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_json
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP JSON Utilities
   description:   JUCE interface for nlohmann::json
   dependencies:  juce_core

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-Clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// third party includes
#include "third_party/nlohmann/json.hpp"

// JUCE includes
#include <juce_core/juce_core.h>

namespace chowdsp
{
/** Alias for nlohmann::json */
using json = nlohmann::json;
} // namespace chowdsp

#include "JSONUtils/chowdsp_StringAdapter.h"

#include "JSONUtils/chowdsp_JSONUtils.h"
