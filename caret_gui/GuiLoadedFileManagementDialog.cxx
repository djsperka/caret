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

#include <iostream>
#include <sstream>

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QToolTip>

#include "AreaColorFile.h"
#include "ArealEstimationFile.h"
#include "BorderColorFile.h"
#include "BorderFile.h"
#include "BorderProjectionFile.h"
#include "BrainModelBorderSet.h"
#include "BrainModelContours.h"
#include "BrainModelSurface.h"
#include "BrainSet.h"
#include "CellColorFile.h"
#include "CellFile.h"
#include "CellProjectionFile.h"
#include "CocomacConnectivityFile.h"
#include "CommaSeparatedValueFile.h"
#include "ContourCellFile.h"
#include "ContourCellColorFile.h"
#include "CutsFile.h"
#include "DeformationFieldFile.h"
#include "DeformationMapFile.h"
#include "DisplaySettingsArealEstimation.h"
#include "DisplaySettingsDeformationField.h"
#include "DisplaySettingsGeodesicDistance.h"
#include "DisplaySettingsMetric.h"
#include "DisplaySettingsPaint.h"
#include "DisplaySettingsRgbPaint.h"
#include "DisplaySettingsSection.h"
#include "DisplaySettingsSurfaceShape.h"
#include "DisplaySettingsVectors.h"
#include "DisplaySettingsTopography.h"
#include "FileFilters.h"
#include "FileUtilities.h"
#include "FociColorFile.h"
#include "FociFile.h"
#include "FociProjectionFile.h"
#include "FociSearchFile.h"
#include "GeodesicDistanceFile.h"
#include "GuiBrainModelOpenGL.h"
#include "GuiCommaSeparatedValueFileEditor.h"
#include "GuiDataFileCommentDialog.h"
#include "GuiDataFileSaveDialog.h"
#include "GuiFilesModified.h"
#include "GuiLoadedFileManagementDialog.h"
#include "ImageFile.h"
#include "LatLonFile.h"
#include "MetricFile.h"
#include "PaintFile.h"
#include "PaletteFile.h"
#include "ParamsFile.h"
#include "ProbabilisticAtlasFile.h"
#include "RgbPaintFile.h"
#include "SceneFile.h"
#include "SectionFile.h"
#include "StudyCollectionFile.h"
#include "StudyMetaDataFile.h"
#include "SurfaceShapeFile.h"
#include "VectorFile.h"
#include "TopographyFile.h"
#include "TopologyFile.h"
#include "VocabularyFile.h"
#include "VolumeFile.h"
#include "VtkModelFile.h"
#include "WuQDataEntryDialog.h"
#include "WustlRegionFile.h"
#include "global_variables.h"

// text for modified file label
static const QString modifiedFileLabelText = "***";
      
/**
 * Constructor
 */
GuiLoadedFileManagementDialog::GuiLoadedFileManagementDialog(QWidget* parent)
   : WuQDialog(parent)
{
   setAttribute(Qt::WA_DeleteOnClose);

   QString captionStr("Manage Loaded Files");
   const QString specFileName = theMainWindow->getBrainSet()->getSpecFileName();
   if (specFileName.isEmpty() == false) {
      captionStr.append(": ");
      captionStr.append(FileUtilities::basename(specFileName));
   }
   setWindowTitle(captionStr);
   
   const SpecFile* sf = theMainWindow->getBrainSet()->getLoadedFilesSpecFile();
   
   GuiFilesModified fm;
   
   //
   // Text for various file widgets
   //
   removeFilePushButtonText = "XF";
   removeFileColumnPushButtonText = "XC";
   clearFileColumnPushButtonText = "CC";
   viewFilePushButtonText = "V";
   commentHeaderPushButtonText = "Edit";
   saveFilePushButtonText = "Save";
    
   //
   // Layout indices
   //
   int numCols = 0;
   columnFileTypeIndex = numCols++;
   columnModifiedIndex = numCols++;
   columnSaveFileIndex = numCols++;
   columnViewFileIndex = numCols++;
   columnCommentIndex = numCols++;
   columnRemoveFileIndex = numCols++;
   columnRemoveFileColumnIndex = numCols++;
   columnClearFileColumnIndex = numCols++;
   columnFileNameIndex = numCols++;

   //
   // Vertical box for all items in dialog
   //
   QVBoxLayout* dialogLayout = new QVBoxLayout(this);
   dialogLayout->setMargin(5);
   dialogLayout->setSpacing(2);
   
   //
   // widget for files widget since there could be many files
   //
   fileGridWidget = new QWidget;
   fileGridLayout = new QGridLayout(fileGridWidget);
   fileGridLayout->setHorizontalSpacing(10);
   fileGridLayout->setVerticalSpacing(2);
   fileGridLayout->setColumnStretch(columnFileNameIndex, 1000);
   
   // QVBox* vbox = new QVBox(sv, "vbox");  // prevents background from showing through
   //fileGrid = new QGrid(2, Qt::Horizontal, vbox, "fileGrid");
   //sv->addChild(vbox);
   
   //
   // Button group for data files view push button
   //
   viewButtonGroup = new QButtonGroup(this);
   QObject::connect(viewButtonGroup, SIGNAL(buttonClicked(int)),
                    this, SLOT(slotViewButtonGroup(int)));
   
   //
   // Button group for data files save push button
   //
   saveButtonGroup = new QButtonGroup(this);
   QObject::connect(saveButtonGroup, SIGNAL(buttonClicked(int)),
                    this, SLOT(slotSaveButtonGroup(int)));
   
   //
   // Button group for data files Comment Header push button
   //
   infoButtonGroup = new QButtonGroup(this);
   QObject::connect(infoButtonGroup, SIGNAL(buttonClicked(int)),
                    this, SLOT(slotInfoButtonGroup(int)));
   
   //
   // Button group for border files comment/header edit push button
   //
   borderInfoButtonGroup = new QButtonGroup(this);
   QObject::connect(borderInfoButtonGroup, SIGNAL(buttonClicked(int)),
                    this, SLOT(slotBorderInfoButtonGroup(int)));
   
   //
   // Remove File  button group for data files "XF" push button
   //
   removeFileButtonGroup = new QButtonGroup(this);
   QObject::connect(removeFileButtonGroup, SIGNAL(buttonClicked(int)),
                    this, SLOT(slotRemoveFileButtonGroup(int)));
   
   //
   // Remove File Column button group for data files "XC" push button
   //
   removeFileColumnButtonGroup = new QButtonGroup(this);
   QObject::connect(removeFileColumnButtonGroup, SIGNAL(buttonClicked(int)),
                    this, SLOT(slotRemoveFileColumnButtonGroup(int)));
   
   //
   // Clear File Column button group for data files "CC" push button
   //
   clearFileColumnButtonGroup = new QButtonGroup(this);
   QObject::connect(clearFileColumnButtonGroup, SIGNAL(buttonClicked(int)),
                    this, SLOT(slotClearFileColumnButtonGroup(int)));
   
   //
   // column labels
   //
   fileGridLayout->addWidget(new QLabel("File Type"), 1, columnFileTypeIndex);
   fileGridLayout->addWidget(new QLabel(" Mod "), 0, columnModifiedIndex, Qt::AlignHCenter);
   fileGridLayout->addWidget(new QLabel("Save"), 0, columnSaveFileIndex, Qt::AlignHCenter);
   fileGridLayout->addWidget(new QLabel("File"), 1, columnSaveFileIndex, Qt::AlignHCenter);
   fileGridLayout->addWidget(new QLabel("View"), 0, columnViewFileIndex, Qt::AlignHCenter);
   fileGridLayout->addWidget(new QLabel("Comment"), 0, columnCommentIndex, Qt::AlignHCenter);
   fileGridLayout->addWidget(new QLabel("Header"), 1, columnCommentIndex, Qt::AlignHCenter);
   fileGridLayout->addWidget(new QLabel("Remove"), 0, columnRemoveFileIndex, Qt::AlignHCenter);
   fileGridLayout->addWidget(new QLabel("File"), 1, columnRemoveFileIndex, Qt::AlignHCenter);
   fileGridLayout->addWidget(new QLabel("Remove"), 0, columnRemoveFileColumnIndex, Qt::AlignHCenter);
   fileGridLayout->addWidget(new QLabel("Column"), 1, columnRemoveFileColumnIndex, Qt::AlignHCenter);
   fileGridLayout->addWidget(new QLabel("Clear"), 0, columnClearFileColumnIndex, Qt::AlignHCenter);
   fileGridLayout->addWidget(new QLabel("Column"), 1, columnClearFileColumnIndex, Qt::AlignHCenter);
   fileGridLayout->addWidget(new QLabel("File Name"), 1, columnFileNameIndex, Qt::AlignLeft);
   
   fm.setStatusForAll(false);
   fm.setAreaColorModified();
   addFileToGrid(theMainWindow->getBrainSet()->getAreaColorFile(), 
                 fm,
                 sf->areaColorFile,
                 "Area Color File",
                 SpecFile::getAreaColorFileTag());

   fm.setStatusForAll(false);
   fm.setArealEstimationModified();
   addFileToGrid(theMainWindow->getBrainSet()->getArealEstimationFile(), 
                 fm,
                 sf->arealEstimationFile,
                 "Areal Estimation File",
                 SpecFile::getArealEstimationFileTag());

   BrainModelBorderSet* bmbs = theMainWindow->getBrainSet()->getBorderSet();
   addBorderFileToGrid(bmbs, BrainModelSurface::SURFACE_TYPE_RAW,
                                               "Border - Raw",
                                                SpecFile::getRawBorderFileTag());
   addBorderFileToGrid(bmbs, BrainModelSurface::SURFACE_TYPE_FIDUCIAL,
                                               "Border - Fiducial",
                                                SpecFile::getFiducialBorderFileTag());
   addBorderFileToGrid(bmbs, BrainModelSurface::SURFACE_TYPE_INFLATED,
                                               "Border - Inflated",
                                                SpecFile::getInflatedBorderFileTag());
   addBorderFileToGrid(bmbs, BrainModelSurface::SURFACE_TYPE_VERY_INFLATED,
                                               "Border - Very Inflated",
                                                SpecFile::getVeryInflatedBorderFileTag());
   addBorderFileToGrid(bmbs, BrainModelSurface::SURFACE_TYPE_SPHERICAL,
                                               "Border - Spherical",
                                                SpecFile::getSphericalBorderFileTag());
   addBorderFileToGrid(bmbs, BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL,
                                               "Border - Ellipsoid",
                                                SpecFile::getEllipsoidBorderFileTag());
   addBorderFileToGrid(bmbs, BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL,
                                               "Border - Comp Med Wall",
                                                SpecFile::getCompressedBorderFileTag());
   addBorderFileToGrid(bmbs, BrainModelSurface::SURFACE_TYPE_FLAT,
                                               "Border - Flat",
                                                SpecFile::getFlatBorderFileTag());
   addBorderFileToGrid(bmbs, BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR,
                                               "Border - Flat Lobar",
                                                SpecFile::getLobarFlatBorderFileTag());
   addBorderFileToGrid(bmbs, BrainModelSurface::SURFACE_TYPE_UNKNOWN,
                                               "Border - Unknown",
                                                SpecFile::getUnknownBorderFileMatchTag());
                                               
   
   fm.setStatusForAll(false);
   fm.setBorderColorModified();
   addFileToGrid(theMainWindow->getBrainSet()->getBorderColorFile(),
                 fm,
                  "Border Color File",
                 SpecFile::getBorderColorFileTag());

   addBorderProjectionFileToGrid(bmbs);
                       
   fm.setStatusForAll(false);
   fm.setCellColorModified();
   addFileToGrid(theMainWindow->getBrainSet()->getCellColorFile(),
                 fm,
                 sf->cellColorFile,
                  "Cell Color File",
                 SpecFile::getCellColorFileTag());
                  
   fm.setStatusForAll(false);
   fm.setCellProjectionModified();
   addFileToGrid(theMainWindow->getBrainSet()->getCellProjectionFile(), fm, 
                 sf->cellProjectionFile,
                 "",
                 SpecFile::getCellProjectionFileTag());

   fm.setStatusForAll(false);
   fm.setCellModified();
   addFileToGrid(theMainWindow->getBrainSet()->getVolumeCellFile(), fm, 
                 sf->volumeCellFile,
                 "Cell File (Volume)",
                 SpecFile::getVolumeCellFileTag());

   fm.setStatusForAll(false);
   fm.setCocomacModified();
   addFileToGrid(theMainWindow->getBrainSet()->getCocomacFile(), fm,
                 sf->cocomacConnectivityFile,
                 "",
                 SpecFile::getCocomacConnectivityFileTag());

   fm.setStatusForAll(false);
   fm.setContourCellModified();
   addFileToGrid(theMainWindow->getBrainSet()->getContourCellFile(), 
                 fm,
                 sf->contourCellFile,
                 "Contour Cells File",
                 SpecFile::getContourCellFileTag());

   fm.setStatusForAll(false);
   fm.setCellColorModified();
   addFileToGrid(theMainWindow->getBrainSet()->getContourCellColorFile(),
                 fm,
                 sf->contourCellColorFile,
                 "Contour Cell Color File",
                 SpecFile::getContourCellColorFileTag());
                 
   for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfBrainModels(); i++) {
      BrainModel* bm = theMainWindow->getBrainSet()->getBrainModel(i);
      switch(bm->getModelType()) {
         case BrainModel::BRAIN_MODEL_CONTOURS:
            {
               BrainModelContours* bmc = dynamic_cast<BrainModelContours*>(bm);
               ContourFile* cf = bmc->getContourFile();
               fm.setStatusForAll(false);
               fm.setContourModified();
               addFileToGrid(cf, fm, "",
                 SpecFile::getContourFileTag()); 
            }
            break;
         case BrainModel::BRAIN_MODEL_SURFACE:
            {
               BrainModelSurface* bms = dynamic_cast<BrainModelSurface*>(bm);
               CoordinateFile* cf = bms->getCoordinateFile();
               fm.setStatusForAll(false);
               fm.setCoordinateModified();
               addFileToGrid(cf, fm, "",
                             BrainModelSurface::getCoordSpecFileTagFromSurfaceType(
                                bms->getSurfaceType()));
            }
            break;
         case BrainModel::BRAIN_MODEL_VOLUME:
            break;
         case BrainModel::BRAIN_MODEL_SURFACE_AND_VOLUME:
            break;
      }
   }   
   
   fm.setStatusForAll(false);
   fm.setCutModified();
   addFileToGrid(theMainWindow->getBrainSet()->getCutsFile(), 
                 fm,
                 sf->cutsFile,
                 "Cuts File",
                 SpecFile::getCutsFileTag());
   
   fm.setStatusForAll(false);
   fm.setDeformationFieldModified();
   addFileToGrid(theMainWindow->getBrainSet()->getDeformationFieldFile(),
                 fm,
                 sf->deformationFieldFile,
                 "Deformation Field File",
                 SpecFile::getDeformationFieldFileTag());
   
   fm.setStatusForAll(false);
   fm.setFociColorModified();
   addFileToGrid(theMainWindow->getBrainSet()->getFociColorFile(),
                 fm,
                 sf->fociColorFile,
                 "Foci Color File",
                 SpecFile::getFociColorFileTag());
   
   fm.setStatusForAll(false);
   fm.setFociProjectionModified();
   addFileToGrid(theMainWindow->getBrainSet()->getFociProjectionFile(),
                 fm,
                 sf->fociProjectionFile,
                 "Foci Projection File",
                 SpecFile::getFociProjectionFileTag());
   
   fm.setStatusForAll(false);
   fm.setFociSearchModified();
   addFileToGrid(theMainWindow->getBrainSet()->getFociSearchFile(),
                 fm,
                 sf->fociSearchFile,
                 "Foci Search File",
                 SpecFile::getFociSearchFileTag());
   
   fm.setStatusForAll(false);
   fm.setGeodesicModified();
   addFileToGrid(theMainWindow->getBrainSet()->getGeodesicDistanceFile(), fm,
                 sf->geodesicDistanceFile,
                 "",
                 SpecFile::getGeodesicDistanceFileTag());
   
   for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfImageFiles(); i++) {
      fm.setStatusForAll(false);
      fm.setImagesModified();
      addFileToGrid(theMainWindow->getBrainSet()->getImageFile(i), fm, "",
                    SpecFile::getImageFileTag());
   }
   
   fm.setStatusForAll(false);
   fm.setLatLonModified();
   addFileToGrid(theMainWindow->getBrainSet()->getLatLonFile(), fm,
                 sf->latLonFile, "",
                 SpecFile::getLatLonFileTag());
   
   fm.setStatusForAll(false);
   fm.setMetricModified();
   addFileToGrid(theMainWindow->getBrainSet()->getMetricFile(), fm, sf->metricFile, "",
                 SpecFile::getMetricFileTag());
   
   fm.setStatusForAll(false);
   fm.setPaintModified();
   addFileToGrid(theMainWindow->getBrainSet()->getPaintFile(), fm, sf->paintFile, "",
                 SpecFile::getPaintFileTag());
   
   fm.setStatusForAll(false);
   fm.setParameterModified();
   addFileToGrid(theMainWindow->getBrainSet()->getParamsFile(), fm, sf->paramsFile, "",
                 SpecFile::getParamsFileTag());
   
   fm.setStatusForAll(false);
   fm.setProbabilisticAtlasModified();
   addFileToGrid(theMainWindow->getBrainSet()->getProbabilisticAtlasSurfaceFile(), 
                 fm,
                 sf->atlasFile,
                 "Probabilistic Atlas File",
                 SpecFile::getAtlasFileTag());
   
   fm.setStatusForAll(false);
   fm.setPaletteModified();
   addFileToGrid(theMainWindow->getBrainSet()->getPaletteFile(), fm, sf->paletteFile, "",
                 SpecFile::getPaletteFileTag());
   
   fm.setStatusForAll(false);
   fm.setRgbPaintModified();
   addFileToGrid(theMainWindow->getBrainSet()->getRgbPaintFile(), fm, sf->rgbPaintFile, "",
                 SpecFile::getRgbPaintFileTag());
   
   fm.setStatusForAll(false);
   fm.setSectionModified();
   addFileToGrid(theMainWindow->getBrainSet()->getSectionFile(), fm, sf->sectionFile, "",
                 SpecFile::getSectionFileTag());
   
   fm.setStatusForAll(false);
   fm.setSceneModified();
   addFileToGrid(theMainWindow->getBrainSet()->getSceneFile(), fm, sf->sceneFile, "",
                 SpecFile::getSceneFileTag());
   
   fm.setStatusForAll(false);
   fm.setStudyCollectionModified();
   addFileToGrid(theMainWindow->getBrainSet()->getStudyCollectionFile(), fm, sf->studyCollectionFile, "",
                 SpecFile::getStudyCollectionFileTag());
   
   fm.setStatusForAll(false);
   fm.setStudyMetaDataModified();
   addFileToGrid(theMainWindow->getBrainSet()->getStudyMetaDataFile(), fm, sf->studyMetaDataFile, "",
                 SpecFile::getStudyMetaDataFileTag());
   
   fm.setStatusForAll(false);
   fm.setSurfaceShapeModified();
   addFileToGrid(theMainWindow->getBrainSet()->getSurfaceShapeFile(), 
                 fm,
                 sf->surfaceShapeFile,
                 "Surface Shape File",
                 SpecFile::getSurfaceShapeFileTag());
   
   for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfVectorFiles(); i++) {
      fm.setStatusForAll(false);
      fm.setVectorModified();
      addFileToGrid(theMainWindow->getBrainSet()->getVectorFile(i),
                    fm,
                    sf->vectorFile,
                    "Vector File",
                    SpecFile::getVectorFileTag());
   }
   
   fm.setStatusForAll(false);
   fm.setTopographyModified();
   addFileToGrid(theMainWindow->getBrainSet()->getTopographyFile(), fm, "",
                 SpecFile::getTopographyFileTag());

   for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfTopologyFiles(); i++) {
      TopologyFile* tf = theMainWindow->getBrainSet()->getTopologyFile(i);   
      fm.setStatusForAll(false);
      fm.setTopologyModified();
      addFileToGrid(tf, fm, "",
                    TopologyFile::getSpecFileTagFromTopologyType(
                       tf->getTopologyType()));
   }   
   
   for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfTransformationDataFiles(); i++) {
      AbstractFile* tdf = theMainWindow->getBrainSet()->getTransformationDataFile(i);
      fm.setStatusForAll(false);
      fm.setTransformationDataModified();
      addFileToGrid(tdf, fm, "Transformation Data File",
                 SpecFile::getTransformationDataFileTag());
   }
   
   fm.setStatusForAll(false);
   fm.setTransformationMatrixModified();
   addFileToGrid(theMainWindow->getBrainSet()->getTransformationMatrixFile(), 
                 fm,
                 sf->transformationMatrixFile,
                 "Transformation Matrix File",
                 SpecFile::getTransformationMatrixFileTag());
   
   fm.setStatusForAll(false);
   fm.setVocabularyModified();
   addFileToGrid(theMainWindow->getBrainSet()->getVocabularyFile(), 
                 fm,
                 sf->vocabularyFile,
                 "Vocabulary File",
                 SpecFile::getVocabularyFileTag());
   
   for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfVolumeAnatomyFiles(); i++) {   
      fm.setStatusForAll(false);
      fm.setVolumeModified();
      addFileToGrid(theMainWindow->getBrainSet()->getVolumeAnatomyFile(i), 
                 fm,
                 "Volume File - Anatomy",
                 SpecFile::getVolumeAnatomyFileTag());
   }
   for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfVolumeFunctionalFiles(); i++) {
      fm.setStatusForAll(false);
      fm.setVolumeModified();
      addFileToGrid(theMainWindow->getBrainSet()->getVolumeFunctionalFile(i), 
                 fm,
                 "Volume File - Functional",
                 SpecFile::getVolumeFunctionalFileTag());
   }
   for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfVolumePaintFiles(); i++) {   
      fm.setStatusForAll(false);
      fm.setVolumeModified();
      addFileToGrid(theMainWindow->getBrainSet()->getVolumePaintFile(i), 
                 fm,
                 "Volume File - Paint",
                 SpecFile::getVolumePaintFileTag());
   }
   for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfVolumeProbAtlasFiles(); i++) {   
      fm.setStatusForAll(false);
      fm.setVolumeModified();
      addFileToGrid(theMainWindow->getBrainSet()->getVolumeProbAtlasFile(i), 
                 fm,
                 "Volume File - Prob Atlas",
                 SpecFile::getVolumeProbAtlasFileTag());
   }
   for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfVolumeRgbFiles(); i++) {
      fm.setStatusForAll(false);
      fm.setVolumeModified();
      addFileToGrid(theMainWindow->getBrainSet()->getVolumeRgbFile(i), 
                 fm,
                 "Volume File - RGB",
                 SpecFile::getVolumeRgbFileTag());
   }
   for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfVolumeSegmentationFiles(); i++) {   
      fm.setStatusForAll(false);
      fm.setVolumeModified();
      addFileToGrid(theMainWindow->getBrainSet()->getVolumeSegmentationFile(i), 
                 fm,
                 "Volume File - Segmentation",
                 SpecFile::getVolumeSegmentationFileTag());
   }
   for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfVolumeVectorFiles(); i++) {   
      fm.setStatusForAll(false);
      fm.setVolumeModified();
      addFileToGrid(theMainWindow->getBrainSet()->getVolumeVectorFile(i), 
                 fm,
                 "Volume File - Vector",
                 SpecFile::getVolumeVectorFileTag());
   }

   for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfVtkModelFiles(); i++) {   
      fm.setStatusForAll(false);
      fm.setVtkModelModified();
      addFileToGrid(theMainWindow->getBrainSet()->getVtkModelFile(i), fm, "",
                 SpecFile::getVtkModelFileTag());
   }
   
   fm.setStatusForAll(false);
   fm.setWustlRegionModified();
   addFileToGrid(theMainWindow->getBrainSet()->getWustlRegionFile(), fm, sf->wustlRegionFile,   "",
                 SpecFile::getWustlRegionFileTag());
   
   //
   // Scrolled widget for files 
   //
   QWidget* scrollAreaWidget = new QWidget;
   QVBoxLayout* scrollAreaWidgetLayout = new QVBoxLayout(scrollAreaWidget);
   scrollAreaWidgetLayout->addWidget(fileGridWidget);
   scrollAreaWidgetLayout->addStretch();
   QScrollArea* sv = new QScrollArea(this);
   sv->setWidgetResizable(true);
   sv->setWidget(scrollAreaWidget);
   dialogLayout->addWidget(sv);
   
   //
   // Horizontal layout for buttons
   //
   QHBoxLayout* buttonsLayout = new QHBoxLayout;
   dialogLayout->addLayout(buttonsLayout);
   buttonsLayout->setSpacing(2);
   
   //
   // Save all checked files button
   //
   QPushButton* saveButton = new QPushButton("Save All Checked Files");
   saveButton->setAutoDefault(false);
   saveButton->setFixedSize(saveButton->sizeHint());
   buttonsLayout->addWidget(saveButton);
   QObject::connect(saveButton, SIGNAL(clicked()),
                    this, SLOT(slotSaveAllCheckFiles()));
                    
   //
   // Close button
   //
   QPushButton* closeButton = new QPushButton("Close");
   closeButton->setAutoDefault(false);
   closeButton->setFixedSize(closeButton->sizeHint());
   buttonsLayout->addWidget(closeButton);
   QObject::connect(closeButton, SIGNAL(clicked()),
                    this, SLOT(close()));
                    
   QSize minSize = this->minimumSize();
   this->setMinimumSize(std::max(minSize.width(), 800), minSize.height());
   //resize(std::max(width(), 800), stdheight());
}

/**
 * Destructor
 */
GuiLoadedFileManagementDialog::~GuiLoadedFileManagementDialog()
{
}

/**
 * Called when an info button is pressed.
 */
void
GuiLoadedFileManagementDialog::slotInfoButtonGroup(int item)
{
   AbstractFile* af = dataFiles[item].dataFile;
   if (af != NULL) {
      GuiDataFileCommentDialog* dfc = new GuiDataFileCommentDialog(this, af);
      dfc->show();
   }
}

/**
 * save all checked files.
 */
void 
GuiLoadedFileManagementDialog::slotSaveAllCheckFiles()
{
   const int num = static_cast<int>(dataFiles.size());
   for (int i = 0; i < num; i++) {
      dataFiles[i].saveFileIfChecked();
   }
}      

/**
 * called when a save button is pressed.
 */
void 
GuiLoadedFileManagementDialog::slotSaveButtonGroup(int item)
{
   AbstractFile* af = dataFiles[item].dataFile;
   if (af != NULL) {
      af->setFileName(dataFiles[item].nameLineEdit->text());

      GuiDataFileSaveDialog fd(this);
      fd.selectFile(af);
      if (fd.exec() == GuiDataFileSaveDialog::Accepted) {
         if (dataFiles[item].modifiedLabel != NULL) {
            if (af->getModified() == 0) {
               dataFiles[item].clearModified();
            }
            dataFiles[item].nameLineEdit->setText(fd.getSelectedFileName());
         }
      }
   }
}      

/**
 * Called when a view button is pressed.
 */
void
GuiLoadedFileManagementDialog::slotViewButtonGroup(int item)
{
   AbstractFile* af = dataFiles[item].dataFile;
   if (af != NULL) {
      //CommaSeparatedValueFile csv;
      //af->writeDataIntoCommaSeparatedValueFile(csv);
      //GuiCommaSeparatedValueFileEditor* csfv = new GuiCommaSeparatedValueFileEditor(this, &csv, af);
      GuiCommaSeparatedValueFileEditor* csfv = 
                new GuiCommaSeparatedValueFileEditor(this, af, dataFiles[item].dataFileTypeMask);
      csfv->show();
   }
}

/**
 * Called when a border info button is pressed.
 */
void
GuiLoadedFileManagementDialog::slotBorderInfoButtonGroup(int item)
{
   BrainModelBorderFileInfo* bfi = borderInfoFileInfo[item];
   if (bfi != NULL) {
      GuiDataFileCommentDialog* dfc = new GuiDataFileCommentDialog(this, bfi);
      dfc->show();
   }
}

/**
 * called when a remove file column button is pressed.
 */
void 
GuiLoadedFileManagementDialog::slotRemoveFileColumnButtonGroup(int item)
{
   AbstractFile* af = dataFiles[item].dataFile;
   
   if (af != NULL) {
      const GuiFilesModified fm = dataFiles[item].dataFileTypeMask;
      
      QStringList columnNames;
      
      GiftiNodeDataFile* gndf = dynamic_cast<GiftiNodeDataFile*>(af);
      if (gndf != NULL) {
         for (int i = 0; i < gndf->getNumberOfColumns(); i++) {
            columnNames += gndf->getColumnName(i);
         }
      }
      
      NodeAttributeFile* naf = dynamic_cast<NodeAttributeFile*>(af);
      if (naf != NULL) {
         for (int i = 0; i < naf->getNumberOfColumns(); i++) {
            columnNames += naf->getColumnName(i);
         }
      }
      
      const int numCols = static_cast<int>(columnNames.size());
      if (numCols <= 0) {
         QMessageBox::critical(this, "ERROR", "File contains no data.");
         return;
      }
      
      std::vector<QCheckBox*> dataColumnCheckBoxes;
      WuQDataEntryDialog ded(this, true);
      ded.setTextAtTop("Select columns that you want REMOVED from the file.", true);
      for (int i = 0; i < numCols; i++) {
         dataColumnCheckBoxes.push_back(ded.addCheckBox(columnNames[i], false));
      }
      if (ded.exec() == WuQDataEntryDialog::Accepted) {
         bool columnDeletedFlag = false;
         for (int i = (numCols - 1); i >= 0; i--) {
            if (dataColumnCheckBoxes[i]->isChecked()) {
               if (gndf != NULL) {
                  gndf->removeColumn(i);
                  columnDeletedFlag = true;
               }
               if (naf != NULL) {
                  naf->removeColumn(i);
                  columnDeletedFlag = true;
               }
            }
         }
         
         if (columnDeletedFlag) {
            BrainSet* bs = theMainWindow->getBrainSet();
            if (gndf != NULL) {
               if (dynamic_cast<SurfaceShapeFile*>(gndf) != NULL) {
                  bs->getDisplaySettingsSurfaceShape()->update();
               }
               else if (dynamic_cast<MetricFile*>(gndf) != NULL) {
                  bs->getDisplaySettingsMetric()->update();
               }
               else if (dynamic_cast<PaintFile*>(gndf) != NULL) {
                  bs->getDisplaySettingsPaint()->update();
               }
               else {
                  std::cout << "PROGRAM ERROR: "
                            << "Unrecognized GIFTI file type at "
                            << __LINE__
                            << " in "
                            << __FILE__
                            << std::endl;
               }
            }
            
            if (naf != NULL) {
               if (dynamic_cast<ArealEstimationFile*>(naf) != NULL) {
                  bs->getDisplaySettingsArealEstimation()->update();
               }
               else if (dynamic_cast<DeformationFieldFile*>(naf) != NULL) {
                  bs->getDisplaySettingsDeformationField()->update();
               }
               else if (dynamic_cast<GeodesicDistanceFile*>(naf) != NULL) {
                  bs->getDisplaySettingsGeodesicDistance()->update();
               }
               else if (dynamic_cast<LatLonFile*>(naf) != NULL) {
                  //bs->getDisplaySettingsPaint()->update();
               }
               else if (dynamic_cast<RgbPaintFile*>(naf) != NULL) {
                  bs->getDisplaySettingsRgbPaint()->update();
               }
               else if (dynamic_cast<SectionFile*>(naf) != NULL) {
                  bs->getDisplaySettingsSection()->update();
               }
               else if (dynamic_cast<VectorFile*>(naf) != NULL) {
                  bs->getDisplaySettingsVectors()->update();
               }
               else if (dynamic_cast<TopographyFile*>(naf) != NULL) {
                  bs->getDisplaySettingsTopography()->update();
               }
               else {
                  std::cout << "PROGRAM ERROR: "
                            << "Unrecognized Node Attribute file type at "
                            << __LINE__
                            << " in "
                            << __FILE__
                            << std::endl;
               }
            }
         }
      }
   }
   
   theMainWindow->fileModificationUpdate(dataFiles[item].dataFileTypeMask);
   GuiBrainModelOpenGL::updateAllGL();
}

/**
 * called when a clear file column button is pressed.
 */
void 
GuiLoadedFileManagementDialog::slotClearFileColumnButtonGroup(int item)
{
   AbstractFile* af = dataFiles[item].dataFile;
   
   if (af != NULL) {
      const GuiFilesModified fm = dataFiles[item].dataFileTypeMask;
      
      QStringList columnNames;
      
      GiftiNodeDataFile* gndf = dynamic_cast<GiftiNodeDataFile*>(af);
      if (gndf != NULL) {
         for (int i = 0; i < gndf->getNumberOfColumns(); i++) {
            columnNames += gndf->getColumnName(i);
         }
      }
      
      NodeAttributeFile* naf = dynamic_cast<NodeAttributeFile*>(af);
      if (naf != NULL) {
         for (int i = 0; i < naf->getNumberOfColumns(); i++) {
            columnNames += naf->getColumnName(i);
         }
      }
      
      const int numCols = static_cast<int>(columnNames.size());
      if (numCols <= 0) {
         QMessageBox::critical(this, "ERROR", "File contains no data.");
         return;
      }
      
      std::vector<QCheckBox*> dataColumnCheckBoxes;
      WuQDataEntryDialog ded(this, true);
      ded.setTextAtTop("Select columns for that you want cleared (zeroized) in the file.", true);
      for (int i = 0; i < numCols; i++) {
         dataColumnCheckBoxes.push_back(ded.addCheckBox(columnNames[i], false));
      }
      if (ded.exec() == WuQDataEntryDialog::Accepted) {
         for (int i = (numCols - 1); i >= 0; i--) {
            if (dataColumnCheckBoxes[i]->isChecked()) {
               if (gndf != NULL) {
                  gndf->resetColumn(i);
               }
               if (naf != NULL) {
                  naf->resetColumn(i);
               }
            }
         }
      }
   }
   
   theMainWindow->fileModificationUpdate(dataFiles[item].dataFileTypeMask);
   GuiBrainModelOpenGL::updateAllGL();
}      

/**
 * Called when a remove file button is pressed.
 */
void
GuiLoadedFileManagementDialog::slotRemoveFileButtonGroup(int item)
{
   AbstractFile* af = dataFiles[item].dataFile;
   
   if (af != NULL) {
      if (QMessageBox::question(this, 
                                "CONFIRM",
                                "Remove from memory: "
                                   + FileUtilities::basename(af->getFileName())
                                   + "?",
                                 (QMessageBox::Yes | QMessageBox::No),
                                 QMessageBox::Yes) != QMessageBox::Yes) {
         return;
      }
   
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   
      const GuiFilesModified fm = dataFiles[item].dataFileTypeMask;
      
      if (fm.getCoordinateModified()) {
         for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfBrainModels(); i++) {
            BrainModelSurface* bms = theMainWindow->getBrainSet()->getBrainModelSurface(i);
            if (bms != NULL) {
               if (bms->getCoordinateFile() == af) {
                  theMainWindow->getBrainSet()->deleteBrainModelSurface(bms);
                  break;
               }
            }
         }
      }
      else if (fm.getVolumeModified()) {
         theMainWindow->getBrainSet()->deleteVolumeFile(dynamic_cast<VolumeFile*>(af));
      }
      else if(fm.getContourModified()) {
         BrainModelContours* bmc = theMainWindow->getBrainSet()->getBrainModelContours();
         if (bmc != NULL) {
            theMainWindow->getBrainSet()->deleteBrainModel(bmc);
         }
      }
      else if (fm.getTopologyModified()) {
         theMainWindow->getBrainSet()->deleteTopologyFile(dynamic_cast<TopologyFile*>(af));
      }
      else if (fm.getTransformationDataModified()) {
         theMainWindow->getBrainSet()->deleteTransformationDataFile(af);
      }
      else if (fm.getVtkModelModified()) {
         theMainWindow->getBrainSet()->deleteVtkModelFile(dynamic_cast<VtkModelFile*>(af));
      }
      else if (fm.getImagesModified()) {
         theMainWindow->getBrainSet()->deleteImageFile(dynamic_cast<ImageFile*>(af));
      }
      else if (dynamic_cast<AreaColorFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearAreaColorFile();
      }
      else if (dynamic_cast<ArealEstimationFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearArealEstimationFile();
      }
      else if (dynamic_cast<BorderColorFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearBorderColorFile();
      }
      else if (dynamic_cast<CellFile*>(af) != NULL) {  // COULD BE FOCI or CELLS
         CellFile* cf = dynamic_cast<CellFile*>(af);
         if (cf == theMainWindow->getBrainSet()->getVolumeCellFile()) {
            theMainWindow->getBrainSet()->deleteAllCells(false, true);
         }
      }
      else if (dynamic_cast<CellProjectionFile*>(af) != NULL) {
         CellProjectionFile* cpf = dynamic_cast<CellProjectionFile*>(af);
         if (cpf == theMainWindow->getBrainSet()->getCellProjectionFile()) {
            theMainWindow->getBrainSet()->deleteAllCells(true, false);
         }
         else if (cpf == theMainWindow->getBrainSet()->getFociProjectionFile()) {
            theMainWindow->getBrainSet()->deleteAllFociProjections();
         }
      }
      else if (dynamic_cast<CellColorFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearCellColorFile();
      }
      else if (dynamic_cast<CocomacConnectivityFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearCocomacConnectivityFile();
      }
      else if (dynamic_cast<ContourCellColorFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearContourCellColorFile();
      }
      else if (dynamic_cast<ContourCellFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearContourCellFile();
      }
      else if (dynamic_cast<CutsFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearCutsFile();
      }
      else if (dynamic_cast<DeformationFieldFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearDeformationFieldFile();
      }
      else if (dynamic_cast<DeformationMapFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearDeformationMapFile();
      }
      else if (dynamic_cast<FociColorFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearFociColorFile();
      }
      else if (dynamic_cast<FociSearchFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearFociSearchFile();
      }
      else if (dynamic_cast<GeodesicDistanceFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearGeodesicDistanceFile();
      }
      else if (dynamic_cast<LatLonFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearLatLonFile();
      }    // shape must be before metric since shape derived from metric
      else if (dynamic_cast<SurfaceShapeFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearSurfaceShapeFile();
      }
      else if (dynamic_cast<VectorFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->removeVectorFile(dynamic_cast<VectorFile*>(af));
      }
      else if (dynamic_cast<MetricFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearMetricFile();
      }
      else if ((dynamic_cast<PaintFile*>(af) != NULL) && // not prob atlas derived from paint
               (dynamic_cast<ProbabilisticAtlasFile*>(af) == NULL)) {
         theMainWindow->getBrainSet()->clearPaintFile();
      }
      else if (dynamic_cast<PaletteFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearPaletteFile();
      }
      else if (dynamic_cast<ParamsFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearParamsFile();
      }
      else if (dynamic_cast<ProbabilisticAtlasFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearProbabilisticAtlasFile();
      }
      else if (dynamic_cast<RgbPaintFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearRgbPaintFile();
      }
      else if (dynamic_cast<SceneFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearSceneFile();
      }
      else if (dynamic_cast<SectionFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearSectionFile();
      }
      else if (dynamic_cast<StudyCollectionFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearStudyCollectionFile();
      }
      else if (dynamic_cast<StudyMetaDataFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearStudyMetaDataFile();
      }
      else if (dynamic_cast<TopographyFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearTopographyFile();
      }
      else if (dynamic_cast<TransformationMatrixFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearTransformationMatrixFile();
      }
      else if (dynamic_cast<VocabularyFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearVocabularyFile();
      }
      else if (dynamic_cast<WustlRegionFile*>(af) != NULL) {
         theMainWindow->getBrainSet()->clearWustlRegionFile();
      }
      else {
         std::ostringstream str;
         str << "PROGRAM ERROR: Unrecognized file type for deleting: "
             << af->getDescriptiveName().toAscii().constData();
         QApplication::restoreOverrideCursor();
         QMessageBox::critical(this, "PROGRAM ERROR", str.str().c_str());
            //af->clear();
      }
      dataFiles[item].dataFile = NULL;
      dataFiles[item].fileTypeCheckBoxOrLabel->setEnabled(false);
      dataFiles[item].nameLineEdit->setEnabled(false);
      
      //
      // Disable info & clear button
      //
      QAbstractButton* clearButton = removeFileButtonGroup->button(item);
      if (clearButton != NULL) {
         clearButton->setEnabled(false);
      }
      QAbstractButton* infoButton = infoButtonGroup->button(item);
      if (infoButton != NULL) {
         infoButton->setEnabled(false);
      }
      QAbstractButton* removeColumnButton = removeFileColumnButtonGroup->button(item);
      if (removeColumnButton != NULL) {
         removeColumnButton->setEnabled(false);
      }
      QAbstractButton* clearColumnButton = clearFileColumnButtonGroup->button(item);
      if (clearColumnButton != NULL) {
         clearColumnButton->setEnabled(false);
      }
      QAbstractButton* viewButton = viewButtonGroup->button(item);
      if (viewButton != NULL) {
         viewButton->setEnabled(false);
      }

      theMainWindow->fileModificationUpdate(dataFiles[item].dataFileTypeMask);
      theMainWindow->getBrainSet()->clearAllDisplayLists();
      GuiBrainModelOpenGL::updateAllGL();
      QApplication::restoreOverrideCursor();
   }   
}

/**
 * Add a border file to the grid.
 */
void
GuiLoadedFileManagementDialog::addBorderFileToGrid(BrainModelBorderSet* bmbs,
                                               const BrainModelSurface::SURFACE_TYPES surfaceType,
                                               const char* typeName,
                                               const QString& specFileTagIn)
{
   BrainModelBorderFileInfo* bmi = bmbs->getBorderFileInfo(surfaceType);
   if (bmi != NULL) {
      const QString fileName(bmi->getFileName());
      if (fileName.isEmpty() == false) {
         const BorderFile* bf = bmbs->copyBordersOfSpecifiedType(surfaceType);
         if (bf != NULL) {
            if (bf->getNumberOfBorders() > 0) {
               const int rowNum = fileGridLayout->rowCount(); // + 1;
               QLabel* typeNameLabel = new QLabel(typeName);
               fileGridLayout->addWidget(typeNameLabel, rowNum, columnFileTypeIndex);
               QLabel* modifyLabel = new QLabel(" ");
               modifyLabel->setStyleSheet("color: red; font: bold"); // show text in red, bold
               fileGridLayout->addWidget(modifyLabel, rowNum, columnModifiedIndex);
               
               QToolButton* infoPB = new QToolButton;
               infoPB->setText(commentHeaderPushButtonText);
               infoPB->setFixedSize(infoPB->sizeHint());
               borderInfoButtonGroup->addButton(infoPB, static_cast<int>(borderInfoFileInfo.size()));
               infoPB->setToolTip("Show file information.");
               fileGridLayout->addWidget(infoPB, rowNum, columnCommentIndex, Qt::AlignHCenter);
               borderInfoFileInfo.push_back(bmi);
               
               fileGridLayout->addWidget(new QLabel(" "), rowNum, columnRemoveFileIndex);    // X button
               QLineEdit* fileNameLineEdit = new QLineEdit;
               fileNameLineEdit->setText(fileName);
               fileGridLayout->addWidget(fileNameLineEdit, rowNum, columnFileNameIndex);
               
               GuiFilesModified noMask;
               dataFiles.push_back(DataFile(NULL,
                                            noMask,
                                            typeNameLabel,
                                            fileNameLineEdit,
                                            modifyLabel,
                                            specFileTagIn));
            }
            if (bf != NULL) {
               delete bf;
            }
         }
      }
   }
}

/**
 * Add a border projection file to the grid.
 */
void
GuiLoadedFileManagementDialog::addBorderProjectionFileToGrid(BrainModelBorderSet* bmbs)
{
   BrainModelBorderFileInfo* bmi = bmbs->getBorderProjectionFileInfo();
   if (bmi != NULL) {
      const QString fileName(bmi->getFileName());
      if (fileName.isEmpty() == false) {
         bmbs->copyBordersToBorderProjectionFile(borderProjectionFileForGrid);
         if (borderProjectionFileForGrid.getNumberOfBorderProjections() > 0) {
            const int rowNum = fileGridLayout->rowCount(); // + 1;
            QCheckBox* typeNameCheckBox = new QCheckBox("Border Projection File");
            fileGridLayout->addWidget(typeNameCheckBox, rowNum, columnFileTypeIndex);
            QLabel* modifyLabel = new QLabel(" ");
            modifyLabel->setStyleSheet("color: red; font: bold"); // show text in red, bold
            fileGridLayout->addWidget(modifyLabel, rowNum, columnModifiedIndex, Qt::AlignHCenter);
            if (bmbs->getProjectionsModified()) {
               modifyLabel->setText("***");
               borderProjectionFileForGrid.setModified();
            }
            else {
               borderProjectionFileForGrid.clearModified();
            }
            
            borderProjInfoPB = new QToolButton;
            borderProjInfoPB->setText(commentHeaderPushButtonText);
            borderProjInfoPB->setFixedSize(borderProjInfoPB->sizeHint());
            borderInfoButtonGroup->addButton(borderProjInfoPB, static_cast<int>(borderInfoFileInfo.size()));
            borderProjInfoPB->setToolTip("Show file information.");
            fileGridLayout->addWidget(borderProjInfoPB, rowNum, columnCommentIndex, Qt::AlignHCenter);
            borderInfoFileInfo.push_back(bmi);
            
            borderProjSavePB = new QToolButton;
            borderProjSavePB->setText(saveFilePushButtonText);
            borderProjSavePB->setFixedSize(borderProjSavePB->sizeHint());
            QObject::connect(borderProjSavePB, SIGNAL(clicked()),
                             this, SLOT(slotSaveBorderProjections()));
            fileGridLayout->addWidget(borderProjSavePB, rowNum, columnSaveFileIndex, Qt::AlignHCenter);
            
            borderProjClearPB = new QToolButton;
            borderProjClearPB->setText(removeFilePushButtonText);
            borderProjClearPB->setFixedSize(borderProjClearPB->sizeHint());
            QObject::connect(borderProjClearPB, SIGNAL(clicked()),
                             this, SLOT(slotDeleteBorderProjections()));
            borderProjClearPB->setToolTip("Delete this file from memory.\n"
                                   "The file is NOT deleted.");
            fileGridLayout->addWidget(borderProjClearPB, rowNum, columnRemoveFileIndex, Qt::AlignHCenter);

            QLineEdit* fileNameLineEdit = new QLineEdit;
            fileNameLineEdit->setText(fileName);
            fileGridLayout->addWidget(fileNameLineEdit, rowNum, columnFileNameIndex);

            borderProjectionDataFileIndex = static_cast<int>(dataFiles.size());
            GuiFilesModified borderProjModMask;
            borderProjModMask.setBorderModified();
            dataFiles.push_back(DataFile(&borderProjectionFileForGrid,
                                         borderProjModMask,
                                         typeNameCheckBox,
                                         fileNameLineEdit,
                                         modifyLabel,
                                         SpecFile::getBorderProjectionFileTag()));
         }
      }
   }
}

/**
 * called to save border projections.
 */
void 
GuiLoadedFileManagementDialog::slotSaveBorderProjections()
{
   BrainModelBorderSet* bmbs = theMainWindow->getBrainSet()->getBorderSet();
   BrainModelBorderFileInfo* bmi = bmbs->getBorderProjectionFileInfo();
   bmi->setFileName(dataFiles[borderProjectionDataFileIndex].nameLineEdit->text());
   bmbs->copyBordersToBorderProjectionFile(borderProjectionFileForGrid);

   GuiDataFileSaveDialog fd(this);
   fd.selectFileType(FileFilters::getBorderProjectionFileFilter());
   if (fd.exec() == GuiDataFileSaveDialog::Accepted) {
      if (dataFiles[borderProjectionDataFileIndex].modifiedLabel != NULL) {
         if (bmbs->getProjectionsModified() == false) {
            dataFiles[borderProjectionDataFileIndex].clearModified();
            bmi->setFileName(fd.getSelectedFileName());
         }
      }
   }
}

/**
 * called to delete border projections.
 */
void 
GuiLoadedFileManagementDialog::slotDeleteBorderProjections()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   
   BrainModelBorderSet* bmbs = theMainWindow->getBrainSet()->getBorderSet();
   bmbs->deleteBorderProjections();
   
   GuiFilesModified fm;
   fm.setBorderModified();
   theMainWindow->fileModificationUpdate(fm);
   GuiBrainModelOpenGL::updateAllGL();
   
   dataFiles[borderProjectionDataFileIndex].fileTypeCheckBoxOrLabel->setEnabled(false);
   dataFiles[borderProjectionDataFileIndex].nameLineEdit->setEnabled(false);
   borderProjInfoPB->setEnabled(false);
   borderProjClearPB->setEnabled(false);
   borderProjSavePB->setEnabled(false);
   
   QApplication::restoreOverrideCursor();
}
      
/**
 * Add a file to the grid.
 */
void
GuiLoadedFileManagementDialog::addFileToGrid(AbstractFile* af,
                                         const GuiFilesModified& fileMask,
                                         const char* typeName,
                                         const QString& specFileTagIn)
{
   SpecFile::Entry dummyEntry;
   addFileToGrid(af, fileMask, dummyEntry, typeName, specFileTagIn);
}

/**
 * add a file to the list of files.
 */
void 
GuiLoadedFileManagementDialog::addFileToGrid(AbstractFile* af,
                               const GuiFilesModified& fileMask,
                               const SpecFile::Entry& dataFileInfo,
                               const char* typeName,
                               const QString& specFileTagIn)
{
   if (af != NULL) {
      if (af->empty() == false) {
         QString label(af->getDescriptiveName());
         if (typeName != NULL) {
            if (strlen(typeName) > 0) {
               label = typeName;
            }
         }
         QString modifiedLabelText(" ");
         if (af->getModified()) {
            modifiedLabelText = modifiedFileLabelText;
         }
         const int rowNum = fileGridLayout->rowCount(); // + 1;
         
         QCheckBox* fileTypeCheckBox = new QCheckBox(label);
         fileGridLayout->addWidget(fileTypeCheckBox, rowNum, columnFileTypeIndex);
         
         QLabel* modifiedLabel = new QLabel;
         modifiedLabel->setStyleSheet("color: red; font: bold"); // show text in red, bold
         modifiedLabel->setText(modifiedLabelText);
         fileGridLayout->addWidget(modifiedLabel, rowNum, columnModifiedIndex, Qt::AlignHCenter);
         
         QToolButton* savePB = new QToolButton;
         savePB->setText(saveFilePushButtonText);
         saveButtonGroup->addButton(savePB, dataFiles.size());
         savePB->setToolTip("Save the file.");
         savePB->setFixedSize(savePB->sizeHint());
         fileGridLayout->addWidget(savePB, rowNum, columnSaveFileIndex, Qt::AlignHCenter);
         
         QToolButton* viewPB = new QToolButton;
         viewPB->setText(viewFilePushButtonText);
         //viewPB->setAutoDefault(false);
         viewButtonGroup->addButton(viewPB, dataFiles.size());
         viewPB->setToolTip("View data file in spreadsheet");
         fileGridLayout->addWidget(viewPB, rowNum, columnViewFileIndex, Qt::AlignHCenter);
         bool readCSV, writeCSV;
         af->getCommaSeparatedFileSupport(readCSV, writeCSV);
         if (writeCSV == false) {
            viewPB->hide();
         }
         viewPB->setFixedSize(viewPB->sizeHint());
         
         QToolButton* infoPB = new QToolButton;
         infoPB->setText(commentHeaderPushButtonText);
         //infoPB->setAutoDefault(false);
         infoButtonGroup->addButton(infoPB, dataFiles.size());
         infoPB->setToolTip("Show file information.");
         fileGridLayout->addWidget(infoPB, rowNum, columnCommentIndex, Qt::AlignHCenter);
         infoPB->setFixedSize(infoPB->sizeHint());
         
         QToolButton* removeFilePB = new QToolButton;
         removeFilePB->setText(removeFilePushButtonText);
         //clearPB->setAutoDefault(false);
         removeFileButtonGroup->addButton(removeFilePB, dataFiles.size());
         removeFilePB->setToolTip("Delete this file from memory.\n"
                                  "The file is NOT deleted.");
         fileGridLayout->addWidget(removeFilePB, rowNum, columnRemoveFileIndex, Qt::AlignHCenter);
         removeFilePB->setFixedSize(removeFilePB->sizeHint());
         
         //
         // Is this a node attribute file?
         //
         if ((dynamic_cast<NodeAttributeFile*>(af) != NULL) ||
             (dynamic_cast<GiftiNodeDataFile*>(af) != NULL)) {
            //
            // Remove column push button
            //
            QToolButton* removeColumnPushButton = new QToolButton;
            removeColumnPushButton->setText(removeFileColumnPushButtonText);
            removeColumnPushButton->setFixedSize(removeColumnPushButton->sizeHint());
            removeColumnPushButton->setToolTip("Remove data column(s)\n"
                                               "from this file.");
            fileGridLayout->addWidget(removeColumnPushButton, rowNum, columnRemoveFileColumnIndex, Qt::AlignHCenter);
            removeFileColumnButtonGroup->addButton(removeColumnPushButton, dataFiles.size());

            //
            // Clear column push button
            //
            QToolButton* clearColumnPushButton = new QToolButton;
            clearColumnPushButton->setText(clearFileColumnPushButtonText);
            clearColumnPushButton->setFixedSize(clearColumnPushButton->sizeHint());
            clearColumnPushButton->setToolTip("Clear (zeroize) data column(s)\n"
                                               "from this file.");
            fileGridLayout->addWidget(clearColumnPushButton, rowNum, columnClearFileColumnIndex, Qt::AlignHCenter);
            clearFileColumnButtonGroup->addButton(clearColumnPushButton, dataFiles.size());
         }
         
         const QString filename(FileUtilities::basename(af->getFileName()));
         QLineEdit* fileNameLineEdit = new QLineEdit;
         fileNameLineEdit->setText(filename);
         fileGridLayout->addWidget(fileNameLineEdit, rowNum, columnFileNameIndex);
         
         dataFiles.push_back(DataFile(af,
                                      fileMask,
                                      fileTypeCheckBox,
                                      fileNameLineEdit,
                                      modifiedLabel,
                                      specFileTagIn));

         if (dynamic_cast<VectorFile*>(af) == NULL) {
            for (int i = 0; i < dataFileInfo.getNumberOfFiles(); i++) {
               if (dataFileInfo.files[i].selected == SpecFile::SPEC_TRUE) {
                  const QString name(FileUtilities::basename(dataFileInfo.files[i].filename));
                  if (name != filename) {
                     const int rowNum = fileGridLayout->rowCount(); // + 1;
                     fileGridLayout->addWidget(new QLabel(name), rowNum, columnFileNameIndex);
                  }
               }
            }
         }
      }
   }
}                         

//
//=============================================================================
//=============================================================================
//

/**
 * constructor.
 */
GuiLoadedFileManagementDialog::DataFile::DataFile(AbstractFile* dataFileIn,
                                                  GuiFilesModified dataFileTypeMaskIn,
                                                  QWidget* fileTypeCheckBoxOrLabelIn,
                                                  QLineEdit* nameLineEditIn,
                                                  QLabel* modifiedLabelIn,
                                                  const QString& specFileTagIn) 
{
   dataFile = dataFileIn;
   fileTypeCheckBoxOrLabel = fileTypeCheckBoxOrLabelIn;
   nameLineEdit = nameLineEditIn;
   dataFileTypeMask = dataFileTypeMaskIn;
   modifiedLabel = modifiedLabelIn;
   specFileTag = specFileTagIn;
   
   if (dataFile != NULL) {
      if (dataFile->getModified()) {
         setModified();
      }
   }
}

/**
 * destructor.
 */
GuiLoadedFileManagementDialog::DataFile::~DataFile()
{
}
            
/**
 * set modified.
 */
void 
GuiLoadedFileManagementDialog::DataFile::setModified()
{
   modifiedLabel->setText(modifiedFileLabelText);
   QCheckBox* cb = dynamic_cast<QCheckBox*>(fileTypeCheckBoxOrLabel);
   if (cb != NULL) {
      cb->setChecked(true);
   }
}
            
/**
 * save the file if it is checked.
 */
QString 
GuiLoadedFileManagementDialog::DataFile::saveFileIfChecked()
{
   QCheckBox* cb = dynamic_cast<QCheckBox*>(fileTypeCheckBoxOrLabel);
   if (cb != NULL) {
      if (cb->isChecked()) {
         const QString name = nameLineEdit->text();
         if (name.isEmpty() == false) {
            // std::cout << "Saving: " << name.toAscii().constData() << std::endl;
            if (dataFile != NULL) {
               if (dynamic_cast<BorderProjectionFile*>(dataFile) != NULL) {
                  BrainModelBorderSet* bmbs = theMainWindow->getBrainSet()->getBorderSet();
                  BrainModelBorderFileInfo* bmi = bmbs->getBorderProjectionFileInfo();
                  bmi->setFileName(nameLineEdit->text());
                  try {
                     theMainWindow->getBrainSet()->writeBorderProjectionFile(
                                       name, "", "");
                  }
                  catch (FileException& e) {
                     return e.whatQString();
                  }
                  
                  if (bmbs->getProjectionsModified() == false) {
                     modifiedLabel->setText(" ");
                     cb->setChecked(false);
                  }
               }
               else {
                  dataFile->setFileName(name);
                  try {
                     dataFile->writeFile(name);
                  }
                  catch (FileException& e) {
                     return e.whatQString();
                  }
                  
                  if (dataFile->getModified() == false) {
                     modifiedLabel->setText(" ");
                     cb->setChecked(false);
                  }
               }
               
               QString name2;
               VolumeFile* vf = dynamic_cast<VolumeFile*>(dataFile);
               if (vf != NULL) {
                  name2 = vf->getDataFileName();
               }
               
               theMainWindow->getBrainSet()->addToSpecFile(
                  specFileTag,
                  name,
                  name2);
            }
         }
      }
   }
   
   return "";
}
            
/**
 * clear modified.
 */
void 
GuiLoadedFileManagementDialog::DataFile::clearModified() 
{
   modifiedLabel->setText(" ");
   QCheckBox* cb = dynamic_cast<QCheckBox*>(fileTypeCheckBoxOrLabel);
   if (cb != NULL) {
      cb->setChecked(false);
   }
}
