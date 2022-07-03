setup_catch_test(chowdsp_core_test)

target_sources(chowdsp_core_test PRIVATE
    DoubleBufferTest.cpp
    AtomicHelpersTest.cpp
    MemoryAliasingTest.cpp

#    ${CHOWDSP_MODULES_DIR}/common/chowdsp_core/chowdsp_core.cpp
)
