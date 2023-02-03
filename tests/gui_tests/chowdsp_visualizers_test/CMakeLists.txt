setup_catch_juce_test(chowdsp_visualizers_test
    juce::juce_dsp
    chowdsp_visualizers
    chowdsp_eq
    chowdsp_plugin_state
)

target_sources(chowdsp_visualizers_test PRIVATE
    SpectrumPlotBaseTest.cpp
    EQFilterPlotsTest.cpp
    EqualizerPlotTest.cpp
    GenericFilterPlotTest.cpp
)
