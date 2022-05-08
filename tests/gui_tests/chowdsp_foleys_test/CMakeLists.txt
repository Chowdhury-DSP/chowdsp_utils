juce_add_console_app(chowdsp_foleys_test)
setup_juce_test(chowdsp_foleys_test)

target_link_libraries(chowdsp_foleys_test PRIVATE
    juce::juce_dsp
    foleys_gui_magic
    chowdsp_foleys
    chowdsp_plugin_base
    chowdsp_presets
)

target_compile_definitions(chowdsp_foleys_test PUBLIC
    CHOWDSP_NEEDS_PRESETS_COMPONENT=0
)

target_sources(chowdsp_foleys_test PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/test_utils/TestRunner.cpp
    FoleysTest.cpp
)
