
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

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QGroupBox>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>

#include "AreaColorFile.h"
#include "ArealEstimationFile.h"
#include "BorderColorFile.h"
#include "BorderFile.h"
#include "BrainModelContours.h"
#include "BrainModelSurfaceNodeColoring.h"
#include "BrainSet.h"
#include "CellColorFile.h"
#include "ContourCellColorFile.h"
#include "ContourCellFile.h"
#include "CoordinateFile.h"
#include "DisplaySettingsBorders.h"
#include "FileFilters.h"
#include "FileUtilities.h"
#include "FociColorFile.h"
#include "FociFile.h"
#include "GeodesicDistanceFile.h"
#include "GuiBrainModelOpenGL.h"
#include "GuiDataFileCommentDialog.h"
#include "GuiDataFileImportOptionsDialog.h"
#include "GuiDataFileOpenDialog.h"
#include "GuiFilesModified.h"
#include "GuiLoadNodeAttributeFileColumnSelectionDialog.h"
#include "GuiMainWindow.h"
#include "GuiSpecFileCreationDialog.h"
#include "LatLonFile.h"
#include "MetricFile.h"
#include "PaintFile.h"
#include "QtRadioButtonSelectionDialog.h"
#include "RgbPaintFile.h"
#include "SpecFile.h"
#include "SurfaceFile.h"
#include "SurfaceShapeFile.h"
#include "VectorFile.h"
#include "TopologyFile.h"
#include "VolumeFile.h"
#include "global_variables.h"

static const QString GENERIC_BORDER("GENERIC_BORDER");
static const QString GENERIC_COORDINATE("GENERIC_COORDINATE");
static const QString GENERIC_SURFACE("GENERIC_SURFACE");
static const QString GENERIC_TOPOLOGY("GENERIC_TOPOLOGY");

/**
 * constructor.
 */
GuiDataFileOpenDialog::GuiDataFileOpenDialog(QWidget* parent,
                                             Qt::WindowFlags flags)
   : WuQFileDialog(parent, flags)
{
   setWindowTitle("Open Data File");
   setAcceptMode(AcceptOpen);
   setFileMode(ExistingFiles);
   setDirectory(QDir::currentPath());

   //
   // Caret files
   //
   QStringList filterNames;
   filterNames << FileFilters::getAnyFileFilter();
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
   filterNames << FileFilters::getDeformationMapFileFilter();
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
      filterNames << FileFilters::getGiftiTimeSeriesFileFilter();
      filterNames << FileFilters::getGiftiTopologyFileFilter();
      filterNames << FileFilters::getGiftiVectorFileFilter();
   }
   filterNames << FileFilters::getImageOpenFileFilter();
   filterNames << FileFilters::getLatitudeLongitudeFileFilter();
   filterNames << FileFilters::getMetricOrShapeAsMetricFileFilter();
   filterNames << FileFilters::getPaintFileFilter();
   filterNames << FileFilters::getPaletteFileFilter();
   filterNames << FileFilters::getParamsFileFilter();
   filterNames << FileFilters::getProbAtlasFileFilter();
   filterNames << FileFilters::getRgbPaintFileFilter();
   filterNames << FileFilters::getSceneFileFilter();
   filterNames << FileFilters::getSectionFileFilter();
   filterNames << FileFilters::getStudyCollectionFileFilter();
   filterNames << FileFilters::getStudyMetaDataFileFilter();
   filterNames << FileFilters::getSurfaceShapeOrMetricAsShapeFileFilter();
   filterNames << FileFilters::getTopographyFileFilter();
   filterNames << FileFilters::getTopologyGenericFileFilter();
   filterNames << FileFilters::getTransformationMatrixFileFilter();
   filterNames << FileFilters::getTransformationDataFileFilter();
   filterNames << FileFilters::getTrajectoryFileFilter();
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
   
   //
   // Import Files
   //
   filterNames << FileFilters::getAnalyzeVolumeFileFilter();
   filterNames << FileFilters::getBrainVoyagerFileFilter();
   filterNames << FileFilters::getByuSurfaceFileFilter();
   filterNames << FileFilters::getFreeSurferAsciiSurfaceFileFilter();
   filterNames << FileFilters::getFreeSurferBinarySurfaceFileFilter();
   filterNames << FileFilters::getFreeSurferAsciiCurvatureFileFilter();
   filterNames << FileFilters::getFreeSurferBinaryCurvatureFileFilter();
   filterNames << FileFilters::getFreeSurferAsciiFunctionalFileFilter();
   filterNames << FileFilters::getFreeSurferBinaryFunctionalFileFilter();
   filterNames << FileFilters::getFreeSurferAsciiLabelFileFilter();
   filterNames << FileFilters::getMdPlotFileFilter();
#ifdef HAVE_MINC
   filterNames << FileFilters::getMincVolumeFileFilter();
#endif // HAVE_MINC
   filterNames << FileFilters::getMniObjSurfaceFileFilter();
   filterNames << FileFilters::getNeurolucidaFileFilter();
   filterNames << FileFilters::getRawVolumeFileFilter();
   filterNames << FileFilters::getStlSurfaceFileFilter();
   filterNames << FileFilters::getSumaRgbFileFilter();
   filterNames << FileFilters::getVtkSurfaceFileFilter();
   filterNames << FileFilters::getVtkXmlSurfaceFileFilter();
   filterNames << FileFilters::getVtkVolumeFileFilter();
   
   //
   // Add the filters to the dialog
   //
   setFilters(filterNames);

   //
   // Connect signals
   //
   QObject::connect(this, SIGNAL(filesSelected(const QStringList&)),
                    this, SLOT(slotFilesSelected(const QStringList&)));
                    
   //
   // View/Edit comment button
   //
   viewEditCommentPushButton = new QPushButton("View/Edit Comment...");
   viewEditCommentPushButton->setEnabled(false);
   QObject::connect(viewEditCommentPushButton, SIGNAL(clicked()),
                    this, SLOT(slotViewEditCommentPushButton()));
   
   //
   // Add to spec file check box
   //
   addToSpecFileCheckBox = new QCheckBox("Add to Specification File");
   addToSpecFileCheckBox->setChecked(true);
   
   //
   // Append to current file check box
   //
   appendToCurrentFileCheckBox = new QCheckBox("Append to Current File");
   appendToCurrentFileCheckBox->setChecked(true);
   
   //
   // Group box for options
   //
   QGroupBox* optionsGroupBox = new QGroupBox("Open File Options");
   QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroupBox);
   optionsLayout->addWidget(addToSpecFileCheckBox);
   optionsLayout->addWidget(appendToCurrentFileCheckBox);
   
   //
   // Add widgets to dialog
   //
   addWidgets(optionsGroupBox, optionsGroupBox, viewEditCommentPushButton);
}
                  
/**
 * destructor.
 */
GuiDataFileOpenDialog::~GuiDataFileOpenDialog()
{
}

/**
 * called when view/edit comment button pressed.
 */
void 
GuiDataFileOpenDialog::slotViewEditCommentPushButton()
{
   const QStringList files = selectedFiles();
   if (files.count() == 1) {
      const QString highlightedFileName = files.at(0);
      if (highlightedFileName.isEmpty() == false) {
         const QString filterName = selectedFilter();
         
         //
         // See if a volume file that needs to be handles specially
         //
         bool volumeFileFlag =
            (filterName == FileFilters::getVolumeAnatomyFileFilter()) ||
            (filterName == FileFilters::getVolumeFunctionalFileFilter()) ||
            (filterName == FileFilters::getVolumePaintFileFilter()) ||
            (filterName == FileFilters::getVolumeProbAtlasFileFilter()) ||
            (filterName == FileFilters::getVolumeRgbFileFilter()) ||
            (filterName == FileFilters::getVolumeSegmentationFileFilter()) ||
            (filterName == FileFilters::getVolumeVectorFileFilter());
         if (selectedFilter() == FileFilters::getAnyFileFilter()) {
            const QStringList mf = matchingFilters(highlightedFileName);
            for (int i = 0; i < mf.count(); i++) {
               if (mf.at(i) == FileFilters::getVolumeAnatomyFileFilter()) {
                  volumeFileFlag = true;
                  break;
               }
            }
         }
         
         GuiDataFileCommentDialog* dfcd = new GuiDataFileCommentDialog(this,
                                                                       highlightedFileName,
                                                                       volumeFileFlag);
         dfcd->show();
      }
   }
}

/**
 * called when the file selection is changed.
 */
void 
GuiDataFileOpenDialog::slotFilesSelected(const QStringList& fileNames)
{
   viewEditCommentPushButton->setEnabled(fileNames.count() == 1);
   
   if (fileNames.count() == 1) {
      const QStringList mf = matchingFilters(fileNames.at(0));
      for (int i = 0; i < mf.count(); i++) {
         //std::cout << "Matching Filters: " << mf.at(i).toAscii().constData() << std::endl;
      }
   }
}

/**
 * returns file filters that match the name but exludes any file filter
 */
QStringList 
GuiDataFileOpenDialog::matchingFilters(const QString& name)
{
   //
   // Try a list of all but ANY and filters that have no extension
   //
   QStringList filterList = WuQFileDialog::matchingFilters(name);
   filterList.removeAll(FileFilters::getAnyFileFilter());
   filterList.removeAll(FileFilters::getFreeSurferBinarySurfaceFileFilter());
   filterList.removeAll(FileFilters::getRawVolumeFileFilter());
   
   if (filterList.count() == 0) {
      //
      // Get a list of all but ANY
      //
      filterList = WuQFileDialog::matchingFilters(name);
      filterList.removeAll(FileFilters::getAnyFileFilter());
   }
   
   return filterList;
}
      
/**
 * called when dialog is closed.
 */
void 
GuiDataFileOpenDialog::done(int r)
{
   if (r == QDialog::Accepted) {
      //
      // Create priority of files - read first: colors, study, topo
      //                            read later: all other files
      //
      const QStringList unsortedFiles(selectedFiles());
      QStringList priorityLow, priorityHigh;
      for (int i = 0; i < unsortedFiles.count(); i++) {
         const QString fn(unsortedFiles.at(i));
         if (fn.endsWith("color") ||
             fn.endsWith(SpecFile::getStudyMetaDataFileExtension()) ||
             fn.endsWith(SpecFile::getTopoFileExtension())) {
                priorityHigh << fn;
         }
         else {
            priorityLow << fn;
         }
      }
      const QStringList filesChosen(priorityHigh + priorityLow);
      
      //
      // Get the selected files
      //
      const int numFiles = filesChosen.count();
      if (numFiles > 0) {
         //
         // Update previous directories
         //
         PreferencesFile* pf = theMainWindow->getBrainSet()->getPreferencesFile();
         pf->addToRecentDataFileDirectories(FileUtilities::dirname(filesChosen.at(0)), true);
         
         //
         // Add to spec file status and append status
         //
         const bool addToSpecFlag = addToSpecFileCheckBox->isChecked();
         const bool appendToCurrentFileFlag = appendToCurrentFileCheckBox->isChecked();
         
         //
         // Allow user to set the spec file name if not already set
         //
         const QString specFileName(theMainWindow->getBrainSet()->getSpecFileName());
         if (addToSpecFlag && specFileName.isEmpty()) {
            const int result = QMessageBox::question(this, "Spec File", 
                                         "Would you like to create a Spec File ?",
                                         (QMessageBox::Yes |
                                          QMessageBox::No  |
                                          QMessageBox::Cancel),
                                         QMessageBox::No);
            if (result == QMessageBox::Yes) {
               //
               // Create spec file dialog will set the directory and create the spec file
               //
               GuiSpecFileCreationDialog sfcd(this);
               sfcd.exec();
               PreferencesFile* pf = theMainWindow->getBrainSet()->getPreferencesFile();
               pf->addToRecentSpecFiles(sfcd.getCreatedSpecFileName(), true);
            }
            else if (result == QMessageBox::No) {
            }
            else if (result == QMessageBox::Cancel) {
               return;
            }
         }

         //
         // See if file(s) may need to be copied to current directory
         //
         bool copyFlag = false;
         if (addToSpecFlag && 
             (specFileName.isEmpty() == false)) {
            //
            // Is data file in a directory different than that of spec file
            //
            QFileInfo dataFileInfo(filesChosen.at(0));
            QFileInfo specFileInfo(specFileName);
            if (dataFileInfo.absolutePath() != specFileInfo.absolutePath()) {
               const int result = QMessageBox::question(this, "Copy File",
                                       "Would you like to copy the "
                                       "file(s) to the current directory.",
                                         (QMessageBox::Yes |
                                          QMessageBox::No  |
                                          QMessageBox::Cancel),
                                         QMessageBox::No);
               if (result == QMessageBox::Yes) {
                  copyFlag = true;
               }
               else if (result == QMessageBox::No) {
               }
               else if (result == QMessageBox::Cancel) {
                  return;
               }
            }
         }
         
         //
         // Get the name of the file filter
         //
         const QString selectedFilterName(selectedFilter());
         
         //
         // Loop through the selected files
         //
         for (int i = 0; i < numFiles; i++) {
            //
            // Get the file
            //
            QString fileName = filesChosen.at(i);
            const QString nameNoPath(FileUtilities::basename(fileName));
            //std::cout << "Loading: " << fileName.toAscii().constData() << std::endl;
            
            //
            // Should file be copied?
            //
            if (copyFlag) {
               if (copyFile(fileName) == false) {
                  return;
               }
               
               //
               // Remove path since file copied
               //
               fileName = FileUtilities::basename(fileName);
            }
            
            //
            // Get the file filter
            //
            QString filterName = selectedFilterName;
            
            //
            // If filter is any file filter, find matching filter
            //
            if (filterName == FileFilters::getAnyFileFilter()) {
               //
               // Get filter that match the file name
               //
               QStringList filtersThatMatch = matchingFilters(fileName);
               
               //
               // If only one filter, then use it, otherwise ask user
               //
               if (filtersThatMatch.count() == 1) {
                  filterName = filtersThatMatch.at(0);
               }
               else if (filtersThatMatch.count() <= 0) {
                  const QString msg(FileUtilities::basename(fileName) 
                                    + " is not a supported Caret file type.");
                  QMessageBox::critical(this,
                                          "ERROR",
                                          msg);
                  return;
               }
               else {
                  //
                  // Ask user which file type to use
                  //
                  std::vector<QString> labels;
                  for (int i = 0; i < filtersThatMatch.count(); i++) {
                     QString name = filtersThatMatch.at(i);
                     const int indx = name.indexOf('(');
                     if (indx > 0) {
                        name = name.left(indx);
                     }
                     labels.push_back(name);
                  }
                  
                  QtRadioButtonSelectionDialog rbsd(this,
                                                    "Choose File Type",
                                                    "Type of File for " + nameNoPath,
                                                    labels,
                                                    0);
                  if (rbsd.exec() == QtRadioButtonSelectionDialog::Accepted) {
                     filterName = filtersThatMatch.at(rbsd.getSelectedItemIndex());
                  }
                  else {
                     return;
                  }
               }
            }
            
            //
            // Read the file
            //
            readFile(fileName,
                     filterName,
                     addToSpecFlag,
                     appendToCurrentFileFlag);
         }
      }
   }
   
   WuQFileDialog::done(r);
}

/**
 * read the file.
 */
void 
GuiDataFileOpenDialog::readFile(const QString& fileName,
                                const QString& filterName,
                                const bool addToSpecFileFlag,
                                const bool appendToCurrentFileFlag)
{
   //
   // SPM Volume use Analyze file extension and sometimes 
   // are left/right flipped
   //
   bool spmLeftOnRightFlag = false;
   const bool volumeFileFlag =
      (filterName == FileFilters::getVolumeAnatomyFileFilter()) ||
      (filterName == FileFilters::getVolumeFunctionalFileFilter()) ||
      (filterName == FileFilters::getVolumePaintFileFilter()) ||
      (filterName == FileFilters::getVolumeProbAtlasFileFilter()) ||
      (filterName == FileFilters::getVolumeRgbFileFilter()) ||
      (filterName == FileFilters::getVolumeSegmentationFileFilter()) ||
      (filterName == FileFilters::getVolumeVectorFileFilter());
   if (volumeFileFlag) {
      if (fileName.endsWith(SpecFile::getAnalyzeVolumeFileExtension())) {
         const QString msg("If the volume being loaded an SPM volume that\n"
                           "is left/right flipped (radiological orientation)?");
         spmLeftOnRightFlag = (QMessageBox::question(this,
                                                       "SPM Flipped",
                                                       msg,
                                                       (QMessageBox::Yes | QMessageBox::No))
                                                     == QMessageBox::Yes);
      }
   }
   
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   theMainWindow->getBrainSet()->setDisplaySplashImage(false);
   
   QString msg;      
   bool error = false;
   bool warning = false;
   
   if (filterName == FileFilters::getAreaColorFileFilter()) {
      error = openDataFile(this, SpecFile::getAreaColorFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getArealEstimationFileFilter()) {
      error = openDataFile(this, SpecFile::getArealEstimationFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderRawFileFilter()) {
      error = openDataFile(this, SpecFile::getRawBorderFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderFiducialFileFilter()) {
      error = openDataFile(this, SpecFile::getFiducialBorderFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderInflatedFileFilter()) {
      error = openDataFile(this, SpecFile::getInflatedBorderFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderVeryInflatedFileFilter()) {
      error = openDataFile(this, SpecFile::getVeryInflatedBorderFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderSphericalFileFilter()) {
      error = openDataFile(this, SpecFile::getSphericalBorderFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderEllipsoidalFileFilter()) {
      error = openDataFile(this, SpecFile::getEllipsoidBorderFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderCompressedFileFilter()) {
      error = openDataFile(this, SpecFile::getCompressedBorderFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderFlatFileFilter()) {
      error = openDataFile(this, SpecFile::getFlatBorderFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderFlatLobarFileFilter()) {
      error = openDataFile(this, SpecFile::getLobarFlatBorderFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderHullFileFilter()) {
      error = openDataFile(this, SpecFile::getHullBorderFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderUnknownFileFilter()) {
      error = openDataFile(this, SpecFile::getUnknownBorderFileMatchTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderVolumeFileFilter()) {
      error = openDataFile(this, SpecFile::getVolumeBorderFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderGenericFileFilter()) {
      error = openDataFile(this, GENERIC_BORDER, fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderColorFileFilter()) {
      error = openDataFile(this, SpecFile::getBorderColorFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getBorderProjectionFileFilter()) {
      error = openDataFile(this, SpecFile::getBorderProjectionFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCellFileFilter()) {
      error = openDataFile(this, SpecFile::getCellFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCellColorFileFilter()) {
      error = openDataFile(this, SpecFile::getCellColorFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCellProjectionFileFilter()) {
      error = openDataFile(this, SpecFile::getCellProjectionFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCellVolumeFileFilter()) {
      error = openDataFile(this, SpecFile::getVolumeCellFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCocomacFileFilter()) {
      error = openDataFile(this, SpecFile::getCocomacConnectivityFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getContourFileFilter()) {
      error = openDataFile(this, SpecFile::getContourFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getContourCellFileFilter()) {
      error = openDataFile(this, SpecFile::getContourCellFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getContourCellColorFileFilter()) {
      error = openDataFile(this, SpecFile::getContourCellColorFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCoordinateRawFileFilter()) {
      error = openDataFile(this, SpecFile::getRawCoordFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCoordinateFiducialFileFilter()) {
      error = openDataFile(this, SpecFile::getFiducialCoordFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCoordinateInflatedFileFilter()) {
      error = openDataFile(this, SpecFile::getInflatedCoordFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCoordinateVeryInflatedFileFilter()) {
      error = openDataFile(this, SpecFile::getVeryInflatedCoordFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCoordinateSphericalFileFilter()) {
      error = openDataFile(this, SpecFile::getSphericalCoordFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCoordinateEllipsoidalFileFilter()) {
      error = openDataFile(this, SpecFile::getEllipsoidCoordFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCoordinateCompressedFileFilter()) {
      error = openDataFile(this, SpecFile::getCompressedCoordFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCoordinateFlatFileFilter()) {
      error = openDataFile(this, SpecFile::getFlatCoordFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCoordinateFlatLobarFileFilter()) {
      error = openDataFile(this, SpecFile::getLobarFlatCoordFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCoordinateHullFileFilter()) {
      error = openDataFile(this, SpecFile::getHullCoordFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCoordinateUnknownFileFilter()) {
      error = openDataFile(this, SpecFile::getUnknownCoordFileMatchTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCoordinateGenericFileFilter()) {
      error = openDataFile(this, GENERIC_COORDINATE, fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getCutsFileFilter()) {
      error = openDataFile(this, SpecFile::getCutsFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getDeformationFieldFileFilter()) {
      error = openDataFile(this, SpecFile::getDeformationFieldFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getDeformationMapFileFilter()) {
      error = openDataFile(this, SpecFile::getDeformationMapFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getFociFileFilter()) {
      error = openDataFile(this, SpecFile::getFociFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getFociColorFileFilter()) {
      error = openDataFile(this, SpecFile::getFociColorFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getFociProjectionFileFilter()) {
      error = openDataFile(this, SpecFile::getFociProjectionFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getFociSearchFileFilter()) {
      error = openDataFile(this, SpecFile::getFociSearchFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getGeodesicDistanceFileFilter()) {
      error = openDataFile(this, SpecFile::getGeodesicDistanceFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getGiftiCoordinateFileFilter()) {
      error = openDataFile(this, GENERIC_COORDINATE, fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if ((filterName == FileFilters::getGiftiFunctionalFileFilter()) ||
            (filterName == FileFilters::getGiftiTimeSeriesFileFilter())) {
      error = openDataFile(this, SpecFile::getMetricFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getGiftiLabelFileFilter()) {
      error = openDataFile(this, SpecFile::getPaintFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getGiftiShapeFileFilter()) {
      error = openDataFile(this, SpecFile::getSurfaceShapeFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getGiftiSurfaceFileFilter()) {
      error = openDataFile(this, GENERIC_SURFACE, fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getGiftiTopologyFileFilter()) {
      error = openDataFile(this, GENERIC_TOPOLOGY, fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getGiftiVectorFileFilter()) {
      error = openDataFile(this, SpecFile::getVectorFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getImageOpenFileFilter()) {
      error = openDataFile(this, SpecFile::getImageFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getLatitudeLongitudeFileFilter()) {
      error = openDataFile(this, SpecFile::getLatLonFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if ((filterName == FileFilters::getMetricFileFilter()) ||
            (filterName == FileFilters::getMetricOrShapeAsMetricFileFilter())) {
      error = openDataFile(this, SpecFile::getMetricFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getPaintFileFilter()) {
      error = openDataFile(this, SpecFile::getPaintFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getPaletteFileFilter()) {
      error = openDataFile(this, SpecFile::getPaletteFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getParamsFileFilter()) {
      error = openDataFile(this, SpecFile::getParamsFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getProbAtlasFileFilter()) {
      error = openDataFile(this, SpecFile::getAtlasFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getRgbPaintFileFilter()) {
      error = openDataFile(this, SpecFile::getRgbPaintFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getSceneFileFilter()) {
      error = openDataFile(this, SpecFile::getSceneFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getSectionFileFilter()) {
      error = openDataFile(this, SpecFile::getSectionFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if ((filterName == FileFilters::getSurfaceShapeFileFilter()) ||
            (filterName == FileFilters::getSurfaceShapeOrMetricAsShapeFileFilter())) {
      error = openDataFile(this, SpecFile::getSurfaceShapeFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getTopographyFileFilter()) {
      error = openDataFile(this, SpecFile::getTopographyFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getTopologyClosedFileFilter()) {
      error = openDataFile(this, SpecFile::getClosedTopoFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getTopologyOpenFileFilter()) {
      error = openDataFile(this, SpecFile::getOpenTopoFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getTopologyCutFileFilter()) {
      error = openDataFile(this, SpecFile::getCutTopoFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getTopologyCutLobarFileFilter()) {
      error = openDataFile(this, SpecFile::getLobarCutTopoFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getTopologyUnknownFileFilter()) {
      error = openDataFile(this, SpecFile::getUnknownTopoFileMatchTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getTopologyGenericFileFilter()) {
      error = openDataFile(this, GENERIC_TOPOLOGY, fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getTrajectoryFileFilter()) {
      error = openDataFile(this, SpecFile::getTrajectoryFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getVolumeAnatomyFileFilter()) {
      error = openDataFile(this, SpecFile::getVolumeAnatomyFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
      if (spmLeftOnRightFlag) {
         const int volumeIndex = theMainWindow->getBrainSet()->getNumberOfVolumeAnatomyFiles();
         if (volumeIndex > 0) {
            VolumeFile* vf = theMainWindow->getBrainSet()->getVolumeAnatomyFile(volumeIndex - 1);
            vf->flip(VolumeFile::VOLUME_AXIS_X, false);
            vf->clearModified();
         }
      }
   }
   else if (filterName == FileFilters::getVolumeFunctionalFileFilter()) {
      error = openDataFile(this, SpecFile::getVolumeFunctionalFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
      if (spmLeftOnRightFlag) {
         const int volumeIndex = theMainWindow->getBrainSet()->getNumberOfVolumeFunctionalFiles();
         if (volumeIndex > 0) {
            VolumeFile* vf = theMainWindow->getBrainSet()->getVolumeFunctionalFile(volumeIndex - 1);
            vf->flip(VolumeFile::VOLUME_AXIS_X, false);
            vf->clearModified();
         }
      }
   }
   else if (filterName == FileFilters::getVolumePaintFileFilter()) {
      error = openDataFile(this, SpecFile::getVolumePaintFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
      if (spmLeftOnRightFlag) {
         const int volumeIndex = theMainWindow->getBrainSet()->getNumberOfVolumePaintFiles();
         if (volumeIndex > 0) {
            VolumeFile* vf = theMainWindow->getBrainSet()->getVolumePaintFile(volumeIndex - 1);
            vf->flip(VolumeFile::VOLUME_AXIS_X, false);
            vf->clearModified();
         }
      }
   }
   else if (filterName == FileFilters::getVolumeProbAtlasFileFilter()) {
      error = openDataFile(this, SpecFile::getVolumeProbAtlasFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
      if (spmLeftOnRightFlag) {
         const int volumeIndex = theMainWindow->getBrainSet()->getNumberOfVolumeProbAtlasFiles();
         if (volumeIndex > 0) {
            VolumeFile* vf = theMainWindow->getBrainSet()->getVolumeProbAtlasFile(volumeIndex - 1);
            vf->flip(VolumeFile::VOLUME_AXIS_X, false);
            vf->clearModified();
         }
      }
   }
   else if (filterName == FileFilters::getVolumeRgbFileFilter()) {
      error = openDataFile(this, SpecFile::getVolumeRgbFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
      if (spmLeftOnRightFlag) {
         const int volumeIndex = theMainWindow->getBrainSet()->getNumberOfVolumeRgbFiles();
         if (volumeIndex > 0) {
            VolumeFile* vf = theMainWindow->getBrainSet()->getVolumeRgbFile(volumeIndex - 1);
            vf->flip(VolumeFile::VOLUME_AXIS_X, false);
            vf->clearModified();
         }
      }
   }
   else if (filterName == FileFilters::getVolumeSegmentationFileFilter()) {
      error = openDataFile(this, SpecFile::getVolumeSegmentationFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
      if (spmLeftOnRightFlag) {
         const int volumeIndex = theMainWindow->getBrainSet()->getNumberOfVolumeSegmentationFiles();
         if (volumeIndex > 0) {
            VolumeFile* vf = theMainWindow->getBrainSet()->getVolumeSegmentationFile(volumeIndex - 1);
            vf->flip(VolumeFile::VOLUME_AXIS_X, false);
            vf->clearModified();
         }
      }
   }
   else if (filterName == FileFilters::getVolumeVectorFileFilter()) {
      error = openDataFile(this, SpecFile::getVolumeVectorFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
      if (spmLeftOnRightFlag) {
         const int volumeIndex = theMainWindow->getBrainSet()->getNumberOfVolumeVectorFiles();
         if (volumeIndex > 0) {
            VolumeFile* vf = theMainWindow->getBrainSet()->getVolumeVectorFile(volumeIndex - 1);
            vf->flip(VolumeFile::VOLUME_AXIS_X, false);
            vf->clearModified();
         }
      }
   }
   else if (filterName == FileFilters::getStudyCollectionFileFilter()) {
      error = openDataFile(this, SpecFile::getStudyCollectionFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getStudyMetaDataFileFilter()) {
      error = openDataFile(this, SpecFile::getStudyMetaDataFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getTransformationMatrixFileFilter()) {
      error = openDataFile(this, SpecFile::getTransformationMatrixFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getTransformationDataFileFilter()) {
      error = openDataFile(this, SpecFile::getTransformationDataFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getVtkModelFileFilter()) {
      error = openDataFile(this, SpecFile::getVtkModelFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getVocabularyFileFilter()) {
      error = openDataFile(this, SpecFile::getVocabularyFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (filterName == FileFilters::getWustlRegionFileFilter()) {
      error = openDataFile(this, SpecFile::getWustlRegionFileTag(), fileName, appendToCurrentFileFlag, addToSpecFileFlag, msg, warning);
   }
   else if (importFile(fileName, filterName)) {
      // do nothing here
   }
   else {
      std::cerr << "PROGRAM ERROR: unhandled file type at " << __LINE__
               << " in " << __FILE__ << std::endl;
      std::cerr << "   File: " << fileName.toAscii().constData() << std::endl;
      std::cerr << "   Filter: " << filterName.toAscii().constData() << std::endl;
      QApplication::restoreOverrideCursor(); 
      return;
   }
   
   QApplication::restoreOverrideCursor(); 
   if (error) {
      QMessageBox::critical(this, "Error Reading File", msg);
      return;
   }
}                    

/**
 * process import files (returns true if filter matched an import file).
 */
bool 
GuiDataFileOpenDialog::importFile(const QString& fileName,
                                  const QString& filterName)
{
   GuiFilesModified fm;
   BrainSet* brainSet = theMainWindow->getBrainSet();

   QApplication::restoreOverrideCursor();
   
   try {
      if (filterName == FileFilters::getAnalyzeVolumeFileFilter()) {
         GuiDataFileImportOptionsDialog iod(this, filterName);
         if (iod.exec() == GuiDataFileImportOptionsDialog::Accepted) {
            VolumeFile::VOLUME_TYPE volumeType;
            int dimensions[3];
            VolumeFile::VOXEL_DATA_TYPE voxelDataType;
            bool byteSwap;
            iod.getVolumeOptions(volumeType,
                                 dimensions,
                                 voxelDataType,
                                 byteSwap);
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            brainSet->importAnalyzeVolumeFile(fileName,
                                              volumeType);
            fm.setVolumeModified();
         }
      }
      else if (filterName == FileFilters::getBrainVoyagerFileFilter()) {
         GuiDataFileImportOptionsDialog iod(this, filterName);
         if (iod.exec() == GuiDataFileImportOptionsDialog::Accepted) {
            Structure::STRUCTURE_TYPE structureType;
            bool importTopology;
            TopologyFile::TOPOLOGY_TYPES topologyType;
            bool importCoordinates;
            BrainModelSurface::SURFACE_TYPES coordinateType;
            bool importColorsAsPaint;
            bool importColorsAsRgbPaint;
            iod.getSurfaceOptions(structureType,
                                  importTopology,
                                  topologyType,
                                  importCoordinates,
                                  coordinateType,
                                  importColorsAsPaint,
                                  importColorsAsRgbPaint);
            if (brainSet->getStructure().getType() == Structure::STRUCTURE_TYPE_INVALID) {
               brainSet->setStructure(structureType);
            }
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            brainSet->importBrainVoyagerFile(fileName,
                                             importCoordinates,
                                             importTopology,
                                             importColorsAsPaint,
                                             coordinateType,
                                             topologyType);
            fm.setCoordinateModified();
            fm.setPaintModified();
            fm.setAreaColorModified();
            fm.setTopologyModified();
         }
      }
      else if (filterName == FileFilters::getByuSurfaceFileFilter()) {
         GuiDataFileImportOptionsDialog iod(this, filterName);
         if (iod.exec() == GuiDataFileImportOptionsDialog::Accepted) {
            Structure::STRUCTURE_TYPE structureType;
            bool importTopology;
            TopologyFile::TOPOLOGY_TYPES topologyType;
            bool importCoordinates;
            BrainModelSurface::SURFACE_TYPES coordinateType;
            bool importColorsAsPaint;
            bool importColorsAsRgbPaint;
            iod.getSurfaceOptions(structureType,
                                  importTopology,
                                  topologyType,
                                  importCoordinates,
                                  coordinateType,
                                  importColorsAsPaint,
                                  importColorsAsRgbPaint);
            if (brainSet->getStructure().getType() == Structure::STRUCTURE_TYPE_INVALID) {
               brainSet->setStructure(structureType);
            }
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            brainSet->importByuSurfaceFile(fileName,
                                             importCoordinates,
                                             importTopology,
                                             coordinateType,
                                             topologyType);
            fm.setCoordinateModified();
            fm.setTopologyModified();
         }
      }
      else if (filterName == FileFilters::getFreeSurferAsciiSurfaceFileFilter()) {
         GuiDataFileImportOptionsDialog iod(this, filterName);
         if (iod.exec() == GuiDataFileImportOptionsDialog::Accepted) {
            Structure::STRUCTURE_TYPE structureType;
            bool importTopology;
            TopologyFile::TOPOLOGY_TYPES topologyType;
            bool importCoordinates;
            BrainModelSurface::SURFACE_TYPES coordinateType;
            bool importColorsAsPaint;
            bool importColorsAsRgbPaint;
            iod.getSurfaceOptions(structureType,
                                  importTopology,
                                  topologyType,
                                  importCoordinates,
                                  coordinateType,
                                  importColorsAsPaint,
                                  importColorsAsRgbPaint);
            if (brainSet->getStructure().getType() == Structure::STRUCTURE_TYPE_INVALID) {
               brainSet->setStructure(structureType);
            }
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            brainSet->importFreeSurferSurfaceFile(fileName,
                                             importCoordinates,
                                             importTopology,
                                             AbstractFile::FILE_FORMAT_ASCII,
                                             coordinateType,
                                             topologyType);
            fm.setCoordinateModified();
            fm.setTopologyModified();
         }
      }
      else if (filterName == FileFilters::getFreeSurferBinarySurfaceFileFilter()) {
         GuiDataFileImportOptionsDialog iod(this, filterName);
         if (iod.exec() == GuiDataFileImportOptionsDialog::Accepted) {
            Structure::STRUCTURE_TYPE structureType;
            bool importTopology;
            TopologyFile::TOPOLOGY_TYPES topologyType;
            bool importCoordinates;
            BrainModelSurface::SURFACE_TYPES coordinateType;
            bool importColorsAsPaint;
            bool importColorsAsRgbPaint;
            iod.getSurfaceOptions(structureType,
                                  importTopology,
                                  topologyType,
                                  importCoordinates,
                                  coordinateType,
                                  importColorsAsPaint,
                                  importColorsAsRgbPaint);
            if (brainSet->getStructure().getType() == Structure::STRUCTURE_TYPE_INVALID) {
               brainSet->setStructure(structureType);
            }
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            brainSet->importFreeSurferSurfaceFile(fileName,
                                             importCoordinates,
                                             importTopology,
                                             AbstractFile::FILE_FORMAT_BINARY,
                                             coordinateType,
                                             topologyType);
            fm.setCoordinateModified();
            fm.setTopologyModified();
         }
      }
      else if (filterName == FileFilters::getFreeSurferAsciiCurvatureFileFilter()) {
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         SurfaceShapeFile* ssf = brainSet->getSurfaceShapeFile();
         ssf->importFreeSurferCurvatureFile(brainSet->getNumberOfNodes(),
                                            fileName,
                                            AbstractFile::FILE_FORMAT_ASCII);
         fm.setSurfaceShapeModified();
      }
      else if (filterName == FileFilters::getFreeSurferBinaryCurvatureFileFilter()) {
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         SurfaceShapeFile* ssf = brainSet->getSurfaceShapeFile();
         ssf->importFreeSurferCurvatureFile(brainSet->getNumberOfNodes(),
                                            fileName,
                                            AbstractFile::FILE_FORMAT_BINARY);
         fm.setSurfaceShapeModified();
      }
      else if (filterName == FileFilters::getFreeSurferAsciiFunctionalFileFilter()) {
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         MetricFile* mf = brainSet->getMetricFile();
         mf->importFreeSurferFunctionalFile(brainSet->getNumberOfNodes(),
                                            fileName,
                                            AbstractFile::FILE_FORMAT_ASCII);
         fm.setMetricModified();
      }
      else if (filterName == FileFilters::getFreeSurferBinaryFunctionalFileFilter()) {
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         MetricFile* mf = brainSet->getMetricFile();
         mf->importFreeSurferFunctionalFile(brainSet->getNumberOfNodes(),
                                            fileName,
                                            AbstractFile::FILE_FORMAT_BINARY);
         fm.setMetricModified();
      }
      else if (filterName == FileFilters::getFreeSurferAsciiLabelFileFilter()) {
         bool importAll = false;
         if (QMessageBox::question(this, "Import All",
                                   "Import all label files in directory ?",
                                   (QMessageBox::Yes | QMessageBox::No)) == QMessageBox::Yes) {
            importAll = true;
         }
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         PaintFile* pf = brainSet->getPaintFile();
         pf->importFreeSurferAsciiLabelFile(brainSet->getNumberOfNodes(),
                                            fileName,
                                            brainSet->getAreaColorFile(),
                                            importAll);
         fm.setPaintModified();
         fm.setAreaColorModified();
      }
      else if (filterName == FileFilters::getMdPlotFileFilter()) {
         GuiDataFileImportOptionsDialog iod(this, filterName);
         if (iod.exec() == GuiDataFileImportOptionsDialog::Accepted) {
            bool importContourCells;
            bool importContours;
            iod.getContourOptions(importContours, importContourCells);
            const BrainModelContours* bmc = brainSet->getBrainModelContours();
            bool appendContoursFlag = false;
            bool appendCellsFlag = false;
            if (importContours) {
               if (bmc != NULL) {
                  const ContourFile* cf = bmc->getContourFile();
                  if (cf->empty() == false) {
                     appendContoursFlag = (QMessageBox::question(this, "Import MD Plot",
                                                 "Append to existing contours?",
                                                 (QMessageBox::Yes | QMessageBox::No)) == QMessageBox::Yes);
                  }
               }
            }
            if (importContourCells) {
               const ContourCellFile* cf = brainSet->getContourCellFile();
               if (cf->empty() == false) {
                  appendCellsFlag = (QMessageBox::question(this, "Import MD Plot",
                                              "Append to existing contour cells?",
                                              (QMessageBox::Yes | QMessageBox::No)) == QMessageBox::Yes);
               }
            }
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            brainSet->importMDPlotFile(fileName, 
                                    importContourCells,
                                    importContours,
                                    appendContoursFlag,
                                    appendCellsFlag);
            fm.setContourModified();
            fm.setContourCellModified();
            fm.setContourCellColorModified();
         }
      }
   #ifdef HAVE_MINC
      else if (filterName == FileFilters::getMincVolumeFileFilter()) {
         GuiDataFileImportOptionsDialog iod(this, filterName);
         if (iod.exec() == GuiDataFileImportOptionsDialog::Accepted) {
            VolumeFile::VOLUME_TYPE volumeType;
            int dimensions[3];
            VolumeFile::VOXEL_DATA_TYPE voxelDataType;
            bool byteSwap;
            iod.getVolumeOptions(volumeType,
                                 dimensions,
                                 voxelDataType,
                                 byteSwap);
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            brainSet->importMincVolumeFile(fileName,
                                           volumeType);
            fm.setVolumeModified();
         }
      }
   #endif // HAVE_MINC
      else if (filterName == FileFilters::getMniObjSurfaceFileFilter()) {
         GuiDataFileImportOptionsDialog iod(this, filterName);
         if (iod.exec() == GuiDataFileImportOptionsDialog::Accepted) {
            Structure::STRUCTURE_TYPE structureType;
            bool importTopology;
            TopologyFile::TOPOLOGY_TYPES topologyType;
            bool importCoordinates;
            BrainModelSurface::SURFACE_TYPES coordinateType;
            bool importColorsAsPaint;
            bool importColorsAsRgbPaint;
            iod.getSurfaceOptions(structureType,
                                  importTopology,
                                  topologyType,
                                  importCoordinates,
                                  coordinateType,
                                  importColorsAsPaint,
                                  importColorsAsRgbPaint);
            if (brainSet->getStructure().getType() == Structure::STRUCTURE_TYPE_INVALID) {
               brainSet->setStructure(structureType);
            }
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            brainSet->importMniObjSurfaceFile(fileName,
                                             importCoordinates,
                                             importTopology,
                                             importColorsAsRgbPaint,
                                             coordinateType,
                                             topologyType);
            fm.setCoordinateModified();
            fm.setRgbPaintModified();
            fm.setTopologyModified();
         }
      }
      else if (filterName == FileFilters::getNeurolucidaFileFilter()) {
         GuiDataFileImportOptionsDialog iod(this, filterName);
         if (iod.exec() == GuiDataFileImportOptionsDialog::Accepted) {
            bool importContourCells;
            bool importContours;
            iod.getContourOptions(importContours, importContourCells);
            const BrainModelContours* bmc = brainSet->getBrainModelContours();
            bool appendContoursFlag = false;
            bool appendCellsFlag = false;
            if (importContours) {
               if (bmc != NULL) {
                  const ContourFile* cf = bmc->getContourFile();
                  if (cf->empty() == false) {
                     appendContoursFlag = (QMessageBox::question(this, "Import Neurolucida",
                                                 "Append to existing contours?",
                                                 (QMessageBox::Yes | QMessageBox::No)) == QMessageBox::Yes);
                  }
               }
            }
            if (importContourCells) {
               const ContourCellFile* cf = brainSet->getContourCellFile();
               if (cf->empty() == false) {
                  appendCellsFlag = (QMessageBox::question(this, "Import Neurolucida",
                                              "Append to existing contour cells?",
                                              (QMessageBox::Yes | QMessageBox::No)) == QMessageBox::Yes);
               }
            }
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            brainSet->importNeurolucidaFile(fileName, 
                                    importContourCells,
                                    importContours,
                                    appendContoursFlag,
                                    appendCellsFlag);
            fm.setContourModified();
            fm.setContourCellModified();
            fm.setContourCellColorModified();
         }
      }
      else if (filterName == FileFilters::getRawVolumeFileFilter()) {
         GuiDataFileImportOptionsDialog iod(this, filterName);
         if (iod.exec() == GuiDataFileImportOptionsDialog::Accepted) {
            VolumeFile::VOLUME_TYPE volumeType;
            int dimensions[3];
            VolumeFile::VOXEL_DATA_TYPE voxelDataType;
            bool byteSwap;
            iod.getVolumeOptions(volumeType,
                                 dimensions,
                                 voxelDataType,
                                 byteSwap);
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            brainSet->importRawVolumeFile(fileName,
                                          volumeType,
                                          dimensions,
                                          voxelDataType,
                                          byteSwap);
            fm.setVolumeModified();
         }
      }
      else if (filterName == FileFilters::getStlSurfaceFileFilter()) {
         GuiDataFileImportOptionsDialog iod(this, filterName);
         if (iod.exec() == GuiDataFileImportOptionsDialog::Accepted) {
            Structure::STRUCTURE_TYPE structureType;
            bool importTopology;
            TopologyFile::TOPOLOGY_TYPES topologyType;
            bool importCoordinates;
            BrainModelSurface::SURFACE_TYPES coordinateType;
            bool importColorsAsPaint;
            bool importColorsAsRgbPaint;
            iod.getSurfaceOptions(structureType,
                                  importTopology,
                                  topologyType,
                                  importCoordinates,
                                  coordinateType,
                                  importColorsAsPaint,
                                  importColorsAsRgbPaint);
            if (brainSet->getStructure().getType() == Structure::STRUCTURE_TYPE_INVALID) {
               brainSet->setStructure(structureType);
            }
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            brainSet->importStlSurfaceFile(fileName,
                                             importCoordinates,
                                             importTopology,
                                             coordinateType,
                                             topologyType);
            fm.setCoordinateModified();
            fm.setTopologyModified();
         }
      }
      else if (filterName == FileFilters::getSumaRgbFileFilter()) {
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         RgbPaintFile* rgb = brainSet->getRgbPaintFile();
         rgb->importFromSuma(fileName);
         fm.setRgbPaintModified();
      }
      else if (filterName == FileFilters::getVtkSurfaceFileFilter()) {
         GuiDataFileImportOptionsDialog iod(this, filterName);
         if (iod.exec() == GuiDataFileImportOptionsDialog::Accepted) {
            Structure::STRUCTURE_TYPE structureType;
            bool importTopology;
            TopologyFile::TOPOLOGY_TYPES topologyType;
            bool importCoordinates;
            BrainModelSurface::SURFACE_TYPES coordinateType;
            bool importColorsAsPaint;
            bool importColorsAsRgbPaint;
            iod.getSurfaceOptions(structureType,
                                  importTopology,
                                  topologyType,
                                  importCoordinates,
                                  coordinateType,
                                  importColorsAsPaint,
                                  importColorsAsRgbPaint);
            if (brainSet->getStructure().getType() == Structure::STRUCTURE_TYPE_INVALID) {
               brainSet->setStructure(structureType);
            }
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            brainSet->importVtkSurfaceFile(fileName,
                                             importCoordinates,
                                             importTopology,
                                             importColorsAsRgbPaint,
                                             coordinateType,
                                             topologyType);
            fm.setCoordinateModified();
            fm.setRgbPaintModified();
            fm.setTopologyModified();
         }
      }
      else if (filterName == FileFilters::getVtkXmlSurfaceFileFilter()) {
         GuiDataFileImportOptionsDialog iod(this, filterName);
         if (iod.exec() == GuiDataFileImportOptionsDialog::Accepted) {
            Structure::STRUCTURE_TYPE structureType;
            bool importTopology;
            TopologyFile::TOPOLOGY_TYPES topologyType;
            bool importCoordinates;
            BrainModelSurface::SURFACE_TYPES coordinateType;
            bool importColorsAsPaint;
            bool importColorsAsRgbPaint;
            iod.getSurfaceOptions(structureType,
                                  importTopology,
                                  topologyType,
                                  importCoordinates,
                                  coordinateType,
                                  importColorsAsPaint,
                                  importColorsAsRgbPaint);
            if (brainSet->getStructure().getType() == Structure::STRUCTURE_TYPE_INVALID) {
               brainSet->setStructure(structureType);
            }
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            brainSet->importVtkXmlSurfaceFile(fileName,
                                             importCoordinates,
                                             importTopology,
                                             importColorsAsRgbPaint,
                                             coordinateType,
                                             topologyType);
            fm.setCoordinateModified();
            fm.setRgbPaintModified();
            fm.setTopologyModified();
         }
      }
      else if (filterName == FileFilters::getVtkVolumeFileFilter()) {
         GuiDataFileImportOptionsDialog iod(this, filterName);
         if (iod.exec() == GuiDataFileImportOptionsDialog::Accepted) {
            VolumeFile::VOLUME_TYPE volumeType;
            int dimensions[3];
            VolumeFile::VOXEL_DATA_TYPE voxelDataType;
            bool byteSwap;
            iod.getVolumeOptions(volumeType,
                                 dimensions,
                                 voxelDataType,
                                 byteSwap);
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            brainSet->importVtkStructuredPointsVolumeFile(fileName,
                                          volumeType);
            fm.setVolumeModified();
         }
      }
      else {
         return false;
      }
   }
   catch (FileException& e) {
      QApplication::restoreOverrideCursor(); 
      QMessageBox::critical(this, "ERROR",
                            e.whatQString());
   }
   
   //
   // Remove splash image
   //
   brainSet->setDisplaySplashImage(false);
   
   //
   // Update controls and redraw
   //
   theMainWindow->fileModificationUpdate(fm);
   if (fm.getCoordinateModified()) {
      theMainWindow->displayNewestSurfaceInMainWindow();
   }
   brainSet->getNodeColoring()->assignColors();
   
   GuiBrainModelOpenGL::updateAllGL(NULL);

   QApplication::restoreOverrideCursor(); 
   
   return true;
}
                      
/**
 * This static method provides one place to open data files and make the necessary
 * updates as a result.  Returns true if an error occurs.
 */
bool
GuiDataFileOpenDialog::openDataFile(QWidget* parentWidget, const QString specFileTag,
                      const QString& name, const bool append, const bool update,
                      QString& errorMessage, bool& relatedFileWarningFlag)
{
   const QString nameNoPath(FileUtilities::basename(name));
   
   errorMessage = "";
   relatedFileWarningFlag = false;
   
   const bool needAreaColors        = (theMainWindow->getBrainSet()->getAreaColorFile()->getNumberOfColors() <= 0);
   const bool needBorderColors      = (theMainWindow->getBrainSet()->getBorderColorFile()->getNumberOfColors() <= 0);
   const bool needCellColors        = (theMainWindow->getBrainSet()->getCellColorFile()->getNumberOfColors() <= 0);
   const bool needContourCellColors = (theMainWindow->getBrainSet()->getContourCellColorFile()->getNumberOfColors() <= 0);
   const bool needFociColors        = (theMainWindow->getBrainSet()->getFociColorFile()->getNumberOfColors() <= 0);

   bool areaColorWarning        = false;
   bool borderColorWarning      = false;
   bool cellColorWarning        = false;
   bool contourCellColorWarning = false;
   bool fociColorWarning        = false;
   
   bool bordersLoaded = false;
   
   //
   // See if file exists
   //
   QFileInfo fileInfo(name);
   if (fileInfo.exists() == false) {
      if (fileInfo.isReadable() == false) {
         errorMessage = "File ";
         errorMessage.append(name);
         errorMessage.append(" does not exist!");
         return true;
      }
   }
   else {
      if (fileInfo.isReadable() == false) {
         errorMessage.append("File ");
         errorMessage.append(name);
         errorMessage.append(" does not have read permission!");
         return true;
      }
   }
   
   GuiFilesModified fm;
   
   try {
      if (specFileTag == SpecFile::getAreaColorFileTag()) {
         theMainWindow->getBrainSet()->readAreaColorFile(name, append, update);
         fm.setAreaColorModified();
      }
      else if (specFileTag == SpecFile::getArealEstimationFileTag()) {
         ArealEstimationFile* aef = theMainWindow->getBrainSet()->getArealEstimationFile();
         ArealEstimationFile newFile;
         newFile.readFileMetaDataOnly(name);
         QApplication::restoreOverrideCursor();
         GuiLoadNodeAttributeFileColumnSelectionDialog fcsd(theMainWindow,
                                                      &newFile,
                                                      aef);
         const int result = fcsd.exec(); 
         newFile.clear(); // frees memory
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         if (result == QDialog::Accepted) {
            if (fcsd.getEraseAllExistingColumns()) {
               theMainWindow->getBrainSet()->clearArealEstimationFile();
            }
            theMainWindow->getBrainSet()->readArealEstimationFile(name, 
                                 fcsd.getDestinationColumns(), 
                                 fcsd.getNewFileColumnNames(),
                                 fcsd.getAppendFileCommentSelection(),
                                 update);
            fm.setArealEstimationModified();;
            areaColorWarning = needAreaColors;
         }
      }
      else if (specFileTag == SpecFile::getRawBorderFileTag()) {
         theMainWindow->getBrainSet()->readBorderFile(name, BrainModelSurface::SURFACE_TYPE_RAW, append, update);
         fm.setBorderModified();
         borderColorWarning = needBorderColors;
         bordersLoaded = true;
      }
      else if (specFileTag == SpecFile::getFiducialBorderFileTag()) {
         theMainWindow->getBrainSet()->readBorderFile(name, BrainModelSurface::SURFACE_TYPE_FIDUCIAL, append, update);
         fm.setBorderModified();
         borderColorWarning = needBorderColors;
         bordersLoaded = true;
      }
      else if (specFileTag == SpecFile::getInflatedBorderFileTag()) {
         theMainWindow->getBrainSet()->readBorderFile(name, BrainModelSurface::SURFACE_TYPE_INFLATED, append, update);
         fm.setBorderModified();
         borderColorWarning = needBorderColors;
         bordersLoaded = true;
      }
      else if (specFileTag == SpecFile::getVeryInflatedBorderFileTag()) {
         theMainWindow->getBrainSet()->readBorderFile(name, BrainModelSurface::SURFACE_TYPE_VERY_INFLATED, append, update);
         fm.setBorderModified();
         borderColorWarning = needBorderColors;
         bordersLoaded = true;
      }
      else if (specFileTag == SpecFile::getSphericalBorderFileTag()) {
         theMainWindow->getBrainSet()->readBorderFile(name, BrainModelSurface::SURFACE_TYPE_SPHERICAL, append, update);
         fm.setBorderModified();
         borderColorWarning = needBorderColors;
         bordersLoaded = true;
      }
      else if (specFileTag == SpecFile::getEllipsoidBorderFileTag()) {
         theMainWindow->getBrainSet()->readBorderFile(name, BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL, append, update);
         fm.setBorderModified();
         borderColorWarning = needBorderColors;
         bordersLoaded = true;
      }
      else if (specFileTag == SpecFile::getCompressedBorderFileTag()) {
         theMainWindow->getBrainSet()->readBorderFile(name, BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL, append, update);
         fm.setBorderModified();
         borderColorWarning = needBorderColors;
         bordersLoaded = true;
      }
      else if (specFileTag == SpecFile::getFlatBorderFileTag()) {
         theMainWindow->getBrainSet()->readBorderFile(name, BrainModelSurface::SURFACE_TYPE_FLAT, append, update);
         fm.setBorderModified();
         borderColorWarning = needBorderColors;
         bordersLoaded = true;
      }
      else if (specFileTag == SpecFile::getLobarFlatBorderFileTag()) {
         theMainWindow->getBrainSet()->readBorderFile(name, BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR, append, update);
         fm.setBorderModified();
         borderColorWarning = needBorderColors;
         bordersLoaded = true;
      }
      else if (specFileTag == SpecFile::getVolumeBorderFileTag()) {
         theMainWindow->getBrainSet()->readVolumeBorderFile(name, append, update);
         fm.setBorderModified();
         bordersLoaded = true;
         borderColorWarning = needBorderColors;
      }
      else if (specFileTag == GENERIC_BORDER) {
         BorderFile bf;
         try {
            bf.readFileMetaDataOnly(name);
         }
         catch (FileException& e) {
            throw FileException(name, "Unable to read border file to check type.\n"
                                      + e.whatQString());
         }
         BrainModelSurface::SURFACE_TYPES st = BrainModelSurface::SURFACE_TYPE_UNSPECIFIED;

/*
         AbstractFileHeaderUpdater afhu;
         afhu.readFile(name);
         const QString typeTag = afhu.getHeaderTag(AbstractFile::headerTagConfigurationID);
*/
         const QString typeTag = bf.getHeaderTag(AbstractFile::headerTagConfigurationID);
         if (typeTag.isEmpty() == false) {
            st = BrainModelSurface::getSurfaceTypeFromConfigurationID(typeTag);
         }
         
         QApplication::restoreOverrideCursor();
         if ((st == BrainModelSurface::SURFACE_TYPE_UNSPECIFIED) ||
             (st == BrainModelSurface::SURFACE_TYPE_UNKNOWN)) {
            std::vector<QString> labels;
            labels.push_back("RAW");
            labels.push_back("FIDUCIAL");
            labels.push_back("INFLATED");
            labels.push_back("VERY_INFLATED");
            labels.push_back("SPHERICAL");
            labels.push_back("ELLIPSOIDAL");
            labels.push_back("COMPRESSED_MEDIAL_WALL");
            labels.push_back("FLAT");
            labels.push_back("FLAT_LOBAR");
            labels.push_back("HULL");
            QApplication::beep();
            QtRadioButtonSelectionDialog rbsd(parentWidget,
                                              "Choose Border Type",
                                              "Type of Borders for " + nameNoPath,
                                              labels,
                                              -1);
            if (rbsd.exec() == QDialog::Accepted) {
               st = static_cast<BrainModelSurface::SURFACE_TYPES>(rbsd.getSelectedItemIndex());
            }
            else {
               throw FileException(name, "You must select the type of border");
            }
         }
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         theMainWindow->getBrainSet()->readBorderFile(name, st, append, update);
         fm.setBorderModified();
         borderColorWarning = needBorderColors;
         bordersLoaded = true;
      }
      else if (specFileTag.indexOf(SpecFile::getUnknownBorderFileMatchTag()) >= 0) {
         theMainWindow->getBrainSet()->readBorderFile(name, BrainModelSurface::SURFACE_TYPE_UNKNOWN, append, update);
         fm.setBorderModified();
         bordersLoaded = true;
         borderColorWarning = needBorderColors;
      }
      else if (specFileTag == SpecFile::getBorderColorFileTag()) {
         theMainWindow->getBrainSet()->readBorderColorFile(name, append, update);
         fm.setBorderColorModified();
      }
      else if (specFileTag == SpecFile::getBorderProjectionFileTag()) {
         theMainWindow->getBrainSet()->readBorderProjectionFile(name, append, update);
         fm.setBorderModified();
         borderColorWarning = needBorderColors;
         bordersLoaded = true;
      }
      else if (specFileTag == SpecFile::getCellFileTag()) {
         theMainWindow->getBrainSet()->readCellFile(name, append, update);
         fm.setCellModified();
         cellColorWarning = needCellColors;
      }
      else if (specFileTag == SpecFile::getCellColorFileTag()) {
         theMainWindow->getBrainSet()->readCellColorFile(name, append, update);
         fm.setCellColorModified();
      }
      else if (specFileTag == SpecFile::getCellProjectionFileTag()) {
         theMainWindow->getBrainSet()->readCellProjectionFile(name, append, update);
         fm.setCellProjectionModified();
         cellColorWarning = needCellColors;
      }
      else if (specFileTag == SpecFile::getVolumeCellFileTag()) {
         theMainWindow->getBrainSet()->readVolumeCellFile(name, append, update);
         fm.setCellModified();
         cellColorWarning = needCellColors;
      }
      else if (specFileTag == SpecFile::getCocomacConnectivityFileTag()) {
         theMainWindow->getBrainSet()->readCocomacConnectivityFile(name, append, update);
         fm.setCocomacModified();
      }
      else if (specFileTag == SpecFile::getContourFileTag()) {
         theMainWindow->getBrainSet()->readContourFile(name, append, update);
         fm.setContourModified();
      }
      else if (specFileTag == SpecFile::getContourCellFileTag()) {
         theMainWindow->getBrainSet()->readContourCellFile(name, append, update);
         fm.setContourCellModified();
         contourCellColorWarning = needContourCellColors;
      }
      else if (specFileTag == SpecFile::getContourCellColorFileTag()) {
         theMainWindow->getBrainSet()->readContourCellColorFile(name, append, update);
         fm.setContourCellModified();
      }
      else if (specFileTag == SpecFile::getRawCoordFileTag()) {
         theMainWindow->getBrainSet()->readCoordinateFile(name, BrainModelSurface::SURFACE_TYPE_RAW, false, append, update);
         fm.setCoordinateModified();
      }
      else if (specFileTag == SpecFile::getFiducialCoordFileTag()) {
         theMainWindow->getBrainSet()->readCoordinateFile(name, BrainModelSurface::SURFACE_TYPE_FIDUCIAL, false, append, update);
         fm.setCoordinateModified();
      }
      else if (specFileTag == SpecFile::getInflatedCoordFileTag()) {
         theMainWindow->getBrainSet()->readCoordinateFile(name, BrainModelSurface::SURFACE_TYPE_INFLATED, false, append, update);
         fm.setCoordinateModified();
      }
      else if (specFileTag == SpecFile::getVeryInflatedCoordFileTag()) {
         theMainWindow->getBrainSet()->readCoordinateFile(name, BrainModelSurface::SURFACE_TYPE_VERY_INFLATED, false, append, update);
         fm.setCoordinateModified();
      }
      else if (specFileTag == SpecFile::getSphericalCoordFileTag()) {
         theMainWindow->getBrainSet()->readCoordinateFile(name, BrainModelSurface::SURFACE_TYPE_SPHERICAL, false, append, update);
         fm.setCoordinateModified();
      }
      else if (specFileTag == SpecFile::getEllipsoidCoordFileTag()) {
         theMainWindow->getBrainSet()->readCoordinateFile(name, BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL, false, append, update);
         fm.setCoordinateModified();
      }
      else if (specFileTag == SpecFile::getCompressedCoordFileTag()) {
         theMainWindow->getBrainSet()->readCoordinateFile(name, BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL, false, append, update);
         fm.setCoordinateModified();
      }
      else if (specFileTag == SpecFile::getFlatCoordFileTag()) {
         theMainWindow->getBrainSet()->readCoordinateFile(name, BrainModelSurface::SURFACE_TYPE_FLAT, false, append, update);
         fm.setCoordinateModified();
      }
      else if (specFileTag == SpecFile::getLobarFlatCoordFileTag()) {
         theMainWindow->getBrainSet()->readCoordinateFile(name, BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR, false, append, update);
         fm.setCoordinateModified();
      }
      else if (specFileTag == SpecFile::getHullCoordFileTag()) {
         theMainWindow->getBrainSet()->readCoordinateFile(name, BrainModelSurface::SURFACE_TYPE_HULL, false, append, update);
         fm.setCoordinateModified();
      }
      else if (specFileTag.indexOf(SpecFile::getUnknownCoordFileMatchTag()) >= 0) {
         theMainWindow->getBrainSet()->readCoordinateFile(name, BrainModelSurface::SURFACE_TYPE_UNKNOWN, false, append, update);
         fm.setCoordinateModified();
      }
      else if (specFileTag == GENERIC_COORDINATE) {
         CoordinateFile cf;
         try {
            cf.readFileMetaDataOnly(name);
         }
         catch (FileException& e) {
            throw FileException(name, "Unable to open coord file to check type.\n"
                                      + e.whatQString());
         }

         BrainModelSurface::SURFACE_TYPES st = BrainModelSurface::SURFACE_TYPE_UNSPECIFIED;
/*
         AbstractFileHeaderUpdater afhu;
         afhu.readFile(name);
         const QString typeTag = afhu.getHeaderTag(AbstractFile::headerTagConfigurationID);
*/
         const QString typeTag = cf.getHeaderTag(AbstractFile::headerTagConfigurationID);
         if (typeTag.isEmpty() == false) {
            st = BrainModelSurface::getSurfaceTypeFromConfigurationID(typeTag);
         }
         
         QApplication::restoreOverrideCursor();
         if ((st == BrainModelSurface::SURFACE_TYPE_UNSPECIFIED) ||
             (st == BrainModelSurface::SURFACE_TYPE_UNKNOWN)) {
            std::vector<QString> labels;
            labels.push_back("RAW");
            labels.push_back("FIDUCIAL");
            labels.push_back("INFLATED");
            labels.push_back("VERY_INFLATED");
            labels.push_back("SPHERICAL");
            labels.push_back("ELLIPSOIDAL");
            labels.push_back("COMPRESSED_MEDIAL_WALL");
            labels.push_back("FLAT");
            labels.push_back("FLAT_LOBAR");
            labels.push_back("HULL");
            QApplication::restoreOverrideCursor();
            QApplication::beep();
            QtRadioButtonSelectionDialog rbsd(parentWidget,
                                              "Choose Coordinate Type",
                                              "Type of Coordinates for " + nameNoPath,
                                              labels,
                                              -1);
            if (rbsd.exec() == QDialog::Accepted) {
               st = static_cast<BrainModelSurface::SURFACE_TYPES>(rbsd.getSelectedItemIndex());
            }
            else {
               throw FileException(name, "You must select the type of coordinate file.");
            }
         }
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         theMainWindow->getBrainSet()->readCoordinateFile(name, st, false, append, update);
         fm.setCoordinateModified();
         QApplication::restoreOverrideCursor();
      }
      else if (specFileTag == SpecFile::getCutsFileTag()) {
         theMainWindow->getBrainSet()->readCutsFile(name, append, update);
         fm.setCutModified();
      }
      else if (specFileTag == SpecFile::getDeformationFieldFileTag()) {
         theMainWindow->getBrainSet()->readDeformationFieldFile(name, append, update);
         fm.setDeformationFieldModified();
      }
      else if (specFileTag == SpecFile::getDeformationMapFileTag()) {
         theMainWindow->getBrainSet()->setDeformationMapFileName(name, update);
         fm.setDeformationMapModified();
      }
      else if (specFileTag == SpecFile::getFociFileTag()) {
         theMainWindow->getBrainSet()->readFociFile(name, append, update);
         fm.setFociModified();
         fociColorWarning = needFociColors;
      }
      else if (specFileTag == SpecFile::getFociColorFileTag()) {
         theMainWindow->getBrainSet()->readFociColorFile(name, append, update);
         fm.setFociColorModified();
      }
      else if (specFileTag == SpecFile::getFociProjectionFileTag()) {
         theMainWindow->getBrainSet()->readFociProjectionFile(name, append, update);
         fm.setFociProjectionModified();
         fociColorWarning = needFociColors;
      }
      else if (specFileTag == SpecFile::getFociSearchFileTag()) {
         theMainWindow->getBrainSet()->readFociSearchFile(name, append, update);
         fm.setFociSearchModified();
      }
      else if (specFileTag == SpecFile::getGeodesicDistanceFileTag()) {
         GeodesicDistanceFile* gdf = theMainWindow->getBrainSet()->getGeodesicDistanceFile();
         GeodesicDistanceFile newFile;
         newFile.readFileMetaDataOnly(name);
         QApplication::restoreOverrideCursor();
         GuiLoadNodeAttributeFileColumnSelectionDialog fcsd(theMainWindow,
                                                      &newFile,
                                                      gdf);
         const int result = fcsd.exec(); 
         newFile.clear(); // frees memory
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         if (result == QDialog::Accepted) {
            if (fcsd.getEraseAllExistingColumns()) {
               theMainWindow->getBrainSet()->clearGeodesicDistanceFile();
            }
            theMainWindow->getBrainSet()->readGeodesicDistanceFile(name, 
                                 fcsd.getDestinationColumns(), 
                                 fcsd.getNewFileColumnNames(),
                                 fcsd.getAppendFileCommentSelection(),
                                 update);
            fm.setGeodesicModified();
         }
      }
      else if (specFileTag == SpecFile::getImageFileTag()) {
         theMainWindow->getBrainSet()->readImageFile(name, 
                              append, 
                              update);
         fm.setImagesModified();
      }
      else if (specFileTag == SpecFile::getLatLonFileTag()) {
         LatLonFile* llf = theMainWindow->getBrainSet()->getLatLonFile();
         LatLonFile newFile;
         newFile.readFileMetaDataOnly(name);
         QApplication::restoreOverrideCursor();
         GuiLoadNodeAttributeFileColumnSelectionDialog fcsd(theMainWindow,
                                                      &newFile,
                                                      llf);
         const int result = fcsd.exec(); 
         newFile.clear(); // frees memory
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         if (result == QDialog::Accepted) {
            if (fcsd.getEraseAllExistingColumns()) {
               theMainWindow->getBrainSet()->clearLatLonFile();
            }
            theMainWindow->getBrainSet()->readLatLonFile(name, 
                                 fcsd.getDestinationColumns(), 
                                 fcsd.getNewFileColumnNames(),
                                 fcsd.getAppendFileCommentSelection(),
                                 update);
            fm.setLatLonModified();
         }
      }
      else if (specFileTag == SpecFile::getMetricFileTag()) {
         MetricFile* mf = theMainWindow->getBrainSet()->getMetricFile();
         MetricFile newFile;
         newFile.readFileMetaDataOnly(name);
         QApplication::restoreOverrideCursor();
         GuiLoadNodeAttributeFileColumnSelectionDialog fcsd(theMainWindow,
                                                      &newFile,
                                                      mf);
         const int result = fcsd.exec(); 
         newFile.clear(); // frees memory
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         if (result == QDialog::Accepted) {
            if (fcsd.getEraseAllExistingColumns()) {
               theMainWindow->getBrainSet()->clearMetricFile();
            }
            theMainWindow->getBrainSet()->readMetricFile(name, 
                                 fcsd.getDestinationColumns(), 
                                 fcsd.getNewFileColumnNames(),
                                 fcsd.getAppendFileCommentSelection(),
                                 update);
            fm.setMetricModified();
         }
      }
      else if (specFileTag == SpecFile::getPaintFileTag()) {
         PaintFile* pf = theMainWindow->getBrainSet()->getPaintFile();
         PaintFile newFile;
         newFile.readFileMetaDataOnly(name);
         QApplication::restoreOverrideCursor();
         GuiLoadNodeAttributeFileColumnSelectionDialog fcsd(theMainWindow,
                                                      &newFile,
                                                      pf);
         const int result = fcsd.exec(); 
         newFile.clear(); // frees memory
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         if (result == QDialog::Accepted) {
            if (fcsd.getEraseAllExistingColumns()) {
               theMainWindow->getBrainSet()->clearPaintFile();
            }
            theMainWindow->getBrainSet()->readPaintFile(name, 
                                 fcsd.getDestinationColumns(), 
                                 fcsd.getNewFileColumnNames(),
                                 fcsd.getAppendFileCommentSelection(),
                                 update);
            fm.setPaintModified();
         }
         areaColorWarning = needAreaColors;
      }
      else if (specFileTag == SpecFile::getPaletteFileTag()) {
         theMainWindow->getBrainSet()->readPaletteFile(name, append, update);
         fm.setPaletteModified();
      }
      else if (specFileTag == SpecFile::getParamsFileTag()) {
         theMainWindow->getBrainSet()->readParamsFile(name, append, update);
         fm.setParameterModified();
      }
      else if (specFileTag == SpecFile::getAtlasFileTag()) {
         theMainWindow->getBrainSet()->readProbabilisticAtlasFile(name, append, update);
         fm.setProbabilisticAtlasModified();
         areaColorWarning = needAreaColors;
      }
      else if (specFileTag == SpecFile::getRgbPaintFileTag()) {
         theMainWindow->getBrainSet()->readRgbPaintFile(name, append, update);
         fm.setRgbPaintModified();
      }
      else if (specFileTag == SpecFile::getSceneFileTag()) {
         theMainWindow->getBrainSet()->readSceneFile(name, append, update);
         fm.setSceneModified();
      }
      else if (specFileTag == SpecFile::getSectionFileTag()) {
         theMainWindow->getBrainSet()->readSectionFile(name, append, update);
         fm.setSectionModified();
      }
      else if (specFileTag == SpecFile::getRawSurfaceFileTag()) {
         theMainWindow->getBrainSet()->readSurfaceFile(name, BrainModelSurface::SURFACE_TYPE_RAW, false, append, update);
         fm.setCoordinateModified();
         fm.setTopologyModified();
      }
      else if (specFileTag == SpecFile::getFiducialSurfaceFileTag()) {
         theMainWindow->getBrainSet()->readSurfaceFile(name, BrainModelSurface::SURFACE_TYPE_FIDUCIAL, false, append, update);
         fm.setCoordinateModified();
         fm.setTopologyModified();
      }
      else if (specFileTag == SpecFile::getInflatedSurfaceFileTag()) {
         theMainWindow->getBrainSet()->readSurfaceFile(name, BrainModelSurface::SURFACE_TYPE_INFLATED, false, append, update);
         fm.setCoordinateModified();
         fm.setTopologyModified();
      }
      else if (specFileTag == SpecFile::getVeryInflatedSurfaceFileTag()) {
         theMainWindow->getBrainSet()->readSurfaceFile(name, BrainModelSurface::SURFACE_TYPE_VERY_INFLATED, false, append, update);
         fm.setCoordinateModified();
         fm.setTopologyModified();
      }
      else if (specFileTag == SpecFile::getSphericalSurfaceFileTag()) {
         theMainWindow->getBrainSet()->readSurfaceFile(name, BrainModelSurface::SURFACE_TYPE_SPHERICAL, false, append, update);
         fm.setCoordinateModified();
         fm.setTopologyModified();
      }
      else if (specFileTag == SpecFile::getEllipsoidSurfaceFileTag()) {
         theMainWindow->getBrainSet()->readSurfaceFile(name, BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL, false, append, update);
         fm.setCoordinateModified();
         fm.setTopologyModified();
      }
      else if (specFileTag == SpecFile::getCompressedSurfaceFileTag()) {
         theMainWindow->getBrainSet()->readSurfaceFile(name, BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL, false, append, update);
         fm.setCoordinateModified();
         fm.setTopologyModified();
      }
      else if (specFileTag == SpecFile::getFlatSurfaceFileTag()) {
         theMainWindow->getBrainSet()->readSurfaceFile(name, BrainModelSurface::SURFACE_TYPE_FLAT, false, append, update);
         fm.setCoordinateModified();
         fm.setTopologyModified();
      }
      else if (specFileTag == SpecFile::getLobarFlatSurfaceFileTag()) {
         theMainWindow->getBrainSet()->readSurfaceFile(name, BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR, false, append, update);
         fm.setCoordinateModified();
         fm.setTopologyModified();
      }
      else if (specFileTag == SpecFile::getHullSurfaceFileTag()) {
         theMainWindow->getBrainSet()->readSurfaceFile(name, BrainModelSurface::SURFACE_TYPE_HULL, false, append, update);
         fm.setCoordinateModified();
         fm.setTopologyModified();
      }
      else if (specFileTag.indexOf(SpecFile::getUnknownSurfaceFileMatchTag()) >= 0) {
         theMainWindow->getBrainSet()->readSurfaceFile(name, BrainModelSurface::SURFACE_TYPE_UNKNOWN, false, append, update);
         fm.setCoordinateModified();
         fm.setTopologyModified();
      }
      else if (specFileTag == GENERIC_SURFACE) {
         SurfaceFile sf;
         try {
            sf.readFileMetaDataOnly(name);
         }
         catch (FileException& e) {
            throw FileException(name, "Unable to open surface file to check type.\n"
                                      + e.whatQString());
         }

         BrainModelSurface::SURFACE_TYPES st = BrainModelSurface::SURFACE_TYPE_UNSPECIFIED;
         const QString typeTag = sf.getHeaderTag(AbstractFile::headerTagConfigurationID);
         if (typeTag.isEmpty() == false) {
            st = BrainModelSurface::getSurfaceTypeFromConfigurationID(typeTag);
         }
         
         if ((st == BrainModelSurface::SURFACE_TYPE_UNSPECIFIED) ||
             (st == BrainModelSurface::SURFACE_TYPE_UNKNOWN)) {
            std::vector<QString> labels;
            labels.push_back("RAW");
            labels.push_back("FIDUCIAL");
            labels.push_back("INFLATED");
            labels.push_back("VERY_INFLATED");
            labels.push_back("SPHERICAL");
            labels.push_back("ELLIPSOIDAL");
            labels.push_back("COMPRESSED_MEDIAL_WALL");
            labels.push_back("FLAT");
            labels.push_back("FLAT_LOBAR");
            labels.push_back("HULL");
            QApplication::restoreOverrideCursor();
            QApplication::beep();
            QtRadioButtonSelectionDialog rbsd(parentWidget,
                                              "Choose Surface Type",
                                              "Type of Surface for " + nameNoPath,
                                              labels,
                                              -1);
            if (rbsd.exec() == QDialog::Accepted) {
               st = static_cast<BrainModelSurface::SURFACE_TYPES>(rbsd.getSelectedItemIndex());
            }
            else {
               throw FileException(name, "You must select the type of surface file.");
            }
         }
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         theMainWindow->getBrainSet()->readSurfaceFile(name, st, false, append, update);
         fm.setCoordinateModified();
         fm.setTopologyModified();
         QApplication::restoreOverrideCursor();
      }
      else if (specFileTag == SpecFile::getStudyCollectionFileTag()) {
         theMainWindow->getBrainSet()->readStudyCollectionFile(name, append, update);
         fm.setStudyCollectionModified();
      }
      else if (specFileTag == SpecFile::getStudyMetaDataFileTag()) {
         theMainWindow->getBrainSet()->readStudyMetaDataFile(name, append, update);
         fm.setStudyMetaDataModified();
      }
      else if (specFileTag == SpecFile::getSurfaceShapeFileTag()) {
         SurfaceShapeFile* ssf = theMainWindow->getBrainSet()->getSurfaceShapeFile();
         SurfaceShapeFile newFile;
         newFile.readFileMetaDataOnly(name);
         QApplication::restoreOverrideCursor();
         GuiLoadNodeAttributeFileColumnSelectionDialog fcsd(theMainWindow,
                                                      &newFile,
                                                      ssf);
         const int result = fcsd.exec(); 
         newFile.clear(); // frees memory
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         if (result == QDialog::Accepted) {
            if (fcsd.getEraseAllExistingColumns()) {
               theMainWindow->getBrainSet()->clearSurfaceShapeFile();
            }
            theMainWindow->getBrainSet()->readSurfaceShapeFile(name, 
                                 fcsd.getDestinationColumns(), 
                                 fcsd.getNewFileColumnNames(),
                                 fcsd.getAppendFileCommentSelection(),
                                 update);
            fm.setSurfaceShapeModified();
         }
      }
      else if (specFileTag == SpecFile::getVectorFileTag()) {
         theMainWindow->getBrainSet()->readVectorFile(name, append, update);
         fm.setVectorModified();
      }
      else if (specFileTag == SpecFile::getTopographyFileTag()) {
         theMainWindow->getBrainSet()->readTopographyFile(name, append, update);
         fm.setTopographyModified();
      }
      else if (specFileTag == SpecFile::getClosedTopoFileTag()) {
         theMainWindow->getBrainSet()->readTopologyFile(name, TopologyFile::TOPOLOGY_TYPE_CLOSED, append, update);
         fm.setTopologyModified();
      }
      else if (specFileTag == SpecFile::getOpenTopoFileTag()) {
         theMainWindow->getBrainSet()->readTopologyFile(name, TopologyFile::TOPOLOGY_TYPE_OPEN, append, update);
         fm.setTopologyModified();
      }
      else if (specFileTag == SpecFile::getCutTopoFileTag()) {
         theMainWindow->getBrainSet()->readTopologyFile(name, TopologyFile::TOPOLOGY_TYPE_CUT, append, update);
         fm.setTopologyModified();
      }
      else if (specFileTag == SpecFile::getLobarCutTopoFileTag()) {
         theMainWindow->getBrainSet()->readTopologyFile(name, TopologyFile::TOPOLOGY_TYPE_LOBAR_CUT, append, update);
         fm.setTopologyModified();
      }
      else if (specFileTag.indexOf(SpecFile::getUnknownTopoFileMatchTag()) >= 0) {
         theMainWindow->getBrainSet()->readTopologyFile(name, TopologyFile::TOPOLOGY_TYPE_UNKNOWN, append, update);
         fm.setTopologyModified();
      }
      else if (specFileTag == GENERIC_TOPOLOGY) {
         TopologyFile::TOPOLOGY_TYPES tt = TopologyFile::TOPOLOGY_TYPE_UNKNOWN;
         TopologyFile tf;
         try {
            tf.readFileMetaDataOnly(name);
         }
         catch (FileException& e) {
            throw FileException(name, "Unable to read topology file to check type.\n"
                                      + e.whatQString());
         }
/*
         AbstractFileHeaderUpdater afhu;
         afhu.readFile(name);
         const QString typeTag = afhu.getHeaderTag(AbstractFile::headerTagPerimeterID);
*/
         const QString typeTag = tf.getHeaderTag(AbstractFile::headerTagPerimeterID);
         if (typeTag.isEmpty() == false) {
            tt = TopologyFile::getTopologyTypeFromPerimeterID(typeTag);
         }
         
         if ((tt == TopologyFile::TOPOLOGY_TYPE_UNKNOWN) ||
             (tt == TopologyFile::TOPOLOGY_TYPE_UNSPECIFIED)) {
            std::vector<QString> labels;
            labels.push_back("CLOSED");
            labels.push_back("OPEN");
            labels.push_back("CUT");
            labels.push_back("LOBAR_CUT");
            QApplication::restoreOverrideCursor();
            QApplication::beep();
            QtRadioButtonSelectionDialog rbsd(parentWidget,
                                              "Choose Topology Type",
                                              "Type of Topology for " + nameNoPath,
                                              labels,
                                              -1);
            if (rbsd.exec() == QDialog::Accepted) {
               tt = static_cast<TopologyFile::TOPOLOGY_TYPES>(rbsd.getSelectedItemIndex());
            }
            else {
               throw FileException(name, "You must select the type of topology.");
            }
         }
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         theMainWindow->getBrainSet()->readTopologyFile(name, tt, append, update);
         fm.setTopologyModified();
         QApplication::restoreOverrideCursor();
      }
      else if (specFileTag == SpecFile::getTrajectoryFileTag()) {
         theMainWindow->getBrainSet()->readTrajectoryFile(name);
         fm.setTrajectoryModified();
      }
      else if (specFileTag == SpecFile::getTransformationMatrixFileTag()) {
         theMainWindow->getBrainSet()->readTransformationMatrixFile(name, append, update);
         fm.setTransformationMatrixModified();
      }
      else if (specFileTag == SpecFile::getTransformationDataFileTag()) {
         theMainWindow->getBrainSet()->readTransformationDataFile(name, append, update);
         theMainWindow->getBrainSet()->assignTransformationDataFileColors();
         fm.setTransformationDataModified();
      }
      else if (specFileTag == SpecFile::getVolumeAnatomyFileTag()) {
         theMainWindow->getBrainSet()->readVolumeFile(name, VolumeFile::VOLUME_TYPE_ANATOMY, append, update);
         fm.setVolumeModified();
      }
      else if (specFileTag == SpecFile::getVolumeFunctionalFileTag()) {
         theMainWindow->getBrainSet()->readVolumeFile(name, VolumeFile::VOLUME_TYPE_FUNCTIONAL, append, update);
         fm.setVolumeModified();
      }
      else if (specFileTag == SpecFile::getVolumePaintFileTag()) {
         theMainWindow->getBrainSet()->readVolumeFile(name, VolumeFile::VOLUME_TYPE_PAINT, append, update);
         fm.setVolumeModified();
         areaColorWarning = needAreaColors;
      }
      else if (specFileTag == SpecFile::getVolumeProbAtlasFileTag()) {
         theMainWindow->getBrainSet()->readVolumeFile(name, VolumeFile::VOLUME_TYPE_PROB_ATLAS, append, update);
         fm.setVolumeModified();
         areaColorWarning = needAreaColors;
      }
      else if (specFileTag == SpecFile::getVolumeRgbFileTag()) {
         theMainWindow->getBrainSet()->readVolumeFile(name, VolumeFile::VOLUME_TYPE_RGB, append, update);
         fm.setVolumeModified();
      }
      else if (specFileTag == SpecFile::getVolumeSegmentationFileTag()) {
         theMainWindow->getBrainSet()->readVolumeFile(name, VolumeFile::VOLUME_TYPE_SEGMENTATION, append, update);
         fm.setVolumeModified();
      }
      else if (specFileTag == SpecFile::getVolumeVectorFileTag()) {
         theMainWindow->getBrainSet()->readVolumeFile(name, VolumeFile::VOLUME_TYPE_VECTOR, append, update);
         fm.setVolumeModified();
      }
      else if (specFileTag == SpecFile::getVtkModelFileTag()) {
         theMainWindow->getBrainSet()->readVtkModelFile(name, append, update);
         fm.setVtkModelModified();
      }
      else if (specFileTag == SpecFile::getVocabularyFileTag()) {
         theMainWindow->getBrainSet()->readVocabularyFile(name, append, update);
         fm.setVocabularyModified();
      }
      else if (specFileTag == SpecFile::getWustlRegionFileTag()) {
         theMainWindow->getBrainSet()->readWustlRegionFile(name, append, update);
         fm.setWustlRegionModified();
      }
      else {
         std::cerr << "PROGRAM ERROR: unhandled file type " << specFileTag.toAscii().constData() << " at " << __LINE__
                  << " in " << __FILE__ << std::endl;
      }
   }
   catch (FileException& e) {
      errorMessage = e.whatQString();
      return true;
   }
   
   if (bordersLoaded) {
      DisplaySettingsBorders* dsb = theMainWindow->getBrainSet()->getDisplaySettingsBorders();
      if (dsb->getDisplayBorders() == false) {
         dsb->setDisplayBorders(true);
         theMainWindow->updateDisplayControlDialog();
      }
   }
   fm.setInhibitSurfaceDefaultScaling();
   theMainWindow->fileModificationUpdate(fm);
   if (fm.getCoordinateModified()) {
      theMainWindow->displayNewestSurfaceInMainWindow();
   }
   
   GuiBrainModelOpenGL::updateAllGL(NULL);
   
   QApplication::restoreOverrideCursor();
   if (areaColorWarning) {
      QMessageBox::warning(theMainWindow, "Warning",
                           "The file just opened requires area colors\n"
                           "but there are no area colors loaded.");
      relatedFileWarningFlag = true;
   }
   
   if (borderColorWarning) {
      QMessageBox::warning(theMainWindow, "Warning",
                           "The file just opened requires border colors\n"
                           "but there are no border colors loaded.");
      relatedFileWarningFlag = true;
   }
   
   if (cellColorWarning) {
      QMessageBox::warning(theMainWindow, "Warning",
                           "The file just opened requires cell colors\n"
                           "but there are no cell colors loaded.");
      relatedFileWarningFlag = true;
   }
   
   if (contourCellColorWarning) {
      QMessageBox::warning(theMainWindow, "Warning",
                           "The file just opened requires contour cell colors\n"
                           "but there are no contour cell colors loaded.");
      relatedFileWarningFlag = true;
   }
   
   if (fociColorWarning) {
      QMessageBox::warning(theMainWindow, "Warning",
                           "The file just opened requires foci colors\n"
                           "but there are no foci colors loaded.");
      relatedFileWarningFlag = true;
   }
   
   return false;
}

/**
 * copy a file to the current directory, returns true if successful.
 */
bool 
GuiDataFileOpenDialog::copyFile(const QString& name)
{
   const QString nameNoExt(FileUtilities::filenameWithoutExtension(name));
   QString volumeDataFileName;
   if (name.endsWith(SpecFile::getAnalyzeVolumeFileExtension())) {
      volumeDataFileName = nameNoExt + ".img";
   }
   else if (name.endsWith(SpecFile::getAfniVolumeFileExtension())) {
      volumeDataFileName = nameNoExt + ".BRIK";
   }
   else if (name.endsWith(SpecFile::getWustlVolumeFileExtension())) {
      volumeDataFileName = nameNoExt + ".img";
   }
   if (volumeDataFileName.isEmpty() == false) {
      if (QFile::exists(volumeDataFileName) == false) {
         volumeDataFileName += ".gz";
      }
   }
   
   if (FileUtilities::copyFile(name, FileUtilities::basename(name))) {
      QString msg("Error copying:\n");
      msg.append(name);
      msg.append("\nDoes the file exist ?");
      QMessageBox::critical(this, "ERROR", msg);
      return false;      
   }
   
   if (volumeDataFileName.isEmpty() == false) {
      if (FileUtilities::copyFile(volumeDataFileName,
                           FileUtilities::basename(volumeDataFileName))) {
         QString msg("Error copying:\n");
         msg.append(volumeDataFileName);
         msg.append("\nDoes the file exist ?");
         QMessageBox::critical(this, "ERROR", msg);
         return false;
      }
   }
   
   return true;
}      
      
      
