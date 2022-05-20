add_executable(chowdsp_filters_test)
setup_juce_test(chowdsp_filters_test)

target_link_libraries(chowdsp_filters_test PRIVATE
    juce::juce_graphics
    juce::juce_dsp
    chowdsp_filters
    chowdsp_dsp_utils
)

target_sources(chowdsp_filters_test PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/test_utils/TestRunner.cpp

    ConformalMapsTest.cpp
    FilterChainTest.cpp
    FirstOrderFiltersTest.cpp
    HilbertFilterTest.cpp
    NthOrderFilterTest.cpp
    SecondOrderFiltersTest.cpp
    ShelfFilterTest.cpp
    ModFilterWrapperTest.cpp
    ButterworthFilterTest.cpp
    ChebyshevIIFilterTest.cpp
    EllipticFilterTest.cpp
    FractionalOrderFilterTest.cpp
    FIRFilterTest.cpp
)
