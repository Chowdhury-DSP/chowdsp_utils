cmake_minimum_required(VERSION 3.14)

project(types_list VERSION "0.1.0" LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(IS_TOPLEVEL_PROJECT TRUE)
else()
    set(IS_TOPLEVEL_PROJECT FALSE)
endif()

option(TYPESLIST_OPT_BUILD_EXAMPLES "Build types_list examples" ${IS_TOPLEVEL_PROJECT})
option(TYPESLIST_OPT_BUILD_TESTS "Build and perform types_list tests" ${IS_TOPLEVEL_PROJECT})

if(TYPESLIST_OPT_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(TYPESLIST_OPT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
