/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_logging
   vendor:        Chowdhury DSP
   version:       2.2.0
   name:          ChowDSP Logging
   description:   ChowDSP Logging module
   dependencies:  chowdsp_core, chowdsp_data_structures, chowdsp_listeners

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-Clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <chowdsp_core/chowdsp_core.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>
#include <chowdsp_listeners/chowdsp_listeners.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum",
                                     "-Wfloat-equal",
                                     "-Wextra-semi",
                                     "-Wzero-length-array",
                                     "-Wc++20-compat",
                                     "-Wlanguage-extension-token")

#include "third_party/spdlog/include/spdlog/spdlog.h"
#include "third_party/spdlog/include/spdlog/sinks/stdout_color_sinks.h"
#include "third_party/spdlog/include/spdlog/sinks/basic_file_sink.h"
#include "third_party/spdlog/include/spdlog/sinks/msvc_sink.h"

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#include "Loggers/chowdsp_BaseLogger.h"
#include "Loggers/chowdsp_LogFileHelpers.h"
#include "Loggers/chowdsp_CrashLogHelpers.h"
#include "Loggers/chowdsp_Logger.h"
#include "Loggers/chowdsp_FormatHelpers.h"

// legacy
#include "Loggers/chowdsp_PluginLogger.h"
