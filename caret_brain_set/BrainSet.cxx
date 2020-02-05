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


#include <QGlobalStatic>
#include <QMutexLocker>
#include <QTextStream>
#include <QTimer>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QGLWidget>
#include <QProgressDialog>

#include "vtkActor.h"
#include "vtkMath.h"
#include "vtkPolyDataMapper.h"
#include "vtkPointData.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkVRMLExporter.h"

#define __BRAIN_SET_MAIN__
#include "BrainSet.h"
#undef __BRAIN_SET_MAIN__

#include "AreaColorFile.h"
#include "ArealEstimationFile.h"
#include "AtlasSurfaceDirectoryFile.h"
#include "BorderColorFile.h"
#include "BorderFile.h"
#include "BorderProjectionFile.h"
#include "BorderProjectionUnprojector.h"
#include "BrainModelBorderSet.h"
#include "BrainModelContours.h"
#include "BrainModelIdentification.h"
#include "BrainModelSurfaceAndVolume.h"
#include "BrainModelSurfaceCurvature.h"
#include "BrainModelSurfaceNodeColoring.h"
#include "BrainModelSurfaceROINodeSelection.h"
#include "BrainModelSurfaceResection.h"
#include "BrainModelVolume.h"
#include "BrainModelVolumeToSurfaceConverter.h"
#include "BrainModelVolumeVoxelColoring.h"
#include "BrainModelVolumeRegionOfInterest.h"
#include "BrainSetMultiThreadedSpecFileReader.h"
#include "BrainSetAutoLoaderManager.h"
#include "BrainVoyagerFile.h"
#include "CellColorFile.h"
#include "CellFile.h"
#include "CellProjectionFile.h"
#include "CellProjectionUnprojector.h"
#include "CocomacConnectivityFile.h"
#include "ColorFile.h"
#include "ContourCellColorFile.h"
#include "ContourCellFile.h"
#include "ContourFile.h"
#include "CutsFile.h"
#include "DebugControl.h"
#include "DeformationFieldFile.h"
#include "DeformationMapFile.h"
#include "DisplaySettingsArealEstimation.h"
#include "DisplaySettingsBorders.h"
#include "DisplaySettingsCells.h"
#include "DisplaySettingsContours.h"
#include "DisplaySettingsCuts.h"
#include "DisplaySettingsCoCoMac.h"
#include "DisplaySettingsDeformationField.h"
#include "DisplaySettingsFoci.h"
#include "DisplaySettingsGeodesicDistance.h"
#include "DisplaySettingsImages.h"
#include "DisplaySettingsMetric.h"
#include "DisplaySettingsModels.h"
#include "DisplaySettingsSection.h"
#include "DisplaySettingsSurface.h"
#include "DisplaySettingsPaint.h"
#include "DisplaySettingsProbabilisticAtlas.h"
#include "DisplaySettingsRgbPaint.h"
#include "DisplaySettingsScene.h"
#include "DisplaySettingsStudyMetaData.h"
#include "DisplaySettingsSurfaceShape.h"
#include "DisplaySettingsVectors.h"
#include "DisplaySettingsTopography.h"
#include "DisplaySettingsVolume.h"
#include "DisplaySettingsWustlRegion.h"
#include "FileUtilities.h"
#include "FociColorFile.h"
#include "FociFile.h"
#include "FociProjectionFile.h"
#include "FociSearchFile.h"
#include "GeodesicDistanceFile.h"
#include "ImageFile.h"
#include "LatLonFile.h"
#include "MDPlotFile.h"
#include "MetricFile.h"
#include "MniObjSurfaceFile.h"
#include "NeurolucidaFile.h"
#include "PaintFile.h"
#include "PaletteFile.h"
#include "ParamsFile.h"
#include "ProbabilisticAtlasFile.h"
#include "RgbPaintFile.h"
#include "SceneFile.h"
#include "SectionFile.h"
#include "TrajectoryFile.h"
#include "StatisticRandomNumber.h"
#include "StringUtilities.h"
#include "StudyCollectionFile.h"
#include "StudyMetaDataFile.h"
#include "SurfaceShapeFile.h"
#include "TopographyFile.h"
#include "TopologyFile.h"
#include "TopologyHelper.h"
#include "TransformationMatrixFile.h"
#include "VectorFile.h"
#include "VocabularyFile.h"
#include "VtkModelFile.h"
#include "WustlRegionFile.h"

#include "vtkBYUReader.h"
#include "vtkBYUWriter.h"
#include "vtkFieldData.h"
#include "vtkIVWriter.h"
#include "vtkLight.h"
#include "vtkOBJExporter.h"
#include "vtkPolyDataNormals.h"
#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"
#include "vtkSTLReader.h"
#include "vtkSTLWriter.h"
#include "vtkUnsignedCharArray.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"

/**
 * The Constructor.
 */
BrainSet::BrainSet(const bool primaryBrainSetFlagIn)
{
   constructBrainSet();
   primaryBrainSetFlag = primaryBrainSetFlagIn;
}

/**
 * Construct a brain set from a vtk surface file.
 */
BrainSet::BrainSet(const QString& vtkSurfaceFileName,
                   const BrainModelSurface::SURFACE_TYPES surfaceType,
                   const TopologyFile::TOPOLOGY_TYPES topologyType)
{
   constructBrainSet();
   
   try {
      importVtkSurfaceFile(vtkSurfaceFileName,
                           true,   // import coords
                           true,   // import topology
                           true,   // import colors
                           surfaceType,
                           topologyType);
   }
   catch (FileException& e) {
      std::cout << "Error constructing brain set from VTK file: "
                << e.whatQString().toAscii().constData() << std::endl;
   }
}
      
/**
 * Construct a brain set from a topology file and a coordinate file.
 * For success, check to see if there is one brain model surface.
 */
BrainSet::BrainSet(const QString& topoFileName,
                   const QString& coordFileName1,
                   const QString& coordFileName2,
                   const bool primaryBrainSetFlagIn)
{
   constructBrainSet();
   primaryBrainSetFlag = primaryBrainSetFlagIn;
   
   //
   // Create a spec file with the two files
   //
   SpecFile sf;
   std::vector<QString> coordFileNames;
   coordFileNames.push_back(coordFileName1);
   coordFileNames.push_back(coordFileName2);
   sf.setTopoAndCoordSelected(topoFileName, 
                              coordFileNames,
                              getStructure());
   
   //
   // Read the spec file
   //
   std::vector<QString> messages;
   readSpecFile(SPEC_FILE_READ_MODE_NORMAL, sf, "", messages, NULL, NULL);
   if (messages.empty() == false) {
      for (unsigned int i = 0; i < messages.size(); i++) {
         std::cout << "BrainSet construction error: "
                   << messages[i].toAscii().constData()
                   << std::endl;
      }
   }
   
   //
   // As a result of this call, the block of code below is probably unnecessary
   //
   setIgnoreTopologyFileInCoordinateFileHeaderFlag(true);
   
   //
   // Some coordinate files have a topology file name listed in the header
   // which causes the topology file to be loaded and assigned to the 
   // coordinate file.  So, force the topology file to the file passed to
   // this constructor.
   //
   if ((getNumberOfBrainModels() > 0) &&
       (getNumberOfTopologyFiles() > 0)) {
      //
      // loop through topology files
      //
      for (int i = 0; i < getNumberOfTopologyFiles(); i++) {
         //
         // Get the topology file and its name
         //
         TopologyFile* topoFile = getTopologyFile(i);
         const QString topoName = topoFile->getFileName();
         
         //
         // Is this the topology file that was passed to this constructor?
         //
         if (FileUtilities::basename(topoName) == FileUtilities::basename(topoFileName)) {
            //
            // Make all surfaces use the topology file
            //
            for (int j = 0; j < getNumberOfBrainModels(); j++) {
               BrainModelSurface* bms = getBrainModelSurface(j);
               if (bms != NULL) {
                  bms->setTopologyFile(topoFile);
               }
            }
            
            break;
         }
      }
   }
   
   //
   // Structure needs to be set
   //
   for (int i = 0; i < getNumberOfBrainModels(); i++) {
      const BrainModelSurface* bms = getBrainModelSurface(i);
      if (bms != NULL) {
         if (bms->getStructure().getType() != Structure::STRUCTURE_TYPE_INVALID) {
            setStructure(bms->getStructure());
            break;
         }
      }
   }
   
   updateDefaultFileNamePrefix();
}               

/**
 * Construct a brain set from all files in a spec file.
 * For success, check to see if there is one brain model surface.
 */
BrainSet::BrainSet(const QString& specFileNameIn, 
                   const bool readAllFilesInSpecFile,
                   const bool primaryBrainSetFlagIn)
{
   constructBrainSet();
   primaryBrainSetFlag = primaryBrainSetFlagIn;
   
   //
   // Create and read spec file
   //
   SpecFile sf;
   try {
      sf.readFile(specFileNameIn);
   }
   catch (FileException&) {
      //return;
   }
   if (readAllFilesInSpecFile) {
      sf.setAllFileSelections(SpecFile::SPEC_TRUE);
   }
   else {
      sf.setAllFileSelections(SpecFile::SPEC_FALSE);
   }
   
   //
   // Read the spec file
   //
   std::vector<QString> messages;
   readSpecFile(SPEC_FILE_READ_MODE_NORMAL, sf, specFileNameIn, messages, NULL, NULL);
}               

/**
 * construct the brain set.
 */
void 
BrainSet::constructBrainSet()
{
   specFileTimeOfLoading = QDateTime::currentDateTime();
   
   webCaretFlag = false;
   readingSpecFileFlag = false;
   ignoreTopologyFileInCoordinateFileHeaderFlag = false;
   numberOfSurfaceOverlays = 4;
   
   initializeStaticStuff();
   
   primaryBrainSetFlag = false;

   progressDialogParent = NULL;
   
   numNodesMessage = "Contains different number of nodes than ";   
   
   areaColorFile          = new AreaColorFile;
   arealEstimationFile    = new ArealEstimationFile;
   borderColorFile        = new BorderColorFile;
   cellColorFile          = new CellColorFile;
   cellProjectionFile     = new CellProjectionFile;
   volumeCellFile         = new CellFile;
   cocomacFile            = new CocomacConnectivityFile;
   contourCellFile        = new ContourCellFile;
   contourCellColorFile   = new ContourCellColorFile;
   cutsFile               = new CutsFile;
   deformationFieldFile   = new DeformationFieldFile;
   fociColorFile          = new FociColorFile;
   fociProjectionFile     = new FociProjectionFile;
   fociSearchFile         = new FociSearchFile;
   geodesicDistanceFile   = new GeodesicDistanceFile;
   latLonFile             = new LatLonFile;
   metricFile             = new MetricFile;
   probabilisticAtlasSurfaceFile = new ProbabilisticAtlasFile;
   paintFile              = new PaintFile;
   paletteFile            = new PaletteFile;
   paramsFile             = new ParamsFile;
   rgbPaintFile           = new RgbPaintFile;
   sceneFile              = new SceneFile;
   sectionFile            = new SectionFile;
   studyCollectionFile    = new StudyCollectionFile;
   studyMetaDataFile      = new StudyMetaDataFile;
   surfaceShapeFile       = new SurfaceShapeFile;
   topographyFile         = new TopographyFile;
   trajectoryFile         = new TrajectoryFile;
   transformationMatrixFile = new TransformationMatrixFile;
   vocabularyFile         = new VocabularyFile;
   wustlRegionFile        = new WustlRegionFile;
      
   brainModelBorderSet    = new BrainModelBorderSet(this);
   
   brainModelIdentification       = new BrainModelIdentification(this);
   brainSetAutoLoaderManager     = new BrainSetAutoLoaderManager(this);

   nodeColoring                   = new BrainModelSurfaceNodeColoring(this);
   voxelColoring                  = new BrainModelVolumeVoxelColoring(this);
   brainModelSurfaceRegionOfInterestNodeSelection = new BrainModelSurfaceROINodeSelection(this);
   brainModelVolumeRegionOfInterest = new BrainModelVolumeRegionOfInterest(this);
   displaySettingsArealEstimation = new DisplaySettingsArealEstimation(this);
   displaySettingsBorders         = new DisplaySettingsBorders(this);
   displaySettingsContours	  = new DisplaySettingsContours(this);
   displaySettingsDeformationField = new DisplaySettingsDeformationField(this);
   displaySettingsGeodesicDistance = new DisplaySettingsGeodesicDistance(this);
   displaySettingsImages          = new DisplaySettingsImages(this);
   displaySettingsMetric          = new DisplaySettingsMetric(this);
   displaySettingsModels          = new DisplaySettingsModels(this);
   displaySettingsSection         = new DisplaySettingsSection(this);
   displaySettingsSurface         = new DisplaySettingsSurface(this);
   displaySettingsPaint           = new DisplaySettingsPaint(this);
   displaySettingsProbabilisticAtlasSurface = new DisplaySettingsProbabilisticAtlas(this,
                                 DisplaySettingsProbabilisticAtlas::PROBABILISTIC_TYPE_SURFACE);
   displaySettingsRgbPaint        = new DisplaySettingsRgbPaint(this);
   displaySettingsScene           = new DisplaySettingsScene(this);
   displaySettingsStudyMetaData   = new DisplaySettingsStudyMetaData(this);
   displaySettingsSurfaceShape    = new DisplaySettingsSurfaceShape(this);
   displaySettingsVectors         = new DisplaySettingsVectors(this);
   displaySettingsCells           = new DisplaySettingsCells(this);
   displaySettingsCoCoMac         = new DisplaySettingsCoCoMac(this);
   displaySettingsCuts            = new DisplaySettingsCuts(this);
   displaySettingsFoci            = new DisplaySettingsFoci(this);
   displaySettingsTopography      = new DisplaySettingsTopography(this);
   displaySettingsVolume          = new DisplaySettingsVolume(this);
   displaySettingsWustlRegion     = new DisplaySettingsWustlRegion(this);
   displaySettingsProbabilisticAtlasVolume = new DisplaySettingsProbabilisticAtlas(this,
                                 DisplaySettingsProbabilisticAtlas::PROBABILISTIC_TYPE_VOLUME);
   
   for (int i = 0; i < getNumberOfSurfaceOverlays(); i++) {
      surfaceOverlays.push_back(new BrainModelSurfaceOverlay(this, i));
   }
   
   reset();   
   
   //
   // load the splash image
   //
   QString imageName(getCaretHomeDirectory());
   imageName.append(QDir::separator());
   imageName.append("data_files"); 
   imageName.append(QDir::separator());
   imageName.append("images"); 
   imageName.append(QDir::separator());
   imageName.append("caret5"); 
   QString jpegImageName(imageName);
   jpegImageName.append(".jpg");
   displaySplashImage = readImage(jpegImageName, "JPEG", splashImage);
   if (displaySplashImage == false) {
      QString pngImageName(imageName);
      pngImageName.append(".png");
      displaySplashImage = readImage(pngImageName, "PNG", splashImage);
/*
      if (displaySplashImage == false) {
         QString msg("Unable to open splash image files ");
         msg.append(jpegImageName);
         msg.append(" or ");
         msg.append(pngImageName);
         msg.append(".  CARET5_HOME may not be set correctly.");
         std::cerr << msg << std::endl;
      }
*/
   }
   
   displayCrossTimer = new QTimer(this);
   QObject::connect(displayCrossTimer, SIGNAL(timeout()),
                    this, SLOT(slotDisplayCrossTimerTimeout()));
}

/**
 * called when cross timer timesout.
 */
void BrainSet::slotDisplayCrossTimerTimeout()
{
   setDisplayCrossForNode(-1, NULL);
   clearAllDisplayLists(); 
   emit signalGraphicsUpdate(this);
}
      
/**
 * set display cross for node.
 */
void 
BrainSet::setDisplayCrossForNode(const int node, BrainModelSurface* surface) 
{ 
   displayCrossForNode = node; 
   displayNoCrossForSurface = surface; 
   if (displayCrossTimer->isActive()) {
      displayCrossTimer->stop();
   }
   if (node >= 0) {
      displayCrossTimer->setSingleShot(true);
      displayCrossTimer->start(2000);
   }
   clearAllDisplayLists(); 
}
           
/**
 * get the name of the bin directory.
 */
QString 
BrainSet::getBinDirectoryName()
{
   QString binName("ERROR_BIN_DIRECTORY_IS_UNKNOWN");

   QString program = qApp->applicationDirPath();
   if (program.contains("macosx32")) {
      binName = "bin_macosx32";
   }
   else if (program.contains("macosx64")) {
      binName = "bin_macosx64";
   }
   else if (program.contains("windows32")) {
      binName = "bin_windows32";
   }
   else if (program.contains("windows64")) {
      binName = "bin_windows64";
   }
   else if (program.contains("linux32")) {
      binName = "bin_linux32";
   }
   else if (program.contains("linux64")) {
      binName = "bin_linux64";
   }
   else if (program.contains("linux_intel64")) {
      binName = "bin_linux_intel64";
   }
//#ifdef  Q_OS_WIN32
//   binName = "bin_windows";
//#endif
//#ifdef  Q_OS_FREEBSD
//   binName = "bin_freebsd";
//#endif
//#ifdef  Q_OS_UNIX
//   binName = "bin_linux";
//#endif
//#ifdef Q_OS_MACX
//   binName = "bin_macosx";
//#endif

   return binName;
}

/**
 * set the random seed.
 */
void 
BrainSet::setRandomSeed(unsigned int randomSeed)
{
   StatisticRandomNumber::setRandomSeed(randomSeed);
}      

/**
 * get caret's home directory.
 */
QString 
BrainSet::getCaretHomeDirectory() 
{ 
   static QString caretHomeDirectory;
   
   if (caretHomeDirectory.isEmpty()) {
      const char* caretHome = getenv("CARET5_HOME");
      if (caretHome != NULL) {
         caretHomeDirectory = caretHome;
      }
      else {
         caretHomeDirectory = qApp->applicationDirPath();
         if (caretHomeDirectory.isEmpty() == false) {
            caretHomeDirectory = FileUtilities::dirname(caretHomeDirectory);
#ifdef Q_OS_MACX
            const bool appFlag = (caretHomeDirectory.indexOf(".app/") > 0);
            if (appFlag) {
               caretHomeDirectory = FileUtilities::dirname(caretHomeDirectory);
               caretHomeDirectory = FileUtilities::dirname(caretHomeDirectory);
               caretHomeDirectory = FileUtilities::dirname(caretHomeDirectory);
            }
#endif
         }
      }

      if (DebugControl::getDebugOn()) {    
         std::cout << "Caret Home Directory: " << caretHomeDirectory.toAscii().constData() << std::endl;
      }
   }
   
   return caretHomeDirectory;
}
      
/**
 * get the preferences file.
 */
PreferencesFile*
BrainSet::getPreferencesFile() {
   if (preferencesFile == NULL) {
       QString preferencesFileName = QDir::homePath();
       if (preferencesFileName.isEmpty() == false) {
          preferencesFileName.append("/");
       }
       preferencesFileName.append(".caret5_preferences");

       preferencesFile = new PreferencesFile;
       try {
          preferencesFile->readFile(preferencesFileName);
       }
       catch (FileException) {
       }
       preferencesFile->setFileName(preferencesFileName);
   }

   return preferencesFile;
}

/**
 * Read the user's caret5 preferences file and intialize other things.
 */
void
BrainSet::initializeStaticStuff()
{
   if (staticStuffInitialized) {
      return;
   }
   staticStuffInitialized = true;

   //
   // Might be set at command line
   //
   const bool debugOn = DebugControl::getDebugOn();

   try {
      AbstractFile::setTextFileDigitsRightOfDecimal(
          getPreferencesFile()->getTextFileDigitsRightOfDecimal());
      AbstractFile::setPreferredWriteType(
          getPreferencesFile()->getPreferredWriteDataType());
   }
   catch (FileException& /*e*/) {
      //std::cerr << "Warning: reading caret preferences file: "
      //          << e.whatQString() << std::endl;
   }

   //
   // Random seed generator
   //
   if (getPreferencesFile()->getRandomSeedOverride()) {
      //
      // Use seed provided by user
      //
      setRandomSeed(getPreferencesFile()->getRandomSeedOverrideValue());
   }
   else {
      //
      // Use number of seconds since 1970
      //
      setRandomSeed(QDateTime::currentDateTime().toTime_t());
   }
   
   if (debugOn) {
      DebugControl::setDebugOn(true);
   }
}

/**
 * The destructor.
 */
BrainSet::~BrainSet()
{
   reset();
   
   delete displayCrossTimer;
   displayCrossTimer = NULL;
   
   displayCrossForNode = -1;
   displayNoCrossForSurface = NULL;
   
   delete areaColorFile;
   delete arealEstimationFile;
   delete borderColorFile;
   delete cellColorFile;
   delete cellProjectionFile;
   delete volumeCellFile;
   delete cocomacFile;
   delete contourCellFile;
   delete contourCellColorFile;
   delete cutsFile;
   delete deformationFieldFile;
   delete fociColorFile;
   delete fociProjectionFile;
   delete geodesicDistanceFile;
   delete latLonFile;
   delete metricFile;
   delete probabilisticAtlasSurfaceFile;
   delete paintFile;
   delete paletteFile;
   delete paramsFile;
   delete rgbPaintFile;
   delete sceneFile;
   delete sectionFile;
   delete studyCollectionFile;
   delete studyMetaDataFile;
   delete surfaceShapeFile;
   delete topographyFile;
   delete trajectoryFile;
   delete transformationMatrixFile;
   delete vocabularyFile;
   delete wustlRegionFile;
   
   delete brainModelBorderSet;

   delete brainSetAutoLoaderManager;
   delete brainModelIdentification;
   delete nodeColoring;
   delete voxelColoring;
   delete brainModelVolumeRegionOfInterest;
   delete brainModelSurfaceRegionOfInterestNodeSelection;
   delete displaySettingsArealEstimation;  
   delete displaySettingsBorders;
   delete displaySettingsDeformationField;
   delete displaySettingsGeodesicDistance;
   delete displaySettingsImages;
   delete displaySettingsMetric;
   delete displaySettingsModels;
   delete displaySettingsSection;
   delete displaySettingsSurface;
   delete displaySettingsPaint;
   delete displaySettingsProbabilisticAtlasSurface;
   delete displaySettingsRgbPaint;  
   delete displaySettingsScene;
   delete displaySettingsStudyMetaData;
   delete displaySettingsSurfaceShape;  
   delete displaySettingsVectors;
   delete displaySettingsCells;
   delete displaySettingsCoCoMac;
   delete displaySettingsContours;
   delete displaySettingsCuts;
   delete displaySettingsFoci;
   delete displaySettingsTopography;
   delete displaySettingsVolume;
   delete displaySettingsProbabilisticAtlasVolume;
   delete displaySettingsWustlRegion;
}

/**
 * get the primary surface overlay.
 */
BrainModelSurfaceOverlay* 
BrainSet::getPrimarySurfaceOverlay()
{
   return getSurfaceOverlay(getNumberOfSurfaceOverlays() - 1);
}

/**
 * get the primary surface overlay (const method).
 */
const BrainModelSurfaceOverlay* 
BrainSet::getPrimarySurfaceOverlay() const
{
   return getSurfaceOverlay(getNumberOfSurfaceOverlays() - 1);
}

/**
 * get the secondary surface overlay.
 */
BrainModelSurfaceOverlay* 
BrainSet::getSecondarySurfaceOverlay()
{
   return getSurfaceOverlay(getNumberOfSurfaceOverlays() - 2);
}

/**
 * get the secondary surface overlay (const method).
 */
const BrainModelSurfaceOverlay* 
BrainSet::getSecondarySurfaceOverlay() const
{
   return getSurfaceOverlay(getNumberOfSurfaceOverlays() - 2);
}

/**
 * get the surface underlay.
 */
BrainModelSurfaceOverlay* 
BrainSet::getSurfaceUnderlay()
{
   return getSurfaceOverlay(0);
}

/**
 * get the surface underlay (const method).
 */
const BrainModelSurfaceOverlay* 
BrainSet::getSurfaceUnderlay() const
{
   return getSurfaceOverlay(0);
}
      
/**
 * get something is an overlay for any model.
 */
bool 
BrainSet::isASurfaceOverlayForAnySurface(const BrainModelSurfaceOverlay::OVERLAY_SELECTIONS os) const
{
   for (int i = 0; i < getNumberOfSurfaceOverlays(); i++) {
      for (int m = 0; m < getNumberOfBrainModels(); m++) {
         if (getBrainModelSurface(m) != NULL) {
            if (getSurfaceOverlay(i)->getOverlay(m) == os) {
               return true;
            }
         }
      }
   }
   
   return false;
}
      
/**
 * get something is an overlay.
 */
bool 
BrainSet::isASurfaceOverlay(const int modelIn,
                     const BrainModelSurfaceOverlay::OVERLAY_SELECTIONS os) const
{
   int model = modelIn;
   if (model < 0) {
      model = 0;
   }
   
   for (int i = 0; i < getNumberOfSurfaceOverlays(); i++) {
      if (getSurfaceOverlay(i)->getOverlay(model) == os) {
         return true;
      }
   }
   
   return false;
}
      
/**
 * copy the overlay selections from specified surface to all other surfaces.
 */
void 
BrainSet::copyOverlaysFromSurface(const int surfaceModelIndex)
{
   for (int i = 0; i < getNumberOfSurfaceOverlays(); i++) {
      getSurfaceOverlay(i)->copyOverlaysFromSurface(surfaceModelIndex);
   }
}
      
/**
 * update the surface overlays due to change in brain models.
 */
void 
BrainSet::updateSurfaceOverlaysDueToChangeInBrainModels()
{
   for (int i = 0; i < getNumberOfSurfaceOverlays(); i++) {
      getSurfaceOverlay(i)->update();
   }
}
      
/**
 * apply all projected files (foci, cells, borders).
 */
void 
BrainSet::applyAllProjectedFiles()
{
   //
   // Unproject all borders
   //
   brainModelBorderSet->unprojectBordersForAllSurfaces();
}

/**
 * Clear all display list.
 */
void
BrainSet::clearAllDisplayLists()
{
   const int num = getNumberOfBrainModels();
   
   //
   //  Clear all surface display lists
   //
   for (int i = 0; i < num; i++) {
      BrainModelSurface* bms = getBrainModelSurface(i);
      if (bms != NULL) {
         CoordinateFile* cf = bms->getCoordinateFile();
         cf->clearDisplayList();
      }
   }
   
   //
   // Clear surface and volume voxel cloud display list
   //
   BrainModelSurfaceAndVolume* bmsv = getBrainModelSurfaceAndVolume();
   if (bmsv != NULL) {
      CoordinateFile* cf = bmsv->getCoordinateFile();
      cf->clearDisplayList();
      bmsv->clearVoxelCloudDisplayList();
   }
   
   for (int i = 0; i < getNumberOfTopologyFiles(); i++) {
      topologyFiles[i]->clearDisplayList();
   }
   
   arealEstimationFile->clearDisplayList();
   
   areaColorFile->clearDisplayList();
   borderColorFile->clearDisplayList();
   cellColorFile->clearDisplayList();
   cellProjectionFile->clearDisplayList();
   volumeCellFile->clearDisplayList();
   fociColorFile->clearDisplayList();
   fociProjectionFile->clearDisplayList();
   fociSearchFile->clearDisplayList();
   cocomacFile->clearDisplayList();
   cutsFile->clearDisplayList();
   deformationFieldFile->clearDisplayList();
   geodesicDistanceFile->clearDisplayList();
   for (int i = 0; i < getNumberOfImageFiles(); i++) {
      imageFiles[i]->clearDisplayList();
   }
   latLonFile->clearDisplayList();
   metricFile->clearDisplayList();
   paintFile->clearDisplayList();
   paletteFile->clearDisplayList();
   paramsFile->clearDisplayList();
   probabilisticAtlasSurfaceFile->clearDisplayList();
   rgbPaintFile->clearDisplayList();
   sectionFile->clearDisplayList();
   studyCollectionFile->clearDisplayList();
   studyMetaDataFile->clearDisplayList();
   surfaceShapeFile->clearDisplayList();
   topographyFile->clearDisplayList();
   for (int i = 0; i < getNumberOfVtkModelFiles(); i++) {
      vtkModelFiles[i]->clearDisplayList();
   }
   for (int i = 0; i < getNumberOfTransformationDataFiles(); i++) {
      transformationDataFiles[i]->clearDisplayList();
   }
   vocabularyFile->clearDisplayList();
   wustlRegionFile->clearDisplayList();
}

/**
 * Delete all of the brain models
 */
void
BrainSet::deleteAllBrainModels()
{
   deleteAllTopologyFiles();
   
   clearVolumeAnatomyFiles();
   clearVolumeFunctionalFiles();
   clearVolumePaintFiles();
   clearVolumeProbabilisticAtlasFiles();
   clearVolumeRgbFiles();
   clearVolumeSegmentationFiles();
   clearVolumeVectorFiles();
   
   for (unsigned int i = 0; i < brainModels.size(); i++) {
      if (brainModels[i] != NULL) {
         delete brainModels[i];
         brainModels[i] = NULL;
      }
   }
   brainModels.clear();

   updateDisplayedModelIndices();   
}

/**
 * Reset the brain surface.  Typically used prior to loading new files.
 */
void
BrainSet::reset(const bool keepSceneData)
{
   for (int i = 0; i < BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; i++) {
      displayedModelIndices[i] = 0;
   }
   
   SpecFile::Entry savedSceneFile = loadedFilesSpecFile.sceneFile;
   loadedFilesSpecFile.clear();
   specFileName = "";
   
   specDataFileTransformationMatrix.identity();
   
   stereotaxicSpace.reset();
   species.reset();
   setSubject("");
   
   deleteAllBrainModels();
   
   resetDataFiles(keepSceneData, false);
   if (keepSceneData) {
      loadedFilesSpecFile.sceneFile = savedSceneFile;
   }
   
   deleteAllBorders();

   brainSetAutoLoaderManager->reset();
   displaySettingsArealEstimation->reset();  
   displaySettingsBorders->reset();
   displaySettingsDeformationField->reset();
   displaySettingsGeodesicDistance->reset();
   displaySettingsImages->reset();
   displaySettingsMetric->reset();
   displaySettingsModels->reset();
   displaySettingsSection->reset();
   displaySettingsSurface->reset();
   displaySettingsPaint->reset();
   displaySettingsProbabilisticAtlasSurface->reset();
   displaySettingsRgbPaint->reset();  
   if (keepSceneData == false) {
      displaySettingsScene->reset();
   }
   displaySettingsStudyMetaData->reset();
   displaySettingsSurfaceShape->reset();  
   displaySettingsVectors->reset();
   displaySettingsCells->reset();
   displaySettingsCoCoMac->reset();
   displaySettingsContours->reset();
   displaySettingsCuts->reset();
   displaySettingsFoci->reset();
   displaySettingsTopography->reset();
   displaySettingsVolume->reset();
   displaySettingsProbabilisticAtlasVolume->reset();
   displaySettingsWustlRegion->reset();
   
   //resetNodeAttributeFiles();
   
   resetNodeAttributes();
   structure = Structure::STRUCTURE_TYPE_INVALID;
   
   activeFiducialSurface = NULL;
   leftFiducialVolumeInteractionSurface = NULL;
   rightFiducialVolumeInteractionSurface = NULL;
   cerebellumFiducialVolumeInteractionSurface = NULL;
   
   cerebralHullFileName = "";
   deleteAllImageFiles();
   deleteAllVtkModelFiles();
   
   nodesHaveBeenClassified = false;
   
   displayAllNodesFlag = true;
}

/**
 * Reset data files.
 */
void
BrainSet::resetDataFiles(const bool keepSceneData,
                         const bool keepFociAndFociColorsAndStudyMetaData)
{
   deleteAllBorders();

   clearCocomacConnectivityFile();
   clearContourCellFile();
   clearContourCellColorFile();
   clearCutsFile();
   clearAreaColorFile();
   clearParamsFile();
   if (keepSceneData == false) {
      sceneFile->clear();
   }
   
   clearPaletteFile();
   paletteFile->addDefaultPalettes();
   paletteFile->clearModified();
   
   clearVectorFiles();

   clearBorderColorFile();
   
   clearCellColorFile();
   deleteAllCells(true, true);
   
   if (keepFociAndFociColorsAndStudyMetaData == false) {
      clearFociColorFile();
      deleteAllFociProjections();
      clearFociSearchFile();
      fociSearchFile->addDefaultSearch();
   }
   
   if (keepFociAndFociColorsAndStudyMetaData == false) {
      clearStudyCollectionFile();
      clearStudyMetaDataFile();
   }
   
   clearVocabularyFile();
   
   clearTransformationMatrixFile();
   
   clearTransformationDataFiles();
   
   deleteAllImageFiles();
   deleteAllVtkModelFiles();
   
   resetNodeAttributeFiles();
}

/** 
 * Reset node attribute files
 */
void
BrainSet::resetNodeAttributeFiles()
{
   clearArealEstimationFile();
   arealEstimationFile->clearModified();
   deformationMapFileName = "";
   
   clearDeformationFieldFile();
   deformationFieldFile->clearModified();
   
   clearLatLonFile();
   clearSectionFile();
   
   clearMetricFile();
   metricFile->clearModified();
   clearProbabilisticAtlasFile();
   probabilisticAtlasSurfaceFile->clearModified();
   clearPaintFile();
   paintFile->clearModified();
   clearRgbPaintFile();
   rgbPaintFile->clearModified();
   
   clearSurfaceShapeFile();
   surfaceShapeFile->clearModified();
   
   clearTopographyFile();
   topographyFile->clearModified();
   
}

/** 
 * initialize data file static members
 */
void 
BrainSet::initializeDataFileStaticMembers()
{
}

/**
 * Get the model for a window from a scene.
 */
BrainModel* 
BrainSet::showSceneGetBrainModel(const int sceneIndex,
                                 const int viewingWindowNumberIn,
                                 int geometryOut[4],
                                 int glWidthHeightOut[2],
                                 bool& yokeOut,
                                 QString& errorMessageOut)
{
   SceneFile* sf = getSceneFile();
   if ((sceneIndex >= 0) && (sceneIndex < sf->getNumberOfScenes())) {
      return showSceneGetBrainModel(sf->getScene(sceneIndex), 
                                    viewingWindowNumberIn,
                                    geometryOut,
                                    glWidthHeightOut,
                                    yokeOut,
                                    errorMessageOut);
   }
   return NULL;
}

/**
 * Get the model for a window from a scene.
 */
BrainModel*
BrainSet::showSceneGetBrainModel(const SceneFile::Scene* scene,
                                 const int viewingWindowNumber,
                                 int geometryOut[4],
                                 int glWidthHeightOut[2],
                                 bool& yokeOut,
                                 QString& errorMessageOut)
{
   geometryOut[0] = -1;
   geometryOut[1] = -1;
   geometryOut[2] = -1;
   geometryOut[3] = -1;
   glWidthHeightOut[0] = -1;
   glWidthHeightOut[1] = -1;
   yokeOut = false;
   errorMessageOut = "";
   
   //SceneFile* sf = getSceneFile();
   //if ((sceneIndex < 0) || (sceneIndex >= sf->getNumberOfScenes())) {
   //   return NULL;
   //}
   if (viewingWindowNumber < 0) {
      return NULL;
   }
   
   QString windowName("GuiMainWindow");
   if (viewingWindowNumber > 0) {
      std::ostringstream str;
      str << "ViewingWindow:"
          << (viewingWindowNumber + 1);
      windowName = str.str().c_str();
   }

   //const SceneFile::Scene* scene = sf->getScene(sceneIndex);
   
   const int numModels = getNumberOfBrainModels();
   
   BrainModel* brainModel = NULL;
   
   const int numClasses = scene->getNumberOfSceneClasses();
   for (int nc = 0; nc < numClasses; nc++) {
      const SceneFile::SceneClass* sc = scene->getSceneClass(nc);
      const QString className(sc->getName());
      if (className == windowName) {
         BrainModelVolume* bmv = NULL;
         BrainModelSurface* bms = NULL;
         const int num = sc->getNumberOfSceneInfo();
         for (int i = 0; i < num; i++) {
            const SceneFile::SceneInfo* si = sc->getSceneInfo(i);
            const QString infoName = si->getName();      
            
            if (infoName == "BrainModel") {
               const QString modelName = si->getModelName();
               const QString value = si->getValueAsString();
               brainModel = NULL;
               if (modelName == "BrainModelContours") {
                  brainModel = getBrainModelContours();
                  bmv = NULL;
               }
               else if (modelName == "BrainModelSurface") {
                  for (int j = 0; j < numModels; j++) {
                     BrainModelSurface* bms1 = getBrainModelSurface(j);
                     if (bms1 != NULL) {
                        if (value == FileUtilities::basename(bms1->getCoordinateFile()->getFileName())) {
                           brainModel = bms1;
                           bms = bms1;
                           break;
                        }
                     }
                  }
                  bmv = NULL;
               }
               else if (modelName == "BrainModelSurfaceAndVolume") {
                  brainModel = getBrainModelSurfaceAndVolume();
                  bmv = NULL;
               }
               else if (modelName == "BrainModelVolume") {
                  brainModel = getBrainModelVolume();
                  bmv = getBrainModelVolume();
               }
               
               if (brainModel == NULL) {
                  errorMessageOut.append("Unable to find brain model: ");
                  errorMessageOut.append(modelName);
                  errorMessageOut.append("\n");
                  return NULL;
               }
            }
            else if (infoName == "Transformation") {
               if (brainModel != NULL) {
                  brainModel->setTransformationsAsString(viewingWindowNumber, si->getValueAsString());
               }
            }
            else if (infoName == "Slices") {
               if (bmv != NULL) {
                  std::vector<QString> tokens;
                  StringUtilities::token(si->getValueAsString(), " ", tokens);
                  if (tokens.size() >= 3) {
                     const int slices[3] = {
                        StringUtilities::toInt(tokens[0]),
                        StringUtilities::toInt(tokens[1]),
                        StringUtilities::toInt(tokens[2])
                     };
                     bmv->setSelectedOrthogonalSlices(viewingWindowNumber, slices);
                  }
               }
            }
            else if (infoName == "viewStereotaxicCoordinatesFlag") {
               if (bmv != NULL) {
                  bmv->setViewStereotaxicCoordinatesFlag(viewingWindowNumber, si->getValueAsBool());
               }
            }
            else if (infoName == "Oblique-Trans") {
               if (bmv != NULL) {
                  bmv->setObliqueTransformationsAsString(viewingWindowNumber, si->getValueAsString());
               }
            }
            else if (infoName == "Oblique-Slices") {
               if (bmv != NULL) {
                  std::vector<QString> tokens;
                  StringUtilities::token(si->getValueAsString(), " ", tokens);
                  if (tokens.size() >= 3) {
                     const int slices[3] = {
                        StringUtilities::toInt(tokens[0]),
                        StringUtilities::toInt(tokens[1]),
                        StringUtilities::toInt(tokens[2])
                     };
                     bmv->setSelectedObliqueSlices(slices);
                  }
               }
            }
            else if (infoName == "Oblique-Slice-Offsets") {
               if (bmv != NULL) {
                  std::vector<QString> tokens;
                  StringUtilities::token(si->getValueAsString(), " ", tokens);
                  if (tokens.size() >= 3) {
                     const int slices[3] = {
                        StringUtilities::toInt(tokens[0]),
                        StringUtilities::toInt(tokens[1]),
                        StringUtilities::toInt(tokens[2])
                     };
                     bmv->setSelectedObliqueSliceOffsets(viewingWindowNumber, slices);
                  }
               }
            }
            else if (infoName == "Axis") {
               if (bmv != NULL) {
                  bmv->setSelectedAxis(viewingWindowNumber, 
                            static_cast<VolumeFile::VOLUME_AXIS>(si->getValueAsInt()));
               }
            }
            else if (infoName == "Topology") {
               const QString topoName(si->getValueAsString());
               for (int mm = 0; mm < getNumberOfTopologyFiles(); mm++) {
                  TopologyFile* tf = getTopologyFile(mm);
                  if (tf != NULL) {
                     if (FileUtilities::basename(tf->getFileName()) == topoName) {
                        if (bms != NULL) {
                           bms->setTopologyFile(tf);
                           break;
                        }
                     }
                  }
               }
            }
            else if (infoName == "Geometry") {
               std::vector<QString> tokens;
               StringUtilities::token(si->getValueAsString(), " ", tokens);
               if (tokens.size() >= 4) {
                  geometryOut[0] = StringUtilities::toInt(tokens[0]);
                  geometryOut[1] = StringUtilities::toInt(tokens[1]);
                  geometryOut[2] = StringUtilities::toInt(tokens[2]);
                  geometryOut[3] = StringUtilities::toInt(tokens[3]);
               }
            }
            else if (infoName == "GLWidgetSize") {
               std::vector<QString> tokens;
               StringUtilities::token(si->getValueAsString(), " ", tokens);
               if (tokens.size() >= 2) {
                  glWidthHeightOut[0] = StringUtilities::toInt(tokens[0]);
                  glWidthHeightOut[1] = StringUtilities::toInt(tokens[1]);
               }
            }
            else if (infoName == "Yoke") {
               yokeOut = si->getValueAsBool();
            }
         }
      }
   }

   return brainModel;
}

/**
 * Save the model for a window from a scene
 */
void
BrainSet::saveSceneForBrainModelWindow(const int viewingWindowNumber,
                                       const int geometry[4],
                                       const int glWidthHeight[2],
                                       const BrainModel* bm,
                                       const bool yokeIn,
                                       SceneFile::SceneClass& sceneClass)
{
   if (viewingWindowNumber < 0) {
      return;
   }
   if (bm == NULL) {
      return;
   }
   
   QString windowName("GuiMainWindow");
   if (viewingWindowNumber > 0) {
      std::ostringstream str;
      str << "ViewingWindow:"
          << (viewingWindowNumber + 1);
      windowName = str.str().c_str();
   }

   QString modelName;
   QString modelValue;
   BrainModelSurface* bms = NULL;
   BrainModelVolume* bmv = NULL;
   if (getBrainModelContours() == bm) {
      modelName = "BrainModelContours";
      modelValue = modelName;
   }
   else if (getBrainModelSurfaceAndVolume() == bm) {
      modelName = "BrainModelSurfaceAndVolume";
      modelValue = modelName;
   }
   else if (getBrainModelVolume() == bm) {
      modelName = "BrainModelVolume";
      modelValue = modelName;
      bmv = getBrainModelVolume();
   }
   else {
      for (int i = 0; i < getNumberOfBrainModels(); i++) {
         if (getBrainModelSurface(i) == bm) {
            bms = getBrainModelSurface(i);
            modelName = "BrainModelSurface";
            modelValue = FileUtilities::basename(bms->getCoordinateFile()->getFileName());
            break;
         }
      }
   }
   
   if (modelName.isEmpty() == false) {
      sceneClass.setName(windowName);    
      sceneClass.addSceneInfo(SceneFile::SceneInfo("BrainModel", modelName, modelValue));
      if (bm != NULL) {
         sceneClass.addSceneInfo(SceneFile::SceneInfo("Transformation", bm->getTransformationsAsString(viewingWindowNumber)));
      }
      if (bms != NULL) {
         const TopologyFile* tf = bms->getTopologyFile();
         if (tf != NULL) {
            sceneClass.addSceneInfo(SceneFile::SceneInfo("Topology",
                                    FileUtilities::basename(tf->getFileName())));
         }
      }
      if (bmv != NULL) {
         int slices[3];
         bmv->getSelectedOrthogonalSlices(viewingWindowNumber, slices);
         std::ostringstream str;
         str << slices[0] << " " << slices[1] << " " << slices[2];
         sceneClass.addSceneInfo(SceneFile::SceneInfo("Slices", str.str().c_str()));
         sceneClass.addSceneInfo(SceneFile::SceneInfo("Axis", bmv->getSelectedAxis(viewingWindowNumber)));
         sceneClass.addSceneInfo(SceneFile::SceneInfo("Oblique-Trans", bmv->getObliqueTransformationsAsString(viewingWindowNumber)));
         sceneClass.addSceneInfo(SceneFile::SceneInfo("viewStereotaxicCoordinatesFlag", bmv->getViewStereotaxicCoordinatesFlag(viewingWindowNumber)));
         str.str("");
         int obliqueSlices[3];
         bmv->getSelectedObliqueSlices(obliqueSlices);
         str << obliqueSlices[0] << " " << obliqueSlices[1] << " " << obliqueSlices[2];
         sceneClass.addSceneInfo(SceneFile::SceneInfo("Oblique-Slices", str.str().c_str()));

         str.str("");
         int obliqueSliceOffsets[3];
         bmv->getSelectedObliqueSliceOffsets(viewingWindowNumber, obliqueSliceOffsets);
         str << obliqueSliceOffsets[0] << " " << obliqueSliceOffsets[1] << " " << obliqueSliceOffsets[2];
         sceneClass.addSceneInfo(SceneFile::SceneInfo("Oblique-Slice-Offsets", str.str().c_str()));
      }
      
      std::ostringstream str;
      str << geometry[0] << " "
          << geometry[1] << " "
          << geometry[2] << " "
          << geometry[3];
      sceneClass.addSceneInfo(SceneFile::SceneInfo("Geometry", str.str().c_str()));
      
      str.str("");
      str << glWidthHeight[0] << " "
          << glWidthHeight[1];
      sceneClass.addSceneInfo(SceneFile::SceneInfo("GLWidgetSize", str.str().c_str()));

      sceneClass.addSceneInfo(SceneFile::SceneInfo("Yoke", yokeIn));
   }
}

/**
 * apply a scene (set display settings).
 */
void 
BrainSet::showScene(const int sceneIndex, 
                    QString& errorMessage,
                    QString& warningMessage)
{
   SceneFile* sf = getSceneFile();
   if ((sceneIndex >= 0) && (sceneIndex < sf->getNumberOfScenes())) {
      showScene(sf->getScene(sceneIndex), false, errorMessage, warningMessage);
   }
}

/**
 * load identification filters from a scene.
 */
void 
BrainSet::showSceneIdentificationFilters(const SceneFile::Scene* ss,
                                         QString& errorMessage)
{
   errorMessage = "";
   
   if (ss != NULL) {
      brainModelIdentification->showScene(*ss, errorMessage);
   }
}

/**
 * apply a scene (set display settings).
 */
void 
BrainSet::showScene(const SceneFile::Scene* ss, 
                    const bool checkSpecFlag,
                    QString& errorMessage,
                    QString& warningMessage)
{
   errorMessage = "";
   
   //const SceneFile::Scene* ss = sceneFile->getScene(sceneIndex);
   if (ss != NULL) {
      //
      // Find out which files are needed for this scene
      //
      SpecFile sf;
      sf.showScene(*ss, errorMessage);
      
      //
      // Should the spec file be checked
      //
      if (checkSpecFlag) {
         SpecFile userSpec;
         try {
            userSpec.readFile(getSpecFileName());
            QString msg;
            if (sf.isSubsetOfOtherSpecFile(userSpec, msg) == false) {
               errorMessage.append("Spec File Errors for Spec File ");
               errorMessage.append(FileUtilities::basename(getSpecFileName()));
               errorMessage.append(": \n");
               errorMessage.append(msg);
            }
         }
         catch (FileException&) {
         }
      }
      
/*
      //
      // clear colors
      //
      clearAreaColorFile();
      clearBorderColorFile();
      clearCellColorFile();
      clearContourCellColorFile();
      clearFociColorFile();
      
      //
      // Clear all borders, cells, cuts, and foci
      //
      deleteAllBorders();
      deleteAllCells(true, true);
      deleteAllFociProjections();
      clearCutsFile();
      clearContourCellFile();
      
      //
      // Clear cocomac
      //
      clearCocomacConnectivityFile();
      
      //
      // Clear probabilistic atlas files
      //
      clearProbabilisticAtlasFile();
      probabilisticAtlasSurfaceFile->clearModified();
*/
      //
      // Clear data files
      //      
      resetDataFiles(true,
                     displaySettingsScene->getPreserveFociAndFociColorsAndStudyMetaDataFlag());

      //
      // Get rid of volume prob atlas files
      //
      this->clearVolumeAnatomyFiles();
      this->clearVolumeFunctionalFiles();
      this->clearVolumePaintFiles();
      this->clearVolumeProbabilisticAtlasFiles();
      this->clearVolumeRgbFiles();
      this->clearVolumeSegmentationFiles();
      this->clearVolumeVectorFiles();
 
      //
      // Clear node identify symbols
      //
      clearNodeHighlightSymbols();
      
      //
      // Do not load any files that are already loaded
      //
      sf.deselectFilesSelectedInOtherSpecFile(loadedFilesSpecFile);
      
      //
      // Read any files that are part of scene but not already loaded
      //
      QString specMsg;
      readSpecFile(SPEC_FILE_READ_MODE_APPEND,
                   sf,
                   "scene",
                   specMsg,
                   NULL,
                   NULL);
      errorMessage.append(specMsg);
      paletteFile->clearModified();

      //
      // Auto-Loaded Files
      //
      brainSetAutoLoaderManager->showScene(*ss, errorMessage);

      //
      // Update overlays
      //
      for (int i = 0; i < getNumberOfSurfaceOverlays(); i++) {
         getSurfaceOverlay(i)->showScene(*ss, errorMessage);
      }
       
      //
      // Update node/voxel coloring
      //
      brainModelIdentification->showScene(*ss, errorMessage);
      nodeColoring->showScene(*ss, errorMessage);
      voxelColoring->showScene(*ss, errorMessage);
      
      //
      // surface and volume settings
      //
      BrainModelSurfaceAndVolume* bmsv = getBrainModelSurfaceAndVolume();
      if (bmsv != NULL) {
         bmsv->showScene(*ss, errorMessage);
      }
      
      //
      // transformation matrices
      //
      transformationMatrixFile->showScene(*ss, errorMessage);

      //
      // display settings
      //
      displaySettingsArealEstimation->showScene(*ss, errorMessage);
      displaySettingsBorders->showScene(*ss, errorMessage);
      displaySettingsCells->showScene(*ss, errorMessage);
      displaySettingsCoCoMac->showScene(*ss, errorMessage);
      displaySettingsContours->showScene(*ss, errorMessage);
      displaySettingsCuts->showScene(*ss, errorMessage);
      displaySettingsFoci->showScene(*ss, errorMessage);
      displaySettingsGeodesicDistance->showScene(*ss, errorMessage);
      displaySettingsDeformationField->showScene(*ss, errorMessage);
      displaySettingsImages->showScene(*ss, errorMessage);
      displaySettingsMetric->showScene(*ss, errorMessage);
      displaySettingsModels->showScene(*ss, errorMessage);
      displaySettingsPaint->showScene(*ss, errorMessage);
      displaySettingsProbabilisticAtlasSurface->showScene(*ss, errorMessage);
      displaySettingsProbabilisticAtlasVolume->showScene(*ss, errorMessage);
      displaySettingsRgbPaint->showScene(*ss, errorMessage);
      displaySettingsScene->showScene(*ss, errorMessage);
      displaySettingsSection->showScene(*ss, errorMessage);
      displaySettingsStudyMetaData->showScene(*ss, errorMessage);
      displaySettingsSurface->showScene(*ss, errorMessage);
      displaySettingsSurfaceShape->showScene(*ss, errorMessage);
      displaySettingsVectors->showScene(*ss, errorMessage);
      displaySettingsTopography->showScene(*ss, errorMessage);
      displaySettingsVolume->showScene(*ss, errorMessage);
      displaySettingsWustlRegion->showScene(*ss, errorMessage);
      
      assignBorderColors();
      assignCellColors();
      assignFociColors();
      
      displaySettingsBorders->determineDisplayedBorders();
      displaySettingsCells->determineDisplayedCells();
      displaySettingsFoci->determineDisplayedCells(true);
      
      clearNodeHighlightSymbols();

      const int numClasses = ss->getNumberOfSceneClasses();
      for (int nc = 0; nc < numClasses; nc++) {
         const SceneFile::SceneClass* sc = ss->getSceneClass(nc);
         const int num = sc->getNumberOfSceneInfo();
         
         //
         // Node highlighting
         //
         if (sc->getName() == "NodeHighlighting") {
            clearNodeHighlightSymbols();

            const int numNodes = getNumberOfNodes();
            for (int i = 0; i < num; i++) {
               const SceneFile::SceneInfo* si = sc->getSceneInfo(i);
               const int nodeNum = StringUtilities::toInt(si->getModelName());
               if (numNodes > 0) {      
                  BrainSetNodeAttribute* bna = getNodeAttributes(nodeNum);
                  bna->setHighlighting(
                     static_cast<BrainSetNodeAttribute::HIGHLIGHT_NODE_TYPE>(si->getValueAsInt()));
               }
            }
         }
         else if ((sc->getName() == "BrainSet") ||
                  (sc->getName() == "BrainSet")) {
            for (int i = 0; i < num; i++) {
               const SceneFile::SceneInfo* si = sc->getSceneInfo(i);
               if (si->getName() == "ActiveFiducial") {
                  const QString activeFiducialName(si->getValueAsString());
                  bool found = false;
                  const int numModels = getNumberOfBrainModels();
                  for (int i = 0; i < numModels; i++) {
                     BrainModelSurface* bms = getBrainModelSurface(i);
                     if (bms != NULL) {
                        const CoordinateFile* cf = bms->getCoordinateFile();
                        const QString name(FileUtilities::basename(cf->getFileName()));
                        if (name == activeFiducialName) {
                           setActiveFiducialSurface(bms);
                           found = true;
                           break;
                        }
                     }
                  }
                  if (found == false) {
                     errorMessage.append("Unable to set active fiducial surface to: ");
                     errorMessage.append(activeFiducialName);
                     errorMessage.append("\n");
                  }
               }  
               else if (si->getName() == "LeftVolumeFiducialInteraction") {
                  const QString fiducialName(si->getValueAsString());
                  bool found = false;
                  const int numModels = getNumberOfBrainModels();
                  for (int i = 0; i < numModels; i++) {
                     BrainModelSurface* bms = getBrainModelSurface(i);
                     if (bms != NULL) {
                        const CoordinateFile* cf = bms->getCoordinateFile();
                        const QString name(FileUtilities::basename(cf->getFileName()));
                        if (name == fiducialName) {
                           setLeftFiducialVolumeInteractionSurface(bms);
                           found = true;
                           break;
                        }
                     }
                  }
                  if (found == false) {
                     errorMessage.append("Unable to set left fiducial volume interaction surface to: ");
                     errorMessage.append(fiducialName);
                     errorMessage.append("\n");
                  }
               }  
               else if (si->getName() == "RightVolumeFiducialInteraction") {
                  const QString fiducialName(si->getValueAsString());
                  bool found = false;
                  const int numModels = getNumberOfBrainModels();
                  for (int i = 0; i < numModels; i++) {
                     BrainModelSurface* bms = getBrainModelSurface(i);
                     if (bms != NULL) {
                        const CoordinateFile* cf = bms->getCoordinateFile();
                        const QString name(FileUtilities::basename(cf->getFileName()));
                        if (name == fiducialName) {
                           setRightFiducialVolumeInteractionSurface(bms);
                           found = true;
                           break;
                        }
                     }
                  }
                  if (found == false) {
                     errorMessage.append("Unable to set right fiducial volume interaction surface to: ");
                     errorMessage.append(fiducialName);
                     errorMessage.append("\n");
                  }
               }  
               else if (si->getName() == "CerebellumVolumeFiducialInteraction") {
                  const QString fiducialName(si->getValueAsString());
                  bool found = false;
                  const int numModels = getNumberOfBrainModels();
                  for (int i = 0; i < numModels; i++) {
                     BrainModelSurface* bms = getBrainModelSurface(i);
                     if (bms != NULL) {
                        const CoordinateFile* cf = bms->getCoordinateFile();
                        const QString name(FileUtilities::basename(cf->getFileName()));
                        if (name == fiducialName) {
                           setCerebellumFiducialVolumeInteractionSurface(bms);
                           found = true;
                           break;
                        }
                     }
                  }
                  if (found == false) {
                     errorMessage.append("Unable to set cerebellum fiducial volume interaction surface to: ");
                     errorMessage.append(fiducialName);
                     errorMessage.append("\n");
                  }
               }  
            }
         }
         else if (sc->getName() == "TransformationDataFile") {
            TransformationMatrixFile* tmf = getTransformationMatrixFile();
            const int numMatrices = tmf->getNumberOfMatrices();
            for (int i = 0; i < num; i++) {
               const SceneFile::SceneInfo* si = sc->getSceneInfo(i);
               const QString fileName = si->getName();
               const QString matrixName = si->getValueAsString();
               
               bool fileFound = false;
               for (int i = 0; i < getNumberOfTransformationDataFiles(); i++) {
                  AbstractFile* af = getTransformationDataFile(i);
                  const QString name = FileUtilities::basename(af->getFileName());
                  if (fileName == name) {
                     fileFound = true;
                     bool matrixFound = true;
                     TransformationMatrix* tm = NULL;
                     if (matrixName != "NULL") {
                        matrixFound = false;
                        for (int i = 0; i < numMatrices; i++) {
                           TransformationMatrix* matrix = tmf->getTransformationMatrix(i);
                           if (matrix->getMatrixName() == matrixName) {
                              tm = matrix;
                              matrixFound = true;
                              break;
                           }
                        }
                     }
                     af->setAssociatedTransformationMatrix(tm);
                     if (matrixFound == false) {
                        errorMessage.append("Unable to find matrix for transformation data file ");
                        errorMessage.append(fileName);
                        errorMessage.append(", matrix ");
                        errorMessage.append(matrixName);
                     }
                     fileFound = true;
                     break;
                  }
               }
               if (fileFound == false) {
                  errorMessage.append("Transformation data file in scene but not loaded ");
                  errorMessage.append(fileName);
               }
            }
         }
      }

      //
      // Assign node/voxel colors
      //
      nodeColoring->assignColors();
      voxelColoring->setVolumeAllColoringInvalid();
   }
   
   checkNodeAttributeFilesForDuplicateColumnNames(warningMessage);
}

/**
 * create a scene (read display settings).
 */
void 
BrainSet::saveScene(SceneFile* sf,
                    const std::vector<SceneFile::SceneClass>& mainWindowSceneClasses,
                    const QString& sceneName, const bool onlyIfSelectedFlag,
                    QString& errorMessageOut,
                    QString& warningMessageOut)
{
   SceneFile::Scene scene(sceneName);
   
   saveReplaceSceneHelper(scene, mainWindowSceneClasses, onlyIfSelectedFlag, errorMessageOut, warningMessageOut);
   
   if (errorMessageOut.isEmpty()) {
      sf->addScene(scene);
   }
   else {
      errorMessageOut.append("Scene HAS NOT been saved.");
   }
}

/**
 * insert after scene (read display settings).
 */
void 
BrainSet::insertScene(SceneFile* sf,
                      const int insertAfterIndex,
                      const std::vector<SceneFile::SceneClass>& mainWindowSceneClasses,
                      const QString& sceneName, const bool onlyIfSelectedFlag,
                      QString& errorMessageOut,
                      QString& warningMessageOut)
{
   SceneFile::Scene scene(sceneName);
   
   saveReplaceSceneHelper(scene, mainWindowSceneClasses, onlyIfSelectedFlag, errorMessageOut, warningMessageOut);
   
   if (errorMessageOut.isEmpty()) {
      sf->insertScene(insertAfterIndex, scene);
   }
   else {
      errorMessageOut.append("Scene HAS NOT been saved.");
   }
}
      
/**
 * replace a scene (read display settings).
 */
void 
BrainSet::replaceScene(SceneFile* sf,
                       const int sceneIndex,
                       const std::vector<SceneFile::SceneClass>& mainWindowSceneClasses,
                       const QString& sceneName, const bool onlyIfSelectedFlag,
                       QString& errorMessageOut,
                       QString& warningMessageOut)
{
   SceneFile::Scene scene(sceneName);
   
   saveReplaceSceneHelper(scene, mainWindowSceneClasses, onlyIfSelectedFlag, errorMessageOut, warningMessageOut);
   
   if (errorMessageOut.isEmpty()) {
      sf->replaceScene(sceneIndex, scene);
   }
   else {
      errorMessageOut.append("Scene HAS NOT been saved.");
   }
}
      
/**
 * save/replace scene helper.
 */
void 
BrainSet::saveReplaceSceneHelper(SceneFile::Scene& scene,
                                 const std::vector<SceneFile::SceneClass>& mainWindowSceneClasses,
                                 const bool onlyIfSelectedFlag,
                                 QString& errorMessageOut,
                                 QString& warningMessageOut)
{
   errorMessageOut = "";
   warningMessageOut = "";
   
   if (mainWindowSceneClasses.empty() == false) {
      for (unsigned int i = 0; i < mainWindowSceneClasses.size(); i++) {
         scene.addSceneClass(mainWindowSceneClasses[i]);
      }
   }
   
   //
   // Update overlays
   //
   for (int i = 0; i < getNumberOfSurfaceOverlays(); i++) {
      getSurfaceOverlay(i)->saveScene(scene, onlyIfSelectedFlag);
   }
   brainModelIdentification->saveScene(scene);
   nodeColoring->saveScene(scene, onlyIfSelectedFlag);
   voxelColoring->saveScene(scene, onlyIfSelectedFlag);
   
   BrainModelSurfaceAndVolume* bmsv = getBrainModelSurfaceAndVolume();
   if (bmsv != NULL) {
      bmsv->saveScene(scene, false);
   }
   
   //
   // Transformation matrices
   //
   transformationMatrixFile->saveScene(scene, onlyIfSelectedFlag);
   
   //
   // Matrices associated with transform data files
   //
   SceneFile::SceneClass tsc("TransformationDataFile");
   for (int i = 0; i < getNumberOfTransformationDataFiles(); i++) {
      const AbstractFile* af = getTransformationDataFile(i);
      const QString name = FileUtilities::basename(af->getFileName());
      const TransformationMatrix* tm = af->getAssociatedTransformationMatrix();
      QString matrixName("NULL");
      if (tm != NULL) {
         matrixName = tm->getMatrixName();
      }
      tsc.addSceneInfo(SceneFile::SceneInfo(name, matrixName));
   }
   if (tsc.getNumberOfSceneInfo() > 0) {
      scene.addSceneClass(tsc);
   }
   
   //
   // display settings
   //
   displaySettingsArealEstimation->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsBorders->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsCells->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsCoCoMac->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsContours->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsCuts->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsFoci->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsGeodesicDistance->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsDeformationField->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsImages->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsMetric->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsModels->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsPaint->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsProbabilisticAtlasSurface->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsProbabilisticAtlasVolume->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsRgbPaint->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsScene->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsSection->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsStudyMetaData->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsSurface->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsSurfaceShape->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsVectors->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsTopography->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsVolume->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   displaySettingsWustlRegion->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);

   brainSetAutoLoaderManager->saveScene(scene, onlyIfSelectedFlag, errorMessageOut);
   
   SceneFile::SceneClass sc("NodeHighlighting");   
   const int numNodes = getNumberOfNodes();
   for (int n = 0; n < numNodes; n++) {
      const BrainSetNodeAttribute* bna = getNodeAttributes(n);
      const BrainSetNodeAttribute::HIGHLIGHT_NODE_TYPE hnt = bna->getHighlighting();
      if (hnt != BrainSetNodeAttribute::HIGHLIGHT_NODE_NONE) {
         sc.addSceneInfo(SceneFile::SceneInfo("nh", StringUtilities::fromNumber(n), hnt));
      }
   }
   if (sc.getNumberOfSceneInfo() > 0) {
      scene.addSceneClass(sc);
   }
   
   loadedFilesSpecFile.saveScene(scene, true);   
   
   SceneFile::SceneClass sca("BrainSet");
   const BrainModelSurface* bms = getActiveFiducialSurface();
   if (bms != NULL) {
      const CoordinateFile* cf = bms->getCoordinateFile();
      const QString name(FileUtilities::basename(cf->getFileName()));
      if (name.isEmpty() == false) {
         sca.addSceneInfo(SceneFile::SceneInfo("ActiveFiducial", name));
      }
   }
   
   const BrainModelSurface* bmsLeft = getLeftFiducialVolumeInteractionSurface();
   if (bmsLeft != NULL) {
      const CoordinateFile* cf = bmsLeft->getCoordinateFile();
      const QString name(FileUtilities::basename(cf->getFileName()));
      if (name.isEmpty() == false) {
         sca.addSceneInfo(SceneFile::SceneInfo("LeftVolumeFiducialInteraction", name));
      }
   }
   
   const BrainModelSurface* bmsRight = getRightFiducialVolumeInteractionSurface();
   if (bmsRight != NULL) {
      const CoordinateFile* cf = bmsRight->getCoordinateFile();
      const QString name(FileUtilities::basename(cf->getFileName()));
      if (name.isEmpty() == false) {
         sca.addSceneInfo(SceneFile::SceneInfo("RightVolumeFiducialInteraction", name));
      }
   }
   
   const BrainModelSurface* bmsCerebellum = getCerebellumFiducialVolumeInteractionSurface();
   if (bmsCerebellum != NULL) {
      const CoordinateFile* cf = bmsCerebellum->getCoordinateFile();
      const QString name(FileUtilities::basename(cf->getFileName()));
      if (name.isEmpty() == false) {
         sca.addSceneInfo(SceneFile::SceneInfo("CerebellumVolumeFiducialInteraction", name));
      }
   }
   
   if (sca.getNumberOfSceneInfo() > 0) {
      scene.addSceneClass(sca);
   }
   
   checkNodeAttributeFilesForDuplicateColumnNames(warningMessageOut);
}                     

/**
 * check node attribute columns for columns with same name.
 */
void 
BrainSet::checkNodeAttributeFilesForDuplicateColumnNames(QString& errorMessageOut)
{
   nodeAttribteDuplicateNamesHelper("Areal Estimation File", arealEstimationFile, errorMessageOut);
   nodeAttribteDuplicateNamesHelper("Deformation Field File", deformationFieldFile, errorMessageOut);
   nodeAttribteDuplicateNamesHelper("Geodesic Distance File", geodesicDistanceFile, errorMessageOut);
   nodeAttribteDuplicateNamesHelper("Lat Lon File", latLonFile, errorMessageOut);
   niftiNodeDataFileDuplicateNamesHelper("Metric File", metricFile, errorMessageOut);
   niftiNodeDataFileDuplicateNamesHelper("Paint File", paintFile, errorMessageOut);
   nodeAttribteDuplicateNamesHelper("Rgb Paint File", rgbPaintFile, errorMessageOut);
   nodeAttribteDuplicateNamesHelper("Section File", sectionFile, errorMessageOut);
   niftiNodeDataFileDuplicateNamesHelper("Shape File", surfaceShapeFile, errorMessageOut);
   nodeAttribteDuplicateNamesHelper("Topography File", topographyFile, errorMessageOut);
}

/**
 * check for duplicate column names in a mode attribute file.
 */
void 
BrainSet::nodeAttribteDuplicateNamesHelper(const QString& fileTypeName,
                                           NodeAttributeFile* naf,
                                           QString& errorMessageOut) const
{
   std::vector<QString> duplicateNames;
   
   if (naf->checkForColumnsWithSameName(duplicateNames)) {
      errorMessageOut.append(fileTypeName);
      errorMessageOut.append(" has the following duplicate column names:\n");
      for (unsigned int i = 0; i < duplicateNames.size(); i++) {
         errorMessageOut.append("   ");
         errorMessageOut.append(duplicateNames[i]);
         errorMessageOut.append("\n");
      }
      errorMessageOut.append("\n");
   }   
}

/**
 * check for duplicate column names in a mode attribute file.
 */
void 
BrainSet::niftiNodeDataFileDuplicateNamesHelper(const QString& fileTypeName,
                                           GiftiNodeDataFile* naf,
                                           QString& errorMessageOut) const
{
   std::vector<QString> duplicateNames;
   
   if (naf->checkForColumnsWithSameName(duplicateNames)) {
      errorMessageOut.append(fileTypeName);
      errorMessageOut.append(" has the following duplicate column names:\n");
      for (unsigned int i = 0; i < duplicateNames.size(); i++) {
         errorMessageOut.append("   ");
         errorMessageOut.append(duplicateNames[i]);
         errorMessageOut.append("\n");
      }
      errorMessageOut.append("\n");
   }   
}

/**
 * Get rid of all topology files
 */
void
BrainSet::deleteAllTopologyFiles()
{
   for (unsigned int j= 0; j < topologyFiles.size(); j++) {
      if (topologyFiles[j] != NULL) {
         loadedFilesSpecFile.closedTopoFile.clearSelectionStatus(topologyFiles[j]->getFileName());
         loadedFilesSpecFile.openTopoFile.clearSelectionStatus(topologyFiles[j]->getFileName());
         loadedFilesSpecFile.cutTopoFile.clearSelectionStatus(topologyFiles[j]->getFileName());
         loadedFilesSpecFile.lobarCutTopoFile.clearSelectionStatus(topologyFiles[j]->getFileName());
         loadedFilesSpecFile.unknownTopoFile.clearSelectionStatus(topologyFiles[j]->getFileName());
         delete topologyFiles[j];
         topologyFiles[j] = NULL;
      }
   }
   topologyFiles.clear();

   topologyClosed   = NULL;
   topologyOpen     = NULL;
   topologyCut      = NULL;
   topologyLobarCut = NULL;
   topologyUnknown  = NULL;
   
}


/**
 * Set the visited flag on all nodes
 */
void
BrainSet::setAllNodesVisited(const bool value)
{
   const int numNodes = getNumberOfNodes();
   for (int i = 0; i < numNodes; i++) {
      nodeAttributes[i].visited = value;
   }
}

/**
 * Set the structure and update in the spec file.
 */
void
BrainSet::setStructure(const Structure::STRUCTURE_TYPE hem)
{
   structure.setType(hem);
   updateDefaultFileNamePrefix();
}

/**
 * Set the structure and update in the spec file.
 */
void
BrainSet::setStructure(const Structure& hem)
{
   structure = hem;
   updateDefaultFileNamePrefix();
}

/**
 * set the subject.
 */
void 
BrainSet::setSubject(const QString& s) 
{ 
   subject = s; 
   updateDefaultFileNamePrefix();
}

/**
 * set the stereotaxic space (const method).
 */
void 
BrainSet::setStereotaxicSpace(const StereotaxicSpace& ss) 
{
   stereotaxicSpace = ss;
}
      
/**
 * set the species.
 */
void 
BrainSet::setSpecies(const Species& s)
{
   species = s;
   updateDefaultFileNamePrefix();
}

/**
 * guess subject, species, and structure if not specified.
 */
void 
BrainSet::guessSubjectSpeciesStructureFromCoordTopoFileNames()
{
   if ((species.getType() == Species::TYPE_UNKNOWN) ||
       subject.isEmpty() ||
       (structure.getType() == Structure::STRUCTURE_TYPE_INVALID)) {
      //
      // Get names of topo and coord files
      //
      std::vector<QString> fileNames;
      for (int i = 0; i < getNumberOfTopologyFiles(); i++) {
         fileNames.push_back(FileUtilities::basename(getTopologyFile(i)->getFileName()));
      }
      for (int i = 0; i < getNumberOfBrainModels(); i++) {
         const BrainModelSurface* bms = getBrainModelSurface(i);
         if (bms != NULL) {
            fileNames.push_back(FileUtilities::basename(bms->getCoordinateFile()->getFileName()));
         }
      }
      
      //
      // Loop through file names
      //
      for (int i = 0; i < static_cast<int>(fileNames.size()); i++) {
         QString fDir, fSpecies, fCasename, fAnatomy, fHemisphere,
                 fDescription, fDescriptionNoTypeName, fTheDate,
                 fNumNodes, fExtension;
         //
         // Is this a valid caret data file name
         //
         if (FileUtilities::parseCaretDataFileName(fileNames[i],
                                                   fDir, 
                                                   fSpecies, 
                                                   fCasename, 
                                                   fAnatomy, 
                                                   fHemisphere,
                                                   fDescription, 
                                                   fDescriptionNoTypeName, 
                                                   fTheDate,
                                                   fNumNodes, 
                                                   fExtension)) {
            //
            // Update data
            //
            if ((species.getType() == Species::TYPE_UNKNOWN) &&
                (fSpecies != "")) {
               species.setUsingName(fSpecies);
            }
            if (subject.isEmpty() &&
                (fCasename.isEmpty() == false)) {
               subject = fCasename;
            }
            if ((structure.getType() == Structure::STRUCTURE_TYPE_INVALID) &&
                (fHemisphere.isEmpty() == false)) {
               const Structure s(fHemisphere);
               if (s.getType() != Structure::STRUCTURE_TYPE_INVALID) {
                  structure = s;
               }
            }
         }
         
         //
         // See if done
         //
         if ((species.getType() != Species::TYPE_UNKNOWN) &&
             (subject.isEmpty() == false) &&
             (structure.getType() == Structure::STRUCTURE_TYPE_INVALID)) {
            break;
         }
      }
   }
}
      
/**
 * update the default file naming prefix.
 */
void 
BrainSet::updateDefaultFileNamePrefix()
{
   if (primaryBrainSetFlag) {
      QString defaultFileNamePrefix;

      guessSubjectSpeciesStructureFromCoordTopoFileNames();
            
      const QString hem = structure.getTypeAsAbbreviatedString();
      if ((hem != "U") && 
          (hem.isEmpty() == false) && 
          (species.getType() != Species::TYPE_UNKNOWN) && 
          (subject.isEmpty() == false)) {
         defaultFileNamePrefix.append(species.getName());
         defaultFileNamePrefix.append(".");
         defaultFileNamePrefix.append(subject);
         defaultFileNamePrefix.append(".");
         defaultFileNamePrefix.append(hem);
      }
      
      AbstractFile::setDefaultFileNamePrefix(defaultFileNamePrefix, getNumberOfNodes());
   }
}
      
/**
 * get a brain model contours (if negative index first one found is returned).
 */
BrainModelContours* 
BrainSet::getBrainModelContours(const int modelIndex) 
{
   if (modelIndex < 0) {
      for (int i = 0; i < getNumberOfBrainModels(); i++) {
         BrainModelContours* bmc = getBrainModelContours(i);
         if (bmc != NULL) {
            return bmc;
         }
      }
   }
   else if (modelIndex < getNumberOfBrainModels()) {
      return dynamic_cast<BrainModelContours*>(brainModels[modelIndex]);
   }
   return NULL;
}

/**
 * get a brain model contours (const method) (if negative index first one found is returned).
 */
const BrainModelContours* 
BrainSet::getBrainModelContours(const int modelIndex) const 
{
   if (modelIndex < 0) {
      for (int i = 0; i < getNumberOfBrainModels(); i++) {
         const BrainModelContours* bmc = getBrainModelContours(i);
         if (bmc != NULL) {
            return bmc;
         }
      }
   }
   else if (modelIndex < getNumberOfBrainModels()) {
      return dynamic_cast<BrainModelContours*>(brainModels[modelIndex]);
   }
   return NULL;
}

/**
 * Get a brain model volume (if negative index first one found is returned).
 */
BrainModelVolume* 
BrainSet::getBrainModelVolume(const int modelIndex) 
{
   if (modelIndex < 0) {
      for (int i = 0; i < getNumberOfBrainModels(); i++) {
         BrainModelVolume* bmv = getBrainModelVolume(i);
         if (bmv != NULL) {
            return bmv;
         }
      }
   }
   else if (modelIndex < getNumberOfBrainModels()) {
      return dynamic_cast<BrainModelVolume*>(brainModels[modelIndex]);
   }
   return NULL;
}
 
/**
 * Get a brain model volume (if negative index first one found is returned) CONST method
 */
const BrainModelVolume* 
BrainSet::getBrainModelVolume(const int modelIndex) const
{
   if (modelIndex < 0) {
      for (int i = 0; i < getNumberOfBrainModels(); i++) {
         const BrainModelVolume* bmv = getBrainModelVolume(i);
         if (bmv != NULL) {
            return bmv;
         }
      }
   }
   else if (modelIndex < getNumberOfBrainModels()) {
      return dynamic_cast<BrainModelVolume*>(brainModels[modelIndex]);
   }
   return NULL;
}
 
/**
 * Get a brain model surface and volume (if negative index first one found is returned).
 */
BrainModelSurfaceAndVolume* 
BrainSet::getBrainModelSurfaceAndVolume(const int modelIndex) 
{
   if (modelIndex < 0) {
      for (int i = 0; i < getNumberOfBrainModels(); i++) {
         BrainModelSurfaceAndVolume* bmsv = getBrainModelSurfaceAndVolume(i);
         if (bmsv != NULL) {
            return bmsv;
         }
      }
   }
   else if (modelIndex < getNumberOfBrainModels()) {
      return dynamic_cast<BrainModelSurfaceAndVolume*>(brainModels[modelIndex]);
   }
   return NULL;
}
 
/**
 * Get a brain model surface and volume (if negative index first one found is returned) CONST method
 */
const BrainModelSurfaceAndVolume* 
BrainSet::getBrainModelSurfaceAndVolume(const int modelIndex) const
{
   if (modelIndex < 0) {
      for (int i = 0; i < getNumberOfBrainModels(); i++) {
         const BrainModelSurfaceAndVolume* bmsv = getBrainModelSurfaceAndVolume(i);
         if (bmsv != NULL) {
            return bmsv;
         }
      }
   }
   else if (modelIndex < getNumberOfBrainModels()) {
      return dynamic_cast<BrainModelSurfaceAndVolume*>(brainModels[modelIndex]);
   }
   return NULL;
}
 
/**
 * add nodes to all surfaces at the origin
 */
void 
BrainSet::addNodes(const int numNodesToAdd)
{
   const float origin[3] = { 0.0, 0.0, 0.0 };
   
   //
   // Add nodes to all coordinate files
   //
   const int numModels = getNumberOfBrainModels();
   for (int i = 0; i < numModels; i++) {
      BrainModelSurface* bms = getBrainModelSurface(i);
      if (bms != NULL) {
         CoordinateFile* cf = bms->getCoordinateFile();
         for (int j = 0; j < numNodesToAdd; j++) {
            cf->addCoordinate(origin);
         }
      }
   }
   
   //
   // Update all topology files
   //
   const int numTopo = getNumberOfTopologyFiles();
   for (int i = 0; i < numTopo; i++) {
      TopologyFile* tf = getTopologyFile(i);
      const int oldNumNodes = tf->getNumberOfNodes();
      const int newNumNodes = oldNumNodes + numNodesToAdd;
      if (newNumNodes > oldNumNodes) {
         tf->setNumberOfNodes(newNumNodes);
      }
   }
   
   //
   // Update the display flags
   //
   updateNodeDisplayFlags();
   
   //
   // Update the node attribute files
   //
   std::vector<NodeAttributeFile*> nodeAttributeFiles;
   nodeAttributeFiles.push_back(arealEstimationFile);
   nodeAttributeFiles.push_back(deformationFieldFile);
   nodeAttributeFiles.push_back(latLonFile);
   nodeAttributeFiles.push_back(rgbPaintFile);
   nodeAttributeFiles.push_back(sectionFile);
   nodeAttributeFiles.push_back(topographyFile);
   for (int i = 0; i < static_cast<int>(nodeAttributeFiles.size()); i++) {
      NodeAttributeFile* naf = nodeAttributeFiles[i];
      if (naf != NULL) {
         if ((naf->getNumberOfNodes() > 0) && (naf->getNumberOfColumns() > 0)) {
            naf->addNodes(numNodesToAdd);
         }
      }
   }
   
   std::vector<GiftiNodeDataFile*> niftiNodeDataFiles;
   niftiNodeDataFiles.push_back(metricFile);
   niftiNodeDataFiles.push_back(paintFile);
   niftiNodeDataFiles.push_back(surfaceShapeFile);
   for (int i = 0; i < static_cast<int>(niftiNodeDataFiles.size()); i++) {
      GiftiNodeDataFile* naf = niftiNodeDataFiles[i];
      if (naf != NULL) {
         if ((naf->getNumberOfNodes() > 0) && (naf->getNumberOfColumns() > 0)) {
            naf->addNodes(numNodesToAdd);
         }
      }
   }

   //
   // Assign node coloring
   //
   nodeColoring->assignColors();
}      

/**
 * Create a brain model surface and volume.
 */
void
BrainSet::createBrainModelSurfaceAndVolume()
{
   //
   // Prevent more than one thread from executing this code
   //
   QMutexLocker locker(&mutexCreateSurfaceAndVolume);
   
   BrainModelSurfaceAndVolume* bmsv = getBrainModelSurfaceAndVolume();
   if (bmsv != NULL) {
      //
      // If no surface attached to surf&vol, add one
      //
      const CoordinateFile* cf = bmsv->getCoordinateFile();
      if (cf->getNumberOfCoordinates() <= 0) {
         const BrainModelSurface* fiducialSurface = 
            getBrainModelSurfaceOfType(BrainModelSurface::SURFACE_TYPE_FIDUCIAL);
         if (fiducialSurface != NULL) {
            bmsv->setSurface();
         }
      }
      return;
   }
   
   if (getNumberOfVolumeAnatomyFiles() > 0) {
      BrainModelSurfaceAndVolume* bmsv = new BrainModelSurfaceAndVolume(this);
      bmsv->initializeSelectedSlices();
      addBrainModel(bmsv);
   }
}

/**
 * get a brain model surface.
 */
BrainModelSurface* 
BrainSet::getBrainModelSurface(const int modelIndex) 
{ 
   if (modelIndex < getNumberOfBrainModels()) {
      if (brainModels[modelIndex]->getModelType() == BrainModel::BRAIN_MODEL_SURFACE) {
         return dynamic_cast<BrainModelSurface*>(brainModels[modelIndex]); 
      }
   }
   return NULL;
}

/**
 * get a brain model surface (const method).
 */
const BrainModelSurface* 
BrainSet::getBrainModelSurface(const int modelIndex) const 
{ 
   if (modelIndex < getNumberOfBrainModels()) {
      if (brainModels[modelIndex]->getModelType() == BrainModel::BRAIN_MODEL_SURFACE) {
         return dynamic_cast<BrainModelSurface*>(brainModels[modelIndex]); 
      }
   }
   return NULL;
}
      
/**
 * get the index of the first brain model surface (returns -1 if not found).
 */
int 
BrainSet::getFirstBrainModelSurfaceIndex() const
{
   for (int i = 0; i < getNumberOfBrainModels(); i++) {
      if (getBrainModelSurface(i) != NULL) {
         return i;
      }
   }
   return -1;
}
      
/**
 * Set the name of the spec file.  Also sets the current directory
 * to the directory of the spec file.  If name is empty, the
 * spec file is cleared.
 */
void
BrainSet::setSpecFileName(const QString& name, const bool readOldSpecFileFlag)
{ 
   //
   // MUST read with the old spec file name and then
   // write with the new spec file name
   //
   const QString oldSpecFileName(specFileName);
   SpecFile sf;
   bool specValidFlag = false;
   if (readOldSpecFileFlag) {
       try {
          sf.readFile(specFileName);
          specValidFlag = true;
       }
       catch (FileException&) {
       }
   }

   specFileName = name;
   if (name.isEmpty() == false) {
      QDir::setCurrent(FileUtilities::dirname(name));
   }
   
   if (specValidFlag) {
      try {
         sf.writeFile(specFileName);
      }
      catch (FileException&) {
      }
   }
}

/**
 * Classify the nodes as interior/edges.  If the topology file is NULL
 * the most appropriate (cut) file will be used.
 */
void
BrainSet::classifyNodes(TopologyFile* tfIn,
                        const bool onlyDoClassificationIfNeeded)
{
   if (onlyDoClassificationIfNeeded) {
      if (nodesHaveBeenClassified) {
         return;
      }
   }
   
   QTime timer;
   timer.start();
   
   TopologyFile* tf = tfIn;
   if (tf == NULL) {
      //
      // Pick most appropriate topology file
      //
      if (topologyCut != NULL) {
         tf = topologyCut;
      }
      else if (topologyLobarCut != NULL) {
         tf = topologyLobarCut;
      }
      else if (topologyOpen != NULL) {
         tf = topologyOpen;
      }
      else if (topologyClosed != NULL) {
         tf = topologyClosed;
      }
      else if (topologyUnknown != NULL) {
         tf = topologyUnknown;
      }
   }
   
   if (tf != NULL) {
      //
      // Default to interior nodes
      //
      const int numNodes = getNumberOfNodes();
      for (int i = 0; i < numNodes; i++) {
         nodeAttributes[i].classification = BrainSetNodeAttribute::CLASSIFICATION_TYPE_INTERIOR;
      }
      
      //
      // Any nodes used by an edge with only one tile is an edge node
      //
      const TopologyHelper* th = tf->getTopologyHelper(true, true, false);
      const std::set<TopologyEdgeInfo>& edges = th->getEdgeInfo();   
      for (std::set<TopologyEdgeInfo>::const_iterator iter = edges.begin();
         iter != edges.end(); iter++) {
         int node1, node2;
         iter->getNodes(node1, node2);
         int tile1, tile2;
         iter->getTiles(tile1, tile2);
         if (tile2 < 0) {
            nodeAttributes[node1].classification = BrainSetNodeAttribute::CLASSIFICATION_TYPE_EDGE;
            nodeAttributes[node2].classification = BrainSetNodeAttribute::CLASSIFICATION_TYPE_EDGE;
         }
      }
      
      //
      // Corner have only two neighbors
      //
      for (int i = 0; i < numNodes; i++) {
         std::vector<int> neighbors;
         th->getNodeNeighbors(i, neighbors);
         if (static_cast<int>(neighbors.size()) == 2) {
            nodeAttributes[i].classification = BrainSetNodeAttribute::CLASSIFICATION_TYPE_CORNER;
         }
      }
      
      nodesHaveBeenClassified = true;
   }
      
   if (DebugControl::getDebugOn()) {
      std::cout << "Time to classify nodes: " 
                << (static_cast<float>(timer.elapsed()) / 1000.0) << std::endl;
   }
   
   updateSurfaceOverlaysDueToChangeInBrainModels();
   clearAllDisplayLists();
}

/**
 * Disconnect the specified nodes and move them to the origin.
 */
void
BrainSet::disconnectNodes(TopologyFile* tf, const std::vector<bool>& nodesToDisconnect)
{
   if (tf == NULL) {
      return;
   }
   tf->deleteTilesWithMarkedNodes(nodesToDisconnect);
   
   //
   // Move disconnected nodes to origin in all surfaces that use the topology file
   //
   const int numDisconnectNodes = static_cast<int>(nodesToDisconnect.size());   
   const float origin[3] = { 0.0, 0.0, 0.0 };
   for (int i = 0; i < getNumberOfBrainModels(); i++) {
      BrainModelSurface* bms = getBrainModelSurface(i);
      if (bms != NULL) {
         if (bms->getTopologyFile() == tf) {
            CoordinateFile* cf = bms->getCoordinateFile();
            int numNodesInCoord = bms->getNumberOfNodes();
            if (numNodesInCoord == numDisconnectNodes) {
               for (int j = 0; j < numNodesInCoord; j++) {
                  if (nodesToDisconnect[j]) {
                     cf->setCoordinate(j, origin);
                  }
               }
            }
         }
      }
   }
}

/**
 * Disconnect the nodes with the specified paint names in the specified paint column.
 */
void 
BrainSet::disconnectNodes(TopologyFile* tf, const std::vector<QString>& paintNames,
                          const int paintColumn)
{
   //
   // Convert the paint names to paint indices
   //
   const PaintFile* pf = getPaintFile();
   std::vector<int> paintIndices;
   for (int i = 0; i < static_cast<int>(paintNames.size()); i++) {
      const int paintIndex = pf->getPaintIndexFromName(paintNames[i]);
      if (paintIndex >= 0) {
         paintIndices.push_back(paintIndex);
      }
   }
   
   //
   // Disconnect nodes with the specified paint indices
   //
   if (paintIndices.empty() == false) {
      disconnectNodes(tf, paintIndices, paintColumn);
   }
}
      
/**
 * Disconnect the nodes with the specified paint indices in the specified paint column.
 */
void 
BrainSet::disconnectNodes(TopologyFile* tf, const std::vector<int>& paintIndices,
                          const int paintColumn)
{
   //
   // Make sure there is something in the paint file
   //
   const PaintFile* pf = getPaintFile();
   const int numNodes = pf->getNumberOfNodes();
   if (numNodes == 0) {
      return;
   }
   
   //
   // Check for valid paint column
   //
   if ((paintColumn >= 0) && (paintColumn < pf->getNumberOfColumns())) {
      //
      // Find the nodes with the paintIndices that should be disconnected
      //
      std::vector<bool> nodesToDisconnect(numNodes, false);
      for (int i = 0; i < numNodes; i++) {
         if (std::find(paintIndices.begin(), paintIndices.end(), pf->getPaint(i, paintColumn))
             != paintIndices.end()) {
            nodesToDisconnect[i] = true;
         }
      }
      
      //
      // Disconnect the nodes
      //
      disconnectNodes(tf, nodesToDisconnect);
   }
}                            

/**
 * convert volume borders to cells.
 */
void 
BrainSet::convertVolumeBordersToFiducialCells()
{
   //
   // Get the border file
   //
   const BorderFile* bf = getVolumeBorderFile();
   const int numBorders = bf->getNumberOfBorders();

   //
   // Get the border and cell color files
   //
   const ColorFile* borderColors = getBorderColorFile();
   ColorFile* cellColors = getCellColorFile();
   
   //
   // Get the fiducial cell files
   //
   CellFile cf;
      
   //
   // Convert the border link points to cells
   //
   for (int i = 0; i < numBorders; i++) {
      //
      // Get the border and info
      //
      const Border* b = bf->getBorder(i);
      const QString borderName(b->getName());
      const int numLinks = b->getNumberOfLinks();
      
      //
      // Transfer color if needed
      //
      const int borderColorIndex = b->getBorderColorIndex();
      if (borderColorIndex >= 0) {
         const QString colorName = borderColors->getColorNameByIndex(borderColorIndex);
         bool exactMatch = false;
         const int colorIndex = cellColors->getColorIndexByName(colorName, exactMatch);
         if (colorIndex < 0) {
            unsigned char r, g, b;
            borderColors->getColorByIndex(borderColorIndex, r, g, b);
            cellColors->addColor(borderName, r, g, b);
         }
      }
      
      //
      // Convert link points to cells
      //
      for (int j = 0; j < numLinks; j++) {
         const float* xyz = b->getLinkXYZ(j);
         CellData cd(borderName, xyz[0], xyz[1], xyz[2], 0);
         if (xyz[0] >= 0) {
             cd.setCellStructure(Structure::STRUCTURE_TYPE_CORTEX_RIGHT);
         }
         else {
             cd.setCellStructure(Structure::STRUCTURE_TYPE_CORTEX_LEFT);
         }
         cf.addCell(cd);
      }
   }
   
   //
   // Append to existing cells
   //
   cellProjectionFile->appendFiducialCellFile(cf);
   
   //
   // Update cell display settings
   //
   displaySettingsCells->update();
}

/**
 * Set the selected topology files
 */
void
BrainSet::setSelectedTopologyFiles()
{
   topologyClosed   = NULL;
   topologyOpen     = NULL;
   topologyCut      = NULL;
   topologyLobarCut = NULL;
   topologyUnknown  = NULL;
   
   const int numTopo = getNumberOfTopologyFiles();
   
   for (int i = 0; i < numTopo; i++) {
      TopologyFile* tf = getTopologyFile(i);
      
      switch(tf->getTopologyType()) {
          case TopologyFile::TOPOLOGY_TYPE_CLOSED:
             if (topologyClosed == NULL) {
                topologyClosed = tf;
             }
             break;
          case TopologyFile::TOPOLOGY_TYPE_OPEN:
             if (topologyOpen == NULL) {
                topologyOpen = tf;
             }
             break;
          case TopologyFile::TOPOLOGY_TYPE_CUT:
             if (topologyCut == NULL) {
                topologyCut = tf;
             }
             break;
          case TopologyFile::TOPOLOGY_TYPE_LOBAR_CUT:
             if (topologyLobarCut == NULL) {
                topologyLobarCut = tf;
             }
             break;
          case TopologyFile::TOPOLOGY_TYPE_UNKNOWN:
          case TopologyFile::TOPOLOGY_TYPE_UNSPECIFIED:
             if (topologyUnknown == NULL) {
                topologyUnknown = tf;
             }
             break;
      }
   }
   
   if (topologyClosed == NULL) {
      if (topologyOpen != NULL) {
         topologyClosed = topologyOpen;
      }
      else if (topologyCut != NULL) {
         topologyClosed = topologyCut;
      }
      else if (topologyLobarCut != NULL) {
         topologyClosed = topologyLobarCut;
      }
      else if (topologyUnknown != NULL) {
         topologyClosed = topologyUnknown;
      }
   }
   
   if (topologyOpen == NULL) {
      if (topologyClosed != NULL) {
         topologyOpen = topologyClosed;
      }
      else if (topologyCut != NULL) {
         topologyOpen = topologyCut;
      }
      else if (topologyLobarCut != NULL) {
         topologyOpen = topologyLobarCut;
      }
      else if (topologyUnknown != NULL) {
         topologyOpen = topologyUnknown;
      }
   }
   
   if (topologyCut == NULL) {
      if (topologyClosed != NULL) {
         topologyCut = topologyClosed;
      }
      else if (topologyOpen != NULL) {
         topologyCut = topologyOpen;
      }
      else if (topologyLobarCut != NULL) {
         topologyCut = topologyLobarCut;
      }
      else if (topologyUnknown != NULL) {
         topologyCut = topologyUnknown;
      }
   }
   
   if (topologyLobarCut == NULL) {
      if (topologyClosed != NULL) {
         topologyLobarCut = topologyClosed;
      }
      else if (topologyOpen != NULL) {
         topologyLobarCut = topologyOpen;
      }
      else if (topologyCut != NULL) {
         topologyLobarCut = topologyCut;
      }
      else if (topologyUnknown != NULL) {
         topologyLobarCut = topologyUnknown;
      }
   }
   
   if (topologyUnknown == NULL) {
      if (topologyClosed != NULL) {
         topologyUnknown = topologyClosed;
      }
      else if (topologyOpen != NULL) {
         topologyUnknown = topologyOpen;
      }
      else if (topologyCut != NULL) {
         topologyUnknown = topologyCut;
      }
      else if (topologyLobarCut != NULL) {
         topologyUnknown = topologyLobarCut;
      }
   }

/*   
   if (getNumberOfClosedTopologyFiles() > 0) {
      topologyClosed   = topologyClosedFiles[0];
      topologyOpen     = topologyClosed;
      topologyCut      = topologyClosed;
      topologyLobarCut = topologyClosed;
   }
   
   if (getNumberOfOpenTopologyFiles() > 0) {
      topologyOpen     = topologyOpenFiles[0];
      topologyCut      = topologyOpen;
      topologyLobarCut = topologyOpen;
      if (topologyClosed == NULL) {
         topologyClosed = topologyOpen;
      }
   }
   
   if (getNumberOfCutTopologyFiles() > 0) {
      topologyCut      = topologyCutFiles[0];
      topologyLobarCut = topologyCut;
      if (topologyClosed == NULL) {
         topologyClosed = topologyCut;
      }
      if (topologyOpen == NULL) {
         topologyOpen = topologyCut;
      }
   }
   
   if (getNumberOfLobarCutTopologyFiles() > 0) {
      topologyLobarCut = topologyLobarCutFiles[0];
      if (topologyClosed == NULL) {
         topologyClosed = topologyLobarCut;
      }
      if (topologyOpen == NULL) {
         topologyOpen = topologyLobarCut;
      }
      if (topologyCut == NULL) {
         topologyCut = topologyLobarCut;
      }
   }
   
   if (getNumberOfUnknownTopologyFiles() > 0) {
      if (topologyLobarCut == NULL) {
         topologyLobarCut = topologyUnknownFiles[0];
      }
      if (topologyClosed == NULL) {
         topologyClosed = topologyUnknownFiles[0];
      }
      if (topologyOpen == NULL) {
         topologyOpen = topologyUnknownFiles[0];
      }
      if (topologyCut == NULL) {
         topologyCut = topologyUnknownFiles[0];
      }
   }
*/
}

/**
 *  Update the file reading progress dialog.  Returns true if the cancel button was pressed
 *  and reading of files should terminate.
 */
bool
BrainSet::updateFileReadProgressDialog(const QString& filename,
                                          int& progressFileCounter,
                                          QProgressDialog* progressDialog)
{
   if (progressDialog != NULL) {
      if (progressDialog->wasCanceled()) {
         readingSpecFileFlag = false;
         return true;
      }
      
      QString s("Reading: ");
      s.append(FileUtilities::basename(filename));
      progressDialog->setLabelText(s);
      progressFileCounter++;
      progressDialog->setValue(progressFileCounter);
      qApp->processEvents();  // note: qApp is global in QApplication
   }
   
   return false;
}

/**
 * create a spec file from all files in the selected scenes.
 */
void 
BrainSet::createSpecFromScenes(const std::vector<int>& sceneIndices,
                               const QString& newSpecFileName,
                               const QString& newSceneFileName,
                               QString& errorMessageOut)
{
   //
   // The new scene file
   //
   SceneFile newSceneFile;
   
   //
   // Create a spec file
   //
   SpecFile newSpecFile;
   
   //
   // Set metadata
   //
   newSpecFile.setStructure(structure.getTypeAsString());
   newSpecFile.setSpecies(species.getName());
   newSpecFile.setSubject(subject);
   newSpecFile.setSpace(stereotaxicSpace);

   //
   // Loop through the scene file
   //
   const int numScenes = static_cast<int>(sceneIndices.size());
   for (int i = 0; i < numScenes; i++) {
      //
      // Get the scene
      //
      const SceneFile::Scene* scene = sceneFile->getScene(sceneIndices[i]);
      
      //
      // Add the scene to the new scene file
      //
      newSceneFile.addScene(*scene);
      
      //
      // Get the scenes spec file entries and add to new spec file
      //
      SpecFile sf;
      sf.showScene(*scene, errorMessageOut);
      newSpecFile.append(sf);
   }
   
   //
   // Write the new scene file
   //
   try {
      newSceneFile.writeFile(newSceneFileName);
   }
   catch (FileException& e) {
      errorMessageOut = e.whatQString();
      return;
   }
   
   //
   // Add the scene to the spec file 
   //
   newSpecFile.addToSpecFile(SpecFile::getSceneFileTag(),
                             FileUtilities::basename(newSceneFileName),
                             "",
                             false);

   //
   // Write the new spec file
   //
   try {
      newSpecFile.writeFile(newSpecFileName);
   }
   catch (FileException& e) {
      errorMessageOut = e.whatQString();
      return;
   }   
}

/**
 * Add a tag and file to the spec file.
 */
void
BrainSet::addToSpecFile(const QString& specFileTag, const QString& fileName,
                        const QString& fileName2)
{
   //
   // The remainder of this routine must not be run by more than one thread
   //
   QMutexLocker locker(&mutexAddToSpecFile);
   
   //
   // Do not call sf.setStructure() since Unknown screws things up.
   //
   switch (structure.getType()) {
      case Structure::STRUCTURE_TYPE_CEREBELLUM:
         loadedFilesSpecFile.setStructure(Structure::getCerebellumAsString());
         break;
      case Structure::STRUCTURE_TYPE_CEREBELLUM_OR_CORTEX_LEFT:
         loadedFilesSpecFile.setStructure(Structure::getCerebellumOrLeftCerebralAsString());
         break;
      case Structure::STRUCTURE_TYPE_CEREBELLUM_OR_CORTEX_RIGHT:
         loadedFilesSpecFile.setStructure(Structure::getCerebellumOrRightCerebralAsString());
         break;
      case Structure::STRUCTURE_TYPE_CORTEX_LEFT:
         loadedFilesSpecFile.setStructure(Structure::getCortextLeftAsString());
         break;
      case Structure::STRUCTURE_TYPE_CORTEX_RIGHT:
         loadedFilesSpecFile.setStructure(Structure::getCortexRightAsString());
         break;
      case Structure::STRUCTURE_TYPE_CORTEX_BOTH:
         loadedFilesSpecFile.setStructure(Structure::getCortexBothAsString());
         break;
      case Structure::STRUCTURE_TYPE_CORTEX_LEFT_OR_CEREBELLUM:
         loadedFilesSpecFile.setStructure(Structure::getLeftCerebralOrCerebellumAsString());
         break;
      case Structure::STRUCTURE_TYPE_CORTEX_RIGHT_OR_CEREBELLUM:
         loadedFilesSpecFile.setStructure(Structure::getRightCerebralOrCerebellumAsString());
         break;
      case Structure::STRUCTURE_TYPE_CEREBRUM_CEREBELLUM:
         loadedFilesSpecFile.setStructure(Structure::getCerebrumAndCerebellumAsString());
         break;
      case Structure::STRUCTURE_TYPE_SUBCORTICAL:
         loadedFilesSpecFile.setStructure(Structure::getSubCorticalAsString());
         break;
      case Structure::STRUCTURE_TYPE_ALL:
         loadedFilesSpecFile.setStructure(Structure::getAllAsString());
         break;
      case Structure::STRUCTURE_TYPE_INVALID:
         // do not override if unknown
         break;
   }
   loadedFilesSpecFile.setSpecies(getSpecies().getName());
   loadedFilesSpecFile.setSubject(getSubject());
   //sf.setSpace(getSpace());
   //sf.setCategory(getCategory());
   
   //
   // add to spec file that keeps track of loaded files but do not write the spec file
   //
   loadedFilesSpecFile.addToSpecFile(specFileTag, fileName, fileName2, false);
   
   //
   // Update the spec file on the disk
   //
   if (readingSpecFileFlag == false) {
      if (specFileName.isEmpty() == false) {
         SpecFile sf;
         try {
            sf.readFile(specFileName);
         }
         catch (FileException& /*e*/) {
            // since file may not yet exist, ignore read error
         }
         sf.setFileName(specFileName);
         if (loadedFilesSpecFile.getStructure().isValid()) {
            sf.setStructure(loadedFilesSpecFile.getStructure());
         }
         if (loadedFilesSpecFile.getSpecies().isValid()) {
            sf.setSpecies(loadedFilesSpecFile.getSpecies().getName());
         }
         if (loadedFilesSpecFile.getSubject().isEmpty() == false) {
            sf.setSubject(loadedFilesSpecFile.getSubject());
         }
         sf.addToSpecFile(specFileTag, fileName, fileName2, true);
      }
   }
}
 
/** 
 * Write the node color file.
 */ 
void
BrainSet::writeAreaColorFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.areaColorFile.setAllSelections(SpecFile::SPEC_FALSE);
   areaColorFile->writeFile(name);
   addToSpecFile(SpecFile::getAreaColorFileTag(), name);
}
/**
 * Read the area color data file file
 */
void 
BrainSet::readAreaColorFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexAreaColorFile);   
   
   if (append == false) {
      clearAreaColorFile();
   }
   const unsigned long modified = areaColorFile->getModified();
   
   if (areaColorFile->getNumberOfColors() == 0) {         
      try {
         areaColorFile->readFile(name);
      }
      catch (FileException& e) {
         clearAreaColorFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      AreaColorFile cf;
      cf.readFile(name);
      QString msg;
      areaColorFile->append(cf);
   }
   
   areaColorFile->setModifiedCounter(modified);
   paintFile->getLabelTable()->assignColors(*areaColorFile);

   if (updateSpec) {
      addToSpecFile(SpecFile::getAreaColorFileTag(), name);
   }
}

/**
 * add a document file.
 */
void 
BrainSet::addDocumentFile(const QString& documentFileName)
{
   addToSpecFile(SpecFile::getDocumentFileTag(), documentFileName);
}
      
/** 
 * Write the areal estimation data file.
 */ 
void
BrainSet::writeArealEstimationFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.arealEstimationFile.setAllSelections(SpecFile::SPEC_FALSE);
   arealEstimationFile->writeFile(name);
   addToSpecFile(SpecFile::getArealEstimationFileTag(), name);
}

/**      
 * Read the areal estimation data file file
 */
void 
BrainSet::readArealEstimationFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexArealEstimationFile);
   
   if (append == false) {
      clearArealEstimationFile();
   }
   const unsigned long modified = arealEstimationFile->getModified();
      
   if (arealEstimationFile->getNumberOfColumns() == 0) {         
      try {
         arealEstimationFile->readFile(name);
         if (arealEstimationFile->getNumberOfNodes() != getNumberOfNodes()) {
            throw FileException(FileUtilities::basename(name), numNodesMessage);
         }
      }
      catch (FileException& e) {
         clearArealEstimationFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing areal estimations
      ArealEstimationFile aef;
      aef.readFile(name);
      if (aef.getNumberOfNodes() != getNumberOfNodes()) {
         throw FileException(FileUtilities::basename(name), numNodesMessage);
      }
      try {
         arealEstimationFile->append(aef);
      }
      catch (FileException& e) {
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   
   arealEstimationFile->setModifiedCounter(modified);
   displaySettingsArealEstimation->update();  

   if (updateSpec) {
      addToSpecFile(SpecFile::getArealEstimationFileTag(), name);
   }
}
      
/**
 * read the areal estimation data file file (only selected columns).
 */
void 
BrainSet::readArealEstimationFile(const QString& name, 
                         const std::vector<int>& columnDestination,
                         const std::vector<QString>& fileBeingReadColumnNames,
                         const AbstractFile::FILE_COMMENT_MODE fcm,
                         const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexArealEstimationFile);
   
   const bool arealEstimationFileEmpty = arealEstimationFile->empty();
   
   ArealEstimationFile aef;
   aef.readFile(name);
   if (aef.getNumberOfNodes() != getNumberOfNodes()) {
      throw FileException(FileUtilities::basename(name), numNodesMessage);
   }
   try {
      for (int i = 0; i < aef.getNumberOfColumns(); i++) {
         if (i < static_cast<int>(fileBeingReadColumnNames.size())) {
            aef.setColumnName(i, fileBeingReadColumnNames[i]);
         }
      }
      std::vector<int> columnDestination2 = columnDestination;
      arealEstimationFile->append(aef, columnDestination2, fcm);
   }
   catch (FileException& e) {
      throw FileException(FileUtilities::basename(name), e.whatQString());
   }
   if (arealEstimationFileEmpty) {
      arealEstimationFile->clearModified();
   }
   else {
      arealEstimationFile->setModified();
   }

   if (readingSpecFileFlag == false) {
      displaySettingsArealEstimation->update(); 
   }
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getArealEstimationFileTag(), name);
   }
}

/**
 * read a volume border file.
 */
void 
BrainSet::readVolumeBorderFile(const QString& name,
                               const bool append,
                               const bool updateSpec) throw (FileException)
{
   BorderFile* borderFile = getVolumeBorderFile();
   
   QMutexLocker locker(&mutexVolumeBorderFile);
   
   if (append || (borderFile->getNumberOfBorders() <= 0)) {
   
      borderFile->readFile(name);
   }
   else {
      BorderFile bf;
      bf.readFile(name);
      borderFile->append(bf);
   }

   if (updateSpec) {
      addToSpecFile(SpecFile::getVolumeBorderFileTag(), name);
   }
}

/**
 * write a volume border file.
 */
void 
BrainSet::writeVolumeBorderFile(const QString& name,
                                const bool removeDuplicates) throw (FileException)
{
   loadedFilesSpecFile.volumeBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
   BorderFile* borderFile = getVolumeBorderFile();
   
   if (removeDuplicates) {
      std::vector<int> indices;
      borderFile->getDuplicateBorderIndices(indices);
      borderFile->removeBordersWithIndices(indices);
   }
   
   borderFile->setHeaderTag(AbstractFile::headerTagConfigurationID, "VOLUME");
   
   borderFile->writeFile(name);
   addToSpecFile(SpecFile::getVolumeBorderFileTag(), name);
}                                 

/** 
 * Write the border data file.
 */ 
void
BrainSet::writeBorderFile(const QString& name,
                          const BrainModelSurface* bms,
                          const BrainModelSurface::SURFACE_TYPES borderFileType,
                          const QString& commentText,
                          const QString& pubMedID,
                          const bool removeDuplicates) throw (FileException)
{
   //
   // Find borders that are used on the specified surface
   //
   BorderFile borderFile;   
   borderFile.setFileComment(commentText);
   borderFile.setFilePubMedID(pubMedID);
   brainModelBorderSet->copyBordersToBorderFile(bms, borderFile);
   if (removeDuplicates) {
      std::vector<int> indices;
      borderFile.getDuplicateBorderIndices(indices);
      borderFile.removeBordersWithIndices(indices);
   }
   borderFile.setHeaderTag(AbstractFile::headerTagConfigurationID,
      BrainModelSurface::getSurfaceConfigurationIDFromType(borderFileType));
   
   if (borderFile.getNumberOfBorders() <= 0) {
      QString msg("There are no borders for surface ");
      msg.append(FileUtilities::basename(bms->getFileName()));
      throw FileException(name, msg);
   }
   
   QString tag;
   
   switch(borderFileType) {
      case BrainModelSurface::SURFACE_TYPE_RAW:
         tag = SpecFile::getRawBorderFileTag();
         loadedFilesSpecFile.rawBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
         break;
      case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
         tag = SpecFile::getFiducialBorderFileTag();
         loadedFilesSpecFile.fiducialBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
         break;
      case BrainModelSurface::SURFACE_TYPE_INFLATED:
         tag = SpecFile::getInflatedBorderFileTag();
         loadedFilesSpecFile.inflatedBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
         break;
      case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
         tag = SpecFile::getVeryInflatedBorderFileTag();
         loadedFilesSpecFile.veryInflatedBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
         break;
      case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
         tag = SpecFile::getSphericalBorderFileTag();
         loadedFilesSpecFile.sphericalBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
         break;
      case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
         tag = SpecFile::getEllipsoidBorderFileTag();
         loadedFilesSpecFile.ellipsoidBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
         break;
      case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         tag = SpecFile::getCompressedBorderFileTag();
         loadedFilesSpecFile.compressedBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT:
         tag = SpecFile::getFlatBorderFileTag();
         loadedFilesSpecFile.flatBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
         tag = SpecFile::getLobarFlatBorderFileTag();
         loadedFilesSpecFile.lobarFlatBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
         break;
      case BrainModelSurface::SURFACE_TYPE_HULL:
         tag = SpecFile::getHullCoordFileTag();
         loadedFilesSpecFile.hullBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
         break;
      case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
      case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
      default:
         tag = SpecFile::getUnknownBorderFileMatchTag();
         loadedFilesSpecFile.unknownBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
         break;
   }
   
   borderFile.setHeaderTag(AbstractFile::headerTagConfigurationID,
                  BrainModelSurface::getSurfaceConfigurationIDFromType(bms->getSurfaceType()));
                            
   borderFile.writeFile(name);
   addToSpecFile(tag, name);
   BrainModelBorderFileInfo* bmi = brainModelBorderSet->getBorderFileInfo(borderFileType);
   bmi->setFileName(name);
   bmi->setFileComment(commentText);
   bmi->setPubMedID(pubMedID);
   brainModelBorderSet->setSurfaceBordersModified(bms, false);
}

/**      
 * Read the border data file file
 */
void 
BrainSet::readBorderFile(const QString& name, const BrainModelSurface::SURFACE_TYPES stin,
                         const bool append,
                         const bool updateSpec) throw (FileException)
{
   BrainModelSurface::SURFACE_TYPES st = stin;
   if (st == BrainModelSurface::SURFACE_TYPE_UNSPECIFIED) {
      BorderFile b;
      b.readFileMetaDataOnly(name);
      const QString typeTag = b.getHeaderTag(AbstractFile::headerTagConfigurationID);
      if (typeTag.isEmpty() == false) {
         st = BrainModelSurface::getSurfaceTypeFromConfigurationID(typeTag);
      }
   }
   
   QString tag;
   
   switch(st) {
      case BrainModelSurface::SURFACE_TYPE_RAW:
         tag = SpecFile::getRawBorderFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
         tag = SpecFile::getFiducialBorderFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_INFLATED:
         tag = SpecFile::getInflatedBorderFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
         tag = SpecFile::getVeryInflatedBorderFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
         tag = SpecFile::getSphericalBorderFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
         tag = SpecFile::getEllipsoidBorderFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         tag = SpecFile::getCompressedBorderFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT:
         tag = SpecFile::getFlatBorderFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
         tag = SpecFile::getLobarFlatBorderFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_HULL:
         tag = SpecFile::getHullBorderFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
      case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
         tag = SpecFile::getUnknownBorderFileMatchTag();
         break;
   }
   
   BorderFile borderFile;
   try {
      borderFile.readFile(name);
   }
   catch (FileException& e) {
      borderFile.clear();
      throw FileException(FileUtilities::basename(name), e.whatQString());
   }
   
   //
   // Update configuration ID in the border file
   //
   const QString configID = 
      BrainModelSurface::getSurfaceConfigurationIDFromType(st);
   borderFile.setHeaderTag(AbstractFile::headerTagConfigurationID, configID);
   borderFile.clearModified();

   //
   // If transformation matrix should be applied
   //
   if ((st == BrainModelSurface::SURFACE_TYPE_RAW) ||
       (st == BrainModelSurface::SURFACE_TYPE_FIDUCIAL)) {
      if (specDataFileTransformationMatrix.isIdentity() == false) {
         borderFile.applyTransformationMatrix(specDataFileTransformationMatrix);
         borderFile.clearModified();
      }
   }
   
   //
   // Add to current borders.
   //
   QMutexLocker locker(&mutexBorderAndBorderProjectionFile);

   if (append == false) {
      deleteAllBorders();
   }
   
   brainModelBorderSet->copyBordersFromBorderFile(&borderFile, st);
   
   if (readingSpecFileFlag == false) {
      displaySettingsBorders->update();   
   }
   
   if (updateSpec) {
      addToSpecFile(tag, name);
   }
}
      
/** 
 * Write the border color data file.
 */ 
void
BrainSet::writeBorderColorFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.borderColorFile.setAllSelections(SpecFile::SPEC_FALSE);
   borderColorFile->writeFile(name);
   addToSpecFile(SpecFile::getBorderColorFileTag(), name);
}

/**      
 * Read the border color data file file
 */
void 
BrainSet::readBorderColorFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexBorderColorFile);

   if (append == false) {
      clearBorderColorFile();
   }
   
   const unsigned long modified = borderColorFile->getModified();
   
   if (borderColorFile->getNumberOfColors() == 0) {         
      try {
         borderColorFile->readFile(name);
      }
      catch (FileException& e) {
         clearBorderColorFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      BorderColorFile cf;
      cf.readFile(name);
      borderColorFile->append(cf);
   }
   
   borderColorFile->setModifiedCounter(modified);
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getBorderColorFileTag(), name);
   }
}
      
/** 
 * Write the border projection data file.
 */ 
void
BrainSet::writeBorderProjectionFile(const QString& name,
                                    const QString& commentText,
                                    const QString& pubMedID,
                                    const bool removeDuplicates) throw (FileException)
{
   loadedFilesSpecFile.borderProjectionFile.setAllSelections(SpecFile::SPEC_FALSE);
   BorderProjectionFile borderProjFile;
   brainModelBorderSet->copyBordersToBorderProjectionFile(borderProjFile);
   if (removeDuplicates) {
      std::vector<int> indices;
      borderProjFile.getDuplicateBorderProjectionIndices(indices);
      borderProjFile.removeBordersWithIndices(indices);
   }
   borderProjFile.setFileComment(commentText);
   borderProjFile.setFilePubMedID(pubMedID);
   borderProjFile.writeFile(name);
   addToSpecFile(SpecFile::getBorderProjectionFileTag(), name);
   BrainModelBorderFileInfo* bmi = brainModelBorderSet->getBorderProjectionFileInfo();
   bmi->setFileName(name);
   bmi->setFileComment(commentText);
   bmi->setPubMedID(pubMedID);
   brainModelBorderSet->setProjectionsModified(false);
}

/**      
 * Read the border projection data file file
 */
void 
BrainSet::readBorderProjectionFile(const QString& name, 
                                   const bool append,
                                   const bool updateSpec) throw (FileException)
{
   BorderProjectionFile borderProjFile;         
   try {
      borderProjFile.readFile(name);
   }
   catch (FileException& e) {
      throw FileException(FileUtilities::basename(name), e.whatQString());
   }

   QMutexLocker locker(&mutexBorderAndBorderProjectionFile);
   
   if (append == false) {
      deleteAllBorders();
   }
   bool modified = brainModelBorderSet->getProjectionsModified();
   
   const bool hadBorders = (brainModelBorderSet->getNumberOfBorders() > 0);
   brainModelBorderSet->copyBordersFromBorderProjectionFile(&borderProjFile);
   brainModelBorderSet->setProjectionsModified(modified);

   if (readingSpecFileFlag == false) {
      displaySettingsBorders->update();   
   }
   
   if (hadBorders == false) {
      brainModelBorderSet->setAllBordersModifiedStatus(false);
      brainModelBorderSet->setProjectionsModified(false);
   }
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getBorderProjectionFileTag(), name);
   }
}
     
/** 
 * Write the cell data file.
 */ 
void
BrainSet::writeCellFile(const QString& name,
                        const BrainModelSurface* bms,
                        const AbstractFile::FILE_FORMAT fileFormat,
                        const QString& commentText) throw (FileException)
{
   //
   // Check for fiducial surface
   //
   const bool fiducialSurfaceFlag = 
      ((bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_RAW) ||
       (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL));
       
   //
   // Get cells for surface
   //
   CellFile cf;
   cellProjectionFile->getCellFile(bms->getCoordinateFile(),
                                   bms->getTopologyFile(),
                                   fiducialSurfaceFlag,
                                   cf);
   if (cf.getNumberOfCells() <= 0) {
      throw FileException("There are no cells that project to the selected surface.");
   }
   cf.setFileComment(commentText);
   cf.setFileWriteType(fileFormat);
   cf.writeFile(name);
   addToSpecFile(SpecFile::getCellFileTag(), name);
}

/**      
 * Read the cell data file file
 */
void 
BrainSet::readCellFile(const QString& name, 
                       const bool append,
                       const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexCellAndCellProjectionFile);
   
   if (append == false) {
      //deleteCellsOfType(st);
      deleteAllCells(true, true);
   }
   
   CellFile cellFile;
   try {
      cellFile.readFile(name);
      //
      // If transformation should be applied
      //
      if (specDataFileTransformationMatrix.isIdentity() == false) {
         cellFile.applyTransformationMatrix(std::numeric_limits<int>::min(),
                                             std::numeric_limits<int>::max(),
                                             specDataFileTransformationMatrix,
                                             false);
      }
   }
   catch (FileException& e) {
      throw FileException(FileUtilities::basename(name), e.whatQString());
   }

   cellProjectionFile->appendFiducialCellFile(cellFile);

   if (updateSpec) {
      addToSpecFile(SpecFile::getCellFileTag(), name);
   }
}
      
/** 
 * Write the cell data file.
 */ 
void
BrainSet::writeVolumeCellFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.volumeCellFile.setAllSelections(SpecFile::SPEC_FALSE);

   CellFile* cf = getVolumeCellFile();
   if (cf != NULL) {
      cf->writeFile(name);
      addToSpecFile(SpecFile::getVolumeCellFileTag(), name);
   }
   else {
      throw FileException("", "There is no volume cell file to write.");
   }
}

/**      
 * Read the cell data file file
 */
void 
BrainSet::readVolumeCellFile(const QString& name, 
                             const bool append,
                             const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexVolumeCellFile);
   
   if (append == false) {
      deleteAllCells(false, true);
   }
   
   const unsigned long modified = volumeCellFile->getModified();
   
   if (volumeCellFile->getNumberOfCells() == 0) {         
      try {
         volumeCellFile->readFile(name);
         //
         // If transformation should be applied
         //
         if (specDataFileTransformationMatrix.isIdentity() == false) {
            volumeCellFile->applyTransformationMatrix(std::numeric_limits<int>::min(),
                                                std::numeric_limits<int>::max(),
                                                specDataFileTransformationMatrix,
                                                false);
         }
      }
      catch (FileException& e) {
         deleteAllCells(false, true);
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      CellFile cf;
      cf.readFile(name);
      //
      // If transformation should be applied
      //
      if (specDataFileTransformationMatrix.isIdentity() == false) {
         cf.applyTransformationMatrix(std::numeric_limits<int>::min(),
                                             std::numeric_limits<int>::max(),
                                             specDataFileTransformationMatrix,
                                             false);
      }
      volumeCellFile->append(cf);
   }
   volumeCellFile->setModifiedCounter(modified);

   displaySettingsCells->update();

   if (updateSpec) {
      addToSpecFile(SpecFile::getVolumeCellFileTag(), name);
   }
}
      
/** 
 * Write the cell color data file.
 */ 
void
BrainSet::writeCellColorFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.cellColorFile.setAllSelections(SpecFile::SPEC_FALSE);
   cellColorFile->writeFile(name);
   addToSpecFile(SpecFile::getCellColorFileTag(), name);
}

/**      
 * Read the cell color data file file
 */
void 
BrainSet::readCellColorFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexCellColorFile);
   
   if (append == false) {
      clearCellColorFile();
   }
   const unsigned long modified = cellColorFile->getModified();
   
   if (cellColorFile->getNumberOfColors() == 0) {         
      try {
         cellColorFile->readFile(name);
      }
      catch (FileException& e) {
         clearCellColorFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      CellColorFile cf;
      cf.readFile(name);
      //QString msg;
      cellColorFile->append(cf);
   }
   
   cellColorFile->setModifiedCounter(modified);
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getCellColorFileTag(), name);
   }
}
      
/** 
 * Write the cell projection data file.
 */ 
void
BrainSet::writeCellProjectionFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.cellProjectionFile.setAllSelections(SpecFile::SPEC_FALSE);
   cellProjectionFile->writeFile(name);
   addToSpecFile(SpecFile::getCellProjectionFileTag(), name);
}

/**      
 * Read the cell projection data file file
 */
void 
BrainSet::readCellProjectionFile(const QString& name, 
                                 const bool append,
                                 const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexCellAndCellProjectionFile);
   
   if (append == false) {
      deleteAllCellProjections();
   }
   const unsigned long modified = cellProjectionFile->getModified();
   
   if (cellProjectionFile->getNumberOfCellProjections() == 0) {         
      try {
         cellProjectionFile->readFile(name);
      }
      catch (FileException& e) {
         deleteAllCellProjections();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      CellProjectionFile cf;
      cf.readFile(name);
      QString msg;
      cellProjectionFile->append(cf);
   }
   
   cellProjectionFile->setModifiedCounter(modified);
   displaySettingsCells->update();
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getCellProjectionFileTag(), name);
   }
}

/** 
 * Write the CocomacConnectivity data file.
 */ 
void
BrainSet::writeCocomacConnectivityFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.cocomacConnectivityFile.setAllSelections(SpecFile::SPEC_FALSE);
   cocomacFile->writeFile(name);
   addToSpecFile(SpecFile::getCocomacConnectivityFileTag(), name);
}

/**
 * Read a Cocomac Connectivity file.
 */
void
BrainSet::readCocomacConnectivityFile(const QString& name, const bool append,
                                      const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexCocomacFile);
   
   if (append == false) {
      clearCocomacConnectivityFile();
   }
   const unsigned long modified = cocomacFile->getModified();
   
   if (cocomacFile->getNumberOfCocomacProjections() == 0) {         
      try {
         cocomacFile->readFile(name);
      }
      catch (FileException& e) {
         clearCocomacConnectivityFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      CocomacConnectivityFile cf;
      cf.readFile(name);
      QString msg;
      cocomacFile->append(cf, msg);
      if (msg.isEmpty() == false) {
         throw FileException(FileUtilities::basename(name), msg);
      }
   }
   
   cocomacFile->setModifiedCounter(modified);
   displaySettingsCoCoMac->update();
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getCocomacConnectivityFileTag(), name);
   }
}

/** 
 * Write the contour data file.
 */ 
void
BrainSet::writeContourFile(const QString& name, ContourFile* cf) throw (FileException)
{
   loadedFilesSpecFile.contourFile.setAllSelections(SpecFile::SPEC_FALSE);
   cf->writeFile(name);
   addToSpecFile(SpecFile::getContourFileTag(), name);
}

/**
 * Read a Contour file.
 */
void
BrainSet::readContourFile(const QString& name, 
                          const bool append,
                          const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexContourFile);
   
   if (append == false) {
      clearContourFile(false);
   }
   
   BrainModelContours* bmc = getBrainModelContours(-1);
   bool createdBrainModelContours = false;
   if (bmc == NULL) {
      bmc = new BrainModelContours(this);
      createdBrainModelContours = true;
   }
   try {
      bmc->readContourFile(name, append);
      if (createdBrainModelContours) {
         addBrainModel(bmc);
      }
      if (updateSpec) {
         addToSpecFile(SpecFile::getContourFileTag(), name);
      }
   }
   catch (FileException& e) {
      if (createdBrainModelContours) {
         delete bmc;
      }
      throw e;
   }
   displaySettingsContours->update();
}

/** 
 * Write the contour cell data file.
 */ 
void
BrainSet::writeContourCellFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.contourCellFile.setAllSelections(SpecFile::SPEC_FALSE);
   contourCellFile->writeFile(name);
   addToSpecFile(SpecFile::getContourCellFileTag(), name);
}

/**      
 * Read the contour cell data file
 */
void 
BrainSet::readContourCellFile(const QString& name, 
                              const bool append,
                              const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexContourCellFile);     
   
   if (append == false) {
      clearContourCellFile();
   }
   const unsigned long modified = contourCellFile->getModified();
   
   if (contourCellFile->getNumberOfCells() == 0) {    
      try {
         contourCellFile->readFile(name);
      }
      catch (FileException& e) {
         clearContourCellFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      ContourCellFile cf;
      cf.readFile(name);
      contourCellFile->append(cf);
   }

   contourCellFile->setModifiedCounter(modified);
   displaySettingsCells->update();
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getContourCellFileTag(), name);
   }
}

/** 
 * Write the contour cell color data file.
 */ 
void
BrainSet::writeContourCellColorFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.contourCellColorFile.setAllSelections(SpecFile::SPEC_FALSE);
   contourCellColorFile->writeFile(name);
   addToSpecFile(SpecFile::getContourCellColorFileTag(), name);
}

/**      
 * Read the contour cell color data file
 */
void 
BrainSet::readContourCellColorFile(const QString& name, 
                                   const bool append,
                                   const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexContourCellColorFile);
   
   if (append == false) {
      clearContourCellColorFile();
   }
   const unsigned long modified = contourCellColorFile->getModified();
   
   if (contourCellColorFile->getNumberOfColors() == 0) {         
      try {
         contourCellColorFile->readFile(name);
      }
      catch (FileException& e) {
         clearContourCellColorFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      ContourCellColorFile cf;
      cf.readFile(name);
      contourCellColorFile->append(cf);
   }
   contourCellColorFile->setModifiedCounter(modified);
   displaySettingsContours->update();
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getContourCellColorFileTag(), name);
   }
}

/**
 * Set the deformation map file name.
 */
void 
BrainSet::setDeformationMapFileName(const QString& name, const bool updateSpec)
{
   deformationMapFileName = name;
   if (updateSpec) {
      addToSpecFile(SpecFile::getDeformationMapFileTag(), name);
   }
}      
      
/** 
 * Write the cuts data file.
 */ 
void
BrainSet::writeCutsFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.cutsFile.setAllSelections(SpecFile::SPEC_FALSE);
   cutsFile->writeFile(name);
   addToSpecFile(SpecFile::getCutsFileTag(), name);
}

/**
 * Read a cuts  file.
 */
void
BrainSet::readCutsFile(const QString& name, const bool append,
                       const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexCutsFile);
   
   if (append == false) {
      clearCutsFile();
   }
   const unsigned long modified = cutsFile->getModified();
   
   if (cutsFile->getNumberOfBorders() == 0) {         
      try {
         cutsFile->readFile(name);
      }
      catch (FileException& e) {
         clearCutsFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      CutsFile cf;
      cf.readFile(name);
      QString msg;
      cutsFile->append(cf);
      if (msg.isEmpty() == false) {
         throw FileException(FileUtilities::basename(name), msg);
      }
   }
   cutsFile->setModifiedCounter(modified);
   displaySettingsCuts->update();
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getCutsFileTag(), name);
   }
} 

/**
 * Remove any surfaces of the specified type from the brain models.
 */
void
BrainSet::deleteSurfacesOfType(const BrainModelSurface::SURFACE_TYPES st)
{
   std::vector<BrainModelSurface*> modelsToDelete;
   
   for (int i = 0; i < getNumberOfBrainModels(); i++) {
      BrainModel* bm = getBrainModel(i);
      if (bm->getModelType() == BrainModel::BRAIN_MODEL_SURFACE) {
         BrainModelSurface* bms = dynamic_cast<BrainModelSurface*>(bm);
         if (bms->getSurfaceType() != st) {
            modelsToDelete.push_back(bms);
         }
      }
   }
   
   for (unsigned int i = 0; i < modelsToDelete.size(); i++) {
      deleteBrainModelSurface(modelsToDelete[i]);
   }
   
   nodeColoring->assignColors();
   clearAllDisplayLists();
}

/**
 * write the surface data file.
 */
void 
BrainSet::writeSurfaceFile(const QString& name, 
                           const BrainModelSurface::SURFACE_TYPES st,
                           BrainModelSurface* bms,
                           const bool updateSpecFile,
                           const AbstractFile::FILE_FORMAT fileFormat) throw (FileException)
{
   QString oldFileName = FileUtilities::filenameWithoutExtension(
                                        bms->getCoordinateFile()->getFileName());
   oldFileName.append(SpecFile::getGiftiSurfaceFileExtension());
   
   const BrainModelSurface::SURFACE_TYPES st1 = bms->getSurfaceType();
   switch (st1) {
      case BrainModelSurface::SURFACE_TYPE_RAW:
         loadedFilesSpecFile.rawSurfaceFile.clearSelectionStatus(oldFileName);
         break;
      case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
         loadedFilesSpecFile.fiducialSurfaceFile.clearSelectionStatus(oldFileName);
         break;
      case BrainModelSurface::SURFACE_TYPE_INFLATED:
         loadedFilesSpecFile.inflatedSurfaceFile.clearSelectionStatus(oldFileName);
         break;
      case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
         loadedFilesSpecFile.veryInflatedSurfaceFile.clearSelectionStatus(oldFileName);
         break;
      case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
         loadedFilesSpecFile.sphericalSurfaceFile.clearSelectionStatus(oldFileName);
         break;
      case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
         loadedFilesSpecFile.ellipsoidSurfaceFile.clearSelectionStatus(oldFileName);
         break;
      case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         loadedFilesSpecFile.compressedSurfaceFile.clearSelectionStatus(oldFileName);
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT:
         loadedFilesSpecFile.flatSurfaceFile.clearSelectionStatus(oldFileName);
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
         loadedFilesSpecFile.lobarFlatSurfaceFile.clearSelectionStatus(oldFileName);
         break;
      case BrainModelSurface::SURFACE_TYPE_HULL:
         loadedFilesSpecFile.hullSurfaceFile.clearSelectionStatus(oldFileName);
         break;
      case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
      case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
         loadedFilesSpecFile.unknownSurfaceFile.clearSelectionStatus(oldFileName);
         break;
   }

   //
   // Update surface to new type
   //
   bms->setSurfaceType(st);
   
   //
   // Write the surface file
   //
   bms->writeSurfaceFile(name, fileFormat);
   
   if (updateSpecFile) {
      addToSpecFile(BrainModelSurface::getSurfaceSpecFileTagFromSurfaceType(st), name);
   }
}

/** 
 * Write the coordinate data file.
 */ 
void
BrainSet::writeCoordinateFile(const QString& name, 
                              const BrainModelSurface::SURFACE_TYPES st,
                              CoordinateFile* cf,
                              const bool updateSpecFile) throw (FileException)
{
   const QString typeTag = cf->getHeaderTag(AbstractFile::headerTagConfigurationID);
   if (typeTag.isEmpty() == false) {
      const BrainModelSurface::SURFACE_TYPES st1 = 
                               BrainModelSurface::getSurfaceTypeFromConfigurationID(typeTag);
      switch (st1) {
         case BrainModelSurface::SURFACE_TYPE_RAW:
            loadedFilesSpecFile.rawCoordFile.clearSelectionStatus(cf->getFileName());
            break;
         case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
            loadedFilesSpecFile.fiducialCoordFile.clearSelectionStatus(cf->getFileName());
            break;
         case BrainModelSurface::SURFACE_TYPE_INFLATED:
            loadedFilesSpecFile.inflatedCoordFile.clearSelectionStatus(cf->getFileName());
            break;
         case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
            loadedFilesSpecFile.veryInflatedCoordFile.clearSelectionStatus(cf->getFileName());
            break;
         case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
            loadedFilesSpecFile.sphericalCoordFile.clearSelectionStatus(cf->getFileName());
            break;
         case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
            loadedFilesSpecFile.ellipsoidCoordFile.clearSelectionStatus(cf->getFileName());
            break;
         case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
            loadedFilesSpecFile.compressedCoordFile.clearSelectionStatus(cf->getFileName());
            break;
         case BrainModelSurface::SURFACE_TYPE_FLAT:
            loadedFilesSpecFile.flatCoordFile.clearSelectionStatus(cf->getFileName());
            break;
         case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
            loadedFilesSpecFile.lobarFlatCoordFile.clearSelectionStatus(cf->getFileName());
            break;
         case BrainModelSurface::SURFACE_TYPE_HULL:
            loadedFilesSpecFile.hullCoordFile.clearSelectionStatus(cf->getFileName());
            break;
         case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
         case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
            loadedFilesSpecFile.unknownCoordFile.clearSelectionStatus(cf->getFileName());
            break;
      }
   }

   const QString configID = 
      BrainModelSurface::getSurfaceConfigurationIDFromType(st);
   cf->setHeaderTag(AbstractFile::headerTagConfigurationID, configID);
   
   //
   // Update the surface type for the BrainModelSurface that uses
   // this coordinate file that is being saved.
   //
   for (int i = 0; i < getNumberOfBrainModels(); i++) {
      BrainModel* bm = getBrainModel(i);
      if (bm->getModelType() == BrainModel::BRAIN_MODEL_SURFACE) {
         BrainModelSurface* bms = dynamic_cast<BrainModelSurface*>(bm);
         CoordinateFile* coordFile = bms->getCoordinateFile();
         if (coordFile == cf) {
            //
            // Get the name of the topology file
            //
            QString topoFileName;
            const TopologyFile* tf = bms->getTopologyFile();
            if (tf != NULL) {
               topoFileName = FileUtilities::basename(tf->getFileName());
               if (topoFileName.length() < 5) {
                  topoFileName = "";
               }
            }
   
            cf->setHeaderTag(SpecFile::getUnknownTopoFileMatchTag(), topoFileName);
            const Structure::STRUCTURE_TYPE hem = bms->getStructure().getType();
            if (hem != Structure::STRUCTURE_TYPE_INVALID) {
               cf->setHeaderTag(AbstractFile::headerTagStructure,
                                Structure::convertTypeToString(hem));
            }
            bms->setSurfaceType(st);
         }
      }
   }

   cf->writeFile(name);
   
   if (updateSpecFile) {
      addToSpecFile(BrainModelSurface::getCoordSpecFileTagFromSurfaceType(st), name);
   }
}

/**
 * Read the volume file data
 */
void
BrainSet::readVolumeFile(const QString& name, 
                         const VolumeFile::VOLUME_TYPE vt,
                         const bool appendIn, 
                         const bool updateSpecIn) throw (FileException)
{
   bool updateSpec = updateSpecIn;
   bool append = appendIn;
   
   std::vector<VolumeFile*> volumes;
   VolumeFile::readFile(name, 
                        VolumeFile::VOLUME_READ_SELECTION_ALL,
                        volumes,
                        false);
                        
   for (unsigned int i = 0; i < volumes.size(); i++) {
      VolumeFile* vf = volumes[i];
      
      //
      // Should a transformation matrix be applied ?
      //
      if (specDataFileTransformationMatrix.isIdentity() == false) {
         vf->applyTransformationMatrix(specDataFileTransformationMatrix);
         vf->clearModified();
      }
      
      if (i > 0) {
         updateSpec = false;
         append = true;
      }
      addVolumeFile(vt, vf, name, append, updateSpec); 
   }
   
   //
   // If not reading a spec file, reading prob atlas volumes, and
   // appending volumes, then region names need to be synchronized
   //
   if (readingSpecFileFlag == false) {
      if (vt == VolumeFile::VOLUME_TYPE_PROB_ATLAS) {
         if (volumes.size() < volumeProbAtlasFiles.size()) {
            synchronizeProbAtlasVolumeRegionNames();
         }
      }
   }
}

/**
 * Add a volume file to this brain set and create a brain model volume if necessary.
 */
void
BrainSet::addVolumeFile(const VolumeFile::VOLUME_TYPE vt, VolumeFile* vf, 
                        const QString& name,
                        const bool append, 
                        const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexAddVolumeFile);
   
   QString tag;
   
   //
   // Add the volume file to this brain set.
   //
   switch (vt) {
      case VolumeFile::VOLUME_TYPE_ANATOMY:
         if (append == false) {
            clearVolumeAnatomyFiles();
         }
         volumeAnatomyFiles.push_back(vf);
         tag = SpecFile::getVolumeAnatomyFileTag();
         break;
      case VolumeFile::VOLUME_TYPE_FUNCTIONAL:
         if (append == false) {
            clearVolumeFunctionalFiles();
         }
         volumeFunctionalFiles.push_back(vf);
         tag = SpecFile::getVolumeFunctionalFileTag();
         break;
      case VolumeFile::VOLUME_TYPE_PAINT:
         if (append == false) {
            clearVolumePaintFiles();
         }
         volumePaintFiles.push_back(vf);
         tag = SpecFile::getVolumePaintFileTag();
         break;
      case VolumeFile::VOLUME_TYPE_PROB_ATLAS:
         if (append == false) {
            clearVolumeProbabilisticAtlasFiles();
         }
         else if (volumeProbAtlasFiles.size() > 0) {
            int dimOld[3];
            volumeProbAtlasFiles[0]->getDimensions(dimOld);
            int dimNew[3];
            vf->getDimensions(dimNew);
            if ((dimOld[0] != dimNew[0]) ||
                (dimOld[1] != dimNew[1]) ||
                (dimOld[2] != dimNew[2])) {
               throw FileException("Newly selected probabilistic atlas volume "
                                   "has different dimensions than previously "
                                   "loaded probabilistic atlas volume(s).  All "
                                   "probabilistic atlas volumes must have the "
                                   "same dimensions.");
            }
         }
         volumeProbAtlasFiles.push_back(vf);
         //if (volumeProbAtlasFiles.size() > 1) {
         //   volumeProbAtlasFiles[0]->setVoxelColoringInvalid();
         //}
         tag = SpecFile::getVolumeProbAtlasFileTag();
         displaySettingsProbabilisticAtlasVolume->update();
         break;
      case VolumeFile::VOLUME_TYPE_RGB:
         if (append == false) {
            clearVolumeRgbFiles();
         }
         volumeRgbFiles.push_back(vf);
         tag = SpecFile::getVolumeRgbFileTag();
         break;
      case VolumeFile::VOLUME_TYPE_ROI:
         return;
         break;
      case VolumeFile::VOLUME_TYPE_SEGMENTATION:
         {
            if (append == false) {
               clearVolumeSegmentationFiles();
            }
            const bool isModified = vf->getModified();
            vf->makeSegmentationZeroTwoFiftyFive();
            if (isModified == false) {
               vf->clearModified();
            }
            volumeSegmentationFiles.push_back(vf);
            tag = SpecFile::getVolumeSegmentationFileTag();
         }
         break;
      case VolumeFile::VOLUME_TYPE_VECTOR:
         {
            if (append == false) {
               clearVolumeVectorFiles();
            }
            volumeVectorFiles.push_back(vf);
            tag = SpecFile::getVolumeVectorFileTag();
         }
         break;
      case VolumeFile::VOLUME_TYPE_UNKNOWN:
         return;
         break;
   }
   
   //
   // Update the spec file
   //
   if (updateSpec) {
      if (name.isEmpty() == false) {
         addToSpecFile(tag, name, vf->getDataFileName());
      }
   }
   
   //
   // Set the volume's type
   //
   vf->setVolumeType(vt);
   
/*
   //
   // Adjust anatomy and segmentation volume ranges.
   //
   QTime timer;
   timer.start();
   vf->scaleAnatomyAndSegmentationVolumesTo255();
   if (DebugControl::getDebugOn()) {
      std::cout << "Time to read scale anatomy/segmentation volume "
                << FileUtilities::basename(vf->getFileName())
                << " was "
                << (static_cast<float>(timer.elapsed()) / 1000.0)
                << " seconds." << std::endl;
   }
*/

   //
   // Create a brain model volume if there is not one
   //
   BrainModelVolume* bmv = NULL;
   for (int i = 0; i < getNumberOfBrainModels(); i++) {
      bmv = getBrainModelVolume(i);
      if (bmv != NULL) {
         break;
      }
   }
   bool createdBrainModelVolume = false;
   if (bmv == NULL) {
      createdBrainModelVolume = true;
      bmv = new BrainModelVolume(this);
      addBrainModel(bmv);
      voxelColoring->initializeUnderlay();
   }
   
   switch (vt) {
      case VolumeFile::VOLUME_TYPE_ANATOMY:
         break;
      case VolumeFile::VOLUME_TYPE_FUNCTIONAL:
         break;
      case VolumeFile::VOLUME_TYPE_PAINT:
         break;
      case VolumeFile::VOLUME_TYPE_PROB_ATLAS:
         //
         // Update indices into name table
         //
    //     if (readingSpecFileFlag == false) {
    //        synchronizeProbAtlasVolumeRegionNames();
    //     }
         displaySettingsProbabilisticAtlasVolume->update();
         break;
      case VolumeFile::VOLUME_TYPE_RGB:
         break;
      case VolumeFile::VOLUME_TYPE_ROI:
         break;
      case VolumeFile::VOLUME_TYPE_SEGMENTATION:
         break;
      case VolumeFile::VOLUME_TYPE_VECTOR:
         break;
      case VolumeFile::VOLUME_TYPE_UNKNOWN:
         return;
         break;
   }
/*
   //
   // Color the voxels for this volume
   //
   QTime colorTime;
   colorTime.start();
   voxelColoring->updateVolumeFileColoring(vf);
   if (DebugControl::getDebugOn()) {
      std::cout << "Time to color volume: " 
                << vf->getFileName()
                << (static_cast<float>(colorTime.elapsed()) / 1000.0)
                << " seconds." << std::endl;
   }
*/
   
   if (readingSpecFileFlag == false) {
      displaySettingsVolume->update();
      displaySettingsWustlRegion->update();
   }
      
   if (createdBrainModelVolume) {
      bmv->initializeSelectedSlicesAllViews(true);
   }
   
   //
   // Possibly create a brain model surface and volume
   //
   createBrainModelSurfaceAndVolume();
}

/**
 * delete a volume file.
 */
void 
BrainSet::deleteVolumeFile(const VolumeFile* vf)
{
   if (vf == NULL) {
      return;
   }
   
   if (getNumberOfVolumeFunctionalFiles() > 0) {
      std::vector<VolumeFile*> files;
      for (int i = 0; i < getNumberOfVolumeFunctionalFiles(); i++) {
         if (vf != getVolumeFunctionalFile(i)) {
            files.push_back(getVolumeFunctionalFile(i));
         }
      }
      volumeFunctionalFiles = files;
   }
   
   if (getNumberOfVolumePaintFiles() > 0) {
      std::vector<VolumeFile*> files;
      for (int i = 0; i < getNumberOfVolumePaintFiles(); i++) {
         if (vf != getVolumePaintFile(i)) {
            files.push_back(getVolumePaintFile(i));
         }
      }
      volumePaintFiles = files;
   }
   
   if (getNumberOfVolumeProbAtlasFiles() > 0) {
      std::vector<VolumeFile*> files;
      int indx = -1;
      for (int i = 0; i < getNumberOfVolumeProbAtlasFiles(); i++) {
         if (vf != getVolumeProbAtlasFile(i)) {
            files.push_back(getVolumeProbAtlasFile(i));
         }
         else {
            indx = i;
         }
      }
      volumeProbAtlasFiles = files;
   }
   
   if (getNumberOfVolumeRgbFiles() > 0) {
      std::vector<VolumeFile*> files;
      for (int i = 0; i < getNumberOfVolumeRgbFiles(); i++) {
         if (vf != getVolumeRgbFile(i)) {
            files.push_back(getVolumeRgbFile(i));
         }
      }
      volumeRgbFiles = files;
   }
   
   if (getNumberOfVolumeSegmentationFiles() > 0) {
      std::vector<VolumeFile*> files;
      for (int i = 0; i < getNumberOfVolumeSegmentationFiles(); i++) {
         if (vf != getVolumeSegmentationFile(i)) {
            files.push_back(getVolumeSegmentationFile(i));
         }
      }
      volumeSegmentationFiles = files;
   }
   
   if (getNumberOfVolumeAnatomyFiles() > 0) {
      std::vector<VolumeFile*> files;
      for (int i = 0; i < getNumberOfVolumeAnatomyFiles(); i++) {
         if (vf != getVolumeAnatomyFile(i)) {
            files.push_back(getVolumeAnatomyFile(i));
         }
      }
      volumeAnatomyFiles = files;
   }
   
   if (getNumberOfVolumeVectorFiles() > 0) {
      std::vector<VolumeFile*> files;
      for (int i = 0; i < getNumberOfVolumeVectorFiles(); i++) {
         if (vf != getVolumeVectorFile(i)) {
            files.push_back(getVolumeVectorFile(i));
         }
      }
      volumeVectorFiles = files;
   }
   
   if ((getNumberOfVolumeFunctionalFiles() == 0) &&
       (getNumberOfVolumeRgbFiles() == 0) &&
       (getNumberOfVolumePaintFiles() == 0) &&
       (getNumberOfVolumeSegmentationFiles() == 0) &&
       (getNumberOfVolumeAnatomyFiles() == 0) &&
       (getNumberOfVolumeVectorFiles() == 0)) {
      BrainModelVolume* bmv = getBrainModelVolume();
      if (bmv != NULL) {
         deleteBrainModel(bmv);
      }
      BrainModelSurfaceAndVolume* bmsv = getBrainModelSurfaceAndVolume();
      if (bmsv != NULL) {
         deleteBrainModel(bmsv);
      }
   }
   
   displaySettingsVolume->update();
   displaySettingsWustlRegion->update();
   
   loadedFilesSpecFile.volumeFunctionalFile.clearSelectionStatus(vf->getFileName());
   loadedFilesSpecFile.volumePaintFile.clearSelectionStatus(vf->getFileName());
   loadedFilesSpecFile.volumeProbAtlasFile.clearSelectionStatus(vf->getFileName());
   loadedFilesSpecFile.volumeRgbFile.clearSelectionStatus(vf->getFileName());
   loadedFilesSpecFile.volumeSegmentationFile.clearSelectionStatus(vf->getFileName());
   loadedFilesSpecFile.volumeAnatomyFile.clearSelectionStatus(vf->getFileName());
   loadedFilesSpecFile.volumeVectorFile.clearSelectionStatus(vf->getFileName());

   delete vf;
   
   nodeColoring->assignColors();
   clearAllDisplayLists();
}      

/**
 * Write the multi-volume file data.
 */
void
BrainSet::writeMultiVolumeFile(const QString& name, 
                          const VolumeFile::VOLUME_TYPE volumeType,
                          const QString& comment,
                          std::vector<VolumeFile*>& subVolumes,
                          const VolumeFile::VOXEL_DATA_TYPE voxelDataTypeToWriteIn,
                          const bool zipAfniVolumeFile) throw (FileException)
{   
   VolumeFile::VOXEL_DATA_TYPE voxelDataTypeToWrite = voxelDataTypeToWriteIn;
   if (voxelDataTypeToWrite == VolumeFile::VOXEL_DATA_TYPE_UNKNOWN) {
      if (subVolumes.empty() == false) {
         voxelDataTypeToWrite = subVolumes[0]->getVoxelDataType();
      }
   }
   
   //
   // Set spec file tag
   //
   QString tag(SpecFile::getVolumeAnatomyFileTag());

   switch (volumeType) {
      case VolumeFile::VOLUME_TYPE_ANATOMY:
         tag = SpecFile::getVolumeAnatomyFileTag();
         break;
      case VolumeFile::VOLUME_TYPE_FUNCTIONAL:
         tag = SpecFile::getVolumeFunctionalFileTag();
         break;
      case VolumeFile::VOLUME_TYPE_PAINT:
         tag = SpecFile::getVolumePaintFileTag();
         break;
      case VolumeFile::VOLUME_TYPE_PROB_ATLAS:
         tag = SpecFile::getVolumeProbAtlasFileTag();
         break;
      case VolumeFile::VOLUME_TYPE_RGB:
         tag = SpecFile::getVolumeRgbFileTag();
         break;
      case VolumeFile::VOLUME_TYPE_ROI:
         throw FileException(FileUtilities::basename(name), "Unrecognized volume type");
         break;
      case VolumeFile::VOLUME_TYPE_SEGMENTATION:
         tag = SpecFile::getVolumeSegmentationFileTag();
         break;
      case VolumeFile::VOLUME_TYPE_VECTOR:
         tag = SpecFile::getVolumeVectorFileTag();
         break;
      case VolumeFile::VOLUME_TYPE_UNKNOWN:
         throw FileException(FileUtilities::basename(name), "Unrecognized volume type");
         break;
   }

   if (subVolumes.empty() == false) {
      subVolumes[0]->setFileComment(comment);
      VolumeFile::writeFile(name,
                            volumeType,
                            voxelDataTypeToWrite,
                            subVolumes,
                            zipAfniVolumeFile);
      addToSpecFile(tag, name, subVolumes[0]->getDataFileName());
   }   
/*
      const QString dataFileName =
         VolumeFile::writeMultiVolumeFile(name, writeFileType, comment, subVolumes);
*/
}

/**
 * Write the volume file data.
 */
void
BrainSet::writeVolumeFile(const QString& nameIn, 
                          const VolumeFile::FILE_READ_WRITE_TYPE writeFileType,
                          const VolumeFile::VOLUME_TYPE volumeType,
                          VolumeFile* vf,
                          const VolumeFile::VOXEL_DATA_TYPE voxelDataTypeToWriteIn,
                          const bool zipAfniVolumeFile) throw (FileException)
{
   VolumeFile::VOXEL_DATA_TYPE voxelDataTypeToWrite = voxelDataTypeToWriteIn;
   if (voxelDataTypeToWrite == VolumeFile::VOXEL_DATA_TYPE_UNKNOWN) {
      voxelDataTypeToWrite = vf->getVoxelDataType();
   }
   
   QString name(nameIn);
   
   //
   // NIFTI GZIP
   //
   if (name.endsWith(SpecFile::getNiftiVolumeFileExtension()) &&
       zipAfniVolumeFile) {
      name += ".gz";
   }
   
   vf->setFileWriteType(writeFileType);
   
   //
   // Set spec file tag
   //
   QString tag(SpecFile::getVolumeAnatomyFileTag());

   switch (volumeType) {
      case VolumeFile::VOLUME_TYPE_ANATOMY:
         tag = SpecFile::getVolumeAnatomyFileTag();
         loadedFilesSpecFile.volumeAnatomyFile.clearSelectionStatus(vf->getFileName());
         break;
      case VolumeFile::VOLUME_TYPE_FUNCTIONAL:
         tag = SpecFile::getVolumeFunctionalFileTag();
         loadedFilesSpecFile.volumeFunctionalFile.clearSelectionStatus(vf->getFileName());
         break;
      case VolumeFile::VOLUME_TYPE_PAINT:
         tag = SpecFile::getVolumePaintFileTag();
         loadedFilesSpecFile.volumePaintFile.clearSelectionStatus(vf->getFileName());
         break;
      case VolumeFile::VOLUME_TYPE_PROB_ATLAS:
         tag = SpecFile::getVolumeProbAtlasFileTag();
         loadedFilesSpecFile.volumeProbAtlasFile.clearSelectionStatus(vf->getFileName());
         break;
      case VolumeFile::VOLUME_TYPE_RGB:
         tag = SpecFile::getVolumeRgbFileTag();
         loadedFilesSpecFile.volumeRgbFile.clearSelectionStatus(vf->getFileName());
         break;
      case VolumeFile::VOLUME_TYPE_ROI:
         throw FileException(FileUtilities::basename(name), "Unrecognized volume type=ROI");
         break;
      case VolumeFile::VOLUME_TYPE_SEGMENTATION:
         tag = SpecFile::getVolumeSegmentationFileTag();
         loadedFilesSpecFile.volumeSegmentationFile.clearSelectionStatus(vf->getFileName());
         break;
      case VolumeFile::VOLUME_TYPE_VECTOR:
         tag = SpecFile::getVolumeVectorFileTag();
         loadedFilesSpecFile.volumeVectorFile.clearSelectionStatus(vf->getFileName());
         break;
      case VolumeFile::VOLUME_TYPE_UNKNOWN:
         throw FileException(FileUtilities::basename(name), "Unrecognized volume type");
         break;
   }
   
   std::vector<VolumeFile*> volumesToWrite;
   volumesToWrite.push_back(vf);
   VolumeFile::writeFile(name,
                         volumeType,
                         voxelDataTypeToWrite,
                         volumesToWrite,
                         zipAfniVolumeFile);
/*                                               
   vf->writeFile(name);

   QString dataFileName(vf->getDataFileName());
   if (vf->getWriteFileGzip()) {
      if (dataFileName.right(3) != ".gz") {
         dataFileName.append(".gz");
      }
      
      if (writeFileType == VolumeFile::FILE_READ_WRITE_TYPE_NIFTI) {
         if (name.right(3) != ".gz") {
            name.append(".gz");
         }
      }
   }
*/
   addToSpecFile(tag, name, vf->getDataFileName());
}

/**
 * read the surface data file file.
 */
void 
BrainSet::readSurfaceFile(const QString& name, 
                          const BrainModelSurface::SURFACE_TYPES surfaceTypeIn,
                          const bool readingSpecFile,
                          const bool append,
                          const bool updateSpec) throw (FileException)
{
   BrainModelSurface::SURFACE_TYPES surfaceType = surfaceTypeIn;
   
   BrainModelSurface* bms = new BrainModelSurface(this);
   bms->readSurfaceFile(name);
   if (surfaceType == BrainModelSurface::SURFACE_TYPE_UNKNOWN) {
      surfaceType = bms->getSurfaceType();
   }

   QMutexLocker mutex(&mutexReadSurfaceFile);
   
   bool needToInitialize = false;
   if (readingSpecFile == false) {
      needToInitialize = true;
      for (int i = 0; i < getNumberOfBrainModels(); i++) {
         if (getBrainModelSurface(i) != NULL) {
            needToInitialize = false;
         }
      }
   }
   
   if (append == false) {
      deleteSurfacesOfType(surfaceType);
   }
   
   if (getNumberOfNodes() == 0) {
      if (bms->getNumberOfNodes() > 0) {
         numNodesMessage = " contains a different number of nodes than ";
         numNodesMessage.append(FileUtilities::basename(name));
      }
      else {
         delete bms;
         bms = NULL;
         throw FileException(name, "Contains no nodes");
      }
   }
   else if (getNumberOfNodes() != bms->getNumberOfNodes()) {
      delete bms;
      bms = NULL;
      throw FileException(FileUtilities::basename(name), numNodesMessage);
   }
   
   //
   // Get the coordinate file
   //
   CoordinateFile* cf = bms->getCoordinateFile();
   
   QString tag;
   
   bool applyTransformFlag = false;
   switch(surfaceType) {
      case BrainModelSurface::SURFACE_TYPE_RAW:
         tag = SpecFile::getRawSurfaceFileTag();
         applyTransformFlag = (specDataFileTransformationMatrix.isIdentity() == false);
         break;
      case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
         tag = SpecFile::getFiducialSurfaceFileTag();
         applyTransformFlag = (specDataFileTransformationMatrix.isIdentity() == false);
         break;
      case BrainModelSurface::SURFACE_TYPE_INFLATED:
         tag = SpecFile::getInflatedSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
         tag = SpecFile::getVeryInflatedSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
         tag = SpecFile::getSphericalSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
         tag = SpecFile::getEllipsoidSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         tag = SpecFile::getCompressedSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT:
         tag = SpecFile::getFlatSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
         tag = SpecFile::getLobarFlatSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_HULL:
         tag = SpecFile::getHullSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
      case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
         tag = SpecFile::getUnknownSurfaceFileMatchTag();
         break;
      default:
         throw FileException(FileUtilities::basename(name),
                              "Unrecognized surface type");
         break;
   }
   
   //
   // If transformation matrix should be applied
   //
   if (applyTransformFlag) {
      cf->applyTransformationMatrix(specDataFileTransformationMatrix);
      cf->clearModified();
   }
      
   bms->computeNormals();
   bms->setSurfaceType(surfaceType);
   
   //
   // Only set the structure for the surface if the structure is unknown
   //
   if (bms->getStructure() == Structure::STRUCTURE_TYPE_INVALID) {
      if (structure.getType() != Structure::STRUCTURE_TYPE_INVALID) {
         bms->setStructure(structure.getType());
      }
      //cf->setHeaderTag(AbstractFile::headerTagHemisphere, "");
   }
   cf->clearModified(); // setSurfaceType causes modified flat to be set
   
   addBrainModel(bms, readingSpecFile);

   setSelectedTopologyFiles();
   
   if (updateSpec) {
      addToSpecFile(tag, name);
   }
   
   if (needToInitialize) {
      postSpecFileReadInitializations();
   }
   
   //
   // Possibly create a brain model surface and volume
   //
   createBrainModelSurfaceAndVolume();
}      

/**      
 * Read the coordinate data file file
 */
void 
BrainSet::readCoordinateFile(const QString& name, const BrainModelSurface::SURFACE_TYPES stin,
                             const bool readingSpecFile,
                             const bool append,
                             const bool updateSpec) throw (FileException)
{
   //
   // get type of surface from file, if needed
   //
   BrainModelSurface::SURFACE_TYPES st = stin;
   if ((st == BrainModelSurface::SURFACE_TYPE_UNSPECIFIED) ||
       (st == BrainModelSurface::SURFACE_TYPE_UNKNOWN)) {
      CoordinateFile cf;
      cf.readFileMetaDataOnly(name);
      const QString typeTag = cf.getHeaderTag(AbstractFile::headerTagConfigurationID);
      if (typeTag.isEmpty() == false) {
         st = BrainModelSurface::getSurfaceTypeFromConfigurationID(typeTag);
      }
   }
   
   //
   // Read the coordinate file
   //
   BrainModelSurface* bms = new BrainModelSurface(this);
   bms->readCoordinateFile(name);
   
   //
   // The remainder of this routine must not be run by more than one thread
   //
   QMutexLocker locker(&mutexReadCoordinateFile);
   
   //
   // See if initialization needed
   //
   bool needToInitialize = false;
   if (readingSpecFile == false) {
      needToInitialize = true;
      for (int i = 0; i < getNumberOfBrainModels(); i++) {
         if ((getBrainModelSurface(i) != NULL) &&
             (getBrainModelSurface(i) != bms)) {
            needToInitialize = false;
         }
      }
   }
   
   //
   // Check nodes
   //
   if (getNumberOfNodes() == 0) {
      if (bms->getNumberOfNodes() > 0) {
         numNodesMessage = " contains a different number of nodes than ";
         numNodesMessage.append(FileUtilities::basename(name));
      }
      else {
         delete bms;
         bms = NULL;
         throw FileException(name, "Contains no nodes");
      }
   }
   else if (getNumberOfNodes() != bms->getNumberOfNodes()) {
      delete bms;
      bms = NULL;
      throw FileException(FileUtilities::basename(name), numNodesMessage);
   }
   
   if (append == false) {
      deleteSurfacesOfType(st);
   }
   
   //
   // Get the coordinate file
   //
   CoordinateFile* cf = bms->getCoordinateFile();
   
   //
   // May automatically set topology file
   //
   TopologyFile* useThisTopologyFile = NULL;

/*   
   if (readingSpecFile) {
      const QString topoFile = cf->getHeaderTag(SpecFile::UnknownTopoFileMatchTag);
      if (topoFile.empty() == false) {
         for (int i = 0; i < getNumberOfTopologyFiles(); i++) {
            if (FileUtilities::basename(topoFile) == 
                FileUtilities::basename(getTopologyFile(i)->getFileName())) {
               useThisTopologyFile = getTopologyFile(i);
               break;
            }
         }
      }
   }
   else {
*/
   
   //
   // Should topo file listed in coord file's header be used
   //
   if (ignoreTopologyFileInCoordinateFileHeaderFlag == false) {
      //
      // Load matching topo file if it is not already loaded
      //
      const QString topoFile = cf->getHeaderTag(SpecFile::getUnknownTopoFileMatchTag());
      if (topoFile.isEmpty() == false) {
         bool loadIt = true;
         for (int i = 0; i < getNumberOfTopologyFiles(); i++) {
            if (FileUtilities::basename(topoFile) == 
                FileUtilities::basename(getTopologyFile(i)->getFileName())) {
               loadIt = false;
               useThisTopologyFile = getTopologyFile(i);
               break;
            }
         }
         if (QFile::exists(topoFile) && loadIt) {
            try {
               //
               // Want to update spec file even if reading files from spec file
               //
               const bool readingSpecFileFlagCOPY = readingSpecFileFlag;
               readingSpecFileFlag = false;
               readTopologyFile(topoFile, TopologyFile::TOPOLOGY_TYPE_UNSPECIFIED, true, true);
               readingSpecFileFlag = readingSpecFileFlagCOPY;
               //const int topoIndex = getNumberOfTopologyFiles() - 1;
               //if ((topoIndex >= 0) && (topoIndex < getNumberOfTopologyFiles())) {
               //
               // Note that readTopologyFile() always places the just read topology
               // file as the first topology file
               //
               if (getNumberOfTopologyFiles() > 0) {
                  useThisTopologyFile = getTopologyFile(0);
               }
            }
            catch (FileException& /*e*/) {
            }
         }
      }
   }
/*
   }
*/   
   //
   // Update configuration ID in the coord file
   //
   const QString configID = 
      BrainModelSurface::getSurfaceConfigurationIDFromType(st);
   cf->setHeaderTag(AbstractFile::headerTagConfigurationID, configID);
   cf->clearModified();
   QString tag;
   
   bool applyTransformFlag = false;
   bool topoError = false; 
   switch(st) {
      case BrainModelSurface::SURFACE_TYPE_RAW:
         topoError = bms->setTopologyFile(topologyClosed);
         tag = SpecFile::getRawCoordFileTag();
         applyTransformFlag = (specDataFileTransformationMatrix.isIdentity() == false);
         break;
      case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
         topoError = bms->setTopologyFile(topologyClosed);
         tag = SpecFile::getFiducialCoordFileTag();
         applyTransformFlag = (specDataFileTransformationMatrix.isIdentity() == false);
         break;
      case BrainModelSurface::SURFACE_TYPE_INFLATED:
         topoError = bms->setTopologyFile(topologyClosed);
         tag = SpecFile::getInflatedCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
         topoError = bms->setTopologyFile(topologyClosed);
         tag = SpecFile::getVeryInflatedCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
         topoError = bms->setTopologyFile(topologyClosed);
         tag = SpecFile::getSphericalCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
         topoError = bms->setTopologyFile(topologyClosed);
         tag = SpecFile::getEllipsoidCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         topoError = bms->setTopologyFile(topologyClosed);
         tag = SpecFile::getCompressedCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT:
         topoError = bms->setTopologyFile(topologyCut);
         tag = SpecFile::getFlatCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
         topoError = bms->setTopologyFile(topologyLobarCut);
         tag = SpecFile::getLobarFlatCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_HULL:
         topoError = bms->setTopologyFile(topologyClosed);
         tag = SpecFile::getHullCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
      case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
         topoError = bms->setTopologyFile(topologyClosed);
         tag = SpecFile::getUnknownCoordFileMatchTag();
         break;
      default:
         throw FileException(FileUtilities::basename(name),
                              "Unrecognized surface type");
         break;
   }
   
   //
   // If transformation matrix should be applied
   //
   if (applyTransformFlag) {
      cf->applyTransformationMatrix(specDataFileTransformationMatrix);
      cf->clearModified();
   }
   
   //
   // Use topology file coord file was using when it was saved
   //
   if (useThisTopologyFile != NULL) {
      topoError =  bms->setTopologyFile(useThisTopologyFile);
   }
   
   if (topoError) {
      std::ostringstream str;
      str << "Topology File "
          << FileUtilities::basename(bms->getTopologyFile()->getFileName()).toAscii().constData()
          << "\n is not for use with coordinate file "
          << FileUtilities::basename(name).toAscii().constData()
          << ".\n  Topo file has tiles with node numbers exceeding \n"
          << "the number of coordinates in the coordinate file.";
      delete bms;
      throw FileException(name, str.str().c_str());
   }
   
   bms->computeNormals();
   bms->setSurfaceType(st);
   
   //
   // Only set the structure for the surface if the structure is unknown
   //
   if (bms->getStructure() == Structure::STRUCTURE_TYPE_INVALID) {
      if (structure.getType() != Structure::STRUCTURE_TYPE_INVALID) {
         bms->setStructure(structure.getType());
      }
      //cf->setHeaderTag(AbstractFile::headerTagHemisphere, "");
   }
   cf->clearModified(); // setSurfaceType causes modified flat to be set
   
   addBrainModel(bms, readingSpecFile);

   if (updateSpec) {
      addToSpecFile(tag, name);
   }
   
   if (needToInitialize) {
      postSpecFileReadInitializations();
   }
   
   //
   // Possibly create a brain model surface and volume
   //
   createBrainModelSurfaceAndVolume();
}

/**
 * Add a brain model to this brain set
 */      
void
BrainSet::addBrainModel(BrainModel* bm, const bool readingSpecFile)
{
   //
   // The remainder of this routine must not be run by more than one thread
   //
   QMutexLocker locker(&mutexAddBrainModel);

   brainModels.push_back(bm);
   brainModelBorderSet->addBrainModel(bm);
   
   updateDisplayedModelIndices();
   updateSurfaceOverlaysDueToChangeInBrainModels();
   updateAllDisplaySettings();
   
   if (readingSpecFile == false) {
      if (dynamic_cast<BrainModelSurface*>(bm) != NULL) {
         nodeColoring->assignColors();
      }
   }
}

/**
 * get the index for a brain model.
 */
int 
BrainSet::getBrainModelIndex(const BrainModel* bm) const
{
   const int numBrainModels = getNumberOfBrainModels();
   for (int i = 0; i < numBrainModels; i++) {
      if (bm == getBrainModel(i)) {
         return i;
      }
   }
   return -1;
}

/**
 * Delete a brain model
 */
void 
BrainSet::deleteBrainModel(const BrainModel* bm)
{
   std::vector<BrainModel*> models;

   const unsigned int num = brainModels.size();
   for (unsigned int i = 0; i < num; i++) {
      if (brainModels[i] != bm) {
         models.push_back(brainModels[i]);
      }
   }
   
   brainModelBorderSet->deleteBrainModel(bm);
   
   brainModels = models;
   delete bm;
   
   updateDisplayedModelIndices();
   updateSurfaceOverlaysDueToChangeInBrainModels();

   updateAllDisplaySettings();
   
   nodeColoring->assignColors();
   clearAllDisplayLists();
}

/** 
 * Write the foci data file of the specified type.
 */ 
void
BrainSet::writeFociFile(const QString& name,
                        const BrainModelSurface* leftBMS,
                        const BrainModelSurface* rightBMS,
                        const BrainModelSurface* cerebellumBMS,
                        const AbstractFile::FILE_FORMAT fileFormat,
                        const QString& commentText) throw (FileException)
{
   FociFile ff;

   const CoordinateFile* leftCF = ((leftBMS != NULL) ?
                                   leftBMS->getCoordinateFile()
                                   : NULL);
   const TopologyFile* leftTF = ((leftBMS != NULL) ?
                                 leftBMS->getTopologyFile()
                                 : NULL);
   
   const CoordinateFile* rightCF = ((rightBMS != NULL) ?
                                   rightBMS->getCoordinateFile()
                                   : NULL);
   const TopologyFile* rightTF = ((rightBMS != NULL) ?
                                 rightBMS->getTopologyFile()
                                 : NULL);
   
   const CoordinateFile* cerebellumCF = ((cerebellumBMS != NULL) ?
                                   cerebellumBMS->getCoordinateFile()
                                   : NULL);
   const TopologyFile* cerebellumTF = ((cerebellumBMS != NULL) ?
                                 cerebellumBMS->getTopologyFile()
                                 : NULL);
   
   fociProjectionFile->getCellFileForRightLeftFiducials(leftCF,
                                                        leftTF,
                                                        rightCF,
                                                        rightTF,
                                                        cerebellumCF,
                                                        cerebellumTF,
                                                        ff);
   
   //
   // Check for foci
   //
   if (ff.getNumberOfCells() <= 0) {
      throw FileException("There are no foci that project to the selected surface(s).");
   }
   ff.setFileComment(commentText);
   ff.setFileWriteType(fileFormat);
   ff.writeFile(name);
   addToSpecFile(SpecFile::getFociFileTag(), name);
}

/**
 * write the Foci data file.
 */
void 
BrainSet::writeFociFileOriginalCoordinates(const QString& name,
                                           const AbstractFile::FILE_FORMAT fileFormat,
                                           const QString& commentText) throw (FileException)
{
   FociFile ff;
   fociProjectionFile->getCellFileOriginalCoordinates(ff);
   
   ff.setFileComment(commentText);
   ff.setFileWriteType(fileFormat);
   ff.writeFile(name);

   addToSpecFile(SpecFile::getFociFileTag(), name);
}
      
/**      
 * Read the foci data file file
 */
void 
BrainSet::readFociFile(const QString& name, 
                       const bool append,
                       const bool updateSpec) throw (FileException)
{
   FociFile fociFile;
   try {
      fociFile.readFile(name);
      
      //
      // If transformation should be applied
      //
      if (specDataFileTransformationMatrix.isIdentity() == false) {
         fociFile.applyTransformationMatrix(std::numeric_limits<int>::min(),
                                             std::numeric_limits<int>::max(),
                                             specDataFileTransformationMatrix,
                                             false);
      }
   }
   catch (FileException& e) {
      throw FileException(FileUtilities::basename(name), e.whatQString());
   }
   
   QMutexLocker locker(&mutexFociAndFociProjectionFile);
   
   if (append == false) {
      deleteAllFociProjections();
   }
   
   fociProjectionFile->appendFiducialCellFile(fociFile);

   if (updateSpec) {
      addToSpecFile(SpecFile::getFociFileTag(), name);
   }
}
      
/** 
 * Write the foci color data file.
 */ 
void
BrainSet::writeFociColorFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.fociColorFile.setAllSelections(SpecFile::SPEC_FALSE);
   fociColorFile->writeFile(name);
   addToSpecFile(SpecFile::getFociColorFileTag(), name);
}

/**      
 * Read the foci color data file file
 */
void 
BrainSet::readFociColorFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexFociColorFile);
   
   if (append == false) {
      clearFociColorFile();
   }
   const unsigned long modified = fociColorFile->getModified();
   
   if (fociColorFile->getNumberOfColors() == 0) {         
      try {
         fociColorFile->readFile(name);
      }
      catch (FileException& e) {
         clearFociColorFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      FociColorFile cf;
      cf.readFile(name);
      QString msg;
      fociColorFile->append(cf);
   }
   fociColorFile->setModifiedCounter(modified);
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getFociColorFileTag(), name);
   }
}
      
/** 
 * Write the Foci Projection data file.
 */ 
void
BrainSet::writeFociProjectionFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.fociProjectionFile.setAllSelections(SpecFile::SPEC_FALSE);
   fociProjectionFile->writeFile(name);
   addToSpecFile(SpecFile::getFociProjectionFileTag(), name);
}

/**      
 * Read the foci projection data file file
 */
void 
BrainSet::readFociProjectionFile(const QString& name, 
                            const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexFociAndFociProjectionFile);
   
   if (append == false) {
      deleteAllFociProjections();
   }
   const unsigned long modified = fociProjectionFile->getModified();
   
   if (fociProjectionFile->getNumberOfCellProjections() == 0) {         
      try {
         fociProjectionFile->readFile(name);
      }
      catch (FileException& e) {
         deleteAllFociProjections();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      FociProjectionFile cf;
      cf.readFile(name);
      QString msg;
      fociProjectionFile->append(cf);
   }
   fociProjectionFile->setModifiedCounter(modified);
   
   if (readingSpecFileFlag == false) {
      displaySettingsFoci->update();
   }
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getFociProjectionFileTag(), name);
   }
}
      
/** 
 * Write the Foci Search data file.
 */ 
void
BrainSet::writeFociSearchFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.fociSearchFile.setAllSelections(SpecFile::SPEC_FALSE);
   fociSearchFile->writeFile(name);
   addToSpecFile(SpecFile::getFociSearchFileTag(), name);
}

/**      
 * Read the foci search data file file
 */
void 
BrainSet::readFociSearchFile(const QString& name, 
                            const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexFociSearchFile);
   
   if (append == false) {
      clearFociSearchFile();
   }
   const unsigned long modified = fociSearchFile->getModified();
   
   if (fociSearchFile->getNumberOfFociSearchSets() == 0) {         
      try {
         fociSearchFile->readFile(name);
      }
      catch (FileException& e) {
         deleteAllFociProjections();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      FociSearchFile fsf;
      fsf.readFile(name);
      QString msg;
      fociSearchFile->append(fsf);
   }
   fociSearchFile->setModifiedCounter(modified);
   
   if (readingSpecFileFlag == false) {
      displaySettingsFoci->update();
   }
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getFociSearchFileTag(), name);
   }
}


/**
 * Read an electrode trajectory file.
 */
void
BrainSet::readTrajectoryFile(const QString& name) throw (FileException)
{
	float xyzOrigin[3];
	double xyzD[3];
	// check for volume anatomy file
	const VolumeFile* vf = getVolumeAnatomyFile(0);
	if (NULL == vf)
	{
		throw FileException(name, "Cannot load trajectory file without a volume anatomy file already loaded!");
	}
	else
	{
		vf->getOrigin(xyzOrigin);
		if (DebugControl::getDebugOn())
		{
			std::cout << "Volume origin is " << xyzOrigin[0] << "," << xyzOrigin[1] << "," << xyzOrigin[2] << std::endl;
		}
		for (int i=0; i<3; i++) xyzD[i] = xyzOrigin[i];
		trajectoryFile->setVolumeOrigin(xyzD);

		// tell trajectory file to generate the nn tree from the fiducial surface
		const BrainModelSurface* bms = getActiveFiducialSurface();
	   if (bms != NULL)
	   {
   	   const CoordinateFile* cf = bms->getCoordinateFile();
   	   trajectoryFile->createFiducialTree(cf);
	   }
	   else
	   {
	   	std::cout << "ERROR in readTrajectoryFile - no fiducial surface!" << std::endl;
	   }
		trajectoryFile->readFile(name);


	}

}

/**
 * Write the electrode trajectory file.
 */
void
BrainSet::writeTrajectoryFile(const QString& name) throw (FileException)
{
	trajectoryFile->writeFile(name);
}





/** 
 * Write the geodesic distance data file.
 */ 
void
BrainSet::writeGeodesicDistanceFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.geodesicDistanceFile.setAllSelections(SpecFile::SPEC_FALSE);
   geodesicDistanceFile->writeFile(name);
   addToSpecFile(SpecFile::getGeodesicDistanceFileTag(), name);
}

/**
 * read the geodesic distance data file file (only selected columns).
 */
void 
BrainSet::readGeodesicDistanceFile(const QString& name, 
                         const std::vector<int>& columnDestination,
                         const std::vector<QString>& fileBeingReadColumnNames,
                         const AbstractFile::FILE_COMMENT_MODE fcm,
                         const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexGeodesicDistanceFile);
   
   GeodesicDistanceFile gdf;
   gdf.readFile(name);
   if (gdf.getNumberOfNodes() != getNumberOfNodes()) {
      throw FileException(FileUtilities::basename(name), numNodesMessage);
   }
   try {
      for (int i = 0; i < gdf.getNumberOfColumns(); i++) {
         if (i < static_cast<int>(fileBeingReadColumnNames.size())) {
            gdf.setColumnName(i, fileBeingReadColumnNames[i]);
         }
      }
      geodesicDistanceFile->append(gdf, columnDestination, fcm);
   }
   catch (FileException& e) {
      throw FileException(FileUtilities::basename(name), e.whatQString());
   }
   geodesicDistanceFile->setModified();
   displaySettingsGeodesicDistance->update();
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getGeodesicDistanceFileTag(), name);
   }
}

/**      
 * Read the geodesic distance data file file
 */
void 
BrainSet::readGeodesicDistanceFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexGeodesicDistanceFile);
   
   if (append == false) {
      clearGeodesicDistanceFile();
   }
   const unsigned long modified = geodesicDistanceFile->getModified();
   
   if (geodesicDistanceFile->getNumberOfColumns() == 0) {         
      try {
         geodesicDistanceFile->readFile(name);
         if (geodesicDistanceFile->getNumberOfNodes() != getNumberOfNodes()) {
            throw FileException(FileUtilities::basename(name), numNodesMessage);
         }
      }
      catch (FileException& e) {
         clearGeodesicDistanceFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      GeodesicDistanceFile gdf;
      gdf.readFile(name);
      if (gdf.getNumberOfNodes() != getNumberOfNodes()) {
         throw FileException(FileUtilities::basename(name), numNodesMessage);
      }
      try {
         geodesicDistanceFile->append(gdf);
      }
      catch (FileException& e) {
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   geodesicDistanceFile->setModifiedCounter(modified);
   displaySettingsGeodesicDistance->update();
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getGeodesicDistanceFileTag(), name);
   }
}
      
/** 
 * Write the lat lon data file.
 */ 
void
BrainSet::writeLatLonFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.latLonFile.setAllSelections(SpecFile::SPEC_FALSE);
   latLonFile->writeFile(name);
   addToSpecFile(SpecFile::getLatLonFileTag(), name);
}

/**
 * read the lat lon data file file (only selected columns).
 */
void 
BrainSet::readLatLonFile(const QString& name, 
                         const std::vector<int>& columnDestination,
                         const std::vector<QString>& fileBeingReadColumnNames,
                         const AbstractFile::FILE_COMMENT_MODE fcm,
                         const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexLatLonFile);
   
   LatLonFile llf;
   llf.readFile(name);
   if (llf.getNumberOfNodes() != getNumberOfNodes()) {
      throw FileException(FileUtilities::basename(name), numNodesMessage);
   }
   try {
      for (int i = 0; i < llf.getNumberOfColumns(); i++) {
         if (i < static_cast<int>(fileBeingReadColumnNames.size())) {
            llf.setColumnName(i, fileBeingReadColumnNames[i]);
         }
      }
      latLonFile->append(llf, columnDestination, fcm);
   }
   catch (FileException& e) {
      throw FileException(FileUtilities::basename(name), e.whatQString());
   }
   latLonFile->setModified();
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getLatLonFileTag(), name);
   }
}

/**      
 * Read the lat lon data file file
 */
void 
BrainSet::readLatLonFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexLatLonFile);
   
   if (append == false) {
      clearLatLonFile();
   }
   const unsigned long modified = latLonFile->getModified();
   
   if (latLonFile->getNumberOfColumns() == 0) {         
      try {
         latLonFile->readFile(name);
         if (latLonFile->getNumberOfNodes() != getNumberOfNodes()) {
            throw FileException(FileUtilities::basename(name), numNodesMessage);
         }
      }
      catch (FileException& e) {
         clearLatLonFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      LatLonFile llf;
      llf.readFile(name);
      if (llf.getNumberOfNodes() != getNumberOfNodes()) {
         throw FileException(FileUtilities::basename(name), numNodesMessage);
      }
      try {
         latLonFile->append(llf);
      }
      catch (FileException& e) {
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   latLonFile->setModifiedCounter(modified);
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getLatLonFileTag(), name);
   }
}
      
/** 
 * Write the metric data file.
 */ 
void
BrainSet::writeMetricFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.metricFile.setAllSelections(SpecFile::SPEC_FALSE);
   metricFile->writeFile(name);
   addToSpecFile(SpecFile::getMetricFileTag(), name);
}

/**
 * read the metric data file file (only selected columns).
 */
void 
BrainSet::readMetricFile(const QString& name, 
                         const std::vector<int>& columnDestination,
                         const std::vector<QString>& fileBeingReadColumnNames,
                         const AbstractFile::FILE_COMMENT_MODE fcm,
                         const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexMetricFile);
   
   const bool metricFileEmpty = metricFile->empty();
   
   MetricFile mf;
   mf.setNumberOfNodesForSparseNodeIndexFiles(getNumberOfNodes());
   mf.readFile(name);
   if (mf.getNumberOfNodes() != getNumberOfNodes()) {
      throw FileException(FileUtilities::basename(name), numNodesMessage);
   }
   try {
      for (int i = 0; i < mf.getNumberOfColumns(); i++) {
         if (i < static_cast<int>(fileBeingReadColumnNames.size())) {
            if (fileBeingReadColumnNames[i].isEmpty() == false) {
               mf.setColumnName(i, fileBeingReadColumnNames[i]);
            }
         }
      }
      std::vector<int> columnDestination2 = columnDestination;
      metricFile->append(mf, columnDestination2, fcm);
   }
   catch (FileException& e) {
      throw FileException(FileUtilities::basename(name), e.whatQString());
   }
   if (metricFileEmpty) {
      metricFile->clearModified();
   }
   else {
      metricFile->setModified();
   }

   if (readingSpecFileFlag == false) {
      displaySettingsMetric->update();
      brainSetAutoLoaderManager->update();
   }
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getMetricFileTag(), name);
   }
}


/**      
 * Read the metric data file file
 */
void 
BrainSet::readMetricFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexMetricFile);
   
   if (append == false) {
      clearMetricFile();
   }
   const unsigned long modified = metricFile->getModified();
   
   if (metricFile->getNumberOfColumns() == 0) {         
      try {
         metricFile->readFile(name);
         if (metricFile->getNumberOfNodes() != getNumberOfNodes()) {
            throw FileException(FileUtilities::basename(name), numNodesMessage);
         }
      }
      catch (FileException& e) {
         clearMetricFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      MetricFile mf;
      mf.readFile(name);
      if (mf.getNumberOfNodes() != getNumberOfNodes()) {
         throw FileException(FileUtilities::basename(name), numNodesMessage);
      }
      try {
         metricFile->append(mf);
      }
      catch (FileException& e) {
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   metricFile->setModifiedCounter(modified);

   if (readingSpecFileFlag == false) {
      displaySettingsMetric->update();
      brainSetAutoLoaderManager->update();
   }
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getMetricFileTag(), name);
   }
}
 
/** 
 * Write the deformation field data file.
 */ 
void
BrainSet::writeDeformationFieldFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.deformationMapFile.setAllSelections(SpecFile::SPEC_FALSE);
   deformationFieldFile->writeFile(name);
   addToSpecFile(SpecFile::getDeformationFieldFileTag(), name);
}

/**
 * read the deformation field data file (only selected columns).
 */
void 
BrainSet::readDeformationFieldFile(const QString& name, 
                         const std::vector<int>& columnDestination,
                         const std::vector<QString>& fileBeingReadColumnNames,
                         const AbstractFile::FILE_COMMENT_MODE fcm,
                         const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexDeformationFieldFile);
   
   DeformationFieldFile dff;
   dff.readFile(name);
   if (dff.getNumberOfNodes() != getNumberOfNodes()) {
      throw FileException(FileUtilities::basename(name), numNodesMessage);
   }
   try {
      for (int i = 0; i < dff.getNumberOfColumns(); i++) {
         if (i < static_cast<int>(fileBeingReadColumnNames.size())) {
            dff.setColumnName(i, fileBeingReadColumnNames[i]);
         }
      }
      deformationFieldFile->append(dff, columnDestination, fcm);
   }
   catch (FileException& e) {
      throw FileException(FileUtilities::basename(name), e.whatQString());
   }
   deformationFieldFile->setModified();
   displaySettingsDeformationField->update(); 
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getDeformationFieldFileTag(), name);
   }
}


/**      
 * Read the deformation field data file file
 */
void 
BrainSet::readDeformationFieldFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexDeformationFieldFile);
   
   if (append == false) {
      clearDeformationFieldFile();
   }
   const unsigned long modified = deformationFieldFile->getModified();
   
   if (deformationFieldFile->getNumberOfColumns() == 0) {         
      try {
         deformationFieldFile->readFile(name);
         if (deformationFieldFile->getNumberOfNodes() != getNumberOfNodes()) {
            throw FileException(FileUtilities::basename(name), numNodesMessage);
         }
      }
      catch (FileException& e) {
         clearDeformationFieldFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      DeformationFieldFile dff;
      dff.readFile(name);
      if (dff.getNumberOfNodes() != getNumberOfNodes()) {
         throw FileException(FileUtilities::basename(name), numNodesMessage);
      }
      try {
         deformationFieldFile->append(dff);
      }
      catch (FileException& e) {
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   deformationFieldFile->setModifiedCounter(modified);
   displaySettingsDeformationField->update(); 
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getDeformationFieldFileTag(), name);
   }
}
      
/** 
 * Write the paint data file.
 */ 
void
BrainSet::writePaintFile(const QString& name) throw (FileException)
{
   paintFile->getLabelTable()->assignColors(*areaColorFile);
   loadedFilesSpecFile.paintFile.setAllSelections(SpecFile::SPEC_FALSE);
   paintFile->writeFile(name);
   addToSpecFile(SpecFile::getPaintFileTag(), name);
}

/**
 * read the paint data file file (only selected columns).
 */
void 
BrainSet::readPaintFile(const QString& name, 
                        const std::vector<int>& columnDestination,
                        const std::vector<QString>& fileBeingReadColumnNames,
                        const AbstractFile::FILE_COMMENT_MODE fcm,
                        const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexPaintFile);
   
   const bool paintFileEmpty = paintFile->empty();
   
   PaintFile pf;
   pf.readFile(name);
   if (pf.getNumberOfNodes() != getNumberOfNodes()) {
      throw FileException(FileUtilities::basename(name), numNodesMessage);
   }
   try {
      for (int i = 0; i < pf.getNumberOfColumns(); i++) {
         if (i < static_cast<int>(fileBeingReadColumnNames.size())) {
            pf.setColumnName(i, fileBeingReadColumnNames[i]);
         }
      }
      std::vector<int> columnDestination2 = columnDestination;
      if ((pf.getFileReadType() == AbstractFile::FILE_FORMAT_XML) ||
          (pf.getFileReadType() == AbstractFile::FILE_FORMAT_XML_BASE64) ||
          (pf.getFileReadType() == AbstractFile::FILE_FORMAT_XML_GZIP_BASE64)) {
         if (pf.getLabelTable()->getHadColorsWhenRead()) {
             // allow area colors to override label table colors
             pf.getLabelTable()->assignColors(*areaColorFile);
             pf.getLabelTable()->addColorsToColorFile(*areaColorFile);
             if (DebugControl::getDebugOn()) {
                std::cout << "After GIFTI Label File reading there are "
                          << areaColorFile->getNumberOfColors()
                          << " area colors."
                          << std::endl;
             }
         }
      }
      paintFile->append(pf, columnDestination2, fcm);
      paintFile->getLabelTable()->assignColors(*areaColorFile);
   }
   catch (FileException& e) {
      throw FileException(FileUtilities::basename(name), e.whatQString());
   }
   if (paintFileEmpty) {
      paintFile->clearModified();
   }
   else {
      paintFile->setModified();
   }

   if (readingSpecFileFlag == false) {
      displaySettingsPaint->update();
   }
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getPaintFileTag(), name);
   }
}

/**      
 * Read the paint data file file
 */
void 
BrainSet::readPaintFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexPaintFile);
   
   if (append == false) {
      clearPaintFile();
   }
   const unsigned long modified = paintFile->getModified();
   
   if (paintFile->getNumberOfColumns() == 0) {         
      try {
         paintFile->readFile(name);
         if (paintFile->getNumberOfNodes() != getNumberOfNodes()) {
            throw FileException(FileUtilities::basename(name), numNodesMessage);
         }
         if ((paintFile->getFileReadType() == AbstractFile::FILE_FORMAT_XML) ||
             (paintFile->getFileReadType() == AbstractFile::FILE_FORMAT_XML_BASE64) ||
             (paintFile->getFileReadType() == AbstractFile::FILE_FORMAT_XML_GZIP_BASE64)) {
            if (paintFile->getLabelTable()->getHadColorsWhenRead()) {
                paintFile->getLabelTable()->assignColors(*areaColorFile);
                paintFile->getLabelTable()->addColorsToColorFile(*areaColorFile);
                if (DebugControl::getDebugOn()) {
                   std::cout << "After GIFTI Label File reading there are "
                             << areaColorFile->getNumberOfColors()
                             << " colors."
                             << std::endl;
                }
            }
         }
      }
      catch (FileException& e) {
         clearPaintFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      PaintFile pf;
      pf.readFile(name);
      if (pf.getNumberOfNodes() != getNumberOfNodes()) {
         throw FileException(FileUtilities::basename(name), numNodesMessage);
      }
      try {
         paintFile->append(pf);
         if ((pf.getFileReadType() == AbstractFile::FILE_FORMAT_XML) ||
             (pf.getFileReadType() == AbstractFile::FILE_FORMAT_XML_BASE64) ||
             (pf.getFileReadType() == AbstractFile::FILE_FORMAT_XML_GZIP_BASE64)) {
            if (pf.getLabelTable()->getHadColorsWhenRead()) {
                pf.getLabelTable()->assignColors(*areaColorFile);
                pf.getLabelTable()->addColorsToColorFile(*areaColorFile);
                if (DebugControl::getDebugOn()) {
                   std::cout << "After GIFTI Label File reading there are "
                             << areaColorFile->getNumberOfColors()
                             << " colors."
                             << std::endl;
                }
            }
         }
      }
      catch (FileException& e) {
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   paintFile->getLabelTable()->assignColors(*areaColorFile);
   paintFile->setModifiedCounter(modified);

   if (readingSpecFileFlag == false) {
      displaySettingsPaint->update();
   }
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getPaintFileTag(), name);
   }
}

/**      
 * Read the study meta data file 
 */
void 
BrainSet::readStudyMetaDataFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexStudyMetaDataFile);
   
   if (append == false) {
      clearStudyMetaDataFile();
   }
   const unsigned long modified = studyMetaDataFile->getModified();
   
   if (studyMetaDataFile->empty()) {
      try {
         studyMetaDataFile->readFile(name);
      }
      catch (FileException& e) {
         clearStudyMetaDataFile();
         throw e;
      }
   }
   else {
      StudyMetaDataFile smdf;
      smdf.readFile(name);
      studyMetaDataFile->append(smdf);
   }
   
   studyMetaDataFile->setModifiedCounter(modified);
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getStudyMetaDataFileTag(), name);
   }
   
   if (readingSpecFileFlag == false) {
      displaySettingsStudyMetaData->update();
   }
}
      
/** 
 * Write the study meta data file.
 */ 
void
BrainSet::writeStudyMetaDataFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.studyMetaDataFile.setAllSelections(SpecFile::SPEC_FALSE);
   studyMetaDataFile->writeFile(name);
   addToSpecFile(SpecFile::getStudyMetaDataFileTag(), name);
}

/**      
 * Read the study collection file 
 */
void 
BrainSet::readStudyCollectionFile(const QString& name, const bool append,
                                  const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexStudyCollectionFile);
   
   if (append == false) {
      clearStudyCollectionFile();
   }
   const unsigned long modified = studyCollectionFile->getModified();
   
   if (studyCollectionFile->empty()) {
      try {
         studyCollectionFile->readFile(name);
      }
      catch (FileException& e) {
         clearStudyCollectionFile();
         throw e;
      }
   }
   else {
      StudyCollectionFile scf;
      scf.readFile(name);
      studyCollectionFile->append(scf);
   }
   
   studyCollectionFile->setModifiedCounter(modified);
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getStudyCollectionFileTag(), name);
   }
   
   if (readingSpecFileFlag == false) {
///      displaySettingsStudyMetaData->update();
   }
}
      
/** 
 * Write the study collection file.
 */ 
void
BrainSet::writeStudyCollectionFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.studyCollectionFile.setAllSelections(SpecFile::SPEC_FALSE);
   studyCollectionFile->writeFile(name);
   addToSpecFile(SpecFile::getStudyCollectionFileTag(), name);
}

      
/**      
 * Read the vocabulary data file 
 */
void 
BrainSet::readVocabularyFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexVocabularyFile);
   
   if (append == false) {
      clearVocabularyFile();
   }
   const unsigned long modified = vocabularyFile->getModified();
   
   if (vocabularyFile->empty()) {
      try {
         vocabularyFile->readFile(name);
      }
      catch (FileException& e) {
         clearVocabularyFile();
         throw e;
      }
   }
   else {
      VocabularyFile vf;
      vf.readFile(name);
      vocabularyFile->append(vf);
   }
   
   vocabularyFile->setModifiedCounter(modified);
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getVocabularyFileTag(), name);
   }
}
      
/** 
 * Write the vocabulary data file.
 */ 
void
BrainSet::writeVocabularyFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.vocabularyFile.setAllSelections(SpecFile::SPEC_FALSE);
   vocabularyFile->writeFile(name);
   addToSpecFile(SpecFile::getVocabularyFileTag(), name);
}

/**      
 * Read the wustl region data file 
 */
void 
BrainSet::readWustlRegionFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexWustlRegionFile);
   
   if (append == false) {
      clearWustlRegionFile();
   }
   const unsigned long modified = wustlRegionFile->getModified();
   
   if (wustlRegionFile->getNumberOfTimeCourses() == 0) {
      try {
         wustlRegionFile->readFile(name);
      }
      catch (FileException& e) {
         clearWustlRegionFile();
         throw e;
      }
   }
   else {
      WustlRegionFile wrf;
      wrf.readFile(name);
      wustlRegionFile->append(wrf);
   }
   
   wustlRegionFile->setModifiedCounter(modified);
   displaySettingsWustlRegion->update();
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getWustlRegionFileTag(), name);
   }
}
      
/** 
 * Write the wustl region data file.
 */ 
void
BrainSet::writeWustlRegionFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.wustlRegionFile.setAllSelections(SpecFile::SPEC_FALSE);
   wustlRegionFile->writeFile(name);
   addToSpecFile(SpecFile::getWustlRegionFileTag(), name);
}

/** 
 * Write the Palette data file.
 */ 
void
BrainSet::writePaletteFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.paletteFile.setAllSelections(SpecFile::SPEC_FALSE);
   paletteFile->writeFile(name);
   addToSpecFile(SpecFile::getPaletteFileTag(), name);
}

/**      
 * Read the palette data file file
 */
void 
BrainSet::readPaletteFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexPaletteFile);
   
   if (append == false) {
      clearPaletteFile();
   }
   const unsigned long modified = paletteFile->getModified();
   
   if (paletteFile->getNumberOfPalettes() == 0) {
      try {
         paletteFile->readFile(name);
      }
      catch (FileException& e) {
         clearPaletteFile();
         throw e;
      }
   }
   else {
      PaletteFile pf;
      pf.readFile(name);
      paletteFile->append(pf);
   }
   
   paletteFile->setModifiedCounter(modified);
   displaySettingsMetric->update(); 
   displaySettingsSurfaceShape->update();
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getPaletteFileTag(), name);
   }
}
      
/** 
 * Write the params data file.
 */ 
void
BrainSet::writeParamsFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.paramsFile.setAllSelections(SpecFile::SPEC_FALSE);
   paramsFile->writeFile(name);
   addToSpecFile(SpecFile::getParamsFileTag(), name);
}

/**      
 * Read the params data file file
 */
void 
BrainSet::readParamsFile(const QString& name, const bool append,
                         const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexParamsFile);
   
   if (append == false) {
      clearParamsFile();
   }
   const unsigned long modified = paramsFile->getModified();
   
   if (paramsFile->empty() == 0) {
      try {
         paramsFile->readFile(name);
      }
      catch (FileException& e) {
         clearParamsFile();
         throw e;
      }
   }
   else {
      ParamsFile pf;
      pf.readFile(name);
      QString msg;
      paramsFile->append(pf, msg);
      if (msg.isEmpty() == false) {
         throw FileException(name, msg);
      }
   }
   
   paramsFile->setModifiedCounter(modified);
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getParamsFileTag(), name);
   }
}
      
/** 
 * Write the probabilistic atlas data file.
 */ 
void
BrainSet::writeProbabilisticAtlasFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.atlasFile.setAllSelections(SpecFile::SPEC_FALSE);
   probabilisticAtlasSurfaceFile->writeFile(name);
   addToSpecFile(SpecFile::getAtlasFileTag(), name);
}

/**      
 * Read the prob atlas data file file
 */
void 
BrainSet::readProbabilisticAtlasFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexProbAtlasFile);
   
   if (append == false) {
      clearProbabilisticAtlasFile();
   }
   const unsigned long modified = probabilisticAtlasSurfaceFile->getModified();
   
   if (probabilisticAtlasSurfaceFile->getNumberOfColumns() == 0) {         
      try {
         probabilisticAtlasSurfaceFile->readFile(name);
         if (probabilisticAtlasSurfaceFile->getNumberOfNodes() != getNumberOfNodes()) {
            throw FileException(FileUtilities::basename(name), numNodesMessage);
         }
      }
      catch (FileException& e) {
         clearProbabilisticAtlasFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      ProbabilisticAtlasFile paf;
      paf.readFile(name);
      if (paf.getNumberOfNodes() != getNumberOfNodes()) {
         throw FileException(FileUtilities::basename(name), numNodesMessage);
      }
      try {
         probabilisticAtlasSurfaceFile->append(paf);
      }
      catch (FileException& e) {
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   probabilisticAtlasSurfaceFile->setModifiedCounter(modified);
   displaySettingsProbabilisticAtlasSurface->update();
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getAtlasFileTag(), name);
   }
}
      
/** 
 * Write the rgb paint data file.
 */ 
void
BrainSet::writeRgbPaintFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.rgbPaintFile.setAllSelections(SpecFile::SPEC_FALSE);
   rgbPaintFile->writeFile(name);
   addToSpecFile(SpecFile::getRgbPaintFileTag(), name);
}

/**      
 * Read the rgb paint data file file
 */
void 
BrainSet::readRgbPaintFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexRgbPaintFile);
   
   if (append == false) {
      clearRgbPaintFile();
   }
   const unsigned long modified = rgbPaintFile->getModified();
   
   if (rgbPaintFile->getNumberOfColumns() == 0) {         
      try {
         rgbPaintFile->readFile(name);
         if (rgbPaintFile->getNumberOfNodes() != getNumberOfNodes()) {
            throw FileException(FileUtilities::basename(name), numNodesMessage);
         }
      }
      catch (FileException& e) {
         clearRgbPaintFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      RgbPaintFile rf;
      rf.readFile(name);
      if (rf.getNumberOfNodes() != getNumberOfNodes()) {
         throw FileException(FileUtilities::basename(name), numNodesMessage);
      }
      try {
         rgbPaintFile->append(rf);
      }
      catch (FileException& e) {
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   rgbPaintFile->setModifiedCounter(modified);
   displaySettingsRgbPaint->update();  
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getRgbPaintFileTag(), name);
   }
}
      
/** 
 * Write the scene data file.
 */ 
void
BrainSet::writeSceneFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.sceneFile.setAllSelections(SpecFile::SPEC_FALSE);
   sceneFile->writeFile(name);
   addToSpecFile(SpecFile::getSceneFileTag(), name);
}

/**      
 * Read the scene data file file
 */
void 
BrainSet::readSceneFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexSceneFile);
   
   if (append == false) {
      clearSceneFile();
   }
   const unsigned long modified = sceneFile->getModified();
   
   if (sceneFile->empty()) {
      sceneFile->readFile(name);
      //
      // Special processing for WebCaret.  In SuMS, a spec file and its data files are
      // stored in a single directory.  However, a scene file may still have relative paths
      // for data files loaded at the time of scene creation so strip the paths from the files
      //
      if (webCaretFlag) {
         sceneFile->removePathsFromAllSpecFileDataFileNames();
      }
   }
   else {
      try {
         SceneFile sf;
         sf.readFile(name);
         //
         // Special processing for WebCaret.  In SuMS, a spec file and its data files are
         // stored in a single directory.  However, a scene file may still have relative paths
         // for data files loaded at the time of scene creation so strip the paths from the files
         //
         if (webCaretFlag) {
            sf.removePathsFromAllSpecFileDataFileNames();
         }
         sceneFile->append(sf);
      }
      catch (FileException& e) {
         clearSceneFile();
         throw e;
      }
   }
   sceneFile->setModifiedCounter(modified);
   displaySettingsScene->update();
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getSceneFileTag(), name);
   }
}
      
/** 
 * Write the section data file.
 */ 
void
BrainSet::writeSectionFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.sectionFile.setAllSelections(SpecFile::SPEC_FALSE);
   sectionFile->writeFile(name);
   addToSpecFile(SpecFile::getSectionFileTag(), name);
}

/**      
 * Read the section data file file
 */
void 
BrainSet::readSectionFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexSectionFile);
   
   if (append == false) {
      clearSectionFile();
   }
   const unsigned long modified = sectionFile->getModified();
   
   if (sectionFile->getNumberOfColumns() == 0) {         
      try {
         sectionFile->readFile(name);
         if (sectionFile->getNumberOfNodes() != getNumberOfNodes()) {
            throw FileException(FileUtilities::basename(name), numNodesMessage);
         }
      }
      catch (FileException& e) {
         clearSectionFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      SectionFile sf;
      sf.readFile(name);
      if (sf.getNumberOfNodes() != getNumberOfNodes()) {
         throw FileException(FileUtilities::basename(name), numNodesMessage);
      }
      try {
         sectionFile->append(sf);
      }
      catch (FileException& e) {
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   sectionFile->setModifiedCounter(modified);
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getSectionFileTag(), name);
   }
   displaySettingsSection->update();
}
      
/** 
 * Write the surface shape data file.
 */ 
void
BrainSet::writeSurfaceShapeFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.surfaceShapeFile.setAllSelections(SpecFile::SPEC_FALSE);
   surfaceShapeFile->writeFile(name);
   addToSpecFile(SpecFile::getSurfaceShapeFileTag(), name);
}

/**
 * read the surface shape data file file (only selected columns).
 */
void 
BrainSet::readSurfaceShapeFile(const QString& name, 
                               const std::vector<int>& columnDestination,
                               const std::vector<QString>& fileBeingReadColumnNames,
                               const AbstractFile::FILE_COMMENT_MODE fcm,
                               const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexSurfaceShapeFile);
   
   const bool shapeEmpty = surfaceShapeFile->empty();
   
   SurfaceShapeFile ssf;
   ssf.setNumberOfNodesForSparseNodeIndexFiles(getNumberOfNodes());
   ssf.readFile(name);
   if (ssf.getNumberOfNodes() != getNumberOfNodes()) {
      throw FileException(FileUtilities::basename(name), numNodesMessage);
   }
   try {
      for (int i = 0; i < ssf.getNumberOfColumns(); i++) {
         if (i < static_cast<int>(fileBeingReadColumnNames.size())) {
            ssf.setColumnName(i, fileBeingReadColumnNames[i]);
         }
      }
      std::vector<int> columnDestination2 = columnDestination;
      surfaceShapeFile->append(ssf, columnDestination2, fcm);
   }
   catch (FileException& e) {
      throw FileException(FileUtilities::basename(name), e.whatQString());
   }
   
   if (shapeEmpty) {
      surfaceShapeFile->clearModified();
   }
   else {
      surfaceShapeFile->setModified();
   }

   if (readingSpecFileFlag == false) {
      displaySettingsSurfaceShape->update();  
   }
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getSurfaceShapeFileTag(), name);
   }
}

/**      
 * Read the surface shape data file file
 */
void 
BrainSet::readSurfaceShapeFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexSurfaceShapeFile);
   
   if (append == false) {
      clearSurfaceShapeFile();
   }
   const unsigned long modified = surfaceShapeFile->getModified();
   
   if (surfaceShapeFile->getNumberOfColumns() == 0) {         
      try {
         surfaceShapeFile->readFile(name);
         if (surfaceShapeFile->getNumberOfNodes() != getNumberOfNodes()) {
            throw FileException(FileUtilities::basename(name), numNodesMessage);
         }
      }
      catch (FileException& e) {
         clearSurfaceShapeFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      SurfaceShapeFile ssf;
      ssf.readFile(name);
      if (ssf.getNumberOfNodes() != getNumberOfNodes()) {
         throw FileException(FileUtilities::basename(name), numNodesMessage);
      }
      try {
         surfaceShapeFile->append(ssf);
      }
      catch (FileException& e) {
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   surfaceShapeFile->setModifiedCounter(modified);

   if (readingSpecFileFlag == false) {
      displaySettingsSurfaceShape->update();  
   }
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getSurfaceShapeFileTag(), name);
   }
}
      
/** 
 * Write the vector data file.
 */ 
void
BrainSet::writeVectorFile(VectorFile* vf, const QString& name) throw (FileException)
{
   loadedFilesSpecFile.vectorFile.clearSelectionStatus(vf->getFileName());
   vf->writeFile(name);
   addToSpecFile(SpecFile::getVectorFileTag(), name);
}

/**      
 * Read the vector data file file
 */
void 
BrainSet::readVectorFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexVectorFile);
   
   if (append == false) {
      clearVectorFiles();
   }
   VectorFile* vf = NULL;
   try {
      vf = new VectorFile();
      vf->readFile(name);
      addVectorFile(vf);
   }
   catch (FileException& e) {
      if (vf != NULL) {
         delete vf;
      }
      throw FileException(FileUtilities::basename(name), e.whatQString());
   }
   displaySettingsVectors->update();  
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getVectorFileTag(), name);
   }
   clearAllDisplayLists();
}

/** 
 * Write the topography data file.
 */ 
void
BrainSet::writeTopographyFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.topographyFile.setAllSelections(SpecFile::SPEC_FALSE);
   topographyFile->writeFile(name);
   addToSpecFile(SpecFile::getTopographyFileTag(), name);
}

/**      
 *BrainSet::read the topography data file file
 */
void 
BrainSet::readTopographyFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexTopographyFile);
   
   if (append == false) {
      clearTopographyFile();
   }
   const unsigned long modified = topographyFile->getModified();
   
   if (topographyFile->getNumberOfColumns() == 0) {         
      try {
         topographyFile->setNumberOfNodesVersion0File(getNumberOfNodes());
         topographyFile->readFile(name);
         if (topographyFile->getNumberOfNodes() != getNumberOfNodes()) {
            throw FileException(FileUtilities::basename(name), numNodesMessage);
         }
      }
      catch (FileException& e) {
         clearTopographyFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      TopographyFile tf;
      tf.setNumberOfNodesVersion0File(getNumberOfNodes());
      tf.readFile(name);
      if (tf.getNumberOfNodes() != getNumberOfNodes()) {
         throw FileException(FileUtilities::basename(name), numNodesMessage);
      }
      try {
         topographyFile->append(tf);
      }
      catch (FileException& e) {
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   topographyFile->setModifiedCounter(modified);
   displaySettingsTopography->update();
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getTopographyFileTag(), name);
   }
}

/** 
 * Write the transformation matrix data file.
 */ 
void
BrainSet::writeTransformationMatrixFile(const QString& name) throw (FileException)
{
   loadedFilesSpecFile.transformationMatrixFile.setAllSelections(SpecFile::SPEC_FALSE);
   transformationMatrixFile->writeFile(name);
   addToSpecFile(SpecFile::getTransformationMatrixFileTag(), name);
}

/**      
 * read the transformation matrix file
 */
void 
BrainSet::readTransformationMatrixFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexTransformationMatrixFile);
   
   if (append == false) {
      clearTransformationMatrixFile();
   }
   const unsigned long modified = transformationMatrixFile->getModified();
   
   if (transformationMatrixFile->getNumberOfMatrices() == 0) {         
      try {
         transformationMatrixFile->readFile(name);
      }
      catch (FileException& e) {
         clearTransformationMatrixFile();
         throw FileException(FileUtilities::basename(name), e.whatQString());
      }
   }
   else {
      // Append to existing 
      TransformationMatrixFile tf;
      tf.readFile(name);
      transformationMatrixFile->append(tf);
   }
   transformationMatrixFile->setModifiedCounter(modified);
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getTransformationMatrixFileTag(), name);
   }
}

/**      
 * read the transformation matrix data file
 */
void 
BrainSet::readTransformationDataFile(const QString& name, const bool append,
                            const bool updateSpec) throw (FileException)
{
   QMutexLocker locker(&mutexTransformationDataFile);
   
   if (append == false) {
      transformationDataFiles.clear();
   }

   QString errorMessage;
   AbstractFile* ab = AbstractFile::readAnySubClassDataFile(name,
                                                            false,
                                                            errorMessage);
   if (ab == NULL) {
      throw FileException(FileUtilities::basename(name), errorMessage);
   }

   transformationDataFiles.push_back(ab);
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getTransformationDataFileTag(), name);
   }
}

/** 
 * Write the topology data file.
 */ 
void
BrainSet::writeTopologyFile(const QString& name,
                    const TopologyFile::TOPOLOGY_TYPES tt,
                    TopologyFile* tf) throw (FileException)
{
   switch(tf->getTopologyType()) {
      case TopologyFile::TOPOLOGY_TYPE_CLOSED:
         loadedFilesSpecFile.closedTopoFile.clearSelectionStatus(tf->getFileName());
         break;
      case TopologyFile::TOPOLOGY_TYPE_OPEN:
         loadedFilesSpecFile.openTopoFile.clearSelectionStatus(tf->getFileName());
         break;
      case TopologyFile::TOPOLOGY_TYPE_CUT:
         loadedFilesSpecFile.cutTopoFile.clearSelectionStatus(tf->getFileName());
         break;
      case TopologyFile::TOPOLOGY_TYPE_LOBAR_CUT:
         loadedFilesSpecFile.lobarCutTopoFile.clearSelectionStatus(tf->getFileName());
         break;
      case TopologyFile::TOPOLOGY_TYPE_UNKNOWN:
      case TopologyFile::TOPOLOGY_TYPE_UNSPECIFIED:
         loadedFilesSpecFile.unknownTopoFile.clearSelectionStatus(tf->getFileName());
         break;
   }


   tf->setTopologyType(tt);
   QString tag;
   switch(tt) {
      case TopologyFile::TOPOLOGY_TYPE_CLOSED:
         tag = SpecFile::getClosedTopoFileTag();
         break;
      case TopologyFile::TOPOLOGY_TYPE_OPEN:
         tag = SpecFile::getOpenTopoFileTag();
         break;
      case TopologyFile::TOPOLOGY_TYPE_CUT:
         tag = SpecFile::getCutTopoFileTag();
         break;
      case TopologyFile::TOPOLOGY_TYPE_LOBAR_CUT:
         tag = SpecFile::getLobarCutTopoFileTag();
         break;
      case TopologyFile::TOPOLOGY_TYPE_UNKNOWN:
      case TopologyFile::TOPOLOGY_TYPE_UNSPECIFIED:
         tag = SpecFile::getUnknownTopoFileMatchTag();
         break;
   }
   
   tf->writeFile(name);
   addToSpecFile(tag, name);
}

/**      
 * Read the topology data file file.  When reading spec file, new topology files are placed at
 * the end of the vector that holds them.  When not reading a spec file, the user is opening
 * a topology file so make it the active one.
 */
void 
BrainSet::readTopologyFile(const QString& name, const TopologyFile::TOPOLOGY_TYPES ttin,
                            const bool append,
                            const bool updateSpec) throw (FileException)
{
   TopologyFile::TOPOLOGY_TYPES tt = ttin;
   if ((tt == TopologyFile::TOPOLOGY_TYPE_UNSPECIFIED) ||
       (tt == TopologyFile::TOPOLOGY_TYPE_UNKNOWN)) {
      TopologyFile tf;
      tf.readFileMetaDataOnly(name);
      const QString typeTag = tf.getHeaderTag(AbstractFile::headerTagPerimeterID);
      if (typeTag.isEmpty() == false) {
         tt = TopologyFile::getTopologyTypeFromPerimeterID(typeTag);
      }
   }
   
   TopologyFile* topoJustRead = new TopologyFile;
   try {
      topoJustRead->readFile(name);
   }
   catch (FileException& e) {
      delete topoJustRead;
      throw FileException(FileUtilities::basename(name), e.whatQString());
   }
   
   QString tag;
   
   //
   // This keeps track of any topology files that are being deleted (because append == false).
   // This allows the new topology file to be applied to coordinate files.
   //
   std::vector<TopologyFile*> replacedTopologyFiles;
   
   //
   // The remainder of this routine must not be run by more than one thread
   //
   QMutexLocker locker(&mutexReadTopologyFile);
   
   //
   // If not appending, remove any topology files that are the same type as
   // the topology file being read.  The newest file is always placed in the 
   // first spot.
   //
   if (append == false) {
      std::vector<TopologyFile*> tempFiles;
      tempFiles.push_back(topoJustRead);
      
      const int numTopo = getNumberOfTopologyFiles();
      for (int i = 0; i < numTopo; i++) {
         TopologyFile* tf = getTopologyFile(i);
         if (tt == tf->getTopologyType()) {
            replacedTopologyFiles.push_back(tf);
            delete tf;
         }
         else {
            tempFiles.push_back(tf);
         }
      }
      topologyFiles = tempFiles;
   }
   else {
      topologyFiles.insert(topologyFiles.begin(), topoJustRead);
   }
   
   //
   // Override topology type in the TopologyFile just read.
   //
   if (topoJustRead->getTopologyType() != tt) {
      topoJustRead->setTopologyType(tt);
   }

   switch(tt) {
      case TopologyFile::TOPOLOGY_TYPE_CLOSED:
         tag = SpecFile::getClosedTopoFileTag();
         break;
      case TopologyFile::TOPOLOGY_TYPE_OPEN:
         tag = SpecFile::getOpenTopoFileTag();
         break;
      case TopologyFile::TOPOLOGY_TYPE_CUT:
         tag = SpecFile::getCutTopoFileTag();
         break;
      case TopologyFile::TOPOLOGY_TYPE_LOBAR_CUT:
         tag = SpecFile::getLobarCutTopoFileTag();
         break;
      case TopologyFile::TOPOLOGY_TYPE_UNKNOWN:
      case TopologyFile::TOPOLOGY_TYPE_UNSPECIFIED:
      default:
         tag = SpecFile::getUnknownTopoFileMatchTag();
         break;
   }
   
   if (updateSpec) {
      addToSpecFile(tag, name);
   }
   
   //
   // If any surface was using a deleted topology file, replace it with
   // the topology file that was just read.
   //
   if (replacedTopologyFiles.size() > 0) {
      for (int i = 0; i < getNumberOfBrainModels(); i++) {
         BrainModel* bm = getBrainModel(i);
         if (bm->getModelType() == BrainModel::BRAIN_MODEL_SURFACE) {
            BrainModelSurface* bms = dynamic_cast<BrainModelSurface*>(bm);
            TopologyFile* stf = bms->getTopologyFile();
            if (std::find(replacedTopologyFiles.begin(), replacedTopologyFiles.end(), stf) !=
                replacedTopologyFiles.end()) {
               bms->setTopologyFile(topoJustRead);
            }
         }
      }
   }
   
   setSelectedTopologyFiles();
}

/**
 * Add a topology file
 */
void
BrainSet::addTopologyFile(TopologyFile* tf)
{
   topologyFiles.push_back(tf);
}

/**
 * delete topology file.
 */
void 
BrainSet::deleteTopologyFile(TopologyFile* tf)
{
   if (tf == NULL) {
      return;
   }
   
   loadedFilesSpecFile.closedTopoFile.clearSelectionStatus(tf->getFileName());
   loadedFilesSpecFile.openTopoFile.clearSelectionStatus(tf->getFileName());
   loadedFilesSpecFile.cutTopoFile.clearSelectionStatus(tf->getFileName());
   loadedFilesSpecFile.lobarCutTopoFile.clearSelectionStatus(tf->getFileName());
   loadedFilesSpecFile.unknownTopoFile.clearSelectionStatus(tf->getFileName());
                                            
   int topoIndex = -1;
   std::vector<TopologyFile*> savedTopologyFiles;
   for (int i = 0; i < getNumberOfTopologyFiles(); i++) {
      TopologyFile* topology = getTopologyFile(i);
      if (topology != tf) {
         savedTopologyFiles.push_back(topology);
      }
      else {
         topoIndex = i;
      }
   }
   topologyFiles = savedTopologyFiles;
   delete tf;

   setSelectedTopologyFiles();
   
   //
   // If delete topology was used by a surface, give the surface a new topo file
   //
   if ((topoIndex < 0) || (topoIndex >= getNumberOfTopologyFiles())) {
      topoIndex = 0;
   }
   TopologyFile* newTF = NULL;
   if ((topoIndex >= 0) && (topoIndex < getNumberOfTopologyFiles())) {
      newTF = getTopologyFile(topoIndex);
   }
   for (int i = 0; i < getNumberOfBrainModels(); i++) {
      BrainModelSurface* bms = getBrainModelSurface(i);
      if (bms != NULL) {
         if (bms->getTopologyFile() == tf) {
            bms->setTopologyFile(newTF);
         }
      }
   }
   clearAllDisplayLists();
}      

/**
 * Read in surface file group.
 */
bool 
BrainSet::readSurfaceFileGroup(const SpecFile::Entry& surfaceFile,
                               const BrainModelSurface::SURFACE_TYPES surfaceType,
                               std::vector<QString>& errorMessages,
                               int& progressFileCounter,
                               QProgressDialog* progressDialog)
{
   for (unsigned i = 0; i < surfaceFile.files.size(); i++) {
      if (surfaceFile.files[i].selected) {
         if (updateFileReadProgressDialog(surfaceFile.files[i].filename, progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readSurfaceFile(surfaceFile.files[i].filename, surfaceType, true, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   return false;
}      

/**
 * Read some coordinate files.  Returns true if file reading was aborted.
 */
bool
BrainSet::readCoordinateFileGroup(const SpecFile::Entry& coordFile,
                               const BrainModelSurface::SURFACE_TYPES surfaceType,
                               std::vector<QString>& errorMessages,
                               int& progressFileCounter,
                               QProgressDialog* progressDialog)
{
   for (unsigned i = 0; i < coordFile.files.size(); i++) {
      if (coordFile.files[i].selected) {
         if (updateFileReadProgressDialog(coordFile.files[i].filename, progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readCoordinateFile(coordFile.files[i].filename, surfaceType, true, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   return false;
}

/**
 * Read some border files.  Returns true if file reading was aborted.
 */
bool
BrainSet::readBorderFiles(const SpecFile::Entry& borderFilesToRead,
                              const BrainModelSurface::SURFACE_TYPES bt,
                              std::vector<QString>& errorMessages,
                              int& progressFileCounter,
                              QProgressDialog* progressDialog)
{
   for (unsigned i = 0; i < borderFilesToRead.files.size(); i++) {
      if (borderFilesToRead.files[i].selected) {
         if (updateFileReadProgressDialog(borderFilesToRead.files[i].filename, progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readBorderFile(borderFilesToRead.files[i].filename, bt, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   return false;
}

/**
 * read the spec file (returns true if reading was aborted by user).
 */
bool 
BrainSet::readSpecFile(const SpecFile& specFileIn, 
                       const QString& specFileNameIn,
                       QString& errorMessagesOut)
{
   return readSpecFile(SPEC_FILE_READ_MODE_NORMAL,
                       specFileIn,
                       specFileNameIn,
                       errorMessagesOut,
                       NULL,
                       NULL);
}

/**
 * Read the BrainSet data files.  Any error message will be placed
 * into "errorMessageOut".  If "errorMessageOut" is empty, then there
 * were no errors reading the spec file's data files.
 *
 * Returns "true" if the user aborted loading files with the progress dialog.
 */
bool
BrainSet::readSpecFile(const SPEC_FILE_READ_MODE specReadMode,
                       const SpecFile& specFileIn,
                       const QString& specFileNameIn,
                       QString& errorMessageOut,
                       const TransformationMatrix* specTransformationMatrixIn,
                       QProgressDialog* progressDialog)
{
   errorMessageOut = "";
   std::vector<QString> msg;
   const bool b = readSpecFile(specReadMode,
                               specFileIn,
                               specFileNameIn,
                               msg,
                               specTransformationMatrixIn,
                               progressDialog);
   if (msg.empty() == false) {
      errorMessageOut = StringUtilities::combine(msg, "\n");
   }
   return b;
}

/**
 * Read the BrainSet data files.  Any error message will be placed
 * into "errorMessages".  The size of errorMessages will correspond to
 * the number files that failed to read correctly.  If the size of 
 * errorMessages is 0, no file reading errors were encountered.
 *
 * Returns "true" if the user aborted loading files with the progress dialog.
 */
bool
BrainSet::readSpecFile(const SPEC_FILE_READ_MODE specReadMode,
                       const SpecFile& specFileIn,
                       const QString& specFileNameIn,
                       std::vector<QString>& errorMessages,
                       const TransformationMatrix* specTransformationMatrixIn,
                       QProgressDialog* progressDialog)
{
   if (getPreferencesFile()->getNumberOfFileReadingThreads() > 1) {
      return readSpecFileMultiThreaded(specReadMode,
                                specFileIn,
                                specFileNameIn,
                                errorMessages,
                                specTransformationMatrixIn,
                                progressDialog);
   }
   
   specFileTimeOfLoading = QDateTime::currentDateTime();

   readingSpecFileFlag = true;
   
   switch (specReadMode) {
      case SPEC_FILE_READ_MODE_NORMAL:
         //
         // clear out "this" brain set
         //
         reset();
         break;
      case SPEC_FILE_READ_MODE_APPEND:
         break;
   }
   
   if (specTransformationMatrixIn != NULL) {
      specDataFileTransformationMatrix = *specTransformationMatrixIn;
   }
   
   errorMessages.clear();

   int progressFileCounter = 0;
   
   switch (specReadMode) {
      case SPEC_FILE_READ_MODE_NORMAL:
         //
         // Copy spec file passed and clear all selections in BrainSet::specFile
         //
         loadedFilesSpecFile = specFileIn;
         loadedFilesSpecFile.setAllFileSelections(SpecFile::SPEC_FALSE);
         //loadedFilesSpecFile.setFileName(specFileNameIn);
         loadedFilesSpecFile.setCurrentDirectoryToSpecFileDirectory();
         
         specFileName = specFileNameIn;
         
         structure = specFileIn.getStructure();
/*
         hemisphere = Hemisphere::HEMISPHERE_UNKNOWN;
         if (specFileIn.getStructure() == "right") {
            hemisphere = BrainModelSurface::HEMISPHERE_RIGHT;
         }
         else if (specFileIn.getStructure() == "left") {
            hemisphere = BrainModelSurface::HEMISPHERE_LEFT;
         }
         else if (specFileIn.getStructure() == "both") {
            hemisphere = BrainModelSurface::HEMISPHERE_BOTH;
         }
         setStructure(hemisphere);
*/
         setSpecies(loadedFilesSpecFile.getSpecies());
         setSubject(loadedFilesSpecFile.getSubject());
         stereotaxicSpace = loadedFilesSpecFile.getSpace();
         resetDataFiles(false, false);
         break;
      case SPEC_FILE_READ_MODE_APPEND:
         break;
   }
   
   const int numTopoFilesBeforeLoading = getNumberOfTopologyFiles();
   
   //
   // Note about reading topology files.  "readTopologyFile()" always places the newest topology
   // file at the beginning of its storage in "topologyFiles".  So, when we read the topology
   // files from the spec file, read them starting with the oldest unknown to the newest 
   // closed topology file (the spec file sorts each file type by date). 
   //
   //
   // Read the unknown topology files
   //
   for (int j = (specFileIn.unknownTopoFile.files.size() - 1); j >= 0; j--) {
      if (specFileIn.unknownTopoFile.files[j].selected) {
         if (updateFileReadProgressDialog(specFileIn.unknownTopoFile.files[j].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         
         try {
            readTopologyFile(specFileIn.unknownTopoFile.files[j].filename, TopologyFile::TOPOLOGY_TYPE_UNKNOWN, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the lobar cut topology files
   //
   for (int j = (specFileIn.lobarCutTopoFile.files.size() - 1); j >= 0; j--) {
      if (specFileIn.lobarCutTopoFile.files[j].selected) {
         if (updateFileReadProgressDialog(specFileIn.lobarCutTopoFile.files[j].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         
         try {
            readTopologyFile(specFileIn.lobarCutTopoFile.files[j].filename, TopologyFile::TOPOLOGY_TYPE_LOBAR_CUT, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the cut topology files
   //
   for (int j = (specFileIn.cutTopoFile.files.size() - 1); j >= 0; j--) {
      if (specFileIn.cutTopoFile.files[j].selected) {
         if (updateFileReadProgressDialog(specFileIn.cutTopoFile.files[j].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         
         try {
            readTopologyFile(specFileIn.cutTopoFile.files[j].filename, TopologyFile::TOPOLOGY_TYPE_CUT, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the open topology files
   //
   for (int j = (specFileIn.openTopoFile.files.size() - 1); j >= 0; j--) {
      if (specFileIn.openTopoFile.files[j].selected) {
         if (updateFileReadProgressDialog(specFileIn.openTopoFile.files[j].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         
         try {
            readTopologyFile(specFileIn.openTopoFile.files[j].filename, TopologyFile::TOPOLOGY_TYPE_OPEN, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the closed topology files
   //
   for (int j = (specFileIn.closedTopoFile.files.size() - 1); j >= 0; j--) {
      if (specFileIn.closedTopoFile.files[j].selected) {
         if (updateFileReadProgressDialog(specFileIn.closedTopoFile.files[j].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         
         try {
            readTopologyFile(specFileIn.closedTopoFile.files[j].filename, TopologyFile::TOPOLOGY_TYPE_CLOSED, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   setSelectedTopologyFiles();
   
   //
   // Read in the raw coordinate files
   //
   if (readCoordinateFileGroup(specFileIn.rawCoordFile,
                    BrainModelSurface::SURFACE_TYPE_RAW,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the fiducial coordinate files
   //
   if (readCoordinateFileGroup(specFileIn.fiducialCoordFile,
                    BrainModelSurface::SURFACE_TYPE_FIDUCIAL,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the inflated coordinate files
   //
   if (readCoordinateFileGroup(specFileIn.inflatedCoordFile,
                    BrainModelSurface::SURFACE_TYPE_INFLATED,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the very inflated coordinate files
   //
   if (readCoordinateFileGroup(specFileIn.veryInflatedCoordFile,
                    BrainModelSurface::SURFACE_TYPE_VERY_INFLATED,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;
   
   //
   // Read in the spherical coordinate files
   //
   if (readCoordinateFileGroup(specFileIn.sphericalCoordFile,
                    BrainModelSurface::SURFACE_TYPE_SPHERICAL,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;
   
   //
   // Read in the ellipsoid coordinate files
   //
   if (readCoordinateFileGroup(specFileIn.ellipsoidCoordFile,
                    BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the compressed medial wall coordinate files
   //
   if (readCoordinateFileGroup(specFileIn.compressedCoordFile,
                    BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   //
   // Read in the flat coordinate files
   //
   if (readCoordinateFileGroup(specFileIn.flatCoordFile,
                    BrainModelSurface::SURFACE_TYPE_FLAT,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the lobar flat coordinate files
   //
   if (readCoordinateFileGroup(specFileIn.lobarFlatCoordFile,
                    BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the hull coordinate files
   //
   if (readCoordinateFileGroup(specFileIn.hullCoordFile,
                    BrainModelSurface::SURFACE_TYPE_HULL,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the unknown coordinate files
   //
   if (readCoordinateFileGroup(specFileIn.unknownCoordFile,
                    BrainModelSurface::SURFACE_TYPE_UNKNOWN,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;
        
   //
   // Read in the raw coordinate files
   //
   if (readSurfaceFileGroup(specFileIn.rawSurfaceFile,
                    BrainModelSurface::SURFACE_TYPE_RAW,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the fiducial coordinate files
   //
   if (readSurfaceFileGroup(specFileIn.fiducialSurfaceFile,
                    BrainModelSurface::SURFACE_TYPE_FIDUCIAL,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the inflated coordinate files
   //
   if (readSurfaceFileGroup(specFileIn.inflatedSurfaceFile,
                    BrainModelSurface::SURFACE_TYPE_INFLATED,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the very inflated coordinate files
   //
   if (readSurfaceFileGroup(specFileIn.veryInflatedSurfaceFile,
                    BrainModelSurface::SURFACE_TYPE_VERY_INFLATED,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;
   
   //
   // Read in the spherical coordinate files
   //
   if (readSurfaceFileGroup(specFileIn.sphericalSurfaceFile,
                    BrainModelSurface::SURFACE_TYPE_SPHERICAL,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;
   
   //
   // Read in the ellipsoid coordinate files
   //
   if (readSurfaceFileGroup(specFileIn.ellipsoidSurfaceFile,
                    BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the compressed medial wall coordinate files
   //
   if (readSurfaceFileGroup(specFileIn.compressedSurfaceFile,
                    BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   //
   // Read in the flat coordinate files
   //
   if (readSurfaceFileGroup(specFileIn.flatSurfaceFile,
                    BrainModelSurface::SURFACE_TYPE_FLAT,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the lobar flat coordinate files
   //
   if (readSurfaceFileGroup(specFileIn.lobarFlatSurfaceFile,
                    BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the hull coordinate files
   //
   if (readSurfaceFileGroup(specFileIn.hullSurfaceFile,
                    BrainModelSurface::SURFACE_TYPE_HULL,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the unknown coordinate files
   //
   if (readSurfaceFileGroup(specFileIn.unknownSurfaceFile,
                    BrainModelSurface::SURFACE_TYPE_UNKNOWN,
                    errorMessages,
                    progressFileCounter,
                    progressDialog)) return true;

   //
   // Read in the anatomy volume files
   //
   for (unsigned int i = 0; i < specFileIn.volumeAnatomyFile.files.size(); i++) {
      if (specFileIn.volumeAnatomyFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.volumeAnatomyFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readVolumeFile(specFileIn.volumeAnatomyFile.files[i].filename, VolumeFile::VOLUME_TYPE_ANATOMY, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read in the functional volume files
   //
   for (unsigned int i = 0; i < specFileIn.volumeFunctionalFile.files.size(); i++) {
      if (specFileIn.volumeFunctionalFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.volumeFunctionalFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readVolumeFile(specFileIn.volumeFunctionalFile.files[i].filename, VolumeFile::VOLUME_TYPE_FUNCTIONAL, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read in the paint volume files
   //
   for (unsigned int i = 0; i < specFileIn.volumePaintFile.files.size(); i++) {
      if (specFileIn.volumePaintFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.volumePaintFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readVolumeFile(specFileIn.volumePaintFile.files[i].filename, VolumeFile::VOLUME_TYPE_PAINT, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read in the prob atlas volume files
   //
   for (unsigned int i = 0; i < specFileIn.volumeProbAtlasFile.files.size(); i++) {
      if (specFileIn.volumeProbAtlasFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.volumeProbAtlasFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readVolumeFile(specFileIn.volumeProbAtlasFile.files[i].filename, VolumeFile::VOLUME_TYPE_PROB_ATLAS, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   if (specFileIn.volumeProbAtlasFile.files.size() > 1) {
      synchronizeProbAtlasVolumeRegionNames();
   }
   
   //
   // Read in the rgb paint volume files
   //
   for (unsigned int i = 0; i < specFileIn.volumeRgbFile.files.size(); i++) {
      if (specFileIn.volumeRgbFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.volumeRgbFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readVolumeFile(specFileIn.volumeRgbFile.files[i].filename, VolumeFile::VOLUME_TYPE_RGB, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read in the segmentation volume files
   //
   for (unsigned int i = 0; i < specFileIn.volumeSegmentationFile.files.size(); i++) {
      if (specFileIn.volumeSegmentationFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.volumeSegmentationFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readVolumeFile(specFileIn.volumeSegmentationFile.files[i].filename, VolumeFile::VOLUME_TYPE_SEGMENTATION, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read in the vector volume files
   //
   for (unsigned int i = 0; i < specFileIn.volumeVectorFile.files.size(); i++) {
      if (specFileIn.volumeVectorFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.volumeVectorFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readVolumeFile(specFileIn.volumeVectorFile.files[i].filename, VolumeFile::VOLUME_TYPE_VECTOR, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read in the paint files
   // 
   for (unsigned int i = 0; i < specFileIn.paintFile.files.size(); i++) {
      if (specFileIn.paintFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.paintFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readPaintFile(specFileIn.paintFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read in the probabilistic atlas files
   // 
   for (unsigned int i = 0; i < specFileIn.atlasFile.files.size(); i++) {
      if (specFileIn.atlasFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.atlasFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readProbabilisticAtlasFile(specFileIn.atlasFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read in the area color file
   //
   for (unsigned int i = 0; i < specFileIn.areaColorFile.files.size(); i++) {
      if (specFileIn.areaColorFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.areaColorFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readAreaColorFile(specFileIn.areaColorFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }

   //
   // Read the rgb paint files
   //
   for (unsigned int i = 0; i < specFileIn.rgbPaintFile.files.size(); i++) {
      if (specFileIn.rgbPaintFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.rgbPaintFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readRgbPaintFile(specFileIn.rgbPaintFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }

   //
   // Read the study collection files
   //
   for (unsigned int i = 0; i < specFileIn.studyCollectionFile.files.size(); i++) {
      if (specFileIn.studyCollectionFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.studyCollectionFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readStudyCollectionFile(specFileIn.studyCollectionFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the study meta data files
   //
   for (unsigned int i = 0; i < specFileIn.studyMetaDataFile.files.size(); i++) {
      if (specFileIn.studyMetaDataFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.studyMetaDataFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readStudyMetaDataFile(specFileIn.studyMetaDataFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the vocabulary files
   //
   for (unsigned int i = 0; i < specFileIn.vocabularyFile.files.size(); i++) {
      if (specFileIn.vocabularyFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.vocabularyFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readVocabularyFile(specFileIn.vocabularyFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the wustl region files
   //
   for (unsigned int i = 0; i < specFileIn.wustlRegionFile.files.size(); i++) {
      if (specFileIn.wustlRegionFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.wustlRegionFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readWustlRegionFile(specFileIn.wustlRegionFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the areal estimation files
   //
   for (unsigned int i = 0; i < specFileIn.arealEstimationFile.files.size(); i++) {
      if (specFileIn.arealEstimationFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.arealEstimationFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readArealEstimationFile(specFileIn.arealEstimationFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the topography files
   //
   for (unsigned int i = 0; i < specFileIn.topographyFile.files.size(); i++) {
      if (specFileIn.topographyFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.topographyFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readTopographyFile(specFileIn.topographyFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }

   //
   // Read the geodesic files
   //
   for (unsigned int i = 0; i < specFileIn.geodesicDistanceFile.files.size(); i++) {
      if (specFileIn.geodesicDistanceFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.geodesicDistanceFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readGeodesicDistanceFile(specFileIn.geodesicDistanceFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   
   //
   // Read the lat/lon files
   //
   for (unsigned int i = 0; i < specFileIn.latLonFile.files.size(); i++) {
      if (specFileIn.latLonFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.latLonFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readLatLonFile(specFileIn.latLonFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the params files
   //
   for (unsigned int i = 0; i < specFileIn.paramsFile.files.size(); i++) {
      if (specFileIn.paramsFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.paramsFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readParamsFile(specFileIn.paramsFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the scene files
   //
   for (unsigned int i = 0; i < specFileIn.sceneFile.files.size(); i++) {
      if (specFileIn.sceneFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.sceneFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readSceneFile(specFileIn.sceneFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the section files
   //
   for (unsigned int i = 0; i < specFileIn.sectionFile.files.size(); i++) {
      if (specFileIn.sectionFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.sectionFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readSectionFile(specFileIn.sectionFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the deformation field files
   //
   for (unsigned int i = 0; i < specFileIn.deformationFieldFile.files.size(); i++) {
      if (specFileIn.deformationFieldFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.deformationFieldFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readDeformationFieldFile(specFileIn.deformationFieldFile.files[i].filename, true, true); 
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the volume border files
   //
   for (unsigned int i = 0; i < specFileIn.volumeBorderFile.files.size(); i++) {
      if (specFileIn.volumeBorderFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.volumeBorderFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readVolumeBorderFile(specFileIn.volumeBorderFile.files[i].filename, true, true); 
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   //
   // Read the metric files
   //
   for (unsigned int i = 0; i < specFileIn.metricFile.files.size(); i++) {
      if (specFileIn.metricFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.metricFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readMetricFile(specFileIn.metricFile.files[i].filename, true, true); 
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the image files
   //
   for (unsigned int i = 0; i < specFileIn.imageFile.files.size(); i++) {
      if (specFileIn.imageFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.imageFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readImageFile(specFileIn.imageFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the model files
   //
   for (unsigned int i = 0; i < specFileIn.vtkModelFile.files.size(); i++) {
      if (specFileIn.vtkModelFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.vtkModelFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readVtkModelFile(specFileIn.vtkModelFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the palette file
   //
   bool firstSelectedPaletteFile = true;
   for (unsigned int i = 0; i < specFileIn.paletteFile.files.size(); i++) {
      if (specFileIn.paletteFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.paletteFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            if (firstSelectedPaletteFile) {
               clearPaletteFile();
            }
            firstSelectedPaletteFile = false;
            readPaletteFile(specFileIn.paletteFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }

   switch (specReadMode) {
      case SPEC_FILE_READ_MODE_NORMAL:
         paletteFile->addDefaultPalettes();
         paletteFile->clearModified();
         break;
      case SPEC_FILE_READ_MODE_APPEND:
         break;
   }
   
   //
   // Read the surface shape files
   //
   for (unsigned int i = 0; i < specFileIn.surfaceShapeFile.files.size(); i++) {
      if (specFileIn.surfaceShapeFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.surfaceShapeFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readSurfaceShapeFile(specFileIn.surfaceShapeFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the vector files
   //
   for (unsigned int i = 0; i < specFileIn.vectorFile.files.size(); i++) {
      if (specFileIn.vectorFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.vectorFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readVectorFile(specFileIn.vectorFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read in the border color file
   // 
   for (unsigned int i = 0; i < specFileIn.borderColorFile.files.size(); i++) {
      if (specFileIn.borderColorFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.borderColorFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readBorderColorFile(specFileIn.borderColorFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }

   //
   // Read in the raw borders
   //
   if (readBorderFiles(specFileIn.rawBorderFile,
                       BrainModelSurface::SURFACE_TYPE_RAW,
                       errorMessages,
                       progressFileCounter,
                       progressDialog)) return true;

   //
   // Read in the fiducial borders
   //
   if (readBorderFiles(specFileIn.fiducialBorderFile,
                       BrainModelSurface::SURFACE_TYPE_FIDUCIAL,
                       errorMessages,
                       progressFileCounter,
                       progressDialog)) return true;

   //
   // Read in the inflated borders
   //
   if (readBorderFiles(specFileIn.inflatedBorderFile,
                       BrainModelSurface::SURFACE_TYPE_INFLATED,
                       errorMessages,
                       progressFileCounter,
                       progressDialog)) return true;

   //
   // Read in the very inflated borders
   //
   if (readBorderFiles(specFileIn.veryInflatedBorderFile,
                       BrainModelSurface::SURFACE_TYPE_VERY_INFLATED,
                       errorMessages,
                       progressFileCounter,
                       progressDialog)) return true;

   //
   // Read in the spherical borders
   //
   if (readBorderFiles(specFileIn.sphericalBorderFile,
                       BrainModelSurface::SURFACE_TYPE_SPHERICAL,
                       errorMessages,
                       progressFileCounter,
                       progressDialog)) return true;

   //
   // Read in the ellipsoid borders
   //
   if (readBorderFiles(specFileIn.ellipsoidBorderFile,
                       BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL,
                       errorMessages,
                       progressFileCounter,
                       progressDialog)) return true;

   //
   // Read in the compressed medial wall borders
   //
   if (readBorderFiles(specFileIn.compressedBorderFile,
                       BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL,
                       errorMessages,
                       progressFileCounter,
                       progressDialog)) return true;

   //
   // Read in the flat borders
   //
   if (readBorderFiles(specFileIn.flatBorderFile,
                       BrainModelSurface::SURFACE_TYPE_FLAT,
                       errorMessages,
                       progressFileCounter,
                       progressDialog)) return true;

   //
   // Read in the lobar flat borders
   //
   if (readBorderFiles(specFileIn.lobarFlatBorderFile,
                       BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR,
                       errorMessages,
                       progressFileCounter,
                       progressDialog)) return true;

   //
   // Read in the hull borders
   //
   if (readBorderFiles(specFileIn.hullBorderFile,
                       BrainModelSurface::SURFACE_TYPE_HULL,
                       errorMessages,
                       progressFileCounter,
                       progressDialog)) return true;

   //
   // Read in the unknown borders
   //
   if (readBorderFiles(specFileIn.unknownBorderFile,
                       BrainModelSurface::SURFACE_TYPE_UNKNOWN,
                       errorMessages,
                       progressFileCounter,
                       progressDialog)) return true;

   //
   // Read the border projection files
   //
   for (unsigned int i = 0; i < specFileIn.borderProjectionFile.files.size(); i++) {
      if (specFileIn.borderProjectionFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.borderProjectionFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readBorderProjectionFile(specFileIn.borderProjectionFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read in the cell color file
   // 
   for (unsigned int i = 0; i < specFileIn.cellColorFile.files.size(); i++) {
      if (specFileIn.cellColorFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.cellColorFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readCellColorFile(specFileIn.cellColorFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }

   //
   // Read the fiducial cell file
   //
   for (unsigned int i = 0; i < specFileIn.cellFile.files.size(); i++) {
      if (specFileIn.cellFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.cellFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readCellFile(specFileIn.cellFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the volume cell file
   //
   for (unsigned int i = 0; i < specFileIn.volumeCellFile.files.size(); i++) {
      if (specFileIn.volumeCellFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.volumeCellFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readVolumeCellFile(specFileIn.volumeCellFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the cell projection file
   //
   for (unsigned int i = 0; i < specFileIn.cellProjectionFile.files.size(); i++) {
      if (specFileIn.cellProjectionFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.cellProjectionFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readCellProjectionFile(specFileIn.cellProjectionFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   cellProjectionFile->clearModified();
   
   //
   // Read the cocomac connectivity file
   //
   for (unsigned int i = 0; i < specFileIn.cocomacConnectivityFile.files.size(); i++) {
      if (specFileIn.cocomacConnectivityFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.cocomacConnectivityFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readCocomacConnectivityFile(specFileIn.cocomacConnectivityFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the contour file
   //
   for (unsigned int i = 0; i < specFileIn.contourFile.files.size(); i++) {
      if (specFileIn.contourFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.contourFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readContourFile(specFileIn.contourFile.files[i].filename, true, true);
            BrainModelContours* bmc = getBrainModelContours(-1);
            if (bmc != NULL) {
               ContourFile* cf = bmc->getContourFile();
               cf->clearModified();
            }
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the contour cell file
   //
   for (unsigned int i = 0; i < specFileIn.contourCellFile.files.size(); i++) {
      if (specFileIn.contourCellFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.contourCellFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readContourCellFile(specFileIn.contourCellFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the contour cell color file
   //
   for (unsigned int i = 0; i < specFileIn.contourCellColorFile.files.size(); i++) {
      if (specFileIn.contourCellColorFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.contourCellColorFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readContourCellColorFile(specFileIn.contourCellColorFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the cuts file
   //
   for (unsigned int i = 0; i < specFileIn.cutsFile.files.size(); i++) {
      if (specFileIn.cutsFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.cutsFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readCutsFile(specFileIn.cutsFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read in the foci color file
   // 
   for (unsigned int i = 0; i < specFileIn.fociColorFile.files.size(); i++) {
      if (specFileIn.fociColorFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.fociColorFile.files[i].filename, 
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readFociColorFile(specFileIn.fociColorFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }

   //
   // Read the fiducial foci file
   //
   for (unsigned int i = 0; i < specFileIn.fociFile.files.size(); i++) {
      if (specFileIn.fociFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.fociFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readFociFile(specFileIn.fociFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the foci projection file
   //
   for (unsigned int i = 0; i < specFileIn.fociProjectionFile.files.size(); i++) {
      if (specFileIn.fociProjectionFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.fociProjectionFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readFociProjectionFile(specFileIn.fociProjectionFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   fociProjectionFile->clearModified();
   
   //
   // Read the foci search file
   //
   for (unsigned int i = 0; i < specFileIn.fociSearchFile.files.size(); i++) {
      if (i == 0) {
         //
         // Remove default search
         //
         clearFociSearchFile();
      }
      if (specFileIn.fociSearchFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.fociSearchFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readFociSearchFile(specFileIn.fociSearchFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   fociSearchFile->clearModified();
   
   //
   // Read the transformation matrix file
   //
   for (unsigned int i = 0; i < specFileIn.transformationMatrixFile.files.size(); i++) {
      if (specFileIn.transformationMatrixFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.transformationMatrixFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readTransformationMatrixFile(specFileIn.transformationMatrixFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Read the trajectory files
   //
   for (unsigned int i = 0; i < specFileIn.trajectoryFile.files.size(); i++) {
      if (specFileIn.trajectoryFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.trajectoryFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readTrajectoryFile(specFileIn.trajectoryFile.files[i].filename);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }



   //
   // Read the transformation data file
   //
   for (unsigned int i = 0; i < specFileIn.transformationDataFile.files.size(); i++) {
      if (specFileIn.transformationDataFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.transformationDataFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         try {
            readTransformationDataFile(specFileIn.transformationDataFile.files[i].filename, true, true);
         }
         catch (FileException& e) {
            errorMessages.push_back(e.whatQString());
         }
      }
   }
   
   //
   // Set the deformation map file name to the first one selected
   //
   for (unsigned int i = 0; i < specFileIn.deformationMapFile.files.size(); i++) {
      if (specFileIn.deformationMapFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.deformationMapFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         deformationMapFileName = specFileIn.deformationMapFile.files[i].filename;
         break;
      }
   }
   
   //
   // Set the cerebral hull file name to the first one selected
   //
   for (unsigned int i = 0; i < specFileIn.cerebralHullFile.files.size(); i++) {
      if (specFileIn.cerebralHullFile.files[i].selected) {
         if (updateFileReadProgressDialog(specFileIn.cerebralHullFile.files[i].filename,
                                          progressFileCounter, progressDialog)) {
            return true;
         }
         cerebralHullFileName = specFileIn.cerebralHullFile.files[i].filename;
         break;
      }
   }
   
   if (progressDialog != NULL) {
      if (progressDialog->wasCanceled()) {
         readingSpecFileFlag = false;
         return true;
      }

      progressDialog->setLabelText("Initializing Data");
      progressFileCounter++;
      progressDialog->setValue(progressFileCounter);
      qApp->processEvents();  // note: qApp is global in QApplication
   }
   
   //
   // If no surface shape file was selected
   //
   if (surfaceShapeFile->getNumberOfColumns() == 0) {
      //
      // Compute curvature for a fiducial or raw surface
      //
      if (getNumberOfTopologyFiles() > 0) {
         BrainModelSurface* curvatureSurface = NULL;
         for (int i = 0; i < getNumberOfBrainModels(); i++) {
            BrainModelSurface* bms = getBrainModelSurface(i);
            if (bms != NULL) {
               if (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL) {
                  curvatureSurface = bms;
                  break;
               }
               else if (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_RAW) {
                  curvatureSurface = bms;
               }
            }
         }
         if (curvatureSurface != NULL) {
            BrainModelSurfaceCurvature bmsc(this,
                                            curvatureSurface,
                                            getSurfaceShapeFile(),
                                            BrainModelSurfaceCurvature::CURVATURE_COLUMN_CREATE_NEW,
                                            BrainModelSurfaceCurvature::CURVATURE_COLUMN_DO_NOT_GENERATE,
                                            "Folding (Mean Curvature)",
                                            "");
            try {
               bmsc.execute();
               getSurfaceShapeFile()->clearModified();
            }
            catch (BrainModelAlgorithmException& /*e*/) {
               clearSurfaceShapeFile();
            }
         }
      }
   }
   
   for (int ii = numTopoFilesBeforeLoading; ii < getNumberOfTopologyFiles(); ii++) {
      TopologyFile* tf = getTopologyFile(ii);
      tf->clearModified();
   }

   postSpecFileReadInitializations();

   resetNodeAttributes();
   
   //
   // Emit the signal that this brain set has changed
   //
   emit signalBrainSetChanged();
   
   readingSpecFileFlag = false;
   
   return false;
}

/**
 * Read the BrainSet data files.  Any error message will be placed
 * into "errorMessages".  The size of errorMessages will correspond to
 * the number files that failed to read correctly.  If the size of 
 * errorMessages is 0, no file reading errors were encountered.
 *
 * Returns "true" if the user aborted loading files with the progress dialog.
 */
bool
BrainSet::readSpecFileMultiThreaded(const SPEC_FILE_READ_MODE specReadMode,
                       const SpecFile& specFileIn,
                       const QString& specFileNameIn,
                       std::vector<QString>& errorMessages,
                       const TransformationMatrix* specTransformationMatrixIn,
                       QProgressDialog* progressDialogIn)
{
   QProgressDialog* progressDialog = progressDialogIn;
   specFileTimeOfLoading = QDateTime::currentDateTime();

   readingSpecFileFlag = true;
   
   switch (specReadMode) {
      case SPEC_FILE_READ_MODE_NORMAL:
         //
         // clear out "this" brain set
         //
         reset();
         break;
      case SPEC_FILE_READ_MODE_APPEND:
         break;
   }
   
   if (specTransformationMatrixIn != NULL) {
      specDataFileTransformationMatrix = *specTransformationMatrixIn;
   }
   
   errorMessages.clear();

   //int progressFileCounter = 0;
   
   switch (specReadMode) {
      case SPEC_FILE_READ_MODE_NORMAL:
         //
         // Copy spec file passed and clear all selections in BrainSet::specFile
         //
         loadedFilesSpecFile = specFileIn;
         loadedFilesSpecFile.setAllFileSelections(SpecFile::SPEC_FALSE);
         //loadedFilesSpecFile.setFileName(specFileNameIn);
         loadedFilesSpecFile.setCurrentDirectoryToSpecFileDirectory();
         
         specFileName = specFileNameIn;
         
         structure = specFileIn.getStructure();
/*
         hemisphere = Hemisphere::HEMISPHERE_UNKNOWN;
         if (specFileIn.getStructure() == "right") {
            hemisphere = BrainModelSurface::HEMISPHERE_RIGHT;
         }
         else if (specFileIn.getStructure() == "left") {
            hemisphere = BrainModelSurface::HEMISPHERE_LEFT;
         }
         else if (specFileIn.getStructure() == "both") {
            hemisphere = BrainModelSurface::HEMISPHERE_BOTH;
         }
         setStructure(hemisphere);
*/
         setSpecies(loadedFilesSpecFile.getSpecies());
         setSubject(loadedFilesSpecFile.getSubject());
         stereotaxicSpace = loadedFilesSpecFile.getSpace();
         resetDataFiles(false, false);
         break;
      case SPEC_FILE_READ_MODE_APPEND:
         break;
   }
   
   const int numTopoFilesBeforeLoading = getNumberOfTopologyFiles();
   
   //
   // Create the multi-threaded reader and read files
   //
   const int numThreads = getPreferencesFile()->getNumberOfFileReadingThreads();
   BrainSetMultiThreadedSpecFileReader multiThreadReader(this);
   multiThreadReader.readDataFiles(numThreads,
                                   specFileIn,
                                   progressDialog,
                                   errorMessages);
   
        

   //
   // Add default palettes
   //
   switch (specReadMode) {
      case SPEC_FILE_READ_MODE_NORMAL:
         paletteFile->addDefaultPalettes();
         paletteFile->clearModified();
         break;
      case SPEC_FILE_READ_MODE_APPEND:
         break;
   }
   
   cellProjectionFile->clearModified();
   
   fociProjectionFile->clearModified();
      
   if (progressDialog != NULL) {
      if (progressDialog->wasCanceled()) {
         readingSpecFileFlag = false;
         return true;
      }

      progressDialog->setLabelText("Initializing Data");
      progressDialog->setValue(progressDialog->value() + 1);
      qApp->processEvents();  // note: qApp is global in QApplication
   }
   
   //
   // If no surface shape file was selected
   //
   if (surfaceShapeFile->getNumberOfColumns() == 0) {
      //
      // Compute curvature for a fiducial or raw surface
      //
      if (getNumberOfTopologyFiles() > 0) {
         BrainModelSurface* curvatureSurface = NULL;
         for (int i = 0; i < getNumberOfBrainModels(); i++) {
            BrainModelSurface* bms = getBrainModelSurface(i);
            if (bms != NULL) {
               if (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL) {
                  curvatureSurface = bms;
                  break;
               }
               else if (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_RAW) {
                  curvatureSurface = bms;
               }
            }
         }
         if (curvatureSurface != NULL) {
            BrainModelSurfaceCurvature bmsc(this,
                                            curvatureSurface,
                                            getSurfaceShapeFile(),
                                            BrainModelSurfaceCurvature::CURVATURE_COLUMN_CREATE_NEW,
                                            BrainModelSurfaceCurvature::CURVATURE_COLUMN_DO_NOT_GENERATE,
                                            "Folding (Mean Curvature)",
                                            "");
            try {
               bmsc.execute();
               getSurfaceShapeFile()->clearModified();
            }
            catch (BrainModelAlgorithmException& /*e*/) {
               clearSurfaceShapeFile();
            }
         }
      }
   }
   
   for (int ii = numTopoFilesBeforeLoading; ii < getNumberOfTopologyFiles(); ii++) {
      TopologyFile* tf = getTopologyFile(ii);
      tf->clearModified();
   }

   postSpecFileReadInitializations();

   resetNodeAttributes();
   
   //
   // Emit the signal that this brain set has changed
   //
   emit signalBrainSetChanged();
   
   readingSpecFileFlag = false;
   
   return false;
}

/**
 * sort the brain models (raw, fiducial, ..., volume, surf&vol, contours).
 */
void 
BrainSet::sortBrainModels()
{
   const int numModels = static_cast<int>(brainModels.size());
   if (numModels <= 0) {
      return;
   }
   
   //
   // Assign priority to the models
   //
   const int surfacePriority = 0;
   const int volumePriority  = 20;
   const int surfaceAndVolumePriority = 21;
   const int contourPriority = 22;
   const int noPriority      = 23;
   std::vector<int> modelPriority(numModels, noPriority);
   for (int i = 0; i < numModels; i++) {
      int priority = noPriority;
      if (getBrainModelSurfaceAndVolume(i) != NULL) {
         priority = surfaceAndVolumePriority;
      }
      else if (getBrainModelSurface(i) != NULL) {
         const BrainModelSurface* bms = getBrainModelSurface(i);
         priority = surfacePriority
                    + static_cast<int>(bms->getSurfaceType());
      }
      else if (getBrainModelVolume(i) != NULL) {
         priority = volumePriority;
      }
      else if (getBrainModelContours(i) != NULL) {
         priority = contourPriority;
      }
      
      modelPriority[i] = priority;
   }
   
   //
   // Replace the models in sorted order
   //
   std::vector<BrainModel*> sortedModels;
   for (int ip = 0; ip <= noPriority; ip++) {
      for (int j = 0; j < numModels; j++) {
         if (modelPriority[j] == ip) {
            sortedModels.push_back(brainModels[j]);
         }
      }
   }
   if (brainModels.size() == sortedModels.size()) {
      brainModels = sortedModels;
   }
   else {
      std::cout << "INFO: Sorting of brain models failed." << std::endl;
   }
}
      
/**
 * Update all display settings.
 */
void
BrainSet::updateAllDisplaySettings()
{
   brainSetAutoLoaderManager->update();
   displaySettingsArealEstimation->update();  
   displaySettingsBorders->update();
   displaySettingsCells->update();
   displaySettingsCoCoMac->update();
   displaySettingsContours->update();
   displaySettingsCuts->update();
   displaySettingsFoci->update();
   displaySettingsDeformationField->update();
   displaySettingsMetric->update(); 
   displaySettingsPaint->update();
   displaySettingsProbabilisticAtlasSurface->update();
   displaySettingsRgbPaint->update();  
   displaySettingsScene->update();
   displaySettingsSection->update();
   displaySettingsStudyMetaData->update();
   displaySettingsSurfaceShape->update();
   displaySettingsVectors->update();  
   displaySettingsTopography->update();
   displaySettingsVolume->update();
   displaySettingsProbabilisticAtlasVolume->update();
   displaySettingsWustlRegion->update();
}

/**
 * Initalize things after reading a spec file or initial surface into caret
 */
void
BrainSet::postSpecFileReadInitializations()
{
   getSectionsFromTopology();
   
   assignBorderColors();
   
   assignCellColors();
   
   assignContourCellColors();

   assignFociColors();
   
   updateAllDisplaySettings();

   clearNodeAttributes();

   brainModelBorderSet->setAllBordersModifiedStatus(false);
   brainModelBorderSet->setProjectionsModified(false);

   if (sectionFile->getNumberOfColumns() == 0) {
      BrainModelSurface* bms = getActiveFiducialSurface();
      if (bms != NULL) {
         BrainModelSurfaceResection bmsr(this,
                                         bms,
                                         bms->getRotationTransformMatrix(0),
                                         BrainModelSurfaceResection::SECTION_AXIS_Y,
                                         BrainModelSurfaceResection::SECTION_TYPE_THICKNESS,
                                         sectionFile,
                                         -1,
                                         "Default Sections 1mm Y-axis",
                                         1.0,
                                         100);
         try {
            bmsr.execute();
         }
         catch (BrainModelAlgorithmException) {
         }
         sectionFile->clearModified();
      }
   }
   
   updateNodeDisplayFlags();
   
   //   
   // Default to surface shape if no overlays/underlays selected
   //
   if (getSurfaceUnderlay()->getOverlay(-1) ==  
                      BrainModelSurfaceOverlay::OVERLAY_NONE) {  
      if (getSurfaceShapeFile()->getNumberOfColumns() > 0) {
         getSurfaceUnderlay()->setOverlay(-1,
                  BrainModelSurfaceOverlay::OVERLAY_SURFACE_SHAPE);
      }
   }
   nodeColoring->assignColors();
   
   //
   // Create default transformation matrices
   //
   ParamsFile* paramsFile = getParamsFile();
   float acX, acY, acZ;
   paramsFile->getParameter(ParamsFile::keyACx, acX);
   paramsFile->getParameter(ParamsFile::keyACy, acY);
   paramsFile->getParameter(ParamsFile::keyACz, acZ);
   if ((acX != 0.0) || (acY != 0.0) || (acZ != 0.0)) {
      TransformationMatrixFile* tmf = getTransformationMatrixFile();
      
      //
      // Create matrix to transform from Native to AC-centered space.
      //
      QString name("Native To AC");
      TransformationMatrix* tm = tmf->getTransformationMatrixWithName(name);
      if (tm == NULL) {
         TransformationMatrix nac;
         nac.setMatrixName(name);
         nac.setMatrixComment("Convert from Native to AC-centered space.");
         nac.translate(-acX, -acY, -acZ);
         tmf->addTransformationMatrix(nac);
      }
      
      //
      // Create matrix to transform from Native to AC-centered space.
      //
      name = "AC To Native";
      tm = tmf->getTransformationMatrixWithName(name);
      if (tm == NULL) {
         TransformationMatrix acn;
         acn.setMatrixName(name);
         acn.setMatrixComment("Convert from AC-centered to Native space.");
         acn.translate(acX, acY, acZ);
         tmf->addTransformationMatrix(acn);
      }
      
      tmf->clearModified();
   }
   
   paramsFile->getParameter(ParamsFile::keyWholeVolumeACx, acX);
   paramsFile->getParameter(ParamsFile::keyWholeVolumeACy, acY);
   paramsFile->getParameter(ParamsFile::keyWholeVolumeACz, acZ);
   if ((acX != 0.0) || (acY != 0.0) || (acZ != 0.0)) {
      TransformationMatrixFile* tmf = getTransformationMatrixFile();
      
      //
      // Create matrix to transform from Native to whole volume AC-centered space.
      //
      QString name("Native To Whole Volume AC");
      TransformationMatrix* tm = tmf->getTransformationMatrixWithName(name);
      if (tm == NULL) {
         TransformationMatrix nac;
         nac.setMatrixName(name);
         nac.setMatrixComment("Convert from Native to Whole Volume AC-centered space.");
         nac.translate(-acX, -acY, -acZ);
         tmf->addTransformationMatrix(nac);
      }
      
      //
      // Create matrix to transform from Native to whole volume AC-centered space.
      //
      name = "Whole Volume AC To Native";
      tm = tmf->getTransformationMatrixWithName(name);
      if (tm == NULL) {
         TransformationMatrix acn;
         acn.setMatrixName(name);
         acn.setMatrixComment("Convert from Whole Volume AC-centered to Native space.");
         acn.translate(acX, acY, acZ);
         tmf->addTransformationMatrix(acn);
      }
      
      tmf->clearModified();
   }
   
   updateDefaultFileNamePrefix();
}

/**
 * delete a brain model surface.
 */
void 
BrainSet::deleteBrainModelSurface(BrainModelSurface* bms)
{
   const QString name(bms->getCoordinateFile()->getFileName());
   loadedFilesSpecFile.rawCoordFile.clearSelectionStatus(name);
   loadedFilesSpecFile.fiducialCoordFile.clearSelectionStatus(name);
   loadedFilesSpecFile.inflatedCoordFile.clearSelectionStatus(name);
   loadedFilesSpecFile.veryInflatedCoordFile.clearSelectionStatus(name);
   loadedFilesSpecFile.sphericalCoordFile.clearSelectionStatus(name);
   loadedFilesSpecFile.ellipsoidCoordFile.clearSelectionStatus(name);
   loadedFilesSpecFile.compressedCoordFile.clearSelectionStatus(name);
   loadedFilesSpecFile.flatCoordFile.clearSelectionStatus(name);
   loadedFilesSpecFile.lobarFlatCoordFile.clearSelectionStatus(name);
   loadedFilesSpecFile.unknownCoordFile.clearSelectionStatus(name);
   deleteBrainModel(bms);
   clearAllDisplayLists();
   updateAllDisplaySettings();   
   nodeColoring->assignColors();
}
      
/**
 * delete all brain model surfaces.
 */
void 
BrainSet::deleteAllBrainModelSurfaces()
{
   std::vector<BrainModel*> modelsToDelete;
   
   const int num = static_cast<int>(brainModels.size());
   for (int i = (num - 1); i >= 0; i--) {
      switch (brainModels[i]->getModelType()) {
         case BrainModel::BRAIN_MODEL_SURFACE:
            {
               BrainModelSurface* bms = getBrainModelSurface(i);
               if (bms != NULL) {
                  deleteBrainModelSurface(bms);
               }
            }
            break;
         case BrainModel::BRAIN_MODEL_SURFACE_AND_VOLUME:
            deleteBrainModel(brainModels[i]);
            break;
         case BrainModel::BRAIN_MODEL_VOLUME:
            break;
         case BrainModel::BRAIN_MODEL_CONTOURS:
            break;
      }
   }
   
   deleteAllTopologyFiles();
   resetNodeAttributeFiles();
   clearAllDisplayLists();
}      

/**
 * Get the "active" fiducial surface.
 */
void
BrainSet::setActiveFiducialSurface(BrainModelSurface* bms)
{
   activeFiducialSurface = bms;
   BrainModelSurfaceAndVolume* bmsv = getBrainModelSurfaceAndVolume();
   if (bmsv != NULL) {
      bmsv->setSurface();
   }
   for (int i = 0; i < getNumberOfVolumeFunctionalFiles(); i++) {
      getVolumeFunctionalFile(i)->setVoxelToSurfaceDistancesValid(false);
   }
}

/**
 * Get the "active" fiducial surface.
 */
BrainModelSurface*
BrainSet::getActiveFiducialSurface()
{
   // 
   // Make sure fiducial surface is still valid
   //
   bool activeFiducialSurfaceFound = false;
   if (activeFiducialSurface != NULL) {
      const int numBrains = getNumberOfBrainModels();
      for (int i = 0; i < numBrains; i++) {
         BrainModelSurface* bms = getBrainModelSurface(i);
         if (bms != NULL) {
            if (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL) {
               if (bms == activeFiducialSurface) {
                  activeFiducialSurfaceFound = true;
               }
            }
         }
      }
   }
   
   //
   // Find the fiducial surface to make the active surface
   //
   if (activeFiducialSurfaceFound == false) {
      activeFiducialSurface = NULL;
      const int numBrains = getNumberOfBrainModels();
      for (int i = 0; i < numBrains; i++) {
         BrainModelSurface* bms = getBrainModelSurface(i);
         if (bms != NULL) {
            if (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL) {
               activeFiducialSurface = bms;
            }
         }
      }
   }
   
   return activeFiducialSurface;
}

/**
 * get the left fiducial volume interaction surface.
 */
BrainModelSurface* 
BrainSet::getLeftFiducialVolumeInteractionSurface()
{
   //
   // Does the surface still exist
   //
   int indx = getBrainModelIndex(leftFiducialVolumeInteractionSurface);
   if (indx >= 0) {
      BrainModelSurface* bms = getBrainModelSurface(indx);
      if (bms->getStructure().getType() != Structure::STRUCTURE_TYPE_CORTEX_LEFT) {
         indx = -1;
      }
   }
   if (indx < 0) {
      //
      // Find the first left fiducial surface
      //
      leftFiducialVolumeInteractionSurface = NULL;
      const int numBrains = getNumberOfBrainModels();
      for (int i = 0; i < numBrains; i++) {
         BrainModelSurface* bms = getBrainModelSurface(i);
         if (bms != NULL) {
            if (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL) {
               if (bms->getStructure().getType() == Structure::STRUCTURE_TYPE_CORTEX_LEFT) {
                  leftFiducialVolumeInteractionSurface = bms;
               }
            }
         }
      }
   }
   
   return leftFiducialVolumeInteractionSurface;
}

/**
 * get the right fiducial volume interaction surface.
 */
BrainModelSurface* 
BrainSet::getRightFiducialVolumeInteractionSurface()
{
   //
   // Does the surface still exist
   //
   int indx = getBrainModelIndex(rightFiducialVolumeInteractionSurface);
   if (indx >= 0) {
      BrainModelSurface* bms = getBrainModelSurface(indx);
      if (bms->getStructure().getType() != Structure::STRUCTURE_TYPE_CORTEX_RIGHT) {
         indx = -1;
      }
   }
   if (indx < 0) {
      //
      // Find the first right fiducial surface
      //
      rightFiducialVolumeInteractionSurface = NULL;
      const int numBrains = getNumberOfBrainModels();
      for (int i = 0; i < numBrains; i++) {
         BrainModelSurface* bms = getBrainModelSurface(i);
         if (bms != NULL) {
            if (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL) {
               if (bms->getStructure().getType() == Structure::STRUCTURE_TYPE_CORTEX_RIGHT) {
                  rightFiducialVolumeInteractionSurface = bms;
               }
            }
         }
      }
   }
   
   return rightFiducialVolumeInteractionSurface;
}

/**
 * get the cerebellum fiducial volume interaction surface.
 */
BrainModelSurface* 
BrainSet::getCerebellumFiducialVolumeInteractionSurface()
{
   //
   // Does the surface still exist
   //
   int indx = getBrainModelIndex(cerebellumFiducialVolumeInteractionSurface);
   if (indx >= 0) {
      BrainModelSurface* bms = getBrainModelSurface(indx);
      if (bms->getStructure().getType() != Structure::STRUCTURE_TYPE_CEREBELLUM) {
         indx = -1;
      }
   }
   if (indx < 0) {
      //
      // Find the first cerebellum fiducial surface
      //
      cerebellumFiducialVolumeInteractionSurface = NULL;
      const int numBrains = getNumberOfBrainModels();
      for (int i = 0; i < numBrains; i++) {
         BrainModelSurface* bms = getBrainModelSurface(i);
         if (bms != NULL) {
            if (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL) {
               if (bms->getStructure().getType() == Structure::STRUCTURE_TYPE_CEREBELLUM) {
                  cerebellumFiducialVolumeInteractionSurface = bms;
               }
            }
         }
      }
   }
   
   return cerebellumFiducialVolumeInteractionSurface;
}

/**
 * set the left fiducial volume interaction surface.
 */
void 
BrainSet::setLeftFiducialVolumeInteractionSurface(BrainModelSurface* bms)
{
   leftFiducialVolumeInteractionSurface = bms;
}

/**
 * set the right fiducial volume interaction surface.
 */
void 
BrainSet::setRightFiducialVolumeInteractionSurface(BrainModelSurface* bms)
{
   rightFiducialVolumeInteractionSurface = bms;
}

/**
 * set the cerebellum fiducial volume interaction surface.
 */
void 
BrainSet::setCerebellumFiducialVolumeInteractionSurface(BrainModelSurface* bms)
{
   cerebellumFiducialVolumeInteractionSurface = bms;
}      

/** 
 * Get a brain model surface of the specified type.
 */
BrainModelSurface* 
BrainSet::getBrainModelSurfaceOfType(const BrainModelSurface::SURFACE_TYPES st)
{
   const int ns = getNumberOfBrainModels();
   for (int i = (ns - 1); i >= 0; i--) {
      BrainModel* bm = getBrainModel(i);
      if (bm->getModelType() == BrainModel::BRAIN_MODEL_SURFACE) {
         BrainModelSurface* s = dynamic_cast<BrainModelSurface*>(bm);
         if (s->getSurfaceType() == st) {
            return s;
         }
      }
   }
   
   return NULL;
}

/**
 * Get a brain model surface with the specified coordinate file name (NULL if not found).
 */
BrainModelSurface* 
BrainSet::getBrainModelSurfaceWithCoordinateFileName(const QString& fileNameIn)
{
   const QString fileName(FileUtilities::basename(fileNameIn));
   
   const int ns = getNumberOfBrainModels();
   for (int i = (ns - 1); i >= 0; i--) {
      BrainModelSurface* bms = getBrainModelSurface(i);
      if (bms != NULL) {
         const CoordinateFile* cf = bms->getCoordinateFile();
         if (cf != NULL) {
            const QString coordName = FileUtilities::basename(cf->getFileName());
            if (fileName == coordName) {
               return bms;
            }
         }
      }
   }
   return NULL;
}

/**
 * generate the cerebral hull vtk file
 */
void 
BrainSet::generateCerebralHullVtkFile(const VolumeFile* segmentationVolume,
                                      const bool saveHullVolumeFileFlag)
                                             throw (BrainModelAlgorithmException)
{
   cerebralHullFileName = "";
   
   //
   // Create the cerebral hull volume
   //
   VolumeFile* hullVolume = new VolumeFile;
   segmentationVolume->createCerebralHullVolume(*hullVolume);

/*   
   QString name(hullVolume->getFileName());
   const QString afniExt("+orig" + SpecFile::getAfniVolumeFileExtension());
   if (name.endsWith(afniExt) == false) {
      if (name.endsWith(SpecFile::getAfniVolumeFileExtension())) {
         const int numChars = name.length() - SpecFile::getAfniVolumeFileExtension().length();
         name = name.left(numChars);
      }
      name.append(afniExt);
   }
   hullVolume->setFileName(name);
*/
      
   //
   // Add the cerbral hull file to the brain set
   //
   addVolumeFile(VolumeFile::VOLUME_TYPE_SEGMENTATION,
                 hullVolume,
                 hullVolume->getFileName(),
                 true,
                 true);
                 
   if(saveHullVolumeFileFlag) {
      //
      // Save file
      //
      try {
         writeVolumeFile(hullVolume->getFileName(),
                         hullVolume->getFileWriteType(),
                         VolumeFile::VOLUME_TYPE_SEGMENTATION,
                         hullVolume);
      }
      catch (FileException& e) {
         QString msg("Error saving cerebral hull volume: \n");
         msg.append(e.whatQString());
         throw BrainModelAlgorithmException(msg);
      }
   }
                        
   //
   // Create a new brain set
   //
   BrainSet bs;
   
   //
   // Generate a surface from the cerebral hull volume
   //
   BrainModelVolumeToSurfaceConverter bmvsc(&bs,
                                            hullVolume,
                     BrainModelVolumeToSurfaceConverter::RECONSTRUCTION_MODE_SUREFIT_SURFACE,
                                            true,
                                            false);
   try {
      bmvsc.execute();
   }
   catch (BrainModelAlgorithmException& e) {
      QString msg("ERROR creating cerebral hull VTK file: \n");
      msg.append(e.whatQString());
      throw BrainModelAlgorithmException(msg);
   }
   
   //
   //  Find the fiducial surface
   //
   BrainModelSurface* bms = bs.getBrainModelSurfaceOfType(BrainModelSurface::SURFACE_TYPE_FIDUCIAL);
   if (bms == NULL) {
      throw BrainModelAlgorithmException("Unable to find surface generated from cerebral hull volume.");
   }
   
   //
   // Name the VTK file
   //
   QString prefix;
   int numNodes;
   AbstractFile::getDefaultFileNamePrefix(prefix, numNodes);
   QString vtkName(prefix);
   if (prefix.isEmpty() == false) {
      vtkName.append(".");
   }
   vtkName.append("CerebralHull.vtk");
   
   //
   // Export hull surface to VTK surface file.
   //
   try {
      bs.exportVtkSurfaceFile(bms, vtkName, false);
   }
   catch (FileException& e) {
      QString msg("Error saving cerebral hull as VTK file.");
      msg.append(e.whatQString());
      throw BrainModelAlgorithmException(e.whatQString());
   }
   
   //
   // Update spec file 
   //
   addToSpecFile(SpecFile::getCerebralHullFileTag(), vtkName);

   //
   // Set name of cerebral null
   //
   cerebralHullFileName = vtkName;
}      

/**
 * generate the cerebral hull vtk file
 * caller must delete the output files (hull volume and VTK file)
 */
void 
BrainSet::generateCerebralHullVtkFile(const VolumeFile* segmentationVolumeIn,
                                      VolumeFile* &cerebralHullVolumeOut,
                                      vtkPolyData* &cerebralHullVtkPolyDataOut)
                                             throw (BrainModelAlgorithmException)
{
   //
   // Create the cerebral hull volume
   //
   cerebralHullVolumeOut = new VolumeFile;
   segmentationVolumeIn->createCerebralHullVolume(*cerebralHullVolumeOut);

   //
   // Create a new brain set
   //
   BrainSet bs;
   
   //
   // Generate a surface from the cerebral hull volume
   //
   BrainModelVolumeToSurfaceConverter bmvsc(&bs,
                                            cerebralHullVolumeOut,
                     BrainModelVolumeToSurfaceConverter::RECONSTRUCTION_MODE_SUREFIT_SURFACE,
                                            true,
                                            false);
   try {
      bmvsc.execute();
   }
   catch (BrainModelAlgorithmException& e) {
      QString msg("ERROR creating cerebral hull VTK file: \n");
      msg.append(e.whatQString());
      throw BrainModelAlgorithmException(msg);
   }
   
   //
   //  Find the fiducial surface
   //
   BrainModelSurface* bms = bs.getBrainModelSurfaceOfType(BrainModelSurface::SURFACE_TYPE_FIDUCIAL);
   if (bms == NULL) {
      throw BrainModelAlgorithmException("Unable to find surface generated from cerebral hull volume.");
   }
   
   //
   // Get VTK poly data from surface
   //
   cerebralHullVtkPolyDataOut = bms->convertToVtkPolyData();
}      

/**
 * If no sections files, search through topology files for sections.
 */
void
BrainSet::getSectionsFromTopology()
{
   if (sectionFile->getNumberOfColumns() == 0) {
      
      const int numNodes = getNumberOfNodes();
      if (numNodes <= 0) {
         return;
      }
      const int numFiles = getNumberOfTopologyFiles();
      for (int i = 0; i < numFiles; i++) {
         TopologyFile* tf = getTopologyFile(i);
         std::vector<int> sectionInfo;
         if (tf->getNodeSections(sectionInfo)) {
            int column = 0;
            if (sectionFile->getNumberOfNodes() == 0) {
               sectionFile->setNumberOfNodesAndColumns(numNodes, 1);
               column = 0;
            }
            else {
               column = sectionFile->getNumberOfColumns();
               sectionFile->addColumns(1);
            }
            
            QString str("From topology file: ");
            str.append(tf->getFileName());
            sectionFile->setColumnComment(column, str);
            sectionFile->setColumnName(column, FileUtilities::basename(tf->getFileName()));
            
            const int num = std::min(static_cast<int>(sectionInfo.size()), numNodes);
            for (int j = 0; j < num; j++) {
               sectionFile->setSection(j, column, sectionInfo[j]);
            }
         }
/*
         if (tf->getNumberOfTopologyNodes() == numNodes) {
            int column = 0;
            if (sectionFile->getNumberOfNodes() == 0) {
               sectionFile->setNumberOfNodesAndColumns(numNodes, 1);
               column = 0;
            }
            else {
               column = sectionFile->getNumberOfColumns();
               sectionFile->addColumns(1);
            }
            
            QString str("From topology file: ");
            str.append(tf->getFileName());
            sectionFile->setColumnComment(column, str);
            sectionFile->setColumnName(column, FileUtilities::basename(tf->getFileName()));
            
            for (int j = 0; j < numNodes; j++) {
               const TopologyNode* tn = tf->getTopologyNode(j);
               sectionFile->setSection(j, column, tn->getSectionNumber());
            }
            sectionFile->setMaximumSelectedSection(column, 
                                sectionFile->getMaximumSection(column));
            sectionFile->setMinimumSelectedSection(column, 
                                sectionFile->getMinimumSection(column));
         }
*/
      }
      sectionFile->clearModified();
   }
}

/**
 * Returns the number of nodes in this surface
 */ 
int
BrainSet::getNumberOfNodes() const
{
   const int numModels = getNumberOfBrainModels();
   for (int i = 0; i < numModels; i++) {
       const BrainModelSurface* bms = getBrainModelSurface(i);
       if (bms != NULL) {
         return bms->getNumberOfNodes();
      }
   }
   return 0;
}

/**
 * Set the default scaling for all surfaces
 */
void
BrainSet::setDefaultScaling(const float orthoRight, const float orthoTop)
{
   const int ns = getNumberOfBrainModels();
   for (int i = 0; i < ns; i++) {
      BrainModel* bm = getBrainModel(i);
      switch (bm->getModelType()) {
         case BrainModel::BRAIN_MODEL_CONTOURS:
            break;
         case BrainModel::BRAIN_MODEL_SURFACE:
            {
               BrainModelSurface* s = dynamic_cast<BrainModelSurface*>(bm);
               s->setDefaultScaling(orthoRight, orthoTop);
               break;
            }
         case BrainModel::BRAIN_MODEL_SURFACE_AND_VOLUME:
            {
               BrainModelSurface* s = dynamic_cast<BrainModelSurface*>(bm);
               s->setDefaultScaling(orthoRight, orthoTop);
               break;
            }
         case BrainModel::BRAIN_MODEL_VOLUME:
            break;
      }
   }
}

/**
 * Assign border colors to the border projection and border files
 */
void
BrainSet::assignBorderColors()
{
   brainModelBorderSet->assignColors();
}

/**
 * Assign cell colors to cell and cell projection files
 */
void
BrainSet::assignCellColors()
{
   //
   // Cell Projection File
   //
   cellProjectionFile->assignColors(*cellColorFile,
                                    displaySettingsCells->getColorMode());
   
   //
   // volume cell file
   //
   volumeCellFile->assignColors(*cellColorFile,
                                displaySettingsCells->getColorMode());
   
   //
   // Transform data files may contain cell files
   //
   assignTransformationDataFileColors();
}
     
/**
 * Assign contour cell colors to contour cells
 */
void
BrainSet::assignContourCellColors()
{
   //
   // Contour Cell File
   //
   contourCellFile->assignColors(*contourCellColorFile,
                                 CellBase::CELL_COLOR_MODE_NAME);
}

/**
 * Assign foci colors to foci and foci projection files
 */
void
BrainSet::assignFociColors()
{
   //
   // Foci Projection File
   //
   fociProjectionFile->assignColors(*fociColorFile,
                                    displaySettingsFoci->getColorMode());
   
   //
   // Transform data files may contain cell files
   //
   assignTransformationDataFileColors();
}

/**
 * delete a transformation data file.
 */
void 
BrainSet::deleteTransformationDataFile(const int fileIndex)
{
   if ((fileIndex >= 0) &&
       (fileIndex < getNumberOfTransformationDataFiles())) {
      delete transformationDataFiles[fileIndex];
      transformationDataFiles.erase(transformationDataFiles.begin() + fileIndex);
   }
}

/**
 * delete a transformation data file.
 */
void 
BrainSet::deleteTransformationDataFile(AbstractFile* af)
{
   for (int i = 0; i < getNumberOfTransformationDataFiles(); i++) {
      if (getTransformationDataFile(i) == af) {
         deleteTransformationDataFile(i);
         break;
      }
   }
}
      
/**
 * get have transformation data cell files.
 */
bool 
BrainSet::getHaveTransformationDataCellFiles() const
{
   for (int i = 0; i < getNumberOfTransformationDataFiles(); i++) {
      if (dynamic_cast<CellFile*>(transformationDataFiles[i]) != NULL) {
         if (dynamic_cast<FociFile*>(transformationDataFiles[i]) == NULL) {
            if (dynamic_cast<ContourCellFile*>(transformationDataFiles[i]) == NULL) {
               return true;
            }
         }
      }
   }
   return false;
}

/**
 * get have transformation data contour files.
 */
bool 
BrainSet::getHaveTransformationDataContourFiles() const
{
   for (int i = 0; i < getNumberOfTransformationDataFiles(); i++) {
      if (dynamic_cast<ContourFile*>(transformationDataFiles[i]) != NULL) {
         return true;
      }
   }
   return false;
}

/**
 * get have transformation data contour cell files.
 */
bool 
BrainSet::getHaveTransformationDataContourCellFiles() const
{
   for (int i = 0; i < getNumberOfTransformationDataFiles(); i++) {
      if (dynamic_cast<ContourCellFile*>(transformationDataFiles[i]) != NULL) {
         return true;
      }
   }
   return false;
}

/**
 * get have transformation data foci files.
 */
bool 
BrainSet::getHaveTransformationDataFociFiles() const
{
   for (int i = 0; i < getNumberOfTransformationDataFiles(); i++) {
      if (dynamic_cast<FociFile*>(transformationDataFiles[i]) != NULL) {
         return true;
      }
   }
   return false;
}

/**
 * get have transformation data vtk files.
 */
bool 
BrainSet::getHaveTransformationDataVtkFiles() const
{
   for (int i = 0; i < getNumberOfTransformationDataFiles(); i++) {
      if (dynamic_cast<VtkModelFile*>(transformationDataFiles[i]) != NULL) {
         return true;
      }
   }
   return false;
}

/**
 * assign transformation data file colors.
 */
void 
BrainSet::assignTransformationDataFileColors()
{
   const int num = getNumberOfTransformationDataFiles();
   for (int i = 0; i < num; i++) {
      AbstractFile* af = getTransformationDataFile(i);
      
      FociFile* ff = dynamic_cast<FociFile*>(af);
      if (ff != NULL) {
         ff->assignColors(*fociColorFile,
                          CellBase::CELL_COLOR_MODE_NAME);
      }
      
      CellFile* cf = dynamic_cast<CellFile*>(af);
      if ((ff == NULL) && (cf != NULL)) {
         cf->assignColors(*cellColorFile,
                          CellBase::CELL_COLOR_MODE_NAME);
      }
   }
}

/**
 * remove unlinked studies from study meta data file.
 * Returns number of studies that are removed.
 */
int 
BrainSet::removeUnlinkedStudiesFromStudyMetaDataFile()
{
   //
   // Used to track linked studies
   //
   const int numStudies = studyMetaDataFile->getNumberOfStudyMetaData();
   if (numStudies <= 0) {
      return 0;
   }

   //
   // Get PubMed IDs of various data types
   //
   std::set<QString> pmidSet;
   std::vector<QString> pmids;
   
   arealEstimationFile->getPubMedIDsOfAllLinkedStudyMetaData(pmids);
   pmidSet.insert(pmids.begin(), pmids.end());
   
   cellProjectionFile->getPubMedIDsOfAllLinkedStudyMetaData(pmids);
   pmidSet.insert(pmids.begin(), pmids.end());
   
   deformationFieldFile->getPubMedIDsOfAllLinkedStudyMetaData(pmids);
   pmidSet.insert(pmids.begin(), pmids.end());
   
   fociProjectionFile->getPubMedIDsOfAllLinkedStudyMetaData(pmids);
   pmidSet.insert(pmids.begin(), pmids.end());
   
   geodesicDistanceFile->getPubMedIDsOfAllLinkedStudyMetaData(pmids);
   pmidSet.insert(pmids.begin(), pmids.end());
   
   latLonFile->getPubMedIDsOfAllLinkedStudyMetaData(pmids);
   pmidSet.insert(pmids.begin(), pmids.end());
   
   paintFile->getPubMedIDsOfAllLinkedStudyMetaData(pmids);
   pmidSet.insert(pmids.begin(), pmids.end());
   
   rgbPaintFile->getPubMedIDsOfAllLinkedStudyMetaData(pmids);
   pmidSet.insert(pmids.begin(), pmids.end());
   
   surfaceShapeFile->getPubMedIDsOfAllLinkedStudyMetaData(pmids);
   pmidSet.insert(pmids.begin(), pmids.end());

   for (int i = 0; i < getNumberOfVectorFiles(); i++) {
      getVectorFile(i)->getPubMedIDsOfAllLinkedStudyMetaData(pmids);
      pmidSet.insert(pmids.begin(), pmids.end());
   }
   
   topographyFile->getPubMedIDsOfAllLinkedStudyMetaData(pmids);
   pmidSet.insert(pmids.begin(), pmids.end());
   
   vocabularyFile->getPubMedIDsOfAllLinkedStudyMetaData(pmids);
   pmidSet.insert(pmids.begin(), pmids.end());
   
   //
   // Identify linked studies
   //
   std::vector<bool> studyUsageFlags(numStudies, 0);
   for (std::set<QString>::iterator iter = pmidSet.begin();
        iter != pmidSet.end();
        iter++) {
      const QString pmid(*iter);
      if (pmid.isEmpty() == false) {
         const int studyNumber = studyMetaDataFile->getStudyIndexFromPubMedID(pmid);
         if (studyNumber >= 0) {
            studyUsageFlags[studyNumber] = true;
         }
      }
   }
   
   //
   // Remove unused studies
   //
   int numUnusedStudies = 0;
   for (int i = (numStudies - 1); i >= 0; i--) {
      if (studyUsageFlags[i] == false) {
         studyMetaDataFile->deleteStudyMetaData(i);
         numUnusedStudies++;
      }
   }
   
   return numUnusedStudies;
}
      
/**
 * move foci study info to the study meta data file.
 */
void 
BrainSet::moveFociStudyInfoToStudyMetaDataFile()
{
   StudyMetaDataFile* smdf = getStudyMetaDataFile();
   FociProjectionFile* fpf = getFociProjectionFile();
   smdf->append(*fpf);
   displaySettingsStudyMetaData->update();
   displaySettingsFoci->update();
}
      
/**
 * Delete all foci projections (including those in foci files).
 */
void
BrainSet::deleteAllFociProjections()
{
   fociProjectionFile->clear();
   loadedFilesSpecFile.fociProjectionFile.setAllSelections(SpecFile::SPEC_FALSE);
   loadedFilesSpecFile.fociFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * Delete a focus.
 */
void
BrainSet::deleteFocus(const int focusNumber)
{
   fociProjectionFile->deleteCellProjection(focusNumber);
}

/**
 * Delete cell and cell projections.
 */
void
BrainSet::deleteAllCells(const bool deleteCellProjections,
                         const bool deleteVolumeCells)
{
   if (deleteCellProjections) {
      deleteAllCellProjections();
   }
   
   if (deleteVolumeCells) {
      volumeCellFile->clear();
      loadedFilesSpecFile.volumeCellFile.setAllSelections(SpecFile::SPEC_FALSE);
   }
}

/**
 * Delete all cell projections (including those in cell files).
 */
void
BrainSet::deleteAllCellProjections()
{
   cellProjectionFile->clear();
   loadedFilesSpecFile.cellProjectionFile.setAllSelections(SpecFile::SPEC_FALSE);
   loadedFilesSpecFile.cellFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * Delete a cell.
 */
void
BrainSet::deleteCell(const int cellNumber)
{
   cellProjectionFile->deleteCellProjection(cellNumber);
}

/**
 * delete all image files.
 */
void 
BrainSet::deleteAllImageFiles()
{
   for (int i = 0; i < getNumberOfImageFiles(); i++) {
      delete imageFiles[i];
   }
   imageFiles.clear();
   loadedFilesSpecFile.imageFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * delete an image file.
 */
void 
BrainSet::deleteImageFile(ImageFile* img)
{
   std::vector<ImageFile*> files;
   
   for (int i = 0; i < getNumberOfImageFiles(); i++) {
      if (imageFiles[i] == img) {
         loadedFilesSpecFile.imageFile.clearSelectionStatus(img->getFileName());
         delete img;
      }
      else {
         files.push_back(imageFiles[i]);
      }
   }
   imageFiles = files;   
}

/**
 * add an image file.
 */
void 
BrainSet::addImageFile(ImageFile* img)
{
   imageFiles.push_back(img);
   displaySettingsImages->update();
}

/**
 * see if an image file is valid.
 */
bool 
BrainSet::getImageFileValid(const ImageFile* img) const
{
   const int num = getNumberOfImageFiles();
   for (int i = 0; i < num; i++) {
      if (imageFiles[i] == img) {
         return true;
      }
   }
   return false;
}
      
/**
 * get an image file.
 */
ImageFile* 
BrainSet::getImageFile(const int fileNum)
{
   if ((fileNum >= 0) && (fileNum < getNumberOfImageFiles())) {
      return imageFiles[fileNum];
   }
   return NULL;
}
      
/**
 * get an image file based upon the image's file name.
 */
ImageFile* 
BrainSet::getImageFile(const QString& filename)
{
   const int num = getNumberOfImageFiles();
   for (int i = 0; i < num; i++) {
      ImageFile* img = getImageFile(i);
      if (img != NULL) {
         if (FileUtilities::basename(filename) == FileUtilities::basename(img->getFileName())) {
            return img;
         }
      }
   }

   return NULL;
}
      
/**
 * read the image data file.
 */
void 
BrainSet::readImageFile(const QString& name, const bool append,
                      const bool updateSpec) throw (FileException)
{
   ImageFile* img = new ImageFile;
   try {
      img->readFile(name);
   }
   catch (FileException& e) {
      delete img;
      throw(e);
   }
   
   QMutexLocker locker(&mutexImageFile);
   
   if (append == false) {
      deleteAllImageFiles();
   }
   
   imageFiles.push_back(img);
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getImageFileTag(), name);
   }
   displaySettingsImages->update();
}

/**
 * write the image data file.
 */
void 
BrainSet::writeImageFile(const QString& name,
                       ImageFile* img) throw (FileException)
{
   loadedFilesSpecFile.imageFile.clearSelectionStatus(img->getFileName());
   img->writeFile(name);
   addToSpecFile(SpecFile::getImageFileTag(), name);
   displaySettingsImages->update();
}

/**
 * get a vtk model file.
 */
VtkModelFile* 
BrainSet::getVtkModelFile(const int modelNum)
{
   if ((modelNum >= 0) && (modelNum < getNumberOfVtkModelFiles())) {
      return vtkModelFiles[modelNum];
   }
   return NULL;
}

/**
 * Delete all VTK model files.
 */
void
BrainSet::deleteAllVtkModelFiles()
{
   for (int i = 0; i < getNumberOfVtkModelFiles(); i++) {
      delete vtkModelFiles[i];
   }
   vtkModelFiles.clear();
   loadedFilesSpecFile.vtkModelFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * add a VTK model file.
 */
void 
BrainSet::addVtkModelFile(VtkModelFile* vmf)
{
   vtkModelFiles.push_back(vmf);
   displaySettingsModels->update();
}
      
/**
 * delete a VTK model file.
 */
void 
BrainSet::deleteVtkModelFile(VtkModelFile* vmf)
{
   std::vector<VtkModelFile*> files;
   
   for (int i = 0; i < getNumberOfVtkModelFiles(); i++) {
      if (vtkModelFiles[i] == vmf) {
         loadedFilesSpecFile.vtkModelFile.clearSelectionStatus(vmf->getFileName());
         delete vmf;
      }
      else {
         files.push_back(vtkModelFiles[i]);
      }
   }
   vtkModelFiles = files;   
}

/**
 * read the vtk model data file.
 */
void 
BrainSet::readVtkModelFile(const QString& name, const bool append,
                        const bool updateSpec) throw (FileException)
{
   
   VtkModelFile* vmf = new VtkModelFile();
   try {
      vmf->readFile(name);
   }
   catch (FileException& e) {
      delete vmf;
      throw(e);
   }
   
   QMutexLocker locker(&mutexVtkModelFile);
   
   if (append == false) {
      deleteAllVtkModelFiles();
   }
   
   vtkModelFiles.push_back(vmf);
   
   if (updateSpec) {
      addToSpecFile(SpecFile::getVtkModelFileTag(), name);
   }
   displaySettingsModels->update();
}

/**
 * write the vtk model data file.
 */
void 
BrainSet::writeVtkModelFile(const QString& name,
                            VtkModelFile* vmf) throw (FileException)
{
   loadedFilesSpecFile.vtkModelFile.clearSelectionStatus(vmf->getFileName());
   vmf->writeFile(name);
   addToSpecFile(SpecFile::getVtkModelFileTag(), name);
}      

/**
 * Get the common node attributes for a node.
 */
BrainSetNodeAttribute* 
BrainSet::getNodeAttributes(const int nodeNum) 
{
   if (nodeNum >= static_cast<int>(nodeAttributes.size())) {
      nodeAttributes.resize(nodeNum + 1);
   }
   return &nodeAttributes[nodeNum];
}

/**
 * reset node attributes
 */
void
BrainSet::resetNodeAttributes()
{
   if (getNumberOfNodes() > static_cast<int>(nodeAttributes.size())) {
      nodeAttributes.resize(getNumberOfNodes());
   }
   brainModelSurfaceRegionOfInterestNodeSelection->update();
}

/**
 * clear the node attributes
 */
void
BrainSet::clearNodeAttributes()
{
   resetNodeAttributes();
   const int numNodes = getNumberOfNodes();
   for (int i = 0; i < numNodes; i++) {
      nodeAttributes[i].reset();
   }
}

/// find a node with the specified morph row and column (returns -1 if not found)
int 
BrainSet::getNodeWithMorphRowColumn(const int row, const int column,
                                    const int startAtNode) const
{
   const int numNodes = static_cast<int>(nodeAttributes.size());
   for (int i = startAtNode; i < numNodes; i++) {
      if ((nodeAttributes[i].morphRow == row) &&
          (nodeAttributes[i].morphColumn == column)) {
         return i;
      }
   }
   return -1;
}

/**
 * export byu surface file.
 */
void 
BrainSet::exportByuSurfaceFile(const BrainModelSurface* bms,
                               const QString& filename) throw (FileException)
{
   if (bms != NULL) {
      vtkPolyData* polyData = bms->convertToVtkPolyData();
      if (polyData != NULL) {
         vtkBYUWriter *writer = vtkBYUWriter::New();
#ifdef HAVE_VTK6
         writer->SetInputData(polyData);
#else
         writer->SetInput(polyData);
         writer->SetHeader("Written by Caret");
#endif
         writer->SetGeometryFileName((char*)filename.toAscii().constData());
         writer->Write();
      
         writer->Delete();      
         polyData->Delete();
      }
   }
}

/**
 * export inventor surface file.
 */
void 
BrainSet::exportInventorSurfaceFile(const BrainModelSurface* bms,
                                    const QString& filename) throw (FileException)
{
   if (bms != NULL) {
      vtkPolyData* polyData = bms->convertToVtkPolyData();
      if (polyData != NULL) {
         const int numNodes = getNumberOfNodes();
         vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
         colors->SetNumberOfComponents(3);
         colors->SetNumberOfTuples(numNodes);
         BrainModelSurfaceNodeColoring* bsnc = getNodeColoring();
         const int model = bms->getBrainModelIndex();
         for (int i = 0; i < numNodes; i++) {
            const unsigned char* charRgb = bsnc->getNodeColor(model, i);
            float rgb[3];
            rgb[0] = charRgb[0];
            rgb[1] = charRgb[1];
            rgb[2] = charRgb[2];
            colors->InsertTuple(i, rgb);
         }
         polyData->GetPointData()->SetScalars(colors);
         colors->Delete();
      
         vtkIVWriter *writer = vtkIVWriter::New();
#ifdef HAVE_VTK6
         writer->SetInputData(polyData);
#else
         writer->SetInput(polyData);
         writer->SetHeader("Written by Caret");
#endif
         writer->SetFileName((char*)filename.toAscii().constData());
         writer->Write();
      
         writer->Delete();      
         polyData->Delete();
      }
   }
}

/**
 * export stl surface file.
 */
void 
BrainSet::exportStlSurfaceFile(const BrainModelSurface* bms,
                               const QString& filename) throw (FileException)
{
   if (bms != NULL) {
      vtkPolyData* polyData = bms->convertToVtkPolyData();
      if (polyData != NULL) {
         vtkSTLWriter *writer = vtkSTLWriter::New();
#ifdef HAVE_VTK6
         writer->SetInputData(polyData);
#else
         writer->SetInput(polyData);
         writer->SetHeader("Written by Caret");
#endif
         writer->SetFileName((char*)filename.toAscii().constData());
         writer->Write();
      
         writer->Delete();      
         polyData->Delete();
      }
   }
}      

/**
 * export wavefront object surface file.
 */
void 
BrainSet::exportWavefrontSurfaceFile(const BrainModelSurface* /*bms*/,
                                     const QString& /*filename*/) throw (FileException)
{
   throw FileException("Export to Wavefront not supported.");
   
/*  RENABLING WILL CAUSE LINK ERRORS!!! to Xt libs and others

   const QString fileExt = FileUtilities::filenameExtension(filename);
   QString filePrefix = filename;
   if (fileExt == ".obj") {
      filePrefix = FileUtilities::filenameWithoutExtension(filename);
   }
   
   if (bms != NULL) {
      vtkPolyData* polyData = convertToVtkPolyData(bms, true); 
      if (polyData != NULL) {
      
         vtkPolyDataMapper* polyDataMapper = vtkPolyDataMapper::New();
         polyDataMapper->SetInput(polyData);
         polyDataMapper->SetColorModeToDefault();
         polyDataMapper->SetScalarModeToUsePointData();
         
         vtkActor* actor = vtkActor::New();
         actor->SetMapper(polyDataMapper);
         
         vtkRenderer* renderer = vtkRenderer::New();
         renderer->AddActor(actor);
         renderer->SetBackground(0, 0, 0);
         
         vtkRenderWindow* window = vtkRenderWindow::New();
         window->AddRenderer(renderer);
         window->SetSize(500, 500);
         
         vtkOBJExporter* obj = vtkOBJExporter::New();
         obj->SetRenderWindow(window);
         obj->SetFilePrefix(filePrefix.toAscii().constData());
         obj->Write();
         
         obj->Delete();
         window->Delete();
         renderer->Delete();
         actor->Delete();
         polyDataMapper->Delete();
         polyData->Delete();
      }
   }
*/
}
      
/**
 * export 3dStudio ASE surface file.
 */
void 
BrainSet::export3DStudioASESurfaceFile(const BrainModelSurface* bms,
                                       const QString& filename) throw (FileException)
{
   if (bms != NULL) {
      QFile file(filename);
      if (file.open(QIODevice::WriteOnly)) {
         QTextStream stream(&file);
         stream.setRealNumberNotation(QTextStream::FixedNotation);
         stream.setRealNumberPrecision(6);
         
         stream << "*3DSMAX_ASCIIEXPORT 200\n";
         stream << "*COMMENT \"cube.ase, created by IVCON.\"\n";
         stream << "*COMMENT \"Original data in cube.iv\"\n";
         stream << "*SCENE {\n";
         stream << "  *SCENE_FILENAME \"\"\n";
         stream << "  *SCENE_FIRSTFRAME 0\n";
         stream << "  *SCENE_LASTFRAME 100\n";
         stream << "  *SCENE_FRAMESPEED 30\n";
         stream << "  *SCENE_TICKSPERFRAME 160\n";
         stream << "  *SCENE_BACKGROUND_STATIC 0.0000 0.0000 0.0000\n";
         stream << "  *SCENE_AMBIENT_STATIC 0.0431 0.0431 0.0431\n";
         stream << "}\n";
         stream << "\n";
                  
         stream << "*MATERIAL_LIST {\n";
         stream << "   *MATERIAL_COUNT 1\n";
         stream << "   *MATERIAL 0 {\n";
         stream << "      *MATERIAL_NAME \"surface\"\n";
         stream << "      *MATERIAL_AMBIENT 0.7 0.7 0.7  \n";
         stream << "      *MATERIAL_DIFFUSE 0.8 0.8 0.8  \n";
         stream << "      *MATERIAL_SHADING phong\n";
         stream << "      *MATERIAL_SHINE 0.3\n";
         stream << "   }\n";
         stream << "}\n";
         stream << "\n";
         
         stream << "*GEOMOBJECT {\n";
         stream << "  *NODE_NAME \"Object01\"\n";
         stream << "  *NODE_TM {\n";
         stream << "    *NODE_NAME \"Object01\"\n";
         stream << "    *INHERIT_POS 0 0 0\n";
         stream << "    *INHERIT_ROT 0 0 0\n";
         stream << "    *INHERIT_SCL 0 0 0\n";
         stream << "    *TM_ROW0 1.0000 0.0000 0.0000\n";
         stream << "    *TM_ROW1 0.0000 1.0000 0.0000\n";
         stream << "    *TM_ROW2 0.0000 0.0000 1.0000\n";
         stream << "    *TM_ROW3 0.0000 0.0000 0.0000\n";
         stream << "    *TM_POS 0.0000 0.0000 0.0000\n";
         stream << "    *TM_ROTAXIS 0.0000 0.0000 0.0000\n";
         stream << "    *TM_ROTANGLE 0.0000\n";
         stream << "    *TM_SCALE 1.0000 1.0000 1.0000\n";
         stream << "    *TM_SCALEAXIS 0.0000 0.0000 0.0000\n";
         stream << "    *TM_SCALEAXISANG 0.0000\n";
         stream << "  }\n";
         stream << "\n";
  
         const CoordinateFile* cf = bms->getCoordinateFile();
         const int numNodes = cf->getNumberOfCoordinates();
         const TopologyFile* tf = bms->getTopologyFile();
         const int numTiles = tf->getNumberOfTiles();
         stream << "  *MESH {\n";
         stream << "    *TIMEVALUE 0\n";
         stream << "    *MESH_NUMVERTEX " << numNodes << "\n";
         stream << "    *MESH_NUMFACES " << numTiles << "\n";
         stream << "    *MESH_VERTEX_LIST {\n";
         for (int i = 0; i < numNodes; i++) {
            const float* xyz = cf->getCoordinate(i);
            stream << "      *MESH_VERTEX " << i << " "
                   << xyz[0] << " " << xyz[1] << " " << xyz[2] << "\n";
         }
         stream << "    }\n";
         stream << "\n";
         
         stream << "   *MESH_FACE_LIST {\n";
         for (int i = 0; i < numTiles; i++) {
            const int* tileNodes = tf->getTile(i);
            stream << "      *MESH_FACE " << i << ": "
                   << "A: " << tileNodes[0] << " "
                   << "B: " << tileNodes[1] << " "
                   << "C: " << tileNodes[2] << " "
                   << "AB: 1 BC: 1 CA: 1 *MESH_SMOOTHING 1 *MESH_MTLID 0"
                   << "\n";
         }
         stream << "   }\n";
         stream << "\n";

         stream << "   *MESH_NUMTVERTEX 0 \n";
         stream << "   *MESH_NUMCVERTEX " << numNodes << " \n";
         stream << "   *MESH_CVERTLIST {" << "\n";
         BrainModelSurfaceNodeColoring* bsnc = getNodeColoring();
         const int modelIndex = getBrainModelIndex(bms);
         for (int i = 0; i < numNodes; i++) {
            const unsigned char* color = bsnc->getNodeColor(modelIndex, i);
            const float rgb[3] = {
               color[0] / 255.0,
               color[1] / 255.0,
               color[2] / 255.0
            };
            stream << "      *MESH_VERTCOL " << i << " "
                   << rgb[0] << " " << rgb[1] << " " << rgb[2] << "\n";
         }
         stream << "   }\n";
         stream << "   *MESH_NORMALS {\n";
         for (int i = 0; i < numNodes; i++) {
            const float* normal = bms->getNormal(i);
            stream << "      *MESH_VERTEXNORMAL " << i << " "
                   << normal[0] << " " << normal[1] << " " << normal[2] << "\n";
         }
         stream << "   }\n";   
         stream << "\n";
   /*
      *MESH_FACENORMAL 6 0.000000 0.000000 1.000000
      *MESH_VERTEXNORMAL 0 0.000000 0.000000 1.000000
      *MESH_VERTEXNORMAL 1 0.000000 0.000000 1.000000
      *MESH_VERTEXNORMAL 2 0.000000 0.000000 1.000000
      *MESH_VERTEXNORMAL 3 0.000000 0.000000 1.000000
   */
        stream << "   *PROP_MOTIONBLUR 0\n";
        stream << "   *PROP_CASTSHADOW 1\n";
        stream << "   *PROP_RECVSHADOW 1\n";
        stream << "  }\n";
        stream << "}\n";
        stream << "\n";
      }
      else {
         throw FileException(filename, "Unable to open for writing");
      }
   }
}

/**
 * export vrml surface file.
 */
void 
BrainSet::exportVrmlSurfaceFile(const BrainModelSurface* bms,
                                const QString& filename) throw (FileException)
{
/*
   if (bms != NULL) {
      vtkPolyData* polyData = convertToVtkPolyData(bms, true); 
      if (polyData != NULL) {
      
         vtkPolyDataMapper* polyDataMapper = vtkPolyDataMapper::New();
         polyDataMapper->SetInput(polyData);
         polyDataMapper->SetColorModeToDefault();
         polyDataMapper->SetScalarModeToUsePointData();
         
         vtkActor* actor = vtkActor::New();
         actor->SetMapper(polyDataMapper);
         
         vtkRenderer* renderer = vtkRenderer::New();
         renderer->AddActor(actor);
         renderer->SetBackground(0, 0, 0);
         
         vtkRenderWindow* window = vtkRenderWindow::New();
         window->AddRenderer(renderer);
         window->SetSize(500, 500);
         
         vtkVRMLExporter* vrml = vtkVRMLExporter::New();
         vrml->SetRenderWindow(window);
         vrml->SetFileName(filename.toAscii().constData());
         vrml->Write();
         
         vrml->Delete();
         window->Delete();
         renderer->Delete();
         actor->Delete();
         polyDataMapper->Delete();
         polyData->Delete();
      }
   }
   
*/
   if (bms == NULL) {
      return;
   }

   QFile file(filename);
   if (file.open(QIODevice::WriteOnly)) {
      QTextStream stream(&file);
      stream.setRealNumberNotation(QTextStream::FixedNotation);
      stream.setRealNumberPrecision(6);
      
      stream << "#VRML V2.0 utf8\n";
      stream << "\n";
   
      stream << "WorldInfo {\n";
      stream << "   title \"title\"\n";
      stream << "   info \"Written by Caret\"\n";
      stream << "}\n";
      stream << "\n";
      
      stream << "NavigationInfo {\n";
      stream << "   headlight TRUE\n";
      stream << "   speed     20.0\n";
      stream << "   type      [\"EXAMINE\", \"WALK\", \"FLY\",\"ANY\"]\n";
      stream << "}\n\n";
      
      //if (lightOn) {
         const float lightPos[3] = { 0.0, 0.0, 1000.0 };
         stream << "PointLight {\n";
         stream << "   ambientIntensity 1.0\n";
         stream << "   attenuation      1.0 1.0 1.0\n";
         stream << "   color            1.0 1.0 1.0\n";
         stream << "   intensity        1.0\n";
         stream << "   location         " 
                << lightPos[0] << " "
                << lightPos[1] << " "
                << lightPos[2] << "\n";
         stream << "   on               TRUE\n";
         stream << "   radius           10000.0\n";
         stream << "}\n";
      //}
      
      stream << "Shape {\n";

      stream << "   appearance Appearance {\n";
      stream << "      material Material {\n";
      stream << "        ambientIntensity 0\n";
      stream << "        diffuseColor 1 1 1\n";
      stream << "        specularColor 0 0 0\n";
      stream << "        shininess 0.0078125\n";
      stream << "        transparency 0\n";
      stream << "      }\n";
      stream << "   }\n";
      stream << "\n";
      
      stream << "   geometry IndexedFaceSet {\n";
      stream << "      solid FALSE\n";
      
      //
      // nodes
      //
      const CoordinateFile* cf = bms->getCoordinateFile();
      const int numCoords = cf->getNumberOfCoordinates();
      stream << "      coord Coordinate {\n";
      stream << "         point [\n";
      for (int i = 0; i < numCoords; i++) {
         const float* xyz = cf->getCoordinate(i);
         stream << "            " << xyz[0] << " " << xyz[1] << " " << xyz[2];
         if (i < (numCoords - 1)) {
            stream << ",";
         }
         stream << "\n";
      }
      stream << "         ]\n";
      stream << "      }\n";  // coordCoordinate
      stream << "\n";
   
      //
      // Tiles
      //
      const TopologyFile* tf = bms->getTopologyFile();
      const int numTiles = tf->getNumberOfTiles();
      stream << "      coordIndex [\n";
      for (int i = 0; i < numTiles; i++) {
         const int* t = tf->getTile(i);
         stream << "         " << t[0] << " " << t[1] << " " << t[2] << " -1";
         if (i < (numTiles - 1)) {
            stream << ",";
         }
         stream << "\n";
      }
      stream << "      ]\n"; // coordIndex
      stream << "\n";
      
      //
      // Node Normals
      //
      stream << "      normal Normal {\n";
      stream << "         vector [\n";
      for (int i = 0; i < numCoords; i++) {
         const float* n = bms->getNormal(i);
         stream << "            "
                << n[0] << " " << n[1] << " " << n[2];
         if (i < (numCoords - 1)) {
            stream << ",";
         }
         stream << "\n";
      }
      stream << "         ]\n";
      stream << "      }\n";  // normal Normal
      stream << "\n";
      
      //
      // Node Colors
      //
      stream << "      color Color {\n";
      stream << "         color [\n";
      BrainModelSurfaceNodeColoring* bsnc = getNodeColoring();
      const int model = bms->getBrainModelIndex();
      for (int i = 0; i < numCoords; i++) {
         const unsigned char* charRgb = bsnc->getNodeColor(model, i);
         float rgb[3];
         rgb[0] = charRgb[0];
         rgb[1] = charRgb[1];
         rgb[2] = charRgb[2];
         stream << "            " 
                 << rgb[0]/255.0 << " " 
                 << rgb[1]/255.0 << " " 
                 << rgb[2]/255.0;
         if (i < (numCoords - 1)) {
            stream << ",";
         }
         stream << "\n";
      }
      stream << "         ]\n";
      stream << "      }\n"; // color Color
      stream << "\n";
   
      stream << "   }\n"; // geometry IndexedFaceSet
      stream << "}\n"; // Shape

      file.close();
   }
   else {
      throw FileException(filename, "Unable to open for writing");
   }
}      

/**
 * export vtk surface file
 */
void 
BrainSet::exportVtkSurfaceFile(const BrainModelSurface* bms,
                               const QString& filename,
                               const bool exportColors) throw (FileException)
{
   if (bms != NULL) {
      vtkPolyData* polyData = convertToVtkPolyData(bms, exportColors);
      if (polyData != NULL) {      
         vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
#ifdef HAVE_VTK6
         writer->SetInputData(polyData);
#else
         writer->SetInput(polyData);
         writer->SetHeader("Written by Caret");
#endif
         writer->SetFileName((char*)filename.toAscii().constData());
         writer->Write();
      
         writer->Delete();      
         polyData->Delete();
      }
   }
}      

/**
 * export vtk XML surface file
 */
void 
BrainSet::exportVtkXmlSurfaceFile(const BrainModelSurface* bms,
                               const QString& filename,
                               const bool exportColors) throw (FileException)
{
   if (bms != NULL) {
      vtkPolyData* polyData = convertToVtkPolyData(bms, exportColors);
      if (polyData != NULL) {      
         vtkXMLPolyDataWriter *writer = vtkXMLPolyDataWriter::New();
#ifdef HAVE_VTK6
         writer->SetInputData(polyData);
#else
         writer->SetInput(polyData);
#endif
         writer->SetDataModeToAscii();
         writer->SetFileName((char*)filename.toAscii().constData());
         writer->Write();
      
         writer->Delete();      
         polyData->Delete();
      }
   }
}      

/**
 * Convert a BrainModelSurface to a vtkPolyData file with option of node colors
 */
vtkPolyData* 
BrainSet::convertToVtkPolyData(const BrainModelSurface* bms,
                               const bool useNodeColors) 
{
   vtkPolyData* polyData = NULL;
   if (bms != NULL) {
      polyData = bms->convertToVtkPolyData();
      if (polyData != NULL) {
         if (useNodeColors) {
            vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
            colors->SetNumberOfComponents(3);
            const int numNodes = getNumberOfNodes();
            colors->SetNumberOfTuples(numNodes);
            BrainModelSurfaceNodeColoring* bsnc = getNodeColoring();
            const int model = bms->getBrainModelIndex();
            for (int i = 0; i < numNodes; i++) {
               const unsigned char* charRgb = bsnc->getNodeColor(model, i);
               float rgb[3];
               rgb[0] = charRgb[0];
               rgb[1] = charRgb[1];
               rgb[2] = charRgb[2];
               colors->InsertTuple(i, rgb);
            }
            polyData->GetPointData()->SetScalars(colors);
            colors->Delete();
         }      
      }
   }
   return polyData;
}      

/**
 * Import Raw volume file.
 */
void 
BrainSet::importRawVolumeFile(const QString& filename,
                              const VolumeFile::VOLUME_TYPE volumeType,
                              const int dimensions[3],
                              const VolumeFile::VOXEL_DATA_TYPE voxelDataType,
                              const bool byteSwap) throw (FileException)
{
   VolumeFile* vf = new VolumeFile;
   
   try {
      switch (volumeType) {
         case VolumeFile::VOLUME_TYPE_FUNCTIONAL:
            break;
         case VolumeFile::VOLUME_TYPE_PAINT:
            break;
         case VolumeFile::VOLUME_TYPE_PROB_ATLAS:
            break;
         case VolumeFile::VOLUME_TYPE_RGB:
            break;
         case VolumeFile::VOLUME_TYPE_ROI:
            throw FileException(FileUtilities::basename(filename), "ROI type not supported.");
            break;
         case VolumeFile::VOLUME_TYPE_SEGMENTATION:
            break;
         case VolumeFile::VOLUME_TYPE_ANATOMY:
            break;
         case VolumeFile::VOLUME_TYPE_VECTOR:
            break;
         case VolumeFile::VOLUME_TYPE_UNKNOWN:
            throw FileException(FileUtilities::basename(filename), "Unrecognized volume type");
            break;
      }
      
      const VolumeFile::ORIENTATION orient[3] = {
         VolumeFile::ORIENTATION_UNKNOWN,
         VolumeFile::ORIENTATION_UNKNOWN,
         VolumeFile::ORIENTATION_UNKNOWN
      };
      const float origin[3] = { 0.0, 0.0, 0.0 };
      const float space[3]  = { 1.0, 1.0, 1.0 };
      vf->readFileVolumeRaw(filename,
                            0,
                            voxelDataType,
                            dimensions,
                            orient,
                            origin,
                            space,
                            byteSwap);
      addVolumeFile(volumeType, vf, filename, true, false);
   }
   catch (FileException& e) {
     delete vf;
     throw e;
   }
   
}

/**
 * Import MINC volume file.
 */
void 
BrainSet::importMincVolumeFile(const QString& filename,
                               const VolumeFile::VOLUME_TYPE volumeType) throw (FileException)
{
   VolumeFile* vf = new VolumeFile;
   
   try {
      switch (volumeType) {
         case VolumeFile::VOLUME_TYPE_FUNCTIONAL:
            break;
         case VolumeFile::VOLUME_TYPE_PAINT:
            break;
         case VolumeFile::VOLUME_TYPE_PROB_ATLAS:
            break;
         case VolumeFile::VOLUME_TYPE_RGB:
            break;
         case VolumeFile::VOLUME_TYPE_ROI:
            throw FileException(FileUtilities::basename(filename), "ROI volume type not supported.");
            break;
         case VolumeFile::VOLUME_TYPE_SEGMENTATION:
            break;
         case VolumeFile::VOLUME_TYPE_ANATOMY:
            break;
         case VolumeFile::VOLUME_TYPE_VECTOR:
            break;
         case VolumeFile::VOLUME_TYPE_UNKNOWN:
            throw FileException(FileUtilities::basename(filename), "Unrecognized volume type");
            break;
      }
      
      vf->importMincVolume(filename);
      addVolumeFile(volumeType, vf, filename, true, false);
   }
   catch (FileException& e) {
     delete vf;
     throw e;
   }
   
}

/**
 * Import Analyze volume file.
 */
void 
BrainSet::importAnalyzeVolumeFile(const QString& filename,
                                  const VolumeFile::VOLUME_TYPE volumeType) throw (FileException)
{
   VolumeFile* vf = new VolumeFile;
   
   try {
      switch (volumeType) {
         case VolumeFile::VOLUME_TYPE_FUNCTIONAL:
            break;
         case VolumeFile::VOLUME_TYPE_PAINT:
            break;
         case VolumeFile::VOLUME_TYPE_PROB_ATLAS:
            break;
         case VolumeFile::VOLUME_TYPE_RGB:
            break;
         case VolumeFile::VOLUME_TYPE_ROI:
            throw FileException(FileUtilities::basename(filename), "ROI volume type not supported.");
            break;
         case VolumeFile::VOLUME_TYPE_SEGMENTATION:
            break;
         case VolumeFile::VOLUME_TYPE_ANATOMY:
            break;
         case VolumeFile::VOLUME_TYPE_VECTOR:
            break;
         case VolumeFile::VOLUME_TYPE_UNKNOWN:
            throw FileException(FileUtilities::basename(filename), "Unrecognized volume type");
            break;
      }
      
      vf->importAnalyzeVolume(filename);
      addVolumeFile(volumeType, vf, filename, true, false);
   }
   catch (FileException& e) {
     delete vf;
     throw e;
   }
   
}

/**
 * Import VTK Structured Points volume file.
 */
void 
BrainSet::importVtkStructuredPointsVolumeFile(const QString& filename,
                               const VolumeFile::VOLUME_TYPE volumeType) throw (FileException)
{
   VolumeFile* vf = new VolumeFile;
   
   try {
      switch (volumeType) {
         case VolumeFile::VOLUME_TYPE_FUNCTIONAL:
            break;
         case VolumeFile::VOLUME_TYPE_PAINT:
            break;
         case VolumeFile::VOLUME_TYPE_PROB_ATLAS:
            break;
         case VolumeFile::VOLUME_TYPE_RGB:
            break;
         case VolumeFile::VOLUME_TYPE_ROI:
            throw FileException(FileUtilities::basename(filename), "ROI volume type not supported.");
            break;
         case VolumeFile::VOLUME_TYPE_SEGMENTATION:
            break;
         case VolumeFile::VOLUME_TYPE_ANATOMY:
            break;
         case VolumeFile::VOLUME_TYPE_VECTOR:
            break;
         case VolumeFile::VOLUME_TYPE_UNKNOWN:
            throw FileException(FileUtilities::basename(filename), "Unrecognized volume type");
            break;
      }
      
      vf->importVtkStructuredPointsVolume(filename);
      addVolumeFile(volumeType, vf, filename, true, false);
   }
   catch (FileException& e) {
     delete vf;
     throw e;
   }   
}

/**
 * Export VTK Structured Points volume file.
 */
void 
BrainSet::exportVtkStructuredPointsVolumeFile(VolumeFile* vf,
                                       const QString& filename) throw (FileException)
{
   vf->exportVtkStructuredPointsVolume(filename);
}

/**
 * Export Analyze volume file.
 */
void 
BrainSet::exportAnalyzeVolumeFile(VolumeFile* vf,
                                  const QString& filename) throw (FileException)
{
   vf->exportAnalyzeVolume(filename);
}

/**
 * Export Minc volume file.
 */
void 
BrainSet::exportMincVolumeFile(VolumeFile* vf,
                                  const QString& filename) throw (FileException)
{
   vf->exportMincVolume(filename);
}

/**
 * import byu surface file.
 */
void 
BrainSet::importByuSurfaceFile(const QString& filename,
                               const bool importCoordinates,
                               const bool importTopology,
                               const BrainModelSurface::SURFACE_TYPES surfaceType,
                               const TopologyFile::TOPOLOGY_TYPES topologyType) throw (FileException)
{
   vtkBYUReader* reader = vtkBYUReader::New();
   reader->SetGeometryFileName((char*)filename.toAscii().constData());
   reader->ReadDisplacementOff(); 
   reader->ReadScalarOff();
   reader->ReadTextureOff();
   reader->Update();
   vtkPolyData* polyData = reader->GetOutput();
   
   try {
      importVtkTypeFileHelper(filename, polyData, importCoordinates, importTopology, false,
                              surfaceType, topologyType);
   }
   catch (FileException& e) {
      reader->Delete();
      throw e;
   }
   
   // don't do this polyData->Delete();
   reader->Delete();
}

/**
 * import stl surface file.
 */
void 
BrainSet::importStlSurfaceFile(const QString& filename,
                               const bool importCoordinates,
                               const bool importTopology,
                               const BrainModelSurface::SURFACE_TYPES surfaceType,
                               const TopologyFile::TOPOLOGY_TYPES topologyType) throw (FileException)
{
   vtkSTLReader* reader = vtkSTLReader::New();
   reader->SetFileName((char*)filename.toAscii().constData());
   reader->Update();
   vtkPolyData* polyData = reader->GetOutput();
   
   try {
      importVtkTypeFileHelper(filename, polyData, importCoordinates, importTopology, false,
                              surfaceType, topologyType);
   }
   catch (FileException& e) {
      reader->Delete();
      throw e;
   }
   
   // don't do this polyData->Delete();
   reader->Delete();
}      

/**
 * import vtk file
 */
void
BrainSet::importVtkSurfaceFile(const QString& filename,
                               const bool importCoordinates,
                               const bool importTopology,
                               const bool importColors,
                               const BrainModelSurface::SURFACE_TYPES surfaceType,
                               const TopologyFile::TOPOLOGY_TYPES topologyType) throw (FileException)
{
   vtkPolyDataReader* reader = vtkPolyDataReader::New();
   reader->SetFileName((char*)filename.toAscii().constData());
   reader->Update();
   vtkPolyData* polyData = reader->GetOutput();
   
   try {
      importVtkTypeFileHelper(filename, polyData, importCoordinates, importTopology, importColors,
                              surfaceType, topologyType);
   }
   catch (FileException& e) {
      reader->Delete();
      throw e;
   }
   
   // don't do this polyData->Delete();
   reader->Delete();
}

/**
 * import vtk XML file
 */
void
BrainSet::importVtkXmlSurfaceFile(const QString& filename,
                               const bool importCoordinates,
                               const bool importTopology,
                               const bool importColors,
                               const BrainModelSurface::SURFACE_TYPES surfaceType,
                               const TopologyFile::TOPOLOGY_TYPES topologyType) throw (FileException)
{
   vtkXMLPolyDataReader* reader = vtkXMLPolyDataReader::New();
   reader->SetFileName((char*)filename.toAscii().constData());
   reader->Update();
   vtkPolyData* polyData = reader->GetOutput();
   
   try {
      importVtkTypeFileHelper(filename, polyData, importCoordinates, importTopology, importColors,
                              surfaceType, topologyType);
   }
   catch (FileException& e) {
      reader->Delete();
      throw e;
   }
   
   // don't do this polyData->Delete();
   reader->Delete();
}

/**
 * Simplify the surface (this also removed all other surfaces and volumes).
 */
void 
BrainSet::simplifySurface(const BrainModelSurface* bms,
                const int maxPolygons) throw (FileException)
{
   //
   // Decimate the surface
   //
   vtkPolyData* polyData = bms->simplifySurface(maxPolygons);
   if (polyData != NULL) {
      //
      // Clear out the brain set
      //
      reset();
      
      //
      // "Import" the decimated surface
      //
      TopologyFile::TOPOLOGY_TYPES tt = TopologyFile::TOPOLOGY_TYPE_UNKNOWN;
      const TopologyFile* tf = bms->getTopologyFile();
      if (tf != NULL) {
         tt = tf->getTopologyType();
      }
      importVtkTypeFileHelper("", polyData, true, true, false,
                              bms->getSurfaceType(),
                              tt);
      
      //
      // Free up memory
      //
      polyData->Delete();
   }
}      

/**
 * import vtk type file helper.
 */
void
BrainSet::importVtkTypeFileHelper(const QString& filename, vtkPolyData* polyData, 
                                  const bool importCoordinates,
                                  const bool importTopology,
                                  const bool importColors,
                                  const BrainModelSurface::SURFACE_TYPES surfaceType,
                                  const TopologyFile::TOPOLOGY_TYPES topologyType) throw (FileException)
{
   if (polyData == NULL) {
      throw FileException(filename, "Unable to read file");
   }
   
   int numNodes = getNumberOfNodes();
   bool noNodesFlag = (numNodes == 0);
   
   const int numPolys = polyData->GetNumberOfPolys();
   const int numPoints = polyData->GetNumberOfPoints();
   if (numPoints <= 0) {
      throw FileException(filename, "File has no points");
   }
   if (numPolys <= 0) {
      if (polyData->GetNumberOfStrips() <= 0) {
         throw FileException(filename, "File has no triangles or strips.");
      }
   }
   
   if (noNodesFlag == false) {
      if (numPoints != numNodes) {
         throw FileException(filename, "File has different number of nodes than current surfaces.");
      }
   }
   
   if (importTopology) {
         //
         // Get the topology
         //
         TopologyFile* vtkTopology = new TopologyFile;
         try {
            vtkTopology->importFromVtkFile(polyData);
            vtkTopology->setTopologyType(topologyType);
            addTopologyFile(vtkTopology);
            setSelectedTopologyFiles();
         }
         catch (FileException& /*e*/) {
            delete vtkTopology;
            throw FileException(filename, "Error getting topology");
         }
   }
   if (importCoordinates) {
      BrainModelSurface* bms = new BrainModelSurface(this);
      bms->importFromVtkFile(polyData, filename);
      if (numNodes == 0) {
         if (bms->getNumberOfNodes() > 0) {
            numNodesMessage = " contains a different number of nodes than ";
            numNodesMessage.append(FileUtilities::basename(filename));
         }
         else {
            delete bms;
            bms = NULL;
            throw FileException(filename, "Contains no nodes");
         }
      }
      else if (numNodes != bms->getNumberOfNodes()) {
         delete bms;
         bms = NULL;
         throw FileException(FileUtilities::basename(filename), numNodesMessage);
      }
      
      if (getNumberOfTopologyFiles() > 0) {
         bms->setTopologyFile(getTopologyFile(getNumberOfTopologyFiles() - 1));
      }
      bms->orientTilesConsistently();
      bms->computeNormals();
      bms->orientNormalsOut();
      bms->setSurfaceType(surfaceType);
      bms->setStructure(getStructure());
      addBrainModel(bms);
   }
   
   if (importColors) {
      rgbPaintFile->importFromVtkFile(polyData);
   }
   
   if (noNodesFlag) {
      //
      // Reset node coloring
      //
      getPrimarySurfaceOverlay()->setOverlay(-1, BrainModelSurfaceOverlay::OVERLAY_NONE);
      getSecondarySurfaceOverlay()->setOverlay(-1, BrainModelSurfaceOverlay::OVERLAY_NONE);
      getSurfaceUnderlay()->setOverlay(-1, BrainModelSurfaceOverlay::OVERLAY_NONE);
      
      if (importColors) {
         if (rgbPaintFile->getNumberOfColumns() > 0) {
            getPrimarySurfaceOverlay()->setOverlay(-1, BrainModelSurfaceOverlay::OVERLAY_RGB_PAINT);
         }
      }

      postSpecFileReadInitializations();
   }
}
   
/**
 * import Neurolucida file.
 */
void 
BrainSet::importNeurolucidaFile(const QString& filename,
                           const bool importMarkersAsCells,
                           const bool importContours,
                           const bool appendToExistingContours,
                           const bool appendToExistingContourCells) throw (FileException)
{
   NeurolucidaFile nf;
   nf.readFile(filename);
   
   BrainModelContours* bmc = getBrainModelContours(-1);
   if (bmc == NULL) {
      bmc = new BrainModelContours(this);
      addBrainModel(bmc);
   }
   
   if (importContours) {
      try {
         if (appendToExistingContours == false) {
            clearContourFile(false);
         }
         bmc->importNeurolucidaFile(nf, appendToExistingContours);
      }
      catch (FileException& e) {
         throw e;
      }
   }
   
   if (importMarkersAsCells) {
      if (appendToExistingContourCells == false) {
         clearContourCellFile();
         clearContourCellColorFile();
      }
      try {
         contourCellFile->importNeurolucidaFile(nf);
         contourCellColorFile->importNeurolucidaFileColors(nf);
      }
      catch (FileException& e) {
         throw e;
      }
   }

   displaySettingsContours->update();
   displaySettingsCells->update();
}
                            
/**
 * import MD Plot file.
 */
void 
BrainSet::importMDPlotFile(const QString& filename,
                           const bool importPointsAsContourCells,
                           const bool importLinesAsContours,
                           const bool appendToExistingContours,
                           const bool appendToExistingContourCells) throw (FileException)
{
   MDPlotFile mdf;
   mdf.readFile(filename);
   
   BrainModelContours* bmc = getBrainModelContours(-1);
   if (bmc == NULL) {
      bmc = new BrainModelContours(this);
      addBrainModel(bmc);
   }
   
   if (importLinesAsContours) {
      try {
         if (appendToExistingContours == false) {
            clearContourFile(false);
         }
         bmc->importMDPlotFile(mdf, appendToExistingContours);
      }
      catch (FileException& e) {
         throw e;
      }
   }
   
   if (importPointsAsContourCells) {
      if (appendToExistingContourCells == false) {
         clearContourCellFile();
         clearContourCellColorFile();
      }
      try {
         contourCellFile->importMDPlotFile(mdf);
         contourCellColorFile->importMDPlotFileColors();
      }
      catch (FileException& e) {
         throw e;
      }
   }

   displaySettingsContours->update();
   displaySettingsCells->update();
}
                            
/**
 * import mni obj surface file.
 */
void 
BrainSet::importMniObjSurfaceFile(const QString& filename,
                                  const bool importCoordinates,
                                  const bool importTopology,
                                  const bool importColors,
                                  const BrainModelSurface::SURFACE_TYPES surfaceType,
                                  const TopologyFile::TOPOLOGY_TYPES topologyType) throw (FileException)
{
   int numNodes = getNumberOfNodes();
   bool noNodesFlag = (numNodes == 0);
   
   MniObjSurfaceFile mni;
   try {
      mni.readFile(filename);
      
      if (importTopology) {
         //
         // Get the topology
         //
         TopologyFile* mniTopology = new TopologyFile;
         try {
            mniTopology->importFromMniObjSurfaceFile(mni);
            mniTopology->setTopologyType(topologyType);
            addTopologyFile(mniTopology);
            setSelectedTopologyFiles();
         }
         catch (FileException& /*e*/) {
            delete mniTopology;
            throw FileException(filename, "Error getting topology");
         }
      }
      
      //
      // Get the coordinates
      //
      if (mni.getNumberOfPoints() > 0) {
         if (importCoordinates) {
            BrainModelSurface* bms = new BrainModelSurface(this);
            bms->importFromMniObjSurfaceFile(mni);
            if (numNodes == 0) {
               if (bms->getNumberOfNodes() > 0) {
                  numNodesMessage = " contains a different number of nodes than ";
                  numNodesMessage.append(FileUtilities::basename(filename));
               }
               else {
                  delete bms;
                  bms = NULL;
                  throw FileException(filename, "Contains no nodes");
               }
            }
            else if (numNodes != bms->getNumberOfNodes()) {
               delete bms;
               bms = NULL;
               throw FileException(FileUtilities::basename(filename), numNodesMessage);
            }
            
            if (getNumberOfTopologyFiles() > 0) {
               bms->setTopologyFile(getTopologyFile(getNumberOfTopologyFiles() - 1));
            }
            bms->orientTilesConsistently();
            bms->computeNormals();
            bms->setSurfaceType(surfaceType);
            bms->setStructure(getStructure());
            addBrainModel(bms);
         }
         numNodes = getNumberOfNodes();
         
         if (importColors && (numNodes > 0)) {
            
            //
            // Add new column or get the column in the rgb file
            //
            const QString rgbColumnName("MNI OBJ Surface");
            int column = 0;
            if (rgbPaintFile->getNumberOfColumns() <= 0) {
               rgbPaintFile->setNumberOfNodesAndColumns(numNodes, 1);
               column = 0;
            }
            else {
               rgbPaintFile->addColumns(1);
               column = paintFile->getNumberOfColumns() - 1;
            }
            rgbPaintFile->setColumnName(0, rgbColumnName);
            
            //
            // Converts a brain voyager color index into a paint file index
            //
            for (int i = 0; i < numNodes; i++) {
               const unsigned char* rgba = mni.getColorRGBA(i);
               const float r = rgba[0];
               const float g = rgba[1];
               const float b = rgba[2];
               rgbPaintFile->setRgb(i, column, r, g, b);
            }
         }
      }
      else {
         throw FileException(filename, "Has no coordinates");
      }
   }
   catch (FileException& e) {
      throw e;
   }
   
   if (noNodesFlag) {
      postSpecFileReadInitializations();
   }
}

/**
 * import brain voyager file
 */
void 
BrainSet::importBrainVoyagerFile(const QString& filename,
                                 const bool importCoordinates,
                                 const bool importTopology,
                                 const bool importColors,
                                 const BrainModelSurface::SURFACE_TYPES surfaceType,
                                 const TopologyFile::TOPOLOGY_TYPES topologyType) throw (FileException)
{
   int numNodes = getNumberOfNodes();
   bool noNodesFlag = (numNodes == 0);
   
   BrainVoyagerFile bvf;
   try {
      bvf.readFile(filename);
      
      if (importTopology) {
         //
         // Get the topology
         //
         TopologyFile* bvTopology = new TopologyFile;
         try {
            bvTopology->importFromBrainVoyagerFile(bvf);
            bvTopology->setTopologyType(topologyType);
            addTopologyFile(bvTopology);
            setSelectedTopologyFiles();
         }
         catch (FileException& /*e*/) {
            delete bvTopology;
            throw FileException(filename, "Error getting topology");
         }
      }
      
      //
      // Get the coordinates
      //
      if (bvf.getNumberOfVertices() > 0) {
         if (importCoordinates) {
            BrainModelSurface* bms = new BrainModelSurface(this);
            bms->importFromBrainVoyagerFile(bvf);
            if (numNodes == 0) {
               if (bms->getNumberOfNodes() > 0) {
                  numNodesMessage = " contains a different number of nodes than ";
                  numNodesMessage.append(FileUtilities::basename(filename));
               }
               else {
                  delete bms;
                  bms = NULL;
                  throw FileException(filename, "Contains no nodes");
               }
            }
            else if (numNodes != bms->getNumberOfNodes()) {
               delete bms;
               bms = NULL;
               throw FileException(FileUtilities::basename(filename), numNodesMessage);
            }
            
            if (getNumberOfTopologyFiles() > 0) {
               bms->setTopologyFile(getTopologyFile(getNumberOfTopologyFiles() - 1));
            }
            bms->orientTilesConsistently();
            bms->computeNormals();
            bms->setSurfaceType(surfaceType);
            bms->setStructure(getStructure());
            addBrainModel(bms);
         }
         numNodes = getNumberOfNodes();
         
         if (importColors && (numNodes > 0)) {
            
            //
            // Add new column or get the column in the paint file
            //
            const QString paintColumnName("Brain_Voyager");
            int column = 0;
            if (paintFile->getNumberOfColumns() <= 0) {
               paintFile->setNumberOfNodesAndColumns(numNodes, 1);
               column = 0;
            }
            else {
               paintFile->addColumns(1);
               column = paintFile->getNumberOfColumns() - 1;
            }
            paintFile->setColumnName(0, paintColumnName);
            
            const int colorTableSize = bvf.getColorTableSize();
            if (colorTableSize > 0) {
               //
               // Converts a brain voyager color index into a paint file index
               //
               int* colorTableToPaintIndices = new int[colorTableSize];
               for (int i = 0; i < colorTableSize; i++) {
                  const BrainVoyagerColorTableElement* te = bvf.getColorTableElement(i);
                  colorTableToPaintIndices[i] = paintFile->addPaintName(te->getColorName());
                  
                  //
                  // Add color to area color file
                  //
                  unsigned char rgb[3];
                  te->getRgb(rgb);
                  areaColorFile->addColor(te->getColorName(), rgb[0], rgb[1], rgb[2], 255,
                                         2.0, 1.0);
               }
               
               //
               // Set the paint index for the vertex
               //
               for (int i = 0; i < numNodes; i++) {
                  const int index = bvf.getVertexColorIndex(i);
                  if (index < colorTableSize) {
                     paintFile->setPaint(i, column, colorTableToPaintIndices[index]); 
                  }
                  else {
                     std::cout << "Brain Voyager Import Node: " << i 
                               << " has an invalid color table index = " 
                               << index << std::endl;
                  }
               }
               
               delete[] colorTableToPaintIndices;
            }
         }
      }
      else {
         throw FileException(filename, "Has no coordinates");
      }
   }
   catch (FileException& e) {
      throw e;
   }
   
   if (noNodesFlag) {
      postSpecFileReadInitializations();
   }
}

/**
 * Export a brain model surface to a free surfer ascii surface file.
 */
void
BrainSet::exportFreeSurferAsciiSurfaceFile(BrainModelSurface* bms,
                                           const QString& filename) throw (FileException)
{
   FreeSurferSurfaceFile fssf;
   
   TopologyFile* tf = bms->getTopologyFile();
   CoordinateFile* cf = bms->getCoordinateFile();
   fssf.setNumberOfVerticesAndTriangles(cf->getNumberOfCoordinates(),
                                        tf->getNumberOfTiles());
   tf->exportToFreeSurferSurfaceFile(fssf);
   cf->exportToFreeSurferSurfaceFile(fssf);
   fssf.writeFile(filename);
}

/**
 * Import a free surfer ascii surface file.
 */
void 
BrainSet::importFreeSurferSurfaceFile(const QString& filename,
                                      const bool importCoordinates,
                                      const bool importTopology,
                                      const AbstractFile::FILE_FORMAT fileFormat,
                                      const BrainModelSurface::SURFACE_TYPES surfaceType,
                                      const TopologyFile::TOPOLOGY_TYPES topologyType) throw (FileException)
{
   int numNodes = getNumberOfNodes();
   bool noNodesFlag = (numNodes == 0);
   
   FreeSurferSurfaceFile fssf;
   fssf.setFileReadType(fileFormat);
   fssf.readFile(filename);
         
   //
   // Create a new brain model surface and topology file
   //
   BrainModelSurface* bms = NULL;
   TopologyFile* tf = NULL;
   
   try {      
      //
      // If importing topology, copy tiles to the topology file
      //
      if (importTopology) {
         TopologyFile* otherTopoFile = getTopologyClosed();
         if (otherTopoFile == NULL) {
            otherTopoFile = getTopologyOpen();
         }
         if (otherTopoFile == NULL) {
            otherTopoFile = getTopologyUnknown();
         }
         tf = new TopologyFile;
         tf->importFromFreeSurferSurfaceFile(fssf, otherTopoFile);
         tf->setTopologyType(topologyType);
      }

      //
      // Add the new surface
      //
      if (importCoordinates) {
         //
         // Import the coordinates
         //
         bms = new BrainModelSurface(this);
         CoordinateFile* coords = bms->getCoordinateFile();
         coords->importFromFreeSurferSurfaceFile(fssf, numNodes);

         if (importTopology) {
            bms->setTopologyFile(tf);
         }
         else {
            bms->setTopologyFile(getTopologyFile(getNumberOfTopologyFiles() - 1));
         }
         bms->orientTilesConsistently();
         bms->computeNormals();
         bms->setSurfaceType(surfaceType);
         bms->setStructure(getStructure());
         addBrainModel(bms);
      }      

      //
      // need this in case topology importing throws an exception
      //
      if (importTopology) {
         addTopologyFile(tf);
         setSelectedTopologyFiles();
      }
   }
   catch (FileException& e) {
      if (bms != NULL) {
         delete bms;
      }
      if (tf != NULL) {
         delete tf;
      }
      throw e;
   }

            
   if (noNodesFlag) {
      postSpecFileReadInitializations();
   }
}

/**
 * See if this iteration is one in which the brain model should be redrawn.
 */
bool
BrainSet::isIterationUpdate(const int currentAlgorithmIteration) const
{
   if (currentAlgorithmIteration > 0) {
      if ((currentAlgorithmIteration % DebugControl::getIterativeUpdate()) != 0) {
         return false;
      }
   }
   return true;
}

/**
 * Request that a brain model be displayed and drawn by the user of this brain set.
 * If "currentAlgorithmIteration" is greater than zero, the display will only be updated
 * if "currentAlgorithmIteration" modulus iterativeDrawNumber is zero.
 */
void 
BrainSet::drawBrainModel(const int brainModelIndex, const int currentAlgorithmIteration)
{
   if (isIterationUpdate(currentAlgorithmIteration)) {
      emit signalDisplayBrainModel(brainModelIndex);
   }
}
      
/**
 * Request that a brain model be displayed and drawn by the user of this brain set.
 */
void 
BrainSet::drawBrainModel(const BrainModel* bm, const int currentAlgorithmIteration)
{
   const int numBrains = getNumberOfBrainModels();
   for (int i = 0; i < numBrains; i++) {
      if (bm == brainModels[i]) {
         drawBrainModel(i, currentAlgorithmIteration);
         break;
      }
   }
}
      
/**
 * get the volume functional file.
 */
VolumeFile* 
BrainSet::getVolumeFunctionalFile(const int index) 
{ 
   if ((index >= 0) && (index < static_cast<int>(volumeFunctionalFiles.size()))) {
      return volumeFunctionalFiles[index];
   }
   else {
      return NULL;
   }
}

/**
 * get the volume functional file (const method)
 */
const VolumeFile* 
BrainSet::getVolumeFunctionalFile(const int index) const 
{ 
   if ((index >= 0) && (index < static_cast<int>(volumeFunctionalFiles.size()))) {
      return volumeFunctionalFiles[index]; 
   }
   else {
      return NULL;
   }
}

/**
 * get the volume paint file
 */
VolumeFile* 
BrainSet::getVolumePaintFile(const int index) 
{ 
   if ((index >= 0) && (index < static_cast<int>(volumePaintFiles.size()))) {
      return volumePaintFiles[index]; 
   }
   else {
      return NULL;
   }
}

/**
 * get the volume paint file (const method).
 */
const VolumeFile* 
BrainSet::getVolumePaintFile(const int index) const 
{ 
   if ((index >= 0) && (index < static_cast<int>(volumePaintFiles.size()))) {
      return volumePaintFiles[index]; 
   }
   else {
      return NULL;
   }
}
      
/**
 * get the volume prob atlas file
 */
VolumeFile* 
BrainSet::getVolumeProbAtlasFile(const int index) 
{ 
   if ((index >= 0) && (index < static_cast<int>(volumeProbAtlasFiles.size()))) {
      return volumeProbAtlasFiles[index]; 
   }
   else {
      return NULL;
   }
}

/**
 * get the volume prob atlas file (const method).
 */
const VolumeFile* 
BrainSet::getVolumeProbAtlasFile(const int index) const 
{ 
   if ((index >= 0) && (index < static_cast<int>(volumeProbAtlasFiles.size()))) {
      return volumeProbAtlasFiles[index]; 
   }
   else {
      return NULL;
   }
}
      
/**
 * synchronize prob atlas volume region names.
 */
void 
BrainSet::synchronizeProbAtlasVolumeRegionNames()
{
   VolumeFile::synchronizeRegionNames(volumeProbAtlasFiles);
}
      
/**
 * get the volume rgb file
 */
VolumeFile* 
BrainSet::getVolumeRgbFile(const int index) 
{ 
   if ((index >= 0) && (index < static_cast<int>(volumeRgbFiles.size()))) {
      return volumeRgbFiles[index]; 
   }
   else {
      return NULL;
   }
}

/**
 * get the volume rgb file (const method).
 */
const VolumeFile* 
BrainSet::getVolumeRgbFile(const int index) const 
{ 
   if ((index >= 0) && (index < static_cast<int>(volumeRgbFiles.size()))) {
      return volumeRgbFiles[index]; 
   }
   else {
      return NULL;
   }
}
      
/**
 * get the volume segmentation file.
 */
VolumeFile* 
BrainSet::getVolumeSegmentationFile(const int index) 
{ 
   if ((index >= 0) && (index < static_cast<int>(volumeSegmentationFiles.size()))) {
      return volumeSegmentationFiles[index]; 
   }
   else {
      return NULL;
   }
}

/**
 * get the volume segmentation file (const method).
 */
const VolumeFile* 
BrainSet::getVolumeSegmentationFile(const int index) const 
{ 
   if ((index >= 0) && (index < static_cast<int>(volumeSegmentationFiles.size()))) {
      return volumeSegmentationFiles[index]; 
   }
   else {
      return NULL;
   }
}
      

/**
 * get the volume anatomy file.
 */
VolumeFile* 
BrainSet::getVolumeAnatomyFile(const int index) 
{ 
   if ((index >= 0) && (index < static_cast<int>(volumeAnatomyFiles.size()))) {
      return volumeAnatomyFiles[index]; 
   }
   else {
      return NULL;
   }
}

/**
 * get the volume anatomy file (const method).
 */
const VolumeFile* 
BrainSet::getVolumeAnatomyFile(const int index) const 
{ 
   if ((index >= 0) && (index < static_cast<int>(volumeAnatomyFiles.size()))) {
      return volumeAnatomyFiles[index]; 
   }
   else {
      return NULL;
   }
}
      
/**
 * get the volume vector file.
 */
VolumeFile* 
BrainSet::getVolumeVectorFile(const int index) 
{ 
   if ((index >= 0) && (index < static_cast<int>(volumeVectorFiles.size()))) {
      return volumeVectorFiles[index]; 
   }
   else {
      return NULL;
   }
}

/**
 * get the volume vector file (const method).
 */
const VolumeFile* 
BrainSet::getVolumeVectorFile(const int index) const 
{ 
   if ((index >= 0) && (index < static_cast<int>(volumeVectorFiles.size()))) {
      return volumeVectorFiles[index]; 
   }
   else {
      return NULL;
   }
}

/**
 * Get the paint volume file with the specified name (NULL if not found).
 */
VolumeFile*
BrainSet::getVolumePaintFileWithName(const QString& name)
{
   std::vector<VolumeFile*> files;
   getVolumePaintFiles(files);
   return getVolumeFileWithName(files, name);
}

/**
 * Get the prob atlas volume file with the specified name (NULL if not found).
 */
VolumeFile*
BrainSet::getVolumeProbAtlasFileWithName(const QString& name)
{
   std::vector<VolumeFile*> files;
   getVolumeProbAtlasFiles(files);
   return getVolumeFileWithName(files, name);
}

/**
 * Get the RGB volume file with the specified name (NULL if not found).
 */
VolumeFile*
BrainSet::getVolumeRgbFileWithName(const QString& name)
{
   std::vector<VolumeFile*> files;
   getVolumeRgbFiles(files);
   return getVolumeFileWithName(files, name);
}

/**
 * Get the segmentation volume file with the specified name (NULL if not found).
 */
VolumeFile*
BrainSet::getVolumeSegmentationFileWithName(const QString& name)
{
   std::vector<VolumeFile*> files;
   getVolumeSegmentationFiles(files);
   return getVolumeFileWithName(files, name);
}

/**
 * Get the anatomy volume file with the specified name (NULL if not found).
 */
VolumeFile*
BrainSet::getVolumeAnatomyFileWithName(const QString& name)
{
   std::vector<VolumeFile*> files;
   getVolumeAnatomyFiles(files);
   return getVolumeFileWithName(files, name);
}

/**
 * Get the functional volume file with the specified name (NULL if not found).
 */
VolumeFile*
BrainSet::getVolumeFunctionalFileWithName(const QString& name)
{
   std::vector<VolumeFile*> files;
   getVolumeFunctionalFiles(files);
   return getVolumeFileWithName(files, name);
}

/**
 * Get the vector volume file with the specified name (NULL if not found).
 */
VolumeFile*
BrainSet::getVolumeVectorFileWithName(const QString& name)
{
   std::vector<VolumeFile*> files;
   getVolumeVectorFiles(files);
   return getVolumeFileWithName(files, name);
}

/**
 * Get the volume file name with the specified name (NULL if not found).
 */
VolumeFile*
BrainSet::getVolumeFileWithName(const std::vector<VolumeFile*>& files,
                                const QString& fileName)
{
   const QString name = FileUtilities::basename(fileName);
   for (unsigned int i = 0; i < files.size(); i++) {
      if (name == files[i]->getFileNameNoPath()) {
         return files[i];
      }
   }
   
   return NULL;
}

/**
 * clear all node highlight symbols.
 */
void
BrainSet::clearNodeHighlightSymbols()
{
   const int numNodes = getNumberOfNodes();
   for (int i = 0; i < numNodes; i++) {
      BrainSetNodeAttribute* bna = getNodeAttributes(i);
      bna->setHighlighting(BrainSetNodeAttribute::HIGHLIGHT_NODE_NONE);
   }
   clearAllDisplayLists();
}

/**
 * read an image (returns true if read successfully).
 */
bool 
BrainSet::readImage(const QString& filename, const QString& format, QImage& image)
{
   QImage tempImage;
   const bool valid = (tempImage.load(filename.toAscii().constData(), format.toAscii().constData()));
   if (valid) {
      image = QGLWidget::convertToGLFormat(tempImage);
   }
   return valid;
}

/**
 * Set node display flags based upon sections and other criteria.
 */
void
BrainSet::updateNodeDisplayFlags()
{
   resetNodeAttributes();

   const int numNodes = getNumberOfNodes();
   displayAllNodesFlag = true;
   
   bool checkNodes = true;
   
   DisplaySettingsSection* dss = getDisplaySettingsSection();
   
   SectionFile* sf = getSectionFile();
   const int column = dss->getSelectedDisplayColumn(-1, -1);

   if (sf->getNumberOfColumns() == 0) {
      checkNodes = false;
   }
   else {
      if (dss->getSelectionType() == DisplaySettingsSection::SELECTION_TYPE_ALL) {
         checkNodes = false;
      }
   }
   
   if (checkNodes == false) {
      for (int i = 0; i < numNodes; i++) {
         nodeAttributes[i].setDisplayFlag(true);
      }
      return;
   }
   
   const int minSection = dss->getMinimumSelectedSection();
   const int maxSection = dss->getMaximumSelectedSection();
   
   for (int i = 0; i < numNodes; i++) {
      bool displayIt = true;
      const int section = sf->getSection(i, column);
      if ((section < minSection) || (section > maxSection)) {
         displayAllNodesFlag = false;
         displayIt = false;
      }
      nodeAttributes[i].setDisplayFlag(displayIt);
   }
}

/**
 * remove coordinate and topoology files from spec file.
 */
void
BrainSet::removeCoordAndTopoFromSpecFile()
{
   loadedFilesSpecFile.rawCoordFile.clear(false);
   loadedFilesSpecFile.fiducialCoordFile.clear(false);
   loadedFilesSpecFile.inflatedCoordFile.clear(false);
   loadedFilesSpecFile.veryInflatedCoordFile.clear(false);
   loadedFilesSpecFile.sphericalCoordFile.clear(false);
   loadedFilesSpecFile.ellipsoidCoordFile.clear(false);
   loadedFilesSpecFile.compressedCoordFile.clear(false);
   loadedFilesSpecFile.flatCoordFile.clear(false);
   loadedFilesSpecFile.lobarFlatCoordFile.clear(false);
   loadedFilesSpecFile.hullCoordFile.clear(false);
   loadedFilesSpecFile.unknownCoordFile.clear(false);

   loadedFilesSpecFile.closedTopoFile.clear(false);
   loadedFilesSpecFile.openTopoFile.clear(false);
   loadedFilesSpecFile.cutTopoFile.clear(false);
   loadedFilesSpecFile.lobarCutTopoFile.clear(false);

   if (specFileName.isEmpty() == false) {
      try {
         SpecFile sf;
         sf.readFile(specFileName);

         sf.rawCoordFile.clear(false);
         sf.fiducialCoordFile.clear(false);
         sf.inflatedCoordFile.clear(false);
         sf.veryInflatedCoordFile.clear(false);
         sf.sphericalCoordFile.clear(false);
         sf.ellipsoidCoordFile.clear(false);
         sf.compressedCoordFile.clear(false);
         sf.flatCoordFile.clear(false);
         sf.lobarFlatCoordFile.clear(false);
         sf.hullCoordFile.clear(false);
         sf.unknownCoordFile.clear(false);

         sf.closedTopoFile.clear(false);
         sf.openTopoFile.clear(false);
         sf.cutTopoFile.clear(false);
         sf.lobarCutTopoFile.clear(false);

         sf.writeFile(specFileName);
      }
      catch (FileException) {
         // do nothing
      }
   }
}


/**
 * Delete all borders
 */
void
BrainSet::deleteAllBorders()
{
   brainModelBorderSet->deleteAllBorders();
   loadedFilesSpecFile.borderProjectionFile.setAllSelections(SpecFile::SPEC_FALSE);
   loadedFilesSpecFile.rawBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
   loadedFilesSpecFile.fiducialBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
   loadedFilesSpecFile.inflatedBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
   loadedFilesSpecFile.veryInflatedBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
   loadedFilesSpecFile.sphericalBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
   loadedFilesSpecFile.ellipsoidBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
   loadedFilesSpecFile.compressedBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
   loadedFilesSpecFile.flatBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
   loadedFilesSpecFile.lobarFlatBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
   loadedFilesSpecFile.hullBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
   loadedFilesSpecFile.unknownBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
   loadedFilesSpecFile.volumeBorderFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearAreaColorFile()
{
   areaColorFile->clear();
   loadedFilesSpecFile.areaColorFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearArealEstimationFile()
{
   arealEstimationFile->clear();
   loadedFilesSpecFile.arealEstimationFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearBorderColorFile()
{
   borderColorFile->clear();
   loadedFilesSpecFile.borderColorFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearCellColorFile()
{
   cellColorFile->clear();
   loadedFilesSpecFile.cellColorFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearCocomacConnectivityFile()
{
   cocomacFile->clear();
   loadedFilesSpecFile.cocomacConnectivityFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearContourCellColorFile()
{
   contourCellColorFile->clear();
   loadedFilesSpecFile.contourCellColorFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearContourCellFile()
{
   contourCellFile->clear();
   loadedFilesSpecFile.contourCellFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearContourFile(const bool deleteBrainModelContoursFlag)
{
   if (deleteBrainModelContoursFlag) {
      BrainModelContours* bmc = getBrainModelContours(-1);
      if (bmc != NULL) {
         bmc->reset();
         deleteBrainModel(bmc);
         bmc = NULL;
      }
   }
   loadedFilesSpecFile.contourFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearCutsFile()
{
   cutsFile->clear();
   loadedFilesSpecFile.cutsFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearDeformationFieldFile()
{
   deformationFieldFile->clear();
   loadedFilesSpecFile.deformationFieldFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearDeformationMapFile()
{
   deformationMapFileName = "";
   loadedFilesSpecFile.deformationMapFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearFociColorFile()
{
   fociColorFile->clear();
   loadedFilesSpecFile.fociColorFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearFociSearchFile()
{
   fociSearchFile->clear();
   loadedFilesSpecFile.fociSearchFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearGeodesicDistanceFile()
{
   geodesicDistanceFile->clear();
   loadedFilesSpecFile.geodesicDistanceFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearLatLonFile()
{
   latLonFile->clear();
   loadedFilesSpecFile.latLonFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearMetricFile()
{
   metricFile->clear();
   loadedFilesSpecFile.metricFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearPaintFile()
{
   paintFile->clear();
   loadedFilesSpecFile.paintFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearPaletteFile()
{
   paletteFile->clear();
   loadedFilesSpecFile.paletteFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearParamsFile()
{
   paramsFile->clear();
   loadedFilesSpecFile.paramsFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearProbabilisticAtlasFile()
{
   probabilisticAtlasSurfaceFile->clear();
   loadedFilesSpecFile.atlasFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearRgbPaintFile()
{
   rgbPaintFile->clear();
   loadedFilesSpecFile.rgbPaintFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearSceneFile()
{
   sceneFile->clear();
   loadedFilesSpecFile.sceneFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearSectionFile()
{
   sectionFile->clear();
   loadedFilesSpecFile.sectionFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearSurfaceShapeFile()
{
   surfaceShapeFile->clear();
   loadedFilesSpecFile.surfaceShapeFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * Add a vector file.
 */
void
BrainSet::addVectorFile(VectorFile* vf)
{
   vectorFiles.push_back(vf);
}

/**
 * Remove a vector file.
 */
void
BrainSet::removeVectorFile(const int indx)
{
   loadedFilesSpecFile.vectorFile.clearSelectionStatus(vectorFiles[indx]->getFileName());
   vectorFiles.erase(vectorFiles.begin() + indx);
}

/**
 * Remove a vector file.
 */
void
BrainSet::removeVectorFile(VectorFile* vf)
{
   for (int i = 0; i < getNumberOfVectorFiles(); i++) {
      if (getVectorFile(i) == vf) {
         removeVectorFile(i);
         break;
      }
   }
}

/**
 *  get vector file's index.
 */
int
BrainSet::getVectorFileIndex(VectorFile* vf)
{
   for (int i = 0; i < getNumberOfVectorFiles(); i++) {
      if (vectorFiles[i] == vf) {
         return i;
      }
   }
   return -1;
}

/**
 * clear the file
 */
void 
BrainSet::clearVectorFiles()
{
   for (int i = 0; i < getNumberOfVectorFiles(); i++) {
      delete vectorFiles[i];
   }
   vectorFiles.clear();
   loadedFilesSpecFile.vectorFile.setAllSelections(SpecFile::SPEC_FALSE);
   clearAllDisplayLists();
}

/**
 * clear the file
 */
void 
BrainSet::clearTopographyFile()
{
   topographyFile->clear();
   loadedFilesSpecFile.topographyFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearTransformationMatrixFile()
{
   transformationMatrixFile->clear();
   loadedFilesSpecFile.transformationMatrixFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the transformation data files.
 */
void 
BrainSet::clearTransformationDataFiles()
{
   for (int i = 0; i < getNumberOfTransformationDataFiles(); i++) {
      delete transformationDataFiles[i];
   }
   transformationDataFiles.clear();
   loadedFilesSpecFile.transformationDataFile.setAllSelections(SpecFile::SPEC_FALSE);
}
      
/**
 * clear the file
 */
void 
BrainSet::clearStudyMetaDataFile()
{
   studyMetaDataFile->clear();
   loadedFilesSpecFile.studyMetaDataFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearStudyCollectionFile()
{
   studyCollectionFile->clear();
   loadedFilesSpecFile.studyCollectionFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearWustlRegionFile()
{
   wustlRegionFile->clear();
   loadedFilesSpecFile.wustlRegionFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the file
 */
void 
BrainSet::clearVocabularyFile()
{
   vocabularyFile->clear();
   loadedFilesSpecFile.vocabularyFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the anatomy volume files.
 */
void 
BrainSet::clearVolumeAnatomyFiles()
{
   for (unsigned int i = 0; i < volumeAnatomyFiles.size(); i++) {
      delete volumeAnatomyFiles[i];
   }
   volumeAnatomyFiles.clear();
   loadedFilesSpecFile.volumeAnatomyFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the functional volume files.
 */
void 
BrainSet::clearVolumeFunctionalFiles()
{
   for (unsigned int i = 0; i < volumeFunctionalFiles.size(); i++) {
      delete volumeFunctionalFiles[i];
   }
   volumeFunctionalFiles.clear();
   loadedFilesSpecFile.volumeFunctionalFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the paint volume files.
 */
void 
BrainSet::clearVolumePaintFiles()
{
   for (unsigned int i = 0; i < volumePaintFiles.size(); i++) {
      delete volumePaintFiles[i];
   }
   volumePaintFiles.clear();
   loadedFilesSpecFile.volumePaintFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the prob atlasvolume files.
 */
void 
BrainSet::clearVolumeProbabilisticAtlasFiles()
{
   for (unsigned int i = 0; i < volumeProbAtlasFiles.size(); i++) {
      delete volumeProbAtlasFiles[i];
   }
   volumeProbAtlasFiles.clear();
   loadedFilesSpecFile.volumeProbAtlasFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the rgb volume files.
 */
void 
BrainSet::clearVolumeRgbFiles()
{
   for (unsigned int i = 0; i < volumeRgbFiles.size(); i++) {
      delete volumeRgbFiles[i];
   }
   volumeRgbFiles.clear();
   loadedFilesSpecFile.volumeRgbFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the segmentation volume files.
 */
void 
BrainSet::clearVolumeSegmentationFiles()
{
   for (unsigned int i = 0; i < volumeSegmentationFiles.size(); i++) {
      delete volumeSegmentationFiles[i];
   }
   volumeSegmentationFiles.clear();
   loadedFilesSpecFile.volumeSegmentationFile.setAllSelections(SpecFile::SPEC_FALSE);
}

/**
 * clear the vector volume files.
 */
void 
BrainSet::clearVolumeVectorFiles()
{
   for (unsigned int i = 0; i < volumeVectorFiles.size(); i++) {
      delete volumeVectorFiles[i];
   }
   volumeVectorFiles.clear();
   loadedFilesSpecFile.volumeVectorFile.setAllSelections(SpecFile::SPEC_FALSE);
}
      
/**
 * convert displayed borders into a VTK model.
 */
void 
BrainSet::convertDisplayedBordersToVtkModel(const BrainModelSurface* bms)
{
   BrainModelBorderSet* bmbs = getBorderSet();
   if (bmbs != NULL) {
      BorderFile bf;
      bmbs->copyBordersToBorderFile(bms, bf);
      
      BorderFile displayedBorders;
      const int num = bf.getNumberOfBorders();
      for (int i = 0; i < num; i++) {
         const Border* b = bf.getBorder(i);
         if (b->getDisplayFlag()) {
            displayedBorders.addBorder(*b);
         }
      }
      
      if (displayedBorders.getNumberOfBorders() > 0) {
         VtkModelFile* vmf = new VtkModelFile(&displayedBorders, borderColorFile);
         addVtkModelFile(vmf);
      }
   }
}

/**
 * convert displayed cells into a VTK model.
 */
void 
BrainSet::convertDisplayedCellsToVtkModel(const BrainModelSurface* bms)
{
   //
   // Check for fiducial surface
   //
   const bool fiducialSurfaceFlag = 
      ((bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_RAW) ||
       (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL));

   CellFile cf;
   cellProjectionFile->getCellFile(bms->getCoordinateFile(),
                                   bms->getTopologyFile(),
                                   fiducialSurfaceFlag,
                                   cf);
   const int numCells = cf.getNumberOfCells();
   if (numCells > 0) {
      CellFile displayedCells;
      for (int i = 0; i < numCells; i++) {
         CellData* cd = cf.getCell(i);
         if (cd->getDisplayFlag()) {
            displayedCells.addCell(*cd);
         }
      }
      
      if (displayedCells.getNumberOfCells() > 0) {
         VtkModelFile* vmf = new VtkModelFile(&displayedCells, cellColorFile);
         addVtkModelFile(vmf);
      }
   }
}

/**
 * convert displayed foci into a VTK model.
 */
void 
BrainSet::convertDisplayedFociToVtkModel(const BrainModelSurface *bms)
{
   //
   // Check for fiducial surface
   //
   const bool fiducialSurfaceFlag = 
      ((bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_RAW) ||
       (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL));

   FociFile ff;
   fociProjectionFile->getCellFile(bms->getCoordinateFile(),
                                   bms->getTopologyFile(),
                                   fiducialSurfaceFlag,
                                   ff);
   const int numCells = ff.getNumberOfCells();
   if (numCells > 0) {
      FociFile displayedFoci;
      const int numFoci = ff.getNumberOfCells();
      for (int i = 0; i < numFoci; i++) {
         CellData* cd = ff.getCell(i);
         if (cd->getDisplayFlag()) {
            displayedFoci.addCell(*cd);
         }
      }
      
      if (displayedFoci.getNumberOfCells() > 0) {
         VtkModelFile* vmf = new VtkModelFile(&displayedFoci, fociColorFile);
         addVtkModelFile(vmf);
      }
   }
}      

/**
 * update displayed model indices.
 */
void 
BrainSet::updateDisplayedModelIndices()
{
   for (int i = 0; i < BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; i++) {
      if (displayedModelIndices[i] >= getNumberOfBrainModels()) {
         displayedModelIndices[i] = 0;
      }
   }
}

/**
 * get the displayed model index for a window.
 */
int 
BrainSet::getDisplayedModelIndexForWindow(const BrainModel::BRAIN_MODEL_VIEW_NUMBER windowNumber) const
{
   if (displayedModelIndices[windowNumber] >= getNumberOfBrainModels()) {
      return 0;
   }
   return displayedModelIndices[windowNumber];
}

/**
 * set the displayed model index for a window.
 */
void 
BrainSet::setDisplayedModelIndexForWindow(const BrainModel::BRAIN_MODEL_VIEW_NUMBER windowNumber,
                                     const int modelIndex)
{
   displayedModelIndices[windowNumber] = modelIndex;
}

