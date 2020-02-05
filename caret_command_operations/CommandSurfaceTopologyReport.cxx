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
#include "CommandSurfaceTopologyReport.h"
#include "CoordinateFile.h"
#include "FileFilters.h"
#include "ProgramParameters.h"
#include "ScriptBuilderParameters.h"
#include "TopologyFile.h"
/**
 * constructor.
 */
CommandSurfaceTopologyReport::CommandSurfaceTopologyReport()
   : CommandBase("-surface-topology-report",
                 "SURFACE TOPOLOGY REPORT")
{
}

/**
 * destructor.
 */
CommandSurfaceTopologyReport::~CommandSurfaceTopologyReport()
{
}

/**
 * get the script builder parameters.
 */
void 
CommandSurfaceTopologyReport::getScriptBuilderParameters(ScriptBuilderParameters& paramsOut) const
{
   paramsOut.clear();
   paramsOut.addFile("Coordinate File Name", 
                     FileFilters::getCoordinateGenericFileFilter());
   paramsOut.addFile("Topology File Name", 
                     FileFilters::getTopologyGenericFileFilter());
}

/**
 * get full help information.
 */
QString 
CommandSurfaceTopologyReport::getHelpInformation() const
{
   QString helpInfo =
      (indent3 + getShortDescription() + "\n"
       + indent6 + parameters->getProgramNameWithoutPath() + " " + getOperationSwitch() + "  \n"
       + indent9 + "<input-coordinate-file-name>\n"
       + indent9 + "<input-topology-file-name>\n"
       + indent9 + "\n"
       + indent9 + "Examine and report on the surface's topology.\n"
       + indent9 + "\n");
      
   return helpInfo;
}

/**
 * execute the command.
 */
void 
CommandSurfaceTopologyReport::executeCommand() throw (BrainModelAlgorithmException,
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
   const QString topologyFileName =
      parameters->getNextParameterAsString("Input Topology File Name");

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
   BrainSet brainSet(topologyFileName,
                     coordinateFileName,
                     "",
                     true);
   BrainModelSurface* bms = brainSet.getBrainModelSurface(0);
   if (bms == NULL) {
      throw CommandException("unable to find surface.");
   }
   const TopologyFile* tf = bms->getTopologyFile();
   if (tf == NULL) {
      throw CommandException("unable to find topology.");
   }

   //
   // Determine if the surface is 2D or 3D
   // 
   bool twoDimFlag = true;
   const CoordinateFile* cf = bms->getCoordinateFile();
   const int numCoords = cf->getNumberOfCoordinates();
   for (int i = 0; i < numCoords; i++) {
      const float* xyz = cf->getCoordinate(i);
      if (xyz[2] != 0.0) {
         twoDimFlag = false;
         break;
      }
   }
   
   int numFaces, numVertices, numEdges, eulerCount, numHoles, numObjects;
   tf->getEulerCount(twoDimFlag, numFaces, numVertices, numEdges, eulerCount,
                     numHoles, numObjects);

   int correctEulerCount = 2;
   if (twoDimFlag) {
      correctEulerCount = 1;
   }
   if (eulerCount == correctEulerCount) {
      std::cout << "Surface is topologically correct." << std::endl;
   }
   else {
      std::cout << "Surface is NOT topologically correct." << std::endl;
      std::cout << "   Euler Count is " 
                << eulerCount
                << " but should be "
                << correctEulerCount
                << std::endl;
      if (numObjects != 1) {
         std::cout << "   Number of disjoint objects: " 
                   << numObjects << std::endl;
      }
      if (numHoles > 0) {
         std::cout << "   Number of holes: "
                   << numHoles << std::endl;
      }
   }

}

      

