add_executable(chowdsp_parameters_test)
setup_juce_test(chowdsp_parameters_test)

target_link_libraries(chowdsp_parameters_test
    PRIVATE
        juce::juce_dsp
        chowdsp_parameters
        chowdsp_dsp_data_structures
        chowdsp_plugin_base
        chowdsp_rhythm
        clap_juce_extensions
)

target_sources(chowdsp_parameters_test PRIVATE
    ForwardingParameterTest.cpp
    ParamHelpersTest.cpp
    ParamModulationTest.cpp
    ParamStringsTest.cpp
    RhythmParameterTest.cpp
)
