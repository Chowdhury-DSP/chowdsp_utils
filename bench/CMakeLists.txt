include(CPM)
include(SubprojectVersion)
include(SetupBenchmark)

message(STATUS "Configuring benchmarks for chowdsp_utils")

# set up JUCE
set(JUCE_MODULES_ONLY ON CACHE INTERNAL "Only configure the JUCE modules")
add_subdirectory(${CMAKE_SOURCE_DIR}/../../JUCE JUCE)
subproject_version(JUCE juce_version)
message(STATUS "VERSION for JUCE: ${juce_version}")

include(AddJUCEModules)

# download google benchmark
CPMAddPackage(
  NAME benchmark
  GITHUB_REPOSITORY google/benchmark
  VERSION 1.5.2
  OPTIONS "BENCHMARK_ENABLE_TESTING Off"
)

setup_benchmark(FloatVectorOperationsBench FloatVectorOperationsBench.cpp chowdsp_math juce_dsp)
setup_benchmark(LookupTableBench LookupTableBench.cpp chowdsp_dsp_data_structures juce_dsp)
setup_benchmark(PowerBench PowerBench.cpp chowdsp_math juce_dsp)
setup_benchmark(FIRFilterBench FIRFilterBench.cpp chowdsp_filters juce_dsp)
setup_benchmark(IIRFilterBench IIRFilterBench.cpp chowdsp_filters juce_dsp)
setup_benchmark(PolynomialBench PolynomialBench.cpp chowdsp_math juce_dsp)
setup_benchmark(MatrixOpsBench MatrixOpsBench.cpp chowdsp_math juce_dsp)
setup_benchmark(BufferBench BufferBench.cpp chowdsp_filters juce_dsp)
setup_benchmark(DecibelsBench DecibelsBench.cpp chowdsp_math juce_audio_basics)
setup_benchmark(AbstractTreeBench AbstractTreeBench.cpp chowdsp_data_structures)
setup_benchmark(TrigBench TrigBench.cpp chowdsp_math juce_dsp)
#setup_benchmark(ConcurrentScanningBench ConcurrentScanningBench.cpp chowdsp_data_structures)
