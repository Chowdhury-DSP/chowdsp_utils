# setup_benchmark(<target-name> <file-name>)
#
# Sets up a minimal benchmarking app
function(setup_benchmark target file)
    set(extra_modules "${ARGV}")
    list(REMOVE_AT extra_modules 0 1)
    message(STATUS "Configuring Benchmark ${target}, with modules: ${extra_modules}")

    add_executable(${target})
    target_sources(${target} PRIVATE ${file})

    target_compile_definitions(${target} PRIVATE
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        JUCE_STANDALONE_APPLICATION=1
    )

    target_link_libraries(${target} PRIVATE
        ${extra_modules}
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
                target_link_libraries(${target} PUBLIC "-framework Accelerate")
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
