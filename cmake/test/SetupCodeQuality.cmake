message(STATUS "Configuring clang-tidy target...")
add_custom_target(chowdsp_utils_clang_tidy
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND clang-tidy-14 -p build "tests/test_utils/CodeQuality.cpp"
)

message(STATUS "Configuring CodeQL target...")
add_library(chowdsp_utils_codeql)
target_sources(chowdsp_utils_codeql PRIVATE test_utils/CodeQuality.cpp)
target_compile_definitions(chowdsp_utils_codeql PRIVATE
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_STANDALONE_APPLICATION=0
    JUCE_MODULE_AVAILABLE_chowdsp_presets=0
)

target_link_libraries(chowdsp_utils_codeql PRIVATE
    chowdsp_core
    chowdsp_data_structures
    chowdsp_json
    chowdsp_listeners
    chowdsp_reflection
    chowdsp_serialization
    chowdsp_units
    chowdsp_buffers
    chowdsp_compressor
    chowdsp_dsp_data_structures
    chowdsp_dsp_utils
    chowdsp_eq
    chowdsp_filters
    chowdsp_math
    chowdsp_modal_dsp
    chowdsp_reverb
    chowdsp_simd
    chowdsp_sources
    chowdsp_waveshapers
    chowdsp_gui
    chowdsp_visualizers
    chowdsp_rhythm
    chowdsp_parameters
    chowdsp_plugin_base
    chowdsp_plugin_state
    chowdsp_plugin_utils
    chowdsp_presets_v2
    chowdsp_version
    chowdsp_clap_extensions
    clap_juce_extensions
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
