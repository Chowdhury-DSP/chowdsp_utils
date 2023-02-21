set(CHOWDSP_MODULES_DIR "${PROJECT_SOURCE_DIR}/modules" CACHE INTERNAL "Source directory for ChowDSP modules")

function(_chowdsp_find_module_dependencies module_header module_deps)
    file(STRINGS "${module_header}" dependencies_raw REGEX "[ \t]*dependencies:")
    string(STRIP ${dependencies_raw} dependencies_raw)
    string(SUBSTRING ${dependencies_raw} 13 -1 dependencies_raw)

    if("${dependencies_raw}" STREQUAL "")
        # No dependencies here! Let's move on...
        set (${module_deps} "" PARENT_SCOPE)
        return()
    endif()

    string(STRIP ${dependencies_raw} dependencies_raw)
    string(REPLACE ", " ";" dependencies_list ${dependencies_raw})
    set (${module_deps} ${dependencies_list} PARENT_SCOPE)
endfunction(_chowdsp_find_module_dependencies)

function(_chowdsp_load_module module)
    # message(STATUS "Loading module: ${module}")

    find_path(chowdsp_module_path
        NAMES "${module}.h"
        PATHS "${CHOWDSP_MODULES_DIR}/common" "${CHOWDSP_MODULES_DIR}/dsp" "${CHOWDSP_MODULES_DIR}/music"
        PATH_SUFFIXES "${module}"
        REQUIRED
    )

    get_filename_component(module_parent_path ${chowdsp_module_path} DIRECTORY)
    target_include_directories(${lib_name} PUBLIC ${module_parent_path})
    target_compile_definitions(${lib_name} PUBLIC JUCE_MODULE_AVAILABLE_${module})

    if(EXISTS "${chowdsp_module_path}/${module}.cpp")
        # message(STATUS "Adding source file: ${chowdsp_module_path}/${module}.cpp")
        target_sources(${lib_name} PRIVATE "${chowdsp_module_path}/${module}.cpp")
    endif()

    # Load any modules that this one depends on:
    _chowdsp_find_module_dependencies(${chowdsp_module_path}/${module}.h module_dependencies)
    unset(chowdsp_module_path CACHE)
    foreach(module_dep IN LISTS module_dependencies)
        get_target_property(_lib_compile_defs ${lib_name} COMPILE_DEFINITIONS)
        if("JUCE_MODULE_AVAILABLE_${module_dep}" IN_LIST _lib_compile_defs)
            continue() # we've already loaded this module, so continue...
        endif()
        _chowdsp_load_module(${module_dep})
    endforeach()
endfunction(_chowdsp_load_module)

function(setup_chowdsp_lib lib_name)
    set(multiValueArgs MODULES)
    cmake_parse_arguments(CHOWDSPLIB "" "" "${multiValueArgs}" ${ARGN})

    message(STATUS "Setting up ChowDSP Static Lib: ${lib_name}, with modules: ${CHOWDSPLIB_MODULES}")
    add_library(${lib_name} STATIC)

    foreach(module IN LISTS CHOWDSPLIB_MODULES)
        _chowdsp_load_module(${module})
    endforeach()

    target_compile_definitions(${lib_name}
        PUBLIC
            $<IF:$<CONFIG:DEBUG>,DEBUG=1 _DEBUG=1,NDEBUG=1 _NDEBUG=1>
    )

    if(APPLE)
        target_link_libraries(${lib_name} PUBLIC "-framework Accelerate")
    elseif(UNIX)
        # We need to link to pthread explicitly on Linux/GCC
        set(THREADS_PREFER_PTHREAD_FLAG ON)
        find_package(Threads REQUIRED)
        target_link_libraries(${lib_name} PUBLIC Threads::Threads)
    endif()

    install(
        TARGETS ${lib_name}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endfunction(setup_chowdsp_lib)
