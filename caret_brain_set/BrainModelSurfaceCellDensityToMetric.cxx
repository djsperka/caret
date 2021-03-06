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
#include "BrainModelSurfaceCellDensityToMetric.h"
#include "BrainSet.h"
#include "CellFile.h"
#include "CellProjectionFile.h"
#include "MetricFile.h"

/**
 * Constructor.
 */
BrainModelSurfaceCellDensityToMetric::BrainModelSurfaceCellDensityToMetric(
                                     BrainSet* bsIn,
                                     BrainModelSurface* flatSurfaceIn,
                                     CellProjectionFile* cellProjectionFileIn,
                                     MetricFile* metricFileIn,
                                     const float gridSpacingIn,
                                     const bool displayedCellsOnlyFlagIn)
   : BrainModelAlgorithm(bsIn)
{
   flatSurface = flatSurfaceIn;
   cellProjectionFile = cellProjectionFileIn;
   metricFile = metricFileIn;
   gridSpacing = gridSpacingIn;
   displayedCellsOnlyFlag = displayedCellsOnlyFlagIn;   
}

/**
 * Constructor.
 */
BrainModelSurfaceCellDensityToMetric::~BrainModelSurfaceCellDensityToMetric()
{
}

/**
 * execute the algorithm.
 */
void 
BrainModelSurfaceCellDensityToMetric::execute() throw (BrainModelAlgorithmException)
{
   //
   // Check inputs
   //
   if (flatSurface == NULL) {
      throw BrainModelAlgorithmException("The flat surface is invalid.");
   }
   if (cellProjectionFile == NULL) {
      throw BrainModelAlgorithmException("Cell Projection File is invalid.");
   }
   if (metricFile == NULL) {
      throw BrainModelAlgorithmException("Metric File is invalid.");
   }
   if (gridSpacing <= 0.0) {
      throw BrainModelAlgorithmException("Grid spacing is invalid.");
   }
   
   //
   // Get the coordinate file for the flat surface
   //
   const CoordinateFile* coordFile = flatSurface->getCoordinateFile();
   
   //
   // Get the flat cell file
   //
   CellFile cellFile;
   cellProjectionFile->getCellFile(flatSurface->getCoordinateFile(),
                    flatSurface->getTopologyFile(),
                    false,
                    cellFile);
   if (cellFile.getNumberOfCells() <= 0) {
      throw BrainModelAlgorithmException(
         "No cells on flat surface.  Do you need to project cells?");
   }
   
   //
   // Get the bounds of the coordinate file
   //
   float bounds[6];
   coordFile->getBounds(bounds);
   const float xmin = bounds[0] - 1.0;
   const float xmax = bounds[1] + 1.0;
   const float ymin = bounds[2] - 1.0;
   const float ymax = bounds[3] + 1.0;
   
   //
   // Compute the grid dimensions
   //
   const int numGridX = static_cast<int>( ((xmax - xmin) / gridSpacing) + 1.0);
   const int numGridY = static_cast<int>( ((ymax - ymin) / gridSpacing) + 1.0);

   const int arraySize = numGridX * numGridY;
   if (arraySize <= 0) {
      throw BrainModelAlgorithmException("Grid is two small.  Is there a flat surface?");
   }
   
   int* cellTotal = new int[arraySize];
   int* cellDeep  = new int[arraySize];
   int* cellSuperficial = new int[arraySize];
   for (int k = 0; k < arraySize; k++) {
      cellTotal[k] = 0;
      cellDeep[k]  = 0;
      cellSuperficial[k] = 0;
   }
   
   //
   // Loop through the cells
   //
   const int numCells = cellFile.getNumberOfCells();
   int num = 0;
   for (int i = 0; i < numCells; i++) {
      const CellData* cd = cellFile.getCell(i);
      
      bool useIt = true;
      if (displayedCellsOnlyFlag) {
         useIt = cd->getDisplayFlag();
      }
      
      if (useIt) {
         //
         // Determine grid coordinate
         //
         const float* cellPos = cd->getXYZ();         
         const int xi = static_cast<int>((cellPos[0] - xmin) / gridSpacing);
         const int yi = static_cast<int>((cellPos[1] - ymin) / gridSpacing);
         
         if (xi >= numGridX) {
            std::cerr << "Invalid x grid index "<<  xi << " for cell " << i << std::endl;
         }
         else if (yi >= numGridY) {
            std::cerr << "Invalid x grid index "<<  yi << " for cell " << i << std::endl;
         }
         else {
            const int index = (numGridX * yi) + xi;
            if (index >= arraySize) {
               std::cerr << "Invalid index " << index
                         << " greater than last " << arraySize << std::endl;
            }
            else {
               cellTotal[index]++;
               if (cellPos[2] < 0.0) {   //if (cd->getSignedDistanceToSurface() < 0.0) {
                  cellDeep[index]++;
               }
               else {
                  cellSuperficial[index]++;
               }
               num++;
            }
         }
      }
   }   
 
   //
   // Find/Add columns to metric file
   //
   int allColumn = -1;
   int deepColumn = -1;
   int superficialColumn = -1;
   
   const QString allColumnName("All Cells");
   const QString deepColumnName("Deep Layers");
   const QString superficialColumnName("Superficial Layers");
   
   if (metricFile->getNumberOfColumns() == 0) {
      metricFile->setNumberOfNodesAndColumns(flatSurface->getNumberOfNodes(), 3);
      metricFile->setFileTitle("Cell Density");
      allColumn = 0;
      deepColumn = 1;
      superficialColumn = 2;
   }
   else {
      allColumn = metricFile->getColumnWithName(allColumnName);
      if (allColumn < 0) {
         allColumn = metricFile->getNumberOfColumns();
         metricFile->addColumns(1);
      }
      deepColumn = metricFile->getColumnWithName(deepColumnName);
      if (deepColumn < 0) {
         deepColumn = metricFile->getNumberOfColumns();
         metricFile->addColumns(1);
      }
      superficialColumn = metricFile->getColumnWithName(superficialColumnName);
      if (superficialColumn < 0) {
         superficialColumn = metricFile->getNumberOfColumns();
         metricFile->addColumns(1);
      }
   }
   metricFile->setColumnName(allColumn, allColumnName);
   metricFile->setColumnName(deepColumn, deepColumnName);
   metricFile->setColumnName(superficialColumn, superficialColumnName);
   
   //
   // Loop through nodes
   //
   const int numCoords = coordFile->getNumberOfCoordinates();
   for (int j = 0; j < numCoords; j++) {
   
      //
      // Get grid coordinate of node
      //
      const float* coordPos = coordFile->getCoordinate(j);      
      const int xi = static_cast<int>((coordPos[0] - xmin) / gridSpacing);
      const int yi = static_cast<int>((coordPos[1] - ymin) / gridSpacing);
 
      if (xi >= numGridX) {
         std::cerr << "Invalid x grid index "<<  xi << " for node " << j << std::endl;
      }
      else if (yi >= numGridY) {
         std::cerr << "Invalid y grid index "<<  yi << " for node " << j << std::endl;
      }
      else {
         const int index = (numGridX * yi) + xi;
         if (index >= arraySize) {
            std::cerr << "Invalid index " << index
                      << " greater than last " << arraySize << std::endl;
         }
         else {
            //
            // Set the metrics for the node
            //
            float metric[3];
            metric[0] = 0.0;  //static_cast<double>(cellTotal[index]);
            metric[1] = 0.0;  //static_cast<double>(cellDeep[index]);
            metric[2] = 0.0;  //static_cast<double>(cellSuperficial[index]);
            if (cellTotal[index] > 0.0) {
               metric[0] = log(static_cast<double>(cellTotal[index]));
            }
            if (cellDeep[index] > 0.0) {
               metric[1] = log(static_cast<double>(cellDeep[index]));
            }
            if (cellSuperficial[index] > 0.0) {
               metric[2] = log(static_cast<double>(cellSuperficial[index]));
            }

            metricFile->setValue(j, allColumn, metric[0]);
            metricFile->setValue(j, deepColumn, metric[1]);
            metricFile->setValue(j, superficialColumn, metric[2]);
         }
      }
   }
   
   delete[] cellTotal;
   delete[] cellDeep;
   delete[] cellSuperficial;
}


