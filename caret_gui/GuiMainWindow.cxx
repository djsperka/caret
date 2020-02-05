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

//
// This file contains the class methods that create the main window.
//

#include <QGlobalStatic>
#include <QCloseEvent>

#include <algorithm>
#include <iostream>
#include <sstream>

#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDesktopWidget>
#include <QDir>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QLabel>
#include <QLayout>
#include <QProgressDialog>
#include <QPushButton>
#include <QStatusBar>
#include <QToolTip>

#include "AreaColorFile.h"
#include "ArealEstimationFile.h"
#include "BrainModelBorderSet.h"
#include "BrainModelVolumeVoxelColoring.h"
#include "BorderColorFile.h"
#include "BorderFile.h"
#include "BorderProjectionFile.h"
#include "BrainModelContours.h"
#include "BrainModelIdentification.h"
#include "BrainModelSurfaceAndVolume.h"
#include "BrainModelSurfaceNodeColoring.h"
#include "BrainSet.h"
#include "GuiCaretCommandDialog.h"
#include "GuiCaretCommandScriptBuilderDialog.h"
#include "CaretVersion.h"
#include "GuiCellAndFociAttributeAssignmentDialog.h"
#include "CellColorFile.h"
#include "CellProjectionFile.h"
#include "CocomacConnectivityFile.h"
#include "CommunicatorClientAFNI.h"
#include "CommunicatorClientFIV.h"
#include "ContourCellColorFile.h"
#include "ContourCellFile.h"
#include "CutsFile.h"
#include "DebugControl.h"
#include "DeformationFieldFile.h"
#include "DisplaySettingsBorders.h"
#include "DisplaySettingsCells.h"
#include "DisplaySettingsContours.h"
#include "DisplaySettingsFoci.h"
#include "DisplaySettingsScene.h"
#include "DisplaySettingsSection.h"
#include "DisplaySettingsSurface.h"
#include "FileUtilities.h"
#include "FociColorFile.h"
#include "FociProjectionFile.h"
#include "FociSearchFile.h"
#include "GeodesicDistanceFile.h"
#include "GuiBrainModelViewingWindow.h"
#include "GuiAddCellsDialog.h"
#include "GuiAutomaticRotationDialog.h"
#include "GuiBordersCreateInterpolatedDialog.h"
#include "GuiBorderDrawUpdateDialog.h"
#include "GuiBorderOperationsDialog.h"
#include "GuiCaptureWindowImageDialog.h"
#include "GuiColorKeyDialog.h"
#include "GuiContourAlignmentDialog.h"
#include "GuiContourDrawDialog.h"
#include "GuiContourSectionControlDialog.h"
#include "GuiContourSetScaleDialog.h"
#include "GuiDataFileMathDialog.h"
#include "GuiDrawBorderDialog.h"
#include "GuiConnectivityDialog.h"
#include "GuiFilesModified.h"
#include "GuiFlattenFullHemisphereDialog.h"
#include "GuiImageEditorWindow.h"
#include "GuiImageViewingWindow.h"
#include "GuiIdentifyDialog.h"
#include "GuiInterpolateSurfacesDialog.h"
#include "GuiMainWindow.h"
#include "GuiMainWindowAttributesActions.h"
#include "GuiMainWindowAttributesMenu.h"
#include "GuiMainWindowCommActions.h"
#include "GuiMainWindowCommMenu.h"
#include "GuiMainWindowFileActions.h"
#include "GuiMainWindowFileMenu.h"
#include "GuiMainWindowHelpActions.h"
#include "GuiMainWindowHelpMenu.h"
#include "GuiMainWindowLayersActions.h"
#include "GuiMainWindowLayersMenu.h"
#include "GuiMainWindowSurfaceActions.h"
#include "GuiMainWindowSurfaceMenu.h"
#include "GuiMainWindowTimingActions.h"
#include "GuiMainWindowTimingMenu.h"
#include "GuiMainWindowVolumeActions.h"
#include "GuiMainWindowVolumeMenu.h"
#include "GuiMainWindowWindowActions.h"
#include "GuiMainWindowWindowMenu.h"
#include "GuiMapStereotaxicFocusDialog.h"
#include "GuiMetricModificationDialog.h"
#include "GuiMetricsToRgbPaintDialog.h"
#include "GuiModelsEditorDialog.h"
#include "GuiMorphingDialog.h"
#include "GuiBrainModelOpenGL.h"
#include "GuiDisplayControlDialog.h"
#include "GuiPaintNameEditorDialog.h"
#include "GuiPaletteEditorDialog.h"
#include "GuiParamsFileEditorDialog.h"
#include "GuiPreferencesDialog.h"
#include "GuiRecordingDialog.h"
#include "GuiSectionControlDialog.h"
#include "GuiSetTopologyDialog.h"
#include "GuiSmoothingDialog.h"
#include "GuiSpecFileDialog.h"
//#include "GuiSpeechGenerator.h"
#include "GuiStandardMeshDialog.h"
#include "GuiStudyCollectionFileEditorDialog.h"
#include "GuiStudyMetaDataFileEditorDialog.h"
#include "GuiSurfaceRegionOfInterestDialog.h"
#include "GuiSurfaceRegionOfInterestDialogOLD.h"
#include "GuiTransformationMatrixDialog.h"
#include "GuiVocabularyFileEditorDialog.h"
#include "GuiVolumeBiasCorrectionDialog.h"
#include "GuiTrajectoryToolDialog.h"
#include "GuiVolumeResizingDialog.h"
#include "GuiVolumeThresholdSegmentationDialog.h"
#include "GuiVolumeAttributesDialog.h"
#include "GuiVolumeRegionOfInterestDialog.h"
#include "GuiVolumeSegmentationEditorDialog.h"
#include "GuiVolumePaintEditorDialog.h"
#include "GuiToolBar.h"
#include "ImageFile.h"
#include "LatLonFile.h"
#include "MetricFile.h"
#include "PaintFile.h"
#include "PaletteFile.h"
#include "ParamsFile.h"
#include "ProbabilisticAtlasFile.h"
#include "GuiHelpViewerWindow.h"
#include "QtUtilities.h"
#include "RgbPaintFile.h"
#include "SceneFile.h"
#include "SectionFile.h"
#include "StringUtilities.h"
#include "StudyCollectionFile.h"
#include "StudyMetaDataFile.h"
#include "SurfaceShapeFile.h"
#include "VectorFile.h"
#include "SystemUtilities.h"
#include "TopographyFile.h"
#include "TrajectoryFile.h"
#include "VocabularyFile.h"
#include "VtkModelFile.h"
#include "WustlRegionFile.h"
#include "WuQDataEntryDialog.h"
#include "WuQMessageBox.h"

/**
 * Constructor.
 */
GuiMainWindow::GuiMainWindow(const bool enableTimingMenu,
                             const int openGLsizeX,
                             const int openGLsizeY)
   : QMainWindow(0)
{
   setAttribute(Qt::WA_DeleteOnClose);
   
   //
   // Create the main brain surface.
   //
   BrainSet* bs = new BrainSet(true);
   addBrainSet(bs);
   //loadedBrainSetDirectory.push_back(QDir::currentPath());
   for (int i = 0; i < BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; i++) {
      brainSetInWindow[i] = bs;
   }
   
   //
   // Create the AFNI communicator
   //
   afniClientCommunicator = new CommunicatorClientAFNI;
   
   //
   // Create the FIV communicator
   //
   fivClientCommunicator = new CommunicatorClientFIV;

   //
   // Initialize caret communicator invalid
   //   
   communicatorServerCaret = NULL;

   //
   // Create the speech generator
   //
   //speechGenerator = new GuiSpeechGenerator(getBrainSet()->getPreferencesFile());

   sizePolicyFixed = new QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   
   //
   // Desired width and size of OpenGL widget
   //
   const int desiredWidth = 500;
   const int desiredHeight = 500;
   
   //
   // Is a specific size OpenGL window requested (typically used when recording MPEG)
   //
   if ((openGLsizeX > 0) && (openGLsizeY > 0)) {
      //
      // Create the OpenGL widget
      //
      mainOpenGL = new GuiBrainModelOpenGL(0, NULL, "mainWidgetOpenGL",
                                             BrainModel:: BRAIN_MODEL_VIEW_MAIN_WINDOW);
      mainOpenGL->setFixedSize(QSize(openGLsizeX, openGLsizeY));

      //
      // Space on right side
      //
      const int rightWidth = desiredWidth - openGLsizeX;
      //
      // Dummy widgets on bottom and right side of OpenGL widget
      //
      QWidget* rightWidget = NULL;
      if (rightWidth > 0) {
         rightWidget = new QWidget;
         rightWidget->setFixedSize(rightWidth, 10);
      }
      
      //
      // OpenGL for brain surface
      //
      QWidget* openGLBox = new QWidget(this);
      QHBoxLayout* openGLLayout = new QHBoxLayout(openGLBox);
      openGLLayout->addWidget(mainOpenGL);
      openGLLayout->setStretchFactor(mainOpenGL, 0);
      if (rightWidget != NULL) {
         openGLLayout->addWidget(rightWidget);
         openGLLayout->setStretchFactor(rightWidget, 0);
      }
      setCentralWidget(openGLBox);
   }
   else {
      //
      // OpenGL for brain surface
      //
      mainOpenGL = new GuiBrainModelOpenGL(this, NULL, "mainWidgetOpenGL",
                                             BrainModel:: BRAIN_MODEL_VIEW_MAIN_WINDOW);
   
      setCentralWidget(mainOpenGL);
      mainOpenGL->setMinimumSize(desiredWidth, desiredHeight);
   }
   
   //
   // Create the actions
   //
   attributeActions = new GuiMainWindowAttributesActions(this);
   commActions = new GuiMainWindowCommActions(this);
   fileActions = new GuiMainWindowFileActions(this);
   helpActions = new GuiMainWindowHelpActions(this);
   layersActions = new GuiMainWindowLayersActions(this);
   surfaceActions = new GuiMainWindowSurfaceActions(this);
   timingActions = new GuiMainWindowTimingActions(this);
   volumeActions = new GuiMainWindowVolumeActions(this);
   windowActions = new GuiMainWindowWindowActions(this);
   
   //
   // Create the menus
   //
   fileMenu = new GuiMainWindowFileMenu(this, mainOpenGL);
   menuBar()->addMenu(fileMenu);
   
   attributesMenu = new GuiMainWindowAttributesMenu(this);
   menuBar()->addMenu(attributesMenu);
   
   layersMenu = new GuiMainWindowLayersMenu(this);
   menuBar()->addMenu(layersMenu);
   
   surfaceMenu = new GuiMainWindowSurfaceMenu(this);
   menuBar()->addMenu(surfaceMenu);
   
   volumeMenu = new GuiMainWindowVolumeMenu(this);
   menuBar()->addMenu(volumeMenu);
   
   commMenu = new GuiMainWindowCommMenu(this);
   menuBar()->addMenu(commMenu);
   
   windowMenu = new GuiMainWindowWindowMenu(this);
   menuBar()->addMenu(windowMenu);
   
   helpMenu = new GuiMainWindowHelpMenu(this);
   menuBar()->addMenu(helpMenu);
   
   if (enableTimingMenu) {
      timingMenu = new GuiMainWindowTimingMenu(this);
      menuBar()->addMenu(timingMenu);
   }
   
   //
   // Create toolbar after GuiBrainModelOpenGL since the toolbar connects
   // to slots  in it.
   //
   toolBar = new GuiToolBar(this, this, mainOpenGL, 
                            BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW);
   addToolBar(toolBar);
   QObject::connect(toolBar, SIGNAL(modelSelection(int)),
                    this, SLOT(updateDisplayedMenus()));
   
   //
   // Create the status bar
   // 
   createStatusBar();
   
   //
   // Initialize other dialog/windows
   //
   displayControlDialog = NULL;
   for (int i = 0; i < BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; i++) {
      modelWindow[i] = NULL;
   }   
   addCellsDialog          = NULL;
   addContourCellsDialog   = NULL;
   alignSurfaceToStandardOrientationDialog = NULL;
   automaticRotationDialog = NULL;
   borderDrawUpdateDialog  = NULL;
   bordersCreateInterpolatedDialog = NULL;
   captureWindowImageDialog = NULL;
   caretCommandExecutorDialog = NULL;
   caretCommandScriptBuilderDialog = NULL;
   contourAlignmentDialog  = NULL;
   contourDrawDialog       = NULL;
   contourSectionControlDialog = NULL;
   contourSetScaleDialog   = NULL;
   borderOperationsDialog  = NULL;
   drawBorderDialog        = NULL;
   flatMorphingDialog      = NULL;
   flattenFullHemisphereDialog = NULL;
   fociAttributeAssignmentDialog = NULL;
   connectivityDialog    = NULL;
   helpViewerDialog        = NULL;
   imageEditorWindow       = NULL;
   identifyDialog          = NULL;
   interpolateSurfaceDialog = NULL;
   mapStereotaxicFocusDialog = NULL;
   metricMathDialog        = NULL;
   metricModificationDialog   = NULL;
   metricsToRgbPaintDialog = NULL;
   modelsEditorDialog      = NULL;
   paletteEditorDialog     = NULL;
   paintNameEditorDialog = NULL;
   paramsFileEditorDialog  = NULL;
   preferencesDialog       = NULL;
   recordingDialog         = NULL;
   sectionControlDialog    = NULL;
   setTopologyDialog       = NULL;
   smoothingDialog         = NULL;
   shapeModificationDialog = NULL;
   shapeMathDialog         = NULL;
   sphereMorphingDialog    = NULL;
   standardMeshDialog      = NULL;
   studyCollectionFileEditorDialog = NULL;
   studyMetaDataFileEditorDialog = NULL;
   surfaceRegionOfInterestDialog = NULL;
   surfaceRegionOfInterestDialogOLD = NULL;
   trajectoryToolDialog = NULL;
   transformMatrixEditorDialog = NULL;
   volumeThresholdSegmentationDialog = NULL;
   volumeResizingDialog    = NULL;
   volumeAttributesDialog  = NULL;
   volumeBiasCorrectionDialog = NULL;
   volumeMathDialog        = NULL;
   volumeRegionOfInterestDialog = NULL;
   volumePaintEditorDialog = NULL;
   volumeSegmentationEditorDialog = NULL;
   
   vocabularyFileEditorDialog = NULL;
   
   arealEstimationColorKeyDialog = NULL;
   borderColorKeyDialog = NULL;
   cellColorKeyDialog = NULL;
   fociColorKeyDialog = NULL;
   paintColorKeyDialog = NULL;
   probAtlasColorKeyDialog = NULL;
   volumePaintColorKeyDialog = NULL;
   volumeProbAtlasColorKeyDialog = NULL;
    
   QString title("SPLASH/CARET v");
#ifdef UBUNTU
   title.append("u");
#endif
   title.append(CaretVersion::getCaretVersionAsString());
   title.append(" (");
   title.append(__DATE__);
   title.append(")");
   setWindowTitle(title);
   
   //
   // Use the main winodw for parents of progress dialog created
   // by the "BrainSet".
   //
   getBrainSet()->setProgressDialogParent(this);
   
   //
   // The BrainSetwill emit signals when it wants a brain model displayed and/or drawn
   //
   QObject::connect(getBrainSet(), SIGNAL(signalDisplayBrainModel(int)),
                    this, SLOT(displayBrainModelInMainWindow(int)));
   
   //
   // The BrainSetwill emit signals the spec file is changed
   //
   QObject::connect(getBrainSet(), SIGNAL(signalBrainSetChanged()),
                    this, SLOT(postSpecFileReadInitializations()));
}

/**
 * Destructor.
 */
GuiMainWindow::~GuiMainWindow()
{
   if (afniClientCommunicator != NULL) {
      delete afniClientCommunicator;
      afniClientCommunicator = NULL;
   }
   if (fivClientCommunicator != NULL) {
      delete fivClientCommunicator;
      fivClientCommunicator = NULL;
   }
   //if (speechGenerator != NULL) {
   //   delete speechGenerator;
   //   speechGenerator = NULL;
   //}
   for (unsigned int i = 0; i < loadedBrainSets.size(); i++) {
      delete loadedBrainSets[i];
   }
   loadedBrainSets.clear();
   //loadedBrainSetDirectory.clear();
}

/**
 * add a brain set.
 */
void 
GuiMainWindow::addBrainSet(BrainSet* bs)
{
   loadedBrainSets.push_back(bs);
   QObject::connect(bs, SIGNAL(signalGraphicsUpdate(BrainSet*)),
                    this, SLOT(slotRedrawWindowsUsingBrainSet(BrainSet*)));
}
      
/**
 * get the active brain structure for specified window.  If no argument 
 * is passed the brain set for the main window will be returned.
 */
BrainSet* 
GuiMainWindow::getBrainSet(const BrainModel::BRAIN_MODEL_VIEW_NUMBER windowNumber) 
{ 
   return brainSetInWindow[windowNumber]; 
}
      
/**
 * get the active brain structure for specified window.  If no argument 
 * is passed the brain set for the main window will be returned.
 */
const BrainSet* 
GuiMainWindow::getBrainSet(const BrainModel::BRAIN_MODEL_VIEW_NUMBER windowNumber) const
{ 
   return brainSetInWindow[windowNumber]; 
}
      
/**
 * set the active brain structure for specified window.
 */
void 
GuiMainWindow::setBrainSet(const BrainModel::BRAIN_MODEL_VIEW_NUMBER windowNumber,
                           BrainSet* newActiveBrainSet)
{
   brainSetInWindow[windowNumber] = newActiveBrainSet; 
   
   //
   // Update current directory to that of current spec file and file name prefixing
   //
   if (windowNumber == BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW) {
      for (unsigned int i = 0; i < loadedBrainSets.size(); i++) {
         if (loadedBrainSets[i] == getBrainSet(windowNumber)) {
            //QDir::setCurrent(loadedBrainSetDirectory[i]);
            QDir::setCurrent(FileUtilities::dirname(loadedBrainSets[i]->getSpecFileName()));
            loadedBrainSets[i]->updateDefaultFileNamePrefix();
            break;
         }
      }
   }
}

/**
 * speak some text.
 */
void 
GuiMainWindow::speakText(const QString& /*text*/, const bool /*verboseSpeech*/)
{
   //if (speechGenerator != NULL) {
   //   speechGenerator->speakText(text, verboseSpeech);
   //}
}      

/**
 * Slot to display the brain model in the main window
 */
void
GuiMainWindow::displayBrainModelInMainWindow(int modelNumberIn)
{
   //
   // Verify valid model index for current brain set
   //
   const int numModels = getBrainSet()->getNumberOfBrainModels();
   if (numModels == 0) {
      return;
   }
   int modelNumber = modelNumberIn;
   if ((modelNumber < 0) || (modelNumber >= numModels)){
      modelNumber = numModels - 1;
   }
   
   //
   // Since the index is for the currently loaded brain set and multiple brain sets
   // may be loaded AND the toolbar lists brain models for all brain sets, get the proper
   // offset into the toolbar.
   //
   if (getNumberOfBrainSets() > 1) {
      int modelOffset = 0;
      for (int i = 0; i < getNumberOfBrainSets(); i++) {
         BrainSet* bs = getBrainSetByIndex(i);
         if (bs == getBrainSet()) {
            break;
         }
         else {
            modelOffset += bs->getNumberOfBrainModels();
         }
      }
      modelNumber += modelOffset;
   }
   
   toolBar->setModelSelection(modelNumber);
   GuiToolBar::updateAllToolBars(true);
   updateDisplayedMenus();
   GuiBrainModelOpenGL::updateAllGL(mainOpenGL);
}

/**
 * Display the brain model in the main window
 */
void
GuiMainWindow::displayBrainModelInMainWindow(BrainModel* bm)
{
   if (bm != NULL) {
      const int numModels = getBrainSet()->getNumberOfBrainModels();
      for (int i = 0; i < numModels; i++) {
         if (getBrainSet()->getBrainModel(i) == bm) {
            displayBrainModelInMainWindow(i);
            break;
         }
      }
   }
}

/**
 * Display a volume in the main window
 */
void
GuiMainWindow::displayVolumeInMainWindow()
{
   displayBrainModelInMainWindow(getBrainSet()->getBrainModelVolume());
}

/**
 * Display contours in the main window
 */
void
GuiMainWindow::displayContoursInMainWindow()
{
   displayBrainModelInMainWindow(getBrainSet()->getBrainModelContours());
}

/**
 * Display the newest surface in the main window.
 */
void
GuiMainWindow::displayNewestSurfaceInMainWindow()
{
   const BrainModelSurface* bmsv = 
      dynamic_cast<BrainModelSurface*>(getBrainSet()->getBrainModelSurfaceAndVolume());
   
   const int startIndex = getBrainSet()->getNumberOfBrainModels() - 1;
   for (int i = startIndex; i >= 0; i--) {
      BrainModelSurface* bms = getBrainSet()->getBrainModelSurface(i);
      if ((bms != NULL) && (bms != bmsv)) {
         displayBrainModelInMainWindow(bms);
         break;
      }
   }
}

/**
 * Get the Brain Model in the main window (NULL if no model).
 */
BrainModel* 
GuiMainWindow::getBrainModel()
{
   return mainOpenGL->getDisplayedBrainModel();
}
      
/**
 * Get the Brain Model Contours in the main window (NULL if not a surface)
 */
BrainModelContours* 
GuiMainWindow::getBrainModelContours()
{
   return mainOpenGL->getDisplayedBrainModelContours();
}

/**
 * Get the Brain Model Surface in the main window (NULL if not a surface)
 */
BrainModelSurface* 
GuiMainWindow::getBrainModelSurface()
{
   return mainOpenGL->getDisplayedBrainModelSurface();
}

/**
 * Get the Brain Model Surface and Volume in the main window (NULL if not a surface)
 */
BrainModelSurfaceAndVolume* 
GuiMainWindow::getBrainModelSurfaceAndVolume()
{
   return mainOpenGL->getDisplayedBrainModelSurfaceAndVolume();
}

/**
 * Get the Brain Model Volume in the main window (NULL if not a volume)
 */
BrainModelVolume* 
GuiMainWindow::getBrainModelVolume()
{
   return mainOpenGL->getDisplayedBrainModelVolume();
}
      
/**
 * Get the index of the Brain Model Volume in the main window (-1 if invalid).
 */
int 
GuiMainWindow::getBrainModelIndex() const
{
   return mainOpenGL->getDisplayedBrainModelIndex();
}
      
/**
 * Called when an item is selected from the file:recent spec file menu
 */
void
GuiMainWindow::recentSpecFileMenuSelection(int menuItem)
{
   if (menuItem < static_cast<int>(recentSpecFiles.size())) {
      readSpecFile(recentSpecFiles[menuItem]);
   }
   else {
      PreferencesFile* pf = getBrainSet()->getPreferencesFile();
      recentSpecFiles.clear();
      pf->setRecentSpecFiles(recentSpecFiles);
      if (pf->getFileName().isEmpty() == false) {
         try {
            pf->writeFile(pf->getFileName());
         }
         catch(FileException& /*e*/) {
         }
      }
   }
}

/**
 * Popup viewing window
 */
void
GuiMainWindow::showViewingWindow(const BrainModel::BRAIN_MODEL_VIEW_NUMBER item)
{
   bool createdWindow = false;
   if (modelWindow[item] == NULL) {
      // passing mainOpenGL will used shared context between OpenGL renderers
      modelWindow[item] = new GuiBrainModelViewingWindow(this, this, item);
      createdWindow = true;
   }
   if (createdWindow) {
      modelWindow[item]->initializeToolBar();
      QtUtilities::positionWindowOffOtherWindow(this, modelWindow[item]);
      
      if (item >= BrainModel::BRAIN_MODEL_VIEW_AUX_WINDOW_3) {
         if (modelWindow[BrainModel::BRAIN_MODEL_VIEW_AUX_WINDOW_2] != NULL) {
            const QSize window2Size = 
               modelWindow[BrainModel::BRAIN_MODEL_VIEW_AUX_WINDOW_2]->size();
            modelWindow[item]->resize(window2Size);
            //modelWindow[item]->updateGeometry();
         }
      }
   }
   
   modelWindow[item]->show();
   modelWindow[item]->activateWindow();
}

/**
 * resize the viewing windows.
 */
void 
GuiMainWindow::resizeViewingWindows()
{
   //
   // Keep list of main and open viewing windows
   //
   QStringList windowNames;
   QList<QVariant> windowNumbers;
   windowNames.push_back("Main Window");
   windowNumbers.push_back(QVariant(BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW));
   
   //
   // Find open viewing windows
   //
   for (int i = BrainModel::BRAIN_MODEL_VIEW_AUX_WINDOW_2; 
        i < BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; i++) {
      const BrainModel::BRAIN_MODEL_VIEW_NUMBER windowNum = 
         static_cast<BrainModel::BRAIN_MODEL_VIEW_NUMBER>(i);
      if (modelWindow[windowNum] != NULL) {
         const QString str("Viewing Window "
                           + QString::number(windowNum + 1));
         windowNames   << str;
         windowNumbers << i;
      }
   }
   
   //
   // Is a viewing window open ?
   //
   if (windowNames.size() > 1) {
      //
      // Create dialog for choosing window 
      //
      WuQDataEntryDialog ded(this);
      ded.setTextAtTop("Choose the window whose size you would like matched.",
                       true);
      ded.setWindowTitle("Resize Viewing Windows");
      QComboBox* windowComboBox = ded.addComboBox("Resize to Match ",
                                                  windowNames,
                                                  &windowNumbers);
      if (ded.exec() == WuQDataEntryDialog::Accepted) {
         //
         // Get the selected window
         //
         QWidget* windowWidget = this;
         const int indx = windowComboBox->currentIndex();
         const BrainModel::BRAIN_MODEL_VIEW_NUMBER windowNum = 
            static_cast<BrainModel::BRAIN_MODEL_VIEW_NUMBER>(
               windowNumbers.at(indx).toInt());
         if (windowNum >= BrainModel::BRAIN_MODEL_VIEW_AUX_WINDOW_2) {
            windowWidget = modelWindow[windowNum];
         }
         
         //
         // New size for window
         //
         const QSize newWindowSize(windowWidget->size());
            
         //
         // Resize all viewing windows
         //
         for (int i = BrainModel::BRAIN_MODEL_VIEW_AUX_WINDOW_2; 
              i < BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; i++) {
            //
            // If not window resizing is based upon
            //
            if (i != windowNum) {
               if (modelWindow[i] != NULL) {
                  //
                  // New size for window
                  //
                  QSize newSize = newWindowSize;
                  
                  //
                  // If resizing to match the main window
                  //
                  if (windowNum == BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW) {
                     //
                     // Get size of main window openGL and
                     // viewing window and openGL
                     //
                     const QSize mainOpenGLSize = mainOpenGL->size();
                     const QSize viewingWindowOpenGLSize = 
                        modelWindow[i]->getBrainModelOpenGL()->size();
                     const int dH = mainOpenGLSize.height()
                                    - viewingWindowOpenGLSize.height();
                     const int dW = mainOpenGLSize.width()
                                    - viewingWindowOpenGLSize.width();
                                    
                     //
                     // Proportionately change window size
                     //
                     const QSize currentSize(modelWindow[i]->size());
                     newSize.setHeight(currentSize.height() + dH);
                     newSize.setWidth(currentSize.width() + dW);
                  }
            
                  //
                  // Update the windows size
                  //
                  modelWindow[i]->resize(newSize);
                  modelWindow[i]->updateGeometry();
               }
            }
         }
      }
   }
}
      
/**
 * Destroy a viewing window
 */
void
GuiMainWindow::removeViewingWindow(const BrainModel::BRAIN_MODEL_VIEW_NUMBER item)
{
   modelWindow[item] = NULL;
}

/**
 * display the models editor.
 */
void 
GuiMainWindow::displayModelsEditorDialog()
{
   if (modelsEditorDialog == NULL) {
      modelsEditorDialog = new GuiModelsEditorDialog(this);
   }
   modelsEditorDialog->show();
   modelsEditorDialog->activateWindow();
}
      
/**
 * create (if necessary) and show the border operations dialog.
 */
void 
GuiMainWindow::displayBorderOperationsDialog()
{
   getBorderOperationsDialog(true);
}

/**
 * create (if necessary) and show the flatten full hemisphere dialog.
 */
void 
GuiMainWindow::displayFlattenFullHemisphereDialog()
{
   getFlattenFullHemisphereDialog(true);
}
      
/**
 * create (if necessary) and show the standard mesh dialog.
 */
void 
GuiMainWindow::displayStandardMeshDialog()
{
   getStandardMeshDialog(true);
}
      
/**
 * Create (if necessary) and show the draw border dialog.
 */
void
GuiMainWindow::displayDrawBorderDialog()
{
   getDrawBorderDialog(true);
}

/**
 * create and display the palette editor dialog.
 */
void 
GuiMainWindow::displayPaletteEditorDialog()
{
   if (paletteEditorDialog == NULL) {
      paletteEditorDialog = new GuiPaletteEditorDialog(this);
   }
   paletteEditorDialog->show();
   paletteEditorDialog->activateWindow();
}
      
/**
 * Create, possibly show, and return the draw border dialog.
 */
GuiDrawBorderDialog*
GuiMainWindow::getDrawBorderDialog(const bool showIt)
{
   bool firstTime = false;
   if (drawBorderDialog == NULL) {
      firstTime = true;
      drawBorderDialog = new GuiDrawBorderDialog(this);
   }
   if (showIt) {
      drawBorderDialog->show();
      drawBorderDialog->activateWindow();
   }
   if (firstTime) {
      QtUtilities::positionWindowOffOtherWindow(this, drawBorderDialog);
   }
   return drawBorderDialog;
}

/**
 * create, possibly show, and return the border operations dialog.
 */
GuiBorderOperationsDialog* 
GuiMainWindow::getBorderOperationsDialog(const bool showIt)
{
   bool firstTime = false;
   if (borderOperationsDialog == NULL) {
      firstTime = true;
      borderOperationsDialog = new GuiBorderOperationsDialog(this);
   }
   if (showIt) {
      borderOperationsDialog->show();
      borderOperationsDialog->activateWindow();
   }
   if (firstTime) {
      QtUtilities::positionWindowOffOtherWindow(this, borderOperationsDialog);
   }
   return borderOperationsDialog;
}

/**
 * create, possibly show, and return the flatten full hemisphere dialog.
 */
GuiFlattenFullHemisphereDialog* 
GuiMainWindow::getFlattenFullHemisphereDialog(const bool showIt)
{
   bool firstTime = false;
   if (flattenFullHemisphereDialog == NULL) {
      firstTime = true;
      flattenFullHemisphereDialog = new GuiFlattenFullHemisphereDialog(this);
   }
   if (showIt) {
      flattenFullHemisphereDialog->showFirstPage();
      flattenFullHemisphereDialog->show();
      flattenFullHemisphereDialog->activateWindow();
   }
   if (firstTime) {
      QtUtilities::positionWindowOffOtherWindow(this, flattenFullHemisphereDialog);
   }
   return flattenFullHemisphereDialog;
}      
      
/**
 * create, possibly show, and return the standard mesh dialog.
 */
GuiStandardMeshDialog* 
GuiMainWindow::getStandardMeshDialog(const bool showIt)
{
   bool firstTime = false;
   if (standardMeshDialog == NULL) {
      firstTime = true;
      standardMeshDialog = new GuiStandardMeshDialog(this);
   }
   if (showIt) {
      standardMeshDialog->showFirstPage();
      standardMeshDialog->show();
      standardMeshDialog->activateWindow();
   }
   if (firstTime) {
      QtUtilities::positionWindowOffOtherWindow(this, standardMeshDialog);
   }
   return standardMeshDialog;
}
      
/**
 *
 */
void
GuiMainWindow::displaySmoothingDialog()
{
   if (smoothingDialog == NULL) {
      smoothingDialog = new GuiSmoothingDialog(this, false, true, NULL);
   }
   smoothingDialog->show();
   smoothingDialog->activateWindow();
}

/**
 * Create, possibly show, and return the recording dialog
 */
GuiRecordingDialog*
GuiMainWindow::getRecordingDialog(const bool showIt)
{
   if (recordingDialog == NULL) {
      recordingDialog = new GuiRecordingDialog(this);
   }
   if (showIt) {
      recordingDialog->show();
      recordingDialog->activateWindow();
   }
   return recordingDialog;
}


/**
 * Create, possibly show, and return the draw cells dialog
 */
GuiAddCellsDialog*
GuiMainWindow::getAddCellsDialog(const bool showIt)
{
   if (addCellsDialog == NULL) {
      addCellsDialog = new GuiAddCellsDialog(GuiAddCellsDialog::DIALOG_MODE_ADD_CELLS, this);
   }
   if (showIt) {
      addCellsDialog->show();
      addCellsDialog->activateWindow();
   }
   return addCellsDialog;
}

/**
 * Create, possibly show, and return the draw cells dialog
 */
GuiAddCellsDialog*
GuiMainWindow::getAddContourCellsDialog(const bool showIt)
{
   if (addContourCellsDialog == NULL) {
      addContourCellsDialog = new GuiAddCellsDialog(GuiAddCellsDialog::DIALOG_MODE_ADD_CONTOUR_CELLS,
                                                    this);
   }
   if (showIt) {
      addContourCellsDialog->show();
      addContourCellsDialog->activateWindow();
   }
   return addContourCellsDialog;
}

/**
 * Create, possibly show, and return the contour alignment dialog.
 */
GuiContourAlignmentDialog*
GuiMainWindow::getContourAlignmentDialog(const bool showIt)
{
   if (contourAlignmentDialog == NULL) {
     contourAlignmentDialog  = new GuiContourAlignmentDialog(this);
   }
   if (showIt) {
      contourAlignmentDialog->show();
      contourAlignmentDialog->activateWindow();
   }
   return contourAlignmentDialog;
}

/**
 * Create, possibly show, and return the draw contour dialog.
 */
GuiContourDrawDialog*
GuiMainWindow::getContourDrawDialog(const bool showIt)
{
   if (contourDrawDialog == NULL) {
      contourDrawDialog = new GuiContourDrawDialog(this);
   }
   if (showIt) {
      contourDrawDialog->show();
      contourDrawDialog->activateWindow();
   }
   return contourDrawDialog;
}

/**
 * Create, possibly show, and return the set scale dialog.
 */
GuiContourSetScaleDialog*
GuiMainWindow::getContourSetScaleDialog(const bool showIt)
{
   if (contourSetScaleDialog == NULL) {
      contourSetScaleDialog = new GuiContourSetScaleDialog(this);
   }
   if (showIt) {
      contourSetScaleDialog->show();
      contourSetScaleDialog->activateWindow();
   }
   return contourSetScaleDialog;
}


/**
 * Create, possibly show, and return the contour section dialog.
 */
GuiContourSectionControlDialog*
GuiMainWindow::getContourSectionControlDialog(const bool showIt)
{
   if (contourSectionControlDialog == NULL) {
      contourSectionControlDialog = new GuiContourSectionControlDialog(this);
   }
   if (showIt) {
      contourSectionControlDialog->show();
      contourSectionControlDialog->activateWindow();
   }
   return contourSectionControlDialog;
}

/**
 * Create, possiblty show and return the map stereotaxic focus dialog
 */
GuiMapStereotaxicFocusDialog* 
GuiMainWindow::getMapStereotaxicFocusDialog(const bool showIt)
{
   if (mapStereotaxicFocusDialog == NULL) {
      mapStereotaxicFocusDialog = new GuiMapStereotaxicFocusDialog(this);
   }
   if (showIt) {
      mapStereotaxicFocusDialog->show();
      mapStereotaxicFocusDialog->activateWindow();
   }
   return mapStereotaxicFocusDialog;
}

/**
 * Display the metric math dialog.
 */
void
GuiMainWindow::displayMetricMathDialog()
{
   if (metricMathDialog == NULL) {
      metricMathDialog = new GuiDataFileMathDialog(this, 
                                             GuiDataFileMathDialog::DIALOG_MODE_METRIC_FILE);
   }
   metricMathDialog->show();
   metricMathDialog->activateWindow();
}

/**
 * Display the shape math dialog.
 */
void
GuiMainWindow::displayShapeMathDialog()
{
   if (shapeMathDialog == NULL) {
      shapeMathDialog = new GuiDataFileMathDialog(this,
                                       GuiDataFileMathDialog::DIALOG_MODE_SURFACE_SHAPE_FILE);
   }
   shapeMathDialog->show();
   shapeMathDialog->activateWindow();
}

/**
 * Display the volume math dialog.
 */
void
GuiMainWindow::displayVolumeMathDialog()
{
   if (volumeMathDialog == NULL) {
      volumeMathDialog = new GuiDataFileMathDialog(this, 
                                             GuiDataFileMathDialog::DIALOG_MODE_VOLUME_FILE);
   }
   volumeMathDialog->show();
   volumeMathDialog->activateWindow();
}

/**
 * Create, possibly show and return the metrics to rgb paint dialog.
 */
GuiMetricsToRgbPaintDialog*
GuiMainWindow::getMetricsToRgbPaintDialog(const bool showIt)
{
   if (metricsToRgbPaintDialog == NULL) {
      metricsToRgbPaintDialog = new GuiMetricsToRgbPaintDialog(this);
   }
   if (showIt) {
      metricsToRgbPaintDialog->show();
      metricsToRgbPaintDialog->activateWindow();
   }
   return metricsToRgbPaintDialog;
}

/**
 * Create, possibly show and return the surface region of interest dialog.
 */
GuiSurfaceRegionOfInterestDialog*
GuiMainWindow::getSurfaceRegionOfInterestDialog(const bool showIt)
{
   if (surfaceRegionOfInterestDialog == NULL) {
      surfaceRegionOfInterestDialog = new GuiSurfaceRegionOfInterestDialog(this);
   }
   if (showIt) {
      surfaceRegionOfInterestDialog->show();
      surfaceRegionOfInterestDialog->activateWindow();
   }
   return surfaceRegionOfInterestDialog;
}

/**
 * Create, possibly show and return the surface region of interest dialog.
 */
GuiSurfaceRegionOfInterestDialogOLD*
GuiMainWindow::getSurfaceRegionOfInterestDialogOLD(const bool showIt)
{
   if (surfaceRegionOfInterestDialogOLD == NULL) {
      surfaceRegionOfInterestDialogOLD = new GuiSurfaceRegionOfInterestDialogOLD(this);
   }
   if (showIt) {
      surfaceRegionOfInterestDialogOLD->show();
      surfaceRegionOfInterestDialogOLD->activateWindow();
   }
   return surfaceRegionOfInterestDialogOLD;
}

/**
 * Create, possibly show and return the volume region of interest dialog.
 */
GuiVolumeRegionOfInterestDialog*
GuiMainWindow::getVolumeRegionOfInterestDialog(const bool showIt)
{
   if (volumeRegionOfInterestDialog == NULL) {
      volumeRegionOfInterestDialog = new GuiVolumeRegionOfInterestDialog(this);
   }
   if (showIt) {
      volumeRegionOfInterestDialog->show();
      volumeRegionOfInterestDialog->activateWindow();
   }
   return volumeRegionOfInterestDialog;
}

/**
 * Create, possibly show and return the volume threshold segmentation dialog.
 */
GuiVolumeThresholdSegmentationDialog*
GuiMainWindow::getVolumeThresholdSegmentationDialog(const bool showIt)
{
   if (volumeThresholdSegmentationDialog == NULL) {
      volumeThresholdSegmentationDialog = new GuiVolumeThresholdSegmentationDialog(this);
   }
   if (showIt) {
      volumeThresholdSegmentationDialog->show();
      volumeThresholdSegmentationDialog->activateWindow();
   }
   return volumeThresholdSegmentationDialog;
}

/**
 * Create, possibly show and return the metric modification dialog.
 */
GuiMetricModificationDialog*
GuiMainWindow::getMetricModificationDialog(const bool showIt)
{
   if (metricModificationDialog == NULL) {
      metricModificationDialog = new GuiMetricModificationDialog(this,
                                        GuiMetricModificationDialog::FILE_TYPE_MODE_METRIC);
   }
   if (showIt) {
      metricModificationDialog->show();
      metricModificationDialog->activateWindow();
   }
   return metricModificationDialog;
}

/**
 * Create, possibly show and return the shape modification dialog.
 */
GuiMetricModificationDialog*
GuiMainWindow::getShapeModificationDialog(const bool showIt)
{
   if (shapeModificationDialog == NULL) {
      shapeModificationDialog = new GuiMetricModificationDialog(this,
                            GuiMetricModificationDialog::FILE_TYPE_MODE_SURFACE_SHAPE);
   }
   if (showIt) {
      shapeModificationDialog->show();
      shapeModificationDialog->activateWindow();
   }
   return shapeModificationDialog;
}

/**
 * Create, possibly show and return the electrode trajectory tool dialog.
 */
GuiTrajectoryToolDialog*
GuiMainWindow::getTrajectoryToolDialog(bool showIt)
{
   if (trajectoryToolDialog == NULL) {
      trajectoryToolDialog = new GuiTrajectoryToolDialog(this, getBrainSet());
   }
   if (showIt)
   {
	   trajectoryToolDialog->show();
	   trajectoryToolDialog->activateWindow();
   }
   return trajectoryToolDialog;
}


void GuiMainWindow::displayTrajectoryToolDialog()
{
	getTrajectoryToolDialog(true);
}

/**
 * Show the flat morphing dialog
 */
void
GuiMainWindow::showFlatMorphingDialog()
{
   if (flatMorphingDialog == NULL) {
      flatMorphingDialog = new GuiMorphingDialog(this,
                                            BrainModelSurfaceMorphing::MORPHING_SURFACE_FLAT);
   }
   flatMorphingDialog->updateDialog();
   flatMorphingDialog->show();
   flatMorphingDialog->activateWindow();
}

/**
 * Show the sphere morphing dialog
 */
void
GuiMainWindow::showSphereMorphingDialog()
{
   if (sphereMorphingDialog == NULL) {
      sphereMorphingDialog = new GuiMorphingDialog(this,
                                    BrainModelSurfaceMorphing::MORPHING_SURFACE_SPHERICAL);
   }
   sphereMorphingDialog->updateDialog();
   sphereMorphingDialog->show();
   sphereMorphingDialog->activateWindow();
}

/**
 * Create, possibly show and return the interpolate surface dialog.
 */
GuiInterpolateSurfacesDialog*
GuiMainWindow::getInterpolateSurfaceDialog(const bool showIt)
{
   if (interpolateSurfaceDialog == NULL) {
      interpolateSurfaceDialog = new GuiInterpolateSurfacesDialog(this);
   }
   if (showIt) {
      interpolateSurfaceDialog->show();
      interpolateSurfaceDialog->activateWindow();
   }
   return interpolateSurfaceDialog;
}

/**
 * Show a page in the help viewer dialog.  If the page name is empty the default page is shown.
 */
void
GuiMainWindow::showHelpViewerDialog(const QString& helpPage)
{
   //
   // See if external web page
   //
   if (helpPage.startsWith("http://")) {
      displayWebPage(helpPage);
   }
   else {
      if (helpViewerDialog == NULL) {
         helpViewerDialog = new GuiHelpViewerWindow(this);
      }
      helpViewerDialog->loadPage(helpPage);
      helpViewerDialog->show();
      helpViewerDialog->activateWindow();
   }
}

/**
 * show a page in the help viewer dialog over a modal dialog.
 */
void 
GuiMainWindow::showHelpPageOverModalDialog(QDialog* modalParent,
                                         const QString& helpPage)
{
   GuiHelpViewerWindow hvw(modalParent, helpPage);
   hvw.exec();
}

/**
 * create, (possibly show), and return the border created interpolated dialog.
 */
GuiBordersCreateInterpolatedDialog* 
GuiMainWindow::getBordersCreateInterpolatedDialog(const bool showIt)
{
   if (bordersCreateInterpolatedDialog == NULL) {
      bordersCreateInterpolatedDialog = new GuiBordersCreateInterpolatedDialog(this);
   }
   if (showIt) {
      bordersCreateInterpolatedDialog->show();
      bordersCreateInterpolatedDialog->raise();
   }
   return bordersCreateInterpolatedDialog;
}

/**
 * Create, possibly show and return the identify dialog
 */
GuiIdentifyDialog* 
GuiMainWindow::getIdentifyDialog(const bool showIt)
{
   bool firstTime = false;
   if (identifyDialog == NULL) {
      identifyDialog = new GuiIdentifyDialog(this);
      firstTime = true;
   }
   if (showIt) {
      identifyDialog->show();
      if (firstTime) {
         QtUtilities::positionWindowOffOtherWindow(this, identifyDialog);
      }
      identifyDialog->raise();
      //identifyDialog->activateWindow();
   }
   return identifyDialog;
}

/**
 * Create (if necessary) and show the identify dialog
 */
void 
GuiMainWindow::displayIdentifyDialog()
{
   getIdentifyDialog(true);
}

/**
 * Create, possibly show and return the volume attributes dialog
 */
GuiVolumeAttributesDialog* 
GuiMainWindow::getVolumeAttributesDialog(const bool showIt)
{
   if (volumeAttributesDialog == NULL) {
      volumeAttributesDialog = new GuiVolumeAttributesDialog(this);
   }
   if (showIt) {
      volumeAttributesDialog->show();
      volumeAttributesDialog->raise();
      volumeAttributesDialog->activateWindow();
   }
   return volumeAttributesDialog;
}

/**
 * Create, possibly show and return the volume segmentation editor dialog
 */
GuiVolumeSegmentationEditorDialog* 
GuiMainWindow::getVolumeSegmentationEditorDialog(const bool showIt)
{
   if (volumeSegmentationEditorDialog == NULL) {
      volumeSegmentationEditorDialog = new GuiVolumeSegmentationEditorDialog(this);
   }
   if (showIt) {
      volumeSegmentationEditorDialog->show();
      volumeSegmentationEditorDialog->raise();
      volumeSegmentationEditorDialog->activateWindow();
   }
   return volumeSegmentationEditorDialog;
}

/**
 * Create, possibly show and return the volume paint editor dialog
 */
GuiVolumePaintEditorDialog* 
GuiMainWindow::getVolumePaintEditorDialog(const bool showIt)
{
   if (volumePaintEditorDialog == NULL) {
      volumePaintEditorDialog = new GuiVolumePaintEditorDialog(this);
   }
   if (showIt) {
      volumePaintEditorDialog->show();
      volumePaintEditorDialog->activateWindow();
   }
   return volumePaintEditorDialog;
}

/**
 * display the study meta data file editor dialog.
 */
void 
GuiMainWindow::displayStudyMetaDataFileEditorDialog()
{
   if (studyMetaDataFileEditorDialog == NULL) {
      studyMetaDataFileEditorDialog = new GuiStudyMetaDataFileEditorDialog(this);
   }
   studyMetaDataFileEditorDialog->show();
   studyMetaDataFileEditorDialog->activateWindow();
}
      
/**
 * display the study collection file editor dialog.
 */
void 
GuiMainWindow::displayStudyCollectionFileEditorDialog()
{
   if (studyCollectionFileEditorDialog == NULL) {
      studyCollectionFileEditorDialog = new GuiStudyCollectionFileEditorDialog(this);
   }
   studyCollectionFileEditorDialog->show();
   studyCollectionFileEditorDialog->activateWindow();
}
      
/**
 * Return the volume resizing dialog.  Dialog will be created only if parameter is true
 */
GuiVolumeResizingDialog* 
GuiMainWindow::getVolumeResizingDialog(const bool createIt)
{
   if (createIt) {
      if (volumeResizingDialog == NULL) {
         volumeResizingDialog = new GuiVolumeResizingDialog(this);
      }
      volumeResizingDialog->show();
      volumeResizingDialog->activateWindow();
   }
   return volumeResizingDialog;
}

/**
 * Create (if necessary) and display the preferences dialog
 */
void
GuiMainWindow::displayPreferencesDialog()
{
   if (preferencesDialog == NULL) {
      preferencesDialog = new GuiPreferencesDialog(this);
   }
   preferencesDialog->show();
   preferencesDialog->raise();
   preferencesDialog->activateWindow();
}

/**
 * display the paint editor dialog.
 */
void 
GuiMainWindow::displayPaintEditorDialog()
{
   if (paintNameEditorDialog == NULL) {
      paintNameEditorDialog = new GuiPaintNameEditorDialog(this);
   }
   paintNameEditorDialog->show();
   paintNameEditorDialog->raise();
   paintNameEditorDialog->activateWindow();
}
      
/**
 * Called when Toolbar "Spec" button is pressed.
 */
void
GuiMainWindow::displayFastOpenDataFileDialog()
{
   const QString specFileName(getBrainSet()->getSpecFileName());
   if (specFileName.isEmpty()) {
      QMessageBox::critical(this, "No Spec File", "There is no spec file loaded.");
      return;
   }
   
   //
   // Read the spec file.
   //
   SpecFile sf;   
   try {
      sf.readFile(specFileName);
      sf.setDefaultFilesFiducialAndFlat();
   }
   catch (FileException& e) {
      QMessageBox::critical(this, 
                            "Error reading spec file",
                            e.whatQString());
      return;
   }

   GuiSpecFileDialog* gsfd = new GuiSpecFileDialog(this, sf, 
                                      GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_DATA_FILE);
   gsfd->show();
   gsfd->raise();
}

/**
 * Create, display, and return the overlay underlay dialog.
 */ 
void
GuiMainWindow::displayDisplayControlDialog()
{
   bool firstTime = false;
   if (displayControlDialog == NULL) {
       displayControlDialog = new GuiDisplayControlDialog(this);
       firstTime = true;
   }
   displayControlDialog->show();
   displayControlDialog->raise();
   displayControlDialog->activateWindow();
   if (firstTime) {
      QtUtilities::positionWindowOffOtherWindow(this, displayControlDialog);
   }
}

/**
 * Update the display control dialog
 */
void
GuiMainWindow::updateDisplayControlDialog()
{
   if (displayControlDialog != NULL) {
      displayControlDialog->updateAllItemsInDialog(true, false);
   }
}

/**
 * Create (if necessary) and display the caret command executor dialog.
 */
void
GuiMainWindow::displayCaretCommandExecutorDialog()
{
   if (caretCommandExecutorDialog == NULL) {
      caretCommandExecutorDialog = new GuiCaretCommandDialog(this,
                                          getBrainSet()->getCaretHomeDirectory(),
                                          GuiCaretCommandDialog::DIALOG_MODE_EXECUTOR_NON_MODAL);
   }
   caretCommandExecutorDialog->show();
   caretCommandExecutorDialog->activateWindow();
}

/**
 * Create (if necessary) and display the caret command script builder dialog.
 */
void
GuiMainWindow::displayCaretCommandScriptBuilderDialog()
{
   if (caretCommandScriptBuilderDialog == NULL) {
      caretCommandScriptBuilderDialog = new GuiCaretCommandScriptBuilderDialog(this,
                                          getBrainSet()->getCaretHomeDirectory());
   }
   caretCommandScriptBuilderDialog->show();
   caretCommandScriptBuilderDialog->activateWindow();
}

/**
 * Create (if necessary) and display the section control dialog.
 */
void
GuiMainWindow::displaySectionControlDialog()
{
   if (sectionControlDialog == NULL) {
      sectionControlDialog = new GuiSectionControlDialog(this);
   }
   sectionControlDialog->show();
   sectionControlDialog->activateWindow();
}

/**
 * Create (if necessary) and display the automatic rotation dialog.
 */
void 
GuiMainWindow::displayAutomaticRotationDialog()
{
   if (automaticRotationDialog == NULL) {
      automaticRotationDialog = new GuiAutomaticRotationDialog(this);
   }
   automaticRotationDialog->show();
   automaticRotationDialog->activateWindow();
}

/**
 *  Update the section control dialog.
 */
void 
GuiMainWindow::updateSectionControlDialog()
{
   if (sectionControlDialog != NULL) {
      sectionControlDialog->updateDialog();
   }
}

/**
 * See if a data file has been modified before quittting program.
 */
void
GuiMainWindow::checkFileModified(const QString typeName,
                                 const AbstractFile* af, QString& msg)
{
   if (af != NULL) {
      if (af->getModified() != 0) {
         msg.append("   ");
         msg.append(typeName);
         msg.append(" ");
         if (af->getFileName().isEmpty()) {
            msg.append("No-Name");
         }
         else {
            msg.append(FileUtilities::basename(af->getFileName()));
         }
         msg.append("\n");
      }
   }
}

/**
 * See if a volume file is modified.
 */
void
GuiMainWindow::checkVolumeFileModified(const QString& typeName,
                                       const VolumeFile* vf, QString& msg)
{
   if (vf != NULL) {
      if (vf->getModified()) {
         msg.append(typeName);
         msg.append("   Volume File - ");
         if (vf->getFileName().isEmpty()) {
            msg.append("No-Name");
         }
         else {
            msg.append(FileUtilities::basename(vf->getFileName()));
         }
         msg.append("\n");
      }
   }
}

/**
 * Check for modified files.
 */
void
GuiMainWindow::checkForModifiedFiles(BrainSet* bs,
                                     QString& msgOut, const bool checkSceneFileFlag)
{
   QString msg;
   
   for (int i = 0; i < bs->getNumberOfTopologyFiles(); i++) {
      TopologyFile* tf = bs->getTopologyFile(i);
      QString s("Topology File - ");
      s.append(tf->getTopologyTypeName());
      checkFileModified(s, tf, msg);
   }   
   const int numBrainModels = bs->getNumberOfBrainModels();
   for (int i = 0; i < numBrainModels; i++) {
      BrainModel* bm = bs->getBrainModel(i);
      switch(bm->getModelType()) {
         case BrainModel::BRAIN_MODEL_CONTOURS:
            {
               BrainModelContours* bmc = dynamic_cast<BrainModelContours*>(bm);
               QString s("Contour File - ");
               ContourFile* cf = bmc->getContourFile();
               checkFileModified(s, cf, msg); 
            }
            break;
         case BrainModel::BRAIN_MODEL_SURFACE:
            {
               BrainModelSurface* bms = dynamic_cast<BrainModelSurface*>(bm);
               QString s("Coordinate File - ");
               CoordinateFile* cf = bms->getCoordinateFile();
               s.append(bms->getSurfaceTypeName());
               checkFileModified(s, cf, msg);
            }
            break;
         case BrainModel::BRAIN_MODEL_VOLUME:
            break;
         case BrainModel::BRAIN_MODEL_SURFACE_AND_VOLUME:
            break;
      }
   }   

   for (int i = 0; i < bs->getNumberOfVolumeAnatomyFiles(); i++) {
      checkVolumeFileModified("Anatomy", bs->getVolumeAnatomyFile(i), msg);
   }
   for (int i = 0; i < bs->getNumberOfVolumeFunctionalFiles(); i++) {
      checkVolumeFileModified("Functional", bs->getVolumeFunctionalFile(i), msg);
   }
   for (int i = 0; i < bs->getNumberOfVolumePaintFiles(); i++) {
      checkVolumeFileModified("Paint", bs->getVolumePaintFile(i), msg);
   }
   for (int i = 0; i < bs->getNumberOfVolumeRgbFiles(); i++) {
      checkVolumeFileModified("RGB", bs->getVolumeRgbFile(i), msg);
   }
   for (int i = 0; i < bs->getNumberOfVolumeSegmentationFiles(); i++) {
      checkVolumeFileModified("Segmentation", bs->getVolumeSegmentationFile(i), msg);
   }
   for (int i = 0; i < bs->getNumberOfVolumeVectorFiles(); i++) {
      checkVolumeFileModified("Vector", bs->getVolumeVectorFile(i), msg);
   }

   checkFileModified("Area Color File", bs->getAreaColorFile(), msg);
   checkFileModified("Areal Estimation File", bs->getArealEstimationFile(), msg);
   
   BrainModelBorderSet* bmbs = bs->getBorderSet();
   for (int i = 0; i < numBrainModels; i++) {
      const BrainModelSurface* bms = bs->getBrainModelSurface(i);
      if (bms != NULL) {
         if (bmbs->getSurfaceBordersModified(bms)) {         
            msg.append("Border File for Surface ");
            msg.append(" ");
            const CoordinateFile* cf = bms->getCoordinateFile();
            if (cf->getFileName().isEmpty()) {
               msg.append("No-Name");
            }
            else {
               msg.append(FileUtilities::basename(cf->getFileName()));
            }
            msg.append("\n");
         }
      }
   }
   checkFileModified("Border File (Volume)", bs->getVolumeBorderFile(), msg);
   checkFileModified("Border Color File", bs->getBorderColorFile(), msg);
   if (bmbs->getProjectionsModified()) {
      msg.append("Border Projection File\n");
   }
   checkFileModified("Cell Color File", bs->getCellColorFile(), msg);
   checkFileModified("Cell Projection File", bs->getCellProjectionFile(), msg);
   checkFileModified("CoCoMac File", bs->getCocomacFile(), msg);
   checkFileModified("Contour Cells File", bs->getContourCellFile(), msg);
   checkFileModified("Contour Cell Color File", bs->getContourCellColorFile(), msg);
   checkFileModified("Cuts File", bs->getCutsFile(), msg);
   checkFileModified("Deformation Field File", bs->getDeformationFieldFile(), msg);
   checkFileModified("Foci Color File", bs->getFociColorFile(), msg);
   checkFileModified("Foci Projection File", bs->getFociProjectionFile(), msg);
   checkFileModified("Foci Search File", bs->getFociSearchFile(), msg);
   checkFileModified("Geodesic Distance File", bs->getGeodesicDistanceFile(), msg);
   for (int i = 0; i < bs->getNumberOfImageFiles(); i++) {
      checkFileModified("Image File", bs->getImageFile(i), msg);
   }
   checkFileModified("Lat/Lon File", bs->getLatLonFile(), msg);
   checkFileModified("Metric File", bs->getMetricFile(), msg);
   checkFileModified("Paint File", bs->getPaintFile(), msg);
   checkFileModified("Params File", bs->getParamsFile(), msg);
   checkFileModified("Probabilistic Atlas File", bs->getProbabilisticAtlasSurfaceFile(), msg);
   checkFileModified("Palette File", bs->getPaletteFile(), msg);
   checkFileModified("RGB Paint File", bs->getRgbPaintFile(), msg);
   if (checkSceneFileFlag) {
      checkFileModified("Scene File", bs->getSceneFile(), msg);
   }
   checkFileModified("Section File", bs->getSectionFile(), msg);
   checkFileModified("Study Collection File", bs->getStudyCollectionFile(), msg);
   checkFileModified("Study Metadata File", bs->getStudyMetaDataFile(), msg);
   checkFileModified("Surface Shape File", bs->getSurfaceShapeFile(), msg);
   checkFileModified("Topography File", bs->getTopographyFile(), msg);
   checkFileModified("Trajectory File", bs->getTrajectoryFile(), msg);
   checkFileModified("Vocabulary File", bs->getVocabularyFile(), msg);
   checkFileModified("Transformation Matrix File", bs->getTransformationMatrixFile(), msg);
   for (int i = 0; i < bs->getNumberOfVectorFiles(); i++) {
      checkFileModified("Vector File", bs->getVectorFile(i), msg);
   }
   for (int i = 0; i < bs->getNumberOfVtkModelFiles(); i++) {
      checkFileModified("VTK Model File", bs->getVtkModelFile(i), msg);
   }
   checkFileModified("Wustl Region File", bs->getWustlRegionFile(), msg);
   
   //checkFileModified(" File", bs->(), msg);
   
   if (msg.isEmpty() == false) {
      msgOut.append(FileUtilities::basename(bs->getSpecFileName()));
      msgOut.append(" has modified files:\n");
      msgOut.append(msg);
      msgOut.append("\n");
   }
}

/**
 * Close the current spec file.
 */
void
GuiMainWindow::slotCloseSpecFile() 
{
   closeSpecFile(false);
}

/**
 * Close the current spec file but keep the scene and spec file.
 */
void
GuiMainWindow::slotCloseSpecFileKeepSceneAndSpec() 
{
   closeSpecFile(true);
}

/**
 * close the spec file.
 */
void 
GuiMainWindow::closeSpecFile(const bool keepSceneAndSpec,
                             const bool checkForModifiedFilesFlag)
{
   //
   // See if files are modified
   //
   bool closeSpecFileFlag = false;
   if (checkForModifiedFilesFlag) {
      QString msg;
      checkForModifiedFiles(getBrainSet(), msg, true);
      if (msg.isEmpty() == false) {
         //
         // Return value of zero is YES button.
         //
         QString msg2;
         if (keepSceneAndSpec) {
            msg2 = "Are you sure that you want to unload all files except\n"
                        "for the spec file and the scenes file?\n\n";
         }
         else { 
            msg2 = "Are you sure that you want to close the spec file ?\n\n";
         }
         msg2.append(msg);
         
         if (WuQMessageBox::warning(this, 
                                 "WARNING",
                                 msg2, 
                                 (QMessageBox::Yes | QMessageBox::No),
                                 QMessageBox::Yes)
                                    == QMessageBox::Yes) {
            closeSpecFileFlag = true;
         }
      }
      else { 
         QString msg2;  
         if (keepSceneAndSpec) {
            msg2 = "Are you sure that you want to unload all files \n"
                   "except for the spec file and the scenes file?";
         }
         else {
            msg2 = "Are you sure you want to close the spec file ?";
         }

         //
         // Return value of zero is YES button.
         //
         if (WuQMessageBox::warning(this, 
                                 "WARNING",
                                 msg2, 
                                 (QMessageBox::Yes | QMessageBox::No),
                                 QMessageBox::Yes)
                                    == QMessageBox::Yes) {
            closeSpecFileFlag = true;
         }
      }
   }
   else {
      closeSpecFileFlag = true;
   }
      
   if (closeSpecFileFlag) {
      const int numberOfSpecFilesLoaded = loadedBrainSets.size();
      const QString specFileName(getBrainSet()->getSpecFileName());
      getBrainSet()->reset(keepSceneAndSpec);
      if (keepSceneAndSpec) {
         getBrainSet()->setSpecFileName(specFileName);
      }
      else{
         getBrainSet()->setSpecFileName("");
         getBrainSet()->setStructure(Structure::STRUCTURE_TYPE_INVALID);
         if (loadedBrainSets.size() > 1) {
            //
            // Delete the brain set
            //
            for (unsigned int i = 0; i < loadedBrainSets.size(); i++) {
               if (loadedBrainSets[i] == getBrainSet()) {
                  delete loadedBrainSets[i];
                  loadedBrainSets.erase(loadedBrainSets.begin() + i);
                  //loadedBrainSetDirectory.erase(loadedBrainSetDirectory.begin() + i);
                  break;
               }
            }
            setBrainSet(BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW, loadedBrainSets[0]);
            //QDir::setCurrent(loadedBrainSetDirectory[0]);
            QDir::setCurrent(FileUtilities::dirname(loadedBrainSets[0]->getSpecFileName()));
         }
      }
      if (numberOfSpecFilesLoaded > 1) {
         removeAllImageViewingWindows();
         removeAllModelViewingWindows();
      }
      GuiFilesModified fm;
      fm.setStatusForAll(true);
      fileModificationUpdate(fm);
      GuiBrainModelOpenGL::updateAllGL();
   }
}

/**
 * close all spec files.
 */
void 
GuiMainWindow::closeAllSpecFiles()
{
   bool keepClosing = true;
   while (keepClosing) {
      keepClosing = (getNumberOfBrainSets() > 1);
      closeSpecFile(false, false);
   }
}      

/**
 * Called when the program should close.
 */
void 
GuiMainWindow::closeEvent(QCloseEvent* event)
{
   slotCloseProgram();
   event->ignore();
}

/**
 * close the program.
 */
void 
GuiMainWindow::slotCloseProgram()
{
   QString msg;

   //
   // See if files are modified
   //
   for (unsigned int i = 0; i < loadedBrainSets.size(); i++) {
      checkForModifiedFiles(loadedBrainSets[i], msg, true);
   }
   if (msg.isEmpty() == false) {
      //
      // Return value of zero is YES button.
      //
      QString msg2("Are you sure that you want to quit?\n"
                   "Changes to these files will be lost:\n\n");
      msg2.append(msg);
      
      WuQDataEntryDialog warningDialog(this);
      warningDialog.setWindowTitle("Caret 5");
      warningDialog.addTextEdit("", msg2, true);
      if (warningDialog.exec() == WuQDataEntryDialog::Accepted) {
         qApp->quit();
      }
/*
      if (QMessageBox::warning(this, 
                              "Caret 5",
                              msg2, 
                              (QMessageBox::Ok | QMessageBox::Cancel),
                              QMessageBox::Cancel)
                                  == QMessageBox::Ok) {
         //speakText("Goodbye carrot user.");
         qApp->quit();
      }
*/
   }
   else {   
      //
      // Return value of zero is YES button.
      //
      if (QMessageBox::warning(this, 
                              "Caret 5",
                              "Are you sure you want to quit ?", 
                              (QMessageBox::Yes | QMessageBox::No),
                              QMessageBox::Yes)
                                 == QMessageBox::Yes) {
         //speakText("Goodbye carrot user.");
         qApp->quit();
      }
   }
}
      
/**
 * Add spec file name to preference's recent spec files if
 * the file is not already in the list
 */
void
GuiMainWindow::addToRecentSpecFiles(const QString& name)
{
   PreferencesFile* pf = getBrainSet()->getPreferencesFile();
   pf->addToRecentSpecFiles(name, true);
   
/*
   std::vector<QString> files;
   pf->getRecentSpecFiles(files);
   int foundIndex = -1;
   for (unsigned int i = 0;  i < files.size(); i++) {
      if (files[i] == name) {
         if (i == 0) {
            // name is last file - do not need to do anything
            return;
         }
         foundIndex = i;
         break;
      }
   }
   
   //
   // Make file just opened the first file and add allow up to 15 files.
   //
   std::vector<QString> filesOut;
   filesOut.push_back(name);
   
   for (int j = 0; j < static_cast<int>(files.size()); j++) {
      if (j != foundIndex) {
         filesOut.push_back(files[j]);
      }
      if (filesOut.size() >= 15) {
         break;
      }
   }
   
   pf->setRecentSpecFiles(filesOut);
   if (pf->getFileName().empty() == false) {
      try {
         pf->writeFile(pf->getFileName());
      }
      catch(FileException&) {
      }
   }
*/
}

/**
 * Read a spec file with the specified name
 */
void
GuiMainWindow::readSpecFile(const QString& filename)
{
   removeAllImageViewingWindows();
   
   //
   // Read the spec file.
   //
   SpecFile sf;   
   try {
      sf.readFile(filename);
      sf.setDefaultFilesFiducialAndFlat();
   }
   catch (FileException& e) {
      QApplication::restoreOverrideCursor();
      QMessageBox::critical(this, 
                            "Error reading spec file",
                            e.whatQString());
      return;
   }

   qDebug() << "GuiMainWindow::readSpecFile() - call GuiSpecFileDialog()";
   GuiSpecFileDialog* specDialog = new GuiSpecFileDialog(this, sf, 
                                        GuiSpecFileDialog::SPEC_DIALOG_MODE_OPEN_SPEC_FILE);
   specDialog->show();
   specDialog->raise();
   specDialog->activateWindow();
}

/**
 * Load the specified spec file's data files.
 */
void
GuiMainWindow::loadSpecFilesDataFiles(SpecFile sf, 
                                      const TransformationMatrix* tm,
                                      const bool appendToExistingLoadedSpecFiles)
{
   if (getBrainSet()->getNumberOfBrainModels() > 0) {
      int result = 0;
      if (appendToExistingLoadedSpecFiles == false) {
         QString msg("Caret is now able to load and view multiple spec files.\n"
                     "\n"
                     "If you are not sure which button to push, use \"New Spec Only\".\n"
                     "\n"
                     "The model selection control in the main window's toolbar and in"
                     "the viewing window's toolbar will list the brain models (contours,"
                     "surfaces, and volumes) for all loaded spec files.  The display "
                     "control dialog and all other dialogs operate on the model (and"
                     "its associated data files) in the main window.\n"
                     "\n"
                     "Choose \"New Spec Only\" if you only want to view the new"
                     "spec file that you are loading.\n"
                     "\n"
                     "Choose \"Keep Loaded Spec\" if you want to view both the new"
                     "spec file and the files already loaded into Caret.\n");
         WuQMessageBox msgBox(this);
         msgBox.setText(msg);
         msgBox.setTheWindowTitle("New Spec?");
         QPushButton* keepLoadedSpecPushButton = msgBox.addButton("Keep Loaded Spec",
                                                                   QMessageBox::ActionRole);
         QPushButton* newSpecOnlyPushButton = msgBox.addButton("New Spec Only",
                                                                   QMessageBox::ActionRole);
         QPushButton* cancelPushButton = msgBox.addButton("Cancel",
                                                                   QMessageBox::ActionRole);
         msgBox.exec();
         if (msgBox.clickedButton() == keepLoadedSpecPushButton) {
            result = 0;
         }
         else if (msgBox.clickedButton() == newSpecOnlyPushButton) {
            result = 1;
         }
         else if (msgBox.clickedButton() == cancelPushButton) {
            result = 2;
         }
      }
      
      switch (result) {
         case 0:
            {
               BrainSet* bs = new BrainSet(false);
               addBrainSet(bs);
               //loadedBrainSetDirectory.push_back(QDir::currentPath());
               setBrainSet(BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW, bs);
            }
            break;
         case 1:
            //
            // Note: Do not need to erase the spec contents from memory
            // since that will be taken care of when new spec is read.
            //
            {
               QString msg;
               for (unsigned int i = 0; i < loadedBrainSets.size(); i++) {
                  checkForModifiedFiles(loadedBrainSets[i], msg, true);
               }
               if (msg.isEmpty() == false) {
                  //
                  // Return value of zero is YES button.
                  //
                  QString msg2("If you choose \"Yes\" changes to these files will be lost:\n\n");
                  msg2.append(msg);
                  
                  WuQDataEntryDialog warningDialog(this);
                  warningDialog.setWindowTitle("Caret 5");
                  warningDialog.addTextEdit("", msg2, true);
                  warningDialog.setOkButtonText("Yes");
                  warningDialog.setCancelButtonText("No");
                  if (warningDialog.exec() != WuQDataEntryDialog::Accepted) {
                     return;
                  }

                  if (QMessageBox::warning(this, 
                                          "WARNING",
                                          msg2, 
                                          (QMessageBox::Yes | QMessageBox::No),
                                          QMessageBox::No)
                                             == QMessageBox::No) {
                     return;
                  }
               }
               
               //
               // Delete all but first brain set
               //
               for (unsigned int i = 1; i < loadedBrainSets.size(); i++) {
                  delete loadedBrainSets[i];
               }
               BrainSet* bs = loadedBrainSets[0];
               loadedBrainSets.clear();
               loadedBrainSets.push_back(bs); // do not use addBrainSet because signals already hooked up
               for (int i = 0; i < BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; i++) {
                  setBrainSet(static_cast<BrainModel::BRAIN_MODEL_VIEW_NUMBER>(i), bs);
               }
            }
            break;
         case 2:
            return;
            break;
      }
   }
   
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

   //
   // do not allow graphics updates while reading data files since the progress
   // dialog causes paintGL() calls with coordinate files only partially loaded.
   //
   GuiBrainModelOpenGL::setPaintingEnabled(false);
   
   const int numFiles = sf.getNumberOfSelectedFiles() + 2;
   
   QProgressDialog* progressDialog = NULL;
   progressDialog = new QProgressDialog(
                              "Reading Spec File",
                              "Cancel",
                              0,
                              numFiles,
                              this);
   if (progressDialog != NULL) {
      progressDialog->setWindowTitle("Reading Data Files");
      progressDialog->show();
   }
   
   std::vector<QString> messages;
  
   QTime timer;
   timer.start();
   getBrainSet()->blockSignals(true);   // so signal "signalBrainSetChanged()" is ignored
   const bool readingAborted = getBrainSet()->readSpecFile(BrainSet::SPEC_FILE_READ_MODE_NORMAL,
                                                   sf, sf.getFileName(), messages, 
                                                   tm, progressDialog);
   getBrainSet()->blockSignals(false);
   
   //
   // Update directory path
   //            
   for (unsigned int i = 0; i < loadedBrainSets.size(); i++) {
      if (loadedBrainSets[i] == getBrainSet()) {
         //loadedBrainSetDirectory[i] = QDir::currentPath();
         break;
      }
   }
   
   if (readingAborted) {
      messages.clear();
      getBrainSet()->reset();
   }
   //if (DebugControl::getDebugOn()) {
      std::cout << "Time to read all files was "
                << (static_cast<float>(timer.elapsed()) / 1000.0)
                << " seconds." << std::endl;
  // }
   
   if ((getBrainSet()->getNumberOfNodes() > 0) ||
       (messages.size() == 0)) {
      addToRecentSpecFiles(sf.getFileName()); //filename);
   }
   
   if (progressDialog != NULL) {
      progressDialog->setValue(numFiles);
      delete progressDialog;
   }
   
   
   //
   // Updates since spec file changed
   //
   postSpecFileReadInitializations();
   
   //
   // Turn off highlight crosses
   //
   for (int m = 0; m < getNumberOfBrainSets(); m++) {
      getBrainSet(static_cast<BrainModel::BRAIN_MODEL_VIEW_NUMBER>(m))->setDisplayCrossForNode(-1, NULL);
   }

   if (messages.size() > 0) {
      QString qs(messages[0]);
      for (unsigned int j = 1; j < messages.size(); j++) {
         qs.append("\n");
         qs.append(messages[j]);
      }
      QApplication::restoreOverrideCursor();
      QMessageBox::critical(this, 
                            "Error loading files from spec file",
                            qs);
      return;
   }
   
   GuiBrainModelOpenGL::updateAllGL();
   QApplication::restoreOverrideCursor(); 
   
   speakText("The specification file has been loaded.", false);
   
   //
   // If only scene files are selected
   //
   if (sf.onlySceneFilesSelected()) {
      //
      // Show the display control dialog and its scene page
      //
      displayDisplayControlDialog();
      displayControlDialog->showScenePage();
   }
   
   static bool firstTipsFlag = true;
   if (firstTipsFlag) {
      firstTipsFlag = false;
      
      //
      // See if tips dialog should be displayed
      //
      int dummy;
      bool startupFlag;
      PreferencesFile* preferencesFile = getBrainSet()->getPreferencesFile();
      preferencesFile->getCaretTips(dummy, startupFlag);
      
      if (startupFlag) {
         helpActions->helpMenuCaretTips();
         QApplication::beep();
      }
   }   
}

/**
 * Make updates when spec file is changed.
 */ 
void 
GuiMainWindow::postSpecFileReadInitializations()
{
   DisplaySettingsSurface* dsn = getBrainSet()->getDisplaySettingsSurface();
   
   //
   // Draw as nodes if have coord files but no topo files
   //
   if (getBrainSet()->getNumberOfTopologyFiles() <= 0) {
      bool haveCoordFiles = false;
      for (int i = 0; i < getBrainSet()->getNumberOfBrainModels(); i++) {
         if (getBrainSet()->getBrainModelSurface(i) != NULL) {
            haveCoordFiles = true;
         }
      }
      if (haveCoordFiles) {
         dsn->setDrawMode(DisplaySettingsSurface::DRAW_MODE_NODES);
      }
   }

   QString title("CARET v");
   title.append(CaretVersion::getCaretVersionAsString());
   title.append(" (");
   title.append(__DATE__);
   title.append(")");
   setWindowTitle(title);
   title.append(" ");
   title.append(FileUtilities::basename(getBrainSet()->getSpecFileName()));  //sf.getFileName()));
   setWindowTitle(title);

   GuiBrainModelOpenGL::setAllDisplayedBrainModelIndices(0);
   
   GuiFilesModified fm;
   fm.setStatusForAll(true);
   fileModificationUpdate(fm);
   
   GuiBrainModelOpenGL::setPaintingEnabled(true);
   GuiToolBar::clearAllYoking();
   GuiToolBar::updateAllToolBars(true);
   
   getBrainSet()->setDisplaySplashImage(false);
   if (displayControlDialog != NULL) {
      displayControlDialog->newSpecFileLoaded();
   }
}

/**
 * Create the status bar
 */
void
GuiMainWindow::createStatusBar()
{
   const int labelStyle = QFrame::Panel | QFrame::Sunken;
   
   statusBarMouseModeLabel      = new QLabel;
   statusBarMouseModeLabel->setFrameStyle(labelStyle);
   statusBarMouseModeLabel->setToolTip( "current mouse mode");
   
   statusBarSectionLowLabel     = new QLabel;
   statusBarSectionLowLabel->setFrameStyle(labelStyle);
   statusBarSectionLowLabel->setToolTip( "lowest section number");
   
   statusBarSectionHighLabel    = new QLabel;
   statusBarSectionHighLabel->setFrameStyle(labelStyle);
   statusBarSectionHighLabel->setToolTip( "highest section number");
   
   statusBarMouseLeftLabel      = new QLabel;
   statusBarMouseLeftLabel->setFrameStyle(labelStyle);
   statusBarMouseLeftLabel->setToolTip( "mouse moved with left button down function");
   
   statusBarMouseShiftLeftLabel = new QLabel;
   statusBarMouseShiftLeftLabel->setFrameStyle(labelStyle);
   statusBarMouseShiftLeftLabel->setToolTip( "mouse moved with shift and left button down function");
   
   statusBarMouseCtrlLeftLabel  = new QLabel;
   statusBarMouseCtrlLeftLabel->setFrameStyle(labelStyle);
#ifdef Q_OS_MACX
   statusBarMouseCtrlLeftLabel->setToolTip( "mouse moved with control and left button down function");
#else
   statusBarMouseCtrlLeftLabel->setToolTip( "mouse moved with apple and left button down function");
#endif
   
   statusBarMouseAltLeftLabel = new QLabel;
   statusBarMouseAltLeftLabel->setFrameStyle(labelStyle);
   statusBarMouseAltLeftLabel->setToolTip("mouse moved with atl button down function");
   
   statusBarMouseClickLeftLabel = new QLabel;
   statusBarMouseClickLeftLabel->setFrameStyle(labelStyle);
   statusBarMouseClickLeftLabel->setToolTip( "mouse left button click function");
   
   QStatusBar* sb = statusBar();
   sb->addWidget(statusBarMouseModeLabel);
   sb->addWidget(statusBarSectionLowLabel);
   sb->addWidget(statusBarSectionHighLabel);
   sb->addWidget(statusBarMouseLeftLabel);
   sb->addWidget(statusBarMouseShiftLeftLabel);
   sb->addWidget(statusBarMouseCtrlLeftLabel);
   sb->addWidget(statusBarMouseAltLeftLabel);
   sb->addWidget(statusBarMouseClickLeftLabel);
   sb->addWidget(new QWidget, 1000);

   updateStatusBarLabel();   
}

/**
 * Update the status bar
 */
void
GuiMainWindow::updateStatusBarLabel()
{
   //statusBar()->removeWidget(statusBarLabel);
   DisplaySettingsSection* dss = getBrainSet()->getDisplaySettingsSection();
   statusBarSectionLowLabel->setHidden(true); 
   statusBarSectionHighLabel->setHidden(true); 
   SectionFile* sf = getBrainSet()->getSectionFile();
   if (sf->getNumberOfColumns() > 0) {
      const int selectedColumn = dss->getSelectedDisplayColumn(-1, -1);
      if ((selectedColumn >= 0) && (selectedColumn < sf->getNumberOfColumns())) {
         const int minSection = dss->getMinimumSelectedSection();
         const int maxSection = dss->getMaximumSelectedSection();
         //statusBarSectionLowLabel->setHidden(false); 
         //statusBarSectionHighLabel->setHidden(false); 
         statusBarSectionLowLabel->setNum(minSection); 
         statusBarSectionHighLabel->setNum(maxSection); 
      }
   }
   
   QString modeLabel;
   QString leftLabel;
   QString ctrlLeftLabel;
   QString shiftLeftLabel;
   QString clickLeftLabel;
   QString altLeftLabel;
   
   switch(mainOpenGL->getMouseMode()) {
      case GuiBrainModelOpenGL::MOUSE_MODE_NONE:
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_BORDER_DRAW:
         modeLabel      = "DRAW BORDER";
         leftLabel      = "Draw";
         ctrlLeftLabel  = "Augment";
         altLeftLabel   = "Rotate(3D)";
         shiftLeftLabel = "Done";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_BORDER_DRAW_NEW:
         modeLabel      = "DRAW BORDER";
         leftLabel      = "Draw";
         ctrlLeftLabel  = "Augment";
         altLeftLabel   = "Rotate(3D)";
         shiftLeftLabel = "Done";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_BORDER_DELETE:
         modeLabel      = "DELETE BORDER";
         clickLeftLabel      = "Delete Border";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_BORDER_DELETE_POINT:
         modeLabel      = "DELETE BORDER";
         clickLeftLabel      = "Delete Border Point";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_VIEW:
         modeLabel      = "VIEW";
         leftLabel      = "Rotate";
         ctrlLeftLabel  = "Zoom";
         shiftLeftLabel = "Pan";
         clickLeftLabel = "ID";
         {
            BrainModelVolume* bmv = getBrainModelVolume();
            if (bmv != NULL) {
               if (bmv->getSelectedAxis(0) == VolumeFile::VOLUME_AXIS_OBLIQUE) {
                  shiftLeftLabel = ""; 
               }
               else if (volumeActions->getTranformRotationChecked() == false) {
                  leftLabel = "";
               }
            }
         }
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_BORDER_INTERPOLATE:
         modeLabel      = "BORDER INTERPOLATE";
         leftLabel      = "Choose Border 1";
         shiftLeftLabel = "Choose Border 2";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_BORDER_INTERPOLATE_NEW:
         modeLabel      = "BORDER INTERPOLATE";
         leftLabel      = "Choose Border 1";
         shiftLeftLabel = "Choose Border 2";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_BORDER_MOVE_POINT:
         modeLabel      = "MOVE BORDER POINT";
         leftLabel      = "Move Border Point";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_BORDER_REVERSE:
         modeLabel      = "REVERSE BORDER";
         clickLeftLabel = "Choose Border";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_BORDER_RENAME:
         modeLabel      = "RENAME BORDER";
         clickLeftLabel = "Choose Border";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_BORDER_UPDATE:
         modeLabel      = "BORDER UPDATE";
         leftLabel      = "Draw Update";
         shiftLeftLabel = "Done";
         altLeftLabel   = "Rotate(3D)";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_BORDER_UPDATE_NEW:
         modeLabel      = "BORDER UPDATE";
         leftLabel      = "Draw Update";
         shiftLeftLabel = "Done";
         altLeftLabel   = "Rotate(3D)";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_FOCI_DELETE:
         modeLabel      = "DELETE FOCI";
         clickLeftLabel = "Delete Focus";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CELL_DELETE:
         modeLabel      = "DELETE CELL";
         clickLeftLabel = "Delete Cell";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CUT_DRAW:
         modeLabel      = "DRAW CUT";
         leftLabel      = "Draw";
         shiftLeftLabel = "Done";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CUT_DELETE:
         modeLabel      = "DELETE CUT";
         clickLeftLabel = "Delete Cut";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_SURFACE_ROI_BORDER_SELECT:
         modeLabel      = "Surface ROI";
         clickLeftLabel = "Select Border";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_SURFACE_ROI_PAINT_INDEX_SELECT:
         modeLabel      = "Surface ROI";
         clickLeftLabel = "Select Node To Get Paint";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_SURFACE_ROI_METRIC_NODE_SELECT:
         modeLabel      = "Surface ROI";
         clickLeftLabel = "Select Node For Metric Query";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_SURFACE_ROI_GEODESIC_NODE_SELECT:
         modeLabel      = "Surface ROI";
         clickLeftLabel = "Select Node For Geodesic Query";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_ALIGN_STANDARD_ORIENTATION:
         modeLabel      = "Align to Std Orientation";
         leftLabel      = "Ventral Tip";
         shiftLeftLabel = "Dorsal-Medial Tip";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_ALIGN_STANDARD_ORIENTATION_FULL_HEM_FLATTEN:
         modeLabel      = "Set Central Sulcus Tips";
         leftLabel      = "Ventral Tip";
         shiftLeftLabel = "Dorsal-Medial Tip";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CONTOUR_SET_SCALE:
         modeLabel      = "Contour Set Scale";
         leftLabel      = "Start Point";
         shiftLeftLabel = "End Point";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CONTOUR_DRAW:
         modeLabel      = "Draw Contour";
         leftLabel      = "Draw the Contour";
         shiftLeftLabel = "Close the Contour (Done)";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CONTOUR_ALIGN:
         modeLabel      = "Align Contour";
         leftLabel      = "Rotate";
         ctrlLeftLabel  = "Zoom";
         shiftLeftLabel = "Pan";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CONTOUR_ALIGN_REGION:
         modeLabel      = "Contour Region";
         leftLabel      = "Delineate";
         ctrlLeftLabel  = "Move Mode";
         shiftLeftLabel = "Clear";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CONTOUR_POINT_MOVE:
         modeLabel      = "Move Contour Point";
         leftLabel      = "Drag Contour Point";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CONTOUR_POINT_DELETE:
         modeLabel      = "Delete Contour Point";
         clickLeftLabel = "Select Contour Point For Deletion";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CONTOUR_DELETE:
         modeLabel      = "Delete Contour";
         clickLeftLabel = "Select Contour For Deletion";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CONTOUR_REVERSE:
         modeLabel      = "Reverse Contour";
         clickLeftLabel = "Select Contour For Point Reversal";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CONTOUR_MERGE:
         modeLabel      = "Merge Contours";
         leftLabel      = "Connect Contour";
         shiftLeftLabel = "To This Contour";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CELL_ADD:
         modeLabel      = "Add Cell";
         clickLeftLabel = "Select Node for Cell Location";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CONTOUR_CELL_ADD:
         modeLabel      = "Add Contour Cell";
         clickLeftLabel = "Click Cell Location";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CONTOUR_CELL_DELETE:
         modeLabel      = "DELETE CONTOUR CELL";
         clickLeftLabel = "Delete Contour Cell";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_CONTOUR_CELL_MOVE:
         modeLabel      = "MOVE CONTOUR CELL";
         clickLeftLabel = "Move Contour Cell";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_VOLUME_SEGMENTATION_EDIT:
         modeLabel      = "SEGMENTATION EDIT";
         leftLabel      = "Apply to Voxels";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_EDIT_ADD_NODE:
         modeLabel      = "ADD NODE";
         clickLeftLabel = "Add Node";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_EDIT_ADD_TILE:
         modeLabel      = "ADD TILE";
         clickLeftLabel = "Choose Three Nodes";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_EDIT_DELETE_TILE_BY_LINK:
         modeLabel      = "DELETE TILE VIA LINK";
         clickLeftLabel = "Choose Link";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_EDIT_DISCONNECT_NODE:
         modeLabel      = "DISCONNECT NODE";
         clickLeftLabel = "Disconnect Node";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_EDIT_MOVE_NODE:
         modeLabel      = "MOVE NODE";
         leftLabel      = "Move Node";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_SURFACE_ROI_SHAPE_NODE_SELECT:
         modeLabel      = "Surface ROI";
         clickLeftLabel = "Select Node For Shape Query";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_TRANSFORMATION_MATRIX_AXES:
         modeLabel      = "TRANS AXES";
         leftLabel      = "Rotate";
         shiftLeftLabel = "Pan";
         ctrlLeftLabel  = "Zoom";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_TRANSFORMATION_MATRIX_SET_TRANSLATE:
         modeLabel      = "TRANS MATRIX";
         leftLabel      = "Set Translation";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_VOLUME_PAINT_EDIT:
         modeLabel      = "PAINT EDIT";
         leftLabel      = "Apply to Voxels";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_IMAGE_SUBREGION:
         modeLabel      = "CAPTURE IMAGE REGION";
         leftLabel      = "Select Region";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_SURFACE_ROI_SULCAL_BORDER_NODE_START:
         modeLabel      = "Surface Border ROI";
         clickLeftLabel = "Select Start Node";
         break;
      case GuiBrainModelOpenGL::MOUSE_MODE_SURFACE_ROI_SULCAL_BORDER_NODE_END:
         modeLabel      = "Surface Border ROI";
         clickLeftLabel = "Select End Node";
         break;
   }
   
   statusBarMouseModeLabel->setText(modeLabel);

   if (leftLabel.isEmpty() == false) {
      QString s("Left:");
      s.append(leftLabel);
      statusBarMouseLeftLabel->setText(s);
      statusBarMouseLeftLabel->setHidden(false);
   }
   else {
      statusBarMouseLeftLabel->setHidden(true);
   }
   
   if (ctrlLeftLabel.isEmpty() == false) {
#ifdef Q_OS_MACX
      QString s("Apple-Left:");
#else
      QString s("Ctrl-Left:");
#endif
      s.append(ctrlLeftLabel);
      statusBarMouseCtrlLeftLabel->setText(s);
      statusBarMouseCtrlLeftLabel->setHidden(false);
   }
   else {
      statusBarMouseCtrlLeftLabel->setHidden(true);
   }
   
   if (shiftLeftLabel.isEmpty() == false) {
      QString s("Shift-Left:");
      s.append(shiftLeftLabel);
      statusBarMouseShiftLeftLabel->setText(s);
      statusBarMouseShiftLeftLabel->setHidden(false);
   }
   else {
      statusBarMouseShiftLeftLabel->setHidden(true);
   }
   
   if (altLeftLabel.isEmpty() == false) {
      QString s("Alt-Left:");
      s.append(altLeftLabel);
      statusBarMouseAltLeftLabel->setText(s);
      statusBarMouseAltLeftLabel->setHidden(false);
   }
   else {
      statusBarMouseAltLeftLabel->setHidden(true);
   }
   
   if (clickLeftLabel.isEmpty() == false) {
      QString s("Click-Left:");
      s.append(clickLeftLabel);
      statusBarMouseClickLeftLabel->setText(s);
      statusBarMouseClickLeftLabel->setHidden(false);
   }
   else {
      statusBarMouseClickLeftLabel->setHidden(true);
   }
   
   statusBar()->adjustSize();
   statusBar()->updateGeometry();
   statusBar()->adjustSize();
   statusBar()->updateGeometry();
}

/**
 * update the displayed menus based upon loaded contours/surface/volume in main window
 */
void 
GuiMainWindow::updateDisplayedMenus()
{
//   menuBar()->setItemEnabled(surfaceMenuID, (getBrainModelSurface() != NULL));
//   menuBar()->setItemEnabled(volumeMenuID, (getBrainModelVolume() != NULL) ||
//                                           (getBrainModelSurfaceAndVolume() != NULL));
}

/**
 * This method should be called when a file is changed or loaded.  It will make any
 * necessary updates to the brain models and all user interface components.  It will
 * not redraw any of the displays.
 */
void
GuiMainWindow::fileModificationUpdate(const GuiFilesModified& fm)
{
   for (int i = 0; i < getBrainSet()->getNumberOfSurfaceOverlays(); i++) {
      getBrainSet()->getSurfaceOverlay(i)->update();
   }
   
   bool updateAddCellsDialog = false;
   bool updateAddContourCellsDialog = false;
   bool updateArealEstimation = false;
   bool updateBorders = false;
   bool updateBorderDisplayFlags = false;
   bool updateBorderColors = false;
   bool updateBorderProjections = false;
   bool updateCellDisplayFlags = false;
   bool updateCellColors   = false;
   bool updateContourCellDisplayFlags = false;
   bool updateContourCellColors = false;
   bool updateCocomac = false;
   bool updateContours = false;
   bool updateCoordinates = false;
   bool updateDeformationMap = false;
   bool updateDeformationField = false;
   bool updateFociDisplayFlags = false;
   bool updateFoci = false;
   bool updateFociColors   = false;
   bool updateFociSearch = false;
   bool updateGeodesic = false;
   bool updateImages = false;
   bool updateLatLon = false;
   bool updateMetrics = false;
   bool updateModels = false;
   bool updateNodeColoring = false;
   bool updatePaint = false;
   bool updateProbAtlas = false;
   bool updateRegion = false;
   bool updateRgbPaint = false;
   bool updateScene = false;
   bool updateSections = false;
   bool updateSurfaceShape = false;
   bool updateVector = false;
   bool updateStatusBar = false;
   bool updateStudyCollection = false;
   bool updateStudyMetaData = false;
   bool updateToolBar      = false;
   bool updateTopography = false;
   bool updateTopology = false;
   bool updateTransformMatrices = false;
   bool updateTransformData = false;
   bool updateVocabulary = false;
   bool updateVolume = false;
   bool updateVolumeColoring = false;
   
   if (fm.areaColor) {
      updateNodeColoring = true;
      updateVolumeColoring = true;
   }
   if (fm.arealEstimation) {
      updateArealEstimation = true;
      updateNodeColoring = true;
   }
   if (fm.border) {
      updateBorders = true;
      updateBorderColors = true;
      updateBorderDisplayFlags = true;
      updateBorderProjections = true;
   }
   if (fm.borderColor) {
      updateBorderColors = true;
      updateBorderDisplayFlags = true;
   }
   if (fm.cell) {
      updateCellColors = true;
      updateCellDisplayFlags = true;
      updateAddCellsDialog = true;
   }
   if (fm.cellColor) {
      updateCellColors = true;
      updateCellDisplayFlags = true;
   }
   if (fm.cellProjection) {
      updateCellColors = true;
      updateCellDisplayFlags = true;
   }
   if (fm.cocomac) {
      updateCocomac = true;
      updateNodeColoring = true;
   }
   if (fm.contour) {
      updateContours = true;
      updateToolBar = true;  
   }
   if (fm.contourCell) {
      updateContourCellDisplayFlags = true;    
      updateAddContourCellsDialog = true;
      updateContourCellColors = true;
   }
   if (fm.contourCellColor) {
      updateContourCellDisplayFlags = true;
      updateContourCellColors = true;
   }
   if (fm.coordinate) {  
      updateAddCellsDialog = true; 
      updateAddContourCellsDialog = true;
      updateCoordinates = true;
      updateToolBar = true;      
      updateBorderProjections = true;
      
      if (fm.inhibitDefaultSurfaceScaling == false) {
         double orthoLeft, orthoRight, orthoBottom, orthoTop,
               orthoNear, orthoFar;
         mainOpenGL->getOrthographicBox(orthoLeft, orthoRight, orthoBottom, orthoTop,
                                       orthoNear, orthoFar);
         getBrainSet()->setDefaultScaling(orthoRight, orthoTop);      
      }
   }
   if (fm.cut) {
   }
   
   if (fm.deformationField) {
      updateDeformationField = true;
   }
   if (fm.deformationMap) {
      updateDeformationMap = true;
   }
   if (fm.foci) {
      updateFoci       = true;
      updateFociColors = true;
      updateFociDisplayFlags = true;
      updateStudyMetaData = true;
   }
   if (fm.fociColor) {
      updateFociColors = true;
      updateFociDisplayFlags = true;
   }
   if (fm.fociProjection) {
      updateFociColors = true;
      updateFociDisplayFlags = true;
      updateStudyMetaData = true;
   }
   if (fm.fociSearch) {
      updateFociSearch = true;
   }
   if (fm.geodesic) {
      updateGeodesic = true;
   }
   if (fm.images) {
      updateImages = true;
   }
   if (fm.latLon) {
      updateLatLon = true;
   }
   if (fm.metric) {
      updateMetrics = true;
      updateNodeColoring = true;
   }
   
   if (fm.paint) {
      updatePaint = true;
      updateNodeColoring = true;
   }
   if (fm.palette) {
      updateMetrics = true;
      updateNodeColoring = true;
      updateSurfaceShape = true;
      updateVolumeColoring = true;
      if (paletteEditorDialog != NULL) {
         paletteEditorDialog->updateDialog();
      }
   }
   if (fm.parameter) {
      if (paramsFileEditorDialog != NULL) {
         paramsFileEditorDialog->updateDialog();
      }
   }
   if (fm.probabilisticAtlas) {
      updateProbAtlas = true;
      updateNodeColoring = true;
   }
   if (fm.rgbPaint) {
      updateNodeColoring = true;
      updateRgbPaint = true;
   }
   if (fm.scene) {
      updateScene = true;
   }
   if (fm.section) {
      updateSections = true;
      updateStatusBar = true;
      updateCellDisplayFlags = true;
   }
   if (fm.surfaceShape) {
      updateNodeColoring = true;
      updateSurfaceShape = true;
   }
   if (fm.vector) {
      updateVector = true;
   }
   if (fm.topography) {
      updateNodeColoring = true;
      updateTopography = true;
   }
   if (fm.topology) {
      updateBorderProjections = true;
      updateTopology = true;
   }
   if (fm.transformationMatrix) {
      updateTransformMatrices = true;
      updateVolume = true;
   }
   if (fm.transformationData) {
      updateFociColors = true;
      updateFociDisplayFlags = true;
      updateCellColors = true;
      updateCellDisplayFlags = true;
      updateTransformData = true;
      updateModels = true;
   }
   if (fm.studyCollection) {
      updateStudyCollection = true;
   }
   if (fm.studyMetaData) {
      updateFoci       = true;
      updateFociColors = true;
      updateFociDisplayFlags = true;
      updateStudyCollection = true;
      updateStudyMetaData = true;
   }
   if (fm.vocabulary) {
      updateVocabulary = true;
      updateStudyMetaData = true;
   }
   if (fm.volume) {
      updateToolBar = true;
      updateVector = true;
      updateVolume = true; 
   }
   
   if (fm.vtkModel) {
      updateModels = true;
   }
   
   if (fm.wustlRegion) {
      updateRegion = true;
   }



   if (updateCoordinates) {
      if (flatMorphingDialog != NULL) {
         flatMorphingDialog->updateDialog();
      }
      if (sphereMorphingDialog != NULL) {
         sphereMorphingDialog->updateDialog();
      }
   }
   
   if (updateCoordinates || updateMetrics) {
      if (metricModificationDialog != NULL) {
         metricModificationDialog->updateDialog();
      }
   }
   
   if (updateBorderProjections) {
      // This should not be necessary but if it is re-enabled it will
      // cause all borders to be unmodified.
      //BrainModelBorderSet* bmbs = getBrainSet()->getBorderSet();
      //bmbs->unprojectBordersForAllSurfaces();
   }
   if (updateBorderColors) {
      getBrainSet()->assignBorderColors();
   }
   
   if (updateBorders || updateBorderProjections || updateBorderColors ||
       updateCoordinates || updatePaint || updateSurfaceShape || updateVolume) {
      if (borderOperationsDialog != NULL) {
         borderOperationsDialog->updateDialog();
      }
   }
   
   if (updateBorders || updateBorderProjections ||
       updateCoordinates || updatePaint || updateSurfaceShape) {
      if (flattenFullHemisphereDialog != NULL) {
         flattenFullHemisphereDialog->updateDialog();
      }
      if (standardMeshDialog != NULL) {
         standardMeshDialog->updateDialog();
      }
   }
   
   if (updateCellColors) {
      getBrainSet()->assignCellColors(); 
   }
   
   if (updateContours) {
      if (contourAlignmentDialog != NULL) contourAlignmentDialog->updateDialog();
      if (contourSectionControlDialog != NULL) contourSectionControlDialog->updateDialog();
      if (contourSetScaleDialog != NULL) contourSetScaleDialog->updateDialog();
   }
   if (updateContourCellColors) {
      getBrainSet()->assignContourCellColors();
   }
   
   if (updateCoordinates || updateTopology) {
      if (interpolateSurfaceDialog != NULL) {
         interpolateSurfaceDialog->updateDialog();
      }
      if (setTopologyDialog != NULL) {
         setTopologyDialog->updateDialog();
      }
   }

   if (updateCoordinates || updateFoci || updatePaint || updateContours || updateVolume) {
      if (fociAttributeAssignmentDialog != NULL) {
         fociAttributeAssignmentDialog->updateDialog();
      }
   }
   if (updateFoci) {
      if (mapStereotaxicFocusDialog != NULL) {
         mapStereotaxicFocusDialog->updateDialog();
      }
   }
   if (updateFociColors) {
      getBrainSet()->assignFociColors();
   }
   if (updateMetrics) {
      if (metricsToRgbPaintDialog != NULL) {
         metricsToRgbPaintDialog->updateDialog();
      }
      if (metricMathDialog != NULL) {
         metricMathDialog->updateDialog();
      }
   }
   if (updateNodeColoring) {
      getBrainSet()->getNodeColoring()->assignColors();
   }
   if (updateNodeColoring) {
      if (paintNameEditorDialog != NULL) {
         paintNameEditorDialog->updateDialog();
      }
   }
   if (updatePaint || updateVolume) {
      if (drawBorderDialog != NULL) {
         drawBorderDialog->updateDialog();
      }
   }
   if (updateRgbPaint) {
      if (metricsToRgbPaintDialog != NULL) {
         metricsToRgbPaintDialog->updateDialog(); 
      }
   }
   
   if (updateBorderDisplayFlags) {
      DisplaySettingsBorders* dsb = getBrainSet()->getDisplaySettingsBorders();
      dsb->determineDisplayedBorders();
   }
   if (updateCellDisplayFlags) {
      DisplaySettingsCells* dsc = getBrainSet()->getDisplaySettingsCells();
      dsc->determineDisplayedCells();
   }
   if (updateFociDisplayFlags) {
      DisplaySettingsFoci* dsf = getBrainSet()->getDisplaySettingsFoci();
      dsf->determineDisplayedFoci();
   }
   if (updateContourCellDisplayFlags) {
      DisplaySettingsContours* dsc = getBrainSet()->getDisplaySettingsContours();
      dsc->determineDisplayedContourCells();
   }
   if (updateAddCellsDialog) {
      if (addCellsDialog != NULL) {
         addCellsDialog->updateDialog();
      }
   }
   if (updateAddContourCellsDialog) {
      if (addContourCellsDialog != NULL) {
         addContourCellsDialog->updateDialog();
      }
   }
   
   if (updateImages) {
      const int num = getBrainSet()->getNumberOfImageFiles();
      if (num <= 0) {
         removeAllImageViewingWindows();
         if (imageEditorWindow != NULL) {
            imageEditorWindow->close();
         }
      }
      else {
         for (unsigned int i = 0; i < imageViewingWindows.size(); i++) {
            imageViewingWindows[i]->updateWindow();
         }
         if (imageEditorWindow != NULL) {
            imageEditorWindow->updateWindow();
         }
      }
   }

   if (updateModels) {
      if (modelsEditorDialog != NULL) {
         modelsEditorDialog->updateDialog();
      }
   }
   
   if (updateSections) {
      updateSectionControlDialog();
   }
   
   if (updateStatusBar) {
      updateStatusBarLabel();
   }
   
   if (updateSurfaceShape) {
      if (shapeModificationDialog != NULL) {
         shapeModificationDialog->updateDialog();
      }
      if (shapeMathDialog != NULL) {
         shapeMathDialog->updateDialog();
      }
   }
   
   if (updateStudyMetaData) {
      if (studyMetaDataFileEditorDialog != NULL) {
         studyMetaDataFileEditorDialog->updateDialog();
      }
   }
   
   if (updateStudyCollection) {
      if (studyCollectionFileEditorDialog != NULL) {
         studyCollectionFileEditorDialog->updateDialog();
      }
   }
   
   if (updateToolBar) {
      GuiToolBar::updateAllToolBars(true);
      updateDisplayedMenus();
   }
   
   if (updateTransformMatrices || updateTransformData) {
      if (transformMatrixEditorDialog != NULL) {
         transformMatrixEditorDialog->updateDialog();
      }
   }
   
   if (updateMetrics || updatePaint || updateSurfaceShape ||
       updateBorders || updateBorderProjections || 
       updateTopology || updateToolBar) {
      if (surfaceRegionOfInterestDialog != NULL) {
         surfaceRegionOfInterestDialog->updateDialog();
      }
      if (surfaceRegionOfInterestDialogOLD != NULL) {
         surfaceRegionOfInterestDialogOLD->updateDialog();
      }
   }
   
   if (updateVocabulary) {
      if (vocabularyFileEditorDialog != NULL) {
         vocabularyFileEditorDialog->updateDialog();
      }
   }
   
   if (updateVolume) {
      if (volumeRegionOfInterestDialog != NULL) {
         volumeRegionOfInterestDialog->updateDialog();
      }
      if (volumeThresholdSegmentationDialog != NULL) {
         volumeThresholdSegmentationDialog->updateDialog();
      }
   }
   
   if (updateVolume) {
      if (volumeResizingDialog != NULL) {
         volumeResizingDialog->updateDialog(true);
      }
      if (volumeAttributesDialog != NULL) {
         volumeAttributesDialog->updateDialog();
      }
      if (volumeMathDialog != NULL) {
         volumeMathDialog->updateDialog();
      }
      if (volumePaintEditorDialog != NULL) {
         volumePaintEditorDialog->updateDialog();
      }
      if (volumeSegmentationEditorDialog != NULL) {
         volumeSegmentationEditorDialog->updateDialog();
      }
   }
   if (updateVolumeColoring) {
      BrainModelVolumeVoxelColoring* vvc = getBrainSet()->getVoxelColoring();
      vvc->setVolumeFunctionalColoringInvalid();
      vvc->setVolumePaintColoringInvalid();
      vvc->setVolumeProbAtlasColoringInvalid();
   }
   
   if (fm.getAllFilesModified()) {
      updateDisplayControlDialog();
   }
   else {
      if (displayControlDialog != NULL) {
         if (updateArealEstimation) {
            // "updateOverlayUnderlayItems()" called below calls this
            //displayControlDialog->updateArealEstimationItems();
         }
         if (updateBorderColors) {
            displayControlDialog->updateBorderItems(true);
         }
         if (updateCellColors) {
            displayControlDialog->updateCellItems(true);
         }
         if (updateCocomac || updatePaint) {
            displayControlDialog->updateCocomacItems();
         }
         if (updateContours) {
            displayControlDialog->updateContourItems(true);
         }
         if (updateCoordinates) {
            displayControlDialog->updateMiscItems();
         }
         if (updateDeformationField) {
            displayControlDialog->updateDeformationFieldPage();
         }
         if (updateFociColors || updateFociSearch) {
            displayControlDialog->updateFociItems(true);
         }
         if (updateGeodesic) {
            displayControlDialog->updateGeodesicItems();
         }
         if (updateImages) {
            displayControlDialog->updateImagesItems();
         }
         if (updateLatLon) {
            displayControlDialog->updateLatLonItems();
         }
         if (updateMetrics || updateVolume) {
            // "updateOverlayUnderlayItems()" called below calls this
            //displayControlDialog->updateMetricItems();
         }
         if (updateModels || updateTransformMatrices) {
            displayControlDialog->updateModelItems();
         }
         if (updatePaint) {
            // "updateOverlayUnderlayItems()" called below calls this
            //displayControlDialog->updatePaintItems();
         }
         if (updateProbAtlas) {
            displayControlDialog->updateProbAtlasSurfaceItems(true);
         }
         if (updateScene) {
            displayControlDialog->updateSceneItems();
         }
         if (updateRegion || updateVolume) {
            displayControlDialog->updateRegionItems();
         }
         if (updateRgbPaint) {
            displayControlDialog->updateRgbPaintItems();
         }
         if (updateSurfaceShape) {
            // "updateOverlayUnderlayItems()" called below calls this
            //displayControlDialog->updateShapeItems();
         }
         if (updateVector) {
            displayControlDialog->updateVectorItems();
         }
         if (updateTopography) {
            // "updateOverlayUnderlayItems()" called below calls this
            //displayControlDialog->updateTopographyItems();
         }
         if (updateVolume || updateCoordinates) {
            displayControlDialog->updateSurfaceAndVolumeItems();
            displayControlDialog->updateVolumeItems();
         }
         displayControlDialog->updateOverlayUnderlayItemsNew();
      }
   }
   
   GuiIdentifyDialog* idDialog = getIdentifyDialog(false);
   if (idDialog != NULL) {
      idDialog->updateDialog();
   }

   if (connectivityDialog != NULL) {
      connectivityDialog->updateDialog();
   }
}

/**
 * display a web page in the user's web browser.
 */
void 
GuiMainWindow::displayWebPage(const QString& webPage)
{
   PreferencesFile* pf = getBrainSet()->getPreferencesFile();
   const int returnCode = SystemUtilities::displayInWebBrowser(webPage,
                                                               pf->getWebBrowser());
   
   if (returnCode != 0) {
      QString msg("Loading the web page appears to have failed.  Unix users may \n"
                      "need to set the environment variable CARET_WEB_BROWSER to the\n"
                      "desired web browser.");
      QMessageBox::warning(this, "Web Browser Error", msg);
   }
}

/**
 * update the transformation matrix Editor.
 */
void 
GuiMainWindow::updateTransformationMatrixEditor(const TransformationMatrix* tm)
{
   if (transformMatrixEditorDialog != NULL) {
      transformMatrixEditorDialog->updateMatrixDisplay(tm);
   }
}
         
/**
 * display the tranformation editor dialog.
 */
void 
GuiMainWindow::displayTransformMatrixEditor()
{
   if (transformMatrixEditorDialog == NULL) {
      transformMatrixEditorDialog = new GuiTransformationMatrixDialog(this);
   }
   transformMatrixEditorDialog->show();
   transformMatrixEditorDialog->activateWindow();
}

/**
 * display the params file editor dialog.
 */
void 
GuiMainWindow::displayParamsFileEditorDialog()
{
   if (paramsFileEditorDialog == NULL) {
      paramsFileEditorDialog = new GuiParamsFileEditorDialog(this);
   }
   paramsFileEditorDialog->show();
   paramsFileEditorDialog->activateWindow();
}
      
/**
 * apply a scene (set display settings).
 */
void 
GuiMainWindow::showScene(const SceneFile::Scene* scene, 
                         const bool checkSpecFlag,
                         QString& errorMessage) 
{
   //SceneFile* sf = getBrainSet()->getSceneFile();
   //if ((sceneNumber < 0) || (sceneNumber >= sf->getNumberOfScenes())) {
   //   return;
   //}   
   //SceneFile::Scene* scene = sf->getScene(sceneNumber);
   
   //
   // Update the brain set
   //
   QString warningMessage;
   getBrainSet()->showScene(scene, checkSpecFlag, errorMessage, warningMessage);
   errorMessage += warningMessage;
   
   removeAllImageViewingWindows();
   //removeAllModelViewingWindows();
   
   //
   // Get screen size
   //
   QDesktopWidget* dt = QApplication::desktop();
   const int screenMaxX = std::max(dt->width() - 200, dt->width() / 2);
   const int screenMaxY = std::max(dt->height() - 200, dt->height() / 2);

   const QString viewingWindowString("ViewingWindow");
   //const int viewingWindowStringLength = viewingWindowString.length();
   
   DisplaySettingsScene* dss = getBrainSet()->getDisplaySettingsScene();
   
   const int screenMinX =  0;
#ifdef Q_OS_MACX
   const int screenMinY = 20;
#else // Q_OS_MACX
   const int screenMinY =  0;
#endif // Q_OS_MACX

   int mainWindowSceneX = 10;
   int mainWindowSceneY = 10;
   int mainWindowX = 10;
   int mainWindowY = 10;
   int mainWindowSizeX = 400;
   int mainWindowSizeY = 400;
   for (int i = 0; i < BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; i++) {
      QString msg;
      int geometry[4];
      int glWidgetWidthHeight[2];
      bool yokeFlag = false;
      BrainModel* bm = getBrainSet()->showSceneGetBrainModel(scene,
                                                        i,
                                                        geometry,
                                                        glWidgetWidthHeight,
                                                        yokeFlag,
                                                        msg);
      if (bm != NULL) {
         if (i == 0) {
            //
            // Show the desired brain model in the main window
            //
            displayBrainModelInMainWindow(bm);

            //
            // If desired size of window is unknown, resize so graphics
            // is proper size
            // 
            if (geometry[0] < 0) {
               geometry[0] = x();
               geometry[1] = y();
               
               const int deltaSizeX = glWidgetWidthHeight[0] - mainOpenGL->width();
               const int deltaSizeY = glWidgetWidthHeight[1] - mainOpenGL->height();
               geometry[2] = width() + deltaSizeX;
               geometry[3] = height() + deltaSizeY;
            }
            geometry[0] = std::min(geometry[0], screenMaxX);
            geometry[1] = std::min(geometry[1], screenMaxY);
            
            switch(dss->getWindowPositionPreference()) {
               case DisplaySettingsScene::WINDOW_POSITIONS_USE_ALL:
                  geometry[0] = std::max(geometry[0], screenMinX);
                  geometry[1] = std::max(geometry[1], screenMinY);
                  move(geometry[0], geometry[1]);
                  resize(geometry[2], geometry[3]);
                  break;
               case DisplaySettingsScene::WINDOW_POSITIONS_IGNORE_MAIN_OTHERS_RELATIVE:
                  resize(geometry[2], geometry[3]);
                  break;
               case DisplaySettingsScene::WINDOW_POSITIONS_IGNORE_ALL:
                  break;
            }
            mainWindowX = x();
            mainWindowY = y();
            mainWindowSceneX = geometry[0];
            mainWindowSceneY = geometry[1];
            mainWindowSizeX  = geometry[2];
            mainWindowSizeY  = geometry[3];
         }
         else {
            //
            // OpenGL for window
            //
            GuiBrainModelOpenGL* openGL = GuiBrainModelOpenGL::getBrainModelOpenGLForWindow(
                                    static_cast<BrainModel::BRAIN_MODEL_VIEW_NUMBER>(i));
            //
            // Create window if needed
            //
            GuiBrainModelViewingWindow* viewWindow = modelWindow[i];
            if (viewWindow == NULL) {
               showViewingWindow(static_cast<BrainModel::BRAIN_MODEL_VIEW_NUMBER>(i));
               viewWindow = modelWindow[i];
            }
            viewWindow->displayBrainModelInWindow(bm);
            if (openGL != NULL) {
               openGL->setYokeView(yokeFlag);
            }
            viewWindow->getToolBar()->setYokeStatus(yokeFlag);
            if (geometry[0] < 0) {
               geometry[0] = static_cast<int>(mainWindowSceneX + 20.0);
               geometry[1] = static_cast<int>(mainWindowSceneY + 20.0);
               if (i == 1) {
                  geometry[0] = mainWindowSceneX + mainWindowSizeX;
                  geometry[1] = mainWindowSceneY;
               }
               else if (i == 2) {
                  geometry[0] = mainWindowSceneX;
                  geometry[1] = mainWindowSceneY + mainWindowSizeY;
               }
               else if (i == 3) {
                  geometry[0] = mainWindowSceneX + mainWindowSizeX;
                  geometry[1] = mainWindowSceneY + mainWindowSizeY;
               }
               if (openGL != NULL) {
                  const int deltaSizeX = glWidgetWidthHeight[0] - openGL->width();
                  const int deltaSizeY = glWidgetWidthHeight[1] - openGL->height();
                  geometry[2] = viewWindow->width() + deltaSizeX;
                  geometry[3] = viewWindow->height() + deltaSizeY;
                  
               }
               else {
                  geometry[2] = 512;
                  geometry[3] = 512;
               }
            }
            geometry[0] = std::min(geometry[0], screenMaxX);
            geometry[1] = std::min(geometry[1], screenMaxY);
            switch(dss->getWindowPositionPreference()) {
               case DisplaySettingsScene::WINDOW_POSITIONS_USE_ALL:
                  geometry[0] = std::max(geometry[0], screenMinX);
                  geometry[1] = std::max(geometry[1], screenMinY);
                  viewWindow->move(geometry[0], geometry[1]);
                  viewWindow->resize(geometry[2], geometry[3]);
                  break;
               case DisplaySettingsScene::WINDOW_POSITIONS_IGNORE_MAIN_OTHERS_RELATIVE:
                  geometry[0] = (geometry[0] - mainWindowSceneX) + mainWindowX;
                  geometry[1] = (geometry[1] - mainWindowSceneY) + mainWindowY;
                  geometry[0] = std::max(geometry[0], screenMinX);
                  geometry[1] = std::max(geometry[1], screenMinY);
                  viewWindow->move(geometry[0], geometry[1]);
                  viewWindow->resize(geometry[2], geometry[3]);
                  break;
               case DisplaySettingsScene::WINDOW_POSITIONS_IGNORE_ALL:
                  break;
            }
            
         }
      }
      else if (i > 0) {
         if (modelWindow[i] != NULL) {
            modelWindow[i]->close();
            qApp->processEvents();
         }
         errorMessage.append(msg);
      }
   }
   
   //
   // Process identify dialog
   //
   GuiIdentifyDialog* id = getIdentifyDialog(false);
   id->showScene(*scene, 
                 mainWindowX,
                 mainWindowY,
                 mainWindowSceneX,
                 mainWindowSceneY,
                 screenMaxX,
                 screenMaxY,
                 errorMessage);
   id->updateDialog();
   
   //
   // popup image viewing windows
   //
   const int numClasses = scene->getNumberOfSceneClasses();
   for (int nc = 0; nc < numClasses; nc++) {
      const SceneFile::SceneClass* sc = scene->getSceneClass(nc);
      const QString className(sc->getName());
      if (className == "ImageViewingWindow") {
         displayImageViewingWindow();
         const int num = imageViewingWindows.size() - 1;
         if (num >= 0) {
            GuiImageViewingWindow* w = imageViewingWindows[num];
            w->showScene(*sc);
            int x = w->x();
            int y = w->y();
            x = std::min(x, screenMaxX);
            y = std::min(y, screenMaxY);
            switch(dss->getWindowPositionPreference()) {
               case DisplaySettingsScene::WINDOW_POSITIONS_USE_ALL:
                  w->move(x, y);
                  break;
               case DisplaySettingsScene::WINDOW_POSITIONS_IGNORE_MAIN_OTHERS_RELATIVE:
                  x = (x - mainWindowSceneX) + mainWindowX;
                  y = (y - mainWindowSceneY) + mainWindowY;
                  w->move(x, y);
                  break;
               case DisplaySettingsScene::WINDOW_POSITIONS_IGNORE_ALL:
                  break;
            }
         }
      }
   }

   //
   // Color Key Dialogs
   //
   showSceneColorKeyDialog(scene,
                           screenMaxX,
                           screenMaxY,
                           mainWindowSceneX,
                           mainWindowSceneY,
                           mainWindowX,
                           mainWindowY);
   
   //
   // Update GUI
   //
   GuiToolBar::updateAllToolBars(false);
   if (displayControlDialog != NULL) {
      displayControlDialog->showScene(*scene, errorMessage);
      displayControlDialog->updateAllItemsInDialog(true, true);
      updateSectionControlDialog();
      
   }

   if (connectivityDialog != NULL) {
      connectivityDialog->updateDialog();
   }
   
   GuiBrainModelOpenGL::updateAllGL(NULL); 
}

/**
 * create a scene (read display settings).
 */
void 
GuiMainWindow::saveScene(std::vector<SceneFile::SceneClass>& mainWindowSceneClasses)
{  
   for (int i = 0; i < BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; i++) {
      if (i == 0) {
         BrainModel* bm = mainOpenGL->getDisplayedBrainModel();
         if (bm != NULL) {
            //QRect geom = frameGeometry();
            const int geometry[4] = {
               x(), //geom.left(),
               y(), //geom.top(),
               width(), //geom.width(),
               height() //geom.height()
            };
            const int glWidgetWidthHeight[2] = {
               mainOpenGL->width(),
               mainOpenGL->height()
            };
            SceneFile::SceneClass sc("");
            getBrainSet()->saveSceneForBrainModelWindow(i, geometry, glWidgetWidthHeight, bm, false, sc);
            mainWindowSceneClasses.push_back(sc);
         }
      }
      else {
         GuiBrainModelViewingWindow* vw = modelWindow[i];
         if (vw != NULL) {
            GuiBrainModelOpenGL* openGL = GuiBrainModelOpenGL::getBrainModelOpenGLForWindow(
                                 static_cast<BrainModel::BRAIN_MODEL_VIEW_NUMBER>(i));
            if (openGL != NULL) {
               BrainModel* bm = openGL->getDisplayedBrainModel();
               if (bm != NULL) {
                  //QRect geom = vw->frameGeometry();
                  const int geometry[4] = {
                     vw->x(), //geom.left(),
                     vw->y(), //geom.top(),
                     vw->width(), //geom.width(),
                     vw->height() //geom.height()
                  };
                  const int glWidgetWidthHeight[2] = {
                     openGL->width(),
                     openGL->height()
                  };
                  SceneFile::SceneClass sc("");
                  getBrainSet()->saveSceneForBrainModelWindow(i, geometry, 
                                                              glWidgetWidthHeight, bm, 
                                                              openGL->getYokeView(), sc);
                  mainWindowSceneClasses.push_back(sc);
               }
            }
         }
      }
   }
   
   for (unsigned int i = 0; i < imageViewingWindows.size(); i++) {
      GuiImageViewingWindow* w = imageViewingWindows[i];
      
      SceneFile::SceneClass sc = w->saveScene();
      if (sc.getNumberOfSceneInfo() > 0) {
         mainWindowSceneClasses.push_back(sc);
      }
   }
   
   //
   // Save color key dialogs
   //
   saveSceneColorKeyDialog(mainWindowSceneClasses, arealEstimationColorKeyDialog);
   saveSceneColorKeyDialog(mainWindowSceneClasses, borderColorKeyDialog);
   saveSceneColorKeyDialog(mainWindowSceneClasses, cellColorKeyDialog);
   saveSceneColorKeyDialog(mainWindowSceneClasses, fociColorKeyDialog);
   saveSceneColorKeyDialog(mainWindowSceneClasses, paintColorKeyDialog);
   saveSceneColorKeyDialog(mainWindowSceneClasses, probAtlasColorKeyDialog);
   saveSceneColorKeyDialog(mainWindowSceneClasses, volumePaintColorKeyDialog);
   saveSceneColorKeyDialog(mainWindowSceneClasses, volumeProbAtlasColorKeyDialog);
   
   //
   // Process identify dialog
   //
   if (identifyDialog != NULL) {
      std::vector<SceneFile::SceneClass> scs;
      identifyDialog->saveScene(scs);
      for (unsigned int i = 0; i < scs.size(); i++) {
         if (scs[i].getNumberOfSceneInfo() > 0) {
            mainWindowSceneClasses.push_back(scs[i]);
         }
      }
   }
   
   if (displayControlDialog != NULL) {
      std::vector<SceneFile::SceneClass> dcSceneClasses = 
         displayControlDialog->saveScene();
      for (unsigned int i = 0; i < dcSceneClasses.size(); i++) {
         mainWindowSceneClasses.push_back(dcSceneClasses[i]);
      }
   }
}
     
/**
 * display the capture window as image dialog.
 */
void 
GuiMainWindow::displayCaptureWindowImageDialog()
{
   if (captureWindowImageDialog == NULL) {
      captureWindowImageDialog = new GuiCaptureWindowImageDialog(this);
      QtUtilities::positionWindowOffOtherWindow(this, 
                                                captureWindowImageDialog);
   }
   captureWindowImageDialog->show();
   captureWindowImageDialog->activateWindow();
}
      
/**
 * display an image viewing window.
 */
void 
GuiMainWindow::displayImageViewingWindow()
{
   GuiImageViewingWindow* vw = new GuiImageViewingWindow(this);
   vw->show();
   vw->activateWindow();
   
   for (unsigned int i = 0; i < imageViewingWindows.size(); i++) {
      if (imageViewingWindows[i] == NULL) {
         imageViewingWindows[i] = vw;
         return;
      }
   }
   
   imageViewingWindows.push_back(vw);
}

/**
 * remove an image viewing window.
 */
void 
GuiMainWindow::removeImageViewingWindow(const GuiImageViewingWindow* ivw)
{
   for (unsigned int i = 0; i < imageViewingWindows.size(); i++) {
      if (imageViewingWindows[i] == ivw) {
         imageViewingWindows.erase(imageViewingWindows.begin()+i);
         return;
      }
   }
}

/**
 * remove all image viewing windows.
 */
void 
GuiMainWindow::removeAllImageViewingWindows()
{
   for (unsigned int i = 0; i < imageViewingWindows.size(); i++) {
      delete imageViewingWindows[i];
   }
   imageViewingWindows.clear();
}      

/**
 * remove all model viewing windows.
 */
void 
GuiMainWindow::removeAllModelViewingWindows()
{
   const int iStart = BrainModel::BRAIN_MODEL_VIEW_AUX_WINDOW_2;
   const int iStop  = BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS;
   for (int i = iStart; i < iStop; i++) {
      if (modelWindow[i] != NULL) {
         modelWindow[i]->close();
      }
   }
   
   //
   // This allows the gui system to call the destructors for the windows
   // argument is the max time (in milliseconds) for the gui to use
   //
   qApp->processEvents();
}      

/**
 * display the set topology dialog.
 */
void 
GuiMainWindow::displaySetTopologyDialog()
{
   if (setTopologyDialog == NULL) {
      setTopologyDialog = new GuiSetTopologyDialog(this);
   }
   setTopologyDialog->show();
   setTopologyDialog->activateWindow();
}

/**
 * display the image editor window.
 */
void 
GuiMainWindow::displayImageEditorWindow()
{
   if (imageEditorWindow == NULL) {
      imageEditorWindow = new GuiImageEditorWindow(this);
   }
   imageEditorWindow->show();
   imageEditorWindow->activateWindow();
}

/**
 * display the border draw update dialog.
 */
void 
GuiMainWindow::displayBorderDrawUpdateDialog()
{
   if (borderDrawUpdateDialog == NULL) {
      borderDrawUpdateDialog = new GuiBorderDrawUpdateDialog(this);
   }
   borderDrawUpdateDialog->show();
   borderDrawUpdateDialog->activateWindow();
   
   if (borderDrawUpdateDialog->getBorderUpdateMode()
       != BrainModelBorderSet::UPDATE_BORDER_MODE_NONE) {
      getBrainModelOpenGL()->setMouseMode(GuiBrainModelOpenGL::MOUSE_MODE_BORDER_UPDATE);
   }
}      

/**
 * save a color key dialog to a scene.
 */
void 
GuiMainWindow::saveSceneColorKeyDialog(std::vector<SceneFile::SceneClass>& mainWindowSceneClasses,
                                       GuiColorKeyDialog* colorKeyDialog)
{
   if (colorKeyDialog != NULL) {
      if (colorKeyDialog->isVisible()) {
         mainWindowSceneClasses.push_back(colorKeyDialog->saveScene());
      }
   }
}

/**
 * load color key dialog from a scene.
 */
void 
GuiMainWindow::showSceneColorKeyDialog(const SceneFile::Scene* scene,
                                       const int screenMaxX,
                                       const int screenMaxY,
                                       const int mainWindowSceneX,
                                       const int mainWindowSceneY,
                                       const int mainWindowX,
                                       const int mainWindowY)
{
   //
   // close all color key dialogs
   //
   if (arealEstimationColorKeyDialog != NULL) {
      arealEstimationColorKeyDialog->close();
   }
   if (borderColorKeyDialog != NULL) {
      borderColorKeyDialog->close();
   }
   if (cellColorKeyDialog != NULL) {
      cellColorKeyDialog->close();
   }
   if (fociColorKeyDialog != NULL) {
      fociColorKeyDialog->close();
   }
   if (paintColorKeyDialog != NULL) {
      paintColorKeyDialog->close();
   }
   if (probAtlasColorKeyDialog != NULL) {
      probAtlasColorKeyDialog->close();
   }
   if (volumePaintColorKeyDialog != NULL) {
      volumePaintColorKeyDialog->close();
   }
   if (volumeProbAtlasColorKeyDialog != NULL) {
      volumeProbAtlasColorKeyDialog->close();
   }
   
   const int numClasses = scene->getNumberOfSceneClasses();
   for (int nc = 0; nc < numClasses; nc++) {
      const SceneFile::SceneClass* sc = scene->getSceneClass(nc);
      const QString className(sc->getName());
      
      if (className.indexOf("Color Key") >= 0) {
         if (className == GuiColorKeyDialog::getTitleOfColorKey(GuiColorKeyDialog::COLOR_KEY_AREAL_ESTIMATION)) {
            displayArealEstimationColorKey();
            arealEstimationColorKeyDialog->showScene(*sc,
                                                     screenMaxX,
                                                     screenMaxY,
                                                     mainWindowSceneX,
                                                     mainWindowSceneY,
                                                     mainWindowX,
                                                     mainWindowY);
         }
         else if (className == GuiColorKeyDialog::getTitleOfColorKey(GuiColorKeyDialog::COLOR_KEY_BORDERS)) {
            displayBorderColorKey();
            borderColorKeyDialog->showScene(*sc,
                                            screenMaxX,
                                            screenMaxY,
                                            mainWindowSceneX,
                                            mainWindowSceneY,
                                            mainWindowX,
                                            mainWindowY);
         }
         else if (className == GuiColorKeyDialog::getTitleOfColorKey(GuiColorKeyDialog::COLOR_KEY_CELLS)) {
            displayCellColorKey();
            cellColorKeyDialog->showScene(*sc,
                                          screenMaxX,
                                          screenMaxY,
                                          mainWindowSceneX,
                                          mainWindowSceneY,
                                          mainWindowX,
                                          mainWindowY);
         }
         else if (className == GuiColorKeyDialog::getTitleOfColorKey(GuiColorKeyDialog::COLOR_KEY_FOCI)) {
            displayFociColorKey();
            fociColorKeyDialog->showScene(*sc,
                                          screenMaxX,
                                          screenMaxY,
                                          mainWindowSceneX,
                                          mainWindowSceneY,
                                          mainWindowX,
                                          mainWindowY);
         }
         else if (className == GuiColorKeyDialog::getTitleOfColorKey(GuiColorKeyDialog::COLOR_KEY_PAINT)) {
            displayPaintColorKey();
            paintColorKeyDialog->showScene(*sc,
                                           screenMaxX,
                                           screenMaxY,
                                           mainWindowSceneX,
                                           mainWindowSceneY,
                                           mainWindowX,
                                           mainWindowY);
         }
         else if (className == GuiColorKeyDialog::getTitleOfColorKey(GuiColorKeyDialog::COLOR_KEY_PROBABILISTIC_ATLAS)) {
            displayProbabilisticAtlasColorKey();
            probAtlasColorKeyDialog->showScene(*sc,
                                               screenMaxX,
                                               screenMaxY,
                                               mainWindowSceneX,
                                               mainWindowSceneY,
                                               mainWindowX,
                                               mainWindowY);
         }
         else if (className == GuiColorKeyDialog::getTitleOfColorKey(GuiColorKeyDialog::COLOR_KEY_VOLUME_PAINT)) {
            displayVolumePaintColorKey();
            volumePaintColorKeyDialog->showScene(*sc,
                                                 screenMaxX,
                                                 screenMaxY,
                                                 mainWindowSceneX,
                                                 mainWindowSceneY,
                                                 mainWindowX,
                                                 mainWindowY);
         }
         else if (className == GuiColorKeyDialog::getTitleOfColorKey(GuiColorKeyDialog::COLOR_KEY_VOLUME_PROBABILISTIC_ATLAS)) {
            displayVolumeProbabilisticAtlasColorKey();
            volumeProbAtlasColorKeyDialog->showScene(*sc,
                                                     screenMaxX,
                                                     screenMaxY,
                                                     mainWindowSceneX,
                                                     mainWindowSceneY,
                                                     mainWindowX,
                                                     mainWindowY);
         }
      }
   }
}
      
/**
 * display the areal estimation color key.
 */
void 
GuiMainWindow::displayArealEstimationColorKey()
{
   if (arealEstimationColorKeyDialog == NULL) {
      arealEstimationColorKeyDialog = new GuiColorKeyDialog(this, GuiColorKeyDialog::COLOR_KEY_AREAL_ESTIMATION);
   }
   arealEstimationColorKeyDialog->updateDialog();
   arealEstimationColorKeyDialog->show();
   arealEstimationColorKeyDialog->activateWindow();
}

/**
 * display the border color key.
 */
void 
GuiMainWindow::displayBorderColorKey()
{
   if (borderColorKeyDialog == NULL) {
      borderColorKeyDialog = new GuiColorKeyDialog(this, GuiColorKeyDialog::COLOR_KEY_BORDERS);
   }
   borderColorKeyDialog->updateDialog();
   borderColorKeyDialog->show();
   borderColorKeyDialog->activateWindow();
}

/**
 * display the cell color key.
 */
void 
GuiMainWindow::displayCellColorKey()
{
   if (cellColorKeyDialog == NULL) {
      cellColorKeyDialog = new GuiColorKeyDialog(this, GuiColorKeyDialog::COLOR_KEY_CELLS);
   }
   cellColorKeyDialog->updateDialog();
   cellColorKeyDialog->show();
   cellColorKeyDialog->activateWindow();
}

/**
 * display the foci color key.
 */
void 
GuiMainWindow::displayFociColorKey()
{
   if (fociColorKeyDialog == NULL) {
     fociColorKeyDialog  = new GuiColorKeyDialog(this, GuiColorKeyDialog::COLOR_KEY_FOCI);
   }
   fociColorKeyDialog->updateDialog();
   fociColorKeyDialog->show();
   fociColorKeyDialog->activateWindow();
}

/**
 * display the paint color key.
 */
void 
GuiMainWindow::displayPaintColorKey()
{
   if (paintColorKeyDialog == NULL) {
      paintColorKeyDialog = new GuiColorKeyDialog(this, GuiColorKeyDialog::COLOR_KEY_PAINT);
   }
   paintColorKeyDialog->updateDialog();
   paintColorKeyDialog->show();
   paintColorKeyDialog->activateWindow();
}

/**
 * display the foci attribute assignment dialog.
 */
void 
GuiMainWindow::displayFociAttributeAssignmentDialog()
{
   if (fociAttributeAssignmentDialog == NULL) {
      fociAttributeAssignmentDialog = new GuiCellAndFociAttributeAssignmentDialog(this, true);
   }
   fociAttributeAssignmentDialog->updateDialog();
   fociAttributeAssignmentDialog->show();
   fociAttributeAssignmentDialog->activateWindow();
}
      
/**
 * display the probabilistic atlas color key.
 */
void 
GuiMainWindow::displayProbabilisticAtlasColorKey()
{
   if (probAtlasColorKeyDialog == NULL) {
      probAtlasColorKeyDialog = new GuiColorKeyDialog(this, GuiColorKeyDialog::COLOR_KEY_PROBABILISTIC_ATLAS);
   }
   probAtlasColorKeyDialog->updateDialog();
   probAtlasColorKeyDialog->show();
   probAtlasColorKeyDialog->activateWindow();
}

/**
 * display the volume paint color key.
 */
void 
GuiMainWindow::displayVolumePaintColorKey()      
{
   if (volumePaintColorKeyDialog == NULL) {
     volumePaintColorKeyDialog  = new GuiColorKeyDialog(this, GuiColorKeyDialog::COLOR_KEY_VOLUME_PAINT);
   }
   volumePaintColorKeyDialog->updateDialog();
   volumePaintColorKeyDialog->show();
   volumePaintColorKeyDialog->activateWindow();
}

/**
 * display the volume probabilistic atlas color key.
 */
void 
GuiMainWindow::displayVolumeProbabilisticAtlasColorKey()
{
   if (volumeProbAtlasColorKeyDialog == NULL) {
      volumeProbAtlasColorKeyDialog = new GuiColorKeyDialog(this, GuiColorKeyDialog::COLOR_KEY_VOLUME_PROBABILISTIC_ATLAS);
   }
   volumeProbAtlasColorKeyDialog->updateDialog();
   volumeProbAtlasColorKeyDialog->show();
   volumeProbAtlasColorKeyDialog->activateWindow();
}

/**
 * display the vocabulary editor dialog.
 */
void 
GuiMainWindow::displayVocabularyFileEditorDialog()
{
   if (vocabularyFileEditorDialog == NULL) {
      vocabularyFileEditorDialog = new GuiVocabularyFileEditorDialog(this);
   }
   vocabularyFileEditorDialog->updateDialog();
   vocabularyFileEditorDialog->show();
   vocabularyFileEditorDialog->activateWindow();
}

/**
 * display volume bias correction dialog.
 */
void 
GuiMainWindow::displayVolumeBiasCorrectionDialog()
{
   if (volumeBiasCorrectionDialog == NULL) {
      volumeBiasCorrectionDialog = new GuiVolumeBiasCorrectionDialog(this);
   }
   volumeBiasCorrectionDialog->show();
   volumeBiasCorrectionDialog->activateWindow();
}

/**
 * identify a node provided by a remote program.
 */
void 
GuiMainWindow::identifyNodeFromRemoteProgram(const int nodeNumber)
{
   BrainSet* bs = getBrainSet(BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW);
   
   //
   // Display cross at the node
   //
   bs->setDisplayCrossForNode(nodeNumber,
                              NULL);
                              
   //
   // Place a symbol at the node
   //
   BrainSetNodeAttribute* bna = bs->getNodeAttributes(nodeNumber);
   bna->setHighlighting(BrainSetNodeAttribute::HIGHLIGHT_NODE_REMOTE);
   
   //
   // Display identification information in the Identify Window
   //
   BrainModelIdentification* brainID = bs->getBrainModelIdentification();
   const QString idString = brainID->getIdentificationTextForNode(bs,
                                                                  nodeNumber,
                                                                  true,
                                                                  true);
   if (idString.isEmpty() == false) {
      GuiIdentifyDialog* id = getIdentifyDialog(true);
      id->appendHtml(idString);
   }

   //
   // Redraw
   //
   GuiBrainModelOpenGL::updateAllGL();
}
      
/**
 * redraw all windows using the brain set.
 */
void 
GuiMainWindow::slotRedrawWindowsUsingBrainSet(BrainSet* bs)
{
   
   for (int i = 0; i < BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; i++) {
      GuiBrainModelOpenGL* gl = 
         mainOpenGL->getBrainModelOpenGLForWindow(static_cast<BrainModel::BRAIN_MODEL_VIEW_NUMBER>(i));
      if (gl != NULL) {
         if (gl->getBrainSet() == bs) {
            GuiBrainModelOpenGL::updateAllGL(gl);
         }
      }
   }
}

/**
 * Display the connectivity dialog.
 */
void
GuiMainWindow::displayConnectivityDialog()
{
   if (connectivityDialog == NULL) {
      connectivityDialog = new GuiConnectivityDialog(this);
   }
   connectivityDialog->show();
   connectivityDialog->activateWindow();
}

