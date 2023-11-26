#include "chowdsp_gui.h"

#include "Assets/chowdsp_BinaryData.cpp"
#include "LookAndFeel/chowdsp_ChowLNF.cpp"

#include "Helpers/chowdsp_LongPressActionHelper.cpp"
#include "Helpers/chowdsp_PopupMenuHelper.cpp"
#include "Helpers/chowdsp_OpenGLHelper.cpp"
#include "Helpers/chowdsp_HostContextProvider.cpp"

#include "InfoUtils/chowdsp_InfoProvider.cpp"

#include "PluginComponents/chowdsp_TitleComp.cpp"
#include "PluginComponents/chowdsp_TooltipComp.cpp"

#if JUCE_MODULE_AVAILABLE_juce_dsp
#include "PluginComponents/chowdsp_CPUMeter.cpp"
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
#include "PluginComponents/chowdsp_ParametersView.cpp"
#endif

#if CHOWDSP_USING_JUCE && JUCE_MODULE_AVAILABLE_chowdsp_dsp_utils
#include "PluginComponents/chowdsp_OversamplingMenu.cpp"
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_presets
#include "Presets/chowdsp_PresetsComp.cpp"
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_presets_v2
#include "Presets/chowdsp_PresetsComponent.cpp"
#endif
