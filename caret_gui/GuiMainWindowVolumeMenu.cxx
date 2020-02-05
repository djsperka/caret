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

#include "BrainSet.h"
#include "GuiMainWindow.h"
#include "GuiMainWindowVolumeActions.h"
#include "GuiMainWindowVolumeMenu.h"
#include "global_variables.h"

/**
 * Constructor.
 */
GuiMainWindowVolumeMenu::GuiMainWindowVolumeMenu(GuiMainWindow* parent) :
   QMenu("Volume", parent)
{
   setObjectName("GuiMainWindowVolumeMenu");

   volumeActions = parent->getVolumeActions();
   
   createAnatomySubMenu();
   
   addAction(volumeActions->getCreateEmptyVolumeAction());
              
   addAction(volumeActions->getEditVolumeAttributesAction());
   
   addAction(volumeActions->getMathOperationsVolumeAction());
              
   //createFunctionalSubMenu();
   
   createPaintSubMenu();
   
   createProbAtlasSubMenu();
   
   addAction(volumeActions->getRegionOfInterestAction());
              
   addAction(volumeActions->getResizeUnderlayVolumeAction());
              
   createSegmentationSubMenu();
   
   addAction(volumeActions->getShowVoxelExtentAction());
                               
   addAction(volumeActions->getSurefitSegmentationAction());

   //addAction(volumeActions->getSurefitMultiHemSegmentationAction());                                   

   createTransformSubMenu();
   
   addAction(volumeActions->getTrajectoryToolAction());

   QObject::connect(this, SIGNAL(aboutToShow()),
                    volumeActions, SLOT(updateActions()));
}

/**
 * Destructor.
 */
GuiMainWindowVolumeMenu::~GuiMainWindowVolumeMenu()
{
}

/**
 * Create the the functional volume sub menu
 */
void
GuiMainWindowVolumeMenu::createFunctionalSubMenu()
{
   functionalSubMenu = addMenu("Functional");
}

/**
 * Create the the tranform volume sub menu
 */
void
GuiMainWindowVolumeMenu::createTransformSubMenu()
{
   transformSubMenu = addMenu("Transform");
   
   transformSubMenu->addAction(volumeActions->getApplyRotationAction());
   transformSubMenu->addAction(volumeActions->getEnableRotationAction());
}

/**
 * Create the the anatomy volume sub menu
 */
void
GuiMainWindowVolumeMenu::createAnatomySubMenu()
{
   anatomySubMenu = addMenu("Anatomy");
   
   anatomySubMenu->addAction(volumeActions->getAnatomyBiasCorrectionAction());
   
   anatomySubMenu->addAction(volumeActions->getAnatomyThresholdAction());
}

/**
 * Create the the paint volume sub menu
 */
void
GuiMainWindowVolumeMenu::createPaintSubMenu()
{
   paintSubMenu = addMenu("Paint");
   
   paintSubMenu->addAction(volumeActions->getEditPaintVolumeAction());
   paintSubMenu->addAction(volumeActions->getPaintVolumeGenerateColorsAction());
}

/**
 * Create the the prob atlas volume sub menu
 */
void
GuiMainWindowVolumeMenu::createProbAtlasSubMenu()
{
   probAtlasSubMenu = addMenu("Probabilistic Atlas");
   
   probAtlasSubMenu->addAction(volumeActions->getProbAtlasToVolumeAction());
}

/**
 * Create the the segmentation volume sub menu
 */
void
GuiMainWindowVolumeMenu::createSegmentationSubMenu()
{
   segmentationSubMenu = addMenu("Segmentation");
   
   //segmentationSubMenu->addAction(volumeActions->getSegmentCorrectTopologyAction());
   
   segmentationSubMenu->addAction(volumeActions->getEditSegmentationAction());
                                   
   segmentationSubMenu->addAction(volumeActions->getFindHandlesAction()); 
   
   segmentationSubMenu->addAction(volumeActions->getFillCavitiesAction()); 
   
   segmentationSubMenu->addAction(volumeActions->getCerebralHullAction()); 
   
   segmentationSubMenu->addAction(volumeActions->getPadSegmentationAction());
                                   
   segmentationSubMenu->addAction(volumeActions->getReconstructAction()); 
   
   segmentationSubMenu->addAction(volumeActions->getRemoveIslandsAction());
   
   segmentationSubMenu->addAction(volumeActions->getTopologyReportAction());
}

