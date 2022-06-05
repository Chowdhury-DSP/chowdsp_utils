add_executable(chowdsp_plugin_base_test)
setup_juce_test(chowdsp_plugin_base_test)

target_link_libraries(chowdsp_plugin_base_test PRIVATE
    juce::juce_dsp
    chowdsp_core
    chowdsp_plugin_base
    chowdsp_presets
    chowdsp_foleys
)

target_sources(chowdsp_plugin_base_test PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/test_utils/TestRunner.cpp
    PluginBaseTest.cpp
    PluginDiagnosticInfoTest.cpp
)
