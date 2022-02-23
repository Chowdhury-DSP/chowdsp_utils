#include "chowdsp_gui.h"

#include "Assets/chowdsp_BinaryData.cpp"
#include "LookAndFeel/chowdsp_ChowLNF.cpp"

#include "Helpers/chowdsp_LongPressActionHelper.cpp"
#include "Helpers/chowdsp_PopupMenuHelper.cpp"
#include "Helpers/chowdsp_OpenGLHelper.cpp"

#include "InfoUtils/chowdsp_InfoProvider.cpp"

#include "PluginComponents/chowdsp_OversamplingMenu.cpp"
#include "PluginComponents/chowdsp_TitleComp.cpp"
#include "PluginComponents/chowdsp_TooltipComp.cpp"

#if CHOWDSP_NEEDS_PRESETS_COMPONENT
#include "Presets/chowdsp_PresetsComp.cpp"
#endif
