setup_catch_lib_test(chowdsp_buffers_test dsp_juce_tests_lib)

target_sources(chowdsp_buffers_test
    PRIVATE
        # Don't need JUCE
        BufferTest.cpp
        BufferSpanTest.cpp
        BufferViewTest.cpp

        # Needs JUCE
        SIMDBufferCopyTest.cpp
        JUCEBufferViewTest.cpp
        BufferConversionTest.cpp
        BufferIteratorsTest.cpp
)
