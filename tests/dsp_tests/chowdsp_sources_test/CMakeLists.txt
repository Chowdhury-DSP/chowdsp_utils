setup_catch_lib_test(chowdsp_sources_test dsp_tests_lib)

target_sources(chowdsp_sources_test
    PRIVATE
        SineTest.cpp
        SawtoothTest.cpp
        SquareTest.cpp
        TriangleTest.cpp
        PolygonalTest.cpp
        AdditiveOscTest.cpp
)
