add_executable(chowdsp_math_test)
setup_juce_test(chowdsp_math_test)

target_link_libraries(chowdsp_math_test PRIVATE
    juce::juce_dsp
    chowdsp_math
)

target_sources(chowdsp_math_test PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/test_utils/TestRunner.cpp

    CombinatoricsTest.cpp
    FloatVectorOperationsTest.cpp
    MatrixOpsTest.cpp
    PolynomialsTest.cpp
    PowerTest.cpp
    OtherMathOpsTest.cpp
    JacobiEllipticTest.cpp
)
