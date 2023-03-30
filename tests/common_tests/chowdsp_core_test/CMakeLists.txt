setup_catch_lib_test(chowdsp_core_test common_tests_lib)

target_sources(chowdsp_core_test
    PRIVATE
        DoubleBufferTest.cpp
        AtomicHelpersTest.cpp
        MemoryAliasingTest.cpp
        TupleHelpersTest.cpp
        TypesListTest.cpp
        BindingsTest.cpp
        IteratorsTest.cpp
        OptionalPointerTest.cpp
        LocalPointerTest.cpp
        StringLiteralTest.cpp
        VectorHelpersTest.cpp
)

target_compile_features(chowdsp_core_test PRIVATE cxx_std_20)
