# setup_benchmark(<target-name> <file-name>)
#
# Sets up a minimal benchmarking app
function(setup_benchmark target file)
    juce_add_console_app(${target})
    juce_generate_juce_header(${target})

    target_sources(${target} PRIVATE ${file})

    target_compile_definitions(${target} PRIVATE
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_MODAL_LOOPS_PERMITTED=1
    )

    target_link_libraries(${target} PRIVATE
        juce::juce_dsp
        chowdsp_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        benchmark::benchmark
    )

    # Link with xsimd (if path is defined)
    if (XSIMD_PATH)
        message(STATUS "Using XSIMD")
        target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/${XSIMD_PATH}/include)
        target_compile_definitions(${target} PRIVATE CHOWDSP_USE_XSIMD=1)
    endif ()
endfunction(setup_benchmark)
