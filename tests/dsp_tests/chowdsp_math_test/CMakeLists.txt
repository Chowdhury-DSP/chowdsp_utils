setup_catch_test(chowdsp_math_test)

setup_chowdsp_lib(chowdsp_math_test_lib MODULES chowdsp_core chowdsp_simd chowdsp_math)
target_link_libraries(chowdsp_math_test PRIVATE chowdsp_math_test_lib)

target_sources(chowdsp_math_test
    PRIVATE
        CombinatoricsTest.cpp
        FloatVectorOperationsTest.cpp
        MatrixOpsTest.cpp
        PolynomialsTest.cpp
        PowerTest.cpp
        OtherMathOpsTest.cpp
        JacobiEllipticTest.cpp
        PolylogarithmTest.cpp
)
