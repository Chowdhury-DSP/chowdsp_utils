add_executable(wdf_test wdf_standalone_test.cpp)

# configure wdf standalone test
target_include_directories(wdf_test PUBLIC ${CMAKE_SOURCE_DIR}/modules/chowdsp_dsp/WDF)
add_test(NAME wdf_test COMMAND wdf_test)

# set coverage flags if needed
if(CODE_COVERAGE)
    enable_coverage_flags(wdf_test)
endif()
