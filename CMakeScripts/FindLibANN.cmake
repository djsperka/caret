# - Try to find libann
# Once done this will define
#  LIBANN_FOUND - System has libann
#  LIBANN_INCLUDE_DIRS - The libann include directories
#  LIBANN_LIBRARIES - The libraries needed to use libann

find_path(LIBANN_INCLUDE_DIR ANN/ANN.h
          HINTS ENV ANN_INCLUDE_DIR)

find_library(LIBANN_LIBRARY NAMES ann
             HINTS ENV ANN_LIB_DIR )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibAnn DEFAULT_MSG LIBANN_LIBRARY LIBANN_INCLUDE_DIR)

mark_as_advanced(LIBANN_INCLUDE_DIR LIBANN_LIBRARY )
set(LIBANN_INCLUDE_DIRS ${LIBANN_INCLUDE_DIR})
set(LIBANN_LIBRARIES ${LIBANN_LIBRARY})
