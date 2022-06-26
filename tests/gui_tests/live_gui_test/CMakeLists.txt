juce_add_gui_app(live_gui_test)
setup_juce_test(live_gui_test)

target_link_libraries(live_gui_test PRIVATE
    chowdsp_gui
    chowdsp_plugin_base
    chowdsp_presets
    chowdsp_plugin_utils
)

target_sources(live_gui_test PRIVATE
    LiveGUITest.cpp
)