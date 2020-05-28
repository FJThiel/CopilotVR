# - Try to find the INPUTEMULATOR libraries
# Once done this will define
# INPUTEMULATOR_FOUND - System has found the libraries
# INPUTEMULATOR_INCLUDEDIR - The include directory
# INPUTEMULATOR_DEBUG - The library for the debug configuration
# INPUTEMULATOR_RELEASE - The library for the release configuration

# set(INPUTEMULATOR_ROOT_DIR 
#     "${INPUTEMULATOR_ROOT_DIR}")
#     CACHE
#     PATH
#     "Directory to search for the OpenVRInputEmulator libraries")



find_path(IEMU_INCDIR 
    NAMES vrinputemulator.h
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/OpenVR-InputEmulator/lib_vrinputemulator/include)

# message("include dir = ${IEMU_INCDIR}")

find_path(IEMU_LIBDIR_REL 
    NAMES libvrinputemulator.lib
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/OpenVR-InputEmulator/Release/lib/x64)    
find_library(IEMU_LIB_REL NAMES libvrinputemulator.lib
    HINTS ${IEMU_LIBDIR_REL})

# message("IEMU_Libs libdir release = ${IEMU_LIBDIR_REL}")
# message("IEMU_Libs lib release = ${IEMU_LIB_REL}")

find_path(IEMU_LIBDIR_DEB 
    NAMES libvrinputemulator.lib
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/OpenVR-InputEmulator/Debug/lib/x64)
find_library(IEMU_LIB_DEB NAMES libvrinputemulator.lib
    HINTS ${IEMU_LIBDIR_DEB})


# message("IEMU_Libs libdir debug = ${IEMU_LIBDIR_DEB}")
# message("IEMU_Libs lib debug = ${IEMU_LIB_DEB}")

set(INPUTEMULATOR_RELEASE ${IEMU_LIB_REL})
set(INPUTEMULATOR_DEBUG ${IEMU_LIB_DEB})
set(INPUTEMULATOR_INCLUDEDIR ${IEMU_INCDIR})


include(FindPackageHandleStandardArgs)

# Handle the QUIETLY and REQUIRED arguments and set the INPUTEMULATOR_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(INPUTEMULATOR DEFAULT_MSG
                                  IEMU_LIBDIR_REL IEMU_LIBDIR_DEB IEMU_LIB_REL IEMU_LIB_DEB IEMU_INCDIR)

mark_as_advanced(INPUTEMULATOR_DEBUG INPUTEMULATOR_RELEASE INPUTEMULATOR_INCLUDEDIR)