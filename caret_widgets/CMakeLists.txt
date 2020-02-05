
#
# Name of Project
#
PROJECT(CaretWidgets)


#
# QT include files
#
INCLUDE(${QT_USE_FILE})

SET(MOC_INPUT_HEADER_FILES
QtDialogWizard.h
QtListBoxSelectionDialog.h
QtMultipleInputDialog.h
QtRadioButtonSelectionDialog.h
QtScriptInputDialog.h
QtTableDialog.h
QtTextEditDialog.h
QtTextFileEditorDialog.h
QtCylinderGridFrame.h
WuQDataEntryDialog.h
WuQDialog.h
WuQFileDialog.h
WuQMessageBox.h
WuQMultiPageDialog.h
WuQSaveWidgetAsImagePushButton.h
WuQSeparatorLine.h
WuQWidgetGroup.h
WuQWizard.h
)

QT4_WRAP_CPP(MOC_SOURCE_FILES ${MOC_INPUT_HEADER_FILES})

#
# Create a library
#
ADD_LIBRARY(CaretWidgets
      QtDialogWizard.h 
	   QtListBoxSelectionDialog.h 
	   QtMultipleInputDialog.h 
	   QtRadioButtonSelectionDialog.h 
      QtScriptInputDialog.h 
      QtTableDialog.h 
	   QtTextEditDialog.h 
      QtTextFileEditorDialog.h 
	   QtUtilities.h 
      WuQDataEntryDialog.h 
      WuQDialog.h 
      WuQFileDialog.h 
      WuQMessageBox.h 
      WuQMultiPageDialog.h 
      WuQSaveWidgetAsImagePushButton.h 
      WuQSeparatorLine.h 
      WuQWidgetGroup.h 
      WuQWizard.h
      QtAngleSpinBox.h
      QtCylinderGridFrame.h

      ${MOC_SOURCE_FILES}

      QtDialogWizard.cxx 
	   QtListBoxSelectionDialog.cxx 
	   QtMultipleInputDialog.cxx 
	   QtRadioButtonSelectionDialog.cxx 
      QtScriptInputDialog.cxx 
      QtTableDialog.cxx 
	   QtTextEditDialog.cxx 
      QtTextFileEditorDialog.cxx 
	   QtUtilities.cxx 
      WuQDataEntryDialog.cxx 
      WuQDialog.cxx 
      WuQFileDialog.cxx 
      WuQMessageBox.cxx 
      WuQMultiPageDialog.cxx 
      WuQSaveWidgetAsImagePushButton.cxx 
      WuQSeparatorLine.cxx 
      WuQWidgetGroup.cxx 
      WuQWizard.cxx 
      QtAngleSpinBox.cxx
      QtCylinderGridFrame.cxx
)

#
# Include directories
#
INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/caret_common
${CMAKE_SOURCE_DIR}/caret_files
${CMAKE_SOURCE_DIR}/caret_gifti
)
