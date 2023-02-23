# setup_catch_test_base(<target-name>)
#
# Configures a Catch unit test
function(setup_catch_test_base target)
    target_sources(${target} PRIVATE ${CMAKE_SOURCE_DIR}/tests/test_utils/CatchTestRunner.cpp)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/tests/test_utils)
    target_link_libraries(${target}
        PRIVATE
            Catch2::Catch2WithMain
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )

    add_custom_command(TARGET ${target}
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E echo "Copying $<TARGET_FILE:${target}> to test-binary"
        COMMAND ${CMAKE_COMMAND} -E make_directory test-binary
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${target}>" test-binary
    )

    if(NOT (CMAKE_GENERATOR STREQUAL Xcode))
        catch_discover_tests(${target}
            TEST_PREFIX ${target}_
        )
    endif()
endfunction(setup_catch_test_base)

# setup_catch_lib_test(<target-name> <library-name>)
#
# Configures a Catch unit test
function(setup_catch_lib_test target library)
    add_executable(${target})

    target_link_libraries(${target} PRIVATE ${library})

    # set coverage flags if needed
    if(CODE_COVERAGE)
        enable_coverage_flags(${target})
        enable_coverage_flags(${library})
    endif()

    setup_catch_test_base(${target})
endfunction(setup_catch_lib_test)

# setup_catch_test(<target-name>)
#
# Configures a Catch unit test from chowdsp modules only
# TODO: Switch to using setup_catch_lib_test with shared static library (chowdsp_rhythm_test)
function(setup_catch_test target)
    setup_chowdsp_lib(${target}_lib ${ARGV})
    setup_catch_lib_test(${target} ${target}_lib)
endfunction(setup_catch_test)

# setup_juce_lib(<library-name> <module1> <module2> ...)
#
# Configures a static library from a set of JUCE modules
function(setup_juce_lib library modules)
    add_library(${library} STATIC)

    list(REMOVE_AT ARGV 0) # Remove "library" argument
    target_link_libraries(${library} PRIVATE ${ARGV})

    target_compile_definitions(${library}
        PUBLIC
            JUCE_USE_CURL=0
            JUCE_WEB_BROWSER=0
            JUCE_MODAL_LOOPS_PERMITTED=1
            JUCE_STANDALONE_APPLICATION=1
        INTERFACE
            $<TARGET_PROPERTY:${library},COMPILE_DEFINITIONS>
    )

    target_include_directories(${library}
        INTERFACE
            $<TARGET_PROPERTY:${library},INCLUDE_DIRECTORIES>
    )
endfunction(setup_juce_lib)
