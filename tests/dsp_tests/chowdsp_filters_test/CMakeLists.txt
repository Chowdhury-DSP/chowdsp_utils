setup_catch_test(chowdsp_filters_test MODULES chowdsp_filters)

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
        HilbertFilterTest.cpp
        WernerFilterTest.cpp
        ButterQsTest.cpp
        LinearTransformsTest.cpp
)
