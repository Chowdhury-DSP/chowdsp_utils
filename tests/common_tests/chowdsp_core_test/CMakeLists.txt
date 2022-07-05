setup_catch_test(chowdsp_core_test)

setup_chowdsp_lib(chowdsp_core_test_lib MODULES chowdsp_core)
target_link_libraries(chowdsp_core_test PRIVATE chowdsp_core_test_lib)

target_sources(chowdsp_core_test
    PRIVATE
        DoubleBufferTest.cpp
        AtomicHelpersTest.cpp
        MemoryAliasingTest.cpp
)
