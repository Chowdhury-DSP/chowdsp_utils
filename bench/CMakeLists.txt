list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(CPM)
include(SubprojectVersion)
include(SetupBenchmark)

message(STATUS "Configuring benchmarks for chowdsp_utils")

# set up JUCE
add_subdirectory(${CMAKE_SOURCE_DIR}/../../JUCE JUCE)
subproject_version(JUCE juce_version)
message(STATUS "VERSION for JUCE: ${juce_version}")

juce_add_modules(${CMAKE_SOURCE_DIR}/modules/chowdsp_dsp)

# download google benchmark
CPMAddPackage(
  NAME benchmark
  GITHUB_REPOSITORY google/benchmark
  VERSION 1.5.2
  OPTIONS "BENCHMARK_ENABLE_TESTING Off"
)

setup_benchmark(FloatVectorOperationsBench FloatVectorOperationsBench.cpp)
