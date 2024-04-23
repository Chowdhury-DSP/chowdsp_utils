setup_catch_lib_test(chowdsp_math_test dsp_tests_lib)

target_sources(chowdsp_math_test
    PRIVATE
        CombinatoricsTest.cpp
        FloatVectorOperationsTest.cpp
        MatrixOpsTest.cpp
        PolynomialsTest.cpp
        ChebyshevPolynomialTest.cpp
        PowerTest.cpp
        OtherMathOpsTest.cpp
        JacobiEllipticTest.cpp
        PolylogarithmTest.cpp
        RatioTest.cpp
        LogApproxTest.cpp
        PowApproxTest.cpp
        DecibelsApproxTest.cpp
        TrigApproxTest.cpp
        RandomFloatTest.cpp
)
