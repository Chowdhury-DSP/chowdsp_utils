setup_catch_lib_test(chowdsp_core_test common_tests_lib)

target_sources(chowdsp_core_test
    PRIVATE
        AtomicHelpersTest.cpp
        MemoryAliasingTest.cpp
        TypesListTest.cpp
        TypeCheckersTest.cpp
        TypeTraitsTest.cpp
        BindingsTest.cpp
        ScopedValueTest.cpp
        EndOfScopeActionTest.cpp
)

target_compile_features(chowdsp_core_test PRIVATE cxx_std_20)
