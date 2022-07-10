setup_catch_test(chowdsp_filters_test
    MODULES chowdsp_core chowdsp_simd chowdsp_math chowdsp_dsp_data_structures chowdsp_filters
)

target_sources(chowdsp_filters_test
    PRIVATE
        ConformalMapsTest.cpp
        FirstOrderFiltersTest.cpp
        ShelfFilterTest.cpp
        SecondOrderFiltersTest.cpp
        StateVariableFilterTest.cpp
        ModFilterWrapperTest.cpp
        FilterChainTest.cpp
        NthOrderFilterTest.cpp
        ButterworthFilterTest.cpp
        ChebyshevIIFilterTest.cpp
        EllipticFilterTest.cpp
        FractionalOrderFilterTest.cpp
        HilbertFilterTest.cpp # needs chowdsp_dsp_utils (remove JUCE dep once chowdsp::DelayLine has been de-juced)
)

target_link_libraries(chowdsp_filters_test_lib
    PUBLIC
        chowdsp::chowdsp_dsp_utils
)
