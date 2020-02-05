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

#include "vtkCellArray.h"
#include "vtkCleanPolyData.h"
#include "vtkClipPolyData.h"
#include "vtkDecimatePro.h"
#include "vtkImageGaussianSmooth.h"
#include "vtkImageShrink3D.h"
#include "vtkMarchingCubes.h"
#include "vtkPlane.h"
#include "vtkPolyDataNormals.h"
#include "vtkPolyDataWriter.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkStructuredPoints.h"
#include "vtkTriangleFilter.h"

#include "BrainModelSurface.h"
#include "BrainModelSurfaceNodeColoring.h"
#include "BrainModelVolumeToSurfaceConverter.h"
#include "BrainSet.h"
#include "DebugControl.h"
#include "VolumeFile.h"
#include "VtkModelFile.h"

/**
 * Constructor
 */
BrainModelVolumeToSurfaceConverter::BrainModelVolumeToSurfaceConverter(
                                    BrainSet* bs,
                                    const VolumeFile* segmentationVolumeFileIn,
                                    const RECONSTRUCTION_MODE reconstructionModeIn,
                                    const bool rightHemisphereFlagIn,
                                    const bool leftHemisphereFlagIn,
                                    const bool createHypersmoothSurfaceFlagIn)
   : BrainModelAlgorithm(bs)
{
   segmentationVolumeFile = new VolumeFile(*segmentationVolumeFileIn);
   reconstructionMode = reconstructionModeIn;
   rightHemisphereFlag = rightHemisphereFlagIn;
   leftHemisphereFlag  = leftHemisphereFlagIn;
   createHypersmoothSurfaceFlag = createHypersmoothSurfaceFlagIn;
}

/**
 * Destructor
 */
BrainModelVolumeToSurfaceConverter::~BrainModelVolumeToSurfaceConverter()
{
   if (segmentationVolumeFile != NULL) {
      delete segmentationVolumeFile;
      segmentationVolumeFile = NULL;
   }
}

/**
 * Execute the algorithm.
 */
void 
BrainModelVolumeToSurfaceConverter::execute() throw (BrainModelAlgorithmException)
{
   //
   // If there are any voxels around the edges that are part of the segmentation
   // they will result in a hole in the surface.  So pad the volume by adding one 
   // slice around all edges to prevent holes.
   //
   const int padAroundEdges[6] = { 1, 1, 1, 1, 1, 1 };
   float origin[3];
   segmentationVolumeFile->getOrigin(origin);
   if (DebugControl::getDebugOn()) {
      std::cout << "Origin before padding: "
                << origin[0] << " "
                << origin[1] << " "
                << origin[2] << std::endl;
   }
   
   //
   // Enlarge "this" volume
   //
   int dim[3];
   segmentationVolumeFile->getDimensions(dim);
   const int dimMod[6] = {
      -padAroundEdges[0],
      dim[0] + padAroundEdges[1],
      -padAroundEdges[2],
      dim[1] + padAroundEdges[3],
      -padAroundEdges[4],
      dim[2] + padAroundEdges[5],
   };
   segmentationVolumeFile->resize(dimMod);
   //segmentationVolumeFile->padSegmentation(padAroundEdges, false);

   segmentationVolumeFile->getOrigin(origin);
   if (DebugControl::getDebugOn()) {
      std::cout << "Origin after padding: "
                << origin[0] << " "
                << origin[1] << " "
                << origin[2] << std::endl;
   }
      
   int indices[6];
   float bounds[6];
   segmentationVolumeFile->getNonZeroVoxelExtent(indices, bounds);
   
   if (DebugControl::getDebugOn()) {
      std::cout << "Extent after padding: "
                << "(" << bounds[0] << ", " << bounds[1] << ") "
                << "(" << bounds[2] << ", " << bounds[3] << ") "
                << "(" << bounds[4] << ", " << bounds[5] << ") " << std::endl;
   }
      
   switch (reconstructionMode) {
      case RECONSTRUCTION_MODE_SUREFIT_SURFACE:
         generateSureFitSurface(false);
         break;
      case RECONSTRUCTION_MODE_SUREFIT_SURFACE_MAXIMUM_POLYGONS:
         generateSureFitSurface(true);
         break;
      case RECONSTRUCTION_MODE_VTK_MODEL:
         generateVtkModel(false);
         break;
      case RECONSTRUCTION_MODE_VTK_MODEL_MAXIMUM_POLYGONS:
         generateVtkModel(true);
         break;
      case RECONSTRUCTION_MODE_SOLID_STRUCTURE:
         generateSolidStructure();
         break;
   }
}

/**
 * generate a solid structure model.
 */
void 
BrainModelVolumeToSurfaceConverter::generateSolidStructure() throw (BrainModelAlgorithmException)
{
   //
   // Reconstruct as a VTK model
   //
   generateVtkModel(false);
   
   //
   // Find the reconstructed vtk model
   //
   const int numVtkModels = brainSet->getNumberOfVtkModelFiles();
   if (numVtkModels <= 0) {
      throw BrainModelAlgorithmException("No VTK models were reconstructed.");
   }
   VtkModelFile* vtkModel = brainSet->getVtkModelFile(numVtkModels - 1);
      
   //
   // Copy the segmentation
   //
   VolumeFile segmentation(*segmentationVolumeFile);
   
   //
   // Erode the segmentation volume to remove the outer layer of voxels
   //
   segmentation.doVolMorphOps(0, 1);
   
   //
   // Get the dimensions of the volume
   //
   int dim[3];
   segmentation.getDimensions(dim);
   
   //
   // Create a point for each non-zero voxel
   //
   for (int i = 0; i < dim[0]; i++) {
      for (int j = 0; j < dim[1]; j++) {
         for (int k = 0; k < dim[2]; k++) {
            if (segmentation.getVoxel(i, j, k, 0) != 0.0) {
               float xyz[3];
               segmentation.getVoxelCoordinate(i, j, k, xyz);
               vtkModel->addCoordinate(xyz);
            }
         }
      }
   }
}

/**
 * Generate surface.
 */
void 
BrainModelVolumeToSurfaceConverter::generateSureFitSurface(const bool maxPolygonsFlag) throw (BrainModelAlgorithmException)
{
   //
   // Convert to structured points
   //
   vtkStructuredPoints* sp = segmentationVolumeFile->convertToVtkStructuredPoints();

   double bounds[6];
   sp->GetBounds(bounds);
   if (DebugControl::getDebugOn()) {
      std::cout << "Segmentation input volume bounds: "
         << bounds[0] << " " << bounds[1] << " "
         << bounds[2] << " " << bounds[3] << " "
         << bounds[4] << " " << bounds[5] << std::endl;
   }
   
   //
   // Shrinker - does this actually do anything ?
   //
   vtkImageShrink3D* shrinker = vtkImageShrink3D::New();
#ifdef HAVE_VTK6
   shrinker->SetInputData(sp);
#else
   shrinker->SetInput(sp);
#endif
   shrinker->SetShrinkFactors(1, 1, 1);
   shrinker->AveragingOn();

   //
   // Gaussian smooth the volume
   //   
   vtkImageGaussianSmooth* gaussian = vtkImageGaussianSmooth::New();
   gaussian->SetDimensionality(3);
   gaussian->SetStandardDeviation(0);
#ifdef HAVE_VTK6
   gaussian->SetInputConnection(shrinker->GetOutputPort());
#else
   gaussian->SetInput(shrinker->GetOutput());
#endif
   gaussian->Update();
   vtkImageData* gaussOut = gaussian->GetOutput();
   gaussOut->GetBounds(bounds);
   
   if (DebugControl::getDebugOn()) {
      std::cout << "Segmentation shrink/gauss volume bounds: "
         << bounds[0] << " " << bounds[1] << " "
         << bounds[2] << " " << bounds[3] << " "
         << bounds[4] << " " << bounds[5] << std::endl;
   }
   
   //
   // Marching cubes converts volume to a surface
   //
   vtkMarchingCubes* mc = vtkMarchingCubes::New();
#ifdef HAVE_VTK6
   mc->SetInputConnection(gaussian->GetOutputPort());
#else
   mc->SetInput(gaussian->GetOutput());
#endif
   //mc->SetValue(0, 511.5);
   mc->SetValue(0, 127.5);
   mc->ComputeScalarsOff();
   mc->ComputeGradientsOff();
   mc->ComputeNormalsOff();
   
   //
   // Clean up surface created by marching cubes
   //
   vtkCleanPolyData* clean = vtkCleanPolyData::New();
#ifdef HAVE_VTK6
   clean->SetInputConnection(mc->GetOutputPort());
#else
   clean->SetInput(mc->GetOutput());
#endif
   //
   // Make sure mesh is only triangles
   //
   vtkTriangleFilter *triangleFilter = vtkTriangleFilter::New();
#ifdef HAVE_VTK6
   triangleFilter->SetInputConnection(clean->GetOutputPort());
#else
   triangleFilter->SetInput(clean->GetOutput());
#endif
   if (DebugControl::getDebugOn()) {
      triangleFilter->Update();
      vtkPolyDataWriter* writer = vtkPolyDataWriter::New();
#ifdef HAVE_VTK6
      writer->SetInputConnection(triangleFilter->GetOutputPort());
#else
      writer->SetInput(triangleFilter->GetOutput());
#endif
      writer->SetFileName("surface_undecimated.vtk");
      writer->Write();
      writer->Delete();
   }
   
   triangleFilter->Update();
   vtkPolyData* boundsPolyData = triangleFilter->GetOutput();
   boundsPolyData->GetBounds(bounds);
   if (DebugControl::getDebugOn()) {
      std::cout << "Surface bounds: "
         << bounds[0] << " " << bounds[1] << " "
         << bounds[2] << " " << bounds[3] << " "
         << bounds[4] << " " << bounds[5] << std::endl;
   }
   
   //
   // See if the surface should be decimated
   //
   vtkDecimatePro* decimater = NULL;
   if (maxPolygonsFlag == false) {
      decimater = vtkDecimatePro::New();
      if (DebugControl::getDebugOn()) {
         decimater->DebugOn();
      }
      double errorVal = 0.001;
      //if (maxPolygonsFlag) {
      //   errorVal = 0.0;
      //}
#ifdef HAVE_VTK6
      decimater->SetInputConnection(triangleFilter->GetOutputPort());
#else
      decimater->SetInput(triangleFilter->GetOutput());
#endif
      decimater->SetTargetReduction(0.90);
      decimater->PreserveTopologyOn();
      decimater->SetFeatureAngle(30.0);  //45.0); //1);   // orig == 30
      decimater->SplittingOff();
      decimater->PreSplitMeshOff();
      decimater->SetMaximumError(errorVal);
      decimater->BoundaryVertexDeletionOff();
      decimater->SetDegree(25);
      decimater->AccumulateErrorOn();
      decimater->SetAbsoluteError(errorVal);
      decimater->SetErrorIsAbsolute(1);
      
      if (DebugControl::getDebugOn()) {
         decimater->PrintSelf(std::cout, static_cast<vtkIndent>(3));
         decimater->Update();
         decimater->PrintSelf(std::cout, static_cast<vtkIndent>(3));
         vtkPolyDataWriter* writer = vtkPolyDataWriter::New();
#ifdef HAVE_VTK6
         writer->SetInputConnection(decimater->GetOutputPort());
#else
         writer->SetInput(decimater->GetOutput());
#endif
         writer->SetFileName("surface_decimated.vtk");
         writer->Write();
         writer->Delete();
      }
   }
   
   //
   // Clean again
   //
   vtkCleanPolyData* clean2 = vtkCleanPolyData::New();
   if (decimater != NULL) {
#ifdef HAVE_VTK6
	      clean2->SetInputConnection(decimater->GetOutputPort());
#else
	      clean2->SetInput(decimater->GetOutput());
#endif
   }
   else {
#ifdef HAVE_VTK6
      clean2->SetInputConnection(triangleFilter->GetOutputPort());
#else
      clean2->SetInput(triangleFilter->GetOutput());
#endif
   }
   
   //
   // Since reconstruction successful clear out any existing surfaces
   //
   brainSet->deleteAllBrainModelSurfaces();

   //
   // Compute normals on the surface
   //
   vtkPolyDataNormals* rawNormals = vtkPolyDataNormals::New();
#ifdef HAVE_VTK6
   rawNormals->SetInputConnection(clean2->GetOutputPort());
#else
   rawNormals->SetInput(clean2->GetOutput());
#endif
   rawNormals->SplittingOff();
   rawNormals->ConsistencyOn();
   rawNormals->ComputePointNormalsOn();
   rawNormals->NonManifoldTraversalOn();
   
   //
   // Force vtk to execute
   //
   rawNormals->Update();
   
   //
   // Get a pointer to the poly data
   //
   vtkPolyData* rawPolyDataSurface = rawNormals->GetOutput();
  
   if (DebugControl::getDebugOn()) {
      vtkPolyDataWriter* writer = vtkPolyDataWriter::New();
#ifdef HAVE_VTK6
      writer->SetInputData(rawPolyDataSurface);
#else
      writer->SetInput(rawPolyDataSurface);
#endif
      writer->SetFileName("raw.vtk");
      writer->Write();
      writer->Delete();
   }
    
   //
   // Convert to vtk file to a brain model surface
   //
   try {
      brainSet->importVtkTypeFileHelper("raw.vtk", rawPolyDataSurface,
                                        true, true, false,
                                        BrainModelSurface::SURFACE_TYPE_FIDUCIAL,
                                        TopologyFile::TOPOLOGY_TYPE_CLOSED);
   }
   catch (FileException& e) {
      throw BrainModelAlgorithmException(e.whatQString());
   }

   //
   // Use volume orientation for surface's orientation
   //
   VolumeFile::ORIENTATION orient[3];
   segmentationVolumeFile->getOrientation(orient);
   QString orientString;
   for (int io = 0; io < 3; io++) {
      switch (orient[io]) {
         case VolumeFile::ORIENTATION_UNKNOWN:
            orientString.append("U");
            break;
         case VolumeFile::ORIENTATION_RIGHT_TO_LEFT:
            orientString.append("R");
            break;
         case VolumeFile::ORIENTATION_LEFT_TO_RIGHT:
            orientString.append("L");
            break;
         case VolumeFile::ORIENTATION_POSTERIOR_TO_ANTERIOR:
            orientString.append("P");
            break;
         case VolumeFile::ORIENTATION_ANTERIOR_TO_POSTERIOR:
            orientString.append("A");
            break;
         case VolumeFile::ORIENTATION_INFERIOR_TO_SUPERIOR:
            orientString.append("I");
            break;
         case VolumeFile::ORIENTATION_SUPERIOR_TO_INFERIOR:
            orientString.append("S");
            break;
      }
   } 

   //
   // Newest brain model should be the surface
   //
   BrainModelSurface* rawBms = brainSet->getBrainModelSurface(brainSet->getNumberOfBrainModels() - 1);
   if (rawBms != NULL) {
      rawBms->orientNormalsOut();
      if (rightHemisphereFlag && leftHemisphereFlag) {
         rawBms->setStructure(Structure::STRUCTURE_TYPE_CEREBELLUM);
      }
      else if (rightHemisphereFlag) {
         rawBms->setStructure(Structure::STRUCTURE_TYPE_CORTEX_RIGHT);
      }
      else if (leftHemisphereFlag) {
         rawBms->setStructure(Structure::STRUCTURE_TYPE_CORTEX_LEFT);
      }
      rawBms->setSurfaceType(BrainModelSurface::SURFACE_TYPE_RAW);
      
      rawBms->appendToCoordinateFileComment("Generated from reconstruction of segmentation: ");
      rawBms->appendToCoordinateFileComment(segmentationVolumeFile->getFileName());
      rawBms->appendToCoordinateFileComment("\n");
      rawBms->appendToTopologyFileComment("Generated from reconstruction of segmentation: ");
      rawBms->appendToTopologyFileComment(segmentationVolumeFile->getFileName());
      rawBms->appendToTopologyFileComment("\n");
      
      TopologyFile* tf = rawBms->getTopologyFile();
      if (tf != NULL) {
         const int numIslands = tf->disconnectIslands();
         if (numIslands > 0) {
            rawBms->moveDisconnectedNodesToOrigin();
         }
         if (DebugControl::getDebugOn()) {
            std::cout << numIslands << " islands were in reconstructed surface." << std::endl;
         }
         tf->setTopologyType(TopologyFile::TOPOLOGY_TYPE_CLOSED);
      }
      
      //
      // Default surface orientation to that of the volume
      //
      CoordinateFile* cf = rawBms->getCoordinateFile();
      cf->setHeaderTag(AbstractFile::headerTagOrientation, orientString);     

      //
      // Set the stereotaxic space in the coord frame id
      //
      cf->setHeaderTag(AbstractFile::headerTagCoordFrameID, brainSet->getStereotaxicSpace().getName());      
   }
  
   //
   // Smooth the surface
   //
   vtkSmoothPolyDataFilter* smooth = vtkSmoothPolyDataFilter::New();
#ifdef HAVE_VTK6
   smooth->SetInputData(rawPolyDataSurface);
#else
   smooth->SetInput(rawPolyDataSurface);
#endif
   smooth->SetNumberOfIterations(10);
   smooth->SetRelaxationFactor(0.2);
   smooth->SetFeatureAngle(180.0);
   smooth->FeatureEdgeSmoothingOff();
   smooth->BoundarySmoothingOff();
   smooth->SetConvergence(0);
   
   //
   // Compute normals on the surface
   //
   vtkPolyDataNormals* normals = vtkPolyDataNormals::New();
#ifdef HAVE_VTK6
   normals->SetInputConnection(smooth->GetOutputPort());
#else
   normals->SetInput(smooth->GetOutput());
#endif
   normals->SplittingOff();
   normals->ConsistencyOn();
   normals->ComputePointNormalsOn();
   normals->NonManifoldTraversalOn();
   
   //
   // Force vtk to execute
   //
   normals->Update();
   
   //
   // Get a pointer to the poly data
   //
   vtkPolyData* polyDataSurface = normals->GetOutput();
   
   //
   // Test to clip a surface DO NOT USE THIS AS IT CHANGES TOPOLOGY
   //
   bool testFlag = false;
   if (testFlag) {
      vtkPlane* plane = vtkPlane::New();
      plane->SetOrigin(37.0, -42.0, 59.0);
      plane->SetNormal(0.0, -1.0, 0.0);
      vtkClipPolyData* clipper = vtkClipPolyData::New();
      clipper->SetClipFunction(plane);
#ifdef HAVE_VTK6
      clipper->SetInputData(polyDataSurface);
#else
      clipper->SetInput(polyDataSurface);
#endif
      clipper->Update();
      vtkPolyDataWriter* writer = vtkPolyDataWriter::New();
#ifdef HAVE_VTK6
      writer->SetInputConnection(clipper->GetOutputPort());
#else
      writer->SetInput(clipper->GetOutput());
#endif
      writer->SetFileName("surface_cut.vtk");
      writer->Write();
      writer->Delete();
      clipper->Delete();
      plane->Delete();
   }
   
   //
   // Convert to vtk file to a brain model surface
   //
   try {
      brainSet->importVtkTypeFileHelper("fiducial.vtk", polyDataSurface,
                                        true, false, false,
                                        BrainModelSurface::SURFACE_TYPE_FIDUCIAL,
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
      if (rightHemisphereFlag && leftHemisphereFlag) {
         bms->setStructure(Structure::STRUCTURE_TYPE_CEREBELLUM);
      }
      else if (rightHemisphereFlag) {
         bms->setStructure(Structure::STRUCTURE_TYPE_CORTEX_RIGHT);
      }
      else if (leftHemisphereFlag) {
         bms->setStructure(Structure::STRUCTURE_TYPE_CORTEX_LEFT);
      }
      bms->setSurfaceType(BrainModelSurface::SURFACE_TYPE_FIDUCIAL);
      
      bms->appendToCoordinateFileComment("Generated from reconstruction of segmentation: ");
      bms->appendToCoordinateFileComment(segmentationVolumeFile->getFileName());
      bms->appendToCoordinateFileComment("\n");

      //
      // Default surface orientation to that of the volume
      //
      CoordinateFile* cf = bms->getCoordinateFile();
      cf->setHeaderTag(AbstractFile::headerTagOrientation, orientString);     
      
      //
      // Set the stereotaxic space in the coord frame id
      //
      cf->setHeaderTag(AbstractFile::headerTagCoordFrameID, brainSet->getStereotaxicSpace().getName());
      
      //
      // Does user want a hypersmoothed surface created
      //
      if (createHypersmoothSurfaceFlag) {
         bms->createInflatedAndEllipsoidFromFiducial(true,
                                                     false,
                                                     false,
                                                     false,
                                                     false,
                                                     false,
                                                     true,
                                                     1.0,
                                                     NULL);
/*
         //
         // Create the hypersmoothed surface
         //
         BrainModelSurface* hypersmoothSurface = new BrainModelSurface(*bms);
         hypersmoothSurface->arealSmoothing(1.0, 50, 0);
         hypersmoothSurface->setSurfaceType(BrainModelSurface::SURFACE_TYPE_INFLATED);
         brainSet->addBrainModel(hypersmoothSurface);
*/
      }
   }
  
   brainSet->postSpecFileReadInitializations();
   
   //
   // Assign colors to the surface
   //
   //brainSet->getNodeColoring()->assignColors();
  
   rawNormals->Delete();
   normals->Delete();
   smooth->Delete();
   if (decimater != NULL) {
      decimater->Delete();
   }
   triangleFilter->Delete();
   clean->Delete();
   mc->Delete();
   
   sp->Delete();
}

/**
 * generate a vtk model.
 */
void 
BrainModelVolumeToSurfaceConverter::generateVtkModel(const bool maxPolygonsFlag) throw (BrainModelAlgorithmException)
{
   //
   // Convert to structured points
   //
   vtkStructuredPoints* sp = segmentationVolumeFile->convertToVtkStructuredPoints();
   
   //
   // Shrinker - does this actually do anything ?
   //
   vtkImageShrink3D* shrinker = vtkImageShrink3D::New();
#ifdef HAVE_VTK6
   shrinker->SetInputData(sp);
#else
   shrinker->SetInput(sp);
#endif
   shrinker->SetShrinkFactors(1, 1, 1);
   shrinker->AveragingOn();

   //
   // Gaussian smooth the volume
   //   
   vtkImageGaussianSmooth* gaussian = vtkImageGaussianSmooth::New();
   gaussian->SetDimensionality(3);
   gaussian->SetStandardDeviation(0);
#ifdef HAVE_VTK6
   gaussian->SetInputConnection(shrinker->GetOutputPort());
#else
   gaussian->SetInput(shrinker->GetOutput());
#endif
   
   //
   // Marching cubes converts volume to a surface
   //
   vtkMarchingCubes* mc = vtkMarchingCubes::New();
#ifdef HAVE_VTK6
   mc->SetInputConnection(gaussian->GetOutputPort());
#else
   mc->SetInput(gaussian->GetOutput());
#endif
   //mc->SetValue(0, 511.5);
   mc->SetValue(0, 127.5);
   mc->ComputeScalarsOff();
   mc->ComputeGradientsOff();
   mc->ComputeNormalsOff();
   
   //
   // Clean up surface created by marching cubes
   //
   vtkCleanPolyData* clean = vtkCleanPolyData::New();
#ifdef HAVE_VTK6
   clean->SetInputConnection(mc->GetOutputPort());
#else
   clean->SetInput(mc->GetOutput());
#endif
   
   //
   // Make sure mesh is only triangles
   //
   vtkTriangleFilter *triangleFilter = vtkTriangleFilter::New();
#ifdef HAVE_VTK6
   triangleFilter->SetInputConnection(clean->GetOutputPort());
#else
   triangleFilter->SetInput(clean->GetOutput());
#endif

   //
   // See if the surface should be decimated
   //
   vtkDecimatePro* decimater = NULL;
   if (maxPolygonsFlag == false) {
      decimater = vtkDecimatePro::New();
      if (DebugControl::getDebugOn()) {
         decimater->DebugOn();
      }
      const double errorVal = 0.001;
#ifdef HAVE_VTK6
      decimater->SetInputConnection(triangleFilter->GetOutputPort());
#else
      decimater->SetInput(triangleFilter->GetOutput());
#endif
      decimater->SetTargetReduction(0.90);
      decimater->PreserveTopologyOn();
      decimater->SetFeatureAngle(30);
      decimater->SplittingOff();
      decimater->PreSplitMeshOff();
      decimater->SetMaximumError(errorVal);
      decimater->BoundaryVertexDeletionOff();
      decimater->SetDegree(25);
      decimater->AccumulateErrorOn();
      decimater->SetAbsoluteError(errorVal);
      decimater->SetErrorIsAbsolute(1);
   }
   
   //
   // Clean again
   //
   vtkCleanPolyData* clean2 = vtkCleanPolyData::New();
   if (decimater != NULL) {
#ifdef HAVE_VTK6
	      clean2->SetInputConnection(decimater->GetOutputPort());
#else
	      clean2->SetInput(decimater->GetOutput());
#endif
   }
   else {
#ifdef HAVE_VTK6
	      clean2->SetInputConnection(triangleFilter->GetOutputPort());
#else
	      clean2->SetInput(triangleFilter->GetOutput());
#endif
   }
   
   //
   // Smooth the surface
   //
   vtkSmoothPolyDataFilter* smooth = vtkSmoothPolyDataFilter::New();
#ifdef HAVE_VTK6
   smooth->SetInputConnection(clean2->GetOutputPort());
#else
   smooth->SetInput(clean2->GetOutput());
#endif
   smooth->SetNumberOfIterations(10);
   smooth->SetRelaxationFactor(0.2);
   smooth->SetFeatureAngle(180.0);
   smooth->FeatureEdgeSmoothingOff();
   smooth->BoundarySmoothingOff();
   smooth->SetConvergence(0);
   
   //
   // Compute normals on the surface
   //
   vtkPolyDataNormals* rawNormals = vtkPolyDataNormals::New();
#ifdef HAVE_VTK6
   rawNormals->SetInputConnection(smooth->GetOutputPort());
#else
   rawNormals->SetInput(smooth->GetOutput());
#endif
   rawNormals->SplittingOff();
   rawNormals->ConsistencyOn();
   rawNormals->ComputePointNormalsOn();
   rawNormals->NonManifoldTraversalOn();
   
   //
   // Force vtk to execute
   //
   rawNormals->Update();
   
   //
   // Get a pointer to the poly data
   //
   vtkPolyData* rawPolyDataSurface = rawNormals->GetOutput();
   
   //
   // Add vtk model file to brain set
   //
   VtkModelFile* vtkModelFile = new VtkModelFile(rawPolyDataSurface);
   brainSet->addVtkModelFile(vtkModelFile);
   vtkModelFile->setModified();
   
   rawNormals->Delete();
   smooth->Delete();
   clean2->Delete();
   if (decimater != NULL) {
      decimater->Delete();
   }
   triangleFilter->Delete();
   clean->Delete();
   mc->Delete();
   
   sp->Delete();
}      
