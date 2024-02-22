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
    chowdsp::chowdsp_core
    chowdsp::chowdsp_data_structures
    chowdsp::chowdsp_json
    chowdsp::chowdsp_listeners
    chowdsp::chowdsp_reflection
    chowdsp::chowdsp_serialization
    chowdsp::chowdsp_logging
    chowdsp::chowdsp_fuzzy_search
    chowdsp::chowdsp_units
    chowdsp::chowdsp_buffers
    chowdsp::chowdsp_compressor
    chowdsp::chowdsp_dsp_data_structures
    chowdsp::chowdsp_dsp_utils
    chowdsp::chowdsp_eq
    chowdsp::chowdsp_filters
    chowdsp::chowdsp_math
    chowdsp::chowdsp_modal_dsp
    chowdsp::chowdsp_reverb
    chowdsp::chowdsp_simd
    chowdsp::chowdsp_sources
    chowdsp::chowdsp_waveshapers
    chowdsp::chowdsp_gui
    chowdsp::chowdsp_visualizers
    chowdsp::chowdsp_rhythm
    chowdsp::chowdsp_parameters
    chowdsp::chowdsp_plugin_base
    chowdsp::chowdsp_plugin_state
    chowdsp::chowdsp_plugin_utils
    chowdsp::chowdsp_presets_v2
    chowdsp::chowdsp_version
    chowdsp::chowdsp_clap_extensions
    clap_juce_extensions
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
