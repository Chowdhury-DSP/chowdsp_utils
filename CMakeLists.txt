cmake_minimum_required(VERSION 3.15)
project(chowdsp_utils VERSION 2.3.0)
set(CMAKE_CXX_STANDARD 17)
include(CTest)

option(CHOWDSP_ENABLE_TESTING "Enable testing for chowdsp modules" OFF)
option(CHOWDSP_ENABLE_BENCHMARKS "Enable benchmarks for chowdsp modules" OFF)
option(CHOWDSP_ENABLE_EXAMPLES "Enable examples for chowdsp modules" OFF)

if(NOT (CHOWDSP_ENABLE_TESTING OR CHOWDSP_ENABLE_BENCHMARKS OR CHOWDSP_ENABLE_EXAMPLES))
    # Add CMake utils
    list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
    include(AddDiagnosticInfo)
    include(SubprojectVersion)

    if("${JUCE_MODULES_DIR}" STREQUAL "")
        message(STATUS "Using ChowDSP modules outside of JUCE...")
        include(SetupChowdspLib)
    else()
        message(STATUS "Adding ChowDSP JUCE modules...")
        include(AddJUCEModules)
    endif()
else()
    list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
    list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/test)

    # Useful for testing with address sanitizer:
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")

    if(CHOWDSP_ENABLE_TESTING)
        enable_testing()
        add_subdirectory(tests)
    elseif(CHOWDSP_ENABLE_BENCHMARKS)
        add_subdirectory(bench)
    elseif(CHOWDSP_ENABLE_EXAMPLES)
        add_subdirectory(examples)
    endif()
endif()
