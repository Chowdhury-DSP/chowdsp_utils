add_executable(chowdsp_math_test)
setup_juce_test(chowdsp_math_test)

target_link_libraries(chowdsp_math_test PRIVATE
    chowdsp_math
)

target_sources(chowdsp_math_test PRIVATE
    CombinatoricsTest.cpp
    FloatVectorOperationsTest.cpp
    MatrixOpsTest.cpp
    PolynomialsTest.cpp
    PowerTest.cpp
    OtherMathOpsTest.cpp
    JacobiEllipticTest.cpp
    PolylogarithmTest.cpp
)
