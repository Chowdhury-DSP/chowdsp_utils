add_executable(chowdsp_dsp_data_structures_test)
setup_juce_test(chowdsp_dsp_data_structures_test)

target_link_libraries(chowdsp_dsp_data_structures_test PRIVATE
    juce::juce_dsp
    chowdsp_dsp_data_structures
    chowdsp_plugin_base
)

target_sources(chowdsp_dsp_data_structures_test PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/test_utils/TestRunner.cpp

    COLAProcessorTest.cpp
    LookupTableTest.cpp
    RebufferProcessorTest.cpp
    SmoothedBufferValueTest.cpp
    ScopedValueTest.cpp
)
