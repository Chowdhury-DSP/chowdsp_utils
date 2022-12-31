# add_diagnostic_info(<target>)
#
# Adds diagnostic definitions to a target.
function(add_diagnostic_info target)
    find_package(Git)

    if (Git_FOUND)
        execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_BRANCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_COMMIT_HASH
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif ()

    if ("${GIT_BRANCH}" STREQUAL "")
        message(WARNING "Could not determine Git branch, using placeholder.")
        set(GIT_BRANCH "git-no-branch")
    endif ()
    if ("${GIT_COMMIT_HASH}" STREQUAL "")
        message(WARNING "Could not determine Git commit hash, using placeholder.")
        set(GIT_COMMIT_HASH "git-no-commit")
    endif ()

    cmake_host_system_information(RESULT PROJECT_BUILD_FQDN QUERY FQDN)

    message(STATUS "Setting Git branch: ${GIT_BRANCH}")
    message(STATUS "Setting Git commit hash: ${GIT_COMMIT_HASH}")
    message(STATUS "Setting build FQDN: ${PROJECT_BUILD_FQDN}")
    message(STATUS "Setting compiler ID: ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "Setting compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

    target_compile_definitions(${target}
        PUBLIC
            CHOWDSP_PLUGIN_GIT_BRANCH="${GIT_BRANCH}"
            CHOWDSP_PLUGIN_GIT_COMMIT_HASH="${GIT_COMMIT_HASH}"
            CHOWDSP_PLUGIN_BUILD_FQDN="${PROJECT_BUILD_FQDN}"
            CHOWDSP_PLUGIN_CXX_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}"
            CHOWDSP_PLUGIN_CXX_COMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}"
    )
endfunction()
