cmake_minimum_required(VERSION 3.15)
project(chowdsp_utils VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
include(CTest)

# set up JUCE
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../JUCE JUCE)
juce_add_modules(../foleys_gui_magic)
juce_add_modules(.)

# set up test executables
juce_add_console_app(chowdsp_utils_tests)
juce_generate_juce_header(chowdsp_utils_tests)

juce_add_gui_app(chowdsp_gui_test)
juce_generate_juce_header(chowdsp_gui_test)

add_executable(wdf_test)
add_subdirectory(tests)

# configure chowdsp_utils_test
target_compile_definitions(chowdsp_utils_tests PRIVATE
    CHOWDSP_UNIT_TESTS=1
    JucePlugin_Name="DummyPlugin"
    JucePlugin_Manufacturer="chowdsp"
    JucePlugin_VersionString="9.9.9"
    JUCE_UNIT_TESTS=0
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
)

target_link_libraries(chowdsp_utils_tests PRIVATE
    chowdsp_utils
    foleys_gui_magic
    UnitTestData
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

target_include_directories(chowdsp_utils_tests PUBLIC tests)

add_test(NAME chowdsp_utils_tests COMMAND chowdsp_utils_tests)

# configure chowdsp_gui_test
target_compile_definitions(chowdsp_gui_test PRIVATE
    CHOWDSP_UNIT_TESTS=1
    JucePlugin_Name="DummyPlugin"
    JucePlugin_Manufacturer="chowdsp"
    JucePlugin_VersionString="9.9.9"
    JUCE_UNIT_TESTS=0
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
)

target_link_libraries(chowdsp_gui_test PRIVATE chowdsp_utils foleys_gui_magic)

add_test(NAME chowdsp_gui_test COMMAND chowdsp_gui_test)

# configure wdf standalone test
target_include_directories(wdf_test PUBLIC DSP/WDF)
add_test(NAME wdf_test COMMAND wdf_test)

# Test with libsamplerate (if available)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(samplerate)
if(SAMPLERATE_FOUND)
    message(STATUS "libsamplerate found")

    target_compile_definitions(chowdsp_utils_tests PRIVATE
        CHOWDSP_USE_LIBSAMPLERATE=1
    )

    target_link_libraries(chowdsp_utils_tests PRIVATE
        samplerate::samplerate
    )
endif()

# Test with xsimd (if path is defined)
if(XSIMD_PATH)
    message(STATUS "Using XSIMD")
    target_include_directories(chowdsp_utils_tests PRIVATE ${XSIMD_PATH}/include)
    target_compile_definitions(chowdsp_utils_tests PRIVATE CHOWDSP_USE_XSIMD=1)
endif()

# supress warning from Foley's
if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
    target_compile_options(chowdsp_utils_tests PUBLIC /wd4458)
    target_compile_options(chowdsp_gui_test PUBLIC /wd4458)
endif()

option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Add required flags (GCC & LLVM/Clang)
    message(STATUS "Appending code coverage compiler flags: -O0 -g --coverage")
    target_compile_options(chowdsp_utils_tests PUBLIC
        -O0        # no optimization
        -g         # generate debug info
        --coverage # sets all required flags
    )

    target_compile_options(chowdsp_gui_test PUBLIC
        -O0        # no optimization
        -g         # generate debug info
        --coverage # sets all required flags
    )

    target_compile_options(wdf_test PUBLIC
        -O0        # no optimization
        -g         # generate debug info
        --coverage # sets all required flags
    )

    target_link_options(chowdsp_utils_tests PUBLIC --coverage)
    target_link_options(chowdsp_gui_test PUBLIC --coverage)
    target_link_options(wdf_test PUBLIC --coverage)
endif(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
