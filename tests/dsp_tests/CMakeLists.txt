setup_chowdsp_lib(dsp_tests_lib 
    MODULES
        chowdsp_dsp_data_structures
        chowdsp_filters
        chowdsp_sources
        chowdsp_dsp_utils
        chowdsp_math
        chowdsp_modal_dsp
        chowdsp_simd
        chowdsp_waveshapers
)
# Setting this to 1 can be useful for debugging!
target_compile_definitions(dsp_tests_lib PUBLIC CHOWDSP_JASSERT_IS_CASSERT=0)

setup_juce_lib(dsp_juce_tests_lib
    juce::juce_dsp
    chowdsp::chowdsp_dsp_data_structures
    chowdsp::chowdsp_eq
    chowdsp::chowdsp_reverb
    chowdsp::chowdsp_sources
    chowdsp::chowdsp_plugin_base
)

add_subdirectory(chowdsp_buffers_test)
add_subdirectory(chowdsp_compressor_test)
add_subdirectory(chowdsp_dsp_data_structures_test)
add_subdirectory(chowdsp_dsp_juce_test)
add_subdirectory(chowdsp_dsp_utils_test)
add_subdirectory(chowdsp_filters_test)
add_subdirectory(chowdsp_math_test)
add_subdirectory(chowdsp_modal_dsp_test)
add_subdirectory(chowdsp_simd_test)
add_subdirectory(chowdsp_sources_test)
add_subdirectory(chowdsp_waveshapers_test)
