setup_catch_test(chowdsp_dsp_data_structures_test MODULES chowdsp_dsp_data_structures)

target_sources(chowdsp_dsp_data_structures_test
    PRIVATE
        BufferTest.cpp
        BufferViewTest.cpp
        LookupTableTest.cpp
        RebufferProcessorTest.cpp
        SmoothedBufferValueTest.cpp
        ScopedValueTest.cpp
        SIMDBufferCopyTest.cpp
)
