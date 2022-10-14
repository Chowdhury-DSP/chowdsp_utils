setup_catch_test(chowdsp_math_test MODULES chowdsp_math)

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
        RatioTest.cpp
)
