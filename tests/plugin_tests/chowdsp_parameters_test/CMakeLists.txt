add_executable(chowdsp_parameters_test)
setup_juce_test(chowdsp_parameters_test)

target_link_libraries(chowdsp_parameters_test PRIVATE
    chowdsp_parameters
    chowdsp_plugin_base
    chowdsp_rhythm
)

target_sources(chowdsp_parameters_test PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/test_utils/TestRunner.cpp

    ForwardingParameterTest.cpp
    ParamHelpersTest.cpp
    ParamStringsTest.cpp
    RhythmParameterTest.cpp
)
