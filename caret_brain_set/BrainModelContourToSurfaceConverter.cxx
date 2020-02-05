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

#include <QGlobalStatic>
#ifdef Q_OS_WIN32
#define NOMINMAX
#endif

#include <iostream>
#include <limits>

#include "BrainModelContourToSurfaceConverter.h"
#include "BrainModelSurface.h"
#include "BrainModelSurfaceNodeColoring.h"
#include "BrainSet.h"
#include "CellColorFile.h"
#include "CellProjectionFile.h"
#include "ContourCellFile.h"
#include "ContourCellColorFile.h"
#include "ContourFile.h"
#include "DebugControl.h"
#include "SectionFile.h"
#include "TransformationMatrixFile.h"
#include "vtkCellArray.h"
#include "vtkCleanPolyData.h"
#include "vtkDecimatePro.h"
#include "vtkPoints.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPolyData.h"
#include "vtkPolyDataWriter.h"
#include "vtkTriangleFilter.h"
#include "vtkVoxelContoursToSurfaceFilter.h"

//#define USE_VTK_DECIMATE_CARET  1
#ifdef USE_VTK_DECIMATE_CARET
#include "vtkDecimateCaret.h"
#endif // USE_VTK_DECIMATE_CARET

/**
 * Constructor.
 */
BrainModelContourToSurfaceConverter::BrainModelContourToSurfaceConverter(
                                    BrainSet* bs,
                                    ContourFile* contourFileIn,
                                    const int voxelDimIn,
                                    const int polygonLimitIn,
                                    const Structure::STRUCTURE_TYPE structureIn,
                                    const bool convertCellsIn)
   : BrainModelAlgorithm(bs)
{
   contourFile         = contourFileIn;
   voxelDimension      = voxelDimIn;
   polygonLimit        = polygonLimitIn;
   structure           = structureIn;
   convertCellsFlag    = convertCellsIn;
}
                                       
/**
 * Destructor.
 */
BrainModelContourToSurfaceConverter::~BrainModelContourToSurfaceConverter()
{
}
                                       
/**
 * execute the algorithm.
 */
void 
BrainModelContourToSurfaceConverter::execute() throw (BrainModelAlgorithmException)
{
   //
   // Make sure contour file is valid and has contours
   //
   if (contourFile == NULL) {
      throw BrainModelAlgorithmException("Contour File is invalid.");
   }

   const int numContours = contourFile->getNumberOfContours();
   if (contourFile->getNumberOfContours() < 0) {
      throw BrainModelAlgorithmException("Contour File contains no contours.");
   }
      
   //
   // VTK requires that section numbers are increasing.
   //
   contourFile->sortBySectionNumber();
   
   //
   // Create the VTK data structures.
   //
   vtkPolyData* contours = vtkPolyData::New();
   vtkPoints* pointData = vtkPoints::New();
   vtkCellArray* cells = vtkCellArray::New();
   
   const float spacing = contourFile->getSectionSpacing();
   
   //
   // VTK requires that Z values are consecutive integers so
   // handle any skips in the section numbers.
   //
   int lastSectionNumber = 0;
   bool lastSectionNumberValid = false;
   int vtkSectionNumber = 1;
   
   float minSectionZ =  std::numeric_limits<float>::max();
   float maxSectionZ = -std::numeric_limits<float>::max();
   
   //
   // Use VTK's piecewise function to convert from a VTK consecutive
   // section number to the true Z of the section
   //
   vtkPiecewiseFunction* vtkSectionToZ = vtkPiecewiseFunction::New();
 
   for (int i = 0; i < numContours; i++) {
      CaretContour* cc = contourFile->getContour(i);
      const int numPoints = cc->getNumberOfPoints();
      const int sectionNumber = cc->getSectionNumber();
      if (numPoints < 3) {
         std::cout << "Contour " << i << " section " << sectionNumber
                   << " has less than three points, ignored" << std::endl;
         continue;
      }
            
      vtkIdType* verts = new vtkIdType[numPoints];

      if (lastSectionNumberValid) {
         if (sectionNumber != lastSectionNumber) {
            vtkSectionNumber++;
            const float sectionZ = sectionNumber * spacing;
            vtkSectionToZ->AddPoint(vtkSectionNumber, sectionZ);
            if (sectionZ < minSectionZ) minSectionZ = sectionZ;
            if (sectionZ > maxSectionZ) maxSectionZ = sectionZ;
         }
      }
      else {
         const float sectionZ = sectionNumber * spacing;
         vtkSectionToZ->AddPoint(vtkSectionNumber, sectionZ);
         if (sectionZ < minSectionZ) minSectionZ = sectionZ;
         if (sectionZ > maxSectionZ) maxSectionZ = sectionZ;
      }
      
      if (DebugControl::getDebugOn()) {
         std::cout << "Contour section=" << sectionNumber
                   << ", VTK-section=" << vtkSectionNumber << std::endl;
      }
      
      vtkIdType pointCount = 0;
      for (int j = 0; j < numPoints; j++) {
         float x, y;
         cc->getPointXY(j, x, y);
         verts[pointCount] = pointData->InsertNextPoint(x, y, vtkSectionNumber);
         pointCount++;
      }
      
      lastSectionNumber = sectionNumber;
      lastSectionNumberValid = true;
      
      if (pointCount > 2) {
         cells->InsertNextCell(pointCount, verts);
      }
      else {
         printf("Contour %d has less than 3 points\n", i);
      }
   }
   vtkSectionToZ->ClampingOn();
   
   //
   // Get extent of contours
   //
   float minX, minY, maxX, maxY;
   contourFile->getExtent(minX, maxX, minY, maxY);
   if (DebugControl::getDebugOn()) {
      std::cout << "Contour Ranges: "  
                << "(" << minX << ", " << maxX << ") "
                << "(" << minY << ", " << maxY << ") "
                << "(" << minSectionZ << ", " << maxSectionZ << ") "
                << lastSectionNumber << std::endl;
   }
   float minZ =  std::numeric_limits<float>::max();
   float maxZ = -std::numeric_limits<float>::max();
   minX =  std::numeric_limits<float>::max();
   maxX = -std::numeric_limits<float>::max();
   minY =  std::numeric_limits<float>::max();
   maxY = -std::numeric_limits<float>::max();
   const int numPoints = pointData->GetNumberOfPoints();
   for (int m = 0; m < numPoints; m++) {
#ifdef HAVE_VTK5
      double xyz[3];
#else // HAVE_VTK5
      float xyz[3];
#endif // HAVE_VTK5
      pointData->GetPoint(m, xyz);
      
      if (xyz[0] > maxX) maxX = xyz[0];
      if (xyz[0] < minX) minX = xyz[0];
      if (xyz[1] > maxY) maxY = xyz[1];
      if (xyz[1] < minY) minY = xyz[1];
      if (xyz[2] > maxZ) maxZ = xyz[2];
      if (xyz[2] < minZ) minZ = xyz[2];
   }
   const float contourMinX = minX;
   const float contourMinY = minY;
   if (DebugControl::getDebugOn()) {
      std::cout << "Contour Convertex to Voxel Ranges: "  
                << "(" << minX << ", " << maxX << ") "
                << "(" << minY << ", " << maxY << ") "
                << "(" << minZ << ", " << maxZ << ") "
                << std::endl;
   }
   
   //
   // Scale to fit within volume
   //
   const float volumeSize = static_cast<float>(voxelDimension);
   const float dx = maxX - minX;
   const float dy = maxY - minY;
   float greatest = dx;
   if (dy > greatest) {
      greatest = dy;
   }
   
   const float xyScale = (volumeSize - 4.0) / greatest;
   
   //
   // Setup matrix to
   //    1) Translate so (minX, minY) is at the origin.
   //    2) Scale to fit inside volume
   //    3) Translate so (minX, minY) is at 2, 2
   //
   TransformationMatrix matrix;
   matrix.translate(-contourMinX, -contourMinY, 0.0f);
   matrix.scale(xyScale, xyScale, 1.0f);
   matrix.translate(2.0, 2.0, 0.0);
   
   minZ =  std::numeric_limits<float>::max();
   maxZ = -std::numeric_limits<float>::max();
   minX =  std::numeric_limits<float>::max();
   maxX = -std::numeric_limits<float>::max();
   minY =  std::numeric_limits<float>::max();
   maxY = -std::numeric_limits<float>::max();
   
   //
   // Apply transformation to the surface points
   //
   for (int m = 0; m < numPoints; m++) {
#ifdef HAVE_VTK5
      double xyz[3];
#else // HAVE_VTK5
      float xyz[3];
#endif // HAVE_VTK5
      pointData->GetPoint(m, xyz);
      double xyzw[4] = { xyz[0], xyz[1], xyz[2], 1.0 };
      matrix.multiplyPoint(xyzw);
      xyz[0] = xyzw[0];
      xyz[1] = xyzw[1];
      xyz[2] = xyzw[2];

      //
      // "Round" the coordinates.  vtkVoxelContoursToSurfaceFilter expects integers.  This must
      // be done after transforming the points or else the surface will look super noisyish.
      //
      if (xyz[0] > 0.0) xyz[0] += 0.5;
      else if (xyz[0] < 0.0) xyz[0] -= 0.5; 
      if (xyz[1] > 0.0) xyz[1] += 0.5;
      else if (xyz[1] < 0.0) xyz[1] -= 0.5; 
      xyz[0] = static_cast<int>(xyz[0]);
      xyz[1] = static_cast<int>(xyz[1]);
      pointData->SetPoint(m, xyz);
      
      if (xyz[0] > maxX) maxX = xyz[0];
      if (xyz[0] < minX) minX = xyz[0];
      if (xyz[1] > maxY) maxY = xyz[1];
      if (xyz[1] < minY) minY = xyz[1];
      if (xyz[2] > maxZ) maxZ = xyz[2];
      if (xyz[2] < minZ) minZ = xyz[2];
   }
   if (DebugControl::getDebugOn()) {
      std::cout << "Contour Ranges after Transformation: "  
                << "(" << minX << ", " << maxX << ") "
                << "(" << minY << ", " << maxY << ") "
                << "(" << minZ << ", " << maxZ << ") "
                << std::endl;
   }
    
   contours->SetPoints(pointData);
   contours->SetPolys(cells);
   
   cells->Delete();
   pointData->Delete();
   
   if (DebugControl::getDebugOn()) {
      vtkPolyDataWriter* contourWriter = vtkPolyDataWriter::New();
#ifdef HAVE_VTK6
      contourWriter->SetInputData(contours);
#else
      contourWriter->SetInput(contours);
#endif
      contourWriter->SetHeader("");
      contourWriter->SetFileName("contours.vtk");
      contourWriter->Write();
      contourWriter->Delete();
   }
   
   //
   // run a filter to convert the contours to a surface
   //
   vtkVoxelContoursToSurfaceFilter *surface = 
                          vtkVoxelContoursToSurfaceFilter::New();
#ifdef HAVE_VTK6
   surface->SetInputData(contours);
#else
   surface->SetInput(contours);
#endif
   surface->SetMemoryLimitInBytes(1000000000);
   
   //
   // Save the surface to a VTK PolyData file.
   //
   if (DebugControl::getDebugOn()) {
      vtkPolyDataWriter* writer1 = vtkPolyDataWriter::New();
#ifdef HAVE_VTK6
      writer1->SetInputConnection(surface->GetOutputPort());
#else
      writer1->SetInput(surface->GetOutput());
#endif
      writer1->SetHeader("");
      writer1->SetFileName("raw_surface.vtk");
      writer1->Write();
      writer1->Delete();
      std::cout << "..._raw_surface.vtk contains the surface from vtkVoxelContoursToSurfaceFilter" << endl;
   }
   
   //
   // Make sure mesh is only triangles
   //
   vtkTriangleFilter *triangleFilter = vtkTriangleFilter::New();
#ifdef HAVE_VTK6
   triangleFilter->SetInputConnection(surface->GetOutputPort());
#else
   triangleFilter->SetInput(surface->GetOutput());
#endif
   triangleFilter->Update();
   
   //
   // Find number of triangles
   //
   vtkPolyData* surfaceData = triangleFilter->GetOutput();
   const int numberOfPolygons = surfaceData->GetPolys()->GetNumberOfCells();
   if (numberOfPolygons <= 0) {
      triangleFilter->Delete();
      surface->Delete();
      contours->Delete();
      throw BrainModelAlgorithmException("Reconstruction failed.\n"
                                         "Are the sections in the XY-plane?");
   }
   
   //
   // Convert Z (section numbers) to true Z, untranslate, and unscale the surface
   //
   vtkPoints *surfacePoints = surfaceData->GetPoints();
   for (int nn = 0; nn < surfacePoints->GetNumberOfPoints(); nn++) {
#ifdef HAVE_VTK5
      double xyz[3];
#else // HAVE_VTK5
      float xyz[3];
#endif // HAVE_VTK5
      surfacePoints->GetPoint(nn, xyz);
      xyz[2] = vtkSectionToZ->GetValue(xyz[2]);
      surfacePoints->SetPoint(nn, xyz);
   }

   vtkCleanPolyData* clean1 = NULL;
   vtkCleanPolyData* clean2 = NULL;

#ifdef USE_VTK_DECIMATE_CARET
   //
   // Decimate the VTK surface if it is large
   //
   vtkDecimateCaret* decimater = NULL;
   if (polygonLimit < numberOfPolygons) {
      if (DebugControl::getDebugOn()) {
         std::cout << "Before decimation surface has " << numberOfPolygons
                   << " polygons." << std::endl;
      }
      decimater = vtkDecimateCaret::New();
      decimater->SetInput(surfaceData);
      const float reduction = 1.0 - ((float)polygonLimit /
                                     (float)numberOfPolygons);
      if (DebugControl::getDebugOn()) {
         std::cout << "Reduction is " << reduction * 100.0 << "%" << std::endl;
      }
      decimater->SetTargetReduction(reduction);
      decimater->PreserveTopologyOn();
      decimater->SetMaximumError(VTK_LARGE_FLOAT);
      decimater->BoundaryVertexDeletionOff();
      //decimater->SplittingOff();
      decimater->Update();
      
      surfaceData = decimater->GetOutput();
      
      if (DebugControl::getDebugOn()) {
         std::cout << "After decimation surface has "
                   << surfaceData->GetPolys()->GetNumberOfCells()
                   << std::endl;
      }
   }
#else // USE_VTK_DECIMATE_CARET
   //
   // Decimate the VTK surface if it is large
   //   
   vtkDecimatePro* decimater = NULL;
   if (polygonLimit < numberOfPolygons) {
   
      clean1 = vtkCleanPolyData::New();
#ifdef HAVE_VTK6
      clean1->SetInputData(surfaceData);
#else
      clean1->SetInput(surfaceData);
#endif
      if (DebugControl::getDebugOn()) {
         std::cout << "Before decimation surface has " << numberOfPolygons
                   << " polygons." << std::endl;
      }
      const double errorVal = 0.001;
      const double reduction = 1.0 - static_cast<double>(polygonLimit) / static_cast<double>(numberOfPolygons);
      decimater = vtkDecimatePro::New();
#ifdef HAVE_VTK6
      decimater->SetInputConnection(clean1->GetOutputPort());
#else
      decimater->SetInput(clean1->GetOutput());
#endif
      decimater->SetTargetReduction(reduction); //0.90);
      decimater->PreserveTopologyOn();
      decimater->SetFeatureAngle(180.0); //5); //30);
      decimater->SplittingOff();
      decimater->PreSplitMeshOff();
      decimater->SetMaximumError(errorVal);
      decimater->BoundaryVertexDeletionOff();
      decimater->SetDegree(25);
      decimater->AccumulateErrorOn();
      decimater->SetAbsoluteError(errorVal);
      decimater->SetErrorIsAbsolute(1);
      
      vtkCleanPolyData* clean2 = vtkCleanPolyData::New();
#if HAVE_VTK6
      clean2->SetInputConnection(decimater->GetOutputPort());
#else
      clean2->SetInput(decimater->GetOutput());
#endif
      clean2->Update();
      surfaceData = clean2->GetOutput();
      
      if (DebugControl::getDebugOn()) {
         std::cout << "After decimation surface has "
                   << surfaceData->GetPolys()->GetNumberOfCells()
                   << std::endl;
      }
   }
#endif
   
   minZ =  std::numeric_limits<float>::max();
   maxZ = -std::numeric_limits<float>::max();
   minX =  std::numeric_limits<float>::max();
   maxX = -std::numeric_limits<float>::max();
   minY =  std::numeric_limits<float>::max();
   maxY = -std::numeric_limits<float>::max();
   
   //
   // Invert the matrix for converting back into contour space
   //
   matrix.inverse();
   
   //
   // Convert Z (section numbers) to true Z, untranslate, and unscale the surface
   //
   surfacePoints = surfaceData->GetPoints();
   for (int nn = 0; nn < surfacePoints->GetNumberOfPoints(); nn++) {
#ifdef HAVE_VTK5
      double xyz[3];
#else // HAVE_VTK5
      float xyz[3];
#endif // HAVE_VTK5
      surfacePoints->GetPoint(nn, xyz);
      double xyzw[4] = { xyz[0], xyz[1], xyz[2], 1.0 };
      matrix.multiplyPoint(xyzw);
      xyz[0] = xyzw[0];
      xyz[1] = xyzw[1];
      xyz[2] = xyzw[2];
      //xyz[2] = vtkSectionToZ->GetValue(xyz[2]);
      surfacePoints->SetPoint(nn, xyz);
      
      if (xyz[0] > maxX) maxX = xyz[0];
      if (xyz[0] < minX) minX = xyz[0];
      if (xyz[1] > maxY) maxY = xyz[1];
      if (xyz[1] < minY) minY = xyz[1];
      if (xyz[2] > maxZ) maxZ = xyz[2];
      if (xyz[2] < minZ) minZ = xyz[2];
   }
   if (DebugControl::getDebugOn()) {
      std::cout << "Reconstructed Surface Ranges: "  
                << "(" << minX << ", " << maxX << ") "
                << "(" << minY << ", " << maxY << ") "
                << "(" << minZ << ", " << maxZ << ") "
                << lastSectionNumber << std::endl;
   }
   
   
   //
   // Save the surface to a VTK PolyData file.
   //
   if (DebugControl::getDebugOn()) {
      vtkPolyDataWriter* writer = vtkPolyDataWriter::New();
#ifdef HAVE_VTK6
      writer->SetInputData(surfaceData);
#else
      writer->SetInput(surfaceData);
#endif
      writer->SetHeader("");
      writer->SetFileName("surface.vtk");
      writer->Write();
      writer->Delete();
      std::cout << "...surface.vtk contains the surface" << endl;
   }
   
   //
   // Since reconstruction successful clear out any existing surfaces
   //
   brainSet->deleteAllBrainModelSurfaces();
   
   //
   // Convert to vtk file to a brain model surface
   //
   try {
      brainSet->importVtkTypeFileHelper("surface.vtk", surfaceData,
                                     true, true, false,
                                     BrainModelSurface::SURFACE_TYPE_RAW,
                                     TopologyFile::TOPOLOGY_TYPE_CLOSED);
   }
   catch (FileException& e) {
      throw BrainModelAlgorithmException(e.whatQString());
   }
   
   //
   // Newest brain model should be the surface
   //
   BrainModelSurface* bms = brainSet->getBrainModelSurface(brainSet->getNumberOfBrainModels() - 1);
   if (bms != NULL) {
      bms->orientNormalsOut();
      bms->setStructure(structure);
      bms->setSurfaceType(BrainModelSurface::SURFACE_TYPE_RAW);
   }
   
   //
   // Release memory
   //
   if (decimater != NULL) decimater->Delete();
   if (clean1 != NULL) clean1->Delete();
   if (clean2 != NULL) clean2->Delete();
   triangleFilter->Delete();
   surface->Delete();
   contours->Delete();
   
   brainSet->getNodeColoring()->assignColors();
   
   //
   // Create sections
   //
   if (bms != NULL) {
      const CoordinateFile* cf = bms->getCoordinateFile();
      const int numNodes = cf->getNumberOfCoordinates();
      if (numNodes > 0) {
         SectionFile* sf = brainSet->getSectionFile();
         sf->setNumberOfNodesAndColumns(numNodes, 1);
         sf->setColumnName(0, "Reconstruction");
         sf->setColumnComment(0, "Created during reconstruction from contours.");
         for (int i = 0; i < numNodes; i++) {
            const float* xyz = cf->getCoordinate(i);
            sf->setSection(i, 0, static_cast<int>(xyz[2] / spacing));
         }
      }
   }
   
   //
   // Convert cells if desired
   //
   if (convertCellsFlag) {
      ContourCellFile* contourCellFile = brainSet->getContourCellFile();
      const int numContourCells = contourCellFile->getNumberOfCells();
      if (numContourCells > 0) {
         brainSet->deleteAllCells(true, false);
         brainSet->getCellProjectionFile()->appendFiducialCellFile(*contourCellFile);
         
         ContourCellColorFile* contourCellColorFile = brainSet->getContourCellColorFile();
         if (contourCellColorFile->getNumberOfColors() > 0) {
            brainSet->clearCellColorFile();
            CellColorFile* cellColor = brainSet->getCellColorFile();
            cellColor->append(*contourCellColorFile);
         }
      }
   }
}
