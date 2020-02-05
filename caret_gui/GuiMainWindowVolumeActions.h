
#ifndef __GUI_MAIN_WINDOW_VOLUME_ACTIONS_H__
#define __GUI_MAIN_WINDOW_VOLUME_ACTIONS_H__

#include <QAction>
#include <QObject>

class GuiMainWindow;

/// actions for volume menu
class GuiMainWindowVolumeActions : public QObject {
   Q_OBJECT
   
   public:
      // Constructor
      GuiMainWindowVolumeActions(GuiMainWindow* mainWindowParent);
      
      // Destructor
      ~GuiMainWindowVolumeActions();
      
      /// create empty volume action
      QAction* getCreateEmptyVolumeAction() { return createEmptyVolumeAction; }
       
      /// resize underlay volume action
      QAction* getResizeUnderlayVolumeAction() { return resizeUnderlayVolumeAction; }
      
      /// show voxel extent action
      QAction* getShowVoxelExtentAction() { return showVoxelExtentAction; }
      
      /// edit volume attributes action
      QAction* getEditVolumeAttributesAction() { return editVolumeAttributesAction; }
      
      /// find handles action
      QAction* getFindHandlesAction() { return findHandlesAction; }
      
      /// reconstruct action
      QAction* getReconstructAction() { return reconstructAction; }
      
      /// edit segmentation action
      QAction* getEditSegmentationAction() { return editSegmentationAction; }
      
      /// surefit segmentation action
      QAction* getSurefitSegmentationAction() { return surefitSegmentationAction; }
      
      /// pad segmentation action
      QAction* getPadSegmentationAction() { return padSegmentationAction; }
      
      /// remove islands action
      QAction* getRemoveIslandsAction() { return removeIslandsAction; }
      
       /// topology report action
      QAction* getTopologyReportAction() { return topologyReportAction; }
      
      /// cerebral hull action
      QAction* getCerebralHullAction() { return cerebralHullAction; }
      
      /// fill cavities action
      QAction* getFillCavitiesAction() { return fillCavitiesAction; }
      
      /// apply rotation action
      QAction* getApplyRotationAction() { return applyRotationAction; }
      
      /// enable rotation action
      QAction* getEnableRotationAction() { return enableRotationAction; }
      
      /// region of interest action
      QAction* getRegionOfInterestAction() { return regionOfInterestAction; }
      
      /// prob atlas to volume action
      QAction* getProbAtlasToVolumeAction() { return probAtlasToVolumeAction; }
      
      /// anatomy threshold action
      QAction* getAnatomyThresholdAction() { return anatomyThresholdAction; }
      
      /// anatomy bias correction
      QAction* getAnatomyBiasCorrectionAction() { return anatomyBiasCorrectionAction; }
      
      /// edit paint volume action
      QAction* getEditPaintVolumeAction() { return editPaintVolumeAction; }
      
      /// generate colors for paint action
      QAction* getPaintVolumeGenerateColorsAction() { return paintVolumeGenerateColorsAction; }
      
      /// create math operations action
      QAction* getMathOperationsVolumeAction() { return mathOperationsVolumeAction; }

      /// generate electrode traj tool
      QAction* getTrajectoryToolAction() { return trajectoryToolAction; }

      /// is transform rotation checked
      bool getTranformRotationChecked() const { return enableRotationAction->isChecked(); }
      
   public slots:
      /// update the actions (typically called when menu is about to show)
      void updateActions();

      // called to create a new empty volume
      void slotCreateNewEmptyVolume();
      
      // called to resize the underlay volume
      void slotResizeUnderlayVolume();
      
      // called when show voxel dimension extent selected
      void slotShowVoxelDimensionExtent();
      
      // called to edit volume attributes
      void slotEditAttributes();
      
      // called to find handles in a segmentation volume
      void slotSegmentationFindHandles();
      
      // called to reconstruct a segmentation volume into a surface
      void slotSegmentationReconstruct();
      
      // called to edit a segmentation volume voxels
      void slotSegmentationEditVoxels();
      
      // Segment the selected anatomical volume.
      void slotSureFitSegmentation();
      
      // Called to pad the segmentation volume
      void slotSegmentPad();
      
      // Called to remove islands from segmentation volume
      void slotSegmentRemoveIslands();
      
      // Called to display topology information
      void slotSegmentationTopologyReport();

      // Called to generate the cerebral hull volume from the segmentation volume
      void slotSegmentationCerebralHull();
      
      // Called to fill cavities
      void slotSegmentationFillCavities();
      
      // Called to display apply rotation dialog. 
      void slotTransformApplyRotation();

      // Called to display enable rotation dialog. 
      void slotTransformEnableRotation();

      // called to display volume region of interest dialog
      void slotRegionOfInterestDialog();
      
      // called to convert prob atlas volume to a functional volume
      void slotProbAtlasConvertToFunctional();
      
      // Called when anatomy threshold menu item selected
      void slotAnatomyThreshold();
      
      // called to edit paint volume voxels
      void slotPaintEditVoxels();
      
      // called to generate colors for paint names
      void slotGenerateColorsForPaints();
      
   protected:
      /// create math operations action
      QAction* mathOperationsVolumeAction;
      
      /// trajectory tool action
      QAction* trajectoryToolAction;

      /// create empty volume action
      QAction* createEmptyVolumeAction;
      
      /// resize underlay volume action
      QAction* resizeUnderlayVolumeAction;
      
      /// show voxel extent action
      QAction* showVoxelExtentAction;
      
      /// edit volume attributes action
      QAction* editVolumeAttributesAction;
      
      /// find handles action
      QAction* findHandlesAction;
      
      /// reconstruct action
      QAction* reconstructAction;
      
      /// edit segmentation action
      QAction* editSegmentationAction;
      
      /// surefit segmentation action
      QAction* surefitSegmentationAction;
      
      /// pad segmentation action
      QAction* padSegmentationAction;
      
      /// remove islands action
      QAction* removeIslandsAction;
      
      /// topology report action
      QAction* topologyReportAction;
      
      /// cerebral hull action
      QAction* cerebralHullAction;
      
      /// fill cavities action
      QAction* fillCavitiesAction;
      
      /// apply rotation action
      QAction* applyRotationAction;
      
      /// enable rotation action
      QAction* enableRotationAction;
      
      /// region of interest action
      QAction* regionOfInterestAction;
      
      /// prob atlas to volume action
      QAction* probAtlasToVolumeAction;
      
      /// anatomy threshold action
      QAction* anatomyThresholdAction;
      
      /// anatomy bias correction action
      QAction* anatomyBiasCorrectionAction;
      
      /// edit paint volume action
      QAction* editPaintVolumeAction;
      
      /// generaet colors for paint volume actiono
      QAction* paintVolumeGenerateColorsAction;
      
};

#endif // __GUI_MAIN_WINDOW_VOLUME_ACTIONS_H__

