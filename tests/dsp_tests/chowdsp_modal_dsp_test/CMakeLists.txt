setup_catch_lib_test(chowdsp_modal_dsp_test dsp_tests_lib)

target_sources(chowdsp_modal_dsp_test
    PRIVATE
        ModalFilterTest.cpp
        ModalFilterBankTest.cpp
)
