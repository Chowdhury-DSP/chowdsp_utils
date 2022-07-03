add_executable(chowdsp_foleys_test)
setup_juce_test(chowdsp_foleys_test)

target_link_libraries(chowdsp_foleys_test PRIVATE
    foleys_gui_magic
    chowdsp_foleys
    chowdsp_plugin_base
    chowdsp_presets
)

target_sources(chowdsp_foleys_test PRIVATE
    FoleysTest.cpp
)
