#include "chowdsp_gui.h"

#include "Assets/chowdsp_BinaryData.cpp"
#include "LookAndFeel/chowdsp_ChowLNF.cpp"

#include "Helpers/chowdsp_LongPressActionHelper.cpp"
#include "Helpers/chowdsp_PopupMenuHelper.cpp"
#include "Helpers/chowdsp_OpenGLHelper.cpp"

#include "InfoUtils/chowdsp_InfoProvider.cpp"

#include "PluginComponents/chowdsp_CPUMeter.cpp"
#include "PluginComponents/chowdsp_TitleComp.cpp"
#include "PluginComponents/chowdsp_TooltipComp.cpp"

#if JUCE_MODULE_AVAILABLE_chowdsp_dsp_utils
#include "PluginComponents/chowdsp_OversamplingMenu.cpp"
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_presets
#include "Presets/chowdsp_PresetsComp.cpp"
#endif
