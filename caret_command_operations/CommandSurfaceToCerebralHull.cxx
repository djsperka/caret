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

#include "BrainModelSurface.h"
#include "BrainModelSurfaceToVolumeSegmentationConverter.h"
#include "BrainSet.h"
#include "CommandSurfaceToCerebralHull.h"
#include "FileFilters.h"
#include "ProgramParameters.h"
#include "ScriptBuilderParameters.h"
#include "vtkPolyDataWriter.h"

/**
 * constructor.
 */
CommandSurfaceToCerebralHull::CommandSurfaceToCerebralHull()
   : CommandBase("-surface-to-cerebral-hull",
                 "SURFACE TO CEREBRAL HULL")
{
}

/**
 * destructor.
 */
CommandSurfaceToCerebralHull::~CommandSurfaceToCerebralHull()
{
}

/**
 * get the script builder parameters.
 */
void 
CommandSurfaceToCerebralHull::getScriptBuilderParameters(ScriptBuilderParameters& paramsOut) const
{
   paramsOut.clear();
   paramsOut.addFile("Fiducial Coordinate File Name", 
                     FileFilters::getCoordinateFiducialFileFilter());
   paramsOut.addFile("Closed Topology File Name", 
                     FileFilters::getTopologyClosedFileFilter());
   paramsOut.addFile("Input Volume File Name", 
                     FileFilters::getVolumeGenericFileFilter());
   paramsOut.addFile("Output Segmentation Volume File Name", 
                     FileFilters::getVolumeSegmentationFileFilter());
   paramsOut.addFile("Output Hull Volume File Name", 
                     FileFilters::getVolumeGenericFileFilter());
   paramsOut.addFile("Output Hull Surface VTK File Name", 
                     FileFilters::getVtkSurfaceFileFilter());
}

/**
 * get full help information.
 */
QString 
CommandSurfaceToCerebralHull::getHelpInformation() const
{
   QString helpInfo =
      (indent3 + getShortDescription() + "\n"
       + indent6 + parameters->getProgramNameWithoutPath() + " " + getOperationSwitch() + "  \n"
       + indent9 + "<fiducial-coordinate-file-name>\n"
       + indent9 + "<closed-topology-file-name>\n"
       + indent9 + "<input-volume-file-name>\n"
       + indent9 + "<output-segmentation-volume-file-name>\n"
       + indent9 + "<output-hull-volume-file-name>\n"
       + indent9 + "<output-cerebral-hull-surface-vtk-file-name>\n"
       + indent9 + "\n"
       + indent9 + "Generate a cerebral hull surface which is typically used\n"
       + indent9 + "for generating sulcal depth.\n"
       + indent9 + "\n"
       + indent9 + "Note: The \"input-volume-file-name\" must exist and be in\n"
       + indent9 + "the same stereotaxic space as the surface.  The input\n"
       + indent9 + "volume is only used to get the stereotaxic space information \n"
       + indent9 + "and its data contents are ignored.  If needed a volume can\n"
       + indent9 + "be created using this program with the command \n"
       + indent9 + "\"-volume-create\".\n"
       + indent9 + "\n"
       + indent9 + "\n"
       + indent9 + "\n"
       + indent9 + "\n"
       + indent9 + "\n");
      
   return helpInfo;
}

/**
 * execute the command.
 */
void 
CommandSurfaceToCerebralHull::executeCommand() throw (BrainModelAlgorithmException,
                                     CommandException,
                                     FileException,
                                     ProgramParametersException,
                                     StatisticException)
{
   //
   // Get required parameters
   //
   const QString fiducialCoordinateFileName =
      parameters->getNextParameterAsString("Fiducial Coordinate File Name");
   const QString closedTopologyFileName =
      parameters->getNextParameterAsString("Closed Topology File Name");
   const QString inputVolumeFileName =
      parameters->getNextParameterAsString("Input Volume File Name");
   QString outputSegmentationVolumeFileName, outputSegmentationVolumeFileLabel;
   parameters->getNextParameterAsVolumeFileNameAndLabel("Output Segmentation Volume File Name and Label",
                                                        outputSegmentationVolumeFileName, 
                                                        outputSegmentationVolumeFileLabel);
   QString outputHullVolumeFileName, outputHullVolumeFileLabel;
   parameters->getNextParameterAsVolumeFileNameAndLabel("Output Hull Volume File Name and Label",
                                                        outputHullVolumeFileName, 
                                                        outputHullVolumeFileLabel);
   const QString outputCerebralHullVtkFileName =
      parameters->getNextParameterAsString("Output Cerebral Hull VTK File Name");
   checkForExcessiveParameters();
   
   //
   // Create a brain set
   //
   BrainSet brainSet(closedTopologyFileName,
                     fiducialCoordinateFileName,
                     "",
                     true);
   BrainModelSurface* bms = brainSet.getBrainModelSurface(0);
   if (bms == NULL) {
      throw CommandException("Unable to find surface.");
   }
   const TopologyFile* tf = bms->getTopologyFile();
   if (tf == NULL) {
      throw CommandException("Unable to find topology.");
   }
   const int numNodes = bms->getNumberOfNodes();
   if (numNodes == 0) {
      throw CommandException("Surface contains no nodes.");
   }
   
   //
   // Read the volume file
   //
   VolumeFile volumeFile;
   volumeFile.readFile(inputVolumeFileName);

   // Create the segmentation volume
   //
   BrainModelSurfaceToVolumeSegmentationConverter bmssc(&brainSet,
                                                        bms,
                                                        &volumeFile,
                                                        false,
                                                        false);
   bmssc.execute();
   volumeFile.setDescriptiveLabel(outputSegmentationVolumeFileLabel);
   volumeFile.writeFile(outputSegmentationVolumeFileName);
   
   //
   // Expand around edges with empty slices
   //
   VolumeFile segmentVolumeExpanded(volumeFile);
   int expDim[3];
   segmentVolumeExpanded.getDimensions(expDim);
   const int expSlices = 7;
   const int resizeCrop[6] = { 
      -expSlices, expDim[0] + expSlices,
      -expSlices, expDim[1] + expSlices,
      -expSlices, expDim[2] + expSlices
   };
   segmentVolumeExpanded.resize(resizeCrop);

   //
   // Generate the hull VTK file and volume
   //
   VolumeFile* hullVolume = NULL;
   vtkPolyData* hullPolyData = NULL;
   brainSet.generateCerebralHullVtkFile(&segmentVolumeExpanded, 
                                        hullVolume,
                                        hullPolyData);
                                        
   //
   // Write the hull volume
   //
   hullVolume->setDescriptiveLabel(outputHullVolumeFileLabel);
   hullVolume->writeFile(outputHullVolumeFileName);
   delete hullVolume;
   
   //
   // Write the Hull VTK file
   //
   vtkPolyDataWriter* writer = vtkPolyDataWriter::New();
#ifdef HAVE_VTK6
   writer->SetInputData(hullPolyData);
#else
   writer->SetInput(hullPolyData);
#endif
   writer->SetHeader("Written by Caret");
   writer->SetFileName((char*)outputCerebralHullVtkFileName.toAscii().constData());
   writer->Write();

   writer->Delete();
   hullPolyData->Delete();
}

      

