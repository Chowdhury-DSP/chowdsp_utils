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
        JUCE_STANDALONE_APPLICATION=1
    )

    target_link_libraries(${target} PRIVATE
        juce::juce_dsp
        chowdsp_simd
        chowdsp_dsp_data_structures
        chowdsp_math
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        benchmark::benchmark
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            # Any Clang
            if(CMAKE_CXX_COMPILER_ID MATCHES "^AppleClang$")
                # Apple Clang only
            endif()
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "^GNU$")
           # GCC only
           target_compile_options(${target} PUBLIC
               -Wno-pessimizing-move
               -Wno-redundant-decls
           )
        endif()
    endif()

    add_custom_command(TARGET ${target}
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E echo "Copying $<TARGET_FILE:${target}> to bench-binary"
        COMMAND ${CMAKE_COMMAND} -E make_directory bench-binary
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${target}>" bench-binary
    )
endfunction(setup_benchmark)
