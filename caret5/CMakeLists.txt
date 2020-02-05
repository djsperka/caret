#
# Name of Project
#
PROJECT(Caret5)

#
# Name of executable
#
set (EXE_NAME caret5)

#
#  QT Libraries
#
SET(QT_USE_QTXML TRUE)
SET(QT_USE_QTOPENGL TRUE)
SET(QT_USE_QTNETWORK TRUE)

#
# QT Include directories
#
INCLUDE (${QT_USE_FILE})

#
# Added by JWH to eliminate OpenGL linking errors in Ubuntu Linux
#
IF (UNIX)
    IF (NOT APPLE)
        FIND_PACKAGE(OpenGL)
    ENDIF (NOT APPLE)
ENDIF (UNIX)
#
# Images
#

#
# Create the executable
# Apple creates a bundle
#
IF (APPLE)
   ADD_EXECUTABLE(${EXE_NAME}
      MACOSX_BUNDLE
      main.cxx
   )
ELSE (APPLE)
   ADD_EXECUTABLE(${EXE_NAME}
      main.cxx
   )
ENDIF (APPLE)

SET (CARET_LIBRARIES
   CaretGui
   CaretCommandOperations
   CaretBrainSet
#   CaretVtk4Classes
   CaretGifti
   CaretFiles
   CaretUniformize
   CaretStatistics
   CaretCommon
   CaretWidgets
   CaretCifti
)

#
# Libraries that are linked
#
TARGET_LINK_LIBRARIES(${EXE_NAME}
   ${CARET_LIBRARIES}
   ${CARET_LIBRARIES}
   ${QT_LIBRARIES}
   ${QWT_LIBRARY}
   ${QT_LIBRARIES}
   ${VTK_LIBRARIES}
   ${MINC_LIBRARY}
   ${NETCDF_LIBRARY}
   ${ZLIB_LIBRARIES}
   ${LIBANN_LIBRARIES}
)

IF(WIN32)
    TARGET_LINK_LIBRARIES(${EXE_NAME}
    opengl32
    glu32
    )
ENDIF(WIN32)

IF (UNIX)
   IF (NOT APPLE) 
      TARGET_LINK_LIBRARIES(${EXE_NAME}
         ${OPENGL_LIBRARIES}
#         gobject-2.0
      )
   ENDIF (NOT APPLE)
ENDIF (UNIX)

#
# At this time, Cocoa needs to be explicitly added for Apple Mac
#
IF (APPLE)
   #SET (QT_MAC_USE_COCOA TRUE)
   TARGET_LINK_LIBRARIES(${EXE_NAME}
     "-framework Cocoa"
     "-framework OpenGL"
   )
ENDIF (APPLE)

#
# Find Headers
#
INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/caret5
${CMAKE_SOURCE_DIR}/caret_gui
${CMAKE_SOURCE_DIR}/caret_brain_set
${CMAKE_SOURCE_DIR}/caret_cifti
${CMAKE_SOURCE_DIR}/caret_command_operations
${CMAKE_SOURCE_DIR}/caret_common
${CMAKE_SOURCE_DIR}/caret_files
${CMAKE_SOURCE_DIR}/caret_gifti
${CMAKE_SOURCE_DIR}/caret_statistics
${CMAKE_SOURCE_DIR}/caret_uniformize
#${CMAKE_SOURCE_DIR}/caret_vtk4_classes
${CMAKE_SOURCE_DIR}/caret_widgets
)

IF (APPLE)
    SET (MACOSX_BUNDLE_INFO_STRING Caret5 Copyright 2012 )
    SET (MACOSX_BUNDLE_ICON_FILE caret5.icns )
    SET (MACOSX_BUNDLE_GUI_IDENTIFIER caret5 )
    SET (MACOSX_BUNDLE_LONG_VERSION_STRING caret5)
    SET (MACOSX_BUNDLE_BUNDLE_NAME caret5)
    SET (MACOSX_BUNDLE_SHORT_VERSION_STRING 5)
    SET (MACOSX_BUNDLE_BUNDLE_VERSION 5)
    SET (MACOSX_BUNDLE_COPYRIGHT  2012 )

    ADD_CUSTOM_COMMAND(
        TARGET ${EXE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SOURCE_DIR}/CMakeScripts/copy_mac_icon.sh ${EXE_NAME} ${CMAKE_SOURCE_DIR}/caret5/caret5.icns
    )

    ADD_CUSTOM_COMMAND(
        TARGET ${EXE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_SOURCE_DIR}/CMakeScripts/run_macdeployqt.sh ${EXE_NAME}
    )
ENDIF (APPLE)
