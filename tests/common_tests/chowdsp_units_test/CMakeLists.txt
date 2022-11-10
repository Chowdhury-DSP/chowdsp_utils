setup_catch_test(chowdsp_units_test MODULES chowdsp_core chowdsp_units)

target_sources(chowdsp_units_test
    PRIVATE
        TimeUnitsTest.cpp
)