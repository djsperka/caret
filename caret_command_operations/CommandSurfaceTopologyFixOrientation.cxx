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

#include "BrainModelSurface.h"
#include "BrainSet.h"
#include "CommandSurfaceTopologyFixOrientation.h"
#include "CoordinateFile.h"
#include "FileFilters.h"
#include "ProgramParameters.h"
#include "ScriptBuilderParameters.h"
#include "TopologyFile.h"
/**
 * constructor.
 */
CommandSurfaceTopologyFixOrientation::CommandSurfaceTopologyFixOrientation()
   : CommandBase("-surface-topology-fix-orientation",
                 "SURFACE TOPOLOGY FIX ORIENTATION")
{
}

/**
 * destructor.
 */
CommandSurfaceTopologyFixOrientation::~CommandSurfaceTopologyFixOrientation()
{
}

/**
 * get the script builder parameters.
 */
void
CommandSurfaceTopologyFixOrientation::getScriptBuilderParameters(ScriptBuilderParameters& paramsOut) const
{
   paramsOut.clear();
   paramsOut.addFile("Coordinate File Name",
                     FileFilters::getCoordinateGenericFileFilter());
   paramsOut.addFile("Input Topology File Name",
                     FileFilters::getTopologyGenericFileFilter());
   paramsOut.addFile("Output Topology File Name",
                     FileFilters::getTopologyGenericFileFilter());
}

/**
 * get full help information.
 */
QString
CommandSurfaceTopologyFixOrientation::getHelpInformation() const
{
   QString helpInfo =
      (indent3 + getShortDescription() + "\n"
       + indent6 + parameters->getProgramNameWithoutPath() + " " + getOperationSwitch() + "  \n"
       + indent9 + "<input-coordinate-file-name>\n"
       + indent9 + "<input-topology-file-name>\n"
       + indent9 + "<output-topology-file-name>\n"
       + indent9 + "\n"
       + indent9 + "Fix surface topology by consistently ordering the.\n"
       + indent9 + "vertices in the triangles so that the node normal\n"
       + indent9 + "vectors point out of the surface.\n"
       + indent9 + "\n");

   return helpInfo;
}

/**
 * execute the command.
 */
void
CommandSurfaceTopologyFixOrientation::executeCommand() throw (BrainModelAlgorithmException,
                                     CommandException,
                                     FileException,
                                     ProgramParametersException,
                                     StatisticException)
{
   //
   // Get the parameters
   //
   const QString coordinateFileName =
      parameters->getNextParameterAsString("Input Coordinate File Name");
   const QString inputTopologyFileName =
      parameters->getNextParameterAsString("Input Topology File Name");
   const QString outputTopologyFileName =
      parameters->getNextParameterAsString("Output Topology File Name");

   //
   // Make sure that are no more parameters
   //
   checkForExcessiveParameters();

   //
   // Create a brain set
   //
   // DJS This call apparently relies on a cast of the bool to an empty string?
   // I am guessing here that the intended constructor is the one with coord file name,
   // an optional second coord file (which defaults to empty string) and a single bool.
   // I am guessing that the two bool's originally supplied here were intended to align
   // with the constructor that has 'bool readAllFilesInSpec' and 'bool primaryBrainSetFlagIn',
   // and since the one I'm targeting only has ' bool primaryBrainSetFlagIn', we retain the 'true'
   // arg.
   //BrainSet brainSet(inputTopologyFileName,
   //                  coordinateFileName,
   //                  false,
   //                  true);
   BrainSet brainSet(inputTopologyFileName,
                     coordinateFileName,
                     "",
                     true);


   BrainModelSurface* bms = brainSet.getBrainModelSurface(0);
   if (bms == NULL) {
      throw CommandException("unable to find surface.");
   }

   /*
    * Orient the nodes in the triangles consistently
    * so that they point out of the surface.
    */
   bms->orientTilesConsistently();
   bms->orientNormalsOut();

   TopologyFile* tf = bms->getTopologyFile();
   if (tf == NULL) {
      throw CommandException("unable to find topology.");
   }

   /*
    * Write the topology file.
    */
   tf->writeFile(outputTopologyFileName);
}



