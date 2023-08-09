setup_catch_lib_test(chowdsp_data_structures_test common_tests_lib)

target_sources(chowdsp_data_structures_test
    PRIVATE
        AbstractTreeTest.cpp
        BucketArrayTest.cpp
        DoubleBufferTest.cpp
        IteratorsTest.cpp
        LocalPointerTest.cpp
        OptionalPointerTest.cpp
        SmallVectorTest.cpp
        StackAllocatorTest.cpp
        StringLiteralTest.cpp
        TupleHelpersTest.cpp
        VectorHelpersTest.cpp
)
