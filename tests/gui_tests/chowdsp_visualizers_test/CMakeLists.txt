add_executable(chowdsp_visualizers_test)
setup_juce_test(chowdsp_visualizers_test)

target_link_libraries(chowdsp_visualizers_test PRIVATE
    juce::juce_dsp
    chowdsp_visualizers
    chowdsp_eq
    chowdsp_plugin_state
)

target_sources(chowdsp_visualizers_test PRIVATE
    SpectrumPlotBaseTest.cpp
    EQFilterPlotsTest.cpp
    EqualizerPlotTest.cpp
)
