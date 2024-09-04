setup_catch_lib_test(chowdsp_visualizers_test gui_tests_lib)

target_sources(chowdsp_visualizers_test PRIVATE
    SpectrumPlotBaseTest.cpp
    EQFilterPlotsTest.cpp
    EqualizerPlotTest.cpp
    GenericFilterPlotTest.cpp
    WaveshaperPlotTest.cpp
    WaveformViewTest.cpp
    LevelDetectorTest.cpp
    GainComputerPlotTest.cpp
)
