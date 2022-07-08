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
)

#add_executable(chowdsp_filters_test)
#setup_juce_test(chowdsp_filters_test)
#
#target_link_libraries(chowdsp_filters_test PRIVATE
#    chowdsp_filters
#    chowdsp_dsp_utils
#)
#
#target_sources(chowdsp_filters_test PRIVATE
#    HilbertFilterTest.cpp
#    FractionalOrderFilterTest.cpp
#    FIRFilterTest.cpp
#)
