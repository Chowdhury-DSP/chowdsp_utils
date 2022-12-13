setup_catch_test(chowdsp_modal_dsp_test MODULES chowdsp_modal_dsp)

target_sources(chowdsp_modal_dsp_test
    PRIVATE
        ModalFilterTest.cpp
        ModalFilterBankTest.cpp
)
