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

    # Link with xsimd (if path is defined)
    if (XSIMD_PATH)
        message(STATUS "Using XSIMD")
        target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/${XSIMD_PATH}/include)
        target_compile_definitions(${target} PRIVATE CHOWDSP_USE_XSIMD=1)
    endif ()

    add_custom_command(TARGET ${target}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E echo "copying $<TARGET_FILE:${target}> to ${PROJECT_BINARY_DIR}/${target}"
                       COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}> ${PROJECT_BINARY_DIR}/${target})
endfunction(setup_benchmark)
