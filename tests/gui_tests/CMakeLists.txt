add_subdirectory(chowdsp_gui_test)
add_subdirectory(chowdsp_foleys_test)

if(CHOWDSP_BUILD_LIVE_GUI_TEST)
    add_subdirectory(live_gui_test)
endif()
