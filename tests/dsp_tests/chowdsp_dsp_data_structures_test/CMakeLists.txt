add_executable(chowdsp_dsp_data_structures_test)
setup_juce_test(chowdsp_dsp_data_structures_test)

target_link_libraries(chowdsp_dsp_data_structures_test
    PRIVATE
        juce_dsp
        chowdsp_dsp_data_structures
        chowdsp_plugin_base
)

target_sources(chowdsp_dsp_data_structures_test
    PRIVATE
        COLAProcessorTest.cpp
        LookupTableTest.cpp
        RebufferProcessorTest.cpp
        SmoothedBufferValueTestJUCE.cpp
)

# Testing without JUCE
setup_catch_test(chowdsp_dsp_data_structures_test_no_juce
    MODULES chowdsp_core chowdsp_simd chowdsp_math chowdsp_dsp_data_structures
)

target_sources(chowdsp_dsp_data_structures_test_no_juce
    PRIVATE
        BufferTest.cpp
        SmoothedBufferValueTest.cpp
        ScopedValueTest.cpp
)
