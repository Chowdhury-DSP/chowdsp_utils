setup_catch_lib_test(chowdsp_dsp_data_structures_test dsp_tests_lib)

target_sources(chowdsp_dsp_data_structures_test
    PRIVATE
        LookupTableTest.cpp
        RebufferProcessorTest.cpp
        SmoothedBufferValueTest.cpp
        UIToAudioPipelineTest.cpp
        BufferMultipleTest.cpp
)
