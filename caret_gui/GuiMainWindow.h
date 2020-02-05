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


#ifndef __VE_GUI_MAIN_WINDOW_H__
#define __VE_GUI_MAIN_WINDOW_H__

class QCloseEvent;
class QLabel;
class GuiToolBar;

#include <QMainWindow>

#include "GuiBrainModelOpenGL.h"
#include "SceneFile.h"
#include "SpecFile.h"

//
// Forward declarations avoid having to include lots
// of include files.
//
class CommunicatorClientAFNI;
class CommunicatorClientFIV;
class CommunicatorServerCaret;
class CommunicatorServerCaretSocket;
class GuiAddCellsDialog;
class GuiAlignSurfaceToStandardOrientationDialog;
class GuiAutomaticRotationDialog;
class GuiBorderDrawUpdateDialog;
class GuiBorderOperationsDialog;
class GuiBordersCreateInterpolatedDialog;
class GuiCaptureWindowImageDialog;
class GuiCaretCommandDialog;
class GuiCaretCommandScriptBuilderDialog;
class GuiCellAndFociAttributeAssignmentDialog;
class GuiContourAlignmentDialog;
class GuiContourDrawDialog;
class GuiContourSectionControlDialog;
class GuiContourSetScaleDialog;
class GuiDrawBorderDialog;
class GuiIdentifyDialog;
class GuiDisplayControlDialog;
class GuiBordersCreateInterpolatedDialog;
class GuiBrainModelViewingWindow;
class GuiColorKeyDialog;
class GuiDataFileMathDialog;
class GuiConnectivityDialog;
class GuiFilesModified;
class GuiFlattenFullHemisphereDialog;
class GuiImageEditorWindow;
class GuiImageViewingWindow;
class GuiInterpolateSurfacesDialog;
class GuiMainWindowAttributesActions;
class GuiMainWindowAttributesMenu;
class GuiMainWindowCommActions;
class GuiMainWindowCommMenu;
class GuiMainWindowFileActions;
class GuiMainWindowFileMenu;
class GuiMainWindowHelpActions;
class GuiMainWindowHelpMenu;
class GuiMainWindowLayersActions;
class GuiMainWindowLayersMenu;
class GuiMainWindowSurfaceActions;
class GuiMainWindowSurfaceMenu;
class GuiMainWindowTimingActions;
class GuiMainWindowTimingMenu;
class GuiMainWindowVolumeActions;
class GuiMainWindowVolumeMenu;
class GuiMainWindowWindowActions;
class GuiMainWindowWindowMenu;
class GuiMapStereotaxicFocusDialog;
class GuiMetricModificationDialog;
class GuiMetricsToRgbPaintDialog;
class GuiModelsEditorDialog;
class GuiMorphingDialog;
class GuiPaintNameEditorDialog;
class GuiPaletteEditorDialog;
class GuiParamsFileEditorDialog;
class GuiSectionControlDialog;
class GuiStandardMeshDialog;
class GuiSurfaceRegionOfInterestDialog;
class GuiSurfaceRegionOfInterestDialogOLD;
class GuiPreferencesDialog;
class GuiRecordingDialog;
class GuiSetTopologyDialog;
class GuiSmoothingDialog;
//class GuiSpeechGenerator;
class GuiStudyCollectionFileEditorDialog;
class GuiStudyMetaDataFileEditorDialog;
class GuiTrajectoryToolDialog;
class GuiTransformationMatrixDialog;
class GuiVocabularyFileEditorDialog;
class GuiVolumeBiasCorrectionDialog;
class GuiVolumeResizingDialog;
class GuiVolumeThresholdSegmentationDialog;
class GuiVolumeAttributesDialog;
class GuiVolumePaintEditorDialog;
class GuiVolumeSegmentationEditorDialog;
class GuiVolumeRegionOfInterestDialog;
class GuiHelpViewerWindow;
class TransformationMatrix;

/// GuiMainWindow class
/**
 * MainWindow for the graphical user-interface
 */
class GuiMainWindow : public QMainWindow {

   Q_OBJECT
   
   public:
      /// Constructor
      GuiMainWindow(const bool enableTimingMenu,
                    const int openGLsizeX,
                    const int openGLsizeY);
      
      /// Destructor
      ~GuiMainWindow();
      
      /// add a brain set
      void addBrainSet(BrainSet* bs);
      
      /// get the number of brain set
      int getNumberOfBrainSets() const { return loadedBrainSets.size(); }
      
      /// get a brain set
      BrainSet* getBrainSetByIndex(const int indx) { return loadedBrainSets[indx]; }
      
      /// get the available brain sets
      void getAllBrainSets(std::vector<BrainSet*>& brainSetsOut) { brainSetsOut = loadedBrainSets; }
      
      /// get the active brain structure for specified window
      BrainSet* getBrainSet(const BrainModel::BRAIN_MODEL_VIEW_NUMBER windowNumber =
                                                BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW);
      
      /// get the active brain structure (const method)
      const BrainSet* getBrainSet(const BrainModel::BRAIN_MODEL_VIEW_NUMBER windowNumber =
                                                BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW) const;
      
      /// set the active brain structure for specified window
      void setBrainSet(const BrainModel::BRAIN_MODEL_VIEW_NUMBER windowNumber,
                       BrainSet* newActiveBrainSet);
             
      /// identify a node provided by a remote program
      void identifyNodeFromRemoteProgram(const int nodeNumber);
      
      /// Update gui due to file changes
      void fileModificationUpdate(const GuiFilesModified& fm);
       
      /// get the file menu
      GuiMainWindowFileMenu* getMainWindowFileMenu() { return fileMenu; }
      
      /// get the layers menu
      GuiMainWindowLayersMenu* getMainWindowLayersMenu() { return layersMenu; }
      
      /// get the surface popup menu
      GuiMainWindowSurfaceMenu* getMainWindowSurfaceMenu() { return surfaceMenu; }
      
      /// get the volume popup menu
      GuiMainWindowVolumeMenu* getMainWindowVolumeMenu() { return volumeMenu; }
      
      /// Get the Brain Model OpenGL widget in the main window
      GuiBrainModelOpenGL* getBrainModelOpenGL() { return mainOpenGL; }
      
      /// Get the Brain Model Contours in the main window (NULL if not a contours)
      BrainModelContours* getBrainModelContours();
      
      /// Get the Brain Model Surface in the main window (NULL if not a surface)
      BrainModelSurface* getBrainModelSurface();
      
      /// Get the Brain Model Surface Volume in the main window (NULL if not a surf&vol)
      BrainModelSurfaceAndVolume* getBrainModelSurfaceAndVolume();
      
      /// Get the Brain Model Volume in the main window (NULL if not a volume)
      BrainModelVolume* getBrainModelVolume();
      
      /// Get the Brain Model in the main window (NULL if no model)
      BrainModel* getBrainModel();
      
      /// Get the index of the Brain Model Volume in the main window (-1 if invalid)
      int getBrainModelIndex() const;
      
      /// Get the toolbar
      GuiToolBar* getToolBar() { return toolBar; }
      
      /// create, possibly show, and return the add cells dialog
      GuiAddCellsDialog* getAddCellsDialog(const bool showIt);
      
      /// create, possibly show, and return the add contour cells dialog
      GuiAddCellsDialog* getAddContourCellsDialog(const bool showIt);
      
      /// create, possibly show, and return the contour alignment dialog
      GuiContourAlignmentDialog* getContourAlignmentDialog(const bool showIt);
      
      /// create, possibly show, and return the contour draw dialog
      GuiContourDrawDialog* getContourDrawDialog(const bool showIt);
      
      /// create, possibly show, and return the contour set scale dialog
      GuiContourSetScaleDialog* getContourSetScaleDialog(const bool showIt);
      
      /// create, possibly show, and return the contour section control dialog
      GuiContourSectionControlDialog* getContourSectionControlDialog(const bool showIt);
      
      /// create, possibly show, and return the draw border dialog
      GuiDrawBorderDialog* getDrawBorderDialog(const bool showIt);
      
      /// create, possibly show, and return the border operations dialog
      GuiBorderOperationsDialog* getBorderOperationsDialog(const bool showIt);
      
      /// create, possibly show, and return the flatten full hemisphere dialog
      GuiFlattenFullHemisphereDialog* getFlattenFullHemisphereDialog(const bool showIt);
      
      /// create, possibly show, and return the standard mesh dialog
      GuiStandardMeshDialog* getStandardMeshDialog(const bool showIt);
      
      /// get the draw border updated dialog
      GuiBorderDrawUpdateDialog* getDrawBorderUpdateDialog() { return borderDrawUpdateDialog; }
      
      /// show the help viewer dialog
      void showHelpViewerDialog(const QString& helpPage = "");
      
      /// show a page in the help viewer dialog over a modal dialog
      void showHelpPageOverModalDialog(QDialog* modalParent,
                                     const QString& helpPage);
                            
      /// create, (possibly show), and return the identify dialog
      GuiIdentifyDialog* getIdentifyDialog(const bool showIt);

      /// create, (possibly show), and return the border created interpolated dialog
      GuiBordersCreateInterpolatedDialog* getBordersCreateInterpolatedDialog(const bool showIt);
      
      /// create, (possibly show), and return the map stereotaxic focus dialog
      GuiMapStereotaxicFocusDialog* getMapStereotaxicFocusDialog(const bool showIt);
      
      /// create, (possibly show), and return the metrics to rgb paint dialog
      GuiMetricsToRgbPaintDialog* getMetricsToRgbPaintDialog(const bool showIt);
      
      /// create, (possibly show), and return the recording dialog
      GuiRecordingDialog* getRecordingDialog(const bool showIt);
      
      /// create, (possibly show), and return the surface region of interest dialog
      GuiSurfaceRegionOfInterestDialog* getSurfaceRegionOfInterestDialog(const bool showIt);
      
      /// create, (possibly show), and return the surface region of interest dialog
      GuiSurfaceRegionOfInterestDialogOLD* getSurfaceRegionOfInterestDialogOLD(const bool showIt);
      
      /// create, (possibly show), and return the volume region of interest dialog
      GuiVolumeRegionOfInterestDialog* getVolumeRegionOfInterestDialog(const bool showIt);

      GuiTrajectoryToolDialog* getTrajectoryToolDialog(const bool showIt = true);

      
      /// create, (possibly show), and return the threshold segmentation dialog
      GuiVolumeThresholdSegmentationDialog* getVolumeThresholdSegmentationDialog(const bool showIt);
      
      /// create, (possibly show), and return the metric modification dialog
      GuiMetricModificationDialog* getMetricModificationDialog(const bool showIt);
      
      /// create, (possibly show), and return the shape modification dialog
      GuiMetricModificationDialog* getShapeModificationDialog(const bool showIt);
      
      /// create, (possibly show), and return the interpolate surface dialog
      GuiInterpolateSurfacesDialog* getInterpolateSurfaceDialog(const bool showIt);

      /// get the align surface to standard orientation dialog
      GuiAlignSurfaceToStandardOrientationDialog* getAlignSurfaceToStandardOrientationDialog() {
         return alignSurfaceToStandardOrientationDialog;
      }
      
      /// create, (possibly show), and return the volume attributes dialog
      GuiVolumeAttributesDialog* getVolumeAttributesDialog(const bool showIt);
      
      /// Return the volume resizing dialog.  Dialog will be created only if parameter is true
      GuiVolumeResizingDialog* getVolumeResizingDialog(const bool createIt);
      
      /// Return the segmentation volume editor dialog
      GuiVolumeSegmentationEditorDialog* getVolumeSegmentationEditorDialog(const bool showIt);
      
      /// Return the paint volume editor dialog
      GuiVolumePaintEditorDialog* getVolumePaintEditorDialog(const bool showIt);
      
      /// set the align surface to standard orientation dialog
      void setAlignSurfaceToStandardOrientationDialog(GuiAlignSurfaceToStandardOrientationDialog* aod) {
         alignSurfaceToStandardOrientationDialog = aod;
      }
      
      ///  load the spec file's data files
      void loadSpecFilesDataFiles(SpecFile sf, 
                                  const TransformationMatrix* tm,
                                  const bool appendToExistingLoadedSpecFiles = false);
            
      /// read the spec file with the specified name and place in spec file dialog
      void readSpecFile(const QString& filename);
      
      /// update the section dialog
      void updateSectionControlDialog();
      
      /// Update the status bar label
      void updateStatusBarLabel();
      
      /// Update the display control dialog
      void updateDisplayControlDialog();
      
      /// display the brain model in the main window
      void displayBrainModelInMainWindow(BrainModel* bm);
      
      /// Display a volume in the main window
      void displayVolumeInMainWindow();

      /// Display contours in the main window
      void displayContoursInMainWindow();
      
      /// Display the newest surface in the main window.
      void displayNewestSurfaceInMainWindow();

      /// display a web page in the user's web browser
      void displayWebPage(const QString& webPage);
      
      /// get the afni communicator
      CommunicatorClientAFNI* getAfniClientCommunicator() { return afniClientCommunicator; }
      
      /// get the FIV communicator
      CommunicatorClientFIV* getFivClientCommunicator() { return fivClientCommunicator; }
      
      /// speak some text
      void speakText(const QString& text, const bool verboseSpeech);
      
      /// apply a scene (set display settings)
      void showScene(const SceneFile::Scene* scene, 
                     const bool checkSpecFlag,
                     QString& errorMessage) ;
      
      /// create a scene (save window info)
      void saveScene(std::vector<SceneFile::SceneClass>& mainWindowSceneClasses);
                       
      /// Check for modified files.
      void checkForModifiedFiles(BrainSet* bs,
                                 QString& msg, const bool checkSceneFileFlag);
      
      /// remove an image viewing window
      void removeImageViewingWindow(const GuiImageViewingWindow* ivw);
      
      /// remove all image viewing windows
      void removeAllImageViewingWindows();
      
      /// remove all model viewing windows
      void removeAllModelViewingWindows();
      
      /// close the spec file 
      void closeSpecFile(const bool keepSceneAndSpec,
                         const bool checkForModifiedFiles = true);
     
      /// close all spec files
      void closeAllSpecFiles();
      
      /// see if communicator with other Caret's valid
      bool getCaretCommunicatorServerValid() const { return (communicatorServerCaret != NULL); }
      
      /// get the main window file actions
      GuiMainWindowFileActions* getFileActions() { return fileActions; }
      
      /// get the main window attribute actions
      GuiMainWindowAttributesActions* getAttributesActions() { return attributeActions; }
      
      /// get the main window layers actions
      GuiMainWindowLayersActions* getLayersActions() { return layersActions; }
      
      /// get the main window help actions
      GuiMainWindowHelpActions* getHelpActions() { return helpActions; }

      /// get the main window surface actions
      GuiMainWindowSurfaceActions* getSurfaceActions() { return surfaceActions; }
      
      /// get the main window timing actions
      GuiMainWindowTimingActions* getTimingActions() { return timingActions; }
      
      /// get the main window volume actions
      GuiMainWindowVolumeActions* getVolumeActions() { return volumeActions; }
      
      /// get the main window window actions
      GuiMainWindowWindowActions* getWindowActions() { return windowActions; }
      
      /// get the main window comm actions
      GuiMainWindowCommActions* getCommActions() { return commActions; }
      
   protected:
      /// Called when application wants to close.
      void closeEvent(QCloseEvent* event);
   
   public slots:
      /// close the program
      void slotCloseProgram();
      
      /// display the models editor
      void displayModelsEditorDialog();
      
      /// display the vocabulary editor dialog
      void displayVocabularyFileEditorDialog();
      
      /// display an image viewing window
      void displayImageViewingWindow();
      
      /// display the capture window as image dialog
      void displayCaptureWindowImageDialog();
      
      /// display the border draw update dialog
      void displayBorderDrawUpdateDialog();
      
      /// display the areal estimation color key
      void displayArealEstimationColorKey();
      
      /// display the border color key
      void displayBorderColorKey();

      /// create, (possibly show), and return the electrode trajectory tool dialog
      void displayTrajectoryToolDialog();
      
      /// display the cell color key
      void displayCellColorKey();
      
      /// display the foci color key
      void displayFociColorKey();
      
      /// display the paint color key
      void displayPaintColorKey();
      
      /// display the foci attribute assignment dialog
      void displayFociAttributeAssignmentDialog();
      
      /// display the probabilistic atlas color key
      void displayProbabilisticAtlasColorKey();
      
      /// display the study meta data file editor dialog
      void displayStudyMetaDataFileEditorDialog();
      
      /// display the study collection file editor dialog
      void displayStudyCollectionFileEditorDialog();
      
      /// display the volume paint color key
      void displayVolumePaintColorKey();
      
      /// display the volume probabilistic atlas color key
      void displayVolumeProbabilisticAtlasColorKey();
      
      /// create (if necessary) and show the automatic rotation dialog
      void displayAutomaticRotationDialog();

      /// create (if necessary) and show the draw border dialog
      void displayDrawBorderDialog();
      
      /// create (if necessary) and show the border operations dialog
      void displayBorderOperationsDialog();
      
      /// create (if necessary) and show the caret command executor dialog
      void displayCaretCommandExecutorDialog();
      
      /// create (if necessary) and show the caret command script builder dialog
      void displayCaretCommandScriptBuilderDialog();
      
      /// create (if necessary) and show the standard mesh dialog
      void displayStandardMeshDialog();
      
      /// create (if necessary) and show the flatten full hemisphere dialog
      void displayFlattenFullHemisphereDialog();
      
      /// create (if necessary) and show the section control dialog
      void displaySectionControlDialog();
      
      /// create (if necessary) and show the smoothing dialog
      void displaySmoothingDialog();
      
      /// create (if necessary) and show the identify dialog
      void displayIdentifyDialog();

      /// create (if necessary) and show the preferences dialog
      void displayPreferencesDialog();

      /// create (if necessary) and show the display control dialog
      void displayDisplayControlDialog();
           
      /// create and display the palette editor dialog
      void displayPaletteEditorDialog();
      
      /// display the set topology dialog
      void displaySetTopologyDialog();
      
      /// display the paint editor dialog
      void displayPaintEditorDialog();
      
      /// display the tranformation editor dialog
      void displayTransformMatrixEditor();
      
      /// display the params file editor dialog
      void displayParamsFileEditorDialog();
      
      /// Display the fast open data file dialog
      void displayFastOpenDataFileDialog();
      
      /// display metric math dialog
      void displayMetricMathDialog();
      
      /// display surface shape math dialog
      void displayShapeMathDialog();
      
      /// display volume math dialog
      void displayVolumeMathDialog();
      
      /// display volume bias correction dialog
      void displayVolumeBiasCorrectionDialog();
      
      /// Close the current spec file.
      void slotCloseSpecFile();
      
      /// Close the current spec file but keep the scene and spec file.
      void slotCloseSpecFileKeepSceneAndSpec();
      
      /// display the image editor window
      void displayImageEditorWindow();
       
      /// show the flat morphing dialog
      void showFlatMorphingDialog();
      
      /// show the sphere morphing dialog
      void showSphereMorphingDialog();
      
      /// update the displayed menus based upon loaded contours/surface/volume in main window
      void updateDisplayedMenus();
            
      /// display the brain model in the main window (negative displays newest)
      void displayBrainModelInMainWindow(int modelNumberIn);
      
      /// Makes updates when the spec file is changed.
      void postSpecFileReadInitializations();
      
      /// redraw all windows using the brain set
      void slotRedrawWindowsUsingBrainSet(BrainSet* bs);

      /// display the connectivity dialog
      void displayConnectivityDialog();

   public:      
      /// update the transformation matrix Editor 
      void updateTransformationMatrixEditor(const TransformationMatrix* tm = NULL);
         
      /// get the transformation matrix editor dialog
      GuiTransformationMatrixDialog* getTransformMatrixEditor()
         { return transformMatrixEditorDialog; }
      
      /// Called when an item is selected from the recent spec file menu
      void recentSpecFileMenuSelection(int menuItem);
      
      /// save a color key dialog to a scene
      void saveSceneColorKeyDialog(std::vector<SceneFile::SceneClass>& mainWindowSceneClasses,
                                     GuiColorKeyDialog* colorKeyDialog);
      
      /// load color key dialog from a scene
      void showSceneColorKeyDialog(const SceneFile::Scene* scene,
                                   const int screenMaxX,
                                   const int screenMaxY,
                                   const int mainWindowSceneX,
                                   const int mainWindowSceneY,
                                   const int mainWindowX,
                                   const int mainWindowY);
      
      /// Called to pop up viewing window
      void showViewingWindow(const BrainModel::BRAIN_MODEL_VIEW_NUMBER item);

      /// Remove a viewing window
      void removeViewingWindow(const BrainModel::BRAIN_MODEL_VIEW_NUMBER item);
      
      /// resize the viewing windows
      void resizeViewingWindows();
      
   private:
      /// the available brain set
      std::vector<BrainSet*> loadedBrainSets;
      
      /// the directory for the brain set
      //std::vector<QString> loadedBrainSetDirectory;
      
      /// the brain sets associated with each viewing window
      BrainSet* brainSetInWindow[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
      
      /// communicator with other Caret's
      CommunicatorServerCaret* communicatorServerCaret;
      
      /// Recent spec files
      std::vector<QString> recentSpecFiles;
      
      /// update recent spec files for recent spec file menu
      void addToRecentSpecFiles(const QString& name);      
      
      /// Fixed size policy
      QSizePolicy* sizePolicyFixed;
      
      /// OpenGL window for drawing brain surface
      GuiBrainModelOpenGL* mainOpenGL;
      
      /// Display Control Dialog
      GuiDisplayControlDialog* displayControlDialog;
      
      /// Surface Windows
      GuiBrainModelViewingWindow* modelWindow[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
      
      /// Tool Bar
      GuiToolBar* toolBar;
      
      /// Attributes Menu
      GuiMainWindowAttributesMenu* attributesMenu;
      
      /// Comm Menu
      GuiMainWindowCommMenu* commMenu;
      
      /// File Menu
      GuiMainWindowFileMenu* fileMenu;
      
      /// Layers Menu
      GuiMainWindowLayersMenu* layersMenu;
      
      /// Help Menu
      GuiMainWindowHelpMenu* helpMenu;
      
      /// surface popup menu
      GuiMainWindowSurfaceMenu* surfaceMenu;
      
      /// timing popup menu
      GuiMainWindowTimingMenu* timingMenu;
      
      /// volume menu
      GuiMainWindowVolumeMenu* volumeMenu;
      
      /// window menu
      GuiMainWindowWindowMenu* windowMenu;
      
      /// align surface to standard orientation dialog
      GuiAlignSurfaceToStandardOrientationDialog* alignSurfaceToStandardOrientationDialog;
      
      /// automatic rotation dialog
      GuiAutomaticRotationDialog* automaticRotationDialog;
      
      /// add cells dialog
      GuiAddCellsDialog* addCellsDialog;
      
      /// add contour cells dialog
      GuiAddCellsDialog* addContourCellsDialog;
      
      /// capture window image dialog
      GuiCaptureWindowImageDialog* captureWindowImageDialog;
      
      /// contour alignment dialog
      GuiContourAlignmentDialog* contourAlignmentDialog;
      
      /// contour draw dialog
      GuiContourDrawDialog* contourDrawDialog;
      
      /// contour set scale dialog
      GuiContourSetScaleDialog* contourSetScaleDialog;
      
      /// contour section control dialog
      GuiContourSectionControlDialog* contourSectionControlDialog;
      
      /// flatten full hemisphere dialog
      GuiFlattenFullHemisphereDialog* flattenFullHemisphereDialog;
      
      /// standard mesh dialog
      GuiStandardMeshDialog* standardMeshDialog;
      
      /// border operations dialog
      GuiBorderOperationsDialog* borderOperationsDialog;
      
      /// draw border dialog
      GuiDrawBorderDialog* drawBorderDialog;

      /// connectivity dialog
      GuiConnectivityDialog* connectivityDialog;

      /// help viewer dialog
      GuiHelpViewerWindow* helpViewerDialog;
      
      /// Identify window
      GuiIdentifyDialog* identifyDialog;
      
      /// recording dialog
      GuiRecordingDialog* recordingDialog;
      
      /// smoothing dialog
      GuiSmoothingDialog* smoothingDialog;

      /// electrode trajectory tool dialog
      GuiTrajectoryToolDialog *trajectoryToolDialog;
      
      /// borders create interpolated dialog
      GuiBordersCreateInterpolatedDialog* bordersCreateInterpolatedDialog;
      
      /// border draw update dialog
      GuiBorderDrawUpdateDialog* borderDrawUpdateDialog;
      
      /// map talairach focus dialog
      GuiMapStereotaxicFocusDialog* mapStereotaxicFocusDialog;
      
      /// metrics to rgb paint dialog
      GuiMetricsToRgbPaintDialog* metricsToRgbPaintDialog;
      
      /// surface region of interest dialog
      GuiSurfaceRegionOfInterestDialog* surfaceRegionOfInterestDialog;
      
      /// surface region of interest dialog
      GuiSurfaceRegionOfInterestDialogOLD* surfaceRegionOfInterestDialogOLD;
      
      /// volume region of interest dialog
      GuiVolumeRegionOfInterestDialog* volumeRegionOfInterestDialog;
      
      /// volume threshold segmentation dialog
      GuiVolumeThresholdSegmentationDialog* volumeThresholdSegmentationDialog;
      
      /// the edit transformation matrix dialog
      GuiTransformationMatrixDialog* transformMatrixEditorDialog;
      
      /// metric modification dialog
      GuiMetricModificationDialog* metricModificationDialog;
      
      /// shape modification dialog
      GuiMetricModificationDialog* shapeModificationDialog;
      
      /// paint name and attributes dialog
      GuiPaintNameEditorDialog* paintNameEditorDialog;
      
      /// metric math dialog
      GuiDataFileMathDialog* metricMathDialog;
      
      /// models editor dialog
      GuiModelsEditorDialog* modelsEditorDialog;
      
      /// palette editor dialog
      GuiPaletteEditorDialog* paletteEditorDialog;
      
      /// surface shape math dialog
      GuiDataFileMathDialog* shapeMathDialog;
      
      /// study meta data editor dialog
      GuiStudyMetaDataFileEditorDialog* studyMetaDataFileEditorDialog;
      
      /// study collection editor dialog
      GuiStudyCollectionFileEditorDialog* studyCollectionFileEditorDialog;
      
      /// volume math dialog
      GuiDataFileMathDialog* volumeMathDialog;
      
      /// interpolate surface dialog
      GuiInterpolateSurfacesDialog* interpolateSurfaceDialog;
      
      /// set topology dialog
      GuiSetTopologyDialog* setTopologyDialog;
      
      /// flat morphing dialog
      GuiMorphingDialog* flatMorphingDialog;
      
      /// sphere morphing dialog
      GuiMorphingDialog* sphereMorphingDialog;
      
      /// preferences dialog
      GuiPreferencesDialog* preferencesDialog;
      
      /// caret command executor dialog
      GuiCaretCommandDialog* caretCommandExecutorDialog;
      
      /// caret command script builder dialog
      GuiCaretCommandScriptBuilderDialog* caretCommandScriptBuilderDialog;
      
      /// section control dialog
      GuiSectionControlDialog* sectionControlDialog;
      
      /// volume attributes dialog
      GuiVolumeAttributesDialog* volumeAttributesDialog;
      
      /// volume resizing dialog
      GuiVolumeResizingDialog* volumeResizingDialog;
      
      /// volume segmentation editor dialog
      GuiVolumeSegmentationEditorDialog* volumeSegmentationEditorDialog;
      
      /// volume paint editor dialog
      GuiVolumePaintEditorDialog* volumePaintEditorDialog;
      
      /// params file editor dialog
      GuiParamsFileEditorDialog* paramsFileEditorDialog;
      
      /// vocabulary editor dialog
      GuiVocabularyFileEditorDialog* vocabularyFileEditorDialog;
      
      /// areal estimation color key dialog
      GuiColorKeyDialog* arealEstimationColorKeyDialog;
      
      /// border color key dialog
      GuiColorKeyDialog* borderColorKeyDialog;
      
      /// cell color key dialog
      GuiColorKeyDialog* cellColorKeyDialog;
      
      /// foci color key dialog
      GuiColorKeyDialog* fociColorKeyDialog;
      
      /// paint color key dialog
      GuiColorKeyDialog* paintColorKeyDialog;
      
      /// probabilistic atlas color key dialog
      GuiColorKeyDialog* probAtlasColorKeyDialog;
      
      /// volume paint color key dialog
      GuiColorKeyDialog* volumePaintColorKeyDialog;
      
      /// volume probabilistic atlas color key dialog
      GuiColorKeyDialog* volumeProbAtlasColorKeyDialog;
      
      /// foci attribute assignemnt dialog
      GuiCellAndFociAttributeAssignmentDialog* fociAttributeAssignmentDialog;
      
      /// volume bias correction dialog
      GuiVolumeBiasCorrectionDialog* volumeBiasCorrectionDialog;
      
      /// status bar mouse mode label
      QLabel* statusBarMouseModeLabel;
      
      /// status bar section low label
      QLabel* statusBarSectionLowLabel;
      
      /// status bar section high label
      QLabel* statusBarSectionHighLabel;
      
      /// status bar mouse left button function label
      QLabel* statusBarMouseLeftLabel;
      
      /// status bar mouse shift left button function label
      QLabel* statusBarMouseShiftLeftLabel;
      
      /// status bar mouse alt left button function label
      QLabel* statusBarMouseAltLeftLabel;
      
      /// status bar mouse ctrl left button function label
      QLabel* statusBarMouseCtrlLeftLabel;
      
      /// status bar mouse click left button function label
      QLabel* statusBarMouseClickLeftLabel;
      
      /// the afni communicator
      CommunicatorClientAFNI* afniClientCommunicator;
      
      /// the FIV communicator
      CommunicatorClientFIV* fivClientCommunicator;

      /// the speech generator
      //GuiSpeechGenerator *speechGenerator;
      
      /// the image editing window
      GuiImageEditorWindow* imageEditorWindow;
      
      /// main window file actions
      GuiMainWindowFileActions* fileActions;
      
      /// main window attribute actions
      GuiMainWindowAttributesActions* attributeActions;
      
      /// main window layers actions
      GuiMainWindowLayersActions* layersActions;
      
      /// main window help actions
      GuiMainWindowHelpActions* helpActions;
      
      /// main window surface actions
      GuiMainWindowSurfaceActions* surfaceActions;
      
      /// main window window actions
      GuiMainWindowWindowActions* windowActions;
      
      /// main window comm actions
      GuiMainWindowCommActions* commActions;
      
      /// main window timing actions
      GuiMainWindowTimingActions* timingActions;
      
      /// main window volume actions
      GuiMainWindowVolumeActions* volumeActions;
      
      /// keeps track of image viewing windows
      std::vector<GuiImageViewingWindow*> imageViewingWindows;
      
      /// Check data file modified when quitting
      void checkFileModified(const QString typeName,
                             const AbstractFile* af, QString& msg);
      
      /// check volume file modified when quitting
      void checkVolumeFileModified(const QString& typeName,
                                   const VolumeFile* vf, QString& msg);      
      
      /// create the status bar
      void createStatusBar();
      
   friend class GuiCaptureWindowImageDialog;
   friend class GuiMainWindowFileMenu;
   friend class GuiMainWindowLayersMenu;
   friend class GuiMainWindowWindowMenu;
   friend class GuiOpenDataFileDialog;
   friend class GuiToolBar;
};

#endif

