setup_juce_lib(gui_tests_lib
    juce::juce_dsp
    chowdsp::chowdsp_dsp_utils
    chowdsp::chowdsp_gui
    chowdsp::chowdsp_visualizers
    chowdsp::chowdsp_eq
    chowdsp::chowdsp_plugin_state
    chowdsp::chowdsp_plugin_base
)

add_subdirectory(chowdsp_gui_test)
add_subdirectory(chowdsp_foleys_test)
add_subdirectory(chowdsp_visualizers_test)

if(CHOWDSP_BUILD_LIVE_GUI_TEST)
    add_subdirectory(live_gui_test)
endif()
