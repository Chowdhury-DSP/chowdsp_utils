setup_catch_test(chowdsp_math_test)

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

        ${CMAKE_SOURCE_DIR}/modules/common/chowdsp_core/chowdsp_core.cpp
        ${CMAKE_SOURCE_DIR}/modules/dsp/chowdsp_math/chowdsp_math.cpp
)
