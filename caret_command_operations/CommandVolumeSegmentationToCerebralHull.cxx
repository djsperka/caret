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
#include "CommandVolumeSegmentationToCerebralHull.h"
#include "FileFilters.h"
#include "ProgramParameters.h"
#include "ScriptBuilderParameters.h"
#include "VolumeFile.h"
#include "vtkPolyData.h"
#include "vtkPolyDataWriter.h"

/**
 * constructor.
 */
CommandVolumeSegmentationToCerebralHull::CommandVolumeSegmentationToCerebralHull()
   : CommandBase("-volume-segmentation-to-cerebral-hull",
                 "VOLUME SEGMENTATION TO CEREBRAL HULL VOLUME AND SURFACE")
{
}

/**
 * destructor.
 */
CommandVolumeSegmentationToCerebralHull::~CommandVolumeSegmentationToCerebralHull()
{
}

/**
 * get the script builder parameters.
 */
void 
CommandVolumeSegmentationToCerebralHull::getScriptBuilderParameters(ScriptBuilderParameters& paramsOut) const
{
   paramsOut.clear();
   paramsOut.addFile("Input Segmentation Volume File Name", FileFilters::getVolumeSegmentationFileFilter());
   paramsOut.addFile("Output Cerebral Hull Volume File Name", FileFilters::getVolumeGenericFileFilter());
   paramsOut.addFile("Output Cerebral Hull VTK Surface File Name", FileFilters::getVtkSurfaceFileFilter());
}

/**
 * get full help information.
 */
QString 
CommandVolumeSegmentationToCerebralHull::getHelpInformation() const
{
   QString helpInfo =
      (indent3 + getShortDescription() + "\n"
       + indent6 + parameters->getProgramNameWithoutPath() + " " + getOperationSwitch() + "  \n"
       + indent9 + "<input-segmentation-volume-file-name>\n"
       + indent9 + "<output-cerebral-hull-volume-file-name>\n"
       + indent9 + "<output-cerebral-hull-vtk-surface-file-name>\n"
       + indent9 + "\n"
       + indent9 + "Generate cerebral hull volume and VTK surface from a.\n"
       + indent9 + "segmentation volume file.\n"
       + indent9 + "\n");
      
   return helpInfo;
}

/**
 * execute the command.
 */
void 
CommandVolumeSegmentationToCerebralHull::executeCommand() throw (BrainModelAlgorithmException,
                                     CommandException,
                                     FileException,
                                     ProgramParametersException,
                                     StatisticException)
{
   //
   // Get parameters
   //
   const QString segmentationVolumeFileName =
      parameters->getNextParameterAsString("Input Segmentation Volume File Name");
   const QString hullVolumeFileName =
      parameters->getNextParameterAsString("Output Cerebral Hull Volume File Name");
   const QString vtkSurfaceFileName =
      parameters->getNextParameterAsString("Output Cerebral Hull VTK Surface File Name");
      
   //
   // Read segemtation
   //
   VolumeFile segmentationVolume;
   segmentationVolume.readFile(segmentationVolumeFileName);
   
   //
   // Expand around edges with empty slices
   //
   int expDim[3];
   segmentationVolume.getDimensions(expDim);
   const int expSlices = 7;
   const int resizeCrop[6] = { 
      -expSlices, expDim[0] + expSlices,
      -expSlices, expDim[1] + expSlices,
      -expSlices, expDim[2] + expSlices
   };
   segmentationVolume.resize(resizeCrop);

   //
   // Generate hull volume and surface
   //
   BrainSet bs;
   VolumeFile* hullVolume = NULL;
   vtkPolyData* hullSurface = NULL;
   bs.generateCerebralHullVtkFile(&segmentationVolume, 
                                  hullVolume,
                                  hullSurface);

   //
   // Write the hull volume
   //
   hullVolume->writeFile(hullVolumeFileName);
   
   //
   // Write the hull surface
   //
   vtkPolyDataWriter* writer = vtkPolyDataWriter::New();
#ifdef HAVE_VTK6
   writer->SetInputData(hullSurface);
#else
   writer->SetInput(hullSurface);
#endif
   writer->SetHeader("Written by Caret");
   writer->SetFileName((char*)vtkSurfaceFileName.toAscii().constData());
   writer->Write();
   writer->Delete();
   
   //
   // free memory
   //
   hullSurface->Delete();
   delete hullVolume;
}

      

