add_executable(chowdsp_gui_test)
setup_juce_test(chowdsp_gui_test)

target_link_libraries(chowdsp_gui_test PRIVATE
    chowdsp_dsp_utils
    chowdsp_gui
    chowdsp_presets
    chowdsp_plugin_base
)

target_sources(chowdsp_gui_test PRIVATE
    LongPressActionTest.cpp
    OpenGLHelperTest.cpp
    OversamplingMenuTest.cpp
    PopupMenuHelperTest.cpp
    PresetsCompTest.cpp
    WindowInPluginTest.cpp
    HostContextProviderTest.cpp
)
