
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

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QTextEdit>

#include "AreaColorFile.h"
#include "ArealEstimationFile.h"
#include "BorderColorFile.h"
#include "BorderProjectionFile.h"
#include "BrainModelBorderSet.h"
#include "BrainModelContours.h"
#include "BrainSet.h"
#include "CellColorFile.h"
#include "CellFile.h"
#include "CellProjectionFile.h"
#include "CocomacConnectivityFile.h"
#include "ContourCellColorFile.h"
#include "ContourCellFile.h"
#include "ContourFile.h"
#include "CutsFile.h"
#include "DeformationFieldFile.h"
#include "DeformationMapFile.h"
#include "FileFilters.h"
#include "FileUtilities.h"
#include "FociColorFile.h"
#include "FociFile.h"
#include "FociProjectionFile.h"
#include "FociSearchFile.h"
#include "GeodesicDistanceFile.h"
#include "GiftiDataArrayFile.h"
#include "GuiBrainModelSelectionComboBox.h"
#include "GuiDataFileSaveDialog.h"
#include "GuiFilesModified.h"
#include "GuiMainWindow.h"
#include "GuiNodeAttributeColumnSelectionComboBox.h"
#include "GuiSpecFileCreationDialog.h"
#include "GuiStructureComboBox.h"
#include "GuiSurfaceTypeComboBox.h"
#include "GuiTopologyFileComboBox.h"
#include "GuiTopologyTypeComboBox.h"
#include "GuiVectorFileComboBox.h"
#include "GuiVolumeFileSelectionComboBox.h"
#include "GuiVolumeSelectionControl.h"
#include "GuiVolumeVoxelDataTypeComboBox.h"
#include "ImageFile.h"
#include "LatLonFile.h"
#include "MetricFile.h"
#include "PaletteFile.h"
#include "ParamsFile.h"
#include "PreferencesFile.h"
#include "ProbabilisticAtlasFile.h"
#include "RgbPaintFile.h"
#include "SectionFile.h"
#include "StereotaxicSpace.h"
#include "StudyCollectionFile.h"
#include "StudyMetaDataFile.h"
#include "SurfaceShapeFile.h"
#include "VectorFile.h"
#include "TopographyFile.h"
#include "TopologyFile.h"
#include "TrajectoryFile.h"
#include "QtMultipleInputDialog.h"
#include "QtUtilities.h"
#include "VocabularyFile.h"
#include "VtkModelFile.h"
#include "WuQFileDialog.h"
#include "WuQWidgetGroup.h"
#include "WustlRegionFile.h"
#include "global_variables.h"

static const bool alwaysSaveAllProbAtlasVolumes = true;

/**
 * constructor.
 */
GuiDataFileSaveDialog::GuiDataFileSaveDialog(QWidget* parent)
   : WuQDialog(parent)
{
   fileOptionsWidgetGroup = NULL;
   setWindowTitle("Save Data File");
   
   //
   // Create the widgets for the different sections
   //
   QWidget* fileInfoWidget = createFileInformationSection();
   QWidget* metadataWidget = createMetadataSection();
   
   //
   // create group boxes for each of the file types
   //
   borderSurfaceOptionsGroupBox = createBorderSurfaceOptionsSection();
   borderProjectionOptionsGroupBox = createBorderProjectionOptionsSection();
   cellOptionsGroupBox = createCellOptionsSection();
   coordinateOptionsGroupBox = createCoordinateOptionsSection();
   fociOptionsGroupBox = createFociOptionsSection();
   imageOptionsGroupBox = createImageOptionsSection();
   topologyOptionsGroupBox = createTopologyOptionsSection();
   vectorOptionsGroupBox = createVectorOptionsSection();
   volumeAnatomyOptionsGroupBox = createVolumeAnatomyOptionsSection();
   volumeFunctionalOptionsGroupBox = createVolumeFunctionalOptionsSection();
   volumePaintOptionsGroupBox = createVolumePaintOptionsSection();
   volumeProbAtlasOptionsGroupBox = createVolumeProbAtlasOptionsSection();
   volumeRgbOptionsGroupBox = createVolumeRgbOptionsSection();
   volumeSegmentationOptionsGroupBox = createVolumeSegmentationOptionsSection();
   volumeVectorOptionsGroupBox = createVolumeVectorOptionsSection();
   vtkModelOptionsGroupBox = createVtkModelOptionsSection();
   exportSurfaceOptionsGroupBox = createExportSurfaceOptionsSection();
   exportVolumeOptionsGroupBox = createExportVolumeOptionsSection();
   exportMetricOptionsGroupBox = createMetricExportOptionsSection();
   exportPaintOptionsGroupBox  = createPaintExportOptionsSection();
   exportShapeOptionsGroupBox  = createShapeExportOptionsSection();
   giftiSurfaceOptionsGroupBox = createGiftiOptionsSection();
   
   //
   // Place all of the file option boxes into a widget group
   //
   fileOptionsWidgetGroup = new WuQWidgetGroup(this);
   fileOptionsWidgetGroup->addWidget(borderSurfaceOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(borderProjectionOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(cellOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(coordinateOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(fociOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(imageOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(topologyOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(vectorOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(volumeAnatomyOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(volumeFunctionalOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(volumePaintOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(volumeProbAtlasOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(volumeRgbOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(volumeSegmentationOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(volumeVectorOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(vtkModelOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(exportSurfaceOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(exportVolumeOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(exportMetricOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(exportPaintOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(exportShapeOptionsGroupBox);
   fileOptionsWidgetGroup->addWidget(giftiSurfaceOptionsGroupBox);
   
   //
   // Save button
   //
   savePushButton = new QPushButton("Save");
   savePushButton->setAutoDefault(true);
   savePushButton->setEnabled(false);
   QObject::connect(savePushButton, SIGNAL(clicked()),
                    this, SLOT(accept()));
                    
   //
   // Cancel button
   //
   QPushButton* cancelPushButton = new QPushButton("Cancel");
   cancelPushButton->setAutoDefault(false);
   QObject::connect(cancelPushButton, SIGNAL(clicked()),
                    this, SLOT(reject()));
                    
   QtUtilities::makeButtonsSameSize(savePushButton, cancelPushButton);
   
   //
   // Layout for buttons
   //
   QHBoxLayout* buttonsLayout = new QHBoxLayout;
   buttonsLayout->addWidget(savePushButton);
   buttonsLayout->addWidget(cancelPushButton);
   
   //
   // Layout for dialog
   //
   QVBoxLayout* dialogLayout = new QVBoxLayout(this);
   dialogLayout->addWidget(fileInfoWidget);
   dialogLayout->addWidget(metadataWidget);
   dialogLayout->addWidget(borderSurfaceOptionsGroupBox);
   dialogLayout->addWidget(borderProjectionOptionsGroupBox);
   dialogLayout->addWidget(cellOptionsGroupBox);
   dialogLayout->addWidget(coordinateOptionsGroupBox);
   dialogLayout->addWidget(fociOptionsGroupBox);
   dialogLayout->addWidget(imageOptionsGroupBox);
   dialogLayout->addWidget(topologyOptionsGroupBox);
   dialogLayout->addWidget(vectorOptionsGroupBox);
   dialogLayout->addWidget(volumeAnatomyOptionsGroupBox);
   dialogLayout->addWidget(volumeFunctionalOptionsGroupBox);
   dialogLayout->addWidget(volumePaintOptionsGroupBox);
   dialogLayout->addWidget(volumeProbAtlasOptionsGroupBox);
   dialogLayout->addWidget(volumeRgbOptionsGroupBox);
   dialogLayout->addWidget(volumeSegmentationOptionsGroupBox);
   dialogLayout->addWidget(volumeVectorOptionsGroupBox);
   dialogLayout->addWidget(vtkModelOptionsGroupBox);
   dialogLayout->addWidget(exportSurfaceOptionsGroupBox);
   dialogLayout->addWidget(exportVolumeOptionsGroupBox);
   dialogLayout->addWidget(exportMetricOptionsGroupBox);
   dialogLayout->addWidget(exportPaintOptionsGroupBox);
   dialogLayout->addWidget(exportShapeOptionsGroupBox);
   dialogLayout->addWidget(giftiSurfaceOptionsGroupBox);
   dialogLayout->addLayout(buttonsLayout);

   //
   // Add file filters to file type combo box
   //
   QStringList filterNames;
   filterNames << FileFilters::getAreaColorFileFilter();
   filterNames << FileFilters::getArealEstimationFileFilter();
   filterNames << FileFilters::getBorderGenericFileFilter();
   filterNames << FileFilters::getBorderVolumeFileFilter();
   filterNames << FileFilters::getBorderColorFileFilter();
   filterNames << FileFilters::getBorderProjectionFileFilter();
   filterNames << FileFilters::getCellFileFilter();
   filterNames << FileFilters::getCellColorFileFilter();
   filterNames << FileFilters::getCellProjectionFileFilter();
   filterNames << FileFilters::getCellVolumeFileFilter();
   filterNames << FileFilters::getCocomacFileFilter();
   filterNames << FileFilters::getContourFileFilter();
   filterNames << FileFilters::getContourCellFileFilter();
   filterNames << FileFilters::getContourCellColorFileFilter();
   filterNames << FileFilters::getCoordinateGenericFileFilter();
   filterNames << FileFilters::getCutsFileFilter();
   filterNames << FileFilters::getDeformationFieldFileFilter();
   filterNames << FileFilters::getFociFileFilter();
   filterNames << FileFilters::getFociColorFileFilter();
   filterNames << FileFilters::getFociProjectionFileFilter();
   filterNames << FileFilters::getFociSearchFileFilter();
   filterNames << FileFilters::getGeodesicDistanceFileFilter();
   if (GiftiDataArrayFile::getGiftiXMLEnabled()) {
      filterNames << FileFilters::getGiftiCoordinateFileFilter();
      filterNames << FileFilters::getGiftiFunctionalFileFilter();
      filterNames << FileFilters::getGiftiLabelFileFilter();
      filterNames << FileFilters::getGiftiShapeFileFilter();
      filterNames << FileFilters::getGiftiSurfaceFileFilter();
      filterNames << FileFilters::getGiftiTopologyFileFilter();
      filterNames << FileFilters::getGiftiVectorFileFilter();
   }
   filterNames << FileFilters::getImageSaveFileFilter();
   filterNames << FileFilters::getLatitudeLongitudeFileFilter();
   filterNames << FileFilters::getMetricFileFilter();
   filterNames << FileFilters::getPaintFileFilter();
   filterNames << FileFilters::getPaletteFileFilter();
   filterNames << FileFilters::getParamsFileFilter();
   filterNames << FileFilters::getProbAtlasFileFilter();
   filterNames << FileFilters::getRgbPaintFileFilter();
   filterNames << FileFilters::getSceneFileFilter();
   filterNames << FileFilters::getSectionFileFilter();
   filterNames << FileFilters::getStudyCollectionFileFilter();
   filterNames << FileFilters::getStudyMetaDataFileFilter();
   filterNames << FileFilters::getSurfaceShapeFileFilter();
   filterNames << FileFilters::getTopographyFileFilter();
   filterNames << FileFilters::getTopologyGenericFileFilter();
   filterNames << FileFilters::getTrajectoryFileFilter();
   filterNames << FileFilters::getTransformationMatrixFileFilter();
   filterNames << FileFilters::getVocabularyFileFilter();
   filterNames << FileFilters::getVolumeAnatomyFileFilter();
   filterNames << FileFilters::getVolumeFunctionalFileFilter();
   filterNames << FileFilters::getVolumePaintFileFilter();
   filterNames << FileFilters::getVolumeProbAtlasFileFilter();
   filterNames << FileFilters::getVolumeRgbFileFilter();
   filterNames << FileFilters::getVolumeSegmentationFileFilter();
   filterNames << FileFilters::getVolumeVectorFileFilter();
   filterNames << FileFilters::getVtkModelFileFilter();
   filterNames << FileFilters::getWustlRegionFileFilter();
   
   filterNames << FileFilters::getAnalyzeVolumeFileFilter();
   filterNames << FileFilters::getByuSurfaceFileFilter();
   filterNames << FileFilters::getFreeSurferAsciiSurfaceFileFilter();
   filterNames << FileFilters::getFreeSurferAsciiCurvatureFileFilter();
   filterNames << FileFilters::getFreeSurferAsciiFunctionalFileFilter();
   filterNames << FileFilters::getFreeSurferAsciiLabelFileFilter();
   filterNames << FileFilters::getMincVolumeFileFilter();
   filterNames << FileFilters::getOpenInventorSurfaceFileFilter();
   filterNames << FileFilters::getStlSurfaceFileFilter();
   filterNames << FileFilters::getVrmlSurfaceFileFilter();
   filterNames << FileFilters::getVtkSurfaceFileFilter();
   filterNames << FileFilters::getVtkXmlSurfaceFileFilter();
   filterNames << FileFilters::getVtkVolumeFileFilter();
   
   fileTypeComboBox->addItems(filterNames); 
   
   slotFileTypeComboBox(fileTypeComboBox->currentText());
   //updateGeometry();
}
                  
/**
 * destructor.
 */
GuiDataFileSaveDialog::~GuiDataFileSaveDialog()
{
}

/**
 * get the selected file name.
 */
QString 
GuiDataFileSaveDialog::getSelectedFileName() const
{
   return fileNameLineEdit->text();
}
      
/**
 * get the file filter.
 */
QString 
GuiDataFileSaveDialog::getFileTypeFilter() const
{
   const QString s(fileTypeComboBox->currentText());
   return s;
}
      
/**
 * select the file type for saving.
 */
void 
GuiDataFileSaveDialog::selectFileType(const QString& fileFilterName)
{
   bool found = false;
   for (int i = 0; i < fileTypeComboBox->count(); i++) {
      if (fileTypeComboBox->itemText(i) == fileFilterName) {
         fileTypeComboBox->setCurrentIndex(i);
         found = true;
         break;
      }
   }
   
   if (found == false) {
      fileTypeComboBox->addItem(fileFilterName);
      fileTypeComboBox->setCurrentIndex(fileTypeComboBox->count() - 1);
   }
}
      
/**
 * called when file name push button pressed.
 */
void 
GuiDataFileSaveDialog::slotFileNamePushButton()
{
   PreferencesFile* pf = theMainWindow->getBrainSet()->getPreferencesFile();
   
   WuQFileDialog fd(this);
   fd.setHistory(pf->getRecentDataFileDirectories());
   fd.setWindowTitle("Select File Name");
   fd.setDirectory(QDir::currentPath());
   fd.setFilters(QStringList(fileTypeComboBox->currentText()));
   fd.setFileMode(WuQFileDialog::AnyFile);
   fd.setAcceptMode(WuQFileDialog::AcceptSave);
   fd.setConfirmOverwrite(false);
   if (fd.exec() == Accepted) { 
      if (fd.selectedFiles().count() > 0) {
         const QString name = fd.selectedFiles().at(0);
         fileNameLineEdit->setText(name);
      }                      
   }            
}
      
/**
 * create the border options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createBorderSurfaceOptionsSection()
{
   //
   // Associated surface
   //
   QLabel* surfaceSelectionLabel = new QLabel("Associated Surface");
   borderSurfaceSelectionComboBox = 
      new GuiBrainModelSelectionComboBox(false,
                                         true,
                                         false,
                                         "");
   borderSurfaceSelectionComboBox->setSelectedBrainModel(theMainWindow->getBrainModelSurface());
   QObject::connect(borderSurfaceSelectionComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotLoadParametersForFileType()));
                                         
   //
   // Surface type
   //
   QLabel* surfaceTypeLabel = new QLabel("Save As Type"); 
   borderSurfaceTypeComboBox = new GuiSurfaceTypeComboBox(false);
   
   //
   // Remove duplicates check box
   //
   borderSurfaceRemoveDuplicatesCheckBox = new QCheckBox("Remove Duplicates");
   borderSurfaceRemoveDuplicatesCheckBox->setChecked(true);

   //
   // Create group box and layout
   //
   QGroupBox* groupBox = new QGroupBox("Border (Surface) Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(surfaceSelectionLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(borderSurfaceSelectionComboBox, 0, 1);
   gridLayout->addWidget(surfaceTypeLabel, 1, 0, Qt::AlignLeft);
   gridLayout->addWidget(borderSurfaceTypeComboBox, 1, 1);
   gridLayout->addWidget(borderSurfaceRemoveDuplicatesCheckBox, 2, 0, 1, 2, Qt::AlignLeft);
   gridLayout->setColumnStretch(0, 0);
   gridLayout->setColumnStretch(1, 100);
   return groupBox;
}
      
/**
 * create the border projection options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createBorderProjectionOptionsSection()
{
   //
   // Remove duplicates check box
   //
   borderProjectionsRemoveDuplicatesCheckBox = new QCheckBox("Remove Duplicates");
   borderProjectionsRemoveDuplicatesCheckBox->setChecked(true);

   QGroupBox* groupBox = new QGroupBox("Border Projection Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(borderProjectionsRemoveDuplicatesCheckBox, 0, 0, Qt::AlignLeft);
   return groupBox;
}
      
/**
 * create the cell options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createCellOptionsSection()
{
   //
   // Surface type
   //
   QLabel* cellTypeLabel = new QLabel("Cells Associated With Surface"); 
   cellSurfaceSelectionComboBox = new GuiBrainModelSelectionComboBox(false, true, false, "");
   cellSurfaceSelectionComboBox->setSelectedBrainModel(theMainWindow->getBrainModelSurface());

   QGroupBox* groupBox = new QGroupBox("Cell Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(cellTypeLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(cellSurfaceSelectionComboBox, 0, 1);
   gridLayout->setColumnStretch(0, 0);
   gridLayout->setColumnStretch(1, 100);
   return groupBox;
}
      
/**
 * create the coordinate options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createCoordinateOptionsSection()
{
   //
   // Associated surface
   //
   QLabel* surfaceSelectionLabel = new QLabel("Associated Surface");
   coordinateSurfaceSelectionComboBox = 
      new GuiBrainModelSelectionComboBox(false,
                                         true,
                                         false,
                                         "");
   coordinateSurfaceSelectionComboBox->setSelectedBrainModel(theMainWindow->getBrainModelSurface());
   QObject::connect(coordinateSurfaceSelectionComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotLoadParametersForFileType()));
                                         
   //
   // Surface type
   //
   QLabel* surfaceTypeLabel = new QLabel("Save As Type"); 
   coordinateTypeComboBox = new GuiSurfaceTypeComboBox(false);
   
   //
   // Structure
   //
   QLabel* structureLabel = new QLabel("Structure");
   coordinateStructureComboBox = new GuiStructureComboBox(0, 0, true);

   //
   // Stereotaxic space
   //
   QLabel* stereotaxicSpaceLabel = new QLabel("Stereotaxic Space");
   coordinateStereotaxicSpaceComboBox = new QComboBox();
   coordinateStereotaxicSpaceComboBox->addItem("AC-PC");
   coordinateStereotaxicSpaceComboBox->addItem("Anterior Commissure");
   coordinateStereotaxicSpaceComboBox->addItem("Cartesian Standard");
   coordinateStereotaxicSpaceComboBox->addItem("Cartesian Non-Standard");
   coordinateStereotaxicSpaceComboBox->addItem("Center of Gravity");
   coordinateStereotaxicSpaceComboBox->addItem("Native");
   coordinateStereotaxicSpaceComboBox->addItem("Spherical Standard");
   coordinateStereotaxicSpaceComboBox->addItem("Talairach");
   coordinateStereotaxicSpaceComboBox->addItem("Unspecified");
   std::vector<StereotaxicSpace> allSpaces;
   StereotaxicSpace::getAllStereotaxicSpaces(allSpaces);
   for (unsigned int j = 0; j < allSpaces.size(); j++) {
       coordinateStereotaxicSpaceComboBox->addItem(allSpaces[j].getName());
   }
   coordinateStereotaxicSpaceComboBox->setToolTip(
                 "This selects the the coordinate frame for\n"
                 "the coordinate file that is being saved.");
   
   //
   // Create group box and layout
   //
   QGroupBox* groupBox = new QGroupBox("Coordinate Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(surfaceSelectionLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(coordinateSurfaceSelectionComboBox, 0, 1);
   gridLayout->addWidget(surfaceTypeLabel, 1, 0, Qt::AlignLeft);
   gridLayout->addWidget(coordinateTypeComboBox, 1, 1);
   gridLayout->addWidget(structureLabel, 2, 0, Qt::AlignLeft);
   gridLayout->addWidget(coordinateStructureComboBox, 2, 1);
   gridLayout->addWidget(stereotaxicSpaceLabel, 3, 0, Qt::AlignLeft);
   gridLayout->addWidget(coordinateStereotaxicSpaceComboBox, 3, 1);
   gridLayout->setColumnStretch(0, 0);
   gridLayout->setColumnStretch(1, 100);
   return groupBox;
}
      
/**
 * create the foci options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createFociOptionsSection()
{
   //
   // Save Coordinates
   //
   fociSaveOriginalCoordinatesRadioButton = new QRadioButton("Save Original Coordinates");
   fociSaveProjectedCoordinatesRadioButton = new QRadioButton("Save Projected Coordinates");
   QButtonGroup* buttGroup = new QButtonGroup(this);
   buttGroup->addButton(fociSaveOriginalCoordinatesRadioButton);
   buttGroup->addButton(fociSaveProjectedCoordinatesRadioButton);
   fociSaveOriginalCoordinatesRadioButton->setChecked(true);
   
   //
   // left surface
   //
   QLabel* leftSurfaceSelectionLabel = new QLabel("Left Surface");
   fociLeftSurfaceSelectionComboBox = 
      new GuiBrainModelSelectionComboBox(false,
                                         true,
                                         false,
                                         "");
   fociLeftSurfaceSelectionComboBox->setSelectedBrainModel(theMainWindow->getBrainModelSurface());

   //
   // right surface
   //
   QLabel* rightSurfaceSelectionLabel = new QLabel("Right Surface");
   fociRightSurfaceSelectionComboBox = 
      new GuiBrainModelSelectionComboBox(false,
                                         true,
                                         false,
                                         "");
   fociRightSurfaceSelectionComboBox->setSelectedBrainModel(theMainWindow->getBrainModelSurface());

   //
   // right surface
   //
   QLabel* cerebellumSurfaceSelectionLabel = new QLabel("Cerebellum Surface");
   fociCerebellumSurfaceSelectionComboBox = 
      new GuiBrainModelSelectionComboBox(false,
                                         true,
                                         false,
                                         "");
   fociCerebellumSurfaceSelectionComboBox->setSelectedBrainModel(theMainWindow->getBrainModelSurface());

   QGroupBox* groupBox = new QGroupBox("Foci Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(fociSaveOriginalCoordinatesRadioButton, 0, 0, 1, 1, Qt::AlignLeft);
   gridLayout->addWidget(fociSaveProjectedCoordinatesRadioButton, 1, 0, 1, 1, Qt::AlignLeft);
   gridLayout->addWidget(leftSurfaceSelectionLabel, 2, 0, Qt::AlignLeft);
   gridLayout->addWidget(fociLeftSurfaceSelectionComboBox, 2, 1);
   gridLayout->addWidget(rightSurfaceSelectionLabel, 3, 0, Qt::AlignLeft);
   gridLayout->addWidget(fociRightSurfaceSelectionComboBox, 3, 1);
   gridLayout->addWidget(cerebellumSurfaceSelectionLabel, 4, 0, Qt::AlignLeft);
   gridLayout->addWidget(fociCerebellumSurfaceSelectionComboBox, 4, 1);
   gridLayout->setColumnStretch(0, 0);
   gridLayout->setColumnStretch(1, 100);
   return groupBox;
}
      
/**
 * select the image file for saving.
 */
void 
GuiDataFileSaveDialog::selectImageFile(const ImageFile* imageFile)
{
   BrainSet* brainSet = theMainWindow->getBrainSet();
   const int numImages = brainSet->getNumberOfImageFiles();
   for (int i = 0; i < numImages; i++) {
      if (brainSet->getImageFile(i) == imageFile) {
         if (i < imageSelectionComboBox->count()) {
            imageSelectionComboBox->setCurrentIndex(i);
            break;
         }
      }
   }
}

/**
 * select a file.
 */
void 
GuiDataFileSaveDialog::selectFile(AbstractFile* af)
{
   BrainSet* bs = theMainWindow->getBrainSet();

   QString fileFilterName;
   if (dynamic_cast<AreaColorFile*>(af) != NULL) {
      fileFilterName = FileFilters::getAreaColorFileFilter();
   }
   else if (dynamic_cast<ArealEstimationFile*>(af) != NULL) {
      fileFilterName = FileFilters::getArealEstimationFileFilter();
   }
   else if (dynamic_cast<BorderColorFile*>(af) != NULL) {
      fileFilterName = FileFilters::getBorderColorFileFilter();
   }
   else if (dynamic_cast<BorderFile*>(af) != NULL) {
      fileFilterName = FileFilters::getBorderGenericFileFilter();
   }
   else if (dynamic_cast<BorderProjectionFile*>(af) != NULL) {
      fileFilterName = FileFilters::getBorderProjectionFileFilter();
   }
   else if (dynamic_cast<CellColorFile*>(af) != NULL) {
      fileFilterName = FileFilters::getCellColorFileFilter();
   }
   else if (dynamic_cast<CutsFile*>(af) != NULL) {
      //
      // CUTS MUST BE BEFORE CELL SINCE CUTS DERIVED FROM CELL
      //
      fileFilterName = FileFilters::getCutsFileFilter();
   }
   else if (dynamic_cast<FociFile*>(af) != NULL) {
      //
      // FOCI MUST BE BEFORE CELL SINCE FOCI DERIVED FROM CELL
      //
      fileFilterName = FileFilters::getFociFileFilter();
   }
   else if (dynamic_cast<ContourCellFile*>(af) != NULL) {
      //
      // CONTOUR CELL MUST BE BEFORE CELL SINCE CONTOUR CELL DERIVED FROM CELL
      //
      fileFilterName = FileFilters::getContourCellFileFilter();
   }
   else if (dynamic_cast<CellFile*>(af) != NULL) {
      fileFilterName = FileFilters::getCellFileFilter();
   }
   else if (dynamic_cast<FociProjectionFile*>(af) != NULL) {
      //
      // FOCI MUST BE BEFORE CELL SINCE FOCI DERIVED FROM CELL
      //
      fileFilterName = FileFilters::getFociProjectionFileFilter();
   }
   else if (dynamic_cast<CellProjectionFile*>(af) != NULL) {
      fileFilterName = FileFilters::getCellProjectionFileFilter();
   }
   else if (dynamic_cast<CocomacConnectivityFile*>(af) != NULL) {
      fileFilterName = FileFilters::getCocomacFileFilter();
   }
   else if (dynamic_cast<ContourCellColorFile*>(af) != NULL) {
      fileFilterName = FileFilters::getContourCellColorFileFilter();
   }
   else if (dynamic_cast<ContourFile*>(af) != NULL) {
      fileFilterName = FileFilters::getContourFileFilter();
   }
   else if (dynamic_cast<CoordinateFile*>(af) != NULL) {
      fileFilterName = FileFilters::getCoordinateGenericFileFilter();
      for (int i = 0; i < bs->getNumberOfBrainModels(); i++) {
         BrainModelSurface* bms = bs->getBrainModelSurface(i);
         if (bms != NULL) {
            if (bms->getCoordinateFile() == af) {
               coordinateSurfaceSelectionComboBox->setSelectedBrainModel(bms);
               break;
            }
         }
      }
   }
   else if (dynamic_cast<DeformationFieldFile*>(af) != NULL) {
      fileFilterName = FileFilters::getDeformationFieldFileFilter();
   }
   else if (dynamic_cast<DeformationMapFile*>(af) != NULL) {
      fileFilterName = FileFilters::getDeformationMapFileFilter();
   }
   else if (dynamic_cast<FociColorFile*>(af) != NULL) {
      fileFilterName = FileFilters::getFociColorFileFilter();
   }
   else if (dynamic_cast<FociSearchFile*>(af) != NULL) {
      fileFilterName = FileFilters::getFociSearchFileFilter();
   }
   else if (dynamic_cast<GeodesicDistanceFile*>(af) != NULL) {
      fileFilterName = FileFilters::getGeodesicDistanceFileFilter();
   }
   else if (dynamic_cast<ImageFile*>(af) != NULL) {
      fileFilterName = FileFilters::getImageSaveFileFilter();
      selectImageFile(dynamic_cast<ImageFile*>(af));
   }
   else if (dynamic_cast<LatLonFile*>(af) != NULL) {
      fileFilterName = FileFilters::getLatitudeLongitudeFileFilter();
   }
   else if (dynamic_cast<SurfaceShapeFile*>(af) != NULL) {
      //
      // SHAPE MUST BE BEFORE METRIC SINCE SHAPE DERIVED FROM METRIC
      //
      fileFilterName = FileFilters::getSurfaceShapeFileFilter();
   }
   else if (dynamic_cast<MetricFile*>(af) != NULL) {
      fileFilterName = FileFilters::getMetricFileFilter();
   }
   else if (dynamic_cast<ProbabilisticAtlasFile*>(af) != NULL) {
      //
      // PROB ATLAS MUST BE BEFORE PAINT SINCE PROB ATLAS DERIVED FROM PAINT
      //
      fileFilterName = FileFilters::getProbAtlasFileFilter();
   }
   else if (dynamic_cast<PaintFile*>(af) != NULL) {
      fileFilterName = FileFilters::getPaintFileFilter();
   }
   else if (dynamic_cast<PaletteFile*>(af) != NULL) {
      fileFilterName = FileFilters::getPaletteFileFilter();
   }
   else if (dynamic_cast<ParamsFile*>(af) != NULL) {
      fileFilterName = FileFilters::getParamsFileFilter();
   }
   else if (dynamic_cast<RgbPaintFile*>(af) != NULL) {
      fileFilterName = FileFilters::getRgbPaintFileFilter();
   }
   else if (dynamic_cast<SceneFile*>(af) != NULL) {
      fileFilterName = FileFilters::getSceneFileFilter();
   }
   else if (dynamic_cast<SectionFile*>(af) != NULL) {
      fileFilterName = FileFilters::getSectionFileFilter();
   }
   else if (dynamic_cast<StudyCollectionFile*>(af) != NULL) {
      fileFilterName = FileFilters::getStudyCollectionFileFilter();
   }
   else if (dynamic_cast<StudyMetaDataFile*>(af) != NULL) {
      fileFilterName = FileFilters::getStudyMetaDataFileFilter();
   }
   else if (dynamic_cast<VectorFile*>(af) != NULL) {
      fileFilterName = FileFilters::getGiftiVectorFileFilter();
      VectorFile* vf = dynamic_cast<VectorFile*>(af);
      vectorFileSelectionComboBox->setSelectedVectorFile(vf);
   }
   else if (dynamic_cast<TopographyFile*>(af) != NULL) {
      fileFilterName = FileFilters::getTopographyFileFilter();
   }
   else if (dynamic_cast<TopologyFile*>(af) != NULL) {
      fileFilterName = FileFilters::getTopologyGenericFileFilter();
      topologySelectionComboBox->setSelectedTopologyFile(dynamic_cast<TopologyFile*>(af));
   }
   else if (dynamic_cast<TransformationMatrixFile*>(af) != NULL) {
      fileFilterName = FileFilters::getTransformationMatrixFileFilter();
   }
   else if (dynamic_cast<VocabularyFile*>(af) != NULL) {
      fileFilterName = FileFilters::getVocabularyFileFilter();
   }
   else if (dynamic_cast<VolumeFile*>(af) != NULL) {
      fileFilterName = FileFilters::getVolumeGenericFileFilter();
      bool gotItFlag = false;
      VolumeFile* vf = dynamic_cast<VolumeFile*>(af);
      for (int i = 0; i < bs->getNumberOfVolumeAnatomyFiles(); i++) {
         if (bs->getVolumeAnatomyFile(i) == vf) {
            volumeAnatomyFileSelectionComboBox->setSelectedVolumeFile(vf);
            fileFilterName = FileFilters::getVolumeAnatomyFileFilter();
            gotItFlag = true;
            break;
         }
      }
      
      if (gotItFlag == false) {
         for (int i = 0; i < bs->getNumberOfVolumeFunctionalFiles(); i++) {
            if (bs->getVolumeFunctionalFile(i) == vf) {
               volumeFunctionalFileSelectionComboBox->setSelectedVolumeFile(vf);
               fileFilterName = FileFilters::getVolumeFunctionalFileFilter();
               gotItFlag = true;
               break;
            }
         }
      }
      
      if (gotItFlag == false) {
         for (int i = 0; i < bs->getNumberOfVolumePaintFiles(); i++) {
            if (bs->getVolumePaintFile(i) == vf) {
               volumePaintFileSelectionComboBox->setSelectedVolumeFile(vf);
               fileFilterName = FileFilters::getVolumePaintFileFilter();
               gotItFlag = true;
               break;
            }
         }
      }
      
      if (gotItFlag == false) {
         for (int i = 0; i < bs->getNumberOfVolumeProbAtlasFiles(); i++) {
            if (bs->getVolumeProbAtlasFile(i) == vf) {
               volumeProbAtlasFileSelectionComboBox->setSelectedVolumeFile(vf);
               fileFilterName = FileFilters::getVolumeProbAtlasFileFilter();
               gotItFlag = true;
               break;
            }
         }
      }
      
      if (gotItFlag == false) {
         for (int i = 0; i < bs->getNumberOfVolumeRgbFiles(); i++) {
            if (bs->getVolumeRgbFile(i) == vf) {
               volumeRgbFileSelectionComboBox->setSelectedVolumeFile(vf);
               fileFilterName = FileFilters::getVolumeRgbFileFilter();
               gotItFlag = true;
               break;
            }
         }
      }
      
      if (gotItFlag == false) {
         for (int i = 0; i < bs->getNumberOfVolumeSegmentationFiles(); i++) {
            if (bs->getVolumeSegmentationFile(i) == vf) {
               volumeSegmentationFileSelectionComboBox->setSelectedVolumeFile(vf);
               fileFilterName = FileFilters::getVolumeSegmentationFileFilter();
               gotItFlag = true;
               break;
            }
         }
      }
      
      if (gotItFlag == false) {
         for (int i = 0; i < bs->getNumberOfVolumeVectorFiles(); i++) {
            if (bs->getVolumeVectorFile(i) == vf) {
               volumeVectorFileSelectionComboBox->setSelectedVolumeFile(vf);
               fileFilterName = FileFilters::getVolumeVectorFileFilter();
               gotItFlag = true;
               break;
            }
         }
      }
   }
   else if (dynamic_cast<VtkModelFile*>(af) != NULL) {
      fileFilterName = FileFilters::getVtkModelFileFilter();
   }
   else if (dynamic_cast<WustlRegionFile*>(af) != NULL) {
      fileFilterName = FileFilters::getWustlRegionFileFilter();
   }      
   else {
      QMessageBox::critical(this,
                             "ERROR",
                             "PROGRAM ERROR: Unrecongnized file type for selection: "
                             + QString(af->getDescriptiveName()));
      return;
   }
   
   //
   // Select the file type
   //
   selectFileType(fileFilterName);
   slotLoadParametersForFileType();
}

/**
 * create the image options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createImageOptionsSection()
{
   BrainSet* brainSet = theMainWindow->getBrainSet();
   
   //
   // image combo box and label
   //
   QLabel* imageLabel = new QLabel("Image");
   imageSelectionComboBox = new QComboBox;
   QObject::connect(imageSelectionComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotLoadParametersForFileType()));
   for (int i = 0; i < brainSet->getNumberOfImageFiles(); i++) {
      imageSelectionComboBox->addItem(
         FileUtilities::basename(brainSet->getImageFile(i)->getFileName()));
   }
   
   QGroupBox* groupBox = new QGroupBox("Image Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(imageLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(imageSelectionComboBox, 0, 1);
   gridLayout->setColumnStretch(0, 0);
   gridLayout->setColumnStretch(1, 100);

   return groupBox;
}
      
/**
 * create the topology options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createTopologyOptionsSection()
{
   //
   // Topology selection
   //
   QLabel* topologySelectionLabel = new QLabel("Topology File");
   topologySelectionComboBox = new GuiTopologyFileComboBox;
   BrainModelSurface* bms = theMainWindow->getBrainModelSurface();
   if (bms != NULL) {
      topologySelectionComboBox->setSelectedTopologyFile(bms->getTopologyFile());
   }
   QObject::connect(topologySelectionComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotLoadParametersForFileType()));
   
   //
   // Topology Type
   //
   QLabel* topologyTypeLabel = new QLabel("Save As Type");
   topologyTypeComboBox = new GuiTopologyTypeComboBox(false);
   
   //
   // Group box and layout
   //
   QGroupBox* groupBox = new QGroupBox("Topology Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(topologySelectionLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(topologySelectionComboBox, 0, 1);
   gridLayout->addWidget(topologyTypeLabel, 1, 0, Qt::AlignLeft);
   gridLayout->addWidget(topologyTypeComboBox, 1, 1);
   gridLayout->setColumnStretch(0, 0);
   gridLayout->setColumnStretch(1, 100);

   return groupBox;
}

/**
 * Create the vector options section.
 */
QGroupBox*
GuiDataFileSaveDialog::createVectorOptionsSection()
{
   //
   // Vector File Selection
   //
   QLabel* vectorSelectionLabel = new QLabel("Vector File");
   vectorFileSelectionComboBox = new GuiVectorFileComboBox;
   QObject::connect(vectorFileSelectionComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotLoadParametersForFileType()));

   QGroupBox* groupBox = new QGroupBox("Vector Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(vectorSelectionLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(vectorFileSelectionComboBox, 0, 1, Qt::AlignLeft);
   gridLayout->setColumnStretch(0, 0);
   gridLayout->setColumnStretch(1, 100);

   return groupBox;
}

/**
 * create the volume anatomy options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createVolumeAnatomyOptionsSection()
{
   //
   // File selection label and combo box
   //
   QLabel* fileSelectionLabel = new QLabel("Volume File");
   volumeAnatomyFileSelectionComboBox = 
      new GuiVolumeFileSelectionComboBox(VolumeFile::VOLUME_TYPE_ANATOMY, true);
   QObject::connect(volumeAnatomyFileSelectionComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotLoadParametersForFileType()));
                       
   //
   // volume label
   //
   QLabel* volumeLabelLabel = new QLabel("Volume Label");
   volumeAnatomyLabelLineEdit = new QLineEdit;
   
   //
   // Voxel data type
   //
   QLabel* voxelDataTypeLabel = new QLabel("Voxel Data Type");
   volumeAnatomyVolumeDataTypeComboBox = new GuiVolumeVoxelDataTypeComboBox;
   
   //
   // Write compressed
   //
   volumeAnatomyWriteCompressedCheckBox = new QCheckBox("Write Volume Data GZipped (compressed)");

   //
   // Group box and layout
   //
   QGroupBox* groupBox = new QGroupBox("Anatomy Volume Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(fileSelectionLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeAnatomyFileSelectionComboBox, 0, 1);
   gridLayout->addWidget(volumeLabelLabel, 1, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeAnatomyLabelLineEdit, 1, 1);
   gridLayout->addWidget(voxelDataTypeLabel, 2, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeAnatomyVolumeDataTypeComboBox, 2, 1);
   gridLayout->addWidget(volumeAnatomyWriteCompressedCheckBox, 3, 0, 1, 2, Qt::AlignLeft);
   return groupBox;
}

/**
 * create the volume functional options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createVolumeFunctionalOptionsSection()
{
   //
   // File selection label and combo box
   //
   QLabel* fileSelectionLabel = new QLabel("Volume File");
   volumeFunctionalFileSelectionComboBox = 
      new GuiVolumeFileSelectionComboBox(VolumeFile::VOLUME_TYPE_FUNCTIONAL, true);
   QObject::connect(volumeFunctionalFileSelectionComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotLoadParametersForFileType()));
                       
   //
   // volume label
   //
   QLabel* volumeLabelLabel = new QLabel("Volume Label");
   volumeFunctionalLabelLineEdit = new QLineEdit;
   
   //
   // Voxel data type
   //
   QLabel* voxelDataTypeLabel = new QLabel("Voxel Data Type");
   volumeFunctionalVolumeDataTypeComboBox = new GuiVolumeVoxelDataTypeComboBox;
   
   //
   // Write compressed
   //
   volumeFunctionalWriteCompressedCheckBox = new QCheckBox("Write Volume Data GZipped (compressed)");

   //
   // Group box and layout
   //
   QGroupBox* groupBox = new QGroupBox("Functional Volume Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(fileSelectionLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeFunctionalFileSelectionComboBox, 0, 1);
   gridLayout->addWidget(volumeLabelLabel, 1, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeFunctionalLabelLineEdit, 1, 1);
   gridLayout->addWidget(voxelDataTypeLabel, 2, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeFunctionalVolumeDataTypeComboBox, 2, 1);
   gridLayout->addWidget(volumeFunctionalWriteCompressedCheckBox, 3, 0, 1, 2, Qt::AlignLeft);
   return groupBox;
}
 
/**
 * create the volume paint options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createVolumePaintOptionsSection()
{
   //
   // File selection label and combo box
   //
   QLabel* fileSelectionLabel = new QLabel("Volume File");
   volumePaintFileSelectionComboBox = 
      new GuiVolumeFileSelectionComboBox(VolumeFile::VOLUME_TYPE_PAINT, true);
   QObject::connect(volumePaintFileSelectionComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotLoadParametersForFileType()));
                       
   //
   // volume label
   //
   QLabel* volumeLabelLabel = new QLabel("Volume Label");
   volumePaintLabelLineEdit = new QLineEdit;
   
   //
   // Voxel data type
   //
   QLabel* voxelDataTypeLabel = new QLabel("Voxel Data Type");
   volumePaintVolumeDataTypeComboBox = new GuiVolumeVoxelDataTypeComboBox;
   
   //
   // Write compressed
   //
   volumePaintWriteCompressedCheckBox = new QCheckBox("Write Volume Data GZipped (compressed)");

   //
   // Group box and layout
   //
   QGroupBox* groupBox = new QGroupBox("Paint Volume Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(fileSelectionLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumePaintFileSelectionComboBox, 0, 1);
   gridLayout->addWidget(volumeLabelLabel, 1, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumePaintLabelLineEdit, 1, 1);
   gridLayout->addWidget(voxelDataTypeLabel, 2, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumePaintVolumeDataTypeComboBox, 2, 1);
   gridLayout->addWidget(volumePaintWriteCompressedCheckBox, 3, 0, 1, 2, Qt::AlignLeft);
   return groupBox;
}
      
/**
 * create the prob atlas volume options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createVolumeProbAtlasOptionsSection()
{
   //
   // File selection label and combo box
   //
   QLabel* fileSelectionLabel = new QLabel("Volume File");
   volumeProbAtlasFileSelectionComboBox = 
      new GuiVolumeFileSelectionComboBox(VolumeFile::VOLUME_TYPE_PROB_ATLAS, true);
   QObject::connect(volumeProbAtlasFileSelectionComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotLoadParametersForFileType()));
   
   if (alwaysSaveAllProbAtlasVolumes) {
      fileSelectionLabel->hide();
      volumeProbAtlasFileSelectionComboBox->hide();
   }
   
   //
   // volume label
   //
   QLabel* volumeLabelLabel = new QLabel("Volume Label");
   volumeProbAtlasLabelLineEdit = new QLineEdit;
   
   //
   // Voxel data type
   //
   QLabel* voxelDataTypeLabel = new QLabel("Voxel Data Type");
   volumeProbAtlasVolumeDataTypeComboBox = new GuiVolumeVoxelDataTypeComboBox;
   
   //
   // Write compressed
   //
   volumeProbAtlasWriteCompressedCheckBox = new QCheckBox("Write Volume Data GZipped (compressed)");

   //
   // Group box and layout
   //
   QGroupBox* groupBox = new QGroupBox("ProbAtlas Volume Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(fileSelectionLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeProbAtlasFileSelectionComboBox, 0, 1);
   gridLayout->addWidget(volumeLabelLabel, 1, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeProbAtlasLabelLineEdit, 1, 1);
   gridLayout->addWidget(voxelDataTypeLabel, 2, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeProbAtlasVolumeDataTypeComboBox, 2, 1);
   gridLayout->addWidget(volumeProbAtlasWriteCompressedCheckBox, 3, 0, 1, 2, Qt::AlignLeft);
   return groupBox;
}
      
/**
 * create the volume rgb options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createVolumeRgbOptionsSection()
{
   //
   // File selection label and combo box
   //
   QLabel* fileSelectionLabel = new QLabel("Volume File");
   volumeRgbFileSelectionComboBox = 
      new GuiVolumeFileSelectionComboBox(VolumeFile::VOLUME_TYPE_RGB, true);
   QObject::connect(volumeRgbFileSelectionComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotLoadParametersForFileType()));
                       
   //
   // volume label
   //
   QLabel* volumeLabelLabel = new QLabel("Volume Label");
   volumeRgbLabelLineEdit = new QLineEdit;
   
   //
   // Voxel data type
   //
   QLabel* voxelDataTypeLabel = new QLabel("Voxel Data Type");
   volumeRgbVolumeDataTypeComboBox = new GuiVolumeVoxelDataTypeComboBox;
   
   //
   // Write compressed
   //
   volumeRgbWriteCompressedCheckBox = new QCheckBox("Write Volume Data GZipped (compressed)");

   //
   // Group box and layout
   //
   QGroupBox* groupBox = new QGroupBox("Rgb Volume Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(fileSelectionLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeRgbFileSelectionComboBox, 0, 1);
   gridLayout->addWidget(volumeLabelLabel, 1, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeRgbLabelLineEdit, 1, 1);
   gridLayout->addWidget(voxelDataTypeLabel, 2, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeRgbVolumeDataTypeComboBox, 2, 1);
   gridLayout->addWidget(volumeRgbWriteCompressedCheckBox, 3, 0, 1, 2, Qt::AlignLeft);
   return groupBox;
}
      
/**
 * create the volume segmentation options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createVolumeSegmentationOptionsSection()
{
   //
   // File selection label and combo box
   //
   QLabel* fileSelectionLabel = new QLabel("Volume File");
   volumeSegmentationFileSelectionComboBox = 
      new GuiVolumeFileSelectionComboBox(VolumeFile::VOLUME_TYPE_SEGMENTATION, true);
   QObject::connect(volumeSegmentationFileSelectionComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotLoadParametersForFileType()));
                       
   //
   // volume label
   //
   QLabel* volumeLabelLabel = new QLabel("Volume Label");
   volumeSegmentationLabelLineEdit = new QLineEdit;
   
   //
   // Voxel data type
   //
   QLabel* voxelDataTypeLabel = new QLabel("Voxel Data Type");
   volumeSegmentationVolumeDataTypeComboBox = new GuiVolumeVoxelDataTypeComboBox;
   
   //
   // Write compressed
   //
   volumeSegmentationWriteCompressedCheckBox = new QCheckBox("Write Volume Data GZipped (compressed)");

   //
   // Group box and layout
   //
   QGroupBox* groupBox = new QGroupBox("Segmentation Volume Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(fileSelectionLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeSegmentationFileSelectionComboBox, 0, 1);
   gridLayout->addWidget(volumeLabelLabel, 1, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeSegmentationLabelLineEdit, 1, 1);
   gridLayout->addWidget(voxelDataTypeLabel, 2, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeSegmentationVolumeDataTypeComboBox, 2, 1);
   gridLayout->addWidget(volumeSegmentationWriteCompressedCheckBox, 3, 0, 1, 2, Qt::AlignLeft);
   return groupBox;
}

/**
 * create the volume options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createVolumeVectorOptionsSection()
{
   //
   // File selection label and combo box
   //
   QLabel* fileSelectionLabel = new QLabel("Volume File");
   volumeVectorFileSelectionComboBox = 
      new GuiVolumeFileSelectionComboBox(VolumeFile::VOLUME_TYPE_VECTOR, true);
   QObject::connect(volumeVectorFileSelectionComboBox, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotLoadParametersForFileType()));
                       
   //
   // volume label
   //
   QLabel* volumeLabelLabel = new QLabel("Volume Label");
   volumeVectorLabelLineEdit = new QLineEdit;
   
   //
   // Voxel data type
   //
   QLabel* voxelDataTypeLabel = new QLabel("Voxel Data Type");
   volumeVectorVolumeDataTypeComboBox = new GuiVolumeVoxelDataTypeComboBox;
   
   //
   // Write compressed
   //
   volumeVectorWriteCompressedCheckBox = new QCheckBox("Write Volume Data GZipped (compressed)");

   //
   // Group box and layout
   //
   QGroupBox* groupBox = new QGroupBox("Vector Volume Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(fileSelectionLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeVectorFileSelectionComboBox, 0, 1);
   gridLayout->addWidget(volumeLabelLabel, 1, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeVectorLabelLineEdit, 1, 1);
   gridLayout->addWidget(voxelDataTypeLabel, 2, 0, Qt::AlignLeft);
   gridLayout->addWidget(volumeVectorVolumeDataTypeComboBox, 2, 1);
   gridLayout->addWidget(volumeVectorWriteCompressedCheckBox, 3, 0, 1, 2, Qt::AlignLeft);
   return groupBox;
}
      
/**
 * create the vtk model options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createVtkModelOptionsSection()
{
   BrainSet* brainSet = theMainWindow->getBrainSet();
   
   //
   // VTK model selection and label
   //
   QLabel* vtkModelLabel = new QLabel("VTK Model");
   vtkModelSelectionComboBox = new QComboBox;
   for (int i = 0; i < brainSet->getNumberOfVtkModelFiles(); i++) {
      vtkModelSelectionComboBox->addItem(
         FileUtilities::basename(brainSet->getVtkModelFile(i)->getFileName()));
   }
   
   QGroupBox* groupBox = new QGroupBox("VTK Model Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(vtkModelLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(vtkModelSelectionComboBox, 0, 1);
   gridLayout->setColumnStretch(0, 0);
   gridLayout->setColumnStretch(1, 100);
   return groupBox;
}
      
/**
 * create the export volume section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createExportVolumeOptionsSection()
{
   //
   // Volume selection
   //
   exportVolumeSelectionControl = 
      new GuiVolumeSelectionControl(0,
                                    true,
                                    true,
                                    true,
                                    true,
                                    true,
                                    true,
                                    true,
                                    GuiVolumeSelectionControl::LABEL_MODE_FILE_LABEL_AND_NAME,
                                    0,
                                    false,
                                    false,
                                    false);
                                    
   QGroupBox* groupBox = new QGroupBox("Export Volume Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(exportVolumeSelectionControl, 0, 0, Qt::AlignLeft);
   return groupBox;
}

/**
 * create the export surface section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createExportSurfaceOptionsSection()
{
   //
   // Surface Selection
   //
   QLabel* surfaceLabel = new QLabel("Surface");
   exportSurfaceSelectionControl = 
      new GuiBrainModelSelectionComboBox(false, true, false, "");
   exportSurfaceSelectionControl->setSelectedBrainModel(theMainWindow->getBrainModelSurface());
      
   QGroupBox* groupBox = new QGroupBox("Export Surface Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(surfaceLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(exportSurfaceSelectionControl, 0, 1);
   gridLayout->setColumnStretch(0, 0);
   gridLayout->setColumnStretch(1, 100);
   return groupBox;
}
      
/** 
 * create paint export options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createPaintExportOptionsSection()
{
   //
   // Surface Selection
   //
   QLabel* surfaceLabel = new QLabel("Surface");
   exportPaintSurfaceSelectionControl = 
      new GuiBrainModelSelectionComboBox(false, true, false, "");
   exportPaintSurfaceSelectionControl->setSelectedBrainModel(theMainWindow->getBrainModelSurface());
     
   //
   // Paint column
   //
   QLabel* paintColumLabel = new QLabel("Paint Column");
   exportPaintColumnSelectionControl = 
      new GuiNodeAttributeColumnSelectionComboBox(GUI_NODE_FILE_TYPE_PAINT,
                                                 false,
                                                 false,
                                                 false);
                                                 
   QGroupBox* groupBox = new QGroupBox("Export Paint Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(surfaceLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(exportPaintSurfaceSelectionControl, 0, 1);
   gridLayout->addWidget(paintColumLabel, 1, 0, Qt::AlignLeft);
   gridLayout->addWidget(exportPaintColumnSelectionControl, 1, 1);
   gridLayout->setColumnStretch(0, 0);
   gridLayout->setColumnStretch(1, 100);
   return groupBox;
}

/**
 * create shape export options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createShapeExportOptionsSection()
{
   //
   // Surface Selection
   //
   QLabel* surfaceLabel = new QLabel("Surface");
   exportShapeSurfaceSelectionControl = 
      new GuiBrainModelSelectionComboBox(false, true, false, "");
   exportShapeSurfaceSelectionControl->setSelectedBrainModel(theMainWindow->getBrainModelSurface());
     
   //
   // Shape column
   //
   QLabel* shapeColumLabel = new QLabel("Shape Column");
   exportShapeColumnSelectionControl = 
      new GuiNodeAttributeColumnSelectionComboBox(GUI_NODE_FILE_TYPE_SURFACE_SHAPE,
                                                 false,
                                                 false,
                                                 false);
                                                 
   QGroupBox* groupBox = new QGroupBox("Export Shape Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(surfaceLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(exportShapeSurfaceSelectionControl, 0, 1);
   gridLayout->addWidget(shapeColumLabel, 1, 0, Qt::AlignLeft);
   gridLayout->addWidget(exportShapeColumnSelectionControl, 1, 1);
   gridLayout->setColumnStretch(0, 0);
   gridLayout->setColumnStretch(1, 100);
   return groupBox;
}

/**
 * create metric export options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createMetricExportOptionsSection()
{
   //
   // Surface Selection
   //
   QLabel* surfaceLabel = new QLabel("Surface");
   exportMetricSurfaceSelectionControl = 
      new GuiBrainModelSelectionComboBox(false, true, false, "");
   exportMetricSurfaceSelectionControl->setSelectedBrainModel(theMainWindow->getBrainModelSurface());
     
   //
   // Metric column
   //
   QLabel* metricColumLabel = new QLabel("Metric Column");
   exportMetricColumnSelectionControl = 
      new GuiNodeAttributeColumnSelectionComboBox(GUI_NODE_FILE_TYPE_METRIC,
                                                 false,
                                                 false,
                                                 false);
                                                 
   QGroupBox* groupBox = new QGroupBox("Export Metric Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(surfaceLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(exportMetricSurfaceSelectionControl, 0, 1);
   gridLayout->addWidget(metricColumLabel, 1, 0, Qt::AlignLeft);
   gridLayout->addWidget(exportMetricColumnSelectionControl, 1, 1);
   gridLayout->setColumnStretch(0, 0);
   gridLayout->setColumnStretch(1, 100);
   return groupBox;
}
      
/**
 * create gifti options section.
 */
QGroupBox* 
GuiDataFileSaveDialog::createGiftiOptionsSection()
{
   //
   // Surface Selection
   //
   QLabel* surfaceLabel = new QLabel("Surface");
   giftiSurfaceSelectionControl = 
      new GuiBrainModelSelectionComboBox(false, true, false, "");
   QObject::connect(giftiSurfaceSelectionControl, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(slotLoadParametersForFileType()));
   giftiSurfaceSelectionControl->setSelectedBrainModel(theMainWindow->getBrainModelSurface());

   QGroupBox* groupBox = new QGroupBox("GIFTI Surface Options");
   QGridLayout* gridLayout = new QGridLayout(groupBox);
   gridLayout->addWidget(surfaceLabel, 0, 0, Qt::AlignLeft);
   gridLayout->addWidget(giftiSurfaceSelectionControl, 0, 1);
   gridLayout->setColumnStretch(0, 0);
   gridLayout->setColumnStretch(1, 100);
   return groupBox;
}
      
/**
 * create the metadata section.
 */
QWidget* 
GuiDataFileSaveDialog::createMetadataSection()
{
   //
   // PubMed ID
   //
   QLabel* pubMedIdLabel = new QLabel("PubMed ID");
   metadataPubMedIDLineEdit = new QLineEdit;
   
   //
   // Comment
   //
   QLabel* commentLabel = new QLabel("Comment");
   metadataCommentTextEdit = new QTextEdit;
   
   //
   // Group box and layout for metadata
   //
   QGroupBox* metadataGroupBox = new QGroupBox("Metadata");
   QGridLayout* metadataGridLayout = new QGridLayout(metadataGroupBox);
   metadataGridLayout->addWidget(pubMedIdLabel, 1, 0);
   metadataGridLayout->addWidget(metadataPubMedIDLineEdit, 1, 1);
   metadataGridLayout->addWidget(commentLabel, 3, 0);
   metadataGridLayout->addWidget(metadataCommentTextEdit, 3, 1);
   return metadataGroupBox;
}

/**
 * create the file information section.
 */
QWidget* 
GuiDataFileSaveDialog::createFileInformationSection()
{
   //
   // file name push button and line edit
   //
   QPushButton* fileNamePushButton = new QPushButton("File Name...");
   fileNamePushButton->setAutoDefault(false);
   fileNamePushButton->setFixedSize(fileNamePushButton->sizeHint());
   QObject::connect(fileNamePushButton, SIGNAL(clicked()),
                    this, SLOT(slotFileNamePushButton()));
   fileNameLineEdit = new QLineEdit;
   QObject::connect(fileNameLineEdit, SIGNAL(textChanged(const QString&)),
                    this, SLOT(slotFileNameLineEditChanged(const QString&)));
                    
   //
   // file type label and combo box
   //
   QLabel* fileTypeLabel = new QLabel("File Type ");
   fileTypeComboBox = new QComboBox;
   QObject::connect(fileTypeComboBox, SIGNAL(currentIndexChanged(const QString&)),
                    this, SLOT(slotFileTypeComboBox(const QString&)));
       
   
   //
   // Encoding
   //
   QLabel* fileEncodingLabel = new QLabel("Encoding");
   fileEncodingComboBox = new QComboBox;
   
   //
   // add file extension check box
   //
   addFileExtensionCheckBox = new QCheckBox("Add File Name Extension (if needed)");
   addFileExtensionCheckBox->setChecked(true);
   
   //
   // Layout the options
   //
   QGroupBox* fileInfoGroupBox = new QGroupBox("File Information");
   QGridLayout* fileInfoGridLayout = new QGridLayout(fileInfoGroupBox);
   fileInfoGridLayout->addWidget(fileNamePushButton, 0, 0, Qt::AlignHCenter);
   fileInfoGridLayout->addWidget(fileNameLineEdit, 0, 1);
   fileInfoGridLayout->addWidget(fileTypeLabel, 1, 0, Qt::AlignHCenter);
   fileInfoGridLayout->addWidget(fileTypeComboBox, 1, 1);
   fileInfoGridLayout->addWidget(fileEncodingLabel, 2, 0, Qt::AlignHCenter);
   fileInfoGridLayout->addWidget(fileEncodingComboBox, 2, 1);
   fileInfoGridLayout->addWidget(addFileExtensionCheckBox, 3, 0, 1, 2, Qt::AlignLeft);
   
   return fileInfoGroupBox;
}
      
/**
 * called when a file type combo box selection is made.
 */
void 
GuiDataFileSaveDialog::slotFileTypeComboBox(const QString&)
{
   slotLoadParametersForFileType();
}

/**
 * get supported encodings from a file.
 */
void 
GuiDataFileSaveDialog::getSupportedEncodings(const AbstractFile& af,
                                             std::vector<AbstractFile::FILE_FORMAT>& encodingsOut,
                                             AbstractFile::FILE_FORMAT& preferredEncodingOut)
{
   encodingsOut.clear();
   if (af.getCanWrite(AbstractFile::FILE_FORMAT_ASCII)) {
      encodingsOut.push_back(AbstractFile::FILE_FORMAT_ASCII);
   }
   
   if (af.getCanWrite(AbstractFile::FILE_FORMAT_BINARY)) {
      encodingsOut.push_back(AbstractFile::FILE_FORMAT_BINARY);
   }
   
   if (af.getCanWrite(AbstractFile::FILE_FORMAT_COMMA_SEPARATED_VALUE_FILE)) {
      encodingsOut.push_back(AbstractFile::FILE_FORMAT_COMMA_SEPARATED_VALUE_FILE);
   }
   
   if (af.getCanWrite(AbstractFile::FILE_FORMAT_XML)) {
      encodingsOut.push_back(AbstractFile::FILE_FORMAT_XML);
   }
   
   if (af.getCanWrite(AbstractFile::FILE_FORMAT_XML_BASE64)) {
      encodingsOut.push_back(AbstractFile::FILE_FORMAT_XML_BASE64);
   }
   
   if (af.getCanWrite(AbstractFile::FILE_FORMAT_XML_GZIP_BASE64)) {
      encodingsOut.push_back(AbstractFile::FILE_FORMAT_XML_GZIP_BASE64);
   }
   
   if (af.getCanWrite(AbstractFile::FILE_FORMAT_OTHER)) {
      encodingsOut.push_back(AbstractFile::FILE_FORMAT_OTHER);
   }
   
   preferredEncodingOut = AbstractFile::FILE_FORMAT_ASCII;
   std::vector<AbstractFile::FILE_FORMAT> prefFormats = 
                             AbstractFile::getPreferredWriteType();
   for (unsigned int i = 0; i < prefFormats.size(); i++) {
      if (af.getCanWrite(prefFormats[i])) {
         preferredEncodingOut = prefFormats[i];
         break;
      }
   }
}
      
/**
 * get supported Xml encodings from a file.
 */
void 
GuiDataFileSaveDialog::getSupportedXmlEncodings(const AbstractFile& af,
                                             std::vector<AbstractFile::FILE_FORMAT>& encodingsOut,
                                             AbstractFile::FILE_FORMAT& preferredEncodingOut)
{
   encodingsOut.clear();
   encodingsOut.push_back(AbstractFile::FILE_FORMAT_XML);
   encodingsOut.push_back(AbstractFile::FILE_FORMAT_XML_BASE64);
   encodingsOut.push_back(AbstractFile::FILE_FORMAT_XML_GZIP_BASE64);

   preferredEncodingOut = AbstractFile::FILE_FORMAT_XML;
   std::vector<AbstractFile::FILE_FORMAT> prefFormats = 
                             AbstractFile::getPreferredWriteType();
   for (unsigned int i = 0; i < prefFormats.size(); i++) {
      if (std::find(encodingsOut.begin(),
                    encodingsOut.end(),
                    prefFormats[i]) != encodingsOut.end()) {
         if (af.getCanWrite(prefFormats[i])) {
            preferredEncodingOut = prefFormats[i];
            break;
         }
      }
   }
}
      
/**
 * load parameters for the selected file type.
 */
void 
GuiDataFileSaveDialog::slotLoadParametersForFileType()      
{
   if (fileOptionsWidgetGroup == NULL) {
      return;
   }
   
   //
   // Hide all file options groups
   //
   fileOptionsWidgetGroup->setHidden(true);
   
   BrainSet* brainSet = theMainWindow->getBrainSet();
   AbstractFile* af = NULL;
   const QString filterName = fileTypeComboBox->currentText();
   
   QString fileName;
   QString fileComment;
   QString filePubMedID;
   
   std::vector<AbstractFile::FILE_FORMAT> fileAvailableEncodings;
   AbstractFile::FILE_FORMAT preferredEncoding = AbstractFile::FILE_FORMAT_OTHER;   
   
   if (filterName == FileFilters::getAreaColorFileFilter()) {
      af = brainSet->getAreaColorFile();
   }
   else if (filterName == FileFilters::getArealEstimationFileFilter()) {
      af = brainSet->getArealEstimationFile();
   }
   else if (filterName == FileFilters::getBorderGenericFileFilter()) {
      borderSurfaceOptionsGroupBox->show();
      const BrainModelSurface* bms = borderSurfaceSelectionComboBox->getSelectedBrainModelSurface();
      if (bms != NULL) {
         BrainModelBorderSet* bmbs = brainSet->getBorderSet();
         BrainModelBorderFileInfo* bmi = bmbs->getBorderFileInfo(bms->getSurfaceType());
         fileComment = bmi->getFileComment();
         filePubMedID = bmi->getPubMedID();
         fileName = bmi->getFileName();
         borderSurfaceTypeComboBox->setSurfaceType(bms->getSurfaceType());
         
         BorderFile bf;
         getSupportedEncodings(bf, fileAvailableEncodings, preferredEncoding);
      }
   }
   else if (filterName == FileFilters::getBorderVolumeFileFilter()) {
      af = brainSet->getVolumeBorderFile();
   }
   else if (filterName == FileFilters::getBorderColorFileFilter()) {
      af = brainSet->getBorderColorFile();
   }
   else if (filterName == FileFilters::getBorderProjectionFileFilter()) {
      borderProjectionOptionsGroupBox->show();
      BrainModelBorderSet* bmbs = brainSet->getBorderSet();
      BrainModelBorderFileInfo* bmi = bmbs->getBorderProjectionFileInfo();
      fileComment = bmi->getFileComment();
      filePubMedID = bmi->getPubMedID();
      fileName = bmi->getFileName();
      
      BorderProjectionFile bpf;
      getSupportedEncodings(bpf, fileAvailableEncodings, preferredEncoding);
   }
   else if (filterName == FileFilters::getCellFileFilter()) {
      cellOptionsGroupBox->show();
      CellProjectionFile* cpf = brainSet->getCellProjectionFile();
      fileComment = cpf->getFileComment();
      filePubMedID = cpf->getFilePubMedID();
      
      CellFile cf;
      getSupportedEncodings(cf, fileAvailableEncodings, preferredEncoding);
   }
   else if (filterName == FileFilters::getCellColorFileFilter()) {
      af = brainSet->getCellColorFile();
   }
   else if (filterName == FileFilters::getCellProjectionFileFilter()) {
      af = brainSet->getCellProjectionFile();
   }
   else if (filterName == FileFilters::getCellVolumeFileFilter()) {
      af = brainSet->getVolumeCellFile();
   }
   else if (filterName == FileFilters::getCocomacFileFilter()) {
      af = brainSet->getCocomacFile();
   }
   else if (filterName == FileFilters::getContourFileFilter()) {
      BrainModelContours* bmc = brainSet->getBrainModelContours();
      if (bmc != NULL) {
         af = bmc->getContourFile();
      }
   }
   else if (filterName == FileFilters::getContourCellFileFilter()) {
      af = brainSet->getContourCellFile();
   }
   else if (filterName == FileFilters::getContourCellColorFileFilter()) {
      af = brainSet->getContourCellColorFile();
   }
   else if (filterName == FileFilters::getCoordinateGenericFileFilter()) {
      coordinateOptionsGroupBox->show();
      BrainModelSurface* bms = coordinateSurfaceSelectionComboBox->getSelectedBrainModelSurface();
      if (bms != NULL) {
         coordinateTypeComboBox->setSurfaceType(bms->getSurfaceType());
         coordinateStructureComboBox->setStructure(bms->getStructure().getType());
         
         CoordinateFile* cf = bms->getCoordinateFile();
         const QString spaceName = cf->getHeaderTag(AbstractFile::headerTagCoordFrameID);
         for (int i = 0; i < coordinateStereotaxicSpaceComboBox->count(); i++) {
            if (spaceName == coordinateStereotaxicSpaceComboBox->itemText(i)) {
               coordinateStereotaxicSpaceComboBox->setCurrentIndex(i);
               break;
            }
         }
         af = cf;
      }
   }
   else if (filterName == FileFilters::getCutsFileFilter()) {
      af = brainSet->getCutsFile();
   }
   else if (filterName == FileFilters::getDeformationFieldFileFilter()) {
      af = brainSet->getDeformationFieldFile();
   }
   else if (filterName == FileFilters::getFociFileFilter()) {
      fociOptionsGroupBox->show();
      const FociProjectionFile* fpf = brainSet->getFociProjectionFile();
      fileComment = fpf->getFileComment();
      filePubMedID = fpf->getFilePubMedID();
      
      FociFile ff;
      getSupportedEncodings(ff, fileAvailableEncodings, preferredEncoding);
   }
   else if (filterName == FileFilters::getFociColorFileFilter()) {
      af = brainSet->getFociColorFile();
   }
   else if (filterName == FileFilters::getFociProjectionFileFilter()) {
      af = brainSet->getFociProjectionFile();
   }
   else if (filterName == FileFilters::getFociSearchFileFilter()) {
      af = brainSet->getFociSearchFile();
   }
   else if (filterName == FileFilters::getGeodesicDistanceFileFilter()) {
      af = brainSet->getGeodesicDistanceFile();
   }
   else if (filterName == FileFilters::getGiftiCoordinateFileFilter()) {
      coordinateOptionsGroupBox->show();
      BrainModelSurface* bms = coordinateSurfaceSelectionComboBox->getSelectedBrainModelSurface();
      if (bms != NULL) {
         coordinateTypeComboBox->setSurfaceType(bms->getSurfaceType());
         coordinateStructureComboBox->setStructure(bms->getStructure().getType());
         
         CoordinateFile* cf = bms->getCoordinateFile();
         const QString spaceName = cf->getHeaderTag(AbstractFile::headerTagCoordFrameID);
         for (int i = 0; i < coordinateStereotaxicSpaceComboBox->count(); i++) {
            if (spaceName == coordinateStereotaxicSpaceComboBox->itemText(i)) {
               coordinateStereotaxicSpaceComboBox->setCurrentIndex(i);
               break;
            }
         }
         af = cf;
      }
   }
   else if (filterName == FileFilters::getGiftiFunctionalFileFilter()) {
      MetricFile* mf = brainSet->getMetricFile();
      fileComment = mf->getFileComment();
      filePubMedID = mf->getFilePubMedID();
      fileName = mf->getFileName();
      getSupportedXmlEncodings(*mf, fileAvailableEncodings, preferredEncoding);
   }
   else if (filterName == FileFilters::getGiftiLabelFileFilter()) {
      PaintFile* pf = brainSet->getPaintFile();
      fileComment = pf->getFileComment();
      filePubMedID = pf->getFilePubMedID();
      fileName = pf->getFileName();
      getSupportedXmlEncodings(*pf, fileAvailableEncodings, preferredEncoding);
   }
   else if (filterName == FileFilters::getGiftiShapeFileFilter()) {
      SurfaceShapeFile* ssf = brainSet->getSurfaceShapeFile();
      fileComment = ssf->getFileComment();
      filePubMedID = ssf->getFilePubMedID();
      fileName = ssf->getFileName();
      getSupportedXmlEncodings(*ssf, fileAvailableEncodings, preferredEncoding);
   }
   else if (filterName == FileFilters::getGiftiSurfaceFileFilter()) {
      giftiSurfaceOptionsGroupBox->show();
      BrainModelSurface* bms = giftiSurfaceSelectionControl->getSelectedBrainModelSurface();
      if (bms != NULL) {
         CoordinateFile* cf = bms->getCoordinateFile();      
         fileComment = cf->getFileComment();
         filePubMedID = cf->getFilePubMedID();
         fileName = cf->getFileName();
         getSupportedXmlEncodings(*cf, fileAvailableEncodings, preferredEncoding);
      }
   }
   else if (filterName == FileFilters::getGiftiTopologyFileFilter()) {
      topologyOptionsGroupBox->show();
      TopologyFile* tf = topologySelectionComboBox->getSelectedTopologyFile();
      if (tf != NULL) {
         topologyTypeComboBox->setTopologyType(tf->getTopologyType());
         af = tf;
      }
   }
   else if (filterName == FileFilters::getImageSaveFileFilter()) {
      imageOptionsGroupBox->show();
      const int indx = imageSelectionComboBox->currentIndex();
      if ((indx >= 0) && (indx < brainSet->getNumberOfImageFiles())) {
         af = brainSet->getImageFile(indx);
      }
   }
   else if (filterName == FileFilters::getLatitudeLongitudeFileFilter()) {
      af = brainSet->getLatLonFile();
   }
   else if (filterName == FileFilters::getMetricFileFilter()) {
      af = brainSet->getMetricFile();
   }
   else if (filterName == FileFilters::getPaintFileFilter()) {
      af = brainSet->getPaintFile();
   }
   else if (filterName == FileFilters::getPaletteFileFilter()) {
      af = brainSet->getPaletteFile();
   }
   else if (filterName == FileFilters::getParamsFileFilter()) {
      af = brainSet->getParamsFile();
   }
   else if (filterName == FileFilters::getProbAtlasFileFilter()) {
      af = brainSet->getProbabilisticAtlasSurfaceFile();
   }
   else if (filterName == FileFilters::getRgbPaintFileFilter()) {
      af = brainSet->getRgbPaintFile();
   }
   else if (filterName == FileFilters::getSceneFileFilter()) {
      af = brainSet->getSceneFile();
   }
   else if (filterName == FileFilters::getSectionFileFilter()) {
      af = brainSet->getSectionFile();
   }
   else if (filterName == FileFilters::getStudyCollectionFileFilter()) {
      af = brainSet->getStudyCollectionFile();
   }
   else if (filterName == FileFilters::getStudyMetaDataFileFilter()) {
      af = brainSet->getStudyMetaDataFile();
   }
   else if (filterName == FileFilters::getSurfaceShapeFileFilter()) {
      af = brainSet->getSurfaceShapeFile();
   }
   else if (filterName == FileFilters::getGiftiVectorFileFilter()) {
      vectorOptionsGroupBox->show();
      af = vectorFileSelectionComboBox->getSelectedVectorFile();
   }
   else if (filterName == FileFilters::getTopographyFileFilter()) {
      af = brainSet->getTopographyFile();
   }
   else if (filterName == FileFilters::getTopologyGenericFileFilter()) {
      topologyOptionsGroupBox->show();
      TopologyFile* tf = topologySelectionComboBox->getSelectedTopologyFile();
      if (tf != NULL) {
         topologyTypeComboBox->setTopologyType(tf->getTopologyType());
         af = tf;
      }
   }
   else if (filterName == FileFilters::getTransformationMatrixFileFilter()) {
      af = brainSet->getTransformationMatrixFile();
   }
   else if (filterName == FileFilters::getVocabularyFileFilter()) {
      af = brainSet->getVocabularyFile();
   }   
   else if (filterName == FileFilters::getVolumeAnatomyFileFilter()) {
      volumeAnatomyOptionsGroupBox->show();
      VolumeFile* vf = 0;
      if (volumeAnatomyFileSelectionComboBox->getAllVolumesSelected()) {
         if (brainSet->getNumberOfVolumeAnatomyFiles() > 0) {
            VolumeFile* temp = brainSet->getVolumeAnatomyFile(0);
            af = temp;
            fileName = temp->getFileName();
            volumeAnatomyVolumeDataTypeComboBox->setVolumeVoxelDataType(temp->getVoxelDataType());
            volumeAnatomyWriteCompressedCheckBox->setChecked(temp->getDataFileWasZipped());
            
            VolumeFile vf;
            getSupportedEncodings(vf, fileAvailableEncodings, preferredEncoding);            
         }
      }
      else {
         vf = volumeAnatomyFileSelectionComboBox->getSelectedVolumeFile();
      }
      if (vf != NULL) {
         volumeAnatomyLabelLineEdit->setText(vf->getDescriptiveLabel());
         volumeAnatomyVolumeDataTypeComboBox->setVolumeVoxelDataType(vf->getVoxelDataType());
         volumeAnatomyWriteCompressedCheckBox->setChecked(vf->getDataFileWasZipped());
         af = vf;
      }
   }
   else if (filterName == FileFilters::getVolumeFunctionalFileFilter()) {
      volumeFunctionalOptionsGroupBox->show();
      VolumeFile* vf = 0;
      if (volumeFunctionalFileSelectionComboBox->getAllVolumesSelected()) {
         if (brainSet->getNumberOfVolumeFunctionalFiles() > 0) {
            VolumeFile* temp = brainSet->getVolumeFunctionalFile(0);
            af = temp;
            fileName = temp->getFileName();
            volumeFunctionalVolumeDataTypeComboBox->setVolumeVoxelDataType(temp->getVoxelDataType());
            volumeFunctionalWriteCompressedCheckBox->setChecked(temp->getDataFileWasZipped());
            
            VolumeFile vf;
            getSupportedEncodings(vf, fileAvailableEncodings, preferredEncoding);            
         }
      }
      else {
         vf = volumeFunctionalFileSelectionComboBox->getSelectedVolumeFile();
      }
      if (vf != NULL) {
         volumeFunctionalLabelLineEdit->setText(vf->getDescriptiveLabel());
         volumeFunctionalVolumeDataTypeComboBox->setVolumeVoxelDataType(vf->getVoxelDataType());
         volumeFunctionalWriteCompressedCheckBox->setChecked(vf->getDataFileWasZipped());
         af = vf;
      }
   }
   else if (filterName == FileFilters::getVolumePaintFileFilter()) {
      volumePaintOptionsGroupBox->show();
      VolumeFile* vf = 0;
      if (volumePaintFileSelectionComboBox->getAllVolumesSelected()) {
         if (brainSet->getNumberOfVolumePaintFiles() > 0) {
            VolumeFile* temp = brainSet->getVolumePaintFile(0);
            af = temp;
            fileName = temp->getFileName();
            volumePaintVolumeDataTypeComboBox->setVolumeVoxelDataType(temp->getVoxelDataType());
            volumePaintWriteCompressedCheckBox->setChecked(temp->getDataFileWasZipped());
            
            VolumeFile vf;
            getSupportedEncodings(vf, fileAvailableEncodings, preferredEncoding);            
         }
      }
      else {
         vf = volumePaintFileSelectionComboBox->getSelectedVolumeFile();
      }
      if (vf != NULL) {
         volumePaintLabelLineEdit->setText(vf->getDescriptiveLabel());
         volumePaintVolumeDataTypeComboBox->setVolumeVoxelDataType(vf->getVoxelDataType());
         volumePaintWriteCompressedCheckBox->setChecked(vf->getDataFileWasZipped());
         af = vf;
      }
   }
   else if (filterName == FileFilters::getVolumeProbAtlasFileFilter()) {
      volumeProbAtlasOptionsGroupBox->show();
      VolumeFile* vf = 0;
      if (volumeProbAtlasFileSelectionComboBox->getAllVolumesSelected() ||
          alwaysSaveAllProbAtlasVolumes) {
         if (brainSet->getNumberOfVolumeProbAtlasFiles() > 0) {
            VolumeFile* temp = brainSet->getVolumeProbAtlasFile(0);
            af = temp;
            fileName = temp->getFileName();
            volumeProbAtlasVolumeDataTypeComboBox->setVolumeVoxelDataType(temp->getVoxelDataType());
            volumeProbAtlasWriteCompressedCheckBox->setChecked(temp->getDataFileWasZipped());
            
            VolumeFile vf;
            getSupportedEncodings(vf, fileAvailableEncodings, preferredEncoding);            
         }
      }
      else {
         vf = volumeProbAtlasFileSelectionComboBox->getSelectedVolumeFile();
      }
      if (vf != NULL) {
         volumeProbAtlasLabelLineEdit->setText(vf->getDescriptiveLabel());
         volumeProbAtlasVolumeDataTypeComboBox->setVolumeVoxelDataType(vf->getVoxelDataType());
         volumeProbAtlasWriteCompressedCheckBox->setChecked(vf->getDataFileWasZipped());
         af = vf;
      }
   }
   else if (filterName == FileFilters::getVolumeRgbFileFilter()) {
      volumeRgbOptionsGroupBox->show();
      VolumeFile* vf = 0;
      if (volumeRgbFileSelectionComboBox->getAllVolumesSelected()) {
         if (brainSet->getNumberOfVolumeRgbFiles() > 0) {
            VolumeFile* temp = brainSet->getVolumeRgbFile(0);
            af = temp;
            fileName = temp->getFileName();
            volumeRgbVolumeDataTypeComboBox->setVolumeVoxelDataType(temp->getVoxelDataType());
            volumeRgbWriteCompressedCheckBox->setChecked(temp->getDataFileWasZipped());
            
            VolumeFile vf;
            getSupportedEncodings(vf, fileAvailableEncodings, preferredEncoding);            
         }
      }
      else {
         vf = volumeRgbFileSelectionComboBox->getSelectedVolumeFile();
      }
      if (vf != NULL) {
         volumeRgbLabelLineEdit->setText(vf->getDescriptiveLabel());
         volumeRgbVolumeDataTypeComboBox->setVolumeVoxelDataType(vf->getVoxelDataType());
         volumeRgbWriteCompressedCheckBox->setChecked(vf->getDataFileWasZipped());
         af = vf;
      }
   }
   else if (filterName == FileFilters::getVolumeSegmentationFileFilter()) {
      volumeSegmentationOptionsGroupBox->show();
      VolumeFile* vf = 0;
      if (volumeSegmentationFileSelectionComboBox->getAllVolumesSelected()) {
         if (brainSet->getNumberOfVolumeSegmentationFiles() > 0) {
            VolumeFile* temp = brainSet->getVolumeSegmentationFile(0);
            af = temp;
            fileName = temp->getFileName();
            volumeSegmentationVolumeDataTypeComboBox->setVolumeVoxelDataType(temp->getVoxelDataType());
            volumeSegmentationWriteCompressedCheckBox->setChecked(temp->getDataFileWasZipped());
            
            VolumeFile vf;
            getSupportedEncodings(vf, fileAvailableEncodings, preferredEncoding);            
         }
      }
      else {
         vf = volumeSegmentationFileSelectionComboBox->getSelectedVolumeFile();
      }
      if (vf != NULL) {
         volumeSegmentationLabelLineEdit->setText(vf->getDescriptiveLabel());
         volumeSegmentationVolumeDataTypeComboBox->setVolumeVoxelDataType(vf->getVoxelDataType());
         volumeSegmentationWriteCompressedCheckBox->setChecked(vf->getDataFileWasZipped());
         af = vf;
      }
   }
   else if (filterName == FileFilters::getVolumeVectorFileFilter()) {
      volumeVectorOptionsGroupBox->show();
      VolumeFile* vf = 0;
      if (volumeVectorFileSelectionComboBox->getAllVolumesSelected()) {
         if (brainSet->getNumberOfVolumeVectorFiles() > 0) {
            VolumeFile* temp = brainSet->getVolumeVectorFile(0);
            af = temp;
            fileName = temp->getFileName();
            volumeVectorVolumeDataTypeComboBox->setVolumeVoxelDataType(temp->getVoxelDataType());
            volumeVectorWriteCompressedCheckBox->setChecked(temp->getDataFileWasZipped());
            
            VolumeFile vf;
            getSupportedEncodings(vf, fileAvailableEncodings, preferredEncoding);            
         }
      }
      else {
         vf = volumeVectorFileSelectionComboBox->getSelectedVolumeFile();
      }
      if (vf != NULL) {
         volumeVectorLabelLineEdit->setText(vf->getDescriptiveLabel());
         volumeVectorVolumeDataTypeComboBox->setVolumeVoxelDataType(vf->getVoxelDataType());
         volumeVectorWriteCompressedCheckBox->setChecked(vf->getDataFileWasZipped());
         af = vf;
      }
   }
   else if (filterName == FileFilters::getVtkModelFileFilter()) {
      vtkModelOptionsGroupBox->show();
      const int indx = vtkModelSelectionComboBox->currentIndex();
      if ((indx >= 0) && (indx < brainSet->getNumberOfVtkModelFiles())) {
         af = brainSet->getVtkModelFile(indx);
      }
   }
   else if (filterName == FileFilters::getWustlRegionFileFilter()) {
      af = brainSet->getWustlRegionFile();
   }
   else if (filterName == FileFilters::getAnalyzeVolumeFileFilter()) {
      exportVolumeOptionsGroupBox->show();
   }
   else if (filterName == FileFilters::getByuSurfaceFileFilter()) {
      exportSurfaceOptionsGroupBox->show();
   }
   else if (filterName == FileFilters::getFreeSurferAsciiSurfaceFileFilter()) {
      exportSurfaceOptionsGroupBox->show();
   }
   else if (filterName == FileFilters::getFreeSurferAsciiCurvatureFileFilter()) {
      exportShapeOptionsGroupBox->show();
   }
   else if (filterName == FileFilters::getFreeSurferAsciiFunctionalFileFilter()) {
      exportMetricOptionsGroupBox->show();
   }
   else if (filterName == FileFilters::getFreeSurferAsciiLabelFileFilter()) {
      exportPaintOptionsGroupBox->show();
   }
   else if (filterName == FileFilters::getMincVolumeFileFilter()) {
      exportVolumeOptionsGroupBox->show();
   }
   else if (filterName == FileFilters::getOpenInventorSurfaceFileFilter()) {
      exportSurfaceOptionsGroupBox->show();
   }
   else if (filterName == FileFilters::getStlSurfaceFileFilter()) {
      exportSurfaceOptionsGroupBox->show();
   }
   else if (filterName == FileFilters::getVrmlSurfaceFileFilter()) {
      exportSurfaceOptionsGroupBox->show();
   }
   else if (filterName == FileFilters::getVtkSurfaceFileFilter()) {
      exportSurfaceOptionsGroupBox->show();
   }
   else if (filterName == FileFilters::getVtkXmlSurfaceFileFilter()) {
      exportSurfaceOptionsGroupBox->show();
   }
   else if (filterName == FileFilters::getVtkVolumeFileFilter()) {
      exportSurfaceOptionsGroupBox->show();
   }
   else {
      std::cerr << "PROGRAM ERROR: unhandled file type " << filterName.toAscii().constData() 
                << " at " << __LINE__
                << " in " << __FILE__ << std::endl;
      return;
   }

   //
   // Get comment, pubmed, name, and encodings from file
   //
   if (af != NULL) {
      //SpecFile* sf = theMainWindow->getBrainSet()->getSpecFile();
      //if (sf->getFileNamePath().empty()  == false) {
      //   sf->setCurrentDirectoryToSpecFileDirectory();
      //}
      fileComment = af->getFileComment();
      filePubMedID = af->getFilePubMedID();
      fileName = af->getFileName();
      getSupportedEncodings(*af, fileAvailableEncodings, preferredEncoding);
      preferredEncoding = af->getFileWriteType();
   }
   
   //
   // Update name, comment and pubmed id in dialog
   //
   fileNameLineEdit->setText(fileName);
   metadataCommentTextEdit->setText(fileComment);
   metadataPubMedIDLineEdit->setText(filePubMedID);
   
   //
   // Update the encoding combo box
   //
   int fileEncodingComboBoxCurrentItem = 0;
   fileEncodingComboBox->clear();
   if (fileAvailableEncodings.empty()) {
      fileAvailableEncodings.push_back(AbstractFile::FILE_FORMAT_OTHER);
   }
   for (unsigned int i = 0; i < fileAvailableEncodings.size(); i++) {
      const AbstractFile::FILE_FORMAT encoding = fileAvailableEncodings[i];
      QString text = "";
      switch (encoding) {
         case AbstractFile::FILE_FORMAT_ASCII:
            text = "ASCII (Text)";
            break;
         case AbstractFile::FILE_FORMAT_BINARY:
            text = "Binary";
            break;
         case AbstractFile::FILE_FORMAT_XML:
            text = "XML";
            break;
         case AbstractFile::FILE_FORMAT_XML_BASE64:
            text = "XML Base64 Encoded Binary";
            break;
         case AbstractFile::FILE_FORMAT_XML_GZIP_BASE64:
            text = "XML GZip Base64 Encoded Binary";
            break;
         case AbstractFile::FILE_FORMAT_XML_EXTERNAL_BINARY:
            text = "XML External Binary";
            break;
         case AbstractFile::FILE_FORMAT_OTHER:
            text = "Other";
            break;
         case AbstractFile::FILE_FORMAT_COMMA_SEPARATED_VALUE_FILE:
            text = "Comma Separated Value File";
            break;
      }

      if (preferredEncoding == encoding) {
         fileEncodingComboBoxCurrentItem = i;
      }

      fileEncodingComboBox->addItem(text, QVariant(int(encoding)));
   }
   if ((fileEncodingComboBoxCurrentItem >= 0) &&
       (fileEncodingComboBoxCurrentItem < fileEncodingComboBox->count())) {
      fileEncodingComboBox->setCurrentIndex(fileEncodingComboBoxCurrentItem);
   }
   
   //
   // Dialog has been resized (maybe)
   //
   updateGeometry();
}

/**
 * called when name line edit changed.
 */
void 
GuiDataFileSaveDialog::slotFileNameLineEditChanged(const QString& text)
{
   savePushButton->setEnabled(text.isEmpty() == false);
}
      
/**
 * update file extension.
 */
void 
GuiDataFileSaveDialog::updateFileNamesFileExtension(QString& fileName,
                                                    const QString& fileExtension)
{
   //
   // Add extension ?
   //
   if (addFileExtensionCheckBox->isChecked()) {
      if (fileExtension.isEmpty() == false) {
         if (fileName.endsWith(fileExtension) == false) {
            fileName += fileExtension;
         }
      }
   }
   
}
                                 
/**
 * update metadata and name.
 */
void 
GuiDataFileSaveDialog::updateMetadataAndName(AbstractFile* af,
                                             QString& fileName,
                                             const QString& fileExtension)
{
   //
   // Set file encoding
   //
   const int indx = fileEncodingComboBox->currentIndex();
   af->setFileWriteType(static_cast<AbstractFile::FILE_FORMAT>(
                                          fileEncodingComboBox->itemData(indx).toInt()));
   
   //
   // Set PubMed ID
   //
   af->setFilePubMedID(metadataPubMedIDLineEdit->text());
   
   //
   // Set comment
   //
   af->setFileComment(metadataCommentTextEdit->toPlainText());
   
   //
   // Update file name extension
   //
   updateFileNamesFileExtension(fileName, fileExtension);
}
                                 
/**
 * called when accept/reject pressed.
 */
void 
GuiDataFileSaveDialog::done(int r)
{
   
   if (r == GuiDataFileSaveDialog::Accepted) {
      BrainSet* brainSet = theMainWindow->getBrainSet();
      QString fileName = fileNameLineEdit->text();
      
      if (fileName.isEmpty()) {
         QMessageBox::critical(this,
                               "ERROR",
                               "File name is empty.",
                               QMessageBox::Ok);
         return;
      }
      const QString fileNameNoPath = FileUtilities::basename(fileName);
      
      QFileInfo fi(fileName);
      if (fi.isDir()) {
         QMessageBox::critical(this,
                               "ERROR",
                               fileNameNoPath + " is a directory.",
                               QMessageBox::Ok);
         return;
      }
      
      if (fi.exists()) {
         const QString message(fileName 
                               + QString(" already exists.\n")
                               + QString("Do you want to replace it?"));
         if (QMessageBox::question(this,
                                   "Confirm Overwrite",
                                   message,
                                   QMessageBox::Yes |
                                      QMessageBox::No) == QMessageBox::No) {
            return;
         }
      }
   
      //
      // Allow user to set the spec file name if not already set
      //
      if (brainSet->getSpecFileName().isEmpty()) {
         if (QMessageBox::information(this, 
                                      "Spec File", 
                                      "Would you like to create a Spec File ?",
                                      QMessageBox::Yes |
                                         QMessageBox::No) == QMessageBox::Yes) {
            //
            // Create spec file dialog will set the directory and create the spec file
            //
            GuiSpecFileCreationDialog sfcd(this);
            sfcd.exec();
            PreferencesFile* pf = brainSet->getPreferencesFile();
            pf->addToRecentSpecFiles(sfcd.getCreatedSpecFileName(), true);
         }
      }

      //
      // Type of file being saved
      //
      const QString filterName = fileTypeComboBox->currentText();
      
      const AbstractFile::FILE_FORMAT fileFormat = 
               static_cast<AbstractFile::FILE_FORMAT>(
                  fileEncodingComboBox->itemData(fileEncodingComboBox->currentIndex()).toInt());
/*
            *  = brainSet->get();
            updateMetadataAndName(,
                                  fileName,
                                  SpecFile::getExtension());
            brainSet->write(fileName);
*/
      try {
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         
         if (filterName == FileFilters::getAreaColorFileFilter()) {
            AreaColorFile* acf = brainSet->getAreaColorFile();
            updateMetadataAndName(acf,
                                  fileName,
                                  SpecFile::getAreaColorFileExtension());
            brainSet->writeAreaColorFile(fileName);
         }
         else if (filterName == FileFilters::getArealEstimationFileFilter()) {
            ArealEstimationFile* aef = brainSet->getArealEstimationFile();
            updateMetadataAndName(aef,
                                  fileName,
                                  SpecFile::getArealEstimationFileExtension());
            brainSet->writeArealEstimationFile(fileName);
         }
         else if (filterName == FileFilters::getBorderGenericFileFilter()) {
            BrainModelSurface* borderSurface = 
               borderSurfaceSelectionComboBox->getSelectedBrainModelSurface();
            if (borderSurface != NULL) {
               if (addFileExtensionCheckBox->isChecked()) {
                  if (fileName.endsWith(SpecFile::getBorderFileExtension()) == false) {
                     fileName += SpecFile::getBorderFileExtension();
                  }
               }
               brainSet->writeBorderFile(
                                   fileName, 
                                   borderSurface, 
                                   borderSurfaceTypeComboBox->getSurfaceType(), 
                                   metadataCommentTextEdit->toPlainText(),
                                   metadataPubMedIDLineEdit->text(), 
                                   borderSurfaceRemoveDuplicatesCheckBox->isChecked());
            }
         }
         else if (filterName == FileFilters::getBorderVolumeFileFilter()) {
            BorderFile* bf = brainSet->getVolumeBorderFile();
            updateMetadataAndName(bf,
                                  fileName,
                                  SpecFile::getBorderFileExtension());
            theMainWindow->getBrainSet()->writeVolumeBorderFile(fileName);
         }
         else if (filterName == FileFilters::getBorderColorFileFilter()) {
            BorderColorFile* bcf = brainSet->getBorderColorFile();
            updateMetadataAndName(bcf,
                                  fileName,
                                  SpecFile::getBorderColorFileExtension());
            brainSet->writeBorderColorFile(fileName);
         }
         else if (filterName == FileFilters::getBorderProjectionFileFilter()) {
            if (addFileExtensionCheckBox->isChecked()) {
               if (fileName.endsWith(SpecFile::getBorderProjectionFileExtension()) == false) {
                  fileName += SpecFile::getBorderProjectionFileExtension();
               }
            }
            brainSet->writeBorderProjectionFile(fileName, 
                                   metadataCommentTextEdit->toPlainText(),
                                   metadataPubMedIDLineEdit->text(), 
                                   borderProjectionsRemoveDuplicatesCheckBox->isChecked());
         }
         else if (filterName == FileFilters::getCellFileFilter()) {
            const BrainModelSurface* bms = 
               cellSurfaceSelectionComboBox->getSelectedBrainModelSurface();
            if (bms == NULL) {
               const QString msg("There is no surface for which cells should be saved.");
               QApplication::restoreOverrideCursor();
               QMessageBox::critical(this, "ERROR", msg, QMessageBox::Ok);
               return;
            }
            if (addFileExtensionCheckBox->isChecked()) {
               if (fileName.endsWith(SpecFile::getCellFileExtension()) == false) {
                  fileName += SpecFile::getCellFileExtension();
               }
            }
            brainSet->writeCellFile(fileName, 
                                    bms, 
                                    fileFormat, 
                                    metadataCommentTextEdit->toPlainText());
         }
         else if (filterName == FileFilters::getCellColorFileFilter()) {
            CellColorFile* ccf = brainSet->getCellColorFile();
            updateMetadataAndName(ccf,
                                  fileName,
                                  SpecFile::getCellColorFileExtension());
            brainSet->writeCellColorFile(fileName);
         }
         else if (filterName == FileFilters::getCellProjectionFileFilter()) {
            CellProjectionFile* cpf = brainSet->getCellProjectionFile();
            updateMetadataAndName(cpf,
                                  fileName,
                                  SpecFile::getCellProjectionFileExtension());
            brainSet->writeCellProjectionFile(fileName);
         }
         else if (filterName == FileFilters::getCellVolumeFileFilter()) {
            CellFile*  cf = brainSet->getVolumeCellFile();
            updateMetadataAndName(cf,
                                  fileName,
                                  SpecFile::getCellFileExtension());
            brainSet->writeVolumeCellFile(fileName);
         }
         else if (filterName == FileFilters::getCocomacFileFilter()) {
            CocomacConnectivityFile* cf = brainSet->getCocomacFile();
            updateMetadataAndName(cf,
                                  fileName,
                                  SpecFile::getCocomacConnectivityFileExtension());
            brainSet->writeCocomacConnectivityFile(fileName);
         }
         else if (filterName == FileFilters::getContourFileFilter()) {
            BrainModelContours* bmc = brainSet->getBrainModelContours();
            if (bmc != NULL) {
               ContourFile* cf = bmc->getContourFile();
               updateMetadataAndName(cf,
                                     fileName,
                                     SpecFile::getContourFileExtension());
               brainSet->writeContourFile(fileName, cf);
            }
         }
         else if (filterName == FileFilters::getContourCellFileFilter()) {
            ContourCellFile* ccf = brainSet->getContourCellFile();
            updateMetadataAndName(ccf,
                                  fileName,
                                  SpecFile::getContourCellFileExtension());
            brainSet->writeContourCellFile(fileName);
         }
         else if (filterName == FileFilters::getContourCellColorFileFilter()) {
            ContourCellColorFile* cccf = brainSet->getContourCellColorFile();
            updateMetadataAndName(cccf,
                                  fileName,
                                  SpecFile::getContourCellColorFileExtension());
            brainSet->writeContourCellColorFile(fileName);
         }
         else if (filterName == FileFilters::getCoordinateGenericFileFilter()) {
            BrainModelSurface* bms = 
               coordinateSurfaceSelectionComboBox->getSelectedBrainModelSurface();
            if (bms == NULL) {
               QApplication::restoreOverrideCursor();
               QMessageBox::critical(this, 
                                     "ERROR", 
                                     "No surface selected.",
                                     QMessageBox::Ok);
               return;
            }
            bms->setStructure(coordinateStructureComboBox->getSelectedStructure());
            CoordinateFile* cf = bms->getCoordinateFile();
            cf->setHeaderTag(AbstractFile::headerTagConfigurationID,
                             coordinateStereotaxicSpaceComboBox->currentText());
            updateMetadataAndName(cf,
                                  fileName,
                                  SpecFile::getCoordinateFileExtension());
            brainSet->writeCoordinateFile(fileName,
                                          coordinateTypeComboBox->getSurfaceType(),
                                          cf);
            GuiFilesModified fm;
            fm.setCoordinateModified();
            theMainWindow->fileModificationUpdate(fm);
         }
         else if (filterName == FileFilters::getCutsFileFilter()) {
            CutsFile* cf = brainSet->getCutsFile();
            updateMetadataAndName(cf,
                                  fileName,
                                  SpecFile::getCutsFileExtension());
            brainSet->writeCutsFile(fileName);
         }
         else if (filterName == FileFilters::getDeformationFieldFileFilter()) {
            DeformationFieldFile* dff = brainSet->getDeformationFieldFile();
            updateMetadataAndName(dff,
                                  fileName,
                                  SpecFile::getDeformationFieldFileExtension());
            brainSet->writeDeformationFieldFile(fileName);
         }
         else if (filterName == FileFilters::getFociFileFilter()) {
            if (addFileExtensionCheckBox->isChecked()) {
               if (fileName.endsWith(SpecFile::getFociFileExtension()) == false) {
                  fileName += SpecFile::getFociFileExtension();
               }
            }
            if (fociSaveOriginalCoordinatesRadioButton->isChecked()) {
               theMainWindow->getBrainSet()->writeFociFileOriginalCoordinates(fileName, 
                                                                              fileFormat, 
                                                      metadataCommentTextEdit->toPlainText());
            }
            else if (fociSaveProjectedCoordinatesRadioButton->isChecked()) {
               const BrainModelSurface* leftBms = fociLeftSurfaceSelectionComboBox->getSelectedBrainModelSurface();
               const BrainModelSurface* rightBms = fociRightSurfaceSelectionComboBox->getSelectedBrainModelSurface();
               const BrainModelSurface* cerebellumBms = fociCerebellumSurfaceSelectionComboBox->getSelectedBrainModelSurface();
               if ((leftBms == NULL) 
                   && (rightBms == NULL)
                   && (cerebellumBms == NULL)) {
                  QString msg("There is no surface selected for which foci should be saved.");
                  QApplication::restoreOverrideCursor();
                  QMessageBox::critical(this, "ERROR", msg, QMessageBox::Ok);
                  return;
               }               
               brainSet->writeFociFile(fileName, 
                                       leftBms, 
                                       rightBms, 
                                       cerebellumBms,
                                       fileFormat, 
                                       metadataCommentTextEdit->toPlainText());
            }
         }
         else if (filterName == FileFilters::getFociColorFileFilter()) {
            FociColorFile* fcf = brainSet->getFociColorFile();
            updateMetadataAndName(fcf,
                                  fileName,
                                  SpecFile::getFociColorFileExtension());
            brainSet->writeFociColorFile(fileName);
         }
         else if (filterName == FileFilters::getFociProjectionFileFilter()) {
            FociProjectionFile* fpf = brainSet->getFociProjectionFile();
            updateMetadataAndName(fpf,
                                  fileName,
                                  SpecFile::getFociProjectionFileExtension());
            brainSet->writeFociProjectionFile(fileName);
         }
         else if (filterName == FileFilters::getFociSearchFileFilter()) {
            FociSearchFile* fsf = brainSet->getFociSearchFile();
            updateMetadataAndName(fsf,
                                  fileName,
                                  SpecFile::getFociSearchFileExtension());
            brainSet->writeFociSearchFile(fileName);
         }
         else if (filterName == FileFilters::getGeodesicDistanceFileFilter()) {
            GeodesicDistanceFile* gdf = brainSet->getGeodesicDistanceFile();
            updateMetadataAndName(gdf,
                                  fileName,
                                  SpecFile::getGeodesicDistanceFileExtension());
            brainSet->writeGeodesicDistanceFile(fileName);
         }
         else if (filterName == FileFilters::getGiftiCoordinateFileFilter()) {
            BrainModelSurface* bms = 
               coordinateSurfaceSelectionComboBox->getSelectedBrainModelSurface();
            if (bms == NULL) {
               QApplication::restoreOverrideCursor();
               QMessageBox::critical(this, 
                                     "ERROR", 
                                     "No surface selected.",
                                     QMessageBox::Ok);
               return;
            }
            bms->setStructure(coordinateStructureComboBox->getSelectedStructure());
            CoordinateFile* cf = bms->getCoordinateFile();
            cf->setHeaderTag(AbstractFile::headerTagConfigurationID,
                             coordinateStereotaxicSpaceComboBox->currentText());
            updateMetadataAndName(cf,
                                  fileName,
                                  SpecFile::getGiftiCoordinateFileExtension());
            brainSet->writeCoordinateFile(fileName,
                                          coordinateTypeComboBox->getSurfaceType(),
                                          cf);
            GuiFilesModified fm;
            fm.setCoordinateModified();
            theMainWindow->fileModificationUpdate(fm);
         }
         else if (filterName == FileFilters::getGiftiFunctionalFileFilter()) {
            MetricFile* mf = brainSet->getMetricFile();
            updateMetadataAndName(mf,
                                  fileName,
                                  SpecFile::getGiftiFunctionalFileExtension());
            mf->setFileWriteType(fileFormat);
            brainSet->writeMetricFile(fileName);
         }
         else if (filterName == FileFilters::getGiftiLabelFileFilter()) {
            PaintFile* pf = brainSet->getPaintFile();
            updateMetadataAndName(pf,
                                  fileName,
                                  SpecFile::getGiftiLabelFileExtension());
            pf->assignColors(*brainSet->getAreaColorFile());
            pf->setFileWriteType(fileFormat);
            brainSet->writePaintFile(fileName);
         }
         else if (filterName == FileFilters::getGiftiShapeFileFilter()) {
            SurfaceShapeFile* ssf = brainSet->getSurfaceShapeFile();
            updateMetadataAndName(ssf,
                                  fileName,
                                  SpecFile::getGiftiShapeFileExtension());
            ssf->setFileWriteType(fileFormat);
            brainSet->writeSurfaceShapeFile(fileName);
         }
         else if (filterName == FileFilters::getGiftiSurfaceFileFilter()) {
            BrainModelSurface* bms = giftiSurfaceSelectionControl->getSelectedBrainModelSurface();
            if (bms != NULL) {
               updateMetadataAndName(bms->getCoordinateFile(),
                                     fileName,
                                     "");
               updateFileNamesFileExtension(fileName,
                                            SpecFile::getGiftiSurfaceFileExtension());
               brainSet->writeSurfaceFile(fileName,
                                          bms->getSurfaceType(),
                                          bms,
                                          true,
                                          fileFormat);
            }
         }
         else if (filterName == FileFilters::getGiftiTopologyFileFilter()) {
            TopologyFile* tf = topologySelectionComboBox->getSelectedTopologyFile();
            if (tf != NULL) {
               updateMetadataAndName(tf,
                                     fileName,
                                     SpecFile::getGiftiTopologyFileExtension());
               brainSet->writeTopologyFile(fileName, 
                                           topologyTypeComboBox->getTopologyType(), 
                                           tf);
            }
         }
         else if (filterName == FileFilters::getImageSaveFileFilter()) {
            const int indx = imageSelectionComboBox->currentIndex();
            if ((indx >= 0) && (indx < brainSet->getNumberOfImageFiles())) {
               ImageFile* img = brainSet->getImageFile(indx);
               updateMetadataAndName(img,
                                     fileName,
                                     "");
               brainSet->writeImageFile(fileName, img);
            }
         }
         else if (filterName == FileFilters::getLatitudeLongitudeFileFilter()) {
            LatLonFile* llf = brainSet->getLatLonFile();
            updateMetadataAndName(llf,
                                  fileName,
                                  SpecFile::getLatLonFileExtension());
            brainSet->writeLatLonFile(fileName);
         }
         else if (filterName == FileFilters::getMetricFileFilter()) {
            MetricFile* mf = brainSet->getMetricFile();
            updateMetadataAndName(mf,
                                  fileName,
                                  SpecFile::getMetricFileExtension());
            brainSet->writeMetricFile(fileName);
         }
         else if (filterName == FileFilters::getPaintFileFilter()) {
            PaintFile* pf = brainSet->getPaintFile();
            updateMetadataAndName(pf,
                                  fileName,
                                  SpecFile::getPaintFileExtension());
            brainSet->writePaintFile(fileName);
         }
         else if (filterName == FileFilters::getPaletteFileFilter()) {
            PaletteFile* pf = brainSet->getPaletteFile();
            updateMetadataAndName(pf,
                                  fileName,
                                  SpecFile::getPaletteFileExtension());
            brainSet->writePaletteFile(fileName);
         }
         else if (filterName == FileFilters::getParamsFileFilter()) {
            ParamsFile* pf = brainSet->getParamsFile();
            updateMetadataAndName(pf,
                                  fileName,
                                  SpecFile::getParamsFileExtension());
            brainSet->writeParamsFile(fileName);
         }
         else if (filterName == FileFilters::getProbAtlasFileFilter()) {
            ProbabilisticAtlasFile* paf = brainSet->getProbabilisticAtlasSurfaceFile();
            updateMetadataAndName(paf,
                                  fileName,
                                  SpecFile::getProbabilisticAtlasFileExtension());
            brainSet->writeProbabilisticAtlasFile(fileName);
         }
         else if (filterName == FileFilters::getRgbPaintFileFilter()) {
            RgbPaintFile* rpf = brainSet->getRgbPaintFile();
            updateMetadataAndName(rpf,
                                  fileName,
                                  SpecFile::getRgbPaintFileExtension());
            brainSet->writeRgbPaintFile(fileName);
         }
         else if (filterName == FileFilters::getSceneFileFilter()) {
            SceneFile* sf = brainSet->getSceneFile();
            updateMetadataAndName(sf,
                                  fileName,
                                  SpecFile::getSceneFileExtension());
            brainSet->writeSceneFile(fileName);
         }
         else if (filterName == FileFilters::getSectionFileFilter()) {
            SectionFile* sf = brainSet->getSectionFile();
            updateMetadataAndName(sf,
                                  fileName,
                                  SpecFile::getSectionFileExtension());
            brainSet->writeSectionFile(fileName);
         }
         else if (filterName == FileFilters::getStudyCollectionFileFilter()) {
            StudyCollectionFile* scf = brainSet->getStudyCollectionFile();
            updateMetadataAndName(scf,
                                  fileName,
                                  SpecFile::getStudyCollectionFileExtension());
            brainSet->writeStudyCollectionFile(fileName);
         }
         else if (filterName == FileFilters::getStudyMetaDataFileFilter()) {
            StudyMetaDataFile* smdf = brainSet->getStudyMetaDataFile();
            const int count = smdf->getNumberOfStudyMetaDatWithoutProvenceEntries();
            if (count > 0) {
               QApplication::restoreOverrideCursor();
               const QString msg("There are " 
                           + QString::number(count)
                           + " studies lacking provenance entries.\n"
                           "Please enter the provenance data below.\n");
                          // "If you press \"Cancel\" the file will NOT be saved.");
               StudyMetaData::Provenance p;
               std::vector<QString> rowLabels, rowValues;
               rowLabels.push_back("Name");      rowValues.push_back(p.getName());
               rowLabels.push_back("Date");      rowValues.push_back(p.getDate());
               rowLabels.push_back("Comment");   rowValues.push_back(p.getComment());
               QtMultipleInputDialog mid(this,
                                         "Add Provenance",
                                         msg,
                                         rowLabels,
                                         rowValues,
                                         false,
                                         true);
               if (mid.exec() == QtMultipleInputDialog::Accepted) {
                  std::vector<QString> values;
                  mid.getValues(values);
                  smdf->addProvenanceToStudiesWithoutProvenanceEntries(values[0],
                                                                       values[1],
                                                                       values[2]);
                  GuiFilesModified fm;
                  fm.setStudyMetaDataModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
               QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            }
            
            updateMetadataAndName(smdf,
                                  fileName,
                                  SpecFile::getStudyMetaDataFileExtension());
            brainSet->writeStudyMetaDataFile(fileName);
         }
         else if (filterName == FileFilters::getSurfaceShapeFileFilter()) {
            SurfaceShapeFile* ssf = brainSet->getSurfaceShapeFile();
            updateMetadataAndName(ssf,
                                  fileName,
                                  SpecFile::getSurfaceShapeFileExtension());
            brainSet->writeSurfaceShapeFile(fileName);
         }
         else if (filterName == FileFilters::getGiftiVectorFileFilter()) {
            VectorFile* vf = vectorFileSelectionComboBox->getSelectedVectorFile();
            if (vf != NULL) {
               updateMetadataAndName(vf,
                                     fileName,
                                     SpecFile::getGiftiVectorFileExtension());
               brainSet->writeVectorFile(vf, fileName);
            }
         }
         else if (filterName == FileFilters::getTopographyFileFilter()) {
            TopographyFile* tf = brainSet->getTopographyFile();
            updateMetadataAndName(tf,
                                  fileName,
                                  SpecFile::getTopographyFileExtension());
            brainSet->writeTopographyFile(fileName);
         }
         else if (filterName == FileFilters::getTopologyGenericFileFilter()) {
            TopologyFile* tf = topologySelectionComboBox->getSelectedTopologyFile();
            if (tf != NULL) {
               updateMetadataAndName(tf,
                                     fileName,
                                     SpecFile::getTopoFileExtension());
               brainSet->writeTopologyFile(fileName, 
                                           topologyTypeComboBox->getTopologyType(), 
                                           tf);
            }
         }
         else if (filterName == FileFilters::getTransformationMatrixFileFilter()) {
            TransformationMatrixFile* tmf = brainSet->getTransformationMatrixFile();
            updateMetadataAndName(tmf,
                                  fileName,
                                  SpecFile::getTransformationMatrixFileExtension());
            brainSet->writeTransformationMatrixFile(fileName);
         }
         else if (filterName == FileFilters::getTrajectoryFileFilter()) {
        	 TrajectoryFile* tjf = brainSet->getTrajectoryFile();
        	 brainSet->writeTrajectoryFile(fileName);
         }
         else if (filterName == FileFilters::getVocabularyFileFilter()) {
            VocabularyFile* vf = brainSet->getVocabularyFile();
            updateMetadataAndName(vf,
                                  fileName,
                                  SpecFile::getVocabularyFileExtension());
            brainSet->writeVocabularyFile(fileName);
         }   
         else if (filterName == FileFilters::getVolumeAnatomyFileFilter()) {
            VolumeFile::VOXEL_DATA_TYPE voxelDataType =
               volumeAnatomyVolumeDataTypeComboBox->getVolumeVoxelDataType();
            if (volumeAnatomyFileSelectionComboBox->getAllVolumesSelected()) {
               std::vector<VolumeFile*> volumes;
               for (int i = 0; i < brainSet->getNumberOfVolumeAnatomyFiles(); i++) {
                  volumes.push_back(brainSet->getVolumeAnatomyFile(i));
               }
               if (volumes.empty() == false) {
                  brainSet->writeMultiVolumeFile(fileName,
                                                 VolumeFile::VOLUME_TYPE_ANATOMY,
                                                 metadataCommentTextEdit->toPlainText(), 
                                                 volumes,
                                                 voxelDataType,
                                                 volumeAnatomyWriteCompressedCheckBox->isChecked());
                                              
                  GuiFilesModified fm;
                  fm.setVolumeModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
            }
            else {
               VolumeFile* vf = volumeAnatomyFileSelectionComboBox->getSelectedVolumeFile();
               if (vf != NULL) {
                  vf->setDescriptiveLabel(volumeAnatomyLabelLineEdit->text());
                  updateMetadataAndName(vf,
                                        fileName,
                                        "");
                  brainSet->writeVolumeFile(fileName, 
                                            VolumeFile::FILE_READ_WRITE_TYPE_UNKNOWN, 
                                            VolumeFile::VOLUME_TYPE_ANATOMY, 
                                            vf,
                                            voxelDataType,
                                            volumeAnatomyWriteCompressedCheckBox->isChecked());
                  GuiFilesModified fm;
                  fm.setVolumeModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
            }
         }
         else if (filterName == FileFilters::getVolumeFunctionalFileFilter()) {
            VolumeFile::VOXEL_DATA_TYPE voxelDataType =
               volumeFunctionalVolumeDataTypeComboBox->getVolumeVoxelDataType();
            if (volumeFunctionalFileSelectionComboBox->getAllVolumesSelected()) {
               std::vector<VolumeFile*> volumes;
               for (int i = 0; i < brainSet->getNumberOfVolumeFunctionalFiles(); i++) {
                  volumes.push_back(brainSet->getVolumeFunctionalFile(i));
               }
               if (volumes.empty() == false) {
                  brainSet->writeMultiVolumeFile(fileName,
                                                 VolumeFile::VOLUME_TYPE_FUNCTIONAL,
                                                 metadataCommentTextEdit->toPlainText(), 
                                                 volumes,
                                                 voxelDataType,
                                                 volumeFunctionalWriteCompressedCheckBox->isChecked());
                                              
                  GuiFilesModified fm;
                  fm.setVolumeModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
            }
            else {
               VolumeFile* vf = volumeFunctionalFileSelectionComboBox->getSelectedVolumeFile();
               if (vf != NULL) {
                  vf->setDescriptiveLabel(volumeFunctionalLabelLineEdit->text());
                  updateMetadataAndName(vf,
                                        fileName,
                                        "");
                  brainSet->writeVolumeFile(fileName, 
                                            VolumeFile::FILE_READ_WRITE_TYPE_UNKNOWN, 
                                            VolumeFile::VOLUME_TYPE_FUNCTIONAL, 
                                            vf,
                                            voxelDataType,
                                            volumeFunctionalWriteCompressedCheckBox->isChecked());
                  GuiFilesModified fm;
                  fm.setVolumeModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
            }
         }
         else if (filterName == FileFilters::getVolumePaintFileFilter()) {
            VolumeFile::VOXEL_DATA_TYPE voxelDataType =
               volumePaintVolumeDataTypeComboBox->getVolumeVoxelDataType();
            if (volumePaintFileSelectionComboBox->getAllVolumesSelected()) {
               std::vector<VolumeFile*> volumes;
               for (int i = 0; i < brainSet->getNumberOfVolumePaintFiles(); i++) {
                  volumes.push_back(brainSet->getVolumePaintFile(i));
               }
               if (volumes.empty() == false) {
                  brainSet->writeMultiVolumeFile(fileName,
                                                 VolumeFile::VOLUME_TYPE_PAINT,
                                                 metadataCommentTextEdit->toPlainText(), 
                                                 volumes,
                                                 voxelDataType,
                                                 volumePaintWriteCompressedCheckBox->isChecked());
                                              
                  GuiFilesModified fm;
                  fm.setVolumeModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
            }
            else {
               VolumeFile* vf = volumePaintFileSelectionComboBox->getSelectedVolumeFile();
               if (vf != NULL) {
                  vf->setDescriptiveLabel(volumePaintLabelLineEdit->text());
                  updateMetadataAndName(vf,
                                        fileName,
                                        "");
                  brainSet->writeVolumeFile(fileName, 
                                            VolumeFile::FILE_READ_WRITE_TYPE_UNKNOWN, 
                                            VolumeFile::VOLUME_TYPE_PAINT, 
                                            vf,
                                            voxelDataType,
                                            volumePaintWriteCompressedCheckBox->isChecked());
                  GuiFilesModified fm;
                  fm.setVolumeModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
            }
         }
         else if (filterName == FileFilters::getVolumeProbAtlasFileFilter()) {
            VolumeFile::VOXEL_DATA_TYPE voxelDataType =
               volumeProbAtlasVolumeDataTypeComboBox->getVolumeVoxelDataType();
            if (volumeProbAtlasFileSelectionComboBox->getAllVolumesSelected() ||
                alwaysSaveAllProbAtlasVolumes) {
               std::vector<VolumeFile*> volumes;
               for (int i = 0; i < brainSet->getNumberOfVolumeProbAtlasFiles(); i++) {
                  volumes.push_back(brainSet->getVolumeProbAtlasFile(i));
               }
               if (volumes.empty() == false) {
                  brainSet->writeMultiVolumeFile(fileName,
                                                 VolumeFile::VOLUME_TYPE_PROB_ATLAS,
                                                 metadataCommentTextEdit->toPlainText(), 
                                                 volumes,
                                                 voxelDataType,
                                                 volumeProbAtlasWriteCompressedCheckBox->isChecked());
                                              
                  GuiFilesModified fm;
                  fm.setVolumeModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
            }
            else {
               VolumeFile* vf = volumeProbAtlasFileSelectionComboBox->getSelectedVolumeFile();
               if (vf != NULL) {
                  vf->setDescriptiveLabel(volumeProbAtlasLabelLineEdit->text());
                  updateMetadataAndName(vf,
                                        fileName,
                                        "");
                  brainSet->writeVolumeFile(fileName, 
                                            VolumeFile::FILE_READ_WRITE_TYPE_UNKNOWN, 
                                            VolumeFile::VOLUME_TYPE_PROB_ATLAS, 
                                            vf,
                                            voxelDataType,
                                            volumeProbAtlasWriteCompressedCheckBox->isChecked());
                  GuiFilesModified fm;
                  fm.setVolumeModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
            }
         }
         else if (filterName == FileFilters::getVolumeRgbFileFilter()) {
            VolumeFile::VOXEL_DATA_TYPE voxelDataType =
               volumeRgbVolumeDataTypeComboBox->getVolumeVoxelDataType();
            if (volumeRgbFileSelectionComboBox->getAllVolumesSelected()) {
               std::vector<VolumeFile*> volumes;
               for (int i = 0; i < brainSet->getNumberOfVolumeRgbFiles(); i++) {
                  volumes.push_back(brainSet->getVolumeRgbFile(i));
               }
               if (volumes.empty() == false) {
                  brainSet->writeMultiVolumeFile(fileName,
                                                 VolumeFile::VOLUME_TYPE_RGB,
                                                 metadataCommentTextEdit->toPlainText(), 
                                                 volumes,
                                                 voxelDataType,
                                                 volumeRgbWriteCompressedCheckBox->isChecked());
                                              
                  GuiFilesModified fm;
                  fm.setVolumeModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
            }
            else {
               VolumeFile* vf = volumeRgbFileSelectionComboBox->getSelectedVolumeFile();
               if (vf != NULL) {
                  vf->setDescriptiveLabel(volumeRgbLabelLineEdit->text());
                  updateMetadataAndName(vf,
                                        fileName,
                                        "");
                  brainSet->writeVolumeFile(fileName, 
                                            VolumeFile::FILE_READ_WRITE_TYPE_UNKNOWN, 
                                            VolumeFile::VOLUME_TYPE_RGB, 
                                            vf,
                                            voxelDataType,
                                            volumeRgbWriteCompressedCheckBox->isChecked());
                  GuiFilesModified fm;
                  fm.setVolumeModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
            }
         }
         else if (filterName == FileFilters::getVolumeSegmentationFileFilter()) {
            VolumeFile::VOXEL_DATA_TYPE voxelDataType =
               volumeSegmentationVolumeDataTypeComboBox->getVolumeVoxelDataType();
            if (volumeSegmentationFileSelectionComboBox->getAllVolumesSelected()) {
               std::vector<VolumeFile*> volumes;
               for (int i = 0; i < brainSet->getNumberOfVolumeSegmentationFiles(); i++) {
                  volumes.push_back(brainSet->getVolumeSegmentationFile(i));
               }
               if (volumes.empty() == false) {
                  brainSet->writeMultiVolumeFile(fileName,
                                                 VolumeFile::VOLUME_TYPE_SEGMENTATION,
                                                 metadataCommentTextEdit->toPlainText(), 
                                                 volumes,
                                                 voxelDataType,
                                                 volumeSegmentationWriteCompressedCheckBox->isChecked());
                                              
                  GuiFilesModified fm;
                  fm.setVolumeModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
            }
            else {
               VolumeFile* vf = volumeSegmentationFileSelectionComboBox->getSelectedVolumeFile();
               if (vf != NULL) {
                  vf->setDescriptiveLabel(volumeSegmentationLabelLineEdit->text());
                  updateMetadataAndName(vf,
                                        fileName,
                                        "");
                  brainSet->writeVolumeFile(fileName, 
                                            VolumeFile::FILE_READ_WRITE_TYPE_UNKNOWN, 
                                            VolumeFile::VOLUME_TYPE_SEGMENTATION, 
                                            vf,
                                            voxelDataType,
                                            volumeSegmentationWriteCompressedCheckBox->isChecked());
                  GuiFilesModified fm;
                  fm.setVolumeModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
            }
         }
         else if (filterName == FileFilters::getVolumeVectorFileFilter()) {
            VolumeFile::VOXEL_DATA_TYPE voxelDataType =
               volumeVectorVolumeDataTypeComboBox->getVolumeVoxelDataType();
            if (volumeVectorFileSelectionComboBox->getAllVolumesSelected()) {
               std::vector<VolumeFile*> volumes;
               for (int i = 0; i < brainSet->getNumberOfVolumeVectorFiles(); i++) {
                  volumes.push_back(brainSet->getVolumeVectorFile(i));
               }
               if (volumes.empty() == false) {
                  brainSet->writeMultiVolumeFile(fileName,
                                                 VolumeFile::VOLUME_TYPE_VECTOR,
                                                 metadataCommentTextEdit->toPlainText(), 
                                                 volumes,
                                                 voxelDataType,
                                                 volumeVectorWriteCompressedCheckBox->isChecked());
                                              
                  GuiFilesModified fm;
                  fm.setVolumeModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
            }
            else {
               VolumeFile* vf = volumeVectorFileSelectionComboBox->getSelectedVolumeFile();
               if (vf != NULL) {
                  vf->setDescriptiveLabel(volumeVectorLabelLineEdit->text());
                  updateMetadataAndName(vf,
                                        fileName,
                                        "");
                  brainSet->writeVolumeFile(fileName, 
                                            VolumeFile::FILE_READ_WRITE_TYPE_UNKNOWN, 
                                            VolumeFile::VOLUME_TYPE_VECTOR, 
                                            vf,
                                            voxelDataType,
                                            volumeVectorWriteCompressedCheckBox->isChecked());
                  GuiFilesModified fm;
                  fm.setVolumeModified();
                  theMainWindow->fileModificationUpdate(fm);
               }
            }
         }
         else if (filterName == FileFilters::getVtkModelFileFilter()) {
            const int indx = vtkModelSelectionComboBox->currentIndex();
            if ((indx >= 0) && (indx < brainSet->getNumberOfVtkModelFiles())) {
               VtkModelFile* vmf = brainSet->getVtkModelFile(indx);
               updateMetadataAndName(vmf,
                                     fileName,
                                     SpecFile::getVtkModelFileExtension());
               brainSet->writeVtkModelFile(fileName, vmf);
            }
         }
         else if (filterName == FileFilters::getWustlRegionFileFilter()) {
            WustlRegionFile* wrf = brainSet->getWustlRegionFile();
            updateMetadataAndName(wrf,
                                  fileName,
                                  SpecFile::getWustlRegionFileExtension());
            brainSet->writeWustlRegionFile(fileName);
         }
         else if (filterName == FileFilters::getAnalyzeVolumeFileFilter()) {
            VolumeFile* vf = exportVolumeSelectionControl->getSelectedVolumeFile();
            if (vf == NULL) {
               QApplication::restoreOverrideCursor();
               throw FileException("No Volume File is selected.");
            }
            updateFileNamesFileExtension(fileName,
                                         SpecFile::getAnalyzeVolumeFileExtension());
            brainSet->exportAnalyzeVolumeFile(vf, fileName);
         }
         else if (filterName == FileFilters::getByuSurfaceFileFilter()) {
            BrainModelSurface* bms = 
               exportSurfaceSelectionControl->getSelectedBrainModelSurface();
            if (bms == NULL) {
               QApplication::restoreOverrideCursor();
               throw FileException("No Surface is selected.");
            }
            updateFileNamesFileExtension(fileName,
                                         ".byu");
            brainSet->exportByuSurfaceFile(bms, fileName); 
         }
         else if (filterName == FileFilters::getFreeSurferAsciiSurfaceFileFilter()) {
            BrainModelSurface* bms = 
               exportSurfaceSelectionControl->getSelectedBrainModelSurface();
            if (bms == NULL) {
               QApplication::restoreOverrideCursor();
               throw FileException("No Surface is selected.");
            }
            updateFileNamesFileExtension(fileName,
                                         ".asc");
            brainSet->exportFreeSurferAsciiSurfaceFile(bms, fileName);
         }
         else if (filterName == FileFilters::getFreeSurferAsciiCurvatureFileFilter()) {
            BrainModelSurface* bms = 
               exportShapeSurfaceSelectionControl->getSelectedBrainModelSurface();
            if (bms == NULL) {
               throw FileException("No Surface is selected.");
            }
            updateFileNamesFileExtension(fileName,
                                         SpecFile::getFreeSurferAsciiCurvatureFileExtension());
            SurfaceShapeFile* ssf = brainSet->getSurfaceShapeFile();
            const int column = exportShapeColumnSelectionControl->currentIndex();
            if ((column >= 0) && (column < ssf->getNumberOfColumns())) {
              ssf->exportFreeSurferAsciiCurvatureFile(column,
                                                      bms->getCoordinateFile(),
                                                      fileName);
            }
            else {
               throw FileException("No Shape column is selected.");
            }
         }
         else if (filterName == FileFilters::getFreeSurferAsciiFunctionalFileFilter()) {
            BrainModelSurface* bms = 
               exportMetricSurfaceSelectionControl->getSelectedBrainModelSurface();
            if (bms == NULL) {
               throw FileException("No Surface is selected.");
            }
            updateFileNamesFileExtension(fileName,
                                         SpecFile::getFreeSurferAsciiFunctionalFileExtension());
            MetricFile* mf = brainSet->getMetricFile();
            const int column = exportMetricColumnSelectionControl->currentIndex();
            if ((column >= 0) && (column < mf->getNumberOfColumns())) {
              mf->exportFreeSurferAsciiFunctionalFile(column,
                                                      fileName);
            }
            else {
               throw FileException("No Metric column is selected.");
            }
         }
         else if (filterName == FileFilters::getFreeSurferAsciiLabelFileFilter()) {
            BrainModelSurface* bms = 
               exportPaintSurfaceSelectionControl->getSelectedBrainModelSurface();
            if (bms == NULL) {
               throw FileException("No Surface is selected.");
            }
            QFileInfo fi(fileName);
            const QString prefixName(fi.absolutePath());
            PaintFile* pf = brainSet->getPaintFile();
            const int column = exportPaintColumnSelectionControl->currentIndex();
            if ((column >= 0) && (column < pf->getNumberOfColumns())) {
              pf->exportFreeSurferAsciiLabelFile(column, 
                                                 prefixName, 
                                                 bms->getCoordinateFile());
            }
            else {
               throw FileException("No Paint column is selected.");
            }
         }
         else if (filterName == FileFilters::getMincVolumeFileFilter()) {
            VolumeFile* vf = exportVolumeSelectionControl->getSelectedVolumeFile();
            if (vf == NULL) {
               throw FileException("No Volume File is selected.");
            }
            updateFileNamesFileExtension(fileName,
                                         SpecFile::getMincVolumeFileExtension());
            brainSet->exportMincVolumeFile(vf, fileName);
         }
         else if (filterName == FileFilters::getOpenInventorSurfaceFileFilter()) {
            BrainModelSurface* bms = 
               exportSurfaceSelectionControl->getSelectedBrainModelSurface();
            if (bms == NULL) {
               throw FileException("No Surface is selected.");
            }
            updateFileNamesFileExtension(fileName,
                                         ".iv");
            brainSet->exportInventorSurfaceFile(bms, fileName);
         }
         else if (filterName == FileFilters::getStlSurfaceFileFilter()) {
            BrainModelSurface* bms = 
               exportSurfaceSelectionControl->getSelectedBrainModelSurface();
            if (bms == NULL) {
               throw FileException("No Surface is selected.");
            }
            updateFileNamesFileExtension(fileName,
                                         ".stl");
            brainSet->exportStlSurfaceFile(bms, fileName);
         }
         else if (filterName == FileFilters::getVrmlSurfaceFileFilter()) {
            BrainModelSurface* bms = 
               exportSurfaceSelectionControl->getSelectedBrainModelSurface();
            if (bms == NULL) {
               throw FileException("No Surface is selected.");
            }
            updateFileNamesFileExtension(fileName,
                                         ".wrl");
            brainSet->exportVrmlSurfaceFile(bms, fileName);
         }
         else if (filterName == FileFilters::getVtkSurfaceFileFilter()) {
            BrainModelSurface* bms = 
               exportSurfaceSelectionControl->getSelectedBrainModelSurface();
            if (bms == NULL) {
               throw FileException("No Surface is selected.");
            }
            updateFileNamesFileExtension(fileName,
                                         ".vtk");
            brainSet->exportVtkSurfaceFile(bms, fileName, true);
         }
         else if (filterName == FileFilters::getVtkXmlSurfaceFileFilter()) {
            BrainModelSurface* bms = 
               exportSurfaceSelectionControl->getSelectedBrainModelSurface();
            if (bms == NULL) {
               throw FileException("No Surface is selected.");
            }
            updateFileNamesFileExtension(fileName,
                                         ".vtkp");
            brainSet->exportVtkXmlSurfaceFile(bms, fileName, true);
         }
         else if (filterName == FileFilters::getVtkVolumeFileFilter()) {
            VolumeFile* vf = exportVolumeSelectionControl->getSelectedVolumeFile();
            if (vf == NULL) {
               throw FileException("No Volume File is selected.");
            }
            updateFileNamesFileExtension(fileName,
                                         ".vtk");
            brainSet->exportVtkStructuredPointsVolumeFile(vf, fileName);
         }
         else {
            const QString msg(QString("PROGRAM ERROR: unhandled file type ") 
                              + filterName.toAscii().constData() 
                              + QString(" at ") + QString(__LINE__)
                              + QString(" in ") + QString(__FILE__));
            throw FileException(msg);
         }

         PreferencesFile* pf = theMainWindow->getBrainSet()->getPreferencesFile();
         pf->addToRecentDataFileDirectories(FileUtilities::dirname(fileName), true);

         QApplication::restoreOverrideCursor();
      }
      catch (FileException& e) {
         QApplication::restoreOverrideCursor();
         QApplication::beep();
         QMessageBox::critical(this,
                               "SAVE ERROR",
                               e.whatQString(),
                               QMessageBox::Ok);
         return;
      }
   }
   
   WuQDialog::done(r);
}

