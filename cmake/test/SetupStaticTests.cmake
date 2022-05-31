set(STATIC_TEST_FILE ${CMAKE_CURRENT_BINARY_DIR}/chowdsp_utils_static_test.cpp)
configure_file(../test_utils/CodeQuality.cpp ${STATIC_TEST_FILE} @ONLY NEWLINE_STYLE LF)

add_executable(static_test_dummy_executable)
target_sources(static_test_dummy_executable PRIVATE ${STATIC_TEST_FILE})
target_compile_definitions(static_test_dummy_executable PRIVATE
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_STANDALONE_APPLICATION=0
)

target_link_libraries(static_test_dummy_executable PRIVATE
    juce::juce_dsp
    chowdsp_core
    chowdsp_json
    chowdsp_reflection
    chowdsp_serialization
    chowdsp_dsp_data_structures
    chowdsp_dsp_utils
    chowdsp_eq
    chowdsp_filters
    chowdsp_math
    chowdsp_reverb
    chowdsp_simd
    chowdsp_gui
    chowdsp_rhythm
    chowdsp_parameters
    chowdsp_plugin_base
    chowdsp_plugin_utils
    chowdsp_presets
    chowdsp_version
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

add_custom_target(chowdsp_utils_static_tests)

function(create_static_test name)
    add_custom_target(${name}
        DEPENDS ${name}-cmd
    )

    add_custom_command(
        OUTPUT ${name}-cmd
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND "${CMAKE_COMMAND}" -E echo "Running static test: ${name}"
    )

    add_dependencies(chowdsp_utils_static_tests ${name})
endfunction(create_static_test name)

function(add_static_test_check target file_to_check should_pass)
    set(file_path ${CMAKE_CURRENT_SOURCE_DIR}/${file_to_check})
    add_custom_command(
        OUTPUT ${target}-cmd APPEND
        COMMAND "${CMAKE_COMMAND}" -E echo "Compiling ${file_path}"
        COMMAND "${CMAKE_COMMAND}" -E copy ${file_path} ${STATIC_TEST_FILE}
    )
    if(should_pass)
        add_custom_command(
            OUTPUT ${target}-cmd APPEND
            COMMAND cmake --build build --parallel --target static_test_dummy_executable || ("${CMAKE_COMMAND}" -E echo "Error: Compiler should have succeeded!" && exit 1)
        )
    else()
        add_custom_command(
            OUTPUT ${target}-cmd APPEND
            COMMAND cmake --build build --parallel --target static_test_dummy_executable && ("${CMAKE_COMMAND}" -E echo "Error: Compiler should have failed!" && exit 1) || "${CMAKE_COMMAND}" -E echo "Compiler failed as expected!"
        )
    endif()
endfunction(add_static_test_check)
