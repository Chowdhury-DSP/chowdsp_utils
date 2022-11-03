/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_foleys
   vendor:        Chowdhury DSP
   version:       1.3.0
   name:          ChowDSP Foley's GUI Magic
   description:   Interface between chowdsp_gui and foleys_gui_magic
   dependencies:  juce_core, foleys_gui_magic, chowdsp_gui

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       GPLv3

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

// JUCE includes
#include <juce_core/juce_core.h>

// third-party includes
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-function") // GCC doesn't like Foley's static functions
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4458) // MSVC doesn't like Foley's hiding class members
#include <foleys_gui_magic/foleys_gui_magic.h>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

#include <chowdsp_gui/chowdsp_gui.h>

#if FOLEYS_ENABLE_BINARY_DATA
#include <BinaryData.h>
#endif

// LCOV_EXCL_START
#include "GuiItems/chowdsp_InfoItem.h"
#include "GuiItems/chowdsp_TitleItem.h"
#include "GuiItems/chowdsp_TooltipItem.h"

#if JUCE_MODULE_AVAILABLE_chowdsp_dsp_utils
#include "GuiItems/chowdsp_OversamplingMenuItem.h"
#endif // JUCE_MODULE_AVAILABLE_chowdsp_dsp_utils

#if JUCE_MODULE_AVAILABLE_chowdsp_presets
#include "GuiItems/chowdsp_PresetsItem.h"
#endif
// LCOV_EXCL_STOP

#include "chowdsp_MagicGUI.h"
