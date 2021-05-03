# - Find samplerate
# Find the native samplerate includes and libraries
#
#  SAMPLERATE_INCLUDE_DIR - where to find samplerate.h, etc.
#  SAMPLERATE_LIBRARIES   - List of libraries when using libsamplerate.
#  SAMPLERATE_FOUND       - True if libsamplerate found.

# Use pkg-config to find library locations in *NIX environments.
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_search_module(PC_SAMPLERATE QUIET samplerate)
endif()

# Find the include directory.
find_path(SAMPLERATE_INCLUDE_DIR "samplerate.h"
    HINTS ${PC_SAMPLERATE_INCLUDE_DIRS})

# Find the version.  I don't know if there is a correct way to find this on
# Windows - the config.h in the tarball is wrong for 0.1.19.
if(PC_SAMPLERATE_VERSION)
    set(SAMPLERATE_VERSION "${PC_SAMPLERATE_VERSION}")
endif()

# Find the library.
find_library(SAMPLERATE_LIBRARY "samplerate"
    HINTS ${PC_SAMPLERATE_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(samplerate
    FOUND_VAR SAMPLERATE_FOUND
    REQUIRED_VARS SAMPLERATE_INCLUDE_DIR SAMPLERATE_LIBRARY
    VERSION_VAR SAMPLERATE_VERSION
)

if(SAMPLERATE_FOUND)
    # Imported target.
    add_library(samplerate::samplerate UNKNOWN IMPORTED)
    set_target_properties(samplerate::samplerate PROPERTIES
        INTERFACE_COMPILE_OPTIONS "${PC_SAMPLERATE_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${SAMPLERATE_INCLUDE_DIR}"
        IMPORTED_LOCATION "${SAMPLERATE_LIBRARY}")
endif()
