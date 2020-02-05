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
#include <set>
#include <sstream>

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QMessageBox>
#include <QSpinBox>

#include "AreaColorFile.h"
#include "AtlasSpaceFile.h"
#include "BrainSet.h"
#include "BrainModelVolume.h"
#include "BrainModelVolumeProbAtlasToFunctional.h"
#include "BrainModelVolumeSureFitErrorCorrection.h"
#include "BrainModelVolumeSureFitSegmentation.h"
#include "BrainModelVolumeVoxelColoring.h"
#include "DisplaySettingsVolume.h"
#include "FileUtilities.h"
#include "GuiBrainModelOpenGL.h"
#include "GuiFilesModified.h"
#include "GuiMainWindow.h"
#include "GuiMainWindowVolumeActions.h"
#include "GuiVolumeCreateDialog.h"
#include "GuiVolumeHandleFinderDialog.h"
#include "GuiVolumeReconstructionDialog.h"
#include "GuiVolumeSegmentationEditorDialog.h"
#include "GuiVolumeSelectionDialog.h"
#include "GuiVolumeSureFitSegmentationDialog.h"
#include "GuiVolumeTopologyReportDialog.h"
#include "QtTextEditDialog.h"
#include "VolumeFile.h"
#include "WuQDataEntryDialog.h"
#include "global_variables.h"
#include "vtkTransform.h"

/**
 * Constructor.
 */
GuiMainWindowVolumeActions::GuiMainWindowVolumeActions(GuiMainWindow* mainWindowParent)
   : QObject(mainWindowParent)
{
   setObjectName("GuiMainWindowVolumeActions");
   
   createEmptyVolumeAction = new QAction(mainWindowParent);
   createEmptyVolumeAction->setText("Create New Volume...");
   createEmptyVolumeAction->setObjectName("createEmptyVolumeAction");
   QObject::connect(createEmptyVolumeAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotCreateNewEmptyVolume()));

   resizeUnderlayVolumeAction = new QAction(mainWindowParent);
   resizeUnderlayVolumeAction->setText("Resize Underlay Volume...");
   resizeUnderlayVolumeAction->setObjectName("resizeUnderlayVolumeAction");
   QObject::connect(resizeUnderlayVolumeAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotResizeUnderlayVolume()));

   showVoxelExtentAction = new QAction(mainWindowParent);
   showVoxelExtentAction->setText("Show Voxel Dimensional Extent...");
   showVoxelExtentAction->setObjectName("showVoxelExtentAction");
   QObject::connect(showVoxelExtentAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotShowVoxelDimensionExtent()));

   editVolumeAttributesAction = new QAction(mainWindowParent);
   editVolumeAttributesAction->setText("Edit Volume Attributes...");
   editVolumeAttributesAction->setObjectName("editVolumeAttributesAction");
   QObject::connect(editVolumeAttributesAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotEditAttributes()));

   findHandlesAction = new QAction(mainWindowParent);
   findHandlesAction->setText("Find Handles...");
   findHandlesAction->setObjectName("findHandlesAction");
   QObject::connect(findHandlesAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotSegmentationFindHandles()));

   reconstructAction = new QAction(mainWindowParent);
   reconstructAction->setText("Reconstruct Into Surface...");
   reconstructAction->setObjectName("reconstructAction");
   QObject::connect(reconstructAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotSegmentationReconstruct()));

   editSegmentationAction = new QAction(mainWindowParent);
   editSegmentationAction->setText("Edit Voxels...");
   editSegmentationAction->setObjectName("editSegmentationAction");
   QObject::connect(editSegmentationAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotSegmentationEditVoxels()));

   surefitSegmentationAction = new QAction(mainWindowParent);
   surefitSegmentationAction->setText("SureFit Operations (Segmentation)...");
   surefitSegmentationAction->setObjectName("surefitSegmentationAction");
   QObject::connect(surefitSegmentationAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotSureFitSegmentation()));

   padSegmentationAction = new QAction(mainWindowParent);
   padSegmentationAction->setText("Pad Volume...");
   padSegmentationAction->setObjectName("padSegmentationAction");
   QObject::connect(padSegmentationAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotSegmentPad()));

   removeIslandsAction = new QAction(mainWindowParent);
   removeIslandsAction->setText("Remove Islands");
   removeIslandsAction->setObjectName("removeIslandsAction");
   QObject::connect(removeIslandsAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotSegmentRemoveIslands()));

   topologyReportAction = new QAction(mainWindowParent);
   topologyReportAction->setText("Topology Error Report...");
   topologyReportAction->setObjectName("topologyReportAction");
   QObject::connect(topologyReportAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotSegmentationTopologyReport()));

   cerebralHullAction = new QAction(mainWindowParent);
   cerebralHullAction->setText("Generate Cerebral Hull Volume");
   cerebralHullAction->setObjectName("cerebralHullAction");
   QObject::connect(cerebralHullAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotSegmentationCerebralHull()));

   fillCavitiesAction = new QAction(mainWindowParent);
   fillCavitiesAction->setText("Fill Cavities");
   fillCavitiesAction->setObjectName("fillCavitiesAction");
   QObject::connect(fillCavitiesAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotSegmentationFillCavities()));

   applyRotationAction = new QAction(mainWindowParent);
   applyRotationAction->setText("Apply Rotation...");
   applyRotationAction->setObjectName("applyRotationAction");
   QObject::connect(applyRotationAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotTransformApplyRotation()));

   enableRotationAction = new QAction(mainWindowParent);
   enableRotationAction->setText("Enable Rotation");
   enableRotationAction->setObjectName("enableRotationAction");
   enableRotationAction->setCheckable(true);
   QObject::connect(enableRotationAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotTransformEnableRotation()));

   regionOfInterestAction = new QAction(mainWindowParent);
   regionOfInterestAction->setText("Region of Interest Operations...");
   regionOfInterestAction->setObjectName("regionOfInterestAction");
   QObject::connect(regionOfInterestAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotRegionOfInterestDialog()));

   probAtlasToVolumeAction = new QAction(mainWindowParent);
   probAtlasToVolumeAction->setText("Convert Probabilistic Atlas to Functional Volume...");
   probAtlasToVolumeAction->setObjectName("probAtlasToVolumeAction");
   QObject::connect(probAtlasToVolumeAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotProbAtlasConvertToFunctional()));

   anatomyBiasCorrectionAction = new QAction(mainWindowParent);
   anatomyBiasCorrectionAction->setText("Bias Correction...");
   anatomyBiasCorrectionAction->setObjectName("anatomyBiasCorrectionAction");
   QObject::connect(anatomyBiasCorrectionAction, SIGNAL(triggered(bool)),
                    mainWindowParent, SLOT(displayVolumeBiasCorrectionDialog()));
                    
   anatomyThresholdAction = new QAction(mainWindowParent);
   anatomyThresholdAction->setText("Threshold Into Segmentation...");
   anatomyThresholdAction->setObjectName("anatomyThresholdAction");
   QObject::connect(anatomyThresholdAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotAnatomyThreshold()));

   editPaintVolumeAction = new QAction(mainWindowParent);
   editPaintVolumeAction->setText("Edit Voxels...");
   editPaintVolumeAction->setObjectName("editPaintVolumeAction");
   QObject::connect(editPaintVolumeAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotPaintEditVoxels()));

   paintVolumeGenerateColorsAction = new QAction(mainWindowParent);
   paintVolumeGenerateColorsAction->setText("Generate Colors for Paints without Colors");
   paintVolumeGenerateColorsAction->setObjectName("paintVolumeGenerateColorsAction");
   QObject::connect(paintVolumeGenerateColorsAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotGenerateColorsForPaints()));

   mathOperationsVolumeAction = new QAction(mainWindowParent);
   mathOperationsVolumeAction->setText("Mathematical Operations...");
   mathOperationsVolumeAction->setObjectName("mathOperationsVolumeAction");
   QObject::connect(mathOperationsVolumeAction, SIGNAL(triggered(bool)),
                    mainWindowParent, SLOT(displayVolumeMathDialog()));

   trajectoryToolAction = new QAction(mainWindowParent);
   trajectoryToolAction->setText("Electrode trajectory tool...");
   trajectoryToolAction->setObjectName("trajectoryToolAction");
   QObject::connect(trajectoryToolAction, SIGNAL(triggered(bool)),
                    mainWindowParent, SLOT(displayTrajectoryToolDialog()));
}

/**
 * Destructor.
 */
GuiMainWindowVolumeActions::~GuiMainWindowVolumeActions()
{
}

/**
 * called to create a new empty volume.
 */
void 
GuiMainWindowVolumeActions::slotCreateNewEmptyVolume()
{
   GuiVolumeCreateDialog cd(theMainWindow);
   cd.exec();
}
      
/**
 * called to display volume region of interest dialog.
 */
void 
GuiMainWindowVolumeActions::slotRegionOfInterestDialog()
{
   theMainWindow->getVolumeRegionOfInterestDialog(true);
}
      
/**
 * called to crop the underlay volume.
 */
void 
GuiMainWindowVolumeActions::slotResizeUnderlayVolume()
{
   theMainWindow->getVolumeResizingDialog(true);
}

/**
 * Called to view/edit volume attributes.
 */
void
GuiMainWindowVolumeActions::slotEditAttributes()
{
   theMainWindow->getVolumeAttributesDialog(true);
}

/**
 * Called to show voxel dimensional extent
 */
void
GuiMainWindowVolumeActions::slotShowVoxelDimensionExtent()
{
   BrainModelVolume* bmv = theMainWindow->getBrainModelVolume();
   if (bmv != NULL) {
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
      int extent[6];
      VolumeFile* vf = bmv->getMasterVolumeFile();
      if (vf != NULL) {
         float coordExtent[6];
         vf->getNonZeroVoxelExtent(extent, coordExtent);
         QString str = 
            ("Non-zero voxel dimensional extent: \n" 
             "   Parasagittal: (" 
                + QString::number(extent[0])
                + ", " 
                + QString::number(extent[1])
                + ")\n"
             "   Coronal:      (" 
                + QString::number(extent[2])
                + ", " 
                + QString::number(extent[3])
                + ")\n"
             "   Horizontal:   (" 
                + QString::number(extent[4])
                + ", " 
                + QString::number(extent[5]) 
                + ")\n"
               );
      
         str += (
            "\nCoordinate range (centers of voxels):\n"
            "   X: ("
               + QString::number(coordExtent[0], 'f', 2)
               + ", "
               + QString::number(coordExtent[1], 'f', 2)
               + ")\n"
            "   Y: ("
               + QString::number(coordExtent[2], 'f', 2)
               + ", "
               + QString::number(coordExtent[3], 'f', 2)
               + ")\n"
            "   Z: ("
               + QString::number(coordExtent[4], 'f', 2)
               + ", "
               + QString::number(coordExtent[5], 'f', 2)
               + ")\n");
         QMessageBox* mb = new QMessageBox("Voxel Dimensional Extent",
                                        str,
                                        QMessageBox::NoIcon,
                                        QMessageBox::Ok,
                                        Qt::NoButton,
                                        Qt::NoButton,
                                        theMainWindow,
                                        Qt::MSWindowsFixedSizeDialogHint);
         mb->setModal(false);
         mb->show();
         mb->setAttribute(Qt::WA_DeleteOnClose);
         QApplication::restoreOverrideCursor();
      }
   }
}

/**
 * Called to display apply rotation dialog. 
 */
void
GuiMainWindowVolumeActions::slotTransformEnableRotation()
{
   //
   // Toggle 
   //
   //enableRotationAction->setChecked(!enableRotationAction->isChecked());

   //
   // If enabling rotation and volume in main window, change mouse mode to VIEW
   //
   if (enableRotationAction->isChecked()) {
      if (theMainWindow->getBrainModelOpenGL()->getDisplayedBrainModelVolume() != NULL) {
         theMainWindow->getBrainModelOpenGL()->setMouseMode(GuiBrainModelOpenGL::MOUSE_MODE_VIEW);
      }
   }
   
   //
   // Update main window status bar
   //
   theMainWindow->updateStatusBarLabel();
}

/**
 * Called to display apply rotation dialog. 
 */
void
GuiMainWindowVolumeActions::slotTransformApplyRotation()
{
   BrainModelVolume* bmv = theMainWindow->getBrainModelVolume();
   if (bmv != NULL) {
      
      //
      // Dialog for choosing volume to which apply rotation is applied
      //
      WuQDataEntryDialog ded(theMainWindow);
      ded.setWindowTitle("Apply Rotation");
      
      //
      // Primary overlay
      //
      QCheckBox* primaryOverlayCheckBox = NULL;
      VolumeFile* primaryOverlay = bmv->getOverlayPrimaryVolumeFile();
      if (primaryOverlay != NULL) {
         QString name("Primary Overlay: ");
         name.append(FileUtilities::basename(primaryOverlay->getFileName()));
         primaryOverlayCheckBox = ded.addCheckBox(name, true);
      }
      
      //
      // Secondary overlay
      //
      QCheckBox* secondaryOverlayCheckBox = NULL;
      VolumeFile* secondaryOverlay = bmv->getOverlaySecondaryVolumeFile();
      if (secondaryOverlay != NULL) {
         QString name("Secondary Overlay: ");
         name.append(FileUtilities::basename(secondaryOverlay->getFileName()));
         secondaryOverlayCheckBox = ded.addCheckBox(name, true);
      }
      
      //
      // Underlay
      //
      QCheckBox* underlayCheckBox = NULL;
      VolumeFile* underlay = bmv->getUnderlayVolumeFile();
      if (underlay != NULL) {
         QString name("Underlay: ");
         name.append(FileUtilities::basename(underlay->getFileName()));
         underlayCheckBox = ded.addCheckBox(name, true);
      }
      
      //
      // Did user press OK ?
      //
      if (ded.exec() == QDialog::Accepted) {
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   
         vtkTransform* transform = vtkTransform::New();
         switch (bmv->getSelectedAxis(0)) {
            case VolumeFile::VOLUME_AXIS_X:
               transform->RotateX(-bmv->getDisplayRotation(0));
               break;
            case VolumeFile::VOLUME_AXIS_Y:
               transform->RotateY(bmv->getDisplayRotation(0));
               break;
            case VolumeFile::VOLUME_AXIS_Z:
               transform->RotateZ(-bmv->getDisplayRotation(0));
               break;
            case VolumeFile::VOLUME_AXIS_ALL:
            case VolumeFile::VOLUME_AXIS_OBLIQUE:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
            case VolumeFile::VOLUME_AXIS_UNKNOWN:
               break;
         }
         //
         // Apply to primary overlay volume
         //
         if (primaryOverlayCheckBox != NULL) {
            if (primaryOverlayCheckBox->isChecked()) {
               primaryOverlay->applyTransformationMatrix(transform);
            }
         }
         
         //
         // Apply to secondary overlay volume
         //
         if (secondaryOverlayCheckBox != NULL) {
            if (secondaryOverlayCheckBox->isChecked()) {
               secondaryOverlay->applyTransformationMatrix(transform);
            }
         }
         
         //
         // Apply to underlay volume
         //
         if (underlayCheckBox != NULL) {
            if (underlayCheckBox->isChecked()) {
               underlay->applyTransformationMatrix(transform);
            }
         }
         
         transform->Delete();
         
         //
         // Reset the transformations
         //
         bmv->resetViewingTransform(0);
         GuiFilesModified fm;
         fm.setVolumeModified();
         theMainWindow->fileModificationUpdate(fm);
         GuiBrainModelOpenGL::updateAllGL();
         
         QApplication::restoreOverrideCursor();
      }
   }
}

/**
 * Called when anatomy threshold menu item selected.
 */
void 
GuiMainWindowVolumeActions::slotAnatomyThreshold()
{
   theMainWindow->getVolumeThresholdSegmentationDialog(true);
}

/**
 * called to edit paint volume voxels.
 */
void 
GuiMainWindowVolumeActions::slotPaintEditVoxels()
{
   theMainWindow->getVolumePaintEditorDialog(true);
}

/**
 * slot for generating colors for non-matching paint names.
 */
void 
GuiMainWindowVolumeActions::slotGenerateColorsForPaints()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   
   std::set<QString> uniquePaintNames;
   for (int i = 0; i < theMainWindow->getBrainSet()->getNumberOfVolumePaintFiles(); i++) {
      VolumeFile* vf = theMainWindow->getBrainSet()->getVolumePaintFile(i);
      vf->createRegionNamesForVoxelsThatDoNotIndexIntoRegionNames();
      const int num = vf->getNumberOfRegionNames();
      for (int j = 0; j < num; j++) {
         uniquePaintNames.insert(vf->getRegionNameFromIndex(j));
      }
      vf->setVoxelColoringInvalid();
   }
   
   std::vector<QString> paintNames(uniquePaintNames.begin(), uniquePaintNames.end());
   ColorFile* cf = theMainWindow->getBrainSet()->getAreaColorFile();
   cf->generateColorsForNamesWithoutColors(paintNames, true);
   GuiBrainModelOpenGL::updateAllGL(NULL);
   GuiFilesModified fm;
   fm.setAreaColorModified();
   theMainWindow->fileModificationUpdate(fm);
   
   QApplication::restoreOverrideCursor();
}
      
/**
 * called to convert prob atlas volume to a functional volume.
 */
void 
GuiMainWindowVolumeActions::slotProbAtlasConvertToFunctional()
{
   GuiVolumeSelectionDialog vsd(theMainWindow,
                                false,
                                true,
                                false,
                                false,
                                false,
                                false,
                                false,
                                GuiVolumeSelectionControl::LABEL_MODE_FILE_LABEL_AND_NAME,
                                "selectFuncForProbAtlasDialog",
                                true,
                                true,
                                true);
   if (vsd.exec() == QDialog::Accepted) {
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
      BrainModelVolumeProbAtlasToFunctional bmvpatf(theMainWindow->getBrainSet(),
                                                    vsd.getSelectedVolumeFile(),
                                                    vsd.getSelectedVolumeFileName(),
                                                    vsd.getSelectedVolumeDescriptiveLabel());
      try {
         bmvpatf.execute();
      }
      catch (BrainModelAlgorithmException& e) {
         QApplication::restoreOverrideCursor();
         QMessageBox::critical(theMainWindow, "ERROR", e.whatQString());
         return;
      }
      GuiFilesModified fm;
      fm.setVolumeModified();
      theMainWindow->fileModificationUpdate(fm);
      GuiBrainModelOpenGL::updateAllGL();
      QApplication::beep();
      QApplication::restoreOverrideCursor();
   }
}

/**
 * Called to remove islands from segmentation volume.
 */
void 
GuiMainWindowVolumeActions::slotSegmentRemoveIslands()
{
   BrainModelVolume* bmv = theMainWindow->getBrainSet()->getBrainModelVolume(-1);
   if (bmv != NULL) {
      VolumeFile* vf = bmv->getSelectedVolumeSegmentationFile();
      
      if (vf != NULL) {
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         vf->removeIslandsFromSegmentation();
         GuiFilesModified fm;
         fm.setVolumeModified();
         theMainWindow->fileModificationUpdate(fm);
         GuiBrainModelOpenGL::updateAllGL();
         QApplication::restoreOverrideCursor();
      }
   }
}      

/**
 * Called to generate the cerebral hull volume from the segmentation volume.
 */
void 
GuiMainWindowVolumeActions::slotSegmentationCerebralHull()
{
   BrainModelVolume* bmv = theMainWindow->getBrainSet()->getBrainModelVolume(-1);
   if (bmv != NULL) {
      VolumeFile* vf = bmv->getSelectedVolumeSegmentationFile();
      
      if (vf != NULL) {
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         
         //
         // Expand around edges with empty slices
         //
         VolumeFile segmentVolumeExpanded(*vf);
         int expDim[3];
         segmentVolumeExpanded.getDimensions(expDim);
         const int expSlices = 7;
         const int resizeCrop[6] = { 
            -expSlices, expDim[0] + expSlices,
            -expSlices, expDim[1] + expSlices,
            -expSlices, expDim[2] + expSlices
         };
         segmentVolumeExpanded.resize(resizeCrop);

         try {
            theMainWindow->getBrainSet()->generateCerebralHullVtkFile(&segmentVolumeExpanded, true);
         }
         catch (BrainModelAlgorithmException& e) {
            QApplication::restoreOverrideCursor();
            QMessageBox::critical(theMainWindow, "ERROR", e.whatQString());
            return;
         }
         DisplaySettingsVolume* dsv = theMainWindow->getBrainSet()->getDisplaySettingsVolume();
         dsv->setSelectedSegmentationVolume(theMainWindow->getBrainSet()->getNumberOfVolumeSegmentationFiles() - 1);
         GuiFilesModified fm;
         fm.setVolumeModified();
         theMainWindow->fileModificationUpdate(fm);
         GuiBrainModelOpenGL::updateAllGL();
         QApplication::restoreOverrideCursor();
      }
   }
}

/**
 * Called to display topology information.
 */
void 
GuiMainWindowVolumeActions::slotSegmentationTopologyReport()
{
   static GuiVolumeTopologyReportDialog* trd = NULL;
   if (trd == NULL) {
      trd = new GuiVolumeTopologyReportDialog(theMainWindow);
   }
   trd->show();
   trd->activateWindow();
}

/**
 * Called to fill cavities.
 */
void 
GuiMainWindowVolumeActions::slotSegmentationFillCavities()
{
   BrainModelVolume* bmv = theMainWindow->getBrainSet()->getBrainModelVolume(-1);
   if (bmv != NULL) {
      VolumeFile* vf = bmv->getSelectedVolumeSegmentationFile();
      
      if (vf != NULL) {
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         vf->fillSegmentationCavities();
         GuiFilesModified fm;
         fm.setVolumeModified();
         theMainWindow->fileModificationUpdate(fm);
         GuiBrainModelOpenGL::updateAllGL();
         QApplication::restoreOverrideCursor();
      }
   }
}
      
/**
 * Called to pad the segmentation volume.
 */
void 
GuiMainWindowVolumeActions::slotSegmentPad()
{
   BrainModelVolume* bmv = theMainWindow->getBrainSet()->getBrainModelVolume(-1);
   if (bmv != NULL) {
      VolumeFile* vf = bmv->getSelectedVolumeSegmentationFile();
      
      if (vf != NULL) {
         WuQDataEntryDialog ded(theMainWindow);
         ded.setWindowTitle("Padding Options");
         QSpinBox* xLeftSpinBox = ded.addSpinBox("X Left", 0, 10000, 1);
         QSpinBox* xRightSpinBox = ded.addSpinBox("X Right", 0, 10000, 1);
         QSpinBox* yPosteriorSpinBox = ded.addSpinBox("Y Posterior", 0, 10000, 1);
         QSpinBox* yAnteriorSpinBox = ded.addSpinBox("Y Anterior", 0, 10000, 1);
         QSpinBox* zInferiorSpinBox = ded.addSpinBox("Z Inferior", 0, 10000, 1);
         QSpinBox* zSuperiorSpinBox = ded.addSpinBox("Z Superior", 0, 10000, 1);
         QCheckBox* erodePaddingCheckBox = ded.addCheckBox("Erode Padding", true);
         if (ded.exec() == QDialog::Accepted) {
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            const int padding[6] = {
               xLeftSpinBox->value(),
               xRightSpinBox->value(),
               yPosteriorSpinBox->value(),
               yAnteriorSpinBox->value(),
               zInferiorSpinBox->value(),
               zSuperiorSpinBox->value()
            };
            
            vf->padSegmentation(padding, erodePaddingCheckBox->isChecked());
            GuiFilesModified fm;
            fm.setVolumeModified();
            theMainWindow->fileModificationUpdate(fm);
            GuiBrainModelOpenGL::updateAllGL();
            QApplication::restoreOverrideCursor();
         }
      }
   }
}

/**
 * Segment the selected anatomical volume.
 */
void
GuiMainWindowVolumeActions::slotSureFitSegmentation()
{
   GuiVolumeSureFitSegmentationDialog* sd = new GuiVolumeSureFitSegmentationDialog(theMainWindow);
   sd->show();
   sd->activateWindow();
}

/**
 * Called to display the segmentation volume editor
 */
void
GuiMainWindowVolumeActions::slotSegmentationEditVoxels()
{
   theMainWindow->getVolumeSegmentationEditorDialog(true);
}

/**
 * Called to find handles in segmentation volume.
 */
void
GuiMainWindowVolumeActions::slotSegmentationFindHandles()
{
   static GuiVolumeHandleFinderDialog* hdf = NULL;
   if (hdf == NULL) {
      hdf = new GuiVolumeHandleFinderDialog(theMainWindow);
   }
   hdf->show();
   hdf->activateWindow();
}

/**
 * Called to reconstruct a segmentation volume into a surface..
 */
void
GuiMainWindowVolumeActions::slotSegmentationReconstruct()
{
   BrainModelVolume* bmv = theMainWindow->getBrainSet()->getBrainModelVolume(-1);
   if (bmv != NULL) {
      VolumeFile* vf = bmv->getSelectedVolumeSegmentationFile();
      
      if (vf != NULL) {
         GuiVolumeReconstructionDialog vrd(theMainWindow, vf, true);
         vrd.exec();
      }
   }
}

/**
 * update the actions (typically called when menu is about to show)
 */
void 
GuiMainWindowVolumeActions::updateActions()
{
   bool haveUnderlayVolume = false;
   BrainModelVolume* bmv = theMainWindow->getBrainModelVolume();
   if (bmv != NULL) {
      haveUnderlayVolume = (bmv->getUnderlayVolumeFile() != NULL);
   }
   
   resizeUnderlayVolumeAction->setEnabled(haveUnderlayVolume);

   anatomyThresholdAction->setEnabled(theMainWindow->getBrainSet()->getNumberOfVolumeAnatomyFiles() > 0);
   
   applyRotationAction->setEnabled(enableRotationAction->isChecked());

   editPaintVolumeAction->setEnabled(theMainWindow->getBrainSet()->getNumberOfVolumePaintFiles() > 0);

   probAtlasToVolumeAction->setEnabled(theMainWindow->getBrainSet()->getNumberOfVolumeProbAtlasFiles() > 0);

}

