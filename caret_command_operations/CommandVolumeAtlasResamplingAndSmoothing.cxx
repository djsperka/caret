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
#include <fstream>

#include "CommandVolumeAtlasResamplingAndSmoothing.h"
#include "FileFilters.h"
#include "ProgramParameters.h"
#include "ScriptBuilderParameters.h"
#include "BrainModelVolumeROIAtlasResamplingAndSmoothing.h"
#include "BrainSet.h"
#include "StringUtilities.h"
#include "FileUtilities.h"
#include "VolumeFile.h"

/**
 * constructor.
 */
CommandVolumeAtlasResamplingAndSmoothing::CommandVolumeAtlasResamplingAndSmoothing()
   : CommandBase("-volume-atlas-resampling-and-smoothing",
                 "PARCEL CONSTRAINED ATLAS RESAMPLING AND SMOOTHING")
{
}

/**
 * destructor.
 */
CommandVolumeAtlasResamplingAndSmoothing::~CommandVolumeAtlasResamplingAndSmoothing()
{
}

/**
 * get the script builder parameters.
 */
void 
CommandVolumeAtlasResamplingAndSmoothing::getScriptBuilderParameters(ScriptBuilderParameters& paramsOut) const
{
   paramsOut.clear();
   paramsOut.addFile("Input Volume File", FileFilters::getVolumeFunctionalFileFilter());
   paramsOut.addFile("Input Volume ROI File", FileFilters::getVolumeSegmentationFileFilter());
   paramsOut.addFile("Output Volume File", FileFilters::getVolumeFunctionalFileFilter());
   paramsOut.addFile("Input Atlas Volume ROI File", FileFilters::getVolumeFunctionalFileFilter());
   paramsOut.addFloat("Sigma", 1.0, 0.0);
}

/**
 * get full help information.
 */
QString 
CommandVolumeAtlasResamplingAndSmoothing::getHelpInformation() const
{
   QString helpInfo =
      (indent3 + getShortDescription() + "\n"
       + indent6 + parameters->getProgramNameWithoutPath() + " " + getOperationSwitch() + "  \n"
       + indent9 + "<input-volume>\n"
       + indent9 + "<input-roi>\n"
       + indent9 + "<atlas-volume-region>\n"
       + indent9 + "<output-volume>\n"
       + indent9 + "<sigma>\n"
       + indent9 + "\n"
       + indent9 + "Perform 3D Gaussian weighted smoothing of kernel <sigma> within\n"
       + indent9 + "the given ROI.  The smoothing only considers voxels inside the\n"
       + indent9 + "ROI and the output will only contain these voxels.  The input\n"
       + indent9 + "volume and input ROI must have the same image matrix and\n"
       + indent9 + "dimensions.  The command will accept either 3D or 4D inputs and\n"
       + indent9 + "will do the smoothing frame by frame.  It will output either 3D\n"
       + indent9 + "or 4D outputs.\n"
       + indent9 + "\n"
       + indent9 + "      values-volume      the input file\n"
       + indent9 + "\n"
       + indent9 + "      region-volume      the individual roi\n"
       + indent9 + "\n"
       + indent9 + "      atlas-volume-region   the atlas roi\n"
       + indent9 + "\n"
       + indent9 + "      output-volume      the output file\n"
       + indent9 + "\n"
       + indent9 + "      sigma      sigma for the weighting function for\n"
       + indent9 + "                       smoothing\n"
       + indent9 + "\n"
       + indent9 + "\n");
      
   return helpInfo;
}

/**
 * execute the command.
 */
void 
CommandVolumeAtlasResamplingAndSmoothing::executeCommand() throw (BrainModelAlgorithmException,
                                     CommandException,
                                     FileException,
                                     ProgramParametersException,
                                     StatisticException)
{
   const QString values =
      parameters->getNextParameterAsString("Input Volume File");
   const QString region =
      parameters->getNextParameterAsString("Input Region File");
   const QString atlasRegion =
      parameters->getNextParameterAsString("Input Atlas Region File");
   const QString output =
      parameters->getNextParameterAsString("Output Volume File");
   const float kernel =
      parameters->getNextParameterAsFloat("Kernel Size");
   BrainSet mybs;
   VolumeFile valueFile, regionFile, atlasRegionFile;//, outFile;
   valueFile.readFile(values);
   regionFile.readFile(region);
   atlasRegionFile.readFile(atlasRegion);
   std::vector<VolumeFile *> outFileVec;
   
   BrainModelVolumeROIAtlasResamplingAndSmoothing mysmooth(&mybs, &valueFile, &regionFile, &atlasRegionFile, &outFileVec, kernel);
   mysmooth.execute();
   
   VolumeFile::writeFile(output, outFileVec[0]->getVolumeType(), outFileVec[0]->getVoxelDataType(), outFileVec);
}
