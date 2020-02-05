#
# Minimum required version of CMAKE
#
CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

#=================================================================================
#
# Print information about required/optional environment variables
#
MESSAGE("   ")
MESSAGE("REQUIRED environment variables for VTK:")
MESSAGE("   VTK_DIR - The directory containing the VTK include and lib directories:" ${VTK_DIR})
MESSAGE("   ")
MESSAGE("OPTIONAL environment variables for MINC and NETCDF:")
MESSAGE("   MINC_DIR - The directory containing the MINC include and lib directories")
MESSAGE("   NETCDF_DIR - The directory containing the NETCDF include and lib directories")
MESSAGE("   ")
MESSAGE("OPTIONAL environment variables for QWT:")
MESSAGE("   QWT_DIR - The directory containing the QWT include and lib directories")
MESSAGE("   ")
MESSAGE("OPTIONAL environment varables for ZLIB but only needed if the ")
MESSAGE("the include files are not in a default include directory")
MESSAGE("   ZLIB_INC_DIR - The directory containing the ZLIB include files")
MESSAGE("   ZLIB_LIB_DIR - The directory containing the ZLIB library")
MESSAGE("   ")
MESSAGE("For Qt, the Qt program 'qmake' must be in the PATH")
MESSAGE("   ")

#=================================================================================
#
# Add folder for cmake modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeScripts/")


#=================================================================================
#
#
#
#message(INFORMATION "\nTo get the correct version of QT, qmake must be in the PATH\n")

#TSC: use "CACHE <type> <docstring>" syntax in SET commands so they can be overridden by cmake options

#=================================================================================
#
# Setting the compiler MUST be done before the PROJECT
# statement or else an infinite loop will occur indicating
# that the compiler has been redefined.
#
IF(APPLE)
    ADD_DEFINITIONS(-DCARET_OS_MACOSX)
ELSEIF(UNIX)
ELSEIF(WIN32)
    ADD_DEFINITIONS(-DCARET_OS_WINDOWS)
    IF(MSVC)
        ADD_DEFINITIONS(-DCARET_OS_WINDOWS_MSVC)
        IF(CMAKE_CL_64)
            ADD_DEFINITIONS(-D_USE_MATH_DEFINES -wd"4290" -wd"4244" -wd"4267" -wd"4305" -wd"4100" -wd"4005" -MP -DNOMINMAX)
        ELSE()
        ENDIF(CMAKE_CL_64)
    ELSE(MSVC)
    ENDIF(MSVC)
ELSE(APPLE)
    MESSAGE(FATAL_ERROR "Unrecognized operating system " ${CMAKE_SYSTEM_NAME})
ENDIF(APPLE)



#cmake_policy(SET CMP0015 OLD)

#=================================================================================
#
# Shows compilation command when true
#
SET(CMAKE_VERBOSE_MAKEFILE TRUE CACHE BOOL "cause all build commands to be displayed")

#=================================================================================
#
# IF GNU compiler, functions without a return type or switch
# statements that do not handle all of the enumerated types
# are treated as an error.  Also, all warnings.
#
IF (NOT MSVC)
if (CMAKE_COMPILER_IS_GNUCC)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)
    string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
#    list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
#    list(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)
#    message("gcc major minor version numbers are: "${GCC_MAJOR},${GCC_MINOR})
endif()
if (CMAKE_COMPILER_IS_GNUCC)
if(${GCC_VERSION} VERSION_LESS "4.2")
    ADD_DEFINITIONS(-W -Wall)
else()
    ADD_DEFINITIONS(-W -Wall -Werror=return-type -Wunused-parameter)
endif()
endif()
ENDIF (NOT MSVC)

#=================================================================================
FIND_PACKAGE(OpenSSL)
IF(OPENSSL_FOUND)
    INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
ENDIF(OPENSSL_FOUND)

#=================================================================================
#
# Must have QT 4.7 or later
#
FIND_PACKAGE(Qt4 4.7.1 REQUIRED)
IF(QT4_FOUND)
ELSE(QT4_FOUND)
    MESSAGE(FATAL_ERROR "QT4 not found")
ENDIF(QT4_FOUND)

#=================================================================================
#
# QT include files
#
INCLUDE(${QT_USE_FILE})
MESSAGE("QT Libraries: " ${QT_LIBRARIES})

#=================================================================================
SET (VTK_DIR $ENV{VTK_DIR})
FIND_PACKAGE(VTK REQUIRED NO_MODULE)
IF (VTK_FOUND)
  MESSAGE("VTK: " ${VTK_INCLUDE_DIRS})
  MESSAGE("VTK: " ${VTK_LIBRARY_DIRS})
  INCLUDE(${VTK_USE_FILE})
  ADD_DEFINITIONS( -DHAVE_VTK )
  ADD_DEFINITIONS( -DHAVE_VTK5 )
  IF (VTK_MAJOR_VERSION EQUAL 6)
    ADD_DEFINITIONS( -DHAVE_VTK6 )
  ENDIF (VTK_MAJOR_VERSION EQUAL 6)
ELSE (VTK_FOUND)
   MESSAGE(FATAL_ERROR "VTK not found, is environment variable VTK_DIR set?")
ENDIF(VTK_FOUND)
#=================================================================================
#
#  Expect OpenMP on Apple and UNIX
#
#FIND_PACKAGE(OpenMP)
#IF (OPENMP_FOUND)
#    ADD_DEFINITIONS(${OpenMP_CXX_FLAGS})
#    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
#ELSE (OPENMP_FOUND)
#    MESSAGE(WARNING "OpenMP was not found")
#ENDIF(OPENMP_FOUND)

#=================================================================================
#
# Flags
# 
ADD_DEFINITIONS( -DCARET_BUILDID=${CARET_BUILDID} )
ADD_DEFINITIONS( -DCARET_FLAG )

#=================================================================================
#
# ZLIB is optional
#
IF (EXISTS $ENV{ZLIB_DIR})
    SET (ZLIB_ROOT $ENV{ZLIB_DIR})
ENDIF (EXISTS $ENV{ZLIB_DIR})
FIND_PACKAGE(ZLIB REQUIRED)
MESSAGE("ZLIB INCLUDE: " ${ZLIB_INCLUDE_DIRS})
MESSAGE("ZLIB LIB: " ${ZLIB_LIBRARIES})

INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})

#=================================================================================
#
# NETCDF is optional
#
SET(NETCDF_LIBRARIES "")
IF (EXISTS $ENV{NETCDF_DIR})
    IF (EXISTS $ENV{MINC_DIR})
       FIND_PATH(NETCDF_INCLUDE_DIRECTORY 
              netcdf.h
              PATHS $ENV{NETCDF_DIR}/include)
       MESSAGE("NETCDF INC: " ${NETCDF_INCLUDE_DIRECTORY})
       IF (EXISTS ${NETCDF_INCLUDE_DIRECTORY})
           INCLUDE_DIRECTORIES(${NETCDF_INCLUDE_DIRECTORY})
       ELSE (EXISTS ${NETCDF_INCLUDE_DIRECTORY})
           MESSAGE(FATAL_ERROR "Environment variable NETCDF_DIR is invalid path or not set")
       ENDIF (EXISTS ${NETCDF_INCLUDE_DIRECTORY})

       FIND_LIBRARY(NETCDF_LIBRARY 
                 NAMES netcdf.lib libnetcdf.a libnetcdf.so netcdf.dll
                 PATHS $ENV{NETCDF_DIR}/lib)
       MESSAGE("NETCDF LIB: " ${NETCDF_LIBRARY})
       IF (EXISTS ${NETCDF_LIBRARY})
       ELSE (EXISTS ${NETCDF_LIBRARY})
           MESSAGE(FATAL_ERROR "Could not find NETCDF library")
       ENDIF (EXISTS ${NETCDF_LIBRARY})
   

       FIND_PATH(MINC_INCLUDE_DIRECTORY
                 minc.h
                 PATHS $ENV{MINC_DIR}/include)
       MESSAGE("MINC INC: " ${MINC_INCLUDE_DIRECTORY})
       IF (EXISTS ${MINC_INCLUDE_DIRECTORY})
           INCLUDE_DIRECTORIES(${MINC_INCLUDE_DIRECTORY})
       ELSE (EXISTS ${MINC_INCLUDE_DIRECTORY})
           MESSAGE(FATAL_ERROR "Environment variable MINC_DIR is invalid path or not set")
       ENDIF (EXISTS ${MINC_INCLUDE_DIRECTORY})

       FIND_LIBRARY(MINC_LIBRARY 
                 NAMES minc.lib libminc.a libminc.so minc.dll
                       minc2.lib libminc2.a libminc2.so minc2.dll
                 PATHS $ENV{MINC_DIR}/lib)
       MESSAGE("MINC LIB: " ${MINC_LIBRARY})
       IF (EXISTS ${MINC_LIBRARY})
       ELSE (EXISTS ${MINC_LIBRARY})
           MESSAGE(FATAL_ERROR "Could not find MINC library")
       ENDIF (EXISTS ${MINC_LIBRARY})
   
       MESSAGE("Configuring WITH MINC and NETCDF")
       
       ADD_DEFINITIONS(-DHAVE_MINC)
   ENDIF (EXISTS $ENV{MINC_DIR})
ENDIF (EXISTS $ENV{NETCDF_DIR})

#=================================================================================
#
# QWT is Optional
#
SET(QWT_LIBRARIES "")
IF (EXISTS $ENV{QWT_DIR})
    FIND_PATH(QWT_INCLUDE_DIRECTORY 
              qwt.h
              PATHS $ENV{QWT_DIR}/include)
    MESSAGE("QWT INC: " ${QWT_INCLUDE_DIRECTORY})
    IF (EXISTS ${QWT_INCLUDE_DIRECTORY})
        ADD_DEFINITIONS(-DHAVE_QWT)
        INCLUDE_DIRECTORIES(${QWT_INCLUDE_DIRECTORY})
    ELSE (EXISTS ${QWT_INCLUDE_DIRECTORY})
        MESSAGE(FATAL_ERROR "Environment variable QWT_INC_DIR is valid but QWT_INC_DIR is invalid path or not set")
    ENDIF (EXISTS ${QWT_INCLUDE_DIRECTORY})

    FIND_LIBRARY(QWT_LIBRARY 
                 NAMES qwt.lib libqwt.a libqwt.so qwt.dll
                 PATHS $ENV{QWT_DIR}/lib)
    MESSAGE("QWT LIB: " ${QWT_LIBRARY})
    IF (EXISTS ${QWT_LIBRARY})
    ELSE (EXISTS ${QWT_LIBRARY})
        MESSAGE(FATAL_ERROR "Could not find QWT library")
    ENDIF (EXISTS ${QWT_LIBRARY})
   

    MESSAGE("Configuring WITH Qwt")
ELSE (EXISTS $ENV{QWT_DIR})
    MESSAGE("Configuring WITHOUT Qwt")
ENDIF (EXISTS $ENV{QWT_DIR})

#    IF (EXISTS $ENV{QWT_LIB_DIR})
#           SET(QWT_LIBRARIES -L$ENV{QWT_LIB_DIR} -lqwt)
#    ELSE (EXISTS $ENV{QWT_LIB_DIR})
#        MESSAGE(FATAL_ERROR "Environment variable QWT_INC_DIR is valid but QWT_LIB_DIR is invalid path or not set")
#    ENDIF (EXISTS $ENV{QWT_LIB_DIR})
    

#=================================================================================
#
# Need OpenGL
#
FIND_PACKAGE(OpenGL REQUIRED)
IF (OPENGL_FOUND)
   #
   # Need help finding includes on Apple
   #
   IF (APPLE)
      # When searching for the include directory, find the location
      # for the OpenGL framework rather than an individual header file.
      FIND_PATH(OPENGL_INCLUDE_DIR OpenGL.framework
        /System/Library/Frameworks
        /Library/Frameworks
        ~/Library/Frameworks
      )
   ENDIF (APPLE)
   #
   # OpenGL Include Directory
   #
   INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
   MESSAGE("OPENGL include is " ${OPENGL_INCLUDE_DIR})
ELSE (OPENGL_FOUND)
   MESSAGE(FATAL_ERROR "OpenGL Libraries were not found")
ENDIF (OPENGL_FOUND)

#=================================================================================
#
# Need libann - will fail on windows as I've written it. linux only now.
#
FIND_PACKAGE(LibANN REQUIRED)
if (LIBANN_FOUND)
	INCLUDE_DIRECTORIES(${LIBANN_INCLUDE_DIR})
endif (LIBANN_FOUND)




#=================================================================================
#
# All subdirectories that will be configured for building
#
ADD_SUBDIRECTORY ( caret_common )
#ADD_SUBDIRECTORY ( caret_vtk4_classes )
ADD_SUBDIRECTORY ( caret_uniformize )
ADD_SUBDIRECTORY ( caret_gifti )
ADD_SUBDIRECTORY ( caret_files )
ADD_SUBDIRECTORY ( caret_command_operations )
ADD_SUBDIRECTORY ( caret_brain_set )
ADD_SUBDIRECTORY ( caret_cifti )
ADD_SUBDIRECTORY ( caret_statistics )
ADD_SUBDIRECTORY ( caret_gui )
ADD_SUBDIRECTORY ( caret_widgets )
ADD_SUBDIRECTORY ( caret5 )
ADD_SUBDIRECTORY ( caret_command )
ADD_SUBDIRECTORY ( caret_edit )
ADD_SUBDIRECTORY ( wb_import )

