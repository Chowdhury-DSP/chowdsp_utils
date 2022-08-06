setup_catch_test(chowdsp_sources_test MODULES chowdsp_sources)

target_sources(chowdsp_sources_test
    PRIVATE
        SineTest.cpp
        SawtoothTest.cpp
        SquareTest.cpp
        TriangleTest.cpp
        PolygonalTest.cpp
)
