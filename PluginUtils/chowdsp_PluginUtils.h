#pragma once

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant", // Clang doesn't like HasAddParameters checker
                                     "-Winconsistent-missing-destructor-override")
#include "chowdsp_PluginBase.h"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#include "chowdsp_DummySynthSound.h"
#include "chowdsp_ParamUtils.h"
#include "chowdsp_SynthBase.h"
#include "chowdsp_VersionUtils.h"
