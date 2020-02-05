/*LICENSE_START*/
/*
 *  Copyright 1995-2002 Washington University School of Medicine
 *
 *  http://brainmap.wustl.edu
 *
 *  This file is part of CARET.
 *
 *  CARET is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  CARET is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CARET; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*LICENSE_END*/


#include <algorithm>
#include <iostream>

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDir>
#include <QFile>
#include "WuQFileDialog.h"
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QToolBar>
#include <QToolButton>
#include <QToolTip>
#include <QGridLayout>
#include <QtDebug>

#include "AreaColorFile.h"
#include "ArealEstimationFile.h"
#include "BorderColorFile.h"
#include "BorderFile.h"
#include "BrainModelBorderSet.h"
#include "BrainModelContours.h"
#include "BrainSet.h"
#include "Category.h"
#include "CellColorFile.h"
#include "CellProjectionFile.h"
#include "CellFile.h"
#include "CellProjectionFile.h"
#include "CocomacConnectivityFile.h"
#include "ContourCellFile.h"
#include "ContourCellColorFile.h"
#include "ContourFile.h"
#include "CutsFile.h"
#include "DebugControl.h"
#include "DeformationFieldFile.h"
#include "DeformationMapFile.h"
#include "FileFilters.h"
#include "FileUtilities.h"
#include "FociColorFile.h"
#include "FociFile.h"
#include "FociProjectionFile.h"
#include "FociSearchFile.h"
#include "GeodesicDistanceFile.h"
#include "GuiCategoryComboBox.h"
#include "GuiDataFileOpenDialog.h"
#include "GuiSpeciesComboBox.h"
#include "GuiStereotaxicSpaceComboBox.h"
#include "GuiStructureComboBox.h"
#include "GuiMainWindow.h"
#include "LatLonFile.h"
#include "MetricFile.h"
#include "QtListBoxSelectionDialog.h"
#include "QtUtilities.h"
#include "GuiDataFileCommentDialog.h"
#include "GuiMainWindow.h"
#include "GuiSpecFileDialog.h"
#include "PaintFile.h"
#include "PaletteFile.h"
#include "ParamsFile.h"
#include "ProbabilisticAtlasFile.h"
#include "QtTextEditDialog.h"
#include "RgbPaintFile.h"
#include "SectionFile.h"
#include "Species.h"
#include "StereotaxicSpace.h"
#include "StringUtilities.h"
#include "StudyCollectionFile.h"
#include "StudyMetaDataFile.h"
#include "SurfaceShapeFile.h"
#include "VectorFile.h"
#include "TopographyFile.h"
#include "VocabularyFile.h"
#include "WuQFileDialog.h"
#include "WustlRegionFile.h"
#include "global_variables.h"

/**
 * The Constructor.  After calling the constructor, call "exec()" to popup this modal 
 * dialog and which returns QDialog::Accepted or QDialog::Rejected.
 */
GuiSpecFileDialog::GuiSpecFileDialog(QWidget* parent,
                                     SpecFile& sfin,
                                     const SPEC_DIALOG_MODE dialogModeIn)
   : WuQDialog(parent)
{
   setAttribute(Qt::WA_DeleteOnClose);
   dialogMode = dialogModeIn;
   
   setSizeGripEnabled(true);
   QString caption("Specification File: ");
   caption.append(FileUtilities::basename(sfin.getFileName()));
   setWindowTitle(caption);
   
   QVBoxLayout* rows = new QVBoxLayout(this);
   rows->setSpacing(5);
   
   //
   // Main window containing tool bar and file selections
   //
   specFileDialogMainWindow = new GuiSpecFileDialogMainWindow(0,
                                                              sfin,
                                                              dialogMode);
   rows->addWidget(specFileDialogMainWindow);
   rows->setStretchFactor(specFileDialogMainWindow, 1);
   
   //
   // Close window when main window requests it
   //
   QObject::connect(specFileDialogMainWindow, SIGNAL(signalCloseDialogRequested()),
                    this, SLOT(slotCloseDialog()));
                    
   switch (dialogMode) {
      case SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
         {
            //
            // Dialog Buttons Row 1
            //
            QHBoxLayout* buttonsLayout1 = new QHBoxLayout;
            rows->addLayout(buttonsLayout1);
            rows->setStretchFactor(buttonsLayout1, 1000);

            //
            // Load Scenes button
            //
            QPushButton* loadScenesButton = new QPushButton("Load Scene(s)");
            loadScenesButton->setAutoDefault(false);
            QObject::connect(loadScenesButton, SIGNAL(clicked()),
                             specFileDialogMainWindow, SLOT(slotLoadScenesButton()));
            buttonsLayout1->addWidget(loadScenesButton);
            loadScenesButton->setToolTip(
                          "Load only the scene files, nothing else.\n"
                          "The Display Control Dialog will be displayed\n"
                          "and preset to the Scene Page.");
            if (sfin.sceneFile.files.empty()) {
               loadScenesButton->setEnabled(false);
            }
         
            //
            // Create new spec file button
            //
            QPushButton* createSpecButton = new QPushButton("Create New Spec...");
            createSpecButton->setAutoDefault(false);
            QObject::connect(createSpecButton, SIGNAL(clicked()),
                             specFileDialogMainWindow, SLOT(slotCreateSpecButton()));
            buttonsLayout1->addWidget(createSpecButton);
            createSpecButton->setToolTip(
                          "Create a new spec file containing\n"
                          "only the currently selected files.");
         
            //
            // Set transform button
            //
            QPushButton* setTransformPushButton = new QPushButton("Set Transform...");
            setTransformPushButton->setAutoDefault(false);
            QObject::connect(setTransformPushButton, SIGNAL(clicked()),
                             specFileDialogMainWindow, SLOT(slotSetTransformPushButton()));
            buttonsLayout1->addWidget(setTransformPushButton);
            setTransformPushButton->setToolTip(
                          "Choose a transformation matrix that will\n"
                          "be applied to RAW and FIDUCIAL border,\n"
                          "cell, coordinate, foci, and volume files\n"
                          "immediately after they are read.  If this\n"
                          "button is pressed and no matrix is selected\n"
                          "the matrix defaults to the identity matrix.");
                          
            QtUtilities::makeButtonsSameSize(loadScenesButton, createSpecButton, setTransformPushButton);
         }
         break;
      case SPEC_DIALOG_MODE_OPEN_DATA_FILE:
         break;
      case SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
         break;
   }
   
   //
   // Dialog Buttons
   //
   QHBoxLayout* buttonsLayout = new QHBoxLayout;
   rows->addLayout(buttonsLayout);
   rows->setStretchFactor(buttonsLayout, 1000);
   
   //
   // OK button
   //
   QPushButton* ok = NULL;
   switch (dialogMode) {
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
         ok = new QPushButton("Load");
         ok->setAutoDefault(false);
         QObject::connect(ok, SIGNAL(clicked()),
                          specFileDialogMainWindow, SLOT(slotOkButton()));
         buttonsLayout->addWidget(ok);
         ok->setToolTip("Load the currently selected\n"
                           "files into Caret.");
         break;
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_DATA_FILE:
         break;
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
         break;
   }
   
   QPushButton* allButton = NULL;
   QPushButton* deselectAllButton = NULL;
   
   switch (dialogMode) {
      case SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
         //
         // Select all files button
         //
         allButton = new QPushButton("Select All");
         allButton->setAutoDefault(false);
         QObject::connect(allButton, SIGNAL(clicked()),
                          specFileDialogMainWindow, SLOT(slotSelectAllFiles()));
         buttonsLayout->addWidget(allButton);
         allButton->setToolTip(
                     "Select All Files");

         //
         // Deselect all files button
         //
         deselectAllButton = new QPushButton("Deselect All");
         deselectAllButton->setAutoDefault(false);
         QObject::connect(deselectAllButton, SIGNAL(clicked()),
                          specFileDialogMainWindow, SLOT(slotDeselectAllFiles()));
         buttonsLayout->addWidget(deselectAllButton);
         deselectAllButton->setToolTip(
                     "Deselect All Files");
         break;
      case SPEC_DIALOG_MODE_OPEN_DATA_FILE:
         break;
      case SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
         break;
   }
 
   //
   // Cancel/Close button
   //
   QPushButton* cancel = new QPushButton("Close");
   cancel->setAutoDefault(false);
   QObject::connect(cancel, SIGNAL(clicked()),
                    this, SLOT(slotCloseDialogButton()));
   buttonsLayout->addWidget(cancel);
   switch (dialogMode) {
      case SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
         break;
      case SPEC_DIALOG_MODE_OPEN_DATA_FILE:
         cancel->setText("Close");
         cancel->setFixedSize(cancel->sizeHint());
         break;
      case SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
         cancel->setText("Close");
         cancel->setFixedSize(cancel->sizeHint());
         break;
   }
   cancel->setToolTip(
                 "Close this dialog and\n"
                 "take no further action.");
   
   autoCloseCheckBox = NULL;

   switch (dialogMode) {
      case SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
         {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ok);
            buttons.push_back(cancel);
            buttons.push_back(allButton);
            buttons.push_back(deselectAllButton);
            QtUtilities::makeButtonsSameSize(buttons);
         }
         break;
      case SPEC_DIALOG_MODE_OPEN_DATA_FILE:
         //
         // Create "Auto Close" checkbox
         //
         autoCloseCheckBox = new QCheckBox("Auto Close");
         autoCloseCheckBox->setChecked(true);
         autoCloseCheckBox->setToolTip(
                       "If checked, the dialog is\n"
                       "closed after a file is chosen.");
         buttonsLayout->addWidget(autoCloseCheckBox);
         break;
      case SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
         break;
   }
      
   QtUtilities::positionAndSetDialogSize(theMainWindow, this, this->sizeHint());
   adjustSize();
   
   multipleFiducialCoordFlag = (sfin.fiducialCoordFile.getNumberOfFiles() > 1);
}

/**
 * The Destructor.
 */
GuiSpecFileDialog::~GuiSpecFileDialog()
{
}

/**
 * Gets called when dialog is to be popped up.  Intercept this call so that the multiple
 * fiducial warning can be displayed.
 */
void
GuiSpecFileDialog::show()
{
   //
   // Popup the dialog first
   //
   WuQDialog::show();
}

/**
 * Called when close pushbutton is hit.  This is called instead of slotCloseDialog
 * since the auto close check box may not be selected.
 */
void
GuiSpecFileDialog::slotCloseDialogButton()
{
   specFileDialogMainWindow->writeSpecFileIfNeeded();
   
   QDialog::close();
}

/**
 * Close the dialog.
 */
void
GuiSpecFileDialog::slotCloseDialog()
{
   if (autoCloseCheckBox != NULL) {
      if (autoCloseCheckBox->isChecked() == false) {
         return;
      }
   }
   
   specFileDialogMainWindow->writeSpecFileIfNeeded();
   
   QDialog::close();
}

//-------------------------------------------------------------------------------

/**
 * Constructor.
 */
GuiSpecFileDialogMainWindow::GuiSpecFileDialogMainWindow(QWidget* parent,
                                                         SpecFile& sf,
                                                         const GuiSpecFileDialog::SPEC_DIALOG_MODE dialogModeIn)
   : QMainWindow(parent)
{
   specFile = sf;
   dialogMode = dialogModeIn,
   directoryName = FileUtilities::dirname(sf.getFileName());

   specFileNeedsToBeWritten = false;
   
   //
   // create the toolbar
   //
   createToolBar();
   
   //
   // Main Widget
   //
   QWidget* vboxm = new QWidget(this);
   setCentralWidget(vboxm);
   QVBoxLayout* vbLayout = new QVBoxLayout(vboxm);
   
   //
   // Button Group for fast open buttons
   //
   fastOpenButtonGroup = new QButtonGroup(this);
   QObject::connect(fastOpenButtonGroup, SIGNAL(buttonClicked(int)),
                    this, SLOT(fastOpenButtonSlot(int)));   
   
   //
   // Button Group for info buttons
   //
   infoButtonGroup = new QButtonGroup(this);
   QObject::connect(infoButtonGroup, SIGNAL(buttonClicked(int)),
                    this, SLOT(infoButtonGroupSlot(int)));   
   
   //
   // Button Group for delete (X) buttons
   //
   deleteButtonGroup = new QButtonGroup(this);
   QObject::connect(deleteButtonGroup, SIGNAL(buttonClicked(int)),
                    this, SLOT(deleteButtonGroupSlot(int)));   
   
   //
   // Save the current directory and set to the spec file's directory
   //
   checkForMissingFiles = false;
   const QString savedDirectory(QDir::currentPath());
   const QString specFileDir(FileUtilities::dirname(sf.getFileName()));
   //if (QDir::isRelativePath(specFileDir) == false) {
      QDir::setCurrent(specFileDir);
      checkForMissingFiles = true;
   //}
      
   BrainSet* bs = theMainWindow->getBrainSet();
   
   //
   // Vertical Box Layout for the entire dialog
   //
   filesListWidget = new QWidget;
   QVBoxLayout* filesLayout = new QVBoxLayout;
   surfaceParametersGroup = listSurfaceParameters(filesLayout);
   
   topologyGroup = listTopologyFiles(filesLayout, specFile);

   coordGroup = listCoordinateFiles(filesLayout, specFile);
   
   surfaceGroup = listSurfaceFiles(filesLayout, specFile);
   
   areaColorGroup = listFiles(filesLayout, 
                              "Area Color Files", 
                              bs->getAreaColorFile(),
                              SpecFile::getAreaColorFileTag(), 
                              specFile.areaColorFile);
   arealEstGroup = listFiles(filesLayout, 
                             "Areal Estimation Files", 
                              bs->getArealEstimationFile(),
                             SpecFile::getArealEstimationFileTag(), 
                             specFile.arealEstimationFile);
   borderGroup = listBorderFiles(filesLayout, 
                                 specFile);   
   borderColorGroup = listFiles(filesLayout, 
                                "Border Color Files", 
                                bs->getBorderColorFile(),
                                SpecFile::getBorderColorFileTag(), 
                                specFile.borderColorFile);
   borderProjGroup = listFiles(filesLayout, 
                               "Border Projection Files", 
                               NULL,
                               SpecFile::getBorderProjectionFileTag(), 
                               specFile.borderProjectionFile);
   cellGroup = listFiles(filesLayout, 
                         "Cell Files (Fiducial)", 
                         bs->getCellProjectionFile(),  // NOTE: all cells are stored in cell projection file
                         SpecFile::getCellFileTag(), 
                         specFile.cellFile);
   cellVolumeGroup = listFiles(filesLayout, 
                               "Cell Files (Volume)", 
                               bs->getVolumeCellFile(),
                               SpecFile::getVolumeCellFileTag(), 
                               specFile.volumeCellFile);
   cellColorGroup = listFiles(filesLayout, 
                              "Cell Color Files", 
                              bs->getCellColorFile(),
                              SpecFile::getCellColorFileTag(), 
                              specFile.cellColorFile);
   cellProjectionGroup = listFiles(filesLayout, 
                                   "Cell Projection Files", 
                                   bs->getCellProjectionFile(),
                                   SpecFile::getCellProjectionFileTag(), 
                                   specFile.cellProjectionFile);
   cerebralHullGroup = listFiles(filesLayout, 
                                 "Cerebral Hull Files", 
                                 NULL,
                                 SpecFile::getCerebralHullFileTag(), 
                                 specFile.cerebralHullFile);
   cocomacGroup = listFiles(filesLayout, 
                            "CoCoMac Connectivity File", 
                              bs->getCocomacFile(),
                            SpecFile::getCocomacConnectivityFileTag(), 
                            specFile.cocomacConnectivityFile, 
                            false);
   ContourFile* contourFile = NULL;
   BrainModelContours* bmc = bs->getBrainModelContours();
   if (bmc != NULL) {
      contourFile = bmc->getContourFile();
   }
   contourGroup = listFiles(filesLayout, 
                            "Contour File", 
                            contourFile,
                            SpecFile::getContourFileTag(), 
                            specFile.contourFile);
   contourCellGroup = listFiles(filesLayout, 
                                "Contour Cell File", 
                                bs->getContourCellFile(),
                                SpecFile::getContourCellFileTag(), 
                                specFile.contourCellFile);
   contourCellColorGroup = listFiles(filesLayout,
                                     "Contour Cell Color File", 
                                     bs->getContourCellColorFile(),
                                     SpecFile::getContourCellColorFileTag(), 
                                     specFile.contourCellColorFile);
   cutsGroup = listFiles(filesLayout, 
                         "Cuts File", 
                         bs->getCutsFile(),
                         SpecFile::getCutsFileTag(), 
                         specFile.cutsFile);
   defFieldGroup = listFiles(filesLayout, 
                             "Deformation Field File", 
                              bs->getDeformationFieldFile(),
                             SpecFile::getDeformationFieldFileTag(),
                             specFile.deformationFieldFile);
   defMapGroup = listFiles(filesLayout, 
                           "Deformation Map File", 
                           NULL,
                           SpecFile::getDeformationMapFileTag(), 
                           specFile.deformationMapFile);
   documentGroup = listFiles(filesLayout,
                             "Document Files",
                             NULL,
                             SpecFile::getDocumentFileTag(),
                             specFile.documentFile);
   fociGroup = listFiles(filesLayout, 
                         "Foci Files (Fiducial)", 
                         bs->getFociProjectionFile(),  // NOTE: All foci are stored in foci projection file
                         SpecFile::getFociFileTag(), 
                         specFile.fociFile);
   fociColorGroup = listFiles(filesLayout, 
                              "Foci Color File", 
                              bs->getFociColorFile(),
                              SpecFile::getFociColorFileTag(), 
                              specFile.fociColorFile);
   fociProjGroup = listFiles(filesLayout, 
                             "Foci Projection File", 
                              bs->getFociProjectionFile(),
                             SpecFile::getFociProjectionFileTag(), 
                             specFile.fociProjectionFile);
   fociSearchGroup = listFiles(filesLayout, 
                             "Foci Search File", 
                              bs->getFociSearchFile(),
                             SpecFile::getFociSearchFileTag(), 
                             specFile.fociSearchFile);
   geodesicGroup = listFiles(filesLayout, 
                             "Geodesic Distance File", 
                              bs->getGeodesicDistanceFile(),
                             SpecFile::getGeodesicDistanceFileTag(),
                             specFile.geodesicDistanceFile);
   imagesGroup = listFiles(filesLayout, 
                           "Image Files",
                           NULL,
                           SpecFile::getImageFileTag(),
                           specFile.imageFile);
   latLonGroup = listFiles(filesLayout, 
                           "Latitude Longitude Files", 
                           bs->getLatLonFile(),
                           SpecFile::getLatLonFileTag(), 
                           specFile.latLonFile);
   metricGroup = listFiles(filesLayout, 
                           "Metric Files", 
                           bs->getMetricFile(),
                           SpecFile::getMetricFileTag(), 
                           specFile.metricFile);
   paintGroup = listFiles(filesLayout, 
                           "Paint Files", 
                           bs->getPaintFile(),
                           SpecFile::getPaintFileTag(), 
                           specFile.paintFile);
   paletteGroup = listFiles(filesLayout, 
                           "Palette Files", 
                           bs->getPaletteFile(),
                           SpecFile::getPaletteFileTag(), 
                           specFile.paletteFile);
   paramsGroup = listFiles(filesLayout, 
                           "Params File", 
                           bs->getParamsFile(),
                           SpecFile::getParamsFileTag(), 
                           specFile.paramsFile);
   probAtlasGroup = listFiles(filesLayout, 
                           "Probabilistic Atlas Files", 
                           bs->getProbabilisticAtlasSurfaceFile(),
                           SpecFile::getAtlasFileTag(), 
                           specFile.atlasFile);
   rgbPaintGroup = listFiles(filesLayout, 
                           "RGB Paint Files", 
                           bs->getRgbPaintFile(),
                           SpecFile::getRgbPaintFileTag(), 
                           specFile.rgbPaintFile);
   sceneGroup = listFiles(filesLayout, 
                           "Scene Files", 
                           bs->getSceneFile(),
                           SpecFile::getSceneFileTag(), 
                           specFile.sceneFile);
   sectionGroup = listFiles(filesLayout, 
                           "Section Files", 
                           bs->getSectionFile(),
                           SpecFile::getSectionFileTag(), 
                           specFile.sectionFile);
   shapeGroup = listFiles(filesLayout, 
                           "Surface Shape Files", 
                           bs->getSurfaceShapeFile(),
                           SpecFile::getSurfaceShapeFileTag(), 
                           specFile.surfaceShapeFile);
   studyCollectionGroup = listFiles(filesLayout, 
                           "Study Collection Files", 
                           bs->getStudyCollectionFile(),
                           SpecFile::getStudyCollectionFileTag(),
                           specFile.studyCollectionFile);
   studyMetaDataGroup = listFiles(filesLayout, 
                           "Study Metadata Files", 
                           bs->getStudyMetaDataFile(),
                           SpecFile::getStudyMetaDataFileTag(),
                           specFile.studyMetaDataFile);
   topographyGroup = listFiles(filesLayout, 
                           "Topography Files", 
                           bs->getTopographyFile(),
                           SpecFile::getTopographyFileTag(), 
                           specFile.topographyFile);
   transMatrixGroup = listFiles(filesLayout, 
                           "Transformation Matrix Files", 
                           bs->getTransformationMatrixFile(),
                           SpecFile::getTransformationMatrixFileTag(), 
                           specFile.transformationMatrixFile);
   transDataGroup = listFiles(filesLayout, 
                           "Transformation Data Files", 
                           NULL,
                           SpecFile::getTransformationDataFileTag(), 
                           specFile.transformationDataFile);
   vectorGroup = listFiles(filesLayout,
                           "Vector Files",
                           NULL,
                           SpecFile::getVectorFileTag(),
                           specFile.vectorFile,
                           true,
                           false);
   vocabularyGroup = listFiles(filesLayout, 
                               "Vocabulary Files", 
                               bs->getVocabularyFile(),
                               SpecFile::getVocabularyFileTag(),
                               specFile.vocabularyFile);
   volumeAnatomyGroup = listFiles(filesLayout, 
                                  "Volume Files - Anatomy", 
                                  NULL,
                                  SpecFile::getVolumeAnatomyFileTag(),
                                  specFile.volumeAnatomyFile, 
                                  true, 
                                  true); 
   volumeFuncGroup = listFiles(filesLayout, 
                               "Volume Files - Functional", 
                               NULL,
                               SpecFile::getVolumeFunctionalFileTag(),
                               specFile.volumeFunctionalFile, 
                               true, 
                               true); 
   volumePaintGroup = listFiles(filesLayout, 
                                "Volume Files - Paint", 
                                NULL,
                                SpecFile::getVolumePaintFileTag(),
                                specFile.volumePaintFile, 
                                true, 
                                true); 
   volumeProbAtlasGroup = listFiles(filesLayout, 
                                    "Volume Files - Prob Atlas", 
                                    NULL,
                                    SpecFile::getVolumeProbAtlasFileTag(),
                                    specFile.volumeProbAtlasFile, 
                                    true, 
                                    true); 
   volumeRgbGroup = listFiles(filesLayout, 
                              "Volume Files - RGB", 
                              NULL,
                              SpecFile::getVolumeRgbFileTag(),
                              specFile.volumeRgbFile, 
                              true, 
                              true); 
   volumeSegmentGroup = listFiles(filesLayout, 
                           "Volume Files - Segmentation", 
                           NULL,
                           SpecFile::getVolumeSegmentationFileTag(),
                           specFile.volumeSegmentationFile, 
                           true, 
                           true); 
   volumeVectorGroup = listFiles(filesLayout, 
                                 "Volume Files - Vector", 
                                 NULL,
                                 SpecFile::getVolumeVectorFileTag(),
                                 specFile.volumeVectorFile, 
                                 true, 
                                 true); 
   vtkModelGroup = listFiles(filesLayout, 
                             "VTK Model Files", 
                             NULL,
                             SpecFile::getVtkModelFileTag(),
                             specFile.vtkModelFile);
   wustlRegionGroup = listFiles(filesLayout, 
                                "Wustl Region Files", 
                                bs->getWustlRegionFile(),
                                SpecFile::getWustlRegionFileTag(),
                                specFile.wustlRegionFile);

   trajectoryGroup = listFiles(filesLayout,
		   	   	   	   	   	   "Electrode trajectory Files",
							   NULL,
							   SpecFile::getTrajectoryFileTag(),
							   specFile.trajectoryFile);


   QLabel* dummyLabel = new QLabel(" ");
   filesLayout->addWidget(dummyLabel);
   filesLayout->setStretchFactor(dummyLabel, 10000000);
   
   filesListWidget->setLayout(filesLayout);
   
   //
   // Scroll View for all selections
   //
   filesScrollArea = new QScrollArea;
   vbLayout->addWidget(filesScrollArea);  
   filesScrollArea->setWidget(filesListWidget);
   filesScrollArea->setWidgetResizable(true);
   
   //
   // Restore the saved directory
   //
   QDir::setCurrent(savedDirectory);
   
   disableToolBarButtons();
   slotToolBarAllButton();
}

/**
 * Destructor.
 */
GuiSpecFileDialogMainWindow::~GuiSpecFileDialogMainWindow()
{
}

/**
 * write the spec file if needed.
 */
void 
GuiSpecFileDialogMainWindow::writeSpecFileIfNeeded()
{
   switch (dialogMode) {
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
         if (savedStructure != structureComboBox->getSelectedStructure()) {
            specFile.setStructure(structureComboBox->getSelectedStructureAsString());
            specFileNeedsToBeWritten = true;
         }
         if (savedSpecies != speciesComboBox->getSelectedSpecies()) {
            specFile.setSpecies(speciesComboBox->getSelectedSpecies().getName());
            specFileNeedsToBeWritten = true;
         }
         if (savedStereotaxicSpace != spaceComboBox->getSelectedStereotaxicSpace()) {
            specFile.setSpace(spaceComboBox->getSelectedStereotaxicSpace());
            specFileNeedsToBeWritten = true;
         }
         if (savedCategory != categoryComboBox->getSelectedCategory()) {
            specFile.setCategory(categoryComboBox->getSelectedCategory());
            specFileNeedsToBeWritten = true;
         }
         if (savedSubject != subjectLineEdit->text()) {
            specFile.setSubject(subjectLineEdit->text());
            specFileNeedsToBeWritten = true;
         }
         if (savedComment != specFile.getFileComment()) {
            specFileNeedsToBeWritten = true;
         }
         break;
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_DATA_FILE:
         break;
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
         break;
   }
   
   if (specFileNeedsToBeWritten) {
      try {
         specFile.writeFile(specFile.getFileName());
      }
      catch (FileException& e) {
         QMessageBox::critical(this, "Error Writing Spec File", e.whatQString());
      }
      specFileNeedsToBeWritten = false;
   }
}

/**
 * Load the file filter combo box
 */
void
GuiSpecFileDialogMainWindow::createToolBar()
{
   //
   // Toolbar for file filtering
   //   
   QToolBar* toolbar = new QToolBar(this);
   addToolBar(toolbar);
   toolbar->setMovable(false);
   
   //
   // All files
   //
   allAction = new QAction("All", this);
   QObject::connect(allAction, SIGNAL(toggled(bool)),
                    this, SLOT(slotToolBarAllButton()));
   allAction->setToolTip("Press this button to\n"
                         "show all files.");
   allAction->setCheckable(true);
   allButton = new QToolButton;
   allButton->setDefaultAction(allAction);
   toolbar->addWidget(allButton);
   
   //
   // Coord and topology
   //
   geomAction = new QAction("Geom", this);
   geomAction->setCheckable(true);
   geomAction->setToolTip("Press this button to show\n"
                          "only coord and topo files.");
   QObject::connect(geomAction, SIGNAL(toggled(bool)),
                    this, SLOT(slotToolBarGeometryButton()));
   geomButton = new QToolButton;
   geomButton->setDefaultAction(geomAction);
   toolbar->addWidget(geomButton);
   
   //
   // Border files
   //
   borderAction = new QAction("Border", this);
   borderAction->setCheckable(true);
   borderAction->setToolTip(
                 "Press this button to\n"
                 "show only border files.");
   QObject::connect(borderAction, SIGNAL(toggled(bool)),
                    this, SLOT(slotToolBarBorderButton()));
   borderButton = new QToolButton;
   borderButton->setDefaultAction(borderAction);
   toolbar->addWidget(borderButton);
   
   //
   // cell files
   //
   cellAction = new QAction("Cell", this);
   cellAction->setCheckable(true);
   cellAction->setToolTip("Press this button to\n"
                          "show only cell files.");
   QObject::connect(cellAction, SIGNAL(toggled(bool)),
                    this, SLOT(slotToolBarCellButton()));
   cellButton = new QToolButton;
   cellButton->setDefaultAction(cellAction);
   toolbar->addWidget(cellButton);

   //
   // foci files
   //
   fociAction = new QAction("Foci", this);
   fociAction->setCheckable(true);
   fociAction->setToolTip(
                 "Press this button to\n"
                 "show only foci files.");
   QObject::connect(fociAction, SIGNAL(toggled(bool)),
                    this, SLOT(slotToolBarFociButton()));
   fociButton = new QToolButton;
   fociButton->setDefaultAction(fociAction);
   toolbar->addWidget(fociButton);
   
   //
   // metric/palette files
   //
   metricAction = new QAction("Metric", this);
   metricAction->setCheckable(true);
   metricAction->setToolTip(
                 "Press this button to show\n"
                 "only metric and palette files.");
   QObject::connect(metricAction, SIGNAL(toggled(bool)),
                    this, SLOT(slotToolBarMetricButton()));
   metricButton = new QToolButton;
   metricButton->setDefaultAction(metricAction);
   toolbar->addWidget(metricButton);
   
   //
   // misc files
   //
   miscAction = new QAction("Misc", this);
   miscAction->setCheckable(true);
   miscAction->setToolTip(
                 "Press this button to\n"
                 "show all other file types.");
   QObject::connect(miscAction, SIGNAL(toggled(bool)),
                    this, SLOT(slotToolBarMiscButton()));
   miscButton = new QToolButton;
   miscButton->setDefaultAction(miscAction);
   toolbar->addWidget(miscButton);
   
   //
   // paint files
   //
   paintAction = new QAction("Paint", this);
   paintAction->setCheckable(true);
   paintAction->setToolTip("Press this button to\n"
                 "show only paint files.");
   QObject::connect(paintAction, SIGNAL(toggled(bool)),
                    this, SLOT(slotToolBarPaintButton()));
   paintButton = new QToolButton;
   paintButton->setDefaultAction(paintAction);
   toolbar->addWidget(paintButton);
   
   //
   // scene files
   //
   sceneAction = new QAction("Scene", this);
   sceneAction->setCheckable(true);
   sceneAction->setToolTip("Press this button to\n"
                 "show only scene files.");
   QObject::connect(sceneAction, SIGNAL(toggled(bool)),
                    this, SLOT(slotToolBarSceneButton()));
   sceneButton = new QToolButton;
   sceneButton->setDefaultAction(sceneAction);
   toolbar->addWidget(sceneButton);

   //
   // volume files
   //
   volumeAction = new QAction("Volume", this);
   volumeAction->setCheckable(true);
   volumeAction->setToolTip(
                 "Press this button to\n"
                 "show only volume files.");
   QObject::connect(volumeAction, SIGNAL(toggled(bool)),
                    this, SLOT(slotToolBarVolumeButton()));
   volumeButton = new QToolButton;
   volumeButton->setDefaultAction(volumeAction);
   toolbar->addWidget(volumeButton);
}

/**
 * Called when the toolbar cell button is pressed.
 */
void
GuiSpecFileDialogMainWindow::slotToolBarCellButton()
{
   fileFilterSelections = FILE_FILTER_CELL;
   setToolBarButtons();
}

/**
 * Called when the toolbar foci button is pressed.
 */
void
GuiSpecFileDialogMainWindow::slotToolBarFociButton()
{
   fileFilterSelections = FILE_FILTER_FOCI;
   setToolBarButtons();
}

/**
 * Called when the toolbar metric button is pressed.
 */
void
GuiSpecFileDialogMainWindow::slotToolBarMetricButton()
{
   fileFilterSelections = FILE_FILTER_METRIC_AND_PALETTE;
   setToolBarButtons();
}

/**
 * Called when the toolbar misc button is pressed.
 */
void
GuiSpecFileDialogMainWindow::slotToolBarMiscButton()
{
   fileFilterSelections = FILE_FILTER_MISC;
   setToolBarButtons();
}

/**
 * Called when the toolbar paint button is pressed.
 */
void
GuiSpecFileDialogMainWindow::slotToolBarPaintButton()
{
   fileFilterSelections = FILE_FILTER_PAINT;
   setToolBarButtons();
}

/**
 * Called when the toolbar scene button is pressed.
 */
void
GuiSpecFileDialogMainWindow::slotToolBarSceneButton()
{
   fileFilterSelections = FILE_FILTER_SCENE;
   setToolBarButtons();
}


/**
 * Called when the toolbar volume button is pressed.
 */
void
GuiSpecFileDialogMainWindow::slotToolBarVolumeButton()
{
   fileFilterSelections = FILE_FILTER_VOLUME;
   setToolBarButtons();
}

/**
 * Called when the toolbar  button is pressed.
 */
void
GuiSpecFileDialogMainWindow::slotToolBarAllButton()
{
   fileFilterSelections = FILE_FILTER_ALL;
   setToolBarButtons();
}

/**
 * Called when the toolbar  button is pressed.
 */
void
GuiSpecFileDialogMainWindow::slotToolBarGeometryButton()
{
   fileFilterSelections = FILE_FILTER_COORD_AND_TOPO;
   setToolBarButtons();
}

/**
 * Called when the toolbar border button is pressed.
 */
void
GuiSpecFileDialogMainWindow::slotToolBarBorderButton()
{
   fileFilterSelections = FILE_FILTER_BORDER;
   setToolBarButtons();
}

/**
 * Disable the file filter buttons for file types not loaded.
 */
void
GuiSpecFileDialogMainWindow::disableToolBarButtons()
{
   if ((coordGroup == NULL) &&
       (topologyGroup == NULL) &&
       (surfaceGroup == NULL)) {
      geomAction->setEnabled(false);
   }
   if ((borderGroup == NULL) &&
       (borderColorGroup == NULL) &&
       (borderProjGroup == NULL)) {
      borderAction->setEnabled(false);
   }
   if ((cellGroup == NULL) &&
       (cellVolumeGroup == NULL) &&
       (cellColorGroup == NULL) &&
       (cellProjectionGroup == NULL)) {
      cellAction->setEnabled(false);
   }
   if ((fociGroup == NULL) &&
       (fociColorGroup == NULL) &&
       (fociProjGroup == NULL) &&
       (fociSearchGroup == NULL) &&
       (studyCollectionGroup == NULL) &&
       (studyMetaDataGroup == NULL)) {
      fociAction->setEnabled(false);
   }
   if ((metricGroup == NULL) &&
       (paletteGroup == NULL)) {
      metricAction->setEnabled(false);
   }
   if ((cerebralHullGroup == NULL) &&
       (cocomacGroup == NULL) &&
       (contourGroup == NULL) &&
       (contourCellGroup == NULL) &&
       (contourCellColorGroup == NULL) &&
       (cutsGroup == NULL) &&
       (defFieldGroup == NULL) &&
       (defMapGroup == NULL) &&
       (documentGroup == NULL) &&
       (geodesicGroup == NULL) &&
       (imagesGroup == NULL) &&
       (latLonGroup == NULL) &&
       (paramsGroup == NULL) &&
       (sectionGroup == NULL) &&
       (shapeGroup == NULL) &&
       (studyCollectionGroup == NULL) &&
       (studyMetaDataGroup == NULL) &&
       (vectorGroup == NULL) &&
       (topographyGroup == NULL) &&
       (transMatrixGroup == NULL) &&
       (transDataGroup == NULL) &&
       (vocabularyGroup == NULL) &&
       (vtkModelGroup == NULL) &&
       (wustlRegionGroup == NULL)) {
      miscAction->setEnabled(false);
   }
   if ((areaColorGroup == NULL) &&
       (arealEstGroup == NULL) &&
       (paintGroup == NULL) &&
       (probAtlasGroup == NULL) &&
       (rgbPaintGroup == NULL)) {
      paintAction->setEnabled(false);
   }
   if (sceneGroup == NULL) {
      sceneAction->setEnabled(false);
   }

   if ((volumeAnatomyGroup == NULL) &&
       (volumeFuncGroup == NULL) &&
       (volumePaintGroup == NULL) &&
       (volumeProbAtlasGroup == NULL) &&
       (volumeRgbGroup == NULL) &&
       (volumeSegmentGroup == NULL) &&
       (volumeVectorGroup == NULL)) {
      volumeAction->setEnabled(false);
   }
}


/**
 * Set the toolbar buttons on/off.
 */
void
GuiSpecFileDialogMainWindow::setToolBarButtons()
{
   bool showSurfaceParameters = false;
   bool showAreaColors = false;
   bool showCoordTopo = false;
   bool showBorder    = false;
   bool showPaint     = false;
   bool showMetric    = false;
   bool showCell      = false;
   bool showFoci      = false;
   bool showScene     = false;
   bool showVolume    = false;
   bool showMisc      = false;
   
   //
   // block signals to toolbar buttons otherwise, this method gets
   // called every time one of buttons is set on or off
   //
   allAction->blockSignals(true);
   geomAction->blockSignals(true);
   borderAction->blockSignals(true);
   cellAction->blockSignals(true);
   fociAction->blockSignals(true);
   metricAction->blockSignals(true);
   miscAction->blockSignals(true);
   paintAction->blockSignals(true);
   sceneAction->blockSignals(true);
   volumeAction->blockSignals(true);
   
   allAction->setChecked(false);
   geomAction->setChecked(false);
   borderAction->setChecked(false);
   cellAction->setChecked(false);
   fociAction->setChecked(false);
   metricAction->setChecked(false);
   miscAction->setChecked(false);
   paintAction->setChecked(false);
   sceneAction->setChecked(false);
   volumeAction->setChecked(false);
   
   switch (fileFilterSelections) {
      case FILE_FILTER_ALL:
         allAction->setChecked(true);
         showSurfaceParameters = true;
         showAreaColors = true;
         showCoordTopo = true;
         showBorder = true;
         showPaint = true;
         showMetric = true;
         showCell = true;
         showFoci = true;
         showVolume = true;
         showMisc = true;
         showScene = true;
         break;
      case FILE_FILTER_COORD_AND_TOPO:
         geomAction->setChecked(true);
         showCoordTopo = true;
         break;
      case FILE_FILTER_BORDER:
         borderAction->setChecked(true);
         showBorder = true;
         break;
      case FILE_FILTER_PAINT:
         paintAction->setChecked(true);
         showAreaColors = true;
         showPaint = true;
         break;
      case FILE_FILTER_METRIC_AND_PALETTE:
         metricAction->setChecked(true);
         showMetric = true;
         break;
      case FILE_FILTER_CELL:
         cellAction->setChecked(true);
         showCell = true;
         break;
      case FILE_FILTER_FOCI:
         fociAction->setChecked(true);
         showFoci = true;
         break;
      case FILE_FILTER_SCENE:
         sceneAction->setChecked(true);
         showScene = true;
         break;
      case FILE_FILTER_VOLUME:
         volumeAction->setChecked(true);
         showVolume = true;
         if ((volumePaintGroup != NULL) || (volumeProbAtlasGroup != NULL)) {
            showAreaColors = true;
         }
         break;
      case FILE_FILTER_MISC:
         miscAction->setChecked(true);
         showMisc = true;
         break;
   }
   
   if (surfaceParametersGroup != NULL) {
      surfaceParametersGroup->setHidden(!showSurfaceParameters);
   }
   if (topologyGroup != NULL) {
      topologyGroup->setHidden(!showCoordTopo);
   }
   if (coordGroup != NULL) {
      coordGroup->setHidden(!showCoordTopo);
   }
   if (surfaceGroup != NULL) {
      surfaceGroup->setHidden(!showCoordTopo);
   }
   if (areaColorGroup != NULL) {
      areaColorGroup->setHidden(!showAreaColors);
   }
   if (arealEstGroup != NULL) {
      arealEstGroup->setHidden(!showPaint);
   }
   if (borderGroup != NULL) {
      borderGroup->setHidden(!showBorder);
   }
   if (borderColorGroup != NULL) {
      borderColorGroup->setHidden(!showBorder);
   }
   if (borderProjGroup != NULL) {
      borderProjGroup->setHidden(!showBorder);
   }
   if (cellGroup != NULL) {
      cellGroup->setHidden(!showCell);
   }
   if (cellVolumeGroup != NULL) {
      cellVolumeGroup->setHidden(!showCell);
   }
   if (cellColorGroup != NULL) {
      cellColorGroup->setHidden(!showCell);
   }
   if (cellProjectionGroup != NULL) {
      cellProjectionGroup->setHidden(!showCell);
   }
   if (cerebralHullGroup != NULL) {
      cerebralHullGroup->setHidden(!showMisc);
   }
   if (cocomacGroup != NULL) {
      cocomacGroup->setHidden(!showMisc);
   }
   if (contourGroup != NULL) {
      contourGroup->setHidden(!showMisc);
   }
   if (contourCellGroup != NULL) {
      contourCellGroup->setHidden(!showMisc);
   }
   if (contourCellColorGroup != NULL) {
      contourCellColorGroup->setHidden(!showMisc);
   }
   if (cutsGroup != NULL) {
      cutsGroup->setHidden(!showMisc);
   }
   if (defFieldGroup != NULL) {
      defFieldGroup->setHidden(!showMisc);
   }
   if (defMapGroup != NULL) {
      defMapGroup->setHidden(!showMisc);
   }
   if (documentGroup != NULL) {
      documentGroup->setHidden(!showMisc);
   }
   if (fociGroup != NULL) {
      fociGroup->setHidden(!showFoci);
   }
   if (fociColorGroup != NULL) {
      fociColorGroup->setHidden(!showFoci);
   }
   if (fociProjGroup != NULL) {
      fociProjGroup->setHidden(!showFoci);
   }
   if (fociSearchGroup != NULL) {
      fociSearchGroup->setHidden(!showFoci);
   }
   if (geodesicGroup != NULL) {
      geodesicGroup->setHidden(!showMisc);
   }
   if (imagesGroup != NULL) {
      imagesGroup->setHidden(!showMisc);
   }
   if (latLonGroup != NULL) {
      latLonGroup->setHidden(!showMisc);
   }
   if (metricGroup != NULL) {
      metricGroup->setHidden(!showMetric);
   }
   if (paintGroup != NULL) {
      paintGroup->setHidden(!showPaint);
   }
   if (paletteGroup != NULL) {
      paletteGroup->setHidden(!showMetric);
   }
   if (paramsGroup != NULL) {
      paramsGroup->setHidden(!showMisc);
   }
   if (probAtlasGroup != NULL) {
      probAtlasGroup->setHidden(!showPaint);
   }
   if (rgbPaintGroup != NULL) {
      rgbPaintGroup->setHidden(!showPaint);
   }
   if (sceneGroup != NULL) {
      sceneGroup->setHidden(!showScene);
   }
   if (sectionGroup != NULL) {
      sectionGroup->setHidden(!showMisc);
   }
   if (shapeGroup != NULL) {
      shapeGroup->setHidden(!showMisc);
   }
   if (studyCollectionGroup != NULL) {
      studyCollectionGroup->setHidden(!showMisc && !showFoci);
   }
   if (studyMetaDataGroup != NULL) {
      studyMetaDataGroup->setHidden(!showMisc && !showFoci);
   }
   if (vectorGroup != NULL) {
      vectorGroup->setHidden(!showMisc);
   }
   if (topographyGroup != NULL) {
      topographyGroup->setHidden(!showMisc);
   }
   if (transMatrixGroup != NULL) {
      transMatrixGroup->setHidden(!showMisc);
   }
   if (transDataGroup != NULL) {
      transDataGroup->setHidden(!showMisc);
   }
   if (vocabularyGroup != NULL) {
      vocabularyGroup->setHidden(!showMisc);
   }
   if (volumeAnatomyGroup != NULL) {
      volumeAnatomyGroup->setHidden(!showVolume);
   }
   if (volumeFuncGroup != NULL) {
      volumeFuncGroup->setHidden(!showVolume);
   }
   if (volumePaintGroup != NULL) {
      volumePaintGroup->setHidden(!showVolume);
   }
   if (volumeProbAtlasGroup != NULL) {
      volumeProbAtlasGroup->setHidden(!showVolume);
   }
   if (volumeRgbGroup != NULL) {
      volumeRgbGroup->setHidden(!showVolume);
   }
   if (volumeSegmentGroup != NULL) {
      volumeSegmentGroup->setHidden(!showVolume);
   }
   if (volumeVectorGroup != NULL) {
      volumeVectorGroup->setHidden(!showVolume);
   }
   if (vtkModelGroup != NULL) {
      vtkModelGroup->setHidden(!showMisc);
   }
   if (wustlRegionGroup != NULL) {
      wustlRegionGroup->setHidden(!showMisc);
   }

   allAction->blockSignals(false);
   geomAction->blockSignals(false);
   borderAction->blockSignals(false);
   cellAction->blockSignals(false);
   fociAction->blockSignals(false);
   metricAction->blockSignals(false);
   miscAction->blockSignals(false);
   paintAction->blockSignals(false);
   sceneAction->blockSignals(false);
   volumeAction->blockSignals(false);   
   
   filesScrollArea->horizontalScrollBar()->setSliderPosition(0);
   filesScrollArea->verticalScrollBar()->setSliderPosition(0);
}

/**
 * Create a new spec file using the selected files.
 */
void
GuiSpecFileDialogMainWindow::slotCreateSpecButton()
{
   //
   // Save current directory and switch to directory of the spec file
   //
   const QString currentDirectory = QDir::currentPath();
   if (DebugControl::getDebugOn()) {
      std::cout << "Current directory: " << currentDirectory.toAscii().constData() << std::endl;
   }
   
   //
   // Set current directory to that of the spec file in the dialog
   //
   specFile.setCurrentDirectoryToSpecFileDirectory();
   if (DebugControl::getDebugOn()) {
      std::cout << "Spec File: " << specFile.getFileName().toAscii().constData() << std::endl;
      std::cout << "Current directory now: " << QDir::currentPath().toAscii().constData() << std::endl;
   }
   
   WuQFileDialog fd(this);
   fd.setModal(true);
   fd.setWindowTitle("Choose New Spec File Name");
   fd.setFileMode(WuQFileDialog::AnyFile);
   fd.setFilters(QStringList(FileFilters::getSpecFileFilter()));
   fd.selectFilter(FileFilters::getSpecFileFilter());
   fd.setDirectory(directoryName);
   fd.rereadDir();
   
//   if (newSpecName.isNull() == false) {
   if (fd.exec() == QDialog::Accepted) {
      if (fd.selectedFiles().count() > 0) {
         QString name = fd.selectedFiles().at(0);
         if (name.isEmpty() == false) {
            //
            // Add file extension if necessary
            //
            if (name.length() > 5) {
               const QString ext = name.right(5);
               if (ext.compare(".spec") != 0) {
                  name.append(".spec");
               }
            }
            else {
               name.append(".spec");
            }
            
            if (QFile::exists(name)) {
               if (QMessageBox::question(this, "Overwrite ?",
                      "The Spec File you have selected exists.\n"
                      "Do you want to replace it with this new\n"
                      "Spec File that will contain only the \n"
                      "selected files in the the Spec File Dialog?",
                      (QMessageBox::Yes | QMessageBox::No),
                      QMessageBox::No)
                         == QMessageBox::No) {
                  return;
               }
            }
            
            //
            // Save current spec file name
            //
            const QString saveName(specFile.getFileName());
            
            //
            // Set the hemisphere, space, species, subject and category
            //
            specFile.setStructure(structureComboBox->getSelectedStructure());
            specFile.setSpace(spaceComboBox->getSelectedStereotaxicSpace());
            specFile.setSpecies(speciesComboBox->getSelectedSpecies());
            specFile.setCategory(categoryComboBox->getSelectedCategory());
            specFile.setSubject(subjectLineEdit->text());
            
            //
            // Set the checked files as selected files in the spec file
            //
            setCheckedFilesAsSelected();

            //
            // Write the spec file with only the selected files.
            //
            specFile.setWriteOnlySelectedFiles(true);
            try {
               specFile.writeFile(name);
            }
            catch (FileException& e) {
               QMessageBox::critical(this, "Error Updating Spec File", e.whatQString());
            }
            specFile.setWriteOnlySelectedFiles(false);
            
            //
            // Restore the spec file's name
            //
            specFile.setFileName(saveName);
            
            //
            // Open new spec in a spec file dialog
            //
            theMainWindow->readSpecFile(name);
            
            //
            // Close this dialog
            //
            dynamic_cast<GuiSpecFileDialog*>(parent())->close();
         }
      }
   }
   else {
      //
      // Restore the current directory
      //
      QDir::setCurrent(currentDirectory);
   }
}

/**
 * Called when Load Scene(s) button is pressed.
 */
void
GuiSpecFileDialogMainWindow::slotLoadScenesButton()
{
   //
   // Spec file might need to be saved
   //
   writeSpecFileIfNeeded();

   specFile.setAllFileSelections(SpecFile::SPEC_FALSE);
   specFile.sceneFile.setAllSelections(SpecFile::SPEC_TRUE);
   theMainWindow->loadSpecFilesDataFiles(specFile, &transformMatrix);
   
   emit signalCloseDialogRequested();
}

/**
 * Set the files that are checked as seleted in the spec file.
 */
void
GuiSpecFileDialogMainWindow::setCheckedFilesAsSelected()
{
   for (unsigned int i = 0; i < selectionItems.size(); i++) {
      if (selectionItems[i].checkbox->isChecked()) {
         (*(selectionItems[i].selected)) = SpecFile::SPEC_TRUE;
      }
      else {
         (*(selectionItems[i].selected)) = SpecFile::SPEC_FALSE;
      }
   }
}

/**
 * Called when set transform button is pressed.
 */
void 
GuiSpecFileDialogMainWindow::slotSetTransformPushButton()
{
   //
   // Reset the transformation matrix
   //
   transformMatrix.identity();
   
   //
   // Popup a dialog to choose a transformation matrix file
   //
   WuQFileDialog fd(this);
   fd.setDirectory(directoryName);
   fd.setFilter(FileFilters::getTransformationMatrixFileFilter());
   fd.selectFilter(FileFilters::getTransformationMatrixFileFilter());
   fd.setModal(true);
   fd.setFileMode(WuQFileDialog::ExistingFile);
   
   //
   // If the user selected a file
   //
   if (fd.exec() == WuQFileDialog::Accepted) {
      const QString filename = fd.selectedFiles().at(0);
      
      //
      // read the TransformationMatrixFile
      //
      TransformationMatrixFile tmf;
      try {
         tmf.readFile(filename);
      }
      catch (FileException& e) {
         QMessageBox::critical(this, "ERROR", e.whatQString());
         return;
      }
      
      //
      // make sure there are matrices in the file
      //
      const int num = tmf.getNumberOfMatrices();
      if (num <= 0) {
         QMessageBox::critical(this, "ERROR", "Matrix file contains no matrices.");
         return;
      }
      std::vector<QString> matrixNames;
      for (int i = 0; i < num; i++) {
         matrixNames.push_back(tmf.getTransformationMatrix(i)->getMatrixName());
      }
      
      //
      // Popup a dialog to choose a matrix from the file
      //
      QtListBoxSelectionDialog lbsd(this, "Choose Matrix", "",
                                    matrixNames, -1);
      if (lbsd.exec() == QDialog::Accepted) {
         //
         // If the user chose a matrix
         //
         const int item = lbsd.getSelectedItemIndex();
         transformMatrix = *(tmf.getTransformationMatrix(item));
      }
   }
}
      
/**
 * Load the data files in my spec file.
 */
void
GuiSpecFileDialogMainWindow::slotOkButton()
{
   //
   // Spec file might need to be saved
   //
   writeSpecFileIfNeeded();
   
   setCheckedFilesAsSelected();
   
   //
   // If a flat coord file is selected, check to see if a cut topo is selected
   //
   if (specFile.flatCoordFile.getNumberOfFilesSelected() > 0) {
      if (specFile.cutTopoFile.getNumberOfFilesSelected() == 0) {
         if (QMessageBox::warning(this, "File Selection Warning",
                                 "You have selected a flat coordinate file but\n"
                                 "no cut topology file is selected.",
                                 (QMessageBox::Ok | QMessageBox::Cancel),
                                 QMessageBox::Cancel)
                                    == QMessageBox::Cancel) {
            return;
         }         
      }
   }
   
   //
   // If a lobar flat coord file is selected, check to see if a lobar cut topo is selected
   //
   if (specFile.lobarFlatCoordFile.getNumberOfFilesSelected() > 0) {
      if (specFile.lobarCutTopoFile.getNumberOfFilesSelected() == 0) {
         if (QMessageBox::warning(this, "File Selection Warning",
                                 "You have selected a lobar flat coordinate file but\n"
                                 "no lobar cut topology file is selected.",
                                 (QMessageBox::Ok | QMessageBox::Cancel),
                                 QMessageBox::Cancel)
                                    == QMessageBox::Cancel) {
            return;
         }         
      }
   }
   
   //
   // Check to see if any files that require an area color file area selected but no
   // area color file is selected.
   //
   if (specFile.areaColorFile.getNumberOfFilesSelected() == 0) {
      const int count = 
         specFile.paintFile.getNumberOfFilesSelected() +
         specFile.arealEstimationFile.getNumberOfFilesSelected() +
         specFile.atlasFile.getNumberOfFilesSelected() +
         specFile.volumeProbAtlasFile.getNumberOfFilesSelected() +
         specFile.volumePaintFile.getNumberOfFilesSelected();
      if (count > 0) {
         if (QMessageBox::warning(this, "File Selection Warning",
                                 "You have selected files that require an area\n"
                                 "color file but no area color file is selected.",
                                 (QMessageBox::Ok | QMessageBox::Cancel),
                                 QMessageBox::Cancel)
                                    == QMessageBox::Cancel) {
            return;
         }
      }
   }
   
   //
   // Check to see if any type of border file is selected, but no border color
   // file is selected.
   //
   if (specFile.borderColorFile.getNumberOfFilesSelected() == 0) {
      const int count = 
            specFile.rawBorderFile.getNumberOfFilesSelected()
            + specFile.fiducialBorderFile.getNumberOfFilesSelected()
            + specFile.inflatedBorderFile.getNumberOfFilesSelected()
            + specFile.veryInflatedBorderFile.getNumberOfFilesSelected()
            + specFile.sphericalBorderFile.getNumberOfFilesSelected()
            + specFile.ellipsoidBorderFile.getNumberOfFilesSelected()
            + specFile.compressedBorderFile.getNumberOfFilesSelected()
            + specFile.flatBorderFile.getNumberOfFilesSelected()
            + specFile.lobarFlatBorderFile.getNumberOfFilesSelected()
            + specFile.unknownBorderFile.getNumberOfFilesSelected()
            + specFile.volumeBorderFile.getNumberOfFilesSelected()
            + specFile.borderProjectionFile.getNumberOfFilesSelected();
      if (count > 0) {
         if (QMessageBox::warning(this, "File Selection Warning",
                                 "You have selected border files that require a border\n"
                                 "color file but no border color file is selected.",
                                 (QMessageBox::Ok | QMessageBox::Cancel),
                                 QMessageBox::Cancel)
                                    == QMessageBox::Cancel) {
            return;
         }
      }
   }
   
   //
   // See if any files requiring a cell color file are selected but no cell color file is selected
   //
   if (specFile.cellColorFile.getNumberOfFilesSelected() == 0) {
      const int count = 
            specFile.cellFile.getNumberOfFilesSelected()
            + specFile.cellProjectionFile.getNumberOfFilesSelected();
      if (count > 0) {
         if (QMessageBox::warning(this, "File Selection Warning",
                                 "You have selected cell files that require a cell\n"
                                 "color file but no cell color file is selected.",
                                 (QMessageBox::Ok | QMessageBox::Cancel),
                                 QMessageBox::Cancel)
                                    == QMessageBox::Cancel) {
            return;
         }
      }
   }
   
   //
   // See if any files requiring a cell color file are selected but no cell color file is selected
   //
   if (specFile.contourCellColorFile.getNumberOfFilesSelected() == 0) {
      const int count = specFile.contourCellFile.getNumberOfFilesSelected();
      if (count > 0) {
         if (QMessageBox::warning(this, "File Selection Warning",
                                 "You have selected contour cell files that require a contour cell\n"
                                 "color file but no contour cell color file is selected.",
                                 (QMessageBox::Ok | QMessageBox::Cancel),
                                 QMessageBox::Cancel)
                                    == QMessageBox::Cancel) {
            return;
         }
      }
   }
   
   const int numFociSelected = specFile.fociFile.getNumberOfFilesSelected()
                               + specFile.fociProjectionFile.getNumberOfFilesSelected();
                               
   //
   // See if any files requiring a foci color file are selected but no foci file is selected
   //
   if (specFile.fociColorFile.getNumberOfFilesSelected() == 0) {
      if (numFociSelected > 0) {
         if (QMessageBox::warning(this, "File Selection Warning",
                                 "You have selected foci files that require a foci\n"
                                 "color file but no foci color file is selected.",
                                 (QMessageBox::Ok | QMessageBox::Cancel),
                                 QMessageBox::Cancel)
                                    == QMessageBox::Cancel) {
            return;
         }
         
      }
   }

   //
   // Have foci selected and have study metadata but no study metadata selected
   //
   if (numFociSelected > 0) {
      if ((specFile.studyMetaDataFile.getNumberOfFiles() > 0) &&
          (specFile.studyMetaDataFile.getNumberOfFilesSelected() == 0)) {
         if (QMessageBox::warning(this, "File Selection Warning",
                                 "You have selected foci or foci projection files.\n"
                                 "There is at least one study metadata file but it\n"
                                 "is not selected.",
                                 (QMessageBox::Ok | QMessageBox::Cancel),
                                 QMessageBox::Cancel)
                                    == QMessageBox::Cancel) {
            return;
         }
      }
   }

   theMainWindow->loadSpecFilesDataFiles(specFile, &transformMatrix);
   
   emit signalCloseDialogRequested();
}

/**
 * Select all files in the dialog.
 */
void
GuiSpecFileDialogMainWindow::slotSelectAllFiles()
{
   for (unsigned int i = 0; i < selectionItems.size(); i++) {
      if (selectionItems[i].groupWidget->isVisible()) {
         const QString name = *(selectionItems[i].fileName);
         if (name.isEmpty() == false) {
            //
            // Note: "Deleted" files start with an '*'
            //
            if (name[0] != '*') {
               selectionItems[i].checkbox->setChecked(true);
            }
         }
      }
   }
}

/**
 * Deselect all files in the dialog.
 */
void
GuiSpecFileDialogMainWindow::slotDeselectAllFiles()
{
   for (unsigned int i = 0; i < selectionItems.size(); i++) {
      if (selectionItems[i].groupWidget->isVisible()) {
         const QString name = *(selectionItems[i].fileName);
         if (name.isEmpty() == false) {
            //
            // Note: "Deleted" files start with an '*'
            //
            if (name[0] != '*') {
               selectionItems[i].checkbox->setChecked(false);
            }
         }
      }
   }
}

/**
 * List the surfaces parameters.
 */
QGroupBox*
GuiSpecFileDialogMainWindow::listSurfaceParameters(QVBoxLayout* layout)
{
   switch (dialogMode) {
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
         break;
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_DATA_FILE:
         return NULL;
         break;
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
         return NULL;
         break;
   }
   
   const int lineEditWidth = 125;
   
   //
   // species
   //
   QLabel* speciesLabel = new QLabel("Species");
   speciesComboBox = new GuiSpeciesComboBox;
   savedSpecies = specFile.getSpecies();
   speciesComboBox->setSelectedSpecies(savedSpecies);
   
   //
   // space
   //
   QLabel* spaceLabel = new QLabel("Space");
   savedStereotaxicSpace = specFile.getSpace();
   spaceComboBox = new GuiStereotaxicSpaceComboBox;
   spaceComboBox->setSelectedStereotaxicSpace(savedStereotaxicSpace);
   
   //
   // subject
   //
   QLabel* subjectLabel = new QLabel("Subject");
   subjectLineEdit = new QLineEdit;
   subjectLineEdit->setFixedWidth(lineEditWidth);
   savedSubject = specFile.getSubject();
   subjectLineEdit->setText(savedSubject);
   subjectLineEdit->setToolTip(
                 "Enter the name of\n"
                 "the subject here.");

   //
   // category
   //
   QLabel* categoryLabel = new QLabel("Category");
   categoryComboBox = new GuiCategoryComboBox;
   savedCategory = specFile.getCategory();
   categoryComboBox->setSelectedCategory(savedCategory);

   //
   // Structure
   //
   QLabel* structureLabel = new QLabel("Structure");
   structureComboBox = new GuiStructureComboBox(0);
   savedStructure = specFile.getStructure().getType();
   structureComboBox->setStructure(savedStructure);
   
   //
   // Comment
   //
   QPushButton* commentPushButton = new QPushButton("Comment...");
   commentPushButton->setAutoDefault(false);
   QObject::connect(commentPushButton, SIGNAL(clicked()),
                    this, SLOT(slotCommentPushButton()));
   commentPushButton->setToolTip(
                 "Press this button to edit\n"
                 "the spec file's comment.");
   savedComment = specFile.getFileComment();
   
   QGroupBox* paramBox = new QGroupBox("Metadata");
   QGridLayout* gridLayout = new QGridLayout(paramBox);
   gridLayout->setColumnMinimumWidth(2, 10);
   gridLayout->addWidget(speciesLabel, 0, 0);
   gridLayout->addWidget(speciesComboBox, 0, 1);
   gridLayout->addWidget(spaceLabel, 0, 3);
   gridLayout->addWidget(spaceComboBox, 0, 4);
   gridLayout->addWidget(subjectLabel, 1, 0);
   gridLayout->addWidget(subjectLineEdit, 1, 1);
   gridLayout->addWidget(categoryLabel, 1, 3);
   gridLayout->addWidget(categoryComboBox, 1, 4);
   gridLayout->addWidget(structureLabel, 2, 0);
   gridLayout->addWidget(structureComboBox, 2, 1);
   gridLayout->addWidget(commentPushButton, 2, 3);
   
   
   paramBox->setFixedSize(paramBox->sizeHint());
   layout->addWidget(paramBox);

   return paramBox;
}

/**
 * called when Comment button is pressed.
 */
void 
GuiSpecFileDialogMainWindow::slotCommentPushButton()
{
   QtTextEditDialog ted(this, false, true);
   ted.setText(specFile.getFileComment());
   if (ted.exec() == QtTextEditDialog::Accepted) {
      specFile.setFileComment(ted.getText());
   }
}

/**
 * List topology files for selection by user.
 */
QGroupBox*
GuiSpecFileDialogMainWindow::listTopologyFiles(QVBoxLayout* layout,
                                     SpecFile& sf)
{
   const int numFiles = sf.closedTopoFile.getNumberOfFiles()
                      + sf.openTopoFile.getNumberOfFiles()
                      + sf.cutTopoFile.getNumberOfFiles()
                      + sf.lobarCutTopoFile.getNumberOfFiles()
                      + sf.unknownTopoFile.getNumberOfFiles();
   if (numFiles == 0) {
      return NULL;
   }
   
   switch (dialogMode) {
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
         break;
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_DATA_FILE:
         break;
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
         if ((sf.closedTopoFile.getNumberOfFilesSelected()
            + sf.cutTopoFile.getNumberOfFilesSelected()
            + sf.lobarCutTopoFile.getNumberOfFilesSelected()
            + sf.unknownTopoFile.getNumberOfFilesSelected()) <= 0) {
            return NULL;
         }
         break;
   }
   
   QGroupBox* group = new QGroupBox("Topology Files");
   QGridLayout* grid = new QGridLayout(group);
   
   int numColumns = 0;
   const int BUTTON_COLUMN = numColumns++;
   const int INFO_COLUMN   = numColumns++;
   const int DELETE_COLUMN = numColumns++;
   const int TYPE_COLUMN   = numColumns++;
   const int NAME_COLUMN   = numColumns++;
   
   int rowNumber = 0;
   
   for (int i = 0; i < 5; i++) {
      QString typeLabel;
      SpecFile::Entry* fileEntryPtr = NULL;
      QString specFileTag;
      
      switch(i) {
         case 0:
            typeLabel = "CLOSED";
            fileEntryPtr = &sf.closedTopoFile;
            specFileTag = SpecFile::getClosedTopoFileTag();
            break;
         case 1:
            typeLabel = "OPEN";
            fileEntryPtr = &sf.openTopoFile;
            specFileTag = SpecFile::getOpenTopoFileTag();
            break;
         case 2:
            typeLabel = "CUT";
            fileEntryPtr = &sf.cutTopoFile;
            specFileTag = SpecFile::getCutTopoFileTag();
            break;
         case 3:
            typeLabel = "LOBAR CUT";
            fileEntryPtr = &sf.lobarCutTopoFile;
            specFileTag = SpecFile::getLobarCutTopoFileTag();
            break;
         case 4:
            typeLabel = "UNKNOWN";
            fileEntryPtr = &sf.unknownTopoFile;
            specFileTag = SpecFile::getUnknownTopoFileMatchTag();
            break;
      }
      
      for (int j = 0; j < static_cast<int>(fileEntryPtr->getNumberOfFiles()); j++ ) {
         QCheckBox* cb = NULL;
         QPushButton* pb = NULL;
         switch (dialogMode) {
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
               cb = new QCheckBox(" ");
               cb->setFixedSize(cb->sizeHint());
               cb->setChecked(fileEntryPtr->files[j].selected);
               grid->addWidget(cb, rowNumber, BUTTON_COLUMN);
               break;
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_DATA_FILE:
               pb = new QPushButton("Open");
               pb->setAutoDefault(false);
               pb->setFixedSize(pb->sizeHint());
               grid->addWidget(pb, rowNumber, BUTTON_COLUMN);
               fastOpenButtonGroup->addButton(pb, fastOpenButtonGroup->buttons().count());
               break;
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
               if (fileEntryPtr->files[j].selected == SpecFile::SPEC_FALSE) {
                  continue;
               }
               break;
         }

         QPushButton* infoButton = new QPushButton("?");
         infoButton->setAutoDefault(false);
         QSize infoButtonSize = infoButton->sizeHint();
         infoButtonSize.setWidth(40);
         infoButton->setFixedSize(infoButtonSize);
         infoButton->setToolTip("View/Edit Comment Information");
         grid->addWidget(infoButton, rowNumber, INFO_COLUMN);
         infoButtonGroup->addButton(infoButton, infoButtonGroup->buttons().count());
         
         QPushButton* deleteButton = new QPushButton("X");
         deleteButton->setAutoDefault(false);
         QSize deleteButtonSize = deleteButton->sizeHint();
         deleteButtonSize.setWidth(40);
         deleteButton->setFixedSize(deleteButtonSize);
         deleteButton->setToolTip("Press this button\n"
                                     "to remove the file\n"
                                     "from the spec file.");
         grid->addWidget(deleteButton, rowNumber, DELETE_COLUMN);
         deleteButtonGroup->addButton(deleteButton, deleteButtonGroup->buttons().count());
         
         QLabel* tl = new QLabel(typeLabel);
         tl->setFixedSize(tl->sizeHint());
         grid->addWidget(tl, rowNumber, TYPE_COLUMN);
         
         QLabel* fn = new QLabel(prepareFileName(fileEntryPtr->files[j].filename));
         fn->setFixedSize(fn->sizeHint());
         grid->addWidget(fn, rowNumber, NAME_COLUMN);
         
         selectionItems.push_back(SelectionItem(group, cb, pb, infoButton, deleteButton, tl, fn,
                                  &fileEntryPtr->files[j].selected, 
                                  specFileTag, 
                                  &fileEntryPtr->files[j].filename, 
                                  NULL, false));
         
         rowNumber++;
      }
   }
   group->setFixedSize(group->sizeHint());
   
   layout->addWidget(group);
   
   return group;
}

/**
 * List coordinate files for selection by user.
 */
QGroupBox*
GuiSpecFileDialogMainWindow::listCoordinateFiles(QVBoxLayout* layout,
                                     SpecFile& sf)
{
   const int numFiles = sf.rawCoordFile.getNumberOfFiles()
                      + sf.fiducialCoordFile.getNumberOfFiles()
                      + sf.inflatedCoordFile.getNumberOfFiles()
                      + sf.veryInflatedCoordFile.getNumberOfFiles()
                      + sf.sphericalCoordFile.getNumberOfFiles()
                      + sf.ellipsoidCoordFile.getNumberOfFiles()
                      + sf.compressedCoordFile.getNumberOfFiles()
                      + sf.flatCoordFile.getNumberOfFiles()
                      + sf.lobarFlatCoordFile.getNumberOfFiles()
                      + sf.hullCoordFile.getNumberOfFiles()
                      + sf.unknownCoordFile.getNumberOfFiles();
   if (numFiles == 0) {
      return NULL;
   }
   
   switch (dialogMode) {
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
         break;
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_DATA_FILE:
         break;
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
         if ((sf.rawCoordFile.getNumberOfFilesSelected()
            + sf.fiducialCoordFile.getNumberOfFilesSelected()
            + sf.inflatedCoordFile.getNumberOfFilesSelected()
            + sf.veryInflatedCoordFile.getNumberOfFilesSelected()
            + sf.sphericalCoordFile.getNumberOfFilesSelected()
            + sf.ellipsoidCoordFile.getNumberOfFilesSelected()
            + sf.compressedCoordFile.getNumberOfFilesSelected()
            + sf.flatCoordFile.getNumberOfFilesSelected()
            + sf.lobarFlatCoordFile.getNumberOfFilesSelected()
            + sf.hullCoordFile.getNumberOfFilesSelected()
            + sf.unknownCoordFile.getNumberOfFilesSelected()) <= 0) {
            return NULL;
         }
         break;
   }

   QGroupBox* group = new QGroupBox("Coordinate Files");   
   QGridLayout* grid = new QGridLayout(group);
   
   int numColumns = 0;
   const int BUTTON_COLUMN = numColumns++;
   const int INFO_COLUMN   = numColumns++;
   const int DELETE_COLUMN = numColumns++;
   const int TYPE_COLUMN   = numColumns++;
   const int NAME_COLUMN   = numColumns++;
   
   
   int rowNumber = 0;
   
   for (int i = 0; i < 11; i++) {
      QString typeLabel;
      SpecFile::Entry* fileEntry = NULL;
      QString specFileTag;
      
      switch(i) {
         case 0:
            typeLabel = "RAW";
            fileEntry = &sf.rawCoordFile;
            specFileTag = SpecFile::getRawCoordFileTag();
            break;
         case 1:
            typeLabel = "FIDUCIAL";
            fileEntry = &sf.fiducialCoordFile;
            specFileTag = SpecFile::getFiducialCoordFileTag();
            break;
         case 2:
            typeLabel = "INFLATED";
            fileEntry = &sf.inflatedCoordFile;
            specFileTag = SpecFile::getInflatedCoordFileTag();
            break;
         case 3:
            typeLabel = "VERY INFLATED";
            fileEntry = &sf.veryInflatedCoordFile;
            specFileTag = SpecFile::getVeryInflatedCoordFileTag();
            break;
         case 4:
            typeLabel = "SPHERICAL";
            fileEntry = &sf.sphericalCoordFile;
            specFileTag = SpecFile::getSphericalCoordFileTag();
            break;
         case 5:
            typeLabel = "ELLIPSOID";
            fileEntry = &sf.ellipsoidCoordFile;
            specFileTag = SpecFile::getEllipsoidCoordFileTag();
            break;
         case 6:
            typeLabel = "COMP MED WALL";
            fileEntry = &sf.compressedCoordFile;
            specFileTag = SpecFile::getCompressedCoordFileTag();
            break;
         case 7:
            typeLabel = "FLAT";
            fileEntry = &sf.flatCoordFile;
            specFileTag = SpecFile::getFlatCoordFileTag();
            break;
         case 8:
            typeLabel = "LOBAR FLAT";
            fileEntry = &sf.lobarFlatCoordFile;
            specFileTag = SpecFile::getLobarFlatCoordFileTag();
            break;
         case 9:
            typeLabel = "HULL";
            fileEntry = &sf.hullCoordFile;
            specFileTag = SpecFile::getHullCoordFileTag();
            break;
         case 10:
            typeLabel = "UNKNOWN";
            fileEntry = &sf.unknownCoordFile;
            specFileTag = SpecFile::getUnknownCoordFileMatchTag();
            break;
      }
      
      for (int j = 0; j < static_cast<int>(fileEntry->getNumberOfFiles()); j++ ) {
         QCheckBox* cb = NULL;
         QPushButton* pb = NULL;
         switch (dialogMode) {
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
               cb = new QCheckBox(" ");
               cb->setFixedSize(cb->sizeHint());
               cb->setChecked(fileEntry->files[j].selected);
               grid->addWidget(cb, rowNumber, BUTTON_COLUMN);
               break;
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_DATA_FILE:
               pb = new QPushButton("Open");
               pb->setAutoDefault(false);
               pb->setFixedSize(pb->sizeHint());
               grid->addWidget(pb, rowNumber, BUTTON_COLUMN);
               fastOpenButtonGroup->addButton(pb, fastOpenButtonGroup->buttons().count());
               break;
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
               if (fileEntry->files[j].selected == SpecFile::SPEC_FALSE) {
                  continue;
               }
               break;
         }

         QPushButton* infoButton = new QPushButton("?");
         infoButton->setAutoDefault(false);
         QSize infoButtonSize = infoButton->sizeHint();
         infoButtonSize.setWidth(40);
         infoButton->setFixedSize(infoButtonSize);
         infoButton->setToolTip("View/Edit Comment Information");
         grid->addWidget(infoButton, rowNumber, INFO_COLUMN);
         infoButtonGroup->addButton(infoButton, infoButtonGroup->buttons().count());
         
         QPushButton* deleteButton = new QPushButton("X");
         deleteButton->setAutoDefault(false);
         QSize deleteButtonSize = deleteButton->sizeHint();
         deleteButtonSize.setWidth(40);
         deleteButton->setFixedSize(deleteButtonSize);
         deleteButton->setToolTip("Press this button\n"
                                     "to remove the file\n"
                                     "from the spec file.");
         grid->addWidget(deleteButton, rowNumber, DELETE_COLUMN);
         deleteButtonGroup->addButton(deleteButton, deleteButtonGroup->buttons().count());
         
         QLabel* tl = new QLabel(typeLabel);
         tl->setFixedSize(tl->sizeHint());
         grid->addWidget(tl, rowNumber, TYPE_COLUMN);
         
         QLabel* fn = new QLabel(prepareFileName((fileEntry->files[j].filename)));
         fn->setFixedSize(fn->sizeHint());
         grid->addWidget(fn, rowNumber, NAME_COLUMN);
         
         selectionItems.push_back(SelectionItem(group, cb, pb, infoButton, deleteButton, tl, fn, 
                                  &(fileEntry->files[j].selected), 
                                  specFileTag, 
                                  &(fileEntry->files[j].filename), 
                                  NULL, false));
         
         rowNumber++;
      }
   }
   group->setFixedSize(group->sizeHint());
   
   layout->addWidget(group);
   
   return group;
}

/**
 * List surface files for selection by user.
 */
QGroupBox*
GuiSpecFileDialogMainWindow::listSurfaceFiles(QVBoxLayout* layout,
                                              SpecFile& sf)
{
   const int numFiles = sf.rawSurfaceFile.getNumberOfFiles()
                      + sf.fiducialSurfaceFile.getNumberOfFiles()
                      + sf.inflatedSurfaceFile.getNumberOfFiles()
                      + sf.veryInflatedSurfaceFile.getNumberOfFiles()
                      + sf.sphericalSurfaceFile.getNumberOfFiles()
                      + sf.ellipsoidSurfaceFile.getNumberOfFiles()
                      + sf.compressedSurfaceFile.getNumberOfFiles()
                      + sf.flatSurfaceFile.getNumberOfFiles()
                      + sf.lobarFlatSurfaceFile.getNumberOfFiles()
                      + sf.hullSurfaceFile.getNumberOfFiles()
                      + sf.unknownSurfaceFile.getNumberOfFiles();
   if (numFiles == 0) {
      return NULL;
   }
   
   switch (dialogMode) {
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
         break;
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_DATA_FILE:
         break;
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
         if ((sf.rawSurfaceFile.getNumberOfFilesSelected()
            + sf.fiducialSurfaceFile.getNumberOfFilesSelected()
            + sf.inflatedSurfaceFile.getNumberOfFilesSelected()
            + sf.veryInflatedSurfaceFile.getNumberOfFilesSelected()
            + sf.sphericalSurfaceFile.getNumberOfFilesSelected()
            + sf.ellipsoidSurfaceFile.getNumberOfFilesSelected()
            + sf.compressedSurfaceFile.getNumberOfFilesSelected()
            + sf.flatSurfaceFile.getNumberOfFilesSelected()
            + sf.lobarFlatSurfaceFile.getNumberOfFilesSelected()
            + sf.hullSurfaceFile.getNumberOfFilesSelected()
            + sf.unknownSurfaceFile.getNumberOfFilesSelected()) <= 0) {
            return NULL;
         }
         break;
   }

   QGroupBox* group = new QGroupBox("Surface Files");   
   QGridLayout* grid = new QGridLayout(group);
   
   int numColumns = 0;
   const int BUTTON_COLUMN = numColumns++;
   const int INFO_COLUMN   = numColumns++;
   const int DELETE_COLUMN = numColumns++;
   const int TYPE_COLUMN   = numColumns++;
   const int NAME_COLUMN   = numColumns++;
   
   
   int rowNumber = 0;
   
   for (int i = 0; i < 11; i++) {
      QString typeLabel;
      SpecFile::Entry* fileEntry = NULL;
      QString specFileTag;
      
      switch(i) {
         case 0:
            typeLabel = "RAW";
            fileEntry = &sf.rawSurfaceFile;
            specFileTag = SpecFile::getRawSurfaceFileTag();
            break;
         case 1:
            typeLabel = "FIDUCIAL";
            fileEntry = &sf.fiducialSurfaceFile;
            specFileTag = SpecFile::getFiducialSurfaceFileTag();
            break;
         case 2:
            typeLabel = "INFLATED";
            fileEntry = &sf.inflatedSurfaceFile;
            specFileTag = SpecFile::getInflatedSurfaceFileTag();
            break;
         case 3:
            typeLabel = "VERY INFLATED";
            fileEntry = &sf.veryInflatedSurfaceFile;
            specFileTag = SpecFile::getVeryInflatedSurfaceFileTag();
            break;
         case 4:
            typeLabel = "SPHERICAL";
            fileEntry = &sf.sphericalSurfaceFile;
            specFileTag = SpecFile::getSphericalSurfaceFileTag();
            break;
         case 5:
            typeLabel = "ELLIPSOID";
            fileEntry = &sf.ellipsoidSurfaceFile;
            specFileTag = SpecFile::getEllipsoidSurfaceFileTag();
            break;
         case 6:
            typeLabel = "COMP MED WALL";
            fileEntry = &sf.compressedSurfaceFile;
            specFileTag = SpecFile::getCompressedSurfaceFileTag();
            break;
         case 7:
            typeLabel = "FLAT";
            fileEntry = &sf.flatSurfaceFile;
            specFileTag = SpecFile::getFlatSurfaceFileTag();
            break;
         case 8:
            typeLabel = "LOBAR FLAT";
            fileEntry = &sf.lobarFlatSurfaceFile;
            specFileTag = SpecFile::getLobarFlatSurfaceFileTag();
            break;
         case 9:
            typeLabel = "HULL";
            fileEntry = &sf.hullSurfaceFile;
            specFileTag = SpecFile::getHullSurfaceFileTag();
            break;
         case 10:
            typeLabel = "UNKNOWN";
            fileEntry = &sf.unknownSurfaceFile;
            specFileTag = SpecFile::getUnknownSurfaceFileMatchTag();
            break;
      }
      
      for (int j = 0; j < static_cast<int>(fileEntry->getNumberOfFiles()); j++ ) {
         QCheckBox* cb = NULL;
         QPushButton* pb = NULL;
         switch (dialogMode) {
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
               cb = new QCheckBox(" ");
               cb->setFixedSize(cb->sizeHint());
               cb->setChecked(fileEntry->files[j].selected);
               grid->addWidget(cb, rowNumber, BUTTON_COLUMN);
               break;
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_DATA_FILE:
               pb = new QPushButton("Open");
               pb->setAutoDefault(false);
               pb->setFixedSize(pb->sizeHint());
               grid->addWidget(pb, rowNumber, BUTTON_COLUMN);
               fastOpenButtonGroup->addButton(pb, fastOpenButtonGroup->buttons().count());
               break;
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
               if (fileEntry->files[j].selected == SpecFile::SPEC_FALSE) {
                  continue;
               }
               break;
         }

         QPushButton* infoButton = new QPushButton("?");
         infoButton->setAutoDefault(false);
         QSize infoButtonSize = infoButton->sizeHint();
         infoButtonSize.setWidth(40);
         infoButton->setFixedSize(infoButtonSize);
         infoButton->setToolTip("View/Edit Comment Information");
         grid->addWidget(infoButton, rowNumber, INFO_COLUMN);
         infoButtonGroup->addButton(infoButton, infoButtonGroup->buttons().count());
         
         QPushButton* deleteButton = new QPushButton("X");
         deleteButton->setAutoDefault(false);
         QSize deleteButtonSize = deleteButton->sizeHint();
         deleteButtonSize.setWidth(40);
         deleteButton->setFixedSize(deleteButtonSize);
         deleteButton->setToolTip("Press this button\n"
                                     "to remove the file\n"
                                     "from the spec file.");
         grid->addWidget(deleteButton, rowNumber, DELETE_COLUMN);
         deleteButtonGroup->addButton(deleteButton, deleteButtonGroup->buttons().count());
         
         QLabel* tl = new QLabel(typeLabel);
         tl->setFixedSize(tl->sizeHint());
         grid->addWidget(tl, rowNumber, TYPE_COLUMN);
         
         QLabel* fn = new QLabel(prepareFileName((fileEntry->files[j].filename)));
         fn->setFixedSize(fn->sizeHint());
         grid->addWidget(fn, rowNumber, NAME_COLUMN);
         
         selectionItems.push_back(SelectionItem(group, cb, pb, infoButton, deleteButton, tl, fn, 
                                  &(fileEntry->files[j].selected), 
                                  specFileTag, 
                                  &(fileEntry->files[j].filename), 
                                  NULL, false));
         
         rowNumber++;
      }
   }
   group->setFixedSize(group->sizeHint());
   
   layout->addWidget(group);
   
   return group;
}

/**
 * List border files for selection by user.
 */
QGroupBox*
GuiSpecFileDialogMainWindow::listBorderFiles(QVBoxLayout* layout,
                                     SpecFile& sf)
{
   const int numFiles = sf.rawBorderFile.getNumberOfFiles()
                      + sf.fiducialBorderFile.getNumberOfFiles()
                      + sf.inflatedBorderFile.getNumberOfFiles()
                      + sf.veryInflatedBorderFile.getNumberOfFiles()
                      + sf.sphericalBorderFile.getNumberOfFiles()
                      + sf.ellipsoidBorderFile.getNumberOfFiles()
                      + sf.compressedBorderFile.getNumberOfFiles()
                      + sf.flatBorderFile.getNumberOfFiles()
                      + sf.lobarFlatBorderFile.getNumberOfFiles()
                      + sf.hullBorderFile.getNumberOfFiles()
                      + sf.unknownBorderFile.getNumberOfFiles()
                      + sf.volumeBorderFile.getNumberOfFiles();
   if (numFiles == 0) {
      return NULL;
   }
   
   QGroupBox* group = new QGroupBox("Border Files");
   QGridLayout* grid = new QGridLayout(group);
   
   int numColumns = 0;
   const int BUTTON_COLUMN = numColumns++;
   const int INFO_COLUMN   = numColumns++;
   const int DELETE_COLUMN = numColumns++;
   const int TYPE_COLUMN   = numColumns++;
   const int NAME_COLUMN   = numColumns++;
   
   
   int rowNumber = 0;
   
   for (int i = 0; i < 12; i++) {
      QString typeLabel;
      SpecFile::Entry* fileEntry = NULL;
      QString specFileTag;
      
      switch(i) {
         case 0:
            typeLabel = "RAW";
            fileEntry = &sf.rawBorderFile;
            specFileTag = SpecFile::getRawBorderFileTag();
            break;
         case 1:
            typeLabel = "FIDUCIAL";
            fileEntry = &sf.fiducialBorderFile;
            specFileTag = SpecFile::getFiducialBorderFileTag();
            break;
         case 2:
            typeLabel = "INFLATED";
            fileEntry = &sf.inflatedBorderFile;
            specFileTag = SpecFile::getInflatedBorderFileTag();
            break;
         case 3:
            typeLabel = "VERY INFLATED";
            fileEntry = &sf.veryInflatedBorderFile;
            specFileTag = SpecFile::getVeryInflatedBorderFileTag();
            break;
         case 4:
            typeLabel = "SPHERICAL";
            fileEntry = &sf.sphericalBorderFile;
            specFileTag = SpecFile::getSphericalBorderFileTag();
            break;
         case 5:
            typeLabel = "ELLIPSOID";
            fileEntry = &sf.ellipsoidBorderFile;
            specFileTag = SpecFile::getEllipsoidBorderFileTag();
            break;
         case 6:
            typeLabel = "COMP MED WALL";
            fileEntry = &sf.compressedBorderFile;
            specFileTag = SpecFile::getCompressedBorderFileTag();
            break;
         case 7:
            typeLabel = "FLAT";
            fileEntry = &sf.flatBorderFile;
            specFileTag = SpecFile::getFlatBorderFileTag();
            break;
         case 8:
            typeLabel = "LOBAR FLAT";
            fileEntry = &sf.lobarFlatBorderFile;
            specFileTag = SpecFile::getLobarFlatBorderFileTag();
            break;
         case 9:
            typeLabel = "HULL";
            fileEntry = &sf.hullBorderFile;
            specFileTag = SpecFile::getHullBorderFileTag();
            break;
         case 10:
            typeLabel = "UNKNOWN";
            fileEntry = &sf.unknownBorderFile;
            specFileTag = SpecFile::getUnknownBorderFileMatchTag();
            break;
         case 11:
            typeLabel = "VOLUME";
            fileEntry = &sf.volumeBorderFile;
            specFileTag = SpecFile::getVolumeBorderFileTag();
      }
      
      for (int j = 0; j < static_cast<int>(fileEntry->getNumberOfFiles()); j++ ) {
         QCheckBox* cb = NULL;
         QPushButton* pb = NULL;
         switch (dialogMode) {
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
               cb = new QCheckBox(" ");
               cb->setFixedSize(cb->sizeHint());
               cb->setChecked(fileEntry->files[j].selected);
               grid->addWidget(cb, rowNumber, BUTTON_COLUMN);
               break;
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_DATA_FILE:
               pb = new QPushButton("Open");
               pb->setAutoDefault(false);
               pb->setFixedSize(pb->sizeHint());
               grid->addWidget(pb, rowNumber, BUTTON_COLUMN);
               fastOpenButtonGroup->addButton(pb, fastOpenButtonGroup->buttons().count());
               break;
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
               if (fileEntry->files[j].selected == SpecFile::SPEC_FALSE) {
                  continue;
               }
               break;
         }

         QPushButton* infoButton = new QPushButton("?");
         infoButton->setAutoDefault(false);
         QSize infoButtonSize = infoButton->sizeHint();
         infoButtonSize.setWidth(40);
         infoButton->setFixedSize(infoButtonSize);
         infoButton->setToolTip("View/Edit Comment Information");
         grid->addWidget(infoButton, rowNumber, INFO_COLUMN);
         infoButtonGroup->addButton(infoButton, infoButtonGroup->buttons().count());
         
         QPushButton* deleteButton = new QPushButton("X");
         deleteButton->setAutoDefault(false);
         QSize deleteButtonSize = deleteButton->sizeHint();
         deleteButtonSize.setWidth(40);
         deleteButton->setFixedSize(deleteButtonSize);
         deleteButton->setToolTip("Remove File From Spec File");
         grid->addWidget(deleteButton, rowNumber, DELETE_COLUMN);
         deleteButtonGroup->addButton(deleteButton, deleteButtonGroup->buttons().count());
         
         QLabel* tl = new QLabel(typeLabel);
         tl->setFixedSize(tl->sizeHint());
         grid->addWidget(tl, rowNumber, TYPE_COLUMN);

         QLabel* fn = new QLabel(prepareFileName(fileEntry->files[j].filename));
         fn->setFixedSize(fn->sizeHint());
         grid->addWidget(fn, rowNumber, NAME_COLUMN);
         
         selectionItems.push_back(SelectionItem(group, cb, pb, infoButton, deleteButton, tl, fn,
                                  &(fileEntry->files[j].selected), specFileTag, &(fileEntry->files[j].filename), 
                                  NULL, false));
         
         rowNumber++;
      }
   }
   group->setFixedSize(group->sizeHint());
   
   layout->addWidget(group);
   
   return group;
}


/**
 * List files for selection by the user.
 */
QGroupBox*
GuiSpecFileDialogMainWindow::listFiles(QVBoxLayout* layout,
                             const QString& title,
                             AbstractFile* dataFile,
                             const QString& specFileTag,
                             SpecFile::Entry& dataFileEntry,
                             const bool allowCommentEditing,
                             const bool volumeFileFlag)
{
   const int numRows = static_cast<int>(dataFileEntry.getNumberOfFiles());

   if (numRows == 0) {
      return NULL;
   }
   
   switch (dialogMode) {
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
         break;
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_DATA_FILE:
         break;
      case GuiSpecFileDialog::SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
         if (dataFileEntry.getNumberOfFilesSelected() <= 0) {
            return NULL;
         }
         break;
   }
   
   QGroupBox* group = new QGroupBox(title);
   QGridLayout* grid = new QGridLayout(group);
   
   int numColumns = 0;
   const int BUTTON_COLUMN = numColumns++;
   const int INFO_COLUMN   = numColumns++;
   const int DELETE_COLUMN = numColumns++;
   const int NAME_COLUMN   = numColumns++;
   
   
   for (int i = 0; i < numRows; i++) {
      QCheckBox* cb = NULL;
      QPushButton* pb = NULL;
      switch (dialogMode) {
         case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
            cb = new QCheckBox(" ");
            cb->setFixedSize(cb->sizeHint());
            cb->setChecked(dataFileEntry.files[i].selected);
            grid->addWidget(cb, i, BUTTON_COLUMN);
            break;
         case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_DATA_FILE:
            pb = new QPushButton("Open");
            pb->setAutoDefault(false);
            pb->setFixedSize(pb->sizeHint());
            grid->addWidget(pb, i, BUTTON_COLUMN);
            fastOpenButtonGroup->addButton(pb, fastOpenButtonGroup->buttons().count());
            break;
         case GuiSpecFileDialog::SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
            if (dataFileEntry.files[i].selected == SpecFile::SPEC_FALSE) {
               continue;
            }
            break;
      }
      
      QPushButton* infoButton = NULL;
      infoButton = new QPushButton("?");
      infoButton->setAutoDefault(false);
      QSize infoButtonSize = infoButton->sizeHint();
      infoButtonSize.setWidth(40);
      infoButton->setFixedSize(infoButtonSize);
      infoButton->setToolTip("View/Edit Comment Information");
      grid->addWidget(infoButton, i, INFO_COLUMN);
      infoButtonGroup->addButton(infoButton, infoButtonGroup->buttons().count());
      if (allowCommentEditing == false) {
         infoButton->setEnabled(false);
      }
      
      QPushButton* deleteButton = new QPushButton("X");
      deleteButton->setAutoDefault(false);
      QSize deleteButtonSize = deleteButton->sizeHint();
      deleteButtonSize.setWidth(40);
      deleteButton->setFixedSize(deleteButtonSize);
      deleteButton->setToolTip("Remove File From Spec File");
      grid->addWidget(deleteButton, i, DELETE_COLUMN);
      deleteButtonGroup->addButton(deleteButton, deleteButtonGroup->buttons().count());
         
      QLabel* fn = new QLabel(prepareFileName(dataFileEntry.files[i].filename));
      fn->setFixedSize(fn->sizeHint());
      grid->addWidget(fn, i, NAME_COLUMN);
      
      selectionItems.push_back(SelectionItem(group, cb, pb, infoButton, deleteButton, NULL, fn,
                                             &dataFileEntry.files[i].selected, 
                                             specFileTag, 
                                             &dataFileEntry.files[i].filename, 
                                             dataFile,
                                             volumeFileFlag));
   }
   group->setFixedSize(group->sizeHint());
   
   layout->addWidget(group);
   
   return group;
}

/**
 * Change the file name so that the path is listed after the file.
 * Ex:  "/usr/local/file.data"   becomes   "file.data (/usr/local)"
 */
QString
GuiSpecFileDialogMainWindow::prepareFileName(const QString& nameIn)
{
   QString name;
   if (checkForMissingFiles) {
      if (QFile::exists(nameIn) == false) {
         name.append("(FILE NOT FOUND) ");
      }
   }
   name.append(FileUtilities::rearrangeFileName(nameIn));
   return name;
}

/**
 * Called when an info button is pressed
 */
void
GuiSpecFileDialogMainWindow::infoButtonGroupSlot(int buttonNumber)
{
   if (buttonNumber < static_cast<int>(selectionItems.size())) {
      QString fileName(*(selectionItems[buttonNumber].fileName));
      const bool volumeFileFlag = selectionItems[buttonNumber].volumeFlag;
      if (fileName.length() > 0) {
         switch (dialogMode) {
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_SPEC_FILE:
               if (fileName[0] != '/') {
                  QString s(specFile.getFileNamePath());
                  if (s.isEmpty() == false) {
                     s.append("/");
                  }
                  s.append(fileName);
                  fileName = s;
               }
               break;
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_DATA_FILE:
               break;
            case GuiSpecFileDialog::SPEC_DIALOG_MODE_VIEW_CURRENT_FILES:
               break;
         }
         
         GuiDataFileCommentDialog* dfcd = new GuiDataFileCommentDialog(this, fileName, 
                                                                       volumeFileFlag);
         dfcd->show();
      }
   }
}
 
/**
 * Called when a delete button is pressed
 */
void
GuiSpecFileDialogMainWindow::deleteButtonGroupSlot(int buttonNumber)
{
   if (buttonNumber < static_cast<int>(selectionItems.size())) {
      SelectionItem& s = selectionItems[buttonNumber];
      QString fileName(*(s.fileName));
      if (fileName.isEmpty()) {
         return;
      }
      
      //
      // If filename was already "removed" but user has pressed the "X" button again
      // it means the user wants to restore the file.  A "removed" file name starts
      // with a "*".
      //
      const bool restoringFile = (fileName[0] == '*');
      
      if (s.checkbox != NULL) {
         s.checkbox->setEnabled(restoringFile);
         s.checkbox->setChecked(false);
      }
      if (s.pushbutton != NULL) s.pushbutton->setEnabled(restoringFile);
      if (s.infobutton != NULL) s.infobutton->setEnabled(restoringFile);
      if (s.typelabel != NULL) s.typelabel->setEnabled(restoringFile);
      if (s.namelabel != NULL) s.namelabel->setEnabled(restoringFile);
      
      //
      // Set item deselected.
      //
      *(s.selected) = SpecFile::SPEC_FALSE;
      
      if (restoringFile) {
         //
         // Remove the first character which is a "*"
         //
         (*(s.fileName)) = fileName.mid(1);
         
         //
         // Update the tooltip
         //
         s.deletebutton->setToolTip("Press this button\n"
                                       "to remove the file\n"
                                       "from the spec file.");
      }
      else {
         //
         // Prefix the file name with a "*" which tells the spec file class
         // not to write it to the spec file.
         //
         QString newName("*");
         newName.append(fileName);
         (*(s.fileName)) = newName;

         //
         // Update the tooltip
         //
         s.deletebutton->setToolTip("Press this button\n"
                                       "to restore the file\n"
                                       "to the spec file.");
      }
      
      //
      // Spec file has been modified
      //
      specFileNeedsToBeWritten = true;
   }
}
 
/**
 * Called when a fast open button is pressed.
 */
void
GuiSpecFileDialogMainWindow::fastOpenButtonSlot(int buttonNumber)
{
   bool error = false;
   
   BrainSet* bs = theMainWindow->getBrainSet();
   
   if (buttonNumber < static_cast<int>(selectionItems.size())) {
      SelectionItem& s = selectionItems[buttonNumber];
      
      //
      // Coord/topo always append.  Ask about others unless atlas file.
      //
      bool askFlag = true;
      bool appendFile = true;
      if ((s.specFileTag == SpecFile::getRawCoordFileTag()) ||
          (s.specFileTag == SpecFile::getFiducialCoordFileTag()) ||
          (s.specFileTag == SpecFile::getInflatedCoordFileTag()) ||
          (s.specFileTag == SpecFile::getVeryInflatedCoordFileTag()) ||
          (s.specFileTag == SpecFile::getSphericalCoordFileTag()) ||
          (s.specFileTag == SpecFile::getEllipsoidCoordFileTag()) ||
          (s.specFileTag == SpecFile::getCompressedCoordFileTag()) ||
          (s.specFileTag == SpecFile::getFlatCoordFileTag()) ||
          (s.specFileTag == SpecFile::getLobarFlatCoordFileTag()) ||
          (s.specFileTag == SpecFile::getUnknownCoordFileMatchTag()) ||
          (s.specFileTag == SpecFile::getClosedTopoFileTag()) ||
          (s.specFileTag == SpecFile::getOpenTopoFileTag()) ||
          (s.specFileTag == SpecFile::getCutTopoFileTag()) ||
          (s.specFileTag == SpecFile::getLobarCutTopoFileTag()) ||
          (s.specFileTag == SpecFile::getUnknownTopoFileMatchTag())) {
         askFlag = false;
      }
      //
      // Paint, Metric, and Surface shape get special dialog for appending
      //
      else if ((s.specFileTag == SpecFile::getArealEstimationFileTag()) ||
               (s.specFileTag == SpecFile::getPaintFileTag()) ||
               (s.specFileTag == SpecFile::getMetricFileTag()) ||
               (s.specFileTag == SpecFile::getSurfaceShapeFileTag())) {
         askFlag = false;
      }
      else if (s.specFileTag == SpecFile::getAtlasFileTag()) {
         askFlag = false;
         appendFile = false;
      }
      else {
         //
         // Only ask if the data file contains data
         //
         AbstractFile* dataFile = s.dataFile;
         if (dataFile != NULL) {
            askFlag = (dataFile->empty() == false);
         }
         else if ((s.specFileTag == SpecFile::getBorderProjectionFileTag()) ||
                  (s.specFileTag.indexOf(SpecFile::getUnknownBorderFileMatchTag()) >= 0)) {
            BrainModelBorderSet* bmbs = bs->getBorderSet();
            askFlag = (bmbs->getNumberOfBorders());
         }
         else if (s.specFileTag == SpecFile::getCerebralHullFileTag()) {
            askFlag = (bs->getCerebralHullFileName().isEmpty() == false);
         }
         else if (s.specFileTag == SpecFile::getDeformationMapFileTag()) {
            askFlag = (bs->getDeformationMapFileName().isEmpty() == false);
         }
         else if (s.specFileTag == SpecFile::getImageFileTag()) {
            askFlag = (bs->getNumberOfImageFiles() > 0);
         }
         else if (s.specFileTag == SpecFile::getTransformationDataFileTag()) {
            askFlag = (bs->getNumberOfTransformationDataFiles() > 0);
         }
         else if (s.specFileTag == SpecFile::getVolumeAnatomyFileTag()) {
            askFlag = (bs->getNumberOfVolumeAnatomyFiles() > 0);
         }
         else if (s.specFileTag == SpecFile::getVolumeFunctionalFileTag()) {
            askFlag = (bs->getNumberOfVolumeFunctionalFiles() > 0);
         }
         else if (s.specFileTag == SpecFile::getVolumePaintFileTag()) {
            askFlag = (bs->getNumberOfVolumePaintFiles() > 0);
         }
         else if (s.specFileTag == SpecFile::getVolumeProbAtlasFileTag()) {
            askFlag = (bs->getNumberOfVolumeProbAtlasFiles() > 0);
         }
         else if (s.specFileTag == SpecFile::getVolumeRgbFileTag()) {
            askFlag = (bs->getNumberOfVolumeRgbFiles() > 0);
         }
         else if (s.specFileTag == SpecFile::getVolumeSegmentationFileTag()) {
            askFlag = (bs->getNumberOfVolumeSegmentationFiles() > 0);
         }
         else if (s.specFileTag == SpecFile::getVolumeVectorFileTag()) {
            askFlag = (bs->getNumberOfVolumeVectorFiles() > 0);
         }
      }
      
      if (askFlag) {
         QMessageBox msgBox(this);
         msgBox.setWindowTitle("Append or Replace");
         msgBox.setText("Append to currently loaded file ?");
         QPushButton* appendPushButton = msgBox.addButton("Append",
                                                        QMessageBox::AcceptRole);
         QPushButton* replacePushButton = msgBox.addButton("Replace",
                                                        QMessageBox::AcceptRole);
         msgBox.addButton("Cancel", QMessageBox::RejectRole);
         msgBox.exec();
         if (msgBox.clickedButton() == appendPushButton) {
            appendFile = true;
         }
         else if (msgBox.clickedButton() == replacePushButton) {
            appendFile = false;
         }
         else {
            return;
         }
      }
      
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   
      QString msg1;
      bool relatedFileWarning;
      error = GuiDataFileOpenDialog::openDataFile(this, s.specFileTag, *(s.fileName), appendFile, 
                                                  true, msg1, relatedFileWarning);
      if (error) {
         QApplication::restoreOverrideCursor();
         QMessageBox::critical(this, "Error Opening File", msg1);
         return;
      }
      else if (relatedFileWarning) {
         QApplication::restoreOverrideCursor();  
         return; 
      }
   }
   QApplication::restoreOverrideCursor();   
   
   emit signalCloseDialogRequested();
}
