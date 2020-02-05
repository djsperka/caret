
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

#include "GuiFilesModified.h"

/**
 * Constructor.
 */
GuiFilesModified::GuiFilesModified()
{
   inhibitDefaultSurfaceScaling = false;
   setStatusForAll(false);
}

/**
 * Destructor.
 */
GuiFilesModified::~GuiFilesModified()
{
}

/**
 * get all files were modified.
 */
bool 
GuiFilesModified::getAllFilesModified() const
{
   const bool allModified = 
      areaColor &&
      arealEstimation &&
      border &&
      borderColor &&
      cell &&
      cellColor &&
      cellProjection &&
      cocomac &&
      contour &&
      contourCell &&
      contourCellColor &&
      coordinate &&
      cut &&
      deformationField &&
      deformationMap &&
      foci &&
      fociColor &&
      fociProjection &&
      fociSearch &&
      geodesic &&
      images &&
      latLon &&
      metric &&
      paint &&
      palette &&
      parameter &&
      probabilisticAtlas &&
      rgbPaint &&
      scene &&
      section &&
      studyCollection &&
      studyMetaData &&
      surfaceShape &&
      vector &&
      topography &&
      topology &&
	  trajectory &&
      transformationMatrix &&
      transformationData &&
      vocabulary &&
      volume &&
      vtkModel &&
      wustlRegion;
      
   return allModified;
}

/**
 * set the status for all files.
 */
void 
GuiFilesModified::setStatusForAll(const bool status)
{
   areaColor = status;
   arealEstimation = status;
   border = status;
   borderColor = status;
   cell = status;
   cellColor = status;
   cellProjection = status;
   cocomac = status;
   contour = status;
   contourCell = status;
   contourCellColor = status;
   coordinate = status;
   cut = status;
   deformationField = status;
   deformationMap = status;
   foci = status;
   fociColor = status;
   fociProjection = status;
   fociSearch = status;
   geodesic = status;
   images = status;
   latLon = status;
   metric = status;
   paint = status;
   palette = status;
   parameter = status;
   probabilisticAtlas = status;
   rgbPaint = status;
   scene = status;
   section = status;
   studyCollection = studyCollection;
   studyMetaData = studyMetaData;
   surfaceShape = status;
   vector = status;
   topography = status;
   topology = status;
   trajectory = status;
   transformationMatrix = status;
   transformationData = status;
   vocabulary = status;
   volume = status;
   vtkModel = status;
   wustlRegion = status;
}
      
