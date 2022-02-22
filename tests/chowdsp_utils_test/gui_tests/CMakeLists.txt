setup_juce_unit_test(chowdsp_gui_unit_tests)

# Test with OpenGL (if not Apple)
if(NOT APPLE)
    message(STATUS "Linking with OpenGL")
    target_link_libraries(chowdsp_gui_unit_tests PRIVATE juce::juce_opengl)
endif()

target_sources(chowdsp_gui_unit_tests PRIVATE
    FoleysTest.cpp
    LongPressActionTest.cpp
    OpenGLHelperTest.cpp
    OversamplingMenuTest.cpp
    PopupMenuHelperTest.cpp
    PresetsCompTest.cpp
    WindowInPluginTest.cpp
)
