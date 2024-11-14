/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_serialization
   vendor:        Chowdhury DSP
   version:       2.3.0
   name:          ChowDSP Serialization Utilities
   description:   Utility methods for serializing data structures into XML, JSON, or some other format
   dependencies:  juce_core, chowdsp_core, chowdsp_json, chowdsp_reflection

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-Clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// JUCE includes
#include <juce_core/juce_core.h>
#include <chowdsp_core/chowdsp_core.h>
#include <chowdsp_json/chowdsp_json.h>
#include <chowdsp_reflection/chowdsp_reflection.h>

#include "Serialization/chowdsp_BaseSerializer.h"
#include "Serialization/chowdsp_Serialization.h"
#include "Serialization/chowdsp_JSONSerializer.h"
#include "Serialization/chowdsp_XMLSerializer.h"
