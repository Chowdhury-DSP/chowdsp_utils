juce_add_console_app(chowdsp_utils_tests)
setup_juce_test(chowdsp_utils_tests)

target_sources(chowdsp_utils_tests PRIVATE TestRunner.cpp)
add_subdirectory(dsp_tests)
add_subdirectory(gui_tests)
add_subdirectory(utils_tests)

target_include_directories(chowdsp_utils_tests PUBLIC .)

# Test with OpenGL (if not Apple)
if(NOT APPLE)
    message(STATUS "Linking with OpenGL")
    target_link_libraries(chowdsp_utils_tests PRIVATE juce::juce_opengl)
endif()

# Test with libsamplerate (if available)
find_package(samplerate)
if (SAMPLERATE_FOUND)
    message(STATUS "libsamplerate found")
    target_compile_definitions(chowdsp_utils_tests PRIVATE CHOWDSP_USE_LIBSAMPLERATE=1)
    target_link_libraries(chowdsp_utils_tests PRIVATE samplerate::samplerate)
endif ()

# set coverage flags if needed
if(CODE_COVERAGE)
    enable_coverage_flags(chowdsp_utils_tests)
endif()
