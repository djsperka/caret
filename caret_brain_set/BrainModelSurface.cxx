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



#include <QGlobalStatic>  // needed for Q_OS_WIN32
#ifdef Q_OS_WIN32     // required for M_PI in <cmath>
#define _USE_MATH_DEFINES
#define NOMINMAX
#endif

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

#include <QApplication>
#include <QProgressDialog>
#include <QString>

#include "vtkCellArray.h"
#include "vtkDecimatePro.h"
#include "vtkDelaunay3D.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkPolyDataNormals.h"
#include "vtkTransform.h"
#include "vtkTriangle.h"
#include "vtkTriangleFilter.h"
#include "vtkUnstructuredGrid.h"

#include "BorderFile.h"
#include "BorderProjectionFile.h"
#include "BrainModelSurface.h"
#include "BrainModelSurfaceCurvature.h"
#include "BrainModelSurfaceROINodeSelection.h"
#include "BrainModelSurfacePointLocator.h"
#include "BrainModelSurfacePointProjector.h"
#include "BrainModelSurfaceSmoothing.h"
#include "BrainModelSurfaceToVolumeSegmentationConverter.h"
#include "BrainSet.h"
#include "DebugControl.h"
#include "DeformationFieldFile.h"
#include "FileUtilities.h"
#include "LatLonFile.h"
#include "MathUtilities.h"
#include "MniObjSurfaceFile.h"
#include "PaintFile.h"
#include "RgbPaintFile.h"
#include "StringUtilities.h"
#include "SurfaceFile.h"
#include "SurfaceShapeFile.h"
#include "SystemUtilities.h"
#include "TopologyFile.h"
#include "TopologyHelper.h"
#include "TransformationMatrixFile.h"
#include "VectorFile.h"

/**
 * The constructor.
 */
BrainModelSurface::BrainModelSurface(BrainSet* bs, 
                                     const BrainModel::BRAIN_MODEL_TYPE bmt) 
   : BrainModel(bs, bmt)
{
   reset();
}

/**
 * The copy constructor
 */
BrainModelSurface::BrainModelSurface(const BrainModelSurface& bms)
   : BrainModel(bms)
{
   reset();

   coordinates = bms.coordinates;
   coordinates.setFileName("");
   coordinates.setModified();

   topology    = bms.topology;
   normals     = bms.normals;
   surfaceType = bms.surfaceType;
   setStructure(bms.getStructure());
   defaultScaling = bms.defaultScaling;
   defaultPerspectiveZooming = bms.defaultPerspectiveZooming;
   pushPopCoordinates = bms.pushPopCoordinates;
   displayHalfX = bms.displayHalfX;
   displayHalfY = bms.displayHalfY;   
}

/**
 * The destructor.
 */
BrainModelSurface::~BrainModelSurface()
{
   reset();
}

/**
 * get information about the surface.
 */
void 
BrainModelSurface::getSurfaceInformation(std::vector<QString>& labels,
                                         std::vector<QString>& values) const
{
   labels.clear();
   values.clear();
   
   labels.push_back("Coordinate File");
   values.push_back(FileUtilities::basename(coordinates.getFileName()));
   
   labels.push_back("Topology File");
   if (topology != NULL) {
      values.push_back(FileUtilities::basename(topology->getFileName()));
   }
   else {
      values.push_back("");
   }
   
   labels.push_back("Topology Type");
   if (topology != NULL) {
      values.push_back(topology->getTopologyTypeName());
   }
   else {
      values.push_back("");
   }
   
   labels.push_back("Surface Type");
   values.push_back(getSurfaceTypeName());
   
   labels.push_back("Structure");
   values.push_back(getStructure().getTypeAsString());
   
   labels.push_back("Number of Nodes");
   values.push_back(QString::number(getNumberOfNodes()));
   
   labels.push_back("Number of Tiles");
   if (topology != NULL) {
      values.push_back(QString::number(topology->getNumberOfTiles()));
   }
   else {
      values.push_back("");
   }
   
   float bounds[6];
   getBounds(bounds);
   
   labels.push_back("X Extent");
   values.push_back(QString::number(bounds[0], 'f', 2) + "     " +
                    QString::number(bounds[1], 'f', 2));
   
   labels.push_back("Y Extent");
   values.push_back(QString::number(bounds[2], 'f', 2) + "     " +
                    QString::number(bounds[3], 'f', 2));
   
   labels.push_back("Z Extent");
   values.push_back(QString::number(bounds[4], 'f', 2) + "     " +
                    QString::number(bounds[5], 'f', 2));
   
   labels.push_back("Surface Area");
   values.push_back(QString::number(getSurfaceArea(), 'f', 2));
   
   float meanDist, minDist, maxDist;
   getMeanDistanceBetweenNodes(NULL, meanDist, minDist, maxDist);
   labels.push_back("Mean/Min/Max Distance");
   values.push_back(QString::number(meanDist, 'f', 6)
                    + "/"
                    + QString::number(minDist, 'f', 6)
                    + "/"
                    + QString::number(maxDist, 'f', 6));
   
   labels.push_back("Spherical Radius");
   if (getSurfaceType() == BrainModelSurface::SURFACE_TYPE_SPHERICAL) {
      values.push_back(QString::number(getSphericalSurfaceRadius(), 'f', 2));
   }
   else {
      values.push_back("N/A");
   }
}
                                 
/**
 * Get a descriptive name of the model.
 */
QString
BrainModelSurface::getDescriptiveName() const
{
   QString name(StringUtilities::makeUpperCase(getSurfaceTypeName()));
   name.append(" ");
   const CoordinateFile* cf = getCoordinateFile();
   name.append(FileUtilities::basename(cf->getFileName()));
   return name;
}

/**
 * append a string to the coordinate file's comment.
 */
void 
BrainModelSurface::appendToCoordinateFileComment(const QString& s)
{
   coordinates.appendToFileComment(s);
}

/**
 * append a string to the topology file's comment.
 */
void 
BrainModelSurface::appendToTopologyFileComment(const QString& s)
{
   if (topology != NULL) {
      topology->appendToFileComment(s);
   }
}

/**
 * Add a node to this brain model surface.
 */
void
BrainModelSurface::addNode(const float xyz[3])
{
   coordinates.addCoordinate(xyz);
   normals.push_back(0.0);
   normals.push_back(0.0);
   normals.push_back(1.0);
   if (topology != NULL) {
      topology->setNumberOfNodes(coordinates.getNumberOfCoordinates());
   }
}

/**
 * surefit ellipsoid crossover check.
 */
int 
BrainModelSurface::crossoverCheckSureFitEllipsoid()
{
   const int numNodes = getNumberOfNodes();
   if (numNodes <= 0) {
      return 0;
   }
 
   float totalCrossovers = 0.0;
   
   //
   // Pointer to node attributes
   //
   BrainSetNodeAttribute* nodeAttributes = brainSet->getNodeAttributes(0);

   //
   // Default to no crssovers
   //
   for (int i = 0; i < numNodes; i++) {
      nodeAttributes[i].setCrossover(BrainSetNodeAttribute::CROSSOVER_NO);
   }
   
   //
   // Create a topology helper with node neighbors sorted
   //
   const TopologyHelper* th = topology->getTopologyHelper(false, true, true);
   
   //
   // Get this surface's coordinate file
   //
   CoordinateFile* cf = getCoordinateFile();
   
   //
   // Get the center of mass for this surface
   //
   float centerOfMass[3];
   getCenterOfMass(centerOfMass);
   
   //
   // Examine each node
   //
   for (int i = 0; i < numNodes; i++) {
      
      int numCrossoversThisNode = 0;
      
      //
      // Get the nodes coordinate
      //
      const float* nodeXYZ = cf->getCoordinate(i);
      
      //
      // Determine the ellipsoid normal for this node
      //
      float pos[3];
      pos[0] = nodeXYZ[0] - centerOfMass[0];
      pos[1] = nodeXYZ[1] - centerOfMass[1];
      pos[2] = nodeXYZ[2] - centerOfMass[2];
         
      // HAD 7.21.97 Want normals to point *out* on ellipsoid
      float ellipsoidNormal[3];
      if (pos[0] < 0) {
         ellipsoidNormal[0] = -1.0;
      }
      else {
         ellipsoidNormal[0] = 1.0;
      }
      if (pos[1] < 0) {
         ellipsoidNormal[1] = -1.0;
      }
      else {
         ellipsoidNormal[1] = 1.0;
      }
      if (pos[2] < 0) {
         ellipsoidNormal[2] = -1.0;
      }
      else {
         ellipsoidNormal[2] = 1.0;
      }
      MathUtilities::normalize(ellipsoidNormal);
      
      //
      // Get this nodes neighbors
      //
      int numNeighbors = 0;
      const int* neighbors = th->getNodeNeighbors(i, numNeighbors);
      
      //
      // Check tiles formed by neighbors
      //
      if (numNeighbors >= 2) {
         for (int j = 0; j < numNeighbors; j++) {
            //
            // Get node numbers of two neighbors
            //
            const int n1 = neighbors[j];
            int nextNeighIndex = j + 1;
            if (nextNeighIndex >= numNeighbors) {
               nextNeighIndex = 0;
            }
            const int n2 = neighbors[nextNeighIndex];
            
            //
            // Calculate normal of tile formed by neighboring nodes
            //
            const float* n1XYZ = cf->getCoordinate(n1);
            const float* n2XYZ = cf->getCoordinate(n2);
            float tileNormal[3];
            MathUtilities::computeNormal((float*)nodeXYZ, (float*)n1XYZ, (float*)n2XYZ, tileNormal);
            
            //
            // Determine angle between node normal and tile normal
            //
            const float dot = MathUtilities::dotProduct(ellipsoidNormal, tileNormal);
            
            //
            // Is this a crossover ?
            //
            if (dot < 0.0) {
               nodeAttributes[i].setCrossover(BrainSetNodeAttribute::CROSSOVER_YES);
               numCrossoversThisNode += 2;
            }
         }
      }
      
      totalCrossovers += (static_cast<float>(numCrossoversThisNode)
                          / static_cast<float>(numNeighbors));
   }  // for 
   
   return static_cast<int>(totalCrossovers);
}      

/**
 * Peform a crossover check.
 */
void
BrainModelSurface::crossoverCheck(int& numberOfTileCrossovers,
                                  int& numberOfNodeCrossovers,
                                  const SURFACE_TYPES surfaceTypeHint)
{
   numberOfTileCrossovers = 0;
   numberOfNodeCrossovers = 0;
   
   const int numNodes = getNumberOfNodes();
   if (numNodes <= 0) {
      return;
   }
   
   //
   // Pointer to node attributes
   //
   BrainSetNodeAttribute* nodeAttributes = brainSet->getNodeAttributes(0);

   //
   // Default to no crssovers
   //
   for (int i = 0; i < numNodes; i++) {
      nodeAttributes[i].setCrossover(BrainSetNodeAttribute::CROSSOVER_NO);
   }
   
   enum METHOD_TYPE { METHOD_FLAT, METHOD_SPHERE, METHOD_OTHER };
   METHOD_TYPE methodType = METHOD_OTHER;
   switch (surfaceTypeHint) {
      case SURFACE_TYPE_RAW:
         methodType = METHOD_OTHER;
         break;
      case SURFACE_TYPE_FIDUCIAL:
         methodType = METHOD_OTHER;
         break;
      case SURFACE_TYPE_INFLATED:
         methodType = METHOD_OTHER;
         break;
      case SURFACE_TYPE_VERY_INFLATED:
         methodType = METHOD_OTHER;
         break;
      case SURFACE_TYPE_SPHERICAL:
         methodType = METHOD_SPHERE;
         break;
      case SURFACE_TYPE_ELLIPSOIDAL:
         methodType = METHOD_OTHER;
         break;
      case SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         methodType = METHOD_OTHER;
         break;
      case SURFACE_TYPE_FLAT:
         methodType = METHOD_FLAT;
         break;
      case SURFACE_TYPE_FLAT_LOBAR:
         methodType = METHOD_FLAT;
         break;
      case SURFACE_TYPE_HULL:
         methodType = METHOD_OTHER;
         break;
      case SURFACE_TYPE_UNKNOWN:
         methodType = METHOD_OTHER;
         break;
      case SURFACE_TYPE_UNSPECIFIED:
         methodType = METHOD_OTHER;
         break;
   }
   
   //
   // Pointer to coordinates
   //
   const float* coords = coordinates.getCoordinate(0);
   
   switch (methodType) {
      case METHOD_FLAT:
         {
            const float cosine30Degrees = 0.866;

            const int numTiles = topology->getNumberOfTiles();
            for (int i = 0; i < numTiles; i++) {
               //
               // Get the nodes of the tile
               //
               int n1, n2, n3;
               topology->getTile(i, n1, n2, n3);
               
               //
               // Determine the tile's normal
               //
               float tileNormal[3];
               MathUtilities::computeNormal((float*)&coords[n1*3],
                                          (float*)&coords[n2*3],
                                          (float*)&coords[n3*3],
                                          tileNormal);
               
               //
               // Normal should be pointing straight up
               //
               if (tileNormal[2] < cosine30Degrees) {
                  nodeAttributes[n1].setCrossover(BrainSetNodeAttribute::CROSSOVER_YES);
                  nodeAttributes[n2].setCrossover(BrainSetNodeAttribute::CROSSOVER_YES);
                  nodeAttributes[n3].setCrossover(BrainSetNodeAttribute::CROSSOVER_YES);
                  numberOfTileCrossovers++;
               }
            }
         }
         break;
      case METHOD_SPHERE:
         {
            const float cosine30Degrees = 0.866;
            
            const int numTiles = topology->getNumberOfTiles();
            for (int i = 0; i < numTiles; i++) {
               //
               // Get the nodes of the tile
               //
               int n1, n2, n3;
               topology->getTile(i, n1, n2, n3);
               
               //
               // Determine the tile's normal
               //
               float tileNormal[3];
               MathUtilities::computeNormal((float*)&coords[n1*3],
                                          (float*)&coords[n2*3],
                                          (float*)&coords[n3*3],
                                          tileNormal);
               
               //
               // Determine the sphere normal (ray from origin thru center of tile)
               //
               float sphereNormal[3] = { (coords[n1*3]   + coords[n2*3]   + coords[n3*3])   / 3.0,
                                         (coords[n1*3+1] + coords[n2*3+1] + coords[n3*3+1]) / 3.0,
                                         (coords[n1*3+2] + coords[n2*3+2] + coords[n3*3+2]) / 3.0 };
               MathUtilities::normalize(sphereNormal);
               
               //
               // Angle between sphere normal and tile normal
               //
               const float dot = MathUtilities::dotProduct(sphereNormal, tileNormal);

               //
               // Normal should be pointing out of the sphere (within 15 degrees)
               //
               if (dot < cosine30Degrees) {
                  nodeAttributes[n1].setCrossover(BrainSetNodeAttribute::CROSSOVER_YES);
                  nodeAttributes[n2].setCrossover(BrainSetNodeAttribute::CROSSOVER_YES);
                  nodeAttributes[n3].setCrossover(BrainSetNodeAttribute::CROSSOVER_YES);
                  numberOfTileCrossovers++;
               }
            }
         }
         break;
      case METHOD_OTHER:
         {
            //
            // distance between two coincident vertices
            //
            const float tooSmall = 0.00001;
   
            //
            // Determine crossovers by examining angles between tiles
            //
            const TopologyHelper* th = topology->getTopologyHelper(true, false, false);
            const std::set<TopologyEdgeInfo>& edges = th->getEdgeInfo();   
            for (std::set<TopologyEdgeInfo>::const_iterator iter = edges.begin();
               iter != edges.end(); iter++) {
               
               BrainSetNodeAttribute::CROSSOVER_STATUS crossoverStatus = 
                  BrainSetNodeAttribute::CROSSOVER_NO;
                  
               //
               // Get nodes used by the edge
               //
               int node1, node2;
               iter->getNodes(node1, node2);
               
               //
               // if edge is very, very, very short then declare it a crossover
               //
               const float mag = MathUtilities::distance3D(&coords[node1*3],
                                                           &coords[node2*3]);
               if (iter->getEdgeUsedByMoreThanTwoTriangles()) {
                  if (DebugControl::getDebugOn()) {
                     std::cout << "Crossover Edge: " << node1 << " " << node2 
                              << " is used by more than two triangles" << std::endl;
                  }
                  crossoverStatus = BrainSetNodeAttribute::CROSSOVER_DEGENERATE_EDGE;
               }
               else if (mag < tooSmall) {
                  if (DebugControl::getDebugOn()) {
                     std::cout << "Crossover Edge: " << node1 << " " << node2 
                              << "are essentially the same " << mag
                              << " units apart" << std::endl;
                  }
                  crossoverStatus = BrainSetNodeAttribute::CROSSOVER_DEGENERATE_EDGE;
               }
               else {
                  int tile1, tile2;
                  iter->getTiles(tile1, tile2);
                  if ((tile1 > 0) && (tile2 > 0)) {
                     
                     //
                     // Get the nodes used by the two tiles
                     //
                     const int* t1n = topology->getTile(tile1);
                     const int* t2n = topology->getTile(tile2);
                     
                     //
                     // Calculate the normals of the two tiles
                     //
                     float tile1Normal[3];
                     float tile2Normal[3];
                     MathUtilities::computeNormal((float*)&coords[t1n[0]*3],
                                                (float*)&coords[t1n[1]*3],
                                                (float*)&coords[t1n[2]*3],
                                                tile1Normal);
                     MathUtilities::computeNormal((float*)&coords[t2n[0]*3],
                                                (float*)&coords[t2n[1]*3],
                                                (float*)&coords[t2n[2]*3],
                                                tile2Normal);
                        
                     //
                     // Dot product between tile normals gives arccos angle between tiles
                     //
                     const float invCosAngle = MathUtilities::dotProduct(tile1Normal, tile2Normal);
                     
                     if (iter->getEdgeOrientation(t1n) ==
                         iter->getEdgeOrientation(t2n)) {
                        if (DebugControl::getDebugOn()) {
                           std::cout << "Crossover Edge: " << node1 << " " << node2 
                                    << " is not oriented correctly for tiles "
                                    << tile1 << " " << tile2 << std::endl;
                        }
                        crossoverStatus = BrainSetNodeAttribute::CROSSOVER_DEGENERATE_EDGE;
                     }      
                     // if angle between tiles is 179 to 181 degrees assume a crossover
                     // std::cos(179) == std::cos(181) == -0.9998477
                     else if (invCosAngle < -0.9998477) {
                        const double angle = std::acos(invCosAngle) * (180.0 / M_PI);
                        if (DebugControl::getDebugOn()) {
                           std::cout << "Crossover Edge: " << node1 << " " << node2 
                                    << " angle " << angle << " inverse " << invCosAngle << std::endl;
                        }
                        crossoverStatus = BrainSetNodeAttribute::CROSSOVER_YES;
                     }   
                  }
               }
               
               if (crossoverStatus != BrainSetNodeAttribute::CROSSOVER_NO) {
                  nodeAttributes[node1].setCrossover(crossoverStatus);
                  nodeAttributes[node2].setCrossover(crossoverStatus);
                  numberOfTileCrossovers++;
               }
            }
         }
         break;
   }
   
   //
   // Count node crossovers
   //
   numberOfNodeCrossovers = 0;
   for (int i = 0; i < numNodes; i++) {
      if (nodeAttributes[i].getCrossover() == BrainSetNodeAttribute::CROSSOVER_YES) {
         numberOfNodeCrossovers++;
      }
   }
   
   brainSet->clearAllDisplayLists();
}

/**
 * read the specified surface file.
 */
void 
BrainModelSurface::readSurfaceFile(const QString& fileName) throw(FileException)
{
   SurfaceFile sf;
   try {
      //
      // Read the surface file
      //
      sf.readFile(fileName);

      //
      // Name of file without an extension
      //
      const QString fileNameNoExt = FileUtilities::filenameWithoutExtension(fileName);

      //
      // copy coordinates to coordinate file and process normals
      // allocate normals once number of coordinates is set
      //
      const int numCoords = sf.getNumberOfCoordinates();
      coordinates.setNumberOfCoordinates(numCoords);
      initializeNormals(numCoords);
      for (int i = 0; i < numCoords; i++) {
         coordinates.setCoordinate(i, sf.getCoordinate(i));
      }
      
      //
      // Set name of coordinate file
      //
      if (fileNameNoExt.isEmpty() == false) {
         QString coordFileName(fileNameNoExt);
         coordFileName.append(SpecFile::getCoordinateFileExtension());
         coordinates.setFileName(coordFileName);
      }
      
      //
      // Transfer coordinate metaData
      //
      const GiftiMetaData* coordMetaData = sf.getCoordinateMetaData();
      if (coordMetaData != NULL) {
         coordMetaData->copyMetaDataToCaretFile(&coordinates);
         //const GiftiMetaData::MetaDataContainer* data = coordMetaData->getMetaData();
         //for (GiftiMetaData::ConstMetaDataIterator iter = data->begin(); iter != data->end(); iter++) {
         //   coordinates.setHeaderTag(iter->first, iter->second);
         //}   
      }
      //
      // Get the type of coordinates
      //
      //coordinates.setHeaderTag(AbstractFile::headerTagConfigurationID, sf.getCoordinateType());
      setSurfaceType(getSurfaceTypeFromConfigurationID(sf.getCoordinateType()));
      
      //
      // Declare coord file not modified
      //
      coordinates.clearModified();
      
      //
      // Get triangles
      //
      const int numTriangles = sf.getNumberOfTriangles();
      if (numTriangles > 0) {
         TopologyFile* topoFile = new TopologyFile;
         topoFile->setNumberOfTiles(numTriangles);
         for (int i = 0; i < numTriangles; i++) {
            topoFile->setTile(i, sf.getTriangle(i));
         }

         //
         // Set name of topo file
         //
         if (fileNameNoExt.isEmpty() == false) {
            QString topoFileName(fileNameNoExt);
            topoFileName.append(SpecFile::getTopoFileExtension());
            topoFile->setFileName(topoFileName);
         }
         
         //
         // Transfer topology metaData
         //
         const GiftiMetaData* topoMetaData = sf.getTopologyMetaData();
         if (topoMetaData != NULL) {
            topoMetaData->copyMetaDataToCaretFile(topoFile);
            //const GiftiMetaData::MetaDataContainer* data = topoMetaData->getMetaData();
            //for (GiftiMetaData::ConstMetaDataIterator iter = data->begin(); iter != data->end(); iter++) {
            //   topoFile->setHeaderTag(iter->first, iter->second);
            //}   
         }

         //
         // Set the type of topology
         //
         topoFile->setTopologyType(TopologyFile::getTopologyTypeFromPerimeterID(sf.getTopologyType()));
         
         //
         // Declare topo file not modified
         //
         topoFile->clearModified();
         
         //
         // Add topology to brain set parent
         //
         if (brainSet != NULL) {
            //
            // Do not load topology if it duplicates already loaded topology
            //
            bool topologyIsDuplicate = false;
            for (int i = 0; i < brainSet->getNumberOfTopologyFiles(); i++) {
               TopologyFile* tf = brainSet->getTopologyFile(i);
               //
               // Are the topology files equivalent (exact same tiles)
               //
               if (tf->equivalent(*topoFile)) {
                  //
                  //  Do not need newly loaded topology file
                  //
                  delete topoFile;
                  
                  //
                  // Use the duplicate topology already loaded
                  //
                  topoFile = tf;
                  topologyIsDuplicate = true;
                  break;
               }
            }
            
            //
            // If the topology does not duplicate an already loaded topology, add to brain set
            //
            if (topologyIsDuplicate == false) {
               brainSet->addTopologyFile(topoFile);
            }
         }
         
         //
         // Use the topology file
         //
         topology = topoFile;
      }
   }
   catch (FileException& e) {
      reset();
      throw e;
   }
}      

/**
 * Write the file's memory in caret6 format to the specified name.
 */
QString
BrainModelSurface::writeSurfaceInCaret6Format(const QString& filenameIn,
                                             const QString& prependToFileNameExtension,
                                            Structure structure,
                                            const bool useCaret6ExtensionFlag) throw (FileException)
{
   this->setStructure(structure);

   //
   // Determine number of coordinates and triangles
   //
   coordinates.updateMetaDataForCaret6();
   const int numCoords = coordinates.getNumberOfCoordinates();
   int numTriangles = 0;
   if (topology != NULL) {
      topology->updateMetaDataForCaret6();
      numTriangles = topology->getNumberOfTiles();
   }

   //
   // Create the surface file
   //
   SurfaceFile sf(numCoords, numTriangles);

   //
   // Copy the coordinates and normals
   //
   for (int i = 0; i < numCoords; i++) {
      sf.setCoordinate(i, coordinates.getCoordinate(i));
   }

   //
   // Copy the triangles
   //
   for (int i = 0; i < numTriangles; i++) {
      sf.setTriangle(i, topology->getTile(i));
   }

   //
   // set the metadata
   //
   GiftiMetaData* coordMetaData = sf.getCoordinateMetaData();
   if (coordMetaData != NULL) {
      coordMetaData->copyMetaDataFromCaretFile(&coordinates);
   }
   GiftiMetaData* topoMetaData = sf.getTopologyMetaData();
   if (topoMetaData != NULL) {
      topoMetaData->copyMetaDataFromCaretFile(topology);
   }

   sf.removeHeaderTag("date");
   sf.removeHeaderTag("encoding");
   sf.setHeaderTag("Date", QDateTime::currentDateTime().toString(Qt::ISODate));
   sf.setHeaderTag("UserName", SystemUtilities::getUserName());

   //
   // Set the coordinate type and topology type
   //
   sf.setCoordinateType(getSurfaceTypeName());
   if (topology != NULL) {
      sf.setTopologyType(topology->getTopologyTypeName());
   }

   QString fileName = FileUtilities::basename(filenameIn);
   if (fileName.endsWith(".coord")) {
      fileName = FileUtilities::replaceExtension(fileName, ".coord",
                                   prependToFileNameExtension + SpecFile::getGiftiSurfaceFileExtension());
   }
   else if (fileName.endsWith(".surf.gii")) {
      fileName = FileUtilities::replaceExtension(fileName, ".surf.gii",
                                   prependToFileNameExtension + SpecFile::getGiftiSurfaceFileExtension());
   }
   else if (fileName.endsWith(".coord.gii")) {
      fileName = FileUtilities::replaceExtension(fileName, ".coord.gii",
                                   prependToFileNameExtension + SpecFile::getGiftiSurfaceFileExtension());
   }
   else {
      fileName = fileName + prependToFileNameExtension + SpecFile::getGiftiSurfaceFileExtension();
   }

   //
   // Write the file
   //
   try {
      sf.setFileWriteType(AbstractFile::FILE_FORMAT_XML_GZIP_BASE64);
      sf.writeFile(fileName);
   }
   catch (FileException& e) {
      throw e;
   }

   //
   // clear modified status for coordinates and topology
   //
   coordinates.clearModified();

   return fileName;
}

/**
 * Write the file's memory in caret7 format to the specified name.
 */
QString
BrainModelSurface::writeSurfaceInCaret7Format(const QString& filenameIn,
                                              const QString& prependToFileNameExtension,
                                              Structure structure,
                                              const bool useCaret7ExtensionFlag) throw (FileException)
{
    this->setStructure(structure);
    
    //
    // Determine number of coordinates and triangles
    //
    coordinates.updateMetaDataForCaret7();
    const int numCoords = coordinates.getNumberOfCoordinates();
    int numTriangles = 0;
    if (topology != NULL) {
        topology->updateMetaDataForCaret7();
        numTriangles = topology->getNumberOfTiles();
    }
    
    //
    // Create the surface file
    //
    SurfaceFile sf(numCoords, numTriangles);
    
    //
    // Copy the coordinates and normals
    //
    for (int i = 0; i < numCoords; i++) {
        sf.setCoordinate(i, coordinates.getCoordinate(i));
    }
    
    //
    // Copy the triangles
    //
    for (int i = 0; i < numTriangles; i++) {
        sf.setTriangle(i, topology->getTile(i));
    }
    
    //
    // Set the coordinate type and topology type
    //
//    sf.setCoordinateType(getSurfaceTypeName());
    sf.setCoordinateType(coordinates.getHeaderTag(AbstractFile::headerTagConfigurationID));
    if (topology != NULL) {
        sf.setTopologyType(topology->getTopologyTypeName());
    }
    
    //
    // set the metadata
    //
    GiftiMetaData* coordMetaData = sf.getCoordinateMetaData();
    if (coordMetaData != NULL) {
        std::cout << "Before copy metadata: " << qPrintable(sf.getCoordinateType()) << std::endl;
        coordMetaData->copyMetaDataFromCaretFile(&coordinates);
        std::cout << "After copy metadata: " << qPrintable(sf.getCoordinateType()) << std::endl;
    }
    
    GiftiMetaData* topoMetaData = sf.getTopologyMetaData();
    if (topoMetaData != NULL) {
        topoMetaData->copyMetaDataFromCaretFile(topology);
    }
    
    sf.removeHeaderTag("date");
    sf.removeHeaderTag("encoding");
    sf.setHeaderTag("Date", QDateTime::currentDateTime().toString(Qt::ISODate));
    sf.setHeaderTag("UserName", SystemUtilities::getUserName());
    
    QString fileName = FileUtilities::basename(filenameIn);
    if (fileName.endsWith(".coord")) {
        fileName = FileUtilities::replaceExtension(fileName, ".coord",
                                                   prependToFileNameExtension + SpecFile::getGiftiSurfaceFileExtension());
    }
    else if (fileName.endsWith(".surf.gii")) {
        fileName = FileUtilities::replaceExtension(fileName, ".surf.gii",
                                                   prependToFileNameExtension + SpecFile::getGiftiSurfaceFileExtension());
    }
    else if (fileName.endsWith(".coord.gii")) {
        fileName = FileUtilities::replaceExtension(fileName, ".coord.gii",
                                                   prependToFileNameExtension + SpecFile::getGiftiSurfaceFileExtension());
    }
    else {
        fileName = fileName + prependToFileNameExtension + SpecFile::getGiftiSurfaceFileExtension();
    }
    
    //
    // Write the file
    //
    try {
        sf.setFileWriteType(AbstractFile::FILE_FORMAT_XML_GZIP_BASE64);
        sf.writeFile(fileName);
    }
    catch (FileException& e) {
        throw e;
    }
    
    //
    // clear modified status for coordinates and topology
    //
    coordinates.clearModified();
    
    return fileName;
}

/**
 * write the surface file.
 */
void 
BrainModelSurface::writeSurfaceFile(const QString& filename,
                            const AbstractFile::FILE_FORMAT fileFormat) throw (FileException)
{
   //
   // Determine number of coordinates and triangles
   //
   const int numCoords = coordinates.getNumberOfCoordinates();
   int numTriangles = 0;
   if (topology != NULL) {
      numTriangles = topology->getNumberOfTiles();
   }
   
   //
   // Create the surface file
   //
   SurfaceFile sf(numCoords, numTriangles);
   
   //
   // Copy the coordinates and normals
   //
   for (int i = 0; i < numCoords; i++) {
      sf.setCoordinate(i, coordinates.getCoordinate(i));
   }
   
   //
   // Copy the triangles
   //
   for (int i = 0; i < numTriangles; i++) {
      sf.setTriangle(i, topology->getTile(i));
   }
   
   //
   // set the metadata
   //
   GiftiMetaData* coordMetaData = sf.getCoordinateMetaData();
   if (coordMetaData != NULL) {
      coordMetaData->copyMetaDataFromCaretFile(&coordinates);
   }
   GiftiMetaData* topoMetaData = sf.getTopologyMetaData();
   if (topoMetaData != NULL) {
      topoMetaData->copyMetaDataFromCaretFile(topology);
   }
   
   //
   // Set the coordinate type and topology type
   //
   sf.setCoordinateType(getSurfaceTypeName());
   if (topology != NULL) {
      sf.setTopologyType(topology->getTopologyTypeName());
   }
   
   //
   // Write the file
   //
   try {
      sf.setFileWriteType(fileFormat);
      sf.writeFile(filename);
   }
   catch (FileException& e) {
      throw e;
   }
   
   //
   // clear modified status for coordinates and topology
   //
   coordinates.clearModified();
   //if (topology != NULL) {
   //   topology->clearModified();
   //}
}

/**
 * Read the specified coordinate file.  May throw FileException.
 */
void
BrainModelSurface::readCoordinateFile(const QString& filename) throw(FileException)
{
   try {
      coordinates.readFile(filename);
      initializeNormals();
      const QString st = coordinates.getHeaderTag(AbstractFile::headerTagStructure);
      structure.setTypeFromString(st);
      //setStructure(hemisphereStringToType(hem));
      coordinates.clearModified(); // setting hem caused modification
   }
   catch (FileException& e) {
      reset();
      throw e;
   }
}

/**
 * set a normal.
 */
void 
BrainModelSurface::setNormal(const int coordinateNumber,
                             const float normalVector[3])
{
   const int i3 = coordinateNumber * 3;
   normals[i3]   = normalVector[0];
   normals[i3+1] = normalVector[1];
   normals[i3+2] = normalVector[2];
}                     

/**
 * Create the normals
 */
void
BrainModelSurface::initializeNormals(const int numCoordsIn)
{
   int numCoords = numCoordsIn;
   if (numCoords <= 0) {
      numCoords = coordinates.getNumberOfCoordinates();
   }
   normals.clear();
   if (numCoords > 0) {
      for (int i = 0; i < numCoords; i++) {
         normals.push_back(0.0);
         normals.push_back(0.0);
         normals.push_back(1.0);
      }
   }
}

/**
 * get the display list for this brain model.
 */
unsigned int 
BrainModelSurface::getDisplayListNumber()
{
   //  
   // Check to see if topology modification has changed
   //
   if (topology != NULL) {
      const unsigned long topoModNumber = topology->getModified();
      if (topoModNumber != lastTopologyModificationNumber) {
         coordinates.clearDisplayList();
      }
      lastTopologyModificationNumber = topoModNumber;
   }
   
   return coordinates.getDisplayListNumber();
}

/**
 * set the display list for this brain model.
 */
void 
BrainModelSurface::setDisplayListNumber(unsigned int num)
{
   coordinates.setDisplayListNumber(num);
}
      
/**
 * Reset the surface - clear everything in it, usually called prior to 
 * loading new files.
 */
void
BrainModelSurface::reset()
{
   surfaceType = SURFACE_TYPE_UNKNOWN;
   structure.setType(Structure::STRUCTURE_TYPE_INVALID);
   coordinates.clear();
   normals.clear();
   topology = NULL;
   defaultScaling = 1.0;
   defaultPerspectiveZooming = 200.0;
   displayHalfX = 0;
   displayHalfY = 0;
   resetViewingTransformations();
   lastTopologyModificationNumber = 100002283;  // big number
}

/**
 * Set the topology for this surface
 */
bool
BrainModelSurface::setTopologyFile(TopologyFile* topologyIn)
{
   bool tooManyNodesInTopology = false;;
   
   if (topologyIn != NULL) {
      if (getNumberOfNodes() < topologyIn->getNumberOfNodes()) {
         tooManyNodesInTopology = true;
      }
   }
   
   topology = topologyIn;
   if (topology != NULL) {
      topology->setNumberOfNodes(getNumberOfNodes());
   }
   coordinates.clearDisplayList();
   
   QString topoFileName;
   if (topology != NULL) {
      topoFileName = FileUtilities::basename(topology->getFileName());
   }
   if (topoFileName.isEmpty() == false) {
      const unsigned long modifiedStatus = coordinates.getModified();
      coordinates.setHeaderTag(SpecFile::getUnknownTopoFileMatchTag(), topoFileName);
      coordinates.setModifiedCounter(modifiedStatus);
   }
   
   return tooManyNodesInTopology;
}

/**
 * see if the surface is topologically correct (has no handles).
 */
bool 
BrainModelSurface::isTopologicallyCorrect() const
{
   int faces,
       vertices,
       edges,
       eulerCount,
       holes,
       objects;
       
   const bool flatFlag = ((surfaceType == SURFACE_TYPE_FLAT) ||
                          (surfaceType == SURFACE_TYPE_FLAT_LOBAR));
   if (topology != NULL) {
      topology->getEulerCount(flatFlag,
                              faces,
                              vertices,
                              edges,
                              eulerCount,
                              holes,
                              objects);
      if (flatFlag) {
         if (eulerCount == 1) {
            return true;
         }
      }
      else if (eulerCount == 2) {
         return true;
      }
   }
   
   return false;
}
      
/**
 * Convert this surface and its current topology to a VTK surface.
 */
vtkPolyData*
BrainModelSurface::convertToVtkPolyData() const
{
   if (topology != NULL) {
      const int numTiles = topology->getNumberOfTiles();
      const int numCoords = coordinates.getNumberOfCoordinates();
      if ((numCoords > 0) && (numTiles > 0)) {
      
         vtkPoints* pointData = vtkPoints::New();
         for (int i = 0; i < numCoords; i++) {
            pointData->InsertPoint(i, coordinates.getCoordinate(i));
         }
         
         vtkCellArray* cells = vtkCellArray::New();
         int size = cells->EstimateSize(numTiles, 3);
         cells->Allocate(size, 25);
         for (int j = 0; j < numTiles; j++) {
            int v[3];
            topology->getTile(j, v[0], v[1], v[2]);
            vtkIdType vt[3] = { v[0], v[1], v[2] };
            cells->InsertNextCell(static_cast<vtkIdType>(3), vt);
         }
         
         vtkPolyData* polyData = vtkPolyData::New();
         polyData->SetPoints(pointData);
         polyData->SetPolys(cells);
         
         vtkPolyDataNormals* normals = vtkPolyDataNormals::New();
#ifdef HAVE_VTK6
         normals->SetInputData(polyData);
#else
         normals->SetInput(polyData);
#endif
         normals->SplittingOff();
         normals->ConsistencyOn();
         normals->ComputePointNormalsOn();
         normals->NonManifoldTraversalOn();
         normals->SetAutoOrientNormals(1);
         normals->Update();
         
         vtkPolyData* polyOut = vtkPolyData::New();
         polyOut->DeepCopy(normals->GetOutput());

         normals->Delete();
         polyData->Delete();
         cells->Delete();
         pointData->Delete();
         
         return polyOut;
      }
   }
   
   return NULL;
}

/**
 * import from a MNI OBJ Surface File.
 */
void 
BrainModelSurface::importFromMniObjSurfaceFile(const MniObjSurfaceFile& mni) throw (FileException)
{
   try {
      coordinates.importFromMniObjSurfaceFile(mni);
      initializeNormals();
      const int num = mni.getNumberOfPoints();
      for (int i = 0; i < num; i++) {
         const float* nv = mni.getNormal(i);
         normals[i*3]   = nv[0];
         normals[i*3+1] = nv[1];
         normals[i*3+2] = nv[2];
      }
      appendToCoordinateFileComment("Imported from ");
      appendToCoordinateFileComment(mni.getFileName());
      appendToCoordinateFileComment("\n");
   }
   catch (FileException& e) {
      reset();
      throw e;
   }
}
      
/**
 * import from a brain voyager file.
 */
void 
BrainModelSurface::importFromBrainVoyagerFile(const BrainVoyagerFile& bvf) throw (FileException)
{
   try {
      coordinates.importFromBrainVoyagerFile(bvf);
      initializeNormals();
      appendToCoordinateFileComment("Imported from ");
      appendToCoordinateFileComment(bvf.getFileName());
      appendToCoordinateFileComment("\n");
   }
   catch (FileException& e) {
      reset();
      throw e;
   }
}

/**
 * import from a VTK surface file
 */
void 
BrainModelSurface::importFromVtkFile(vtkPolyData* polyData,
                                     const QString& fileName) throw (FileException)
{
   try {
      coordinates.importFromVtkFile(polyData);
      initializeNormals();
      if (fileName.isEmpty() == false) {
         appendToCoordinateFileComment("Imported from ");
         appendToCoordinateFileComment(fileName);
         appendToCoordinateFileComment("\n");
      }
   }
   catch (FileException& e) {
      reset();
      throw e;
   }
}      

/**
 * Copy the Coordinates from the VTK PolyData.
 */
void
BrainModelSurface::copyCoordinatesFromVTK(vtkPolyData* polyData)
{
   const int numCoords = coordinates.getNumberOfCoordinates();
   const int numPoints = polyData->GetNumberOfPoints();
   
   if (numCoords == numPoints) {
      vtkPoints* points = polyData->GetPoints();
      for (int i = 0; i < numPoints; i++) {
#ifdef HAVE_VTK5
         double xyz[3];
#else // HAVE_VTK5
         float xyz[3];
#endif // HAVE_VTK5
         points->GetPoint(i, xyz);
         coordinates.setCoordinate(i, xyz); 
      }
      
      if ((normals.size() == 0) && (numPoints > 0)) {
         initializeNormals();
      }
   }
   else {
      std::cerr << "VTK poly data has different number of points" << std::endl;
   }
}

/**
 * Copy the Topology from the VTK PolyData.
 */
void
BrainModelSurface::copyTopologyFromVTK(vtkPolyData* polyData)
{
   if (topology != NULL) {
   
      vtkTriangleFilter* triangleFilter = NULL;
      if (polyData->GetNumberOfStrips() > 0) {
         triangleFilter = vtkTriangleFilter::New();
#ifdef HAVE_VTK6
         triangleFilter->SetInputData(polyData);
#else
         triangleFilter->SetInput(polyData);
#endif
         triangleFilter->Update();
         polyData->Delete();
         polyData = triangleFilter->GetOutput();
      }
      
      const int numTiles = topology->getNumberOfTiles();
      vtkCellArray* cells = polyData->GetPolys();
      if (cells->GetNumberOfCells() == numTiles) {
         int cellID = 0;
         vtkIdType npts;
         vtkIdType* pts;
         for (cells->InitTraversal(); cells->GetNextCell(npts, pts); cellID++) {
            if (npts == 3) {
               int verts[3];
               topology->getTile(cellID, verts[0], verts[1], verts[2]);
               if ((pts[0] != verts[0]) ||
                   (pts[1] != verts[1]) ||
                   (pts[2] != verts[2])) {
                  topology->setTile(cellID, pts[0], pts[1], pts[2]);
               }
            }
         }
      }
      else {
         std::cerr << "VTK poly data has different number of tiles" << std::endl;
      }
   }
}

/**
 * Orient the normals so that they point out of the surface.  This is accomplished by
 * finding the coordinate with the largest Z.  The normal's Z should be positive.  If
 * not, all tiles in the surface need to have their orientation flipped and the normals
 * need to be flipped.  NOTE: Normals must be computed prior to calling this method.
 * Returns true if the normals were flipped.
 */
bool
BrainModelSurface::orientNormalsOut()
{
   bool normalsWereFlipped = false;
   
   if (topology != NULL) {
      const int numTiles = topology->getNumberOfTiles();
      const int numCoords = coordinates.getNumberOfCoordinates();
      if ((numCoords > 0) && (numTiles > 0)) {
         
         //
         // Some coordinates may not be used by tiles so they need to be ignored.
         //
         bool* coordUsed = new bool[numCoords];
         for (int k = 0; k < numCoords; k++) {
            coordUsed[k] = false;
         }
         
         for (int j = 0; j < numTiles; j++) {
            int v1, v2, v3;
            topology->getTile(j, v1, v2, v3);
            coordUsed[v1] = true;
            coordUsed[v2] = true;
            coordUsed[v3] = true;
         }
         
         //
         // Find coordinate with largest Z value;
         //
         float maxZ = 0.0;
         int maxIndex = -1;
         for (int i = 0; i < numCoords; i++) {
            if (coordUsed[i]) {
               const float* xyz = coordinates.getCoordinate(i);
               if ((maxIndex < 0) || (xyz[2] > maxZ)) {
                  maxZ = xyz[2];
                  maxIndex = i;
               }
            }
         }
         
         //
         // If normal at coord with max Z is negative, then flip tiles and normals
         //
         if (maxIndex >= 0) {
            const float* normal = getNormal(maxIndex);
            if (normal[2] < 0.0) {
               flipNormals();
               normalsWereFlipped = true;
            }
         }
         
         delete[] coordUsed;
      }
   }
   
   coordinates.clearDisplayList();

   return normalsWereFlipped;
}

/**
 * Flip the normal's orientation
 */
void
BrainModelSurface::flipNormals()
{
/*
   const int num = normals.size();
   for (int i = 0; i < num; i++) {
      normals[i] = -normals[i];
   }
*/
   if (topology != NULL) {
      topology->flipTileOrientation();
      
      for (int i = 0; i < brainSet->getNumberOfBrainModels(); i++) {
         BrainModelSurface* bms = brainSet->getBrainModelSurface(i);
         if (bms != NULL) {
            if (bms->getTopologyFile() == topology) {
               bms->computeNormals();
            }
         }
      }
   }
   coordinates.clearDisplayList();
}

/**
 * convert normals to rgb paint.
 */
void 
BrainModelSurface::convertNormalsToRgbPaint(RgbPaintFile* rpf)
{
   const int numNodes = getNumberOfNodes();
   if (numNodes <= 0) {
      return;
   }
   
   if (rpf->getNumberOfNodes() <= 0) {
      rpf->setNumberOfNodesAndColumns(numNodes, 1);
   }
   else {
      rpf->addColumns(1);
   }
   const int columnNumber = rpf->getNumberOfColumns() - 1;
   
   QString comment("Surface normals from ");
   comment.append(FileUtilities::basename(coordinates.getFileName()));
   rpf->setColumnComment(columnNumber, comment);
   
   rpf->setScaleRed(columnNumber,   0.0, 255.0);
   rpf->setScaleGreen(columnNumber, 0.0, 255.0);
   rpf->setScaleBlue(columnNumber,  0.0, 255.0);
   
   for (int i = 0; i < numNodes; i++) {
      const int i3 = i * 3;
      float r = fabs(normals[i3]) * 255.0;
      float g = fabs(normals[i3+1]) * 255.0;
      float b = fabs(normals[i3+2]) * 255.0;
      rpf->setRgb(i, columnNumber, r, g, b);
/*
      float r = fabs(normals[i3]   - 0.5);
      float g = fabs(normals[i3+1] - 0.5);
      float b = fabs(normals[i3+2] - 0.5);
      const float tot = std::sqrt(r*r + g*g + b*b);
      if (tot != 0.0) {
         r /= tot;
         g /= tot;
         b /= tot;
         r *= 255.0;
         g *= 255.0;
         b *= 255.0;
         rpf->setRgb(i, columnNumber, r, g, b);
      }
*/
   }
}      

/**
 * copy normals to surface vector file.
 */
void 
BrainModelSurface::copyNormalsToVectorFile(VectorFile* vf) const
{
   const int numNodes = getNumberOfNodes();
   
   //
   // Add column to surface vector file (if needed)
   //
   if (vf->getNumberOfVectors() != this->getNumberOfNodes()) {
      vf->setNumberOfVectors(numNodes);
   }
   for (int i = 0; i < numNodes; i++) {
      const float* xyz = this->coordinates.getCoordinate(i);
      vf->setVectorData(i, xyz, &normals[i*3], 1.0, i);
   }
}
                                    
/**
 * Compute the normals for this surface.
 * If "coordsIn" is not NULL, then "coordsIn" is used as the coordinates for
 * determining the normals.  Otherwise, the surface's coordinates are used.
 * Passing the coordinates in may speed up some algorithms so that they 
 * do not have to load the coordinates into the surface.
 */
void
BrainModelSurface::computeNormals(const float* coordsIn, bool averageNormals)
{
   if (topology != NULL) {
      const int numCoords = coordinates.getNumberOfCoordinates();
      if (numCoords > 0) {

         //
         // pointer to coordinates
         //
         const float* coords = (coordsIn != NULL) ? coordsIn : coordinates.getCoordinate(0);
         
         //
         // See if normals need to be created
         //
         if (static_cast<int>(normals.size()) != (numCoords * 3)) {
            initializeNormals();
         }
         std::fill(normals.begin(), normals.end(), 0.0);

         //
         // node normals are average of the node's tiles' normals
         //
         const int numTiles = topology->getNumberOfTiles();
         for (int i = 0; i < numTiles; i++) {
            int va, vb, vc; 
            topology->getTile(i, va, vb, vc);            
            
            const int va3 = va * 3;
            const int vb3 = vb * 3;
            const int vc3 = vc * 3;
            
            float tileNormal[3];
            MathUtilities::computeNormal((float*)&coords[va3],
                                       (float*)&coords[vb3],
                                       (float*)&coords[vc3],
                                       tileNormal);
            normals[va3]   += tileNormal[0];
            normals[va3+1] += tileNormal[1];
            normals[va3+2] += tileNormal[2];
            normals[vb3]   += tileNormal[0];
            normals[vb3+1] += tileNormal[1];
            normals[vb3+2] += tileNormal[2];
            normals[vc3]   += tileNormal[0];
            normals[vc3+1] += tileNormal[1];
            normals[vc3+2] += tileNormal[2];
         }
         
         for (int k = 0; k < numCoords; k++) {
            const int k3 = k * 3;
               MathUtilities::normalize(&normals[k3]);//checks for length 0 vectors, so we don't need to track number that contribute
         }
         
         if (averageNormals)
         {
            float* normalSum = new float[3 * numCoords];
         	const TopologyHelper* myhelper = topology->getTopologyHelper(false, true, false);
         	std::vector<int> neighbors;
            for (int i = 0; i < numCoords; ++i)
            {
               const int i3 = i * 3;
               normalSum[i3] = normals[i3];
               normalSum[i3 + 1] = normals[i3 + 1];
               normalSum[i3 + 2] = normals[i3 + 2];
               myhelper->getNodeNeighbors(i, neighbors);
               const int numNeigh = (int)neighbors.size();
               for (int j = 0; j < numNeigh; ++j)
               {
                  const int node3 = neighbors[j] * 3;
                  normalSum[i3] += normals[node3];
                  normalSum[i3 + 1] += normals[node3 + 1];
                  normalSum[i3 + 2] += normals[node3 + 2];
               }
               MathUtilities::normalize(&normalSum[i3]);//don't need to divide by 1 + number of neighbors if we just normalize
            }
            const int numCoords3 = numCoords * 3;
            for (int i = 0; i < numCoords3; ++i)
            {//now that we are done with the original normals, we can overwrite them
               normals[i] = normalSum[i];
            }
            delete[] normalSum;
         }
      }
   }
   coordinates.clearDisplayList();
}

/*
 * Compute normals but use VTK's normals generation which tries to make
 * sure all tiles have their nodes ordered consistently.
 *
void
BrainModelSurface::computeNormalsWithTileOrientationConsistency()
{
   const int numCoords = coordinates.getNumberOfCoordinates();
   if (topology == NULL) {
      if ((numCoords * 3) != static_cast<int>(normals.size())) {
         initializeNormals();
      }
      return;
   }

   vtkPolyData* polyData = convertToVtkPolyData();      

   //
   // Use VTK to compute normals.
   //    Do not split edges.
   //    Make tiles consistent.      
   //    Compute Normals for the points.
   //  
   vtkPolyDataNormals* polyNormals = vtkPolyDataNormals::New();
   polyNormals->SetInput(polyData);
   polyNormals->SplittingOff();
   polyNormals->ConsistencyOn();
   polyNormals->ComputePointNormalsOn();
   polyNormals->NonManifoldTraversalOn();
   polyNormals->Update();
  
   vtkPolyData* output = polyNormals->GetOutput();
   vtkPointData* outputPoints = output->GetPointData();
   vtkDataArray* normalsOut = outputPoints->GetNormals();
  
   if ((numCoords * 3) != static_cast<int>(normals.size())) {
      initializeNormals();
   }

   if ((normalsOut->GetNumberOfTuples() == numCoords) &&
       (normalsOut->GetNumberOfComponents() == 3)) {
      for (int i = 0; i < numCoords; i++) {
         const float* xyz = normalsOut->GetTuple3(i);
         normals[i * 3]     = xyz[0];
         normals[i * 3 + 1] = xyz[1];
         normals[i * 3 + 2] = xyz[2];
      }
   
      copyTopologyFromVTK(output);
   }
   else {
      for (int i = 0; i < numCoords; i++) {
         normals[i * 3]     = 0.0;
         normals[i * 3 + 1] = 0.0;
         normals[i * 3 + 2] = 1.0;
      }
      std::cerr << "Normals calculation failed" << std::endl;
   }
  
//   normalsOut->Delete();
//   outputPoints->Delete();
//   output->Delete();
   polyNormals->Delete();
   polyData->Delete();
}
*/


/**
 * Orient the tiles consistently for this surface
 */ 
void
BrainModelSurface::orientTilesConsistently()
{
   //
   // Ensure topology
   //
   if (topology == NULL) {
      return;
   }
   if (topology->getNumberOfTiles() <= 0) {
      return;
   }
   
   vtkPolyData* polyData = convertToVtkPolyData();
   
   //
   // Use VTK to compute normals.
   //    Do not split edges.
   //    Make tiles CONSISTENT.
   //    Compute Normals for the points.
   //
   vtkPolyDataNormals* polyNormals = vtkPolyDataNormals::New();
#ifdef HAVE_VTK6
   polyNormals->SetInputData(polyData);
#else
   polyNormals->SetInput(polyData);
#endif
   polyNormals->SplittingOff(); 
   polyNormals->ConsistencyOn();
   polyNormals->ComputePointNormalsOn(); 
   polyNormals->NonManifoldTraversalOn();
   polyNormals->Update();
   
   vtkPolyData* output = polyNormals->GetOutput();
   vtkPointData* outputPoints = output->GetPointData();
   vtkDataArray* normalsOut = outputPoints->GetNormals();
   
   const int numCoords = coordinates.getNumberOfCoordinates();
   if ((normalsOut->GetNumberOfTuples() == numCoords) &&
       (normalsOut->GetNumberOfComponents() == 3)) {
      copyTopologyFromVTK(output);
   }
   else { 
      std::cerr << "Normals calculation failed for orienting tiles" << std::endl;
   }
   
//   normalsOut->Delete();
//   outputPoints->Delete();
//   output->Delete();
   polyNormals->Delete();
   polyData->Delete();
   
   computeNormals();
   orientNormalsOut();
   coordinates.clearDisplayList();
}

/**
 * Orient the tiles so that each tile's normal points outward.
 * This should only be used for flat and spherical surfaces.
 * This is probably a bad thing to do since it may screw up the topology.
 */
void
BrainModelSurface::orientTilesOutward(const SURFACE_TYPES st)
{
   //
   // Check for flat or spherical surface
   //
   bool sphereFlag = false;
   bool flatFlag   = false;
   switch (st) {
      case SURFACE_TYPE_RAW:
         break;
      case SURFACE_TYPE_FIDUCIAL:
         break;
      case SURFACE_TYPE_INFLATED:
         break;
      case SURFACE_TYPE_VERY_INFLATED:
         break;
      case SURFACE_TYPE_SPHERICAL:
         sphereFlag = true;
         break;
      case SURFACE_TYPE_ELLIPSOIDAL:
         break;
      case SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         break;
      case SURFACE_TYPE_FLAT:
         flatFlag = true;
         break;
      case SURFACE_TYPE_FLAT_LOBAR:
         flatFlag = true;
         break;
      case SURFACE_TYPE_HULL:
         break;
      case SURFACE_TYPE_UNKNOWN:
         break;
      case SURFACE_TYPE_UNSPECIFIED:
         break;
   }
   if ((sphereFlag == false) && (flatFlag == false)) {
      return;
   }
   
   TopologyFile* tf = getTopologyFile();
   if (tf != NULL) {
      //
      // Check each tile
      //
      const int numTiles = tf->getNumberOfTiles();
      for (int i = 0; i < numTiles; i++) {
         //
         // Get the nodes in the tile
         //
         int n1, n2, n3;
         tf->getTile(i, n1, n2, n3);
         
         //
         // Get the coordinates of the nodes
         //
         const float* c1 = coordinates.getCoordinate(n1);
         const float* c2 = coordinates.getCoordinate(n2);
         const float* c3 = coordinates.getCoordinate(n3);
         //
         // Compute the tiles normal
         //
         float normal[3];
         MathUtilities::computeNormal((float*)c1,
                                    (float*)c2,
                                    (float*)c3,
                                    normal);
          
         bool flipOrientation = false;
         if (flatFlag) {
            //
            // Does normal point downward ?
            //
            if (normal[2] < 0.0) {
               flipOrientation = true;
            }
         }
         else if (sphereFlag) {
            //
            // Use average of the three nodes as normal
            // Assumes sphere is centered at origin
            //
            float avg[3] = { (c1[0] + c2[0] + c3[0]) / 3.0,
                             (c1[1] + c2[1] + c3[1]) / 3.0,
                             (c1[2] + c2[2] + c3[2]) / 3.0 };
            MathUtilities::normalize(avg);
            
            //
            // Does normal point into the sphere
            //
            if (MathUtilities::dotProduct(normal, avg) < 0.0) {
               flipOrientation = true;
            }
         }
         
         if (flipOrientation) {
            tf->setTile(i, n3, n2, n1);
         }
      }
   }
   coordinates.clearDisplayList();
}

/**
 * smooth the surface by moving nodes along their normals multiplied by curvature.
 */
void
BrainModelSurface::smoothSurfaceUsingCurvature(const float strength,
                                               const int numSteps,
                                               const float curvatureMaximum)
{
   QTime timer;
   timer.start();

   const int numNodes = coordinates.getNumberOfCoordinates();
   int curvatureCounter = 1;
   const int updateCurvatureEveryX = 1;
   SurfaceShapeFile ssf;
   ssf.setNumberOfNodesAndColumns(numNodes, 1);
   const int curvatureColumn = 0;
   
   float* coords = coordinates.getCoordinate(0);
   
   const TopologyHelper* th = topology->getTopologyHelper(false, true, false);

   for (int steps = 1; steps <= numSteps; steps++) {
      //
      // Determine if curvature should be generated
      //
      bool doCurvature = false;
      if (steps == 1) {
         doCurvature = true;
      }
      if (updateCurvatureEveryX > 0) {
         if (curvatureCounter == updateCurvatureEveryX) {
            doCurvature = true;
            curvatureCounter = 1;
         }
         else {
            curvatureCounter++;
         }
      }
      
      if (doCurvature) {
         //
         // Update the surface curvature (does normals too)
         //
         BrainModelSurfaceCurvature bmsc(brainSet,
                                this,
                                &ssf,
                                curvatureColumn,
                                BrainModelSurfaceCurvature::CURVATURE_COLUMN_DO_NOT_GENERATE,
                                "meanCurv",
                                "");
         try {
            bmsc.execute();
         }
         catch (BrainModelAlgorithmException&) {
         }
      }
      else {
         computeNormals();
      }
      
      //
      // Adjust each nodes position
      //
      for (int i = 0; i < numNodes; i++) {
         if (th->getNodeHasNeighbors(i)) {
            float curv = ssf.getValue(i, curvatureColumn);
            curv = std::max(curv, -curvatureMaximum);
            curv = std::min(curv,  curvatureMaximum);
            float* norm = &normals[i*3];
            const float delta[3] = {
               norm[0] * curv * strength,
               norm[1] * curv * strength,
               norm[2] * curv * strength,
            };
         
            coords[i*3]   += delta[0];
            coords[i*3+1] += delta[1];
            coords[i*3+2] += delta[2];
         }
      }
      coordinates.setModified();
      
      //
      // Possibly redraw this brain model
      //
      brainSet->drawBrainModel(this, steps);
      
      std::cout << "Iter: " << steps << std::endl;
   }
   
   std::cout << "time: " << (static_cast<float>(timer.elapsed()) / 1000.0) 
             << std::endl;
}

/**
 * expand the surface by moving nodes along their normals by the specified amount.
 */
void 
BrainModelSurface::expandSurface(const float expandAmount)
{
   const int numNodes = getNumberOfNodes();
   for (int i = 0; i < numNodes; i++) {
      float xyz[3];
      coordinates.getCoordinate(i, xyz);
      if ((xyz[0] != 0.0) || (xyz[1] != 0.0) || (xyz[2] != 0.0)) {
         const int i3 = i * 3;
         xyz[0] += (normals[i3]   * expandAmount);
         xyz[1] += (normals[i3+1] * expandAmount);
         xyz[2] += (normals[i3+2] * expandAmount);
         coordinates.setCoordinate(i, xyz);
      }
   }
   computeNormals();
}

/**
 * Set the surface to a standard view
 */
void 
BrainModelSurface::setToStandardView(const int viewNumber, const STANDARD_VIEWS view)
{
   float m[16];
   bool valid = true;
   
   switch(view) {
      case VIEW_LATERAL:
         if (structure.getType() != Structure::STRUCTURE_TYPE_CORTEX_RIGHT) {
           m[0]  =  0.0;
           m[1]  =  0.0;
           m[2]  = -1.0;
           m[3]  =  0.0;
           m[4]  = -1.0;
           m[5]  =  0.0;
           m[6]  =  0.0;
           m[7]  =  0.0;
           m[8]  =  0.0;
           m[9]  =  1.0;
           m[10] =  0.0;
           m[11] =  0.0;
           m[12] =  0.0;
           m[13] =  0.0;
           m[14] =  0.0;
           m[15] =  1.0;
         }
         else {
           m[0]  =  0.0;
           m[1]  =  0.0;
           m[2]  =  1.0;
           m[3]  =  0.0;
           m[4]  =  1.0;
           m[5]  =  0.0;
           m[6]  =  0.0;
           m[7]  =  0.0;
           m[8]  =  0.0;
           m[9]  =  1.0;
           m[10] =  0.0;
           m[11] =  0.0;
           m[12] =  0.0;
           m[13] =  0.0;
           m[14] =  0.0;
           m[15] =  1.0;
         }
         break;
      case VIEW_MEDIAL:
         if (structure.getType() != Structure::STRUCTURE_TYPE_CORTEX_RIGHT) {
           m[0]  =  0.0;       
           m[1]  =  0.0;
           m[2]  =  1.0;
           m[3]  =  0.0;  
           m[4]  =  1.0;
           m[5]  =  0.0;
           m[6]  =  0.0;
           m[7]  =  0.0;
           m[8]  =  0.0;
           m[9]  =  1.0;
           m[10] =  0.0;
           m[11] =  0.0;
           m[12] =  0.0;
           m[13] =  0.0;
           m[14] =  0.0;
           m[15] =  1.0;
         }
         else {
           m[0]  =  0.0;
           m[1]  =  0.0;
           m[2]  = -1.0;
           m[3]  =  0.0;
           m[4]  = -1.0;
           m[5]  =  0.0;
           m[6]  =  0.0;
           m[7]  =  0.0;
           m[8]  =  0.0;
           m[9]  =  1.0;
           m[10] =  0.0;
           m[11] =  0.0;
           m[12] =  0.0;
           m[13] =  0.0;
           m[14] =  0.0;
           m[15] =  1.0;
         }
         break;
      case VIEW_POSTERIOR:
         if (structure.getType() != Structure::STRUCTURE_TYPE_CORTEX_RIGHT) {
           m[0]  =  1.0;
           m[1]  =  0.0;
           m[2]  =  0.0;
           m[3]  =  0.0;
           m[4]  =  0.0;
           m[5]  =  0.0;
           m[6]  = -1.0;
           m[7]  =  0.0;
           m[8]  =  0.0;
           m[9]  =  1.0;
           m[10] =  0.0;
           m[11] =  0.0;
           m[12] =  0.0;
           m[13] =  0.0;
           m[14] =  0.0;
           m[15] =  1.0;
         }
         else {
           m[0]  =  1.0;
           m[1]  =  0.0;
           m[2]  =  0.0;
           m[3]  =  0.0;
           m[4]  =  0.0;
           m[5]  =  0.0;
           m[6]  = -1.0;
           m[7]  =  0.0;
           m[8]  =  0.0;
           m[9]  =  1.0;
           m[10] =  0.0;
           m[11] =  0.0;
           m[12] =  0.0;
           m[13] =  0.0;
           m[14] =  0.0;
           m[15] =  1.0;
         }
         break;
      case VIEW_ANTERIOR:
         if (structure.getType() != Structure::STRUCTURE_TYPE_CORTEX_RIGHT) {
           m[0]  = -1.0;
           m[1]  =  0.0;
           m[2]  =  0.0;
           m[3]  =  0.0;
           m[4]  =  0.0;
           m[5]  =  0.0;
           m[6]  =  1.0;
           m[7]  =  0.0;
           m[8]  =  0.0;
           m[9]  =  1.0;
           m[10] =  0.0;
           m[11] =  0.0;
           m[12] =  0.0;
           m[13] =  0.0;
           m[14] =  0.0;
           m[15] =  1.0;
         }
         else {
           m[0]  = -1.0;
           m[1]  =  0.0;
           m[2]  =  0.0;
           m[3]  =  0.0;
           m[4]  =  0.0;
           m[5]  =  0.0;
           m[6]  =  1.0;
           m[7]  =  0.0;
           m[8]  =  0.0;
           m[9]  =  1.0;
           m[10] =  0.0;
           m[11] =  0.0;
           m[12] =  0.0;
           m[13] =  0.0;
           m[14] =  0.0;
           m[15] =  1.0;
         }
         break;
      case VIEW_VENTRAL:
         if (structure.getType() != Structure::STRUCTURE_TYPE_CORTEX_RIGHT) {
           m[0]  = -1.0;
           m[1]  =  0.0;
           m[2]  =  0.0;
           m[3]  =  0.0;
           m[4]  =  0.0;
           m[5]  =  1.0;
           m[6]  =  0.0;
           m[7]  =  0.0;
           m[8]  =  0.0;
           m[9]  =  0.0;
           m[10] = -1.0;
           m[11] =  0.0;
           m[12] =  0.0;
           m[13] =  0.0;
           m[14] =  0.0;
           m[15] =  1.0;
         }
         else {
           m[0]  = -1.0;
           m[1]  =  0.0;
           m[2]  =  0.0;
           m[3]  =  0.0;
           m[4]  =  0.0;
           m[5]  =  1.0;
           m[6]  =  0.0;
           m[7]  =  0.0;
           m[8]  =  0.0;
           m[9]  =  0.0;
           m[10] = -1.0;
           m[11] =  0.0;
           m[12] =  0.0;
           m[13] =  0.0;
           m[14] =  0.0;
           m[15] =  1.0;
         }
         break;
      case VIEW_DORSAL:
         if (structure.getType() != Structure::STRUCTURE_TYPE_CORTEX_RIGHT) {
           m[0]  =  1.0;
           m[1]  =  0.0;
           m[2]  =  0.0;
           m[3]  =  0.0;
           m[4]  =  0.0;
           m[5]  =  1.0;
           m[6]  =  0.0;
           m[7]  =  0.0;
           m[8]  =  0.0;
           m[9]  =  0.0;
           m[10] =  1.0;
           m[11] =  0.0;
           m[12] =  0.0;
           m[13] =  0.0;
           m[14] =  0.0;
           m[15] =  1.0;
         }
         else {
           m[0]  =  1.0;
           m[1]  =  0.0;
           m[2]  =  0.0;
           m[3]  =  0.0;
           m[4]  =  0.0;
           m[5]  =  1.0;
           m[6]  =  0.0;
           m[7]  =  0.0;
           m[8]  =  0.0;
           m[9]  =  0.0;
           m[10] =  1.0;
           m[11] =  0.0;
           m[12] =  0.0;
           m[13] =  0.0;
           m[14] =  0.0;
           m[15] =  1.0;
         }
         break;
      case VIEW_RESET:
         m[0]  =  1.0;
         m[1]  =  0.0;
         m[2]  =  0.0;
         m[3]  =  0.0;
         m[4]  =  0.0;
         m[5]  =  1.0;
         m[6]  =  0.0;
         m[7]  =  0.0;
         m[8]  =  0.0;
         m[9]  =  0.0;
         m[10] =  1.0;
         m[11] =  0.0;
         m[12] =  0.0;
         m[13] =  0.0;
         m[14] =  0.0;
         m[15] =  1.0;
         perspectiveZooming[viewNumber] = defaultPerspectiveZooming;
         scaling[viewNumber][0] = defaultScaling;
         scaling[viewNumber][1] = defaultScaling;
         scaling[viewNumber][2] = defaultScaling;
         translation[viewNumber][0] = 0.0;
         translation[viewNumber][1] = 0.0;
         translation[viewNumber][2] = 0.0;
         break;
      case VIEW_ROTATE_X_90:
         rotationMatrix[viewNumber]->RotateX(90.0);
         valid = false;
         break;
      case VIEW_ROTATE_Y_90:
         rotationMatrix[viewNumber]->RotateY(90.0);
         valid = false;
         break;
      case VIEW_ROTATE_Z_90:
         rotationMatrix[viewNumber]->RotateZ(-90.0);
         valid = false;
         break;
      case VIEW_NONE:
      default:
         valid = false;
         break;
   }
   
   if (valid) {
      setRotationMatrix(viewNumber, m);
   }
}

/**
 * Get the text name of the surface type
 */
QString
BrainModelSurface::getSurfaceTypeName() const
{
   QString s("Other");
   
   switch(surfaceType) {
      case SURFACE_TYPE_RAW:
         s = "Raw";
         break;
      case SURFACE_TYPE_FIDUCIAL:
         s = "Fiducial";
         break;
      case SURFACE_TYPE_INFLATED:
         s = "Inflated";
         break;
      case SURFACE_TYPE_VERY_INFLATED:
         s = "VeryInflated";
         break;
      case SURFACE_TYPE_SPHERICAL:
         s = "Spherical";
         break;
      case SURFACE_TYPE_ELLIPSOIDAL:
         s = "Ellipsoidal";
         break;
      case SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         s = "CompMedWall";
         break;
      case SURFACE_TYPE_FLAT:
         s = "Flat";
         break;
      case SURFACE_TYPE_FLAT_LOBAR:
         s = "FlatLobar";
         break;
      case SURFACE_TYPE_HULL:
         s = "Hull";
         break;
      case SURFACE_TYPE_UNKNOWN:
         s = "Unknown";
         break;
      case SURFACE_TYPE_UNSPECIFIED:
      default:
         s = "Unspecified";
         break;
   }
   
   return s;
}

/**
 * Get all surface types and names.
 */
void 
BrainModelSurface::getSurfaceTypesAndNames(std::vector<SURFACE_TYPES>& typesOut,
                                           std::vector<QString>& typeNamesOut)
{
   typesOut.clear();
   typeNamesOut.clear();
   
   typesOut.push_back(SURFACE_TYPE_RAW);
      typeNamesOut.push_back(getSurfaceConfigurationIDFromType(SURFACE_TYPE_RAW));
   typesOut.push_back(SURFACE_TYPE_FIDUCIAL);
      typeNamesOut.push_back(getSurfaceConfigurationIDFromType(SURFACE_TYPE_FIDUCIAL));
   typesOut.push_back(SURFACE_TYPE_INFLATED);
      typeNamesOut.push_back(getSurfaceConfigurationIDFromType(SURFACE_TYPE_INFLATED));
   typesOut.push_back(SURFACE_TYPE_VERY_INFLATED);
      typeNamesOut.push_back(getSurfaceConfigurationIDFromType(SURFACE_TYPE_VERY_INFLATED));
   typesOut.push_back(SURFACE_TYPE_SPHERICAL);
      typeNamesOut.push_back(getSurfaceConfigurationIDFromType(SURFACE_TYPE_SPHERICAL));
   typesOut.push_back(SURFACE_TYPE_ELLIPSOIDAL);
      typeNamesOut.push_back(getSurfaceConfigurationIDFromType(SURFACE_TYPE_ELLIPSOIDAL));
   typesOut.push_back(SURFACE_TYPE_COMPRESSED_MEDIAL_WALL);
      typeNamesOut.push_back(getSurfaceConfigurationIDFromType(SURFACE_TYPE_COMPRESSED_MEDIAL_WALL));
   typesOut.push_back(SURFACE_TYPE_FLAT);
      typeNamesOut.push_back(getSurfaceConfigurationIDFromType(SURFACE_TYPE_FLAT));
   typesOut.push_back(SURFACE_TYPE_FLAT_LOBAR);
      typeNamesOut.push_back(getSurfaceConfigurationIDFromType(SURFACE_TYPE_FLAT_LOBAR));
   typesOut.push_back(SURFACE_TYPE_HULL);
      typeNamesOut.push_back(getSurfaceConfigurationIDFromType(SURFACE_TYPE_HULL));
   typesOut.push_back(SURFACE_TYPE_UNKNOWN);
      typeNamesOut.push_back(getSurfaceConfigurationIDFromType(SURFACE_TYPE_UNKNOWN));
   typesOut.push_back(SURFACE_TYPE_UNSPECIFIED);
      typeNamesOut.push_back(getSurfaceConfigurationIDFromType(SURFACE_TYPE_UNSPECIFIED));
}

/**
 * Get the surface type for a surface type name
 */
BrainModelSurface::SURFACE_TYPES
BrainModelSurface::getSurfaceTypeFromConfigurationID(const QString& name)
{
   if (name == "RAW") return SURFACE_TYPE_RAW;
   else if (name == "FIDUCIAL") return SURFACE_TYPE_FIDUCIAL;
   else if (name == "INFLATED") return SURFACE_TYPE_INFLATED;
   else if (name == "VERY_INFLATED") return SURFACE_TYPE_VERY_INFLATED;
   else if (name == "SPHERICAL") return SURFACE_TYPE_SPHERICAL;
   else if (name == "ELLIPSOIDAL") return SURFACE_TYPE_ELLIPSOIDAL;
   else if (name == "CMW") return SURFACE_TYPE_COMPRESSED_MEDIAL_WALL;
   else if (name == "FLAT") return SURFACE_TYPE_FLAT;
   else if (name == "FLAT_LOBAR") return SURFACE_TYPE_FLAT_LOBAR;
   else if (name == "HULL") return SURFACE_TYPE_HULL;
   else return SURFACE_TYPE_UNKNOWN;
}

/**
 * Get the Configuration ID from the surface type.
 */
QString
BrainModelSurface::getSurfaceConfigurationIDFromType(const SURFACE_TYPES st)
{
   QString configID("UNKNOWN");
   switch(st) {
      case SURFACE_TYPE_RAW:
         configID = "RAW";
         break;
      case SURFACE_TYPE_FIDUCIAL:
         configID = "FIDUCIAL";
         break;
      case SURFACE_TYPE_INFLATED:
         configID = "INFLATED";
         break;
      case SURFACE_TYPE_VERY_INFLATED:
         configID = "VERY_INFLATED";
         break;
      case SURFACE_TYPE_SPHERICAL:
         configID = "SPHERICAL";
         break;
      case SURFACE_TYPE_ELLIPSOIDAL:
         configID = "ELLIPSOIDAL";
         break;
      case SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         configID = "CMW";
         break;
      case SURFACE_TYPE_FLAT:
         configID = "FLAT";
         break;
      case SURFACE_TYPE_FLAT_LOBAR:
         configID = "FLAT_LOBAR";
         break;
      case SURFACE_TYPE_HULL:
         configID = "HULL";
         break;
      case SURFACE_TYPE_UNKNOWN:
      case SURFACE_TYPE_UNSPECIFIED:
         configID = "UNKNOWN";
         break;
   }
   return configID;
}

/**
 * Get the spec file tag from the coord type.
 */
QString
BrainModelSurface::getCoordSpecFileTagFromSurfaceType(const SURFACE_TYPES st)
{
   QString tag(SpecFile::getUnknownCoordFileMatchTag());
   
   switch(st) {
      case BrainModelSurface::SURFACE_TYPE_RAW:
         tag = SpecFile::getRawCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
         tag = SpecFile::getFiducialCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_INFLATED:
         tag = SpecFile::getInflatedCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
         tag = SpecFile::getVeryInflatedCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
         tag = SpecFile::getSphericalCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
         tag = SpecFile::getEllipsoidCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         tag = SpecFile::getCompressedCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT:
         tag = SpecFile::getFlatCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
         tag = SpecFile::getLobarFlatCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_HULL:
         tag = SpecFile::getHullCoordFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
      case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
         tag = SpecFile::getUnknownCoordFileMatchTag();
         break;
   }
   
   return tag;
}

/**
 * Get the spec file tag from the surface type.
 */
QString
BrainModelSurface::getSurfaceSpecFileTagFromSurfaceType(const SURFACE_TYPES st)
{
   QString tag(SpecFile::getUnknownSurfaceFileMatchTag());
   
   switch(st) {
      case BrainModelSurface::SURFACE_TYPE_RAW:
         tag = SpecFile::getRawSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
         tag = SpecFile::getFiducialSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_INFLATED:
         tag = SpecFile::getInflatedSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
         tag = SpecFile::getVeryInflatedSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
         tag = SpecFile::getSphericalSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
         tag = SpecFile::getEllipsoidSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         tag = SpecFile::getCompressedSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT:
         tag = SpecFile::getFlatSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
         tag = SpecFile::getLobarFlatSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_HULL:
         tag = SpecFile::getHullSurfaceFileTag();
         break;
      case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
      case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
         tag = SpecFile::getUnknownSurfaceFileMatchTag();
         break;
   }
   
   return tag;
}

/**
 * see if surface is a fiducial surface.
 */
bool 
BrainModelSurface::getIsFiducialSurface() const
{
   switch(surfaceType) {
      case BrainModelSurface::SURFACE_TYPE_RAW:
         break;
      case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
         return true;
         break;
      case BrainModelSurface::SURFACE_TYPE_INFLATED:
         break;
      case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
         break;
      case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
         break;
      case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
         break;
      case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT:
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
         break;
      case BrainModelSurface::SURFACE_TYPE_HULL:
         break;
      case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
      case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
         break;
   }
   
   return false;
}

/**
 * see if surface is a flat surface.
 */
bool 
BrainModelSurface::getIsFlatSurface() const
{
   switch(surfaceType) {
      case BrainModelSurface::SURFACE_TYPE_RAW:
         break;
      case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
         break;
      case BrainModelSurface::SURFACE_TYPE_INFLATED:
         break;
      case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
         break;
      case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
         break;
      case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
         break;
      case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT:
         return true;
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
         return true;
         break;
      case BrainModelSurface::SURFACE_TYPE_HULL:
         break;
      case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
      case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
         break;
   }

   return false;
}

/**
 * set the surface type and update the configuration ID too.
 */
void 
BrainModelSurface::setSurfaceType(const SURFACE_TYPES st)
{ 
   surfaceType = st;
   const QString configID = 
      BrainModelSurface::getSurfaceConfigurationIDFromType(st);
   coordinates.setHeaderTag(AbstractFile::headerTagConfigurationID, configID);
}      

/**
 * set the structure.
 */
void 
BrainModelSurface::setStructure(const Structure::STRUCTURE_TYPE st) 
{ 
   structure.setType(st); 
   coordinates.setHeaderTag(AbstractFile::headerTagStructure, 
                            structure.getTypeAsString());
}

/**
 * set the structure.
 */
void 
BrainModelSurface::setStructure(const Structure st) 
{ 
   structure = st; 
   coordinates.setHeaderTag(AbstractFile::headerTagStructure, 
                            structure.getTypeAsString());
}

/**
 *  Set the default scaling for the surface so that the surface fills 
 *  most of the window.
 */
void 
BrainModelSurface::setDefaultScaling(const double orthoRight,
                           const double orthoTop)
{
   displayHalfX = orthoRight;
   displayHalfY = orthoTop;
   updateForDefaultScaling();
   coordinates.clearDisplayList();
}

/**
 * set the default scaling.
 */
void 
BrainModelSurface::setDefaultScaling(const float scale) 
{ 
   defaultScaling = scale; 
   coordinates.clearDisplayList();
}

/**
 * Update the default scaling so that the surface fits in the graphics window
 */
void
BrainModelSurface::updateForDefaultScaling()
{
   const CoordinateFile* cf = getCoordinateFile();
   if (cf->getNumberOfCoordinates() <= 0) {
      defaultScaling = 1.0;
      defaultPerspectiveZooming = 200.0;
      return;
   }

   if ((displayHalfX <= 0) || (displayHalfY <= 0)) {
      return;
   }
   
   float bounds[6];
   cf->getBounds(bounds);
   
   float bigX = bounds[1];
   if (fabs(bounds[0]) > bigX) {
      bigX = fabs(bounds[0]);
   }
   float bigY = bounds[3];
   if (fabs(bounds[2]) > bigY) {
      bigY = fabs(bounds[2]);
   }

   const float ratio = displayHalfX / displayHalfY;
   
   const float percentScreenToUse = 0.90;
   
   const float percentScreenX = displayHalfX * percentScreenToUse * ratio;
   const float percentScreenY = displayHalfY   * percentScreenToUse;
   
   const float scaleX = percentScreenX / bigX;
   const float scaleY = percentScreenY / bigY;
   
   if (scaleX < scaleY) {
      defaultScaling = scaleX;
   }
   else {
      defaultScaling = scaleY;
   }

   defaultPerspectiveZooming = 200;
   if ((bigX > 0.0) && (bigY > 0.0)) {
      const float big = std::max(bigX, bigY) * 1.10;
      defaultPerspectiveZooming = big / 
         tan(perspectiveFieldOfView * 0.5 * MathUtilities::degreesToRadians());
   }
   
   for (int i = 0; i < NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; i++) {
      setScaling(i, defaultScaling, defaultScaling, defaultScaling);
      setPerspectiveZooming(i, defaultPerspectiveZooming);
   }
   coordinates.clearDisplayList();
}

/**
 * Create latitude longitude for this spherical surface
 */
void
BrainModelSurface::createLatitudeLongitude(LatLonFile* llf, 
                                           const int columnNumberIn,
                                           const QString& columnName,
                                           const bool setDeformedLatLonFlag,
                                           const bool sphereInDorsalViewFlag)
{
   const int numNodes = getNumberOfNodes();

   int columnNumber = columnNumberIn;
   if ((columnNumber < 0) || (columnNumber >= llf->getNumberOfColumns())) {
      if (llf->getNumberOfColumns() > 0) {
         llf->addColumns(1);
      }
      else {
         llf->setNumberOfNodesAndColumns(numNodes, 1);
      }
      columnNumber = llf->getNumberOfColumns() - 1;
   }
   llf->setColumnName(columnNumber, columnName);
   
   std::ostringstream str;
   str << "\nLatitude-Longitude calculated on surface "
       << FileUtilities::basename(coordinates.getFileName()).toAscii().constData();
   if (topology != NULL) {
      str << " with topology " << FileUtilities::basename(topology->getFileName()).toAscii().constData();
   }
   str << "\n";
   llf->appendToFileComment(str.str().c_str());
   
   std::vector<float> savedNormals;
   if (sphereInDorsalViewFlag) {
      //
      // Put surface in lateral view for lat/lon computation
      //
      pushCoordinates();
      
      savedNormals = normals;
      
      float m[16];
      if (structure.getType() != Structure::STRUCTURE_TYPE_CORTEX_RIGHT) {
        m[0]  =  0.0;
        m[1]  =  0.0;
        m[2]  = -1.0;
        m[3]  =  0.0;
        m[4]  = -1.0;
        m[5]  =  0.0;
        m[6]  =  0.0;
        m[7]  =  0.0;
        m[8]  =  0.0;
        m[9]  =  1.0;
        m[10] =  0.0;
        m[11] =  0.0;
        m[12] =  0.0;
        m[13] =  0.0;
        m[14] =  0.0;
        m[15] =  1.0;
      }
      else {
        m[0]  =  0.0;
        m[1]  =  0.0;
        m[2]  =  1.0;
        m[3]  =  0.0;
        m[4]  =  1.0;
        m[5]  =  0.0;
        m[6]  =  0.0;
        m[7]  =  0.0;
        m[8]  =  0.0;
        m[9]  =  1.0;
        m[10] =  0.0;
        m[11] =  0.0;
        m[12] =  0.0;
        m[13] =  0.0;
        m[14] =  0.0;
        m[15] =  1.0;
      }
      vtkMatrix4x4* matrix =vtkMatrix4x4::New();
      int cnt = 0; 
      for (int i = 0; i < 4; i++) {
         for (int j = 0; j < 4; j++) { 
            matrix->SetElement(i, j, m[cnt]); 
            cnt++; 
         }
      }
      vtkTransform* rotationMatrix = vtkTransform::New();
      rotationMatrix->SetMatrix(matrix);
      
      TransformationMatrix rot;
      rot.setMatrix(rotationMatrix);
      rot.transpose();  // I'm not sure why this works JWH perhaps OpenGL backwards from VTK.
      applyTransformationMatrix(rot);
      
      matrix->Delete();
      rotationMatrix->Delete();
   }
   
   const bool leftHemisphereFlag = (structure.getType() == Structure::STRUCTURE_TYPE_CORTEX_LEFT);
   
   for (int i = 0; i < numNodes; i++) {
      float xyz[3];
      coordinates.getCoordinate(i, xyz);
      if (leftHemisphereFlag) {
         xyz[0] = -xyz[0];
      }
      const float xy = std::sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1]);
      float lat = 0.0, lon = 0.0;
      if (xy == 0.0) {
         if (xyz[0] >= 0.0) {
            lat = 90.0;
         }
         else {
            lat = -90.0;
         }
      }
      else {
         lat = std::atan2(xyz[2], xy) * MathUtilities::radiansToDegrees();
      }
      
      //if (xyz[0] == 0.0) {
      //   lon = 0.0;
      //}
      //else {
         lon = std::atan2(xyz[1], xyz[0]) * MathUtilities::radiansToDegrees();
      //}
      
      if (setDeformedLatLonFlag) {
         llf->setDeformedLatLon(i, columnNumber, lat, lon);
      }
      else {
         llf->setLatLon(i, columnNumber, lat, lon);
      }
   }
   
   if (sphereInDorsalViewFlag) {
      popCoordinates();
      normals = savedNormals;
   }   
}

/**
 * Create Spherical Lat/Lon borders.
 */
void
BrainModelSurface::createSphericalLatLonBorders(BorderFile& bf,
                                                const bool sphereInDorsalViewFlag)
{
   //
   // Create a matrix that is applied to the borders
   //
   TransformationMatrix transMatrix;
   
   //
   // Is the sphere in a dorsal view ?
   //
   const bool leftHemFlag = (structure.getType() == Structure::STRUCTURE_TYPE_CORTEX_LEFT);
   if (sphereInDorsalViewFlag) {
      if (leftHemFlag) {
         transMatrix.rotate(TransformationMatrix::ROTATE_Y_AXIS, 90.0);
         transMatrix.rotate(TransformationMatrix::ROTATE_Z_AXIS, 90.0);
         transMatrix.inverse();
      }
      else {
         transMatrix.rotate(TransformationMatrix::ROTATE_Y_AXIS, -90.0);
         transMatrix.rotate(TransformationMatrix::ROTATE_Z_AXIS, -90.0);
         transMatrix.inverse();
      }
   }
   
   const float sphereRadius = getSphericalSurfaceRadius();
   
   const int numLat = 6;
   const int latStep = 180 / numLat;
   const int latStart = -90 + latStep;   

   for (int lat = latStart; lat < 90.0; lat += latStep) {
      QString ML;
      if (lat > 0.0) {
         if (leftHemFlag) {
            ML = "+";
         }
         else {
            ML = "+";
         }
      }
      else if (lat < 0.0) {
         if (leftHemFlag) {
            ML = "-";
         }
         else {
            ML = "-";
         }
      }
      std::ostringstream str;
      str << "LAT."
          << ML.toAscii().constData()
          << abs(lat);
      Border border(str.str().c_str());
      
      //
      // angle from positive Z axis
      //
      const float z = sphereRadius * std::sin(lat * MathUtilities::degreesToRadians());
      float radiusAtLat = std::sqrt(sphereRadius * sphereRadius - z * z);
      
      int imax = static_cast<int>(360.0 * (radiusAtLat / sphereRadius));
      
      for (int i = 0; i < imax; i++) {
         const float phi = (2.0 * i * M_PI) / imax;
         const float xyz[3] = { radiusAtLat * std::cos(phi),
                                radiusAtLat * std::sin(phi),
                                z };
         border.addBorderLink(xyz);
      }
      border.applyTransformationMatrix(transMatrix);
      bf.addBorder(border);
   }

   //
   // For a right hemisphere in a lateral view, 0 longitude is on the screen
   // X-axis.  Hence the zero longitude line starts at the lateral pole and
   // and wraps around the frontal pole, ending at the medial pole.
   // Longitude increases in a counter-clockwise direction.
   //
   const int longStep = latStep;
   for (int longitude = 0; longitude < 360; longitude += longStep) {
      std::ostringstream str;
      
      QString negLabel("");;
      int lonLabel = longitude;
      if (leftHemFlag) {
         lonLabel = 180 - lonLabel;
      }
      if (lonLabel > 180.0) {
         negLabel = "-";
         lonLabel = 360 - lonLabel;
      }
      else if ((lonLabel > 0) && (lonLabel < 180.0)) {
         negLabel = "+";
      }
      /*
      if (getStructure() == HEMISPHERE_RIGHT) {
         int lonLabel = 180 - longitude;
         if (lonLabel < 0) {
            lonLabel += 360;
         }
      }
      */
      
      str << "LONG."
          << negLabel.toAscii().constData()
          << lonLabel;
      Border border(str.str().c_str());

      const float clon = std::cos(longitude * MathUtilities::degreesToRadians()); 
      const float slon = std::sin(longitude * MathUtilities::degreesToRadians());

      for (int lat = -90; lat <= 90; lat++) {
         const float clat = std::cos(lat * MathUtilities::degreesToRadians()); 
         const float slat = std::sin(lat * MathUtilities::degreesToRadians());
         const float xyz[3] = { sphereRadius * clat * clon,
                                sphereRadius * clat * slon,
                                sphereRadius * slat };
         border.addBorderLink(xyz);
      }
      border.applyTransformationMatrix(transMatrix);
      bf.addBorder(border);
   }
}
/*
void
BrainModelSurface::createSphericalLatLonBorders(BorderFile& bf)
{
   TransformationMatrix matrix;
   if (getStructure() == HEMISPHERE_LEFT) {
      matrix.rotate(TransformationMatrix::ROTATE_Y_AXIS, 90.0);
      matrix.rotate(TransformationMatrix::ROTATE_X_AXIS, 90.0);
   }
   else {
      matrix.rotate(TransformationMatrix::ROTATE_Y_AXIS, -90.0);
      matrix.rotate(TransformationMatrix::ROTATE_X_AXIS,  90.0);
   }
   
   const float sphereRadius = getSphericalSurfaceRadius();
   
   const int sectorAngle = 30;
   const int sectorsPerQuadrant = static_cast<int>(((90.0 / sectorAngle) - 1.0));
   const int longTotal = 4 * (sectorsPerQuadrant + 1);
   
   int latitude = 90 - sectorAngle;
   for (int n = -sectorsPerQuadrant; n <= sectorsPerQuadrant; n++) {
      QString ML;
      if (latitude > 0.0) {
         ML = "L");
      }
      else if (latitude < 0.0) {
         ML = "M");
      }
      std::ostringstream str;
      str << "LAT."
          << ML
          << latitude;
      Border border(str.str().c_str());
      
      float t1 = (n * M_PI) / (2.0 * (sectorsPerQuadrant + 1.0));
      float z = 0.0;
      if (n != 0) {
         z = (n / fabs(n)) * sphereRadius * std::cos(t1);
      }
      
      float radiusAtLat = std::sqrt(sphereRadius * sphereRadius - z * z);
      
      int imax = static_cast<int>(360.0 * (radiusAtLat / sphereRadius));
      
      for (int i = 0; i < imax; i++) {
         const float phi = (2.0 * i * M_PI) / imax;
         const float xyz[3] = { radiusAtLat * std::cos(phi),
                                radiusAtLat * std::sin(phi),
                                z };
         border.addBorderLink(xyz);
      }
      border.applyTransformationMatrix(matrix);
      bf.addBorder(border);
      
      latitude -= sectorAngle;
   }
   
   int longitude = 0;
   for (int n = 0; n < longTotal; n++) {
      std::ostringstream str;
      str << "LON."
          << longitude;
      Border border(str.str().c_str());
      const float phi = (2.0 * M_PI * n) / static_cast<float>(longTotal);
      for (int i = 0; i < 180; i++) {
         const float theta = (static_cast<float>(i) * M_PI) / 180.0;
         const float z = sphereRadius * std::cos(theta);
         const float t1 = std::sqrt(sphereRadius * sphereRadius - z * z);
         const float xyz[3] = { t1 * std::cos(phi),
                                t1 * std::sin(phi),
                                z };
         border.addBorderLink(xyz);
      }
      border.applyTransformationMatrix(matrix);
      bf.addBorder(border);
      
      longitude += sectorAngle;
   }
}
*/

/**
 * create flat grid borders for analysis.
 * Each border consists of one point and each border has a unique name.
 */
void 
BrainModelSurface::createFlatGridBordersForAnalysis(BorderFile& bf,
                                                    const float bounds[4],
                                                    const float resolution)
{
   const float minX = bounds[0];
   const float maxX = bounds[1];
   const float minY = bounds[2];
   const float maxY = bounds[3];
   
   int j = 0;
   for (float y = minY; y <= maxY; y += resolution) {
      int i = 0;
      for (float x = minX; x <= maxX; x += resolution) {
         const QString name("Grid_"
                            + QString::number(i)
                            + "_"
                            + QString::number(j));
         float xyz[3] = { x, y, 0.0 };
         
         Border b(name);
         b.addBorderLink(xyz);
         bf.addBorder(b);
         
         i++;
      }
      
      j++;
   }
}
                                            
/**
 * Create Flat Grid Borders.
 */
void
BrainModelSurface::createFlatGridBorders(BorderFile& bf, const float gridSpacing,
                                         const int pointSpacing)
{
   float bounds[6];
   coordinates.getBounds(bounds);
   const int gridSpacingInt = static_cast<int>(gridSpacing);
   
   int xGridLeft = (static_cast<int>(bounds[0]) / gridSpacingInt) * gridSpacingInt;
   if (xGridLeft > 0) {
      xGridLeft -= gridSpacingInt;
   }
   else {
      xGridLeft -= (2 * gridSpacingInt);
   }
   
   int xGridRight = (static_cast<int>(bounds[1]) / gridSpacingInt) * gridSpacingInt;
   if (xGridRight >= 0) {
      xGridRight += (2 * gridSpacingInt);
   }
   else {
      xGridRight += gridSpacingInt;
   }
   const int xGridLines = ((xGridRight - xGridLeft) / gridSpacingInt) + 1;

   int yGridBottom = (static_cast<int>(bounds[2]) / gridSpacingInt) * gridSpacingInt;
   if (yGridBottom > 0) {
      yGridBottom -= gridSpacingInt;
   }
   else {
      yGridBottom -= (2 * gridSpacingInt);
   }
   
   int yGridTop = (static_cast<int>(bounds[3]) / gridSpacingInt) * gridSpacingInt;
   if (yGridTop >= 0) {
      yGridTop += (2 * gridSpacingInt);
   }
   else {
      yGridTop += gridSpacingInt;
   }
   const int yGridLines = ((yGridTop - yGridBottom) / gridSpacingInt) + 1;

   
   float x = static_cast<float>(xGridLeft);
   for (int i = 0; i < xGridLines; i++) {
      Border border("GridX");

      float y = static_cast<float>(yGridBottom);
      for (int j = 0; j < yGridLines; j++) {
         float xyz[3] = { x, y, 0.0 };
         border.addBorderLink(xyz);
         float nextY = y + gridSpacing;
         
         if ((pointSpacing > 0) && (j < (yGridLines - 1))) {
            float increment = gridSpacing / static_cast<float>(pointSpacing);
            y += increment;
            for (int k = 0; k < pointSpacing - 1; k++) {
               float xyz[3] = { x, y, 0.0 };
               border.addBorderLink(xyz);
               y += increment;
            }
         }
         y = nextY;
      }
      bf.addBorder(border);
      x += gridSpacing;
   }
   
   float y = static_cast<float>(yGridBottom);
   for (int i = 0; i < yGridLines; i++) {
      Border border("GridY");

      float x = static_cast<float>(xGridLeft);
      for (int j = 0; j < xGridLines; j++) {
         float xyz[3] = { x, y, 0.0 };
         border.addBorderLink(xyz);
         float nextX = x + gridSpacing;
         
         if ((pointSpacing > 0) && (j < (xGridLines - 1))) {
            float increment = gridSpacing / static_cast<float>(pointSpacing);
            x += increment;
            for (int k = 0; k < pointSpacing - 1; k++) {
               float xyz[3] = { x, y, 0.0 };
               border.addBorderLink(xyz);
               x += increment;
            }
         }
         x = nextX;
      }
      bf.addBorder(border);
      y += gridSpacing;
   }
}

/**
 * get the volume displacment of the surface.  Returns negative if it fails.
 */
float 
BrainModelSurface::getSurfaceVolumeDisplacement() const
{

   //
   // Get the bounds of the surface
   //
   float bounds[6];
   getBounds(bounds);
   const float dx = bounds[1] - bounds[0];
   const float dy = bounds[3] - bounds[2];
   const float dz = bounds[5] - bounds[4];
   
   //
   // Create the empty volume
   //
   const float pad = 10.0;
   const int dim[3] = {
      static_cast<int>(dx + pad),
      static_cast<int>(dy + pad),
      static_cast<int>(dz + pad)
   };
   const float origin[3] = {
      bounds[0] - (pad / 2.0),
      bounds[2] - (pad / 2.0),
      bounds[4] - (pad / 2.0),
   };
   const float spacing[3] = { 1.0, 1.0, 1.0 };
   const VolumeFile::ORIENTATION orient[3] = {
      VolumeFile::ORIENTATION_LEFT_TO_RIGHT,
      VolumeFile::ORIENTATION_POSTERIOR_TO_ANTERIOR,
      VolumeFile::ORIENTATION_INFERIOR_TO_SUPERIOR
   };   
   VolumeFile vf;
   vf.initialize(VolumeFile::VOXEL_DATA_TYPE_FLOAT,
                 dim,
                 orient,
                 origin,
                 spacing,
                 true,
                 true);
                 
   //
   // Convert the surface to a volume
   //
   try {
      BrainModelSurfaceToVolumeSegmentationConverter
         s2v((BrainSet*)getBrainSet(),
             (BrainModelSurface*)this,
             &vf,
             true,
             false);
      s2v.execute();
   }
   catch (BrainModelAlgorithmException&) {
      return 0;
   }
   
   if (DebugControl::getDebugOn()) {
      try {
         vf.writeFile("DebugSurfaceVolumeDisplacement.nii.gz");
      }
      catch (FileException&) {
      }
   }
   
   float displacement = vf.getNumberOfNonZeroVoxels();
   
   //
   // If all voxels set, probably open topology file
   //
   if (displacement == vf.getTotalNumberOfVoxelElements()) {
      displacement = -1;
   }
   
   return displacement;
}
      
/**
 * Get the surface's area
 */
float
BrainModelSurface::getSurfaceArea(const TopologyFile* tfin) const
{
   float area = 0.0;

   const TopologyFile* tf = ((tfin != NULL) ? tfin : topology);
   
   const int numTiles = tf->getNumberOfTiles();
   for (int i = 0; i < numTiles; i++) {
      const int* nodes = tf->getTile(i);
      const float* p1 = coordinates.getCoordinate(nodes[0]);
      const float* p2 = coordinates.getCoordinate(nodes[1]);
      const float* p3 = coordinates.getCoordinate(nodes[2]);
      area += MathUtilities::triangleArea((float*)p1,
                                        (float*)p2,
                                        (float*)p3);
   }
      
   return area;
}

/**
 * Get the area of a tile in the surface
 */
float
BrainModelSurface::getTileArea(const int tileNum) const
{
   const int* nodes = topology->getTile(tileNum);
   const float* p1 = coordinates.getCoordinate(nodes[0]);
   const float* p2 = coordinates.getCoordinate(nodes[1]);
   const float* p3 = coordinates.getCoordinate(nodes[2]);
   const float area = MathUtilities::triangleArea((float*)p1,
                                                (float*)p2,
                                                (float*)p3);
   return area;
}

/**
 * Get the area of a tile in the surface using three nodes
 */
float
BrainModelSurface::getTileArea(const int n1, const int n2, const int n3) const
{
   const float* p1 = coordinates.getCoordinate(n1);
   const float* p2 = coordinates.getCoordinate(n2);
   const float* p3 = coordinates.getCoordinate(n3);
   const float area = MathUtilities::triangleArea((float*)p1,
                                                (float*)p2,
                                                (float*)p3);
   return area;
}

/**
 * get the area of all tiles.
 */
void 
BrainModelSurface::getAreaOfAllTiles(std::vector<float>& tileAreas) const
{
   tileAreas.clear();
   if (topology != NULL) {
      const int numTiles = topology->getNumberOfTiles();
      tileAreas.resize(numTiles);
      for (int i = 0; i < numTiles; i++) {
         tileAreas[i] = getTileArea(i);
      }
   }
}

/**
 * get the area of all nodes.
 */
void 
BrainModelSurface::getAreaOfAllNodes(std::vector<float>& nodeAreas) const
{
   //
   // Since each tile consists of three nodes, apply one third of the tile area
   // to the node and sum these values
   //
   nodeAreas.clear();
   const int numNodes = getNumberOfNodes();
   if (numNodes >= 0) {
      nodeAreas.resize(numNodes, 0.0);

      std::vector<float> tileAreas;
      getAreaOfAllTiles(tileAreas);

      const int numTiles = static_cast<int>(tileAreas.size());
      if (numTiles > 0) {
         for (int i = 0; i < numTiles; i++) {
            const int* nodes = topology->getTile(i);
            const float oneThirdTileArea = tileAreas[i] * 0.33333;
            nodeAreas[nodes[0]] += oneThirdTileArea;
            nodeAreas[nodes[1]] += oneThirdTileArea;
            nodeAreas[nodes[2]] += oneThirdTileArea;
         }
      }
   }
}
      
/**
 * Apply a transformation matrix to the surface.
 */
void
BrainModelSurface::applyTransformationMatrix(TransformationMatrix& tm)
{
   const TopologyHelper* th = topology->getTopologyHelper(false, true, false);
   const int numNodes = getNumberOfNodes();
   for (int i = 0; i < numNodes; i++) {
      float xyz[3] = { 0.0, 0.0, 0.0 };
      if (th->getNodeHasNeighbors(i)) {
         coordinates.getCoordinate(i, xyz);
         double p[4] = { xyz[0], xyz[1], xyz[2], 1.0 };
         tm.multiplyPoint(p);
         xyz[0] = p[0];
         xyz[1] = p[1];
         xyz[2] = p[2];
      }
      coordinates.setCoordinate(i, xyz);
   }
//   resetViewingTransformations();
   computeNormals();
   
/*
   appendToCoordinateFileComment("Applied transformation matrix:\n");
   double m[4][4];
   tm.getMatrix(m);
   for (int i = 0; i < 4; i++) {
      appendToCoordinateFileComment("   ");
      for (int j = 0; j < 4; j++) {
         appendToCoordinateFileComment(" ");
         appendToCoordinateFileComment(StringUtilities::fromNumber(static_cast<float>(m[i][j])));
      }
      appendToCoordinateFileComment("\n");
   }
*/
}

/**
 * apply a view (transformation) to the coordinates of the surface.
 */
void 
BrainModelSurface::applyViewToCoordinates(const BrainModel::STANDARD_VIEWS surfaceView)
{
   if (structure.getType() == Structure::STRUCTURE_TYPE_CORTEX_RIGHT) {
      switch (surfaceView) {
         case BrainModel::VIEW_NONE:          
            break;
         case BrainModel::VIEW_RESET:
            break;
         case BrainModel::VIEW_ANTERIOR:
            {
               TransformationMatrix tm;
               tm.rotateX(-90.0);
               applyTransformationMatrix(tm);
               tm.identity();
               tm.rotateY(180.0);
               applyTransformationMatrix(tm);
            }
            break;
         case BrainModel::VIEW_DORSAL:
            //
            // Do nothing, dorsal is default view
            //
            break;
         case BrainModel::VIEW_LATERAL:
            {
               TransformationMatrix tm;
               tm.rotateX(-90.0);
               applyTransformationMatrix(tm);
               tm.identity();
               tm.rotateY(-90.0);
               applyTransformationMatrix(tm);
            }
            break;
         case BrainModel::VIEW_MEDIAL:
            {
               TransformationMatrix tm;
               tm.rotateX(-90.0);
               applyTransformationMatrix(tm);
               tm.identity();
               tm.rotateY(90.0);
               applyTransformationMatrix(tm);
            }
            break;
         case BrainModel::VIEW_POSTERIOR:
            {
               TransformationMatrix tm;
               tm.rotateX(-90.0);
               applyTransformationMatrix(tm);
            }
            break;
         case BrainModel::VIEW_VENTRAL:
            {
               TransformationMatrix tm;
               tm.rotateY(180.0);
               applyTransformationMatrix(tm);
            }
            break;
         case BrainModel::VIEW_ROTATE_X_90:
            break;
         case BrainModel::VIEW_ROTATE_Y_90:
            break;
         case BrainModel::VIEW_ROTATE_Z_90:
            break;
      }
   }
   else if (structure.getType() == Structure::STRUCTURE_TYPE_CORTEX_LEFT) {
      switch (surfaceView) {
         case BrainModel::VIEW_NONE:          
            break;
         case BrainModel::VIEW_RESET:
            break;
         case BrainModel::VIEW_ANTERIOR:
            {
               TransformationMatrix tm;
               tm.rotateX(-90.0);
               applyTransformationMatrix(tm);
               tm.identity();
               tm.rotateY(180.0);
               applyTransformationMatrix(tm);
            }
            break;
         case BrainModel::VIEW_DORSAL:
            //
            // Do nothing, dorsal is default view
            //
            break;
         case BrainModel::VIEW_LATERAL:
            {
               TransformationMatrix tm;
               tm.rotateX(-90.0);
               applyTransformationMatrix(tm);
               tm.identity();
               tm.rotateY(90.0);
               applyTransformationMatrix(tm);
            }
            break;
         case BrainModel::VIEW_MEDIAL:
            {
               TransformationMatrix tm;
               tm.rotateX(-90.0);
               applyTransformationMatrix(tm);
               tm.identity();
               tm.rotateY(-90.0);
               applyTransformationMatrix(tm);
            }
            break;
         case BrainModel::VIEW_POSTERIOR:
            {
               TransformationMatrix tm;
               tm.rotateX(-90.0);
               applyTransformationMatrix(tm);
            }
            break;
         case BrainModel::VIEW_VENTRAL:
            {
               TransformationMatrix tm;
               tm.rotateY(180.0);
               applyTransformationMatrix(tm);
            }
            break;
         case BrainModel::VIEW_ROTATE_X_90:
            break;
         case BrainModel::VIEW_ROTATE_Y_90:
            break;
         case BrainModel::VIEW_ROTATE_Z_90:
            break;
      }
   }   
}
      
/**
 * Apply current view to surface
 */
void 
BrainModelSurface::OLDapplyCurrentView(const int surfaceViewNumber,
                                    const bool applyTranslation,
                                    const bool applyRotation,
                                    const bool applyScaling)
{
/*
   TransformationMatrix tm;
   if (applyTranslation) {
      tm.translate(translation[surfaceViewNumber][0],
                   translation[surfaceViewNumber][1],
                   translation[surfaceViewNumber][2]);
      translation[surfaceViewNumber][0] = 0.0;
      translation[surfaceViewNumber][1] = 0.0;
      translation[surfaceViewNumber][2] = 0.0;
   }
   if (applyRotation) {
      TransformationMatrix rot;
      rot.setMatrix(rotationMatrix[surfaceViewNumber]);
      rot.transpose();  // I'm not sure why this works JWH perhaps OpenGL backwards from VTK.
      tm.multiply(rot);
      setToStandardView(surfaceViewNumber, VIEW_DORSAL);
   }
   if (applyScaling) {
      tm.scale(scaling[surfaceViewNumber]);
      setDefaultScaling(1.0);
      setScaling(surfaceViewNumber, 1.0, 1.0, 1.0);
   }
   applyTransformationMatrix(tm);
   coordinates.clearDisplayList();
*/
/*
   if (applyRotation) {
      TransformationMatrix rot;
      rot.setMatrix(rotationMatrix[surfaceViewNumber]);
      rot.transpose();  // I'm not sure why this works JWH perhaps OpenGL backwards from VTK.
      TransformationMatrix tm;
      tm.multiply(rot);
      setToStandardView(surfaceViewNumber, VIEW_DORSAL);
      applyTransformationMatrix(tm);
   }
   if (applyTranslation) {
      TransformationMatrix tm;
      tm.translate(translation[surfaceViewNumber][0],
                   translation[surfaceViewNumber][1],
                   translation[surfaceViewNumber][2]);
      applyTransformationMatrix(tm);
      translation[surfaceViewNumber][0] = 0.0;
      translation[surfaceViewNumber][1] = 0.0;
      translation[surfaceViewNumber][2] = 0.0;
   }
   if (applyScaling) {
      TransformationMatrix tm;
      tm.scale(scaling[surfaceViewNumber]);
      setDefaultScaling(1.0);
      setScaling(surfaceViewNumber, 1.0, 1.0, 1.0);
      applyTransformationMatrix(tm);
   }
*/
   glPushMatrix();
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      if (applyTranslation) {
         float t[3];
         getTranslation(surfaceViewNumber, t);
         glTranslatef(t[0], t[1], t[2]);
         t[0] = 0.0;
         t[1] = 0.0;
         t[2] = 0.0;
         setTranslation(surfaceViewNumber, t);
      }
      if (applyRotation) {
         float m[16];
         getRotationMatrix(surfaceViewNumber, m);
         glMultMatrixf(m);
      }
      if (applyScaling) {
         float s[3];
         getScaling(surfaceViewNumber, s);
         glScalef(s[0], s[1], s[2]);
         s[0] = 1.0;
         s[1] = 1.0;
         s[2] = 1.0;
         setScaling(surfaceViewNumber, s);
         setDefaultScaling(1.0);
      }
      
      float matrix[16];
      glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
      TransformationMatrix tm;
      tm.setMatrix(matrix);
      applyTransformationMatrix(tm);
      
      if (applyRotation) {
         setToStandardView(surfaceViewNumber, VIEW_DORSAL);
      }      
   glPopMatrix();
   
   coordinates.clearDisplayList();
}

/**
 * Apply current view to surface
 */
void
BrainModelSurface::applyCurrentView(const int surfaceViewNumber,
                                    const bool applyTranslation,
                                    const bool applyRotation,
                                    const bool applyScaling)
{
   TransformationMatrix tm;
   if (applyRotation) {
     float m[16];
     getRotationMatrix(surfaceViewNumber, m);
     TransformationMatrix rotMatrix;
     rotMatrix.setMatrix(m);
     tm.preMultiply(rotMatrix);
   }

   if (applyScaling) {
     float s[3];
     getScaling(surfaceViewNumber, s);
     tm.scale(s);
   }

   if (applyTranslation) {
     float t[3];
     getTranslation(surfaceViewNumber, t);
     tm.translate(t);
   }

   applyTransformationMatrix(tm);

   if (applyRotation) {
     setToStandardView(surfaceViewNumber, VIEW_DORSAL);
   }
   if (applyTranslation) {
     float t[3];
     t[0] = 0.0;
     t[1] = 0.0;
     t[2] = 0.0;
     setTranslation(surfaceViewNumber, t);
   }
   if (applyScaling) {
     float s[3];
     s[0] = 1.0;
     s[1] = 1.0;
     s[2] = 1.0;
     setScaling(surfaceViewNumber, s);
     setDefaultScaling(1.0);
   }
}
/**
 * Get the bounds of the connected surface.
 */
void
BrainModelSurface::getBounds(float bounds[6]) const
{
   const TopologyFile* tf = getTopologyFile();
   if (tf != NULL) {
      const TopologyHelper* th = tf->getTopologyHelper(false, true, false);
      const int numNodes = getNumberOfNodes();
      
      bounds[0] =  std::numeric_limits<float>::max();
      bounds[1] = -std::numeric_limits<float>::max();
      bounds[2] =  std::numeric_limits<float>::max();
      bounds[3] = -std::numeric_limits<float>::max();
      bounds[4] =  std::numeric_limits<float>::max();
      bounds[5] = -std::numeric_limits<float>::max();
      
      for (int i = 0; i < numNodes; i++) {
         if (th->getNodeHasNeighbors(i)) {
            const float* xyz = coordinates.getCoordinate(i);
            bounds[0] = std::min(bounds[0], xyz[0]);
            bounds[1] = std::max(bounds[1], xyz[0]);
            bounds[2] = std::min(bounds[2], xyz[1]);
            bounds[3] = std::max(bounds[3], xyz[1]);
            bounds[4] = std::min(bounds[4], xyz[2]);
            bounds[5] = std::max(bounds[5], xyz[2]);
         }
      }
   }
   else {
      bounds[0] = 0.0;
      bounds[1] = 0.0;
      bounds[2] = 0.0;
      bounds[3] = 0.0;
      bounds[4] = 0.0;
      bounds[5] = 0.0;
   }
}

/**
 * move disconnected nodes to origin
 */
void 
BrainModelSurface::moveDisconnectedNodesToOrigin()
{
   const TopologyHelper* th = topology->getTopologyHelper(false, true, false);
   const int numNodes = getNumberOfNodes();
   const float xyz[3] = { 0.0, 0.0, 0.0 };
   for (int i = 0; i < numNodes; i++) {
      if (th->getNodeHasNeighbors(i) == false) {
         coordinates.setCoordinate(i, xyz);
      }
   }
}

/**
 * Scale a flat surface to the specified area.
 * The "flatRatioFlag" should be set if the surface being scaled is flat
 * and the desired area is from a non-flat surface.
 */
void
BrainModelSurface::scaleSurfaceToArea(const float desiredArea,
                                      const bool flatRatioFlag)
{
   float currentSurfaceArea = getSurfaceArea();
   float ratio = desiredArea / currentSurfaceArea;
   if (flatRatioFlag) {
      ratio = std::sqrt(ratio);
      if (ratio > 0.0) {
         TransformationMatrix tm;
         tm.scale(ratio, ratio, ratio);
         applyTransformationMatrix(tm);
      }
   }
   else {
      const int maxIter = 5;
      std::vector<CoordinateFile> coords(maxIter + 1);
      std::vector<float> coordAreas(maxIter + 1, -1.0);
      
      coords[0]     = coordinates;
      coordAreas[0] = currentSurfaceArea;
      
      //
      // Save unscaled coordinates
      //
      pushCoordinates();
      
      for (int i = 1; i <= maxIter; i++) {
         if (ratio != 0.0) {
            TransformationMatrix tm;
            tm.scale(ratio, ratio, ratio);
            applyTransformationMatrix(tm);
         }

         //
         // Save coordinates and surface area
         //
         coords[i] = coordinates;
         coordAreas[i] = getSurfaceArea();
      
         //
         // See if new area is within an acceptable range
         //
         const float newArea = getSurfaceArea();
         const float diffArea = newArea - desiredArea;
         //if ((fabs(diffArea) / desiredArea) > 0.05) {
            //
            // Was surface being made larger
            //
            if (ratio > 1.0) {
               const float delta = ratio - 1.0;
               
               if (diffArea > 0) {
                  //
                  // Reduce scaling since surface is too large
                  //
                  ratio = delta * 0.5 + 1.0;
               }
               else {
                  //
                  // Increase scaling since surface is too small
                  //
                  ratio = delta * 2.0 + 1.0;
               }
            }
            //
            // Otherwise surface being made smaller
            //
            else {
               const float delta = 1.0 - ratio;
               
               if (diffArea > 0) {
                  //
                  // Reduce scaling since surface is too large
                  //
                  ratio = 1.0 - (delta * 2.0);
               }
               else {
                  //
                  // Increase scaling since surface is too small
                  //
                  ratio = 1.0 - (delta * 0.5);
               }
            }
         //}
         
         //
         // Restore saved coordinates
         //
         popCoordinates();
      }
      
      if (DebugControl::getDebugOn()) {
         std::cout << "Scaling to area: " << desiredArea << std::endl;
      }
      
      int bestFit = -1;
      float smallestDiff = std::numeric_limits<float>::max();
      for (int j = 0; j < static_cast<int>(coords.size()); j++) {
         if (DebugControl::getDebugOn()) {
            std::cout << "   Iteration: " << j << " surface area: " << coordAreas[j] << std::endl;
            float* xyz = coords[j].getCoordinate(0);
            std::cout << "   Coord 0: "
                      << xyz[0] << ", "
                      << xyz[1] << ", "
                      << xyz[2] << std::endl;
         }
         if (coordAreas[j] > 0.0) {
            const float diff = fabs(coordAreas[j] - desiredArea);
            if (diff < smallestDiff) {
               bestFit = j;
               smallestDiff = diff;
            }
         }
      }
      
      if (bestFit >= 0) {
         coordinates = coords[bestFit];
      }
      
      if (DebugControl::getDebugOn()) {
         std::cout << "   best fit is : " << bestFit << std::endl;      
      }
   }
   coordinates.clearDisplayList();

   if (DebugControl::getDebugOn()) {
      std::cout << "Surface area after scaling: " << this->getSurfaceArea() << std::endl;
   }
}

/**
 * Get the mean, min, max distance between the nodes
 */
void
BrainModelSurface::getMeanDistanceBetweenNodes(BrainModelSurfaceROINodeSelection* surfaceROI,
                                               float& meanDist,
                                               float& minDist,
                                               float& maxDist) const
{
   const TopologyHelper* helper = topology->getTopologyHelper(false, true, false);
   const float numNodes = helper->getNumberOfNodes();
   
   bool useRoiFlag = false;
   if (surfaceROI != NULL) {
      surfaceROI->update();
      useRoiFlag = true;
   }
   
   meanDist = 0.0;
   minDist = std::numeric_limits<float>::max();
   maxDist = -std::numeric_limits<float>::max();
   
   for (int i = 0; i < numNodes; i++) {
      bool doNode = true;
      if (useRoiFlag) {
         doNode = surfaceROI->getNodeSelected(i);
      }
      if (doNode) {
         std::vector<int> neighbors;
         helper->getNodeNeighbors(i, neighbors);
         const int numNeighbors = static_cast<int>(neighbors.size());
         
         float dist = 0.0;
         int neighCount = 0;
         for (int j = 0; j < numNeighbors; j++) {
            const int neigh = neighbors[j];
            bool checkNeigh = true;
            if (useRoiFlag) {
               checkNeigh = surfaceROI->getNodeSelected(neigh);
            }
            if (checkNeigh) {
               neighCount++;
               float nodeDist = coordinates.getDistanceBetweenCoordinates(i, neigh);
               dist += nodeDist;
               minDist = std::min(nodeDist, minDist);
               maxDist = std::max(nodeDist, maxDist);
            }
         }
         if (neighCount > 1) {
            meanDist += (dist / static_cast<float>(neighCount));
         }
      }
   }
   
   if (numNodes > 1) {
      meanDist /= static_cast<float>(numNodes);
   }
}

/**
 * Linearly Smooth the surface based upon edges connecting nodes
 */
void
BrainModelSurface::linearSmoothing(const float strength, const int iterations,
                                   const int smoothEdgesEveryXIterations,
                                   const std::vector<bool>* smoothOnlyTheseNodes,
                                   const int projectToSphereEveryXIterations)
{
   PreferencesFile* pf = brainSet->getPreferencesFile();
   const int numThreads = pf->getMaximumNumberOfThreads();
   
   if (numThreads > 0) {
      BrainModelSurfaceSmoothing smoothObject(brainSet,
                                              this,
                                              BrainModelSurfaceSmoothing::SMOOTHING_TYPE_LINEAR,
                                              strength,
                                              iterations,
                                              smoothEdgesEveryXIterations,
                                              0,
                                              smoothOnlyTheseNodes,
                                              NULL,
                                              projectToSphereEveryXIterations,
                                              numThreads);
      try {
         smoothObject.execute();
      }
      catch (BrainModelAlgorithmException&) {
      }
      
      return;
   }
   
/*
   appendToCoordinateFileComment("Linear Smoothing: ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(strength));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(iterations));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(smoothEdgesEveryXIterations));
   appendToCoordinateFileComment("\n");
*/
   
   //
   // Update node classification
   //
   brainSet->classifyNodes(getTopologyFile(), true);
   
   const float invStrength = 1.0 - strength;
   
   const int numNodes = getNumberOfNodes();
   if (numNodes <= 0) {
      return;
   }
   
   QTime timer;
   timer.start();
   const TopologyHelper* helper = topology->getTopologyHelper(false, true, false);
   if (DebugControl::getDebugOn()) {
      std::cout << "Topology Helper time: " << (static_cast<float>(timer.elapsed()) / 1000.0) << std::endl;
   }
   
   std::vector<bool> interiorNode(numNodes);
   for (int i = 0; i < numNodes; i++) {
      const BrainSetNodeAttribute* bna = brainSet->getNodeAttributes(i);
      interiorNode[i] = (bna->getClassification() == 
                         BrainSetNodeAttribute::CLASSIFICATION_TYPE_INTERIOR);
   }
   
   //
   // See smoothing is limited to a subset of nodes
   //
   bool smoothSubsetOfNodes = false;
   if (smoothOnlyTheseNodes != NULL) {
      if (static_cast<int>(smoothOnlyTheseNodes->size()) == numNodes) {
         smoothSubsetOfNodes = true;
      }
   }
   
   //
   // Get the radius in the event that this is a sphere
   //
   const float sphereRadius = getSphericalSurfaceRadius();
   
   //
   // Reduce calls to coord file
   //
   float* nodeCoords = new float[numNodes * 3];
   float* nodes      = new float[numNodes * 3];
   for (int i = 0; i < numNodes; i++) {
      coordinates.getCoordinate(i, &nodeCoords[i*3]);
   }
   
   //
   // Do 1 to iterations so that modulus check for edge smoothing works
   //
   for (int k = 1; k <= iterations; k++) {
      //std::vector<float> nodes(numNodes * 3);
      
      //
      // See if edges should be smoothed
      //
      bool smoothEdges = false;
      if (smoothEdgesEveryXIterations > 0) {
         if ((k % smoothEdgesEveryXIterations) == 0) {
            smoothEdges = true;
         }
      }
      
      for (int i = 0; i < numNodes; i++) {
         //coordinates.getCoordinate(i, &nodes[i*3]);
         nodes[i*3]   = nodeCoords[i*3];
         nodes[i*3+1] = nodeCoords[i*3+1];
         nodes[i*3+2] = nodeCoords[i*3+2];
         
         bool smoothIt = true;
         if (interiorNode[i] == false) {
            smoothIt = smoothEdges;
         }
         
         if (smoothIt && smoothSubsetOfNodes) {
            smoothIt = (*smoothOnlyTheseNodes)[i];
         }
         
         if (smoothIt) {   
            //std::vector<int> neighbors;
            //helper->getNodeNeighbors(i, neighbors);
            //const int numNeighbors = static_cast<int>(neighbors.size());
            
            int numNeighbors;
            const int* neighbors = helper->getNodeNeighbors(i, numNeighbors);
            
            if (numNeighbors > 0) {
               float neighXYZ[3] = { 0.0, 0.0, 0.0 };
               for (int j = 0; j < numNeighbors; j++) {
                  //float np[3];
                  //coordinates.getCoordinate(neighbors[j], np);
                  const int n = neighbors[j];
                  neighXYZ[0] += nodeCoords[n*3]; //np[0];
                  neighXYZ[1] += nodeCoords[n*3+1]; //np[1];
                  neighXYZ[2] += nodeCoords[n*3+2]; //np[2];
               }
               
               const float floatNumNeigh = numNeighbors;
               neighXYZ[0] /= floatNumNeigh;
               neighXYZ[1] /= floatNumNeigh;
               neighXYZ[2] /= floatNumNeigh;
               
               
               nodes[i*3]   = nodeCoords[i*3]   * invStrength + neighXYZ[0] * strength;
               nodes[i*3+1] = nodeCoords[i*3+1] * invStrength + neighXYZ[1] * strength;
               nodes[i*3+2] = nodeCoords[i*3+2] * invStrength + neighXYZ[2] * strength;
            }
         }
      }
         
      //
      // If the surface should be projected to a sphere
      //
      if (projectToSphereEveryXIterations > 0) {
         if ((k % projectToSphereEveryXIterations) == 0) {
            for (int j = 0; j < numNodes; j++) {
               MathUtilities::setVectorLength(&nodes[j*3], sphereRadius);
            }
         }
      }
      
      for (int i = 0; i < numNodes; i++) {
         nodeCoords[i*3]   = nodes[i*3];
         nodeCoords[i*3+1] = nodes[i*3+1];
         nodeCoords[i*3+2] = nodes[i*3+2];
         coordinates.setCoordinate(i, &nodes[i*3]);
      }    
      
      //
      // Update the displayed brain model
      //
      brainSet->drawBrainModel(this, k);
   }
   
   //for (int i = 0; i < numNodes; i++) {
   //   coordinates.setCoordinate(i, &nodeCoords[i*3]);
   //}
   
   delete[] nodes;
   delete[] nodeCoords;

   if (DebugControl::getDebugOn()) {
      std::cout << "Total time: " << (static_cast<float>(timer.elapsed()) / 1000.0) << std::endl;
   }
}

/**
 * Areal Smooth the surface based upon tile areas.
 * If "smoothOnlyTheseNodes" is not NULL and its number of elements is equal
 * to the number of nodes, only those nodes with *smoothOnlyTheseNodes[nodeNum]
 * will be smoothed.
 */
void
BrainModelSurface::arealSmoothing(const float strength, const int iterations,
                                  const int smoothEdgesEveryXIterations,
                                  const std::vector<bool>* smoothOnlyTheseNodes,
                                  const int projectToSphereEveryXIterations)
{
   PreferencesFile* pf = brainSet->getPreferencesFile();
   const int numThreads = pf->getMaximumNumberOfThreads();
   
   if (numThreads > 0) {
      BrainModelSurfaceSmoothing smoothObject(brainSet,
                                              this,
                                              BrainModelSurfaceSmoothing::SMOOTHING_TYPE_AREAL,
                                              strength,
                                              iterations,
                                              smoothEdgesEveryXIterations,
                                              0,
                                              smoothOnlyTheseNodes,
                                              NULL,
                                              projectToSphereEveryXIterations,
                                              numThreads);
      try {
         smoothObject.execute();
      }
      catch (BrainModelAlgorithmException&) {
      }
      
      return;
   }
   
/*
   appendToCoordinateFileComment("Areal Smoothing: ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(strength));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(iterations));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(smoothEdgesEveryXIterations));
   appendToCoordinateFileComment("\n");
*/

   //
   // Update node classification
   //
   brainSet->classifyNodes(getTopologyFile(), true);
   
   const float invStrength = 1.0 - strength;
   
   const int numNodes = getNumberOfNodes();
   
   if ((numNodes <= 0) || (topology->getNumberOfTiles() <= 0)) {
      return;
   }
   
   QTime timer;
   timer.start();
   const TopologyHelper* helper = topology->getTopologyHelper(false, true, true);
   if (DebugControl::getDebugOn()) {
      std::cout << "Topology Helper time: " << (static_cast<float>(timer.elapsed()) / 1000.0) << std::endl;
   }
   
   int* interiorNode = new int[numNodes];
   for (int i = 0; i < numNodes; i++) {
      const BrainSetNodeAttribute* bna = brainSet->getNodeAttributes(i);
      interiorNode[i] = (bna->getClassification() == 
                         BrainSetNodeAttribute::CLASSIFICATION_TYPE_INTERIOR);
   }
   
   //
   // See smoothing is limited to a subset of nodes
   //
   bool smoothSubsetOfNodes = false;
   if (smoothOnlyTheseNodes != NULL) {
      if (static_cast<int>(smoothOnlyTheseNodes->size()) == numNodes) {
         smoothSubsetOfNodes = true;
      }
   }
   
   //
   // Get the radius in the event that this is a sphere
   //
   const float sphereRadius = getSphericalSurfaceRadius();
   
   //
   // Reduce calls to coord file
   //
   float* nodeCoords = new float[numNodes * 3];
   float* nodes      = new float[numNodes * 3];
   for (int i = 0; i < numNodes; i++) {
      coordinates.getCoordinate(i, &nodeCoords[i*3]);
   }
   
   // Do 1 to iterations so that modulus check for edge smoothing works
   //
   for (int k = 1; k <= iterations; k++) {
      //std::vector<float> nodes(numNodes * 3);
      
      //
      // See if edges should be smoothed
      //
      bool smoothEdges = false;
      if (smoothEdgesEveryXIterations > 0) {
         if ((k % smoothEdgesEveryXIterations) == 0) {
            smoothEdges = true;
         }
      }
      
      for (int i = 0; i < numNodes; i++) {
         nodes[i*3]   = nodeCoords[i*3];
         nodes[i*3+1] = nodeCoords[i*3+1];
         nodes[i*3+2] = nodeCoords[i*3+2];
        // coordinates.getCoordinate(i, &nodes[i*3]);
      }
      
      for (int i = 0; i < numNodes; i++) {
         bool smoothIt = true;
         if (interiorNode[i] == false) {
            smoothIt = smoothEdges;
         }
         
         if (smoothIt && smoothSubsetOfNodes) {
            smoothIt = (*smoothOnlyTheseNodes)[i];
         }
         
         if (smoothIt) {   
            //std::vector<int> neighbors;
            //helper->getNodeNeighbors(i, neighbors);
            //const int numNeighbors = static_cast<int>(neighbors.size());
            
            int numNeighbors;
            const int* neighbors = helper->getNodeNeighbors(i, numNeighbors);
            
            if (numNeighbors > 1) {
               
               std::vector<float> tileAreas(numNeighbors);
               std::vector<float> tileCenters(numNeighbors*3);
               float totalArea = 0.0;
               for (int j = 0; j < numNeighbors; j++) {
               
                  //
                  // get 2 consecutive neighbors of this node
                  const int n1 = neighbors[j];
                  int next = j + 1;
                  if (next >= numNeighbors) {
                     next = 0;
                  }
                  const int n2 = neighbors[next];
               
                  //
                  // Area of the triangle
                  //
                  const float area = MathUtilities::triangleArea(&nodeCoords[i*3],
                                                               &nodeCoords[n1*3], &nodeCoords[n2*3]);
                  tileAreas[j] = area;
                  totalArea += area;
                  
                  //
                  // Save center of this tile
                  //
                  for (int k = 0; k < 3; k++) {
                     float p = (nodeCoords[i*3+k] + nodeCoords[n1*3+k] + nodeCoords[n2*3+k]) / 3.0;
                     tileCenters[j*3+k] = p;
                  }
               }
                  
               //
               // Compute the influence of the neighboring nodes
               //
               float xa = 0.0;
               float ya = 0.0;
               float za = 0.0;
               
               for (int j = 0; j < numNeighbors; j++) {
                  if (tileAreas[j] > 0.0) {
                     const float weight = tileAreas[j] / totalArea;
                     xa += weight * tileCenters[j*3];
                     ya += weight * tileCenters[j*3+1];
                     za += weight * tileCenters[j*3+2];
                  }
               }

               //
               // Update the nodes position
               //
               nodes[i*3]   = nodeCoords[i*3]   * invStrength + xa * strength;
               nodes[i*3+1] = nodeCoords[i*3+1] * invStrength + ya * strength;
               nodes[i*3+2] = nodeCoords[i*3+2] * invStrength + za * strength;
            }
         }
      }
         
      //
      // If the surface should be projected to a sphere
      //
      if (projectToSphereEveryXIterations > 0) {
         if ((k % projectToSphereEveryXIterations) == 0) {
            for (int j = 0; j < numNodes; j++) {
               MathUtilities::setVectorLength(&nodes[j*3], sphereRadius);
            }
         }
      }
      
      for (int i = 0; i < numNodes; i++) {
         nodeCoords[i*3]   = nodes[i*3];
         nodeCoords[i*3+1] = nodes[i*3+1];
         nodeCoords[i*3+2] = nodes[i*3+2];
         coordinates.setCoordinate(i, &nodes[i*3]);
      } 
      
      //
      // Update the displayed brain model
      //
      brainSet->drawBrainModel(this, k);
   }
   
   //for (int i = 0; i < numNodes; i++) {
   //   coordinates.setCoordinate(i, &nodeCoords[i*3]);
   //}
   
   delete[] nodes;
   delete[] nodeCoords;
   delete[] interiorNode;
   
   //if (DebugControl::getDebugOn()) {
      std::cout << "Total time: " << (static_cast<float>(timer.elapsed()) / 1000.0) << std::endl;
   //}
}

/**
 * Search topology to mark nodes to the specified depth.  "Depth" cycles are run and during
 * each cycle, if a node's "nodeFlags" is set, the neighbors of the node have their
 * "nodeFlags" set.  This is a non-recursive algorithm since recursion can be slow and 
 * since the call stack is small on OSX operating systems.
 */
void
BrainModelSurface::markNeighborNodesToDepth(std::vector<bool>& nodeFlags, const int depthIn)
{
   const int numNodes = getNumberOfNodes();
   if (numNodes <= 0) {
      return;
   }
   
   //
   // Get the topology helper
   //
   const TopologyFile* tf = getTopologyFile();
   const TopologyHelper* th = tf->getTopologyHelper(false, true, false);
   
   //
   // Flag that marks neighbors
   //
   std::vector<bool> neighborFlag(numNodes);
   
   //
   // Loop for specified depth
   //
   for (int depth = 0; depth < depthIn; depth++) {
      //
      // Clear neighbor flags
      //
      std::fill(neighborFlag.begin(), neighborFlag.end(), false);
      
      //
      // loop for each node
      //
      for (int i = 0; i < numNodes; i++) {
         //
         // If this node is flagged
         //
         if (nodeFlags[i]) {
            //
            // Mark its neighbors
            //
            int numNeighbors = 0;
            const int* neighbors = th->getNodeNeighbors(i, numNeighbors);
            for (int j = 0; j < numNeighbors; j++) {
               neighborFlag[neighbors[j]] = true;
            }
         }
      }
      
      //
      // Update node flags with neighbors that were just marked
      //
      for (int i = 0; i < numNodes; i++) {
         if (neighborFlag[i]) {
            nodeFlags[i] = true;
         }
      }
   }
}

/**
 * Smooth out crossovers in a surface
 */
void
BrainModelSurface::smoothOutSurfaceCrossovers(const float strength,
                                              const int numberOfCycles,
                                              const int iterationsPerCycle,
                                              const int smoothEdgesEveryXIterations,
                                              const int projectToSphereEveryXIterations,
                                              const int neighborDepth,
                                              const SURFACE_TYPES surfaceTypeHint)
{
   const int numNodes = getNumberOfNodes();
   if (numNodes <= 0) {
      return;
   }
   
   std::vector<bool> crossoverFlags(numNodes);
   
   //
   // Loop for the number of cycles
   //
   for (int i = 0; i < numberOfCycles; i++) {
      //
      // Get number of crossovers
      //
      int numNodeCrossovers = 0;
      int numTileCrossovers = 0;
      crossoverCheck(numTileCrossovers, numNodeCrossovers, surfaceTypeHint);
      
      //
      // See if there are crossovers
      //
      if (numNodeCrossovers > 0) {
         //
         // Find the nodes that are crossovers
         //
         std::fill(crossoverFlags.begin(), crossoverFlags.end(), false);
         for (int j = 0; j < numNodes; j++) {
            BrainSetNodeAttribute* bna = brainSet->getNodeAttributes(j);
            if (bna->getCrossover() == BrainSetNodeAttribute::CROSSOVER_YES) {
               crossoverFlags[j] = true;
            }
         }
         
         //
         // Mark the neighboring nodes to the specified depth
         //
         markNeighborNodesToDepth(crossoverFlags, neighborDepth);
         
         if (DebugControl::getDebugOn()) {
            int numXOver = 0;
            for (int k = 0; k < numNodes; k++) {
               if (crossoverFlags[k]) {
                  numXOver++;
               }
               BrainSetNodeAttribute* bna = brainSet->getNodeAttributes(k);
               if (crossoverFlags[k]) {
                  bna->setCrossover(BrainSetNodeAttribute::CROSSOVER_YES);
               }
               else {
                  bna->setCrossover(BrainSetNodeAttribute::CROSSOVER_NO);
               }
               
            }
            std::cout << "Crossover Smoothing: " << numXOver << " nodes will be smoothed." << std::endl;
         }
         
         //
         // Smooth the crossovers and surrounding nodes
         //
         arealSmoothing(strength, iterationsPerCycle, smoothEdgesEveryXIterations,
                        &crossoverFlags, projectToSphereEveryXIterations);
      }
      else {
         break;
      }
   }
   coordinates.clearDisplayList();
}

/**
 * (try to) smooth out flat surface overlap.  Returns true if any smoothing was done
 */
bool 
BrainModelSurface::smoothOutFlatSurfaceOverlap(const float strength,
                                               const int numberOfCycles,
                                               const int iterationsPerCycle,
                                               const int smoothEdgesEveryXIterations,
                                               const int neighborDepth)
{
   bool surfaceWasSmoothed = false;
   
   const int numNodes = getNumberOfNodes();
   if (numNodes <= 0) {
      return surfaceWasSmoothed;
   }
   
   //
   // Get the topology file and the number of tiles
   //
   TopologyFile* tf = getTopologyFile();
   const int numTiles = tf->getNumberOfTiles();
   if (numTiles <= 0) {
      return surfaceWasSmoothed;
   }
   
   //
   // Use a multiple of the mean distance between nodes for searching around node
   //
   float meanDist, minDist, maxDist;
   getMeanDistanceBetweenNodes(NULL, meanDist, minDist, maxDist);
   float longestLinkDistance =  meanDist * 3;
   if (DebugControl::getDebugOn()) {
      std::cout << "Longest link distance is " << longestLinkDistance << std::endl;
   }
   
   //
   // Topology helper to get tiles used by each node
   //
   const TopologyHelper* topologyHelper = topology->getTopologyHelper(false, true, false);
   
   //
   // Classify the nodes as interior and edges
   //
   brainSet->classifyNodes(tf);
   
   for (int cycles = 0; cycles < numberOfCycles; cycles++) {
      //
      // Flags overlapping nodes
      //
      std::vector<bool> overlapNodeFlag(numNodes, false);
   
      //
      // Create a point locator to find nodes
      //
      BrainModelSurfacePointLocator bmspl(this, true);
      
      //
      // Check all edge nodes
      //
      for (int i = 0; i < numNodes; i++) {
         if (brainSet->getNodeAttributes(i)->getClassification()
                     != BrainSetNodeAttribute::CLASSIFICATION_TYPE_INTERIOR) {
            const float* xyz = coordinates.getCoordinate(i);
            
            //
            // Find nodes near this node
            //
            std::vector<int> nearbyNodes;
            bmspl.getPointsWithinRadius(xyz, longestLinkDistance, nearbyNodes);
            if (DebugControl::getDebugNodeNumber() == i) {
               std::cout << "Node "
                         << i
                         << " has "
                         << nearbyNodes.size()
                         << " nearby neighbors"
                         << std::endl;
            }
            
            //
            // Find tiles used by the nearby nodes
            //
            std::set<int> tilesToCheck;
            const int numNearbyNodes = static_cast<int>(nearbyNodes.size());
            for (int j = 0 ; j < numNearbyNodes; j++) {
               const int node = nearbyNodes[j];
               if (node != i) {
                  std::vector<int> tiles;
                  topologyHelper->getNodeTiles(node, tiles);
                  tilesToCheck.insert(tiles.begin(), tiles.end());
               }
            }
            
            //
            // Check to see if node is on another tile indicating overlap
            //
            for (std::set<int>::iterator iter = tilesToCheck.begin();
               iter != tilesToCheck.end(); iter++) {
               const int tileNum = *iter;
               int v1, v2, v3;
               topology->getTile(tileNum, v1, v2, v3);
               if ((v1 != i) && (v2 != i) && (v3 != i)) {
                  const float area1 = MathUtilities::signedTriangleArea2D(coordinates.getCoordinate(v1),
                                                                        coordinates.getCoordinate(v2),
                                                                        xyz);
                  if (area1 > 0.0) {
                     const float area2 = MathUtilities::signedTriangleArea2D(coordinates.getCoordinate(v2),
                                                                           coordinates.getCoordinate(v3),
                                                                           xyz);
                     if (area2 > 0.0) {
                        const float area3 = MathUtilities::signedTriangleArea2D(coordinates.getCoordinate(v3),
                                                                              coordinates.getCoordinate(v1),
                                                                              xyz);
                        if (area3 > 0.0) {
                           overlapNodeFlag[i] = true;
                           if (DebugControl::getDebugOn()) {
                              std::cout << "Node " << i << " overlaps on tile " << tileNum
                                       << " with nodes " << v1 << " " << v2 << " " << v3 << std::endl;
                           }
                           break;
                        }
                     }
                  }
               }
            }
         }
      }   
      
      //
      // Mark the neighbors of overlap nodes
      //
      for (int depth = 0; depth < neighborDepth; depth++) {
         std::vector<bool> overlapNodeNeighborsFlag(numNodes, false);
         for (int i = 0; i < numNodes; i++) {
            if (overlapNodeFlag[i]) {
               int numNeighbors = 0;
               const int* neighbors = topologyHelper->getNodeNeighbors(i, numNeighbors);
               for (int j = 0; j < numNeighbors; j++) {
                  overlapNodeNeighborsFlag[neighbors[j]] = true;
               }
            }
         }
         for (int i = 0; i < numNodes; i++) {
            if (overlapNodeNeighborsFlag[i]) {
               overlapNodeFlag[i] = true;
            }
         }
      }
      
      //
      // Mark overlapped nodes as crossovers
      //
      int numMarked = 0;
      for (int i = 0; i < numNodes; i++) {
         brainSet->getNodeAttributes(i)->setCrossover(BrainSetNodeAttribute::CROSSOVER_NO);
         if (overlapNodeFlag[i]) {
            brainSet->getNodeAttributes(i)->setCrossover(BrainSetNodeAttribute::CROSSOVER_YES);
            numMarked++;
         }
      }
      if (DebugControl::getDebugOn()) {
         std::cout << "Num nodes flagged as overlap " << numMarked << std::endl;
      }
      
      if (numMarked > 0) {
         linearSmoothing(strength, iterationsPerCycle, smoothEdgesEveryXIterations, &overlapNodeFlag);
         //linearSmoothing(0.5, 3, 2);
         surfaceWasSmoothed = true;
      }
      else {
         break;
      }
   }
   
   if (surfaceWasSmoothed) {
     // appendToCoordinateFileComment("Smoothed out flat surface overlap\n");
   }
   coordinates.clearDisplayList();
   
   return surfaceWasSmoothed;
}

/// Keeps track of info for nodes for landmark neighbors
class NodeInfo {
   public:
      //
      // Three types of nodes
      //
      enum NODE_TYPE {
         NODE_NORMAL,
         NODE_LANDMARK,
         NODE_LANDMARK_NEIGHBOR 
      };

      float xyz[3];
      float offset[3];
      NODE_TYPE nodeType;
      int   numLandmarkNeighbors;
      
      NodeInfo() {
         xyz[0] = xyz[1] = xyz[2] = 0.0;
         offset[0] = offset[1] = offset[2] = 0;
         nodeType = NODE_NORMAL;
         numLandmarkNeighbors = 0;
      }
      
};

/**
 * Performed landmark constrained smoothing.  
 */
void 
BrainModelSurface::landmarkConstrainedSmoothing(const float strength, 
                                                const int iterations, 
                                                const std::vector<bool>& landmarkNodeFlag,
                                                const int projectToSphereEveryXIterations)
{  
   PreferencesFile* pf = brainSet->getPreferencesFile();
   const int numThreads = pf->getMaximumNumberOfThreads();
   if (numThreads > 0) {
      BrainModelSurfaceSmoothing smoothObject(brainSet,
                                              this,
                                              BrainModelSurfaceSmoothing::SMOOTHING_TYPE_LANDMARK_CONSTRAINED,
                                              strength,
                                              iterations,
                                              0,
                                              0,
                                              NULL,
                                              &landmarkNodeFlag,
                                              projectToSphereEveryXIterations,
                                              numThreads);
      try {
         smoothObject.execute();
      }
      catch (BrainModelAlgorithmException&) {
      }
      
      return;
   }
   
   const float inverseStrength = 1.0 - strength;
   
   const int numNodes = getNumberOfNodes();
   const TopologyHelper* helper = topology->getTopologyHelper(false, true, true);

   std::vector<NodeInfo> nodeInfo(numNodes);
   
   float scale = 0.0;
   bool  haveScale = false;
      
   appendToCoordinateFileComment("Landmark Constrained Smoothing: ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(strength));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(iterations));
   appendToCoordinateFileComment("\n");

   //
   // Set node info for all nodes
   //
   for (int i = 0; i < numNodes; i++) {
      //
      // Is this node a landmarkd
      //
      if (landmarkNodeFlag[i]) {
         nodeInfo[i].nodeType = NodeInfo::NODE_LANDMARK;
      }
      else {
         //
         // See if any of nodes neighbors are landmarks
         //
         std::vector<int> neighbors;
         helper->getNodeNeighbors(i, neighbors);
         for (int j = 0; j < static_cast<int>(neighbors.size()); j++) {
            const int n = neighbors[j];
            if (landmarkNodeFlag[n]) {
               nodeInfo[i].numLandmarkNeighbors++;
            }
            else {
               if (haveScale == false) {
                  haveScale = true;
                  scale = MathUtilities::distance3D(coordinates.getCoordinate(i),
                                                    coordinates.getCoordinate(n))  /  std::sqrt(3.0);
               }
            }
         }
         if (nodeInfo[i].numLandmarkNeighbors > 0) {
            nodeInfo[i].nodeType = NodeInfo::NODE_LANDMARK_NEIGHBOR;
         }
      }
   }
   
   //
   // smooth for specified number of iterations
   //
   for (int iter = 0; iter < iterations; iter++) {
      
      for (int i = 0; i < numNodes; i++) {
         //
         // Get the coordinate
         //
         coordinates.getCoordinate(i, nodeInfo[i].xyz);
         
         //
         // Get the neighbors for this node
         //
         //std::vector<int> neighbors;
         //helper->getNodeNeighbors(i, neighbors);
         //const int numNeighbors = static_cast<int>(neighbors.size());
         int numNeighbors;
         const int* neighbors = helper->getNodeNeighbors(i, numNeighbors);
         const float floatNumNeighbors = numNeighbors;
         
         //
         // Ignore nodes without neighbors and ignore landmark nodes
         //
         if ((numNeighbors > 1) && (nodeInfo[i].nodeType != NodeInfo::NODE_LANDMARK)) {
               
            //
            // Get average of neighbor coordinates
            //
            float sumx = 0.0, sumy = 0.0, sumz = 0.0;
            for (int k = 0; k < numNeighbors; k++) {
               const float* neighXYZ = coordinates.getCoordinate(neighbors[k]);
               sumx += neighXYZ[0];
               sumy += neighXYZ[1];
               sumz += neighXYZ[2];
            }
            float meanX = sumx / floatNumNeighbors;
            float meanY = sumy / floatNumNeighbors;
            float meanZ = sumz / floatNumNeighbors;
            
            //
            // Is this a node neither landmark nor landmark neighbor
            //
            if (nodeInfo[i].nodeType == NodeInfo::NODE_NORMAL) {
               //
               // Normal smoothing
               //
               nodeInfo[i].xyz[0] = nodeInfo[i].xyz[0] * inverseStrength
                                 + strength * meanX;
               nodeInfo[i].xyz[1] = nodeInfo[i].xyz[1] * inverseStrength
                                 + strength * meanY;
               nodeInfo[i].xyz[2] = nodeInfo[i].xyz[2] * inverseStrength
                                 + strength * meanZ;
            }
            else {
               //
               // To get here node must be a neighbor of a landmark node
               //
               for (int k = 0; k < numNeighbors; k++) {
                  //
                  // Is neighbor a landmark node ?
                  //
                  const int neigh = neighbors[k];
                  if (nodeInfo[neigh].nodeType == NodeInfo::NODE_LANDMARK) {
                     //
                     // Get the next and previous neigbors of this landmark node
                     //
                     int prevNeighIndex = k - 1;
                     if (prevNeighIndex < 0) {
                        prevNeighIndex = numNeighbors - 1;
                     }
                     const int neighA = neighbors[prevNeighIndex];
                     int nextNeighIndex = k + 1;
                     if (nextNeighIndex >= numNeighbors) {
                        nextNeighIndex = 0;
                     }
                     const int neighB = neighbors[nextNeighIndex];
                     
                     //
                     // Get coordinates of this landmark neighbor and next and previous neighbors
                     //
                     float ai[3];
                     coordinates.getCoordinate(neighA, ai);
                     float bi[3];
                     coordinates.getCoordinate(neighB, bi);
                     float li[3];
                     coordinates.getCoordinate(neigh, li);

                     //
                     // Adjust the position of neighbor average
                     //
                     float p[3];
                     p[0] = 2*li[0] - ai[0] - bi[0];
                     p[1] = 2*li[1] - ai[1] - bi[1];
                     p[2] = 2*li[2] - ai[2] - bi[2];
                     
                     float len = std::sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
                     p[0] /= len;
                     p[1] /= len;
                     p[2] /= len;
                     
                     meanX += li[0] + scale * p[0];
                     meanY += li[1] + scale * p[1];
                     meanZ += li[2] + scale * p[2];
                  }
               }
               
               meanX /= (nodeInfo[i].numLandmarkNeighbors + 1);
               meanY /= (nodeInfo[i].numLandmarkNeighbors + 1);
               meanZ /= (nodeInfo[i].numLandmarkNeighbors + 1);
               nodeInfo[i].xyz[0] = inverseStrength * nodeInfo[i].xyz[0]
                                  + strength * meanX;
               nodeInfo[i].xyz[1] = inverseStrength * nodeInfo[i].xyz[1]
                                  + strength * meanY;
               nodeInfo[i].xyz[2] = inverseStrength * nodeInfo[i].xyz[2]
                                  + strength * meanZ;
            }
         }
      }
      
      //
      // Set the coordinates
      //
      for (int i = 0; i < numNodes; i++) {
         coordinates.setCoordinate(i, nodeInfo[i].xyz);
      }
      
      //
      // Update the displayed brain model
      //
      brainSet->drawBrainModel(this, iter);
   }                 
   coordinates.clearDisplayList();
}

/**
 * Performed landmark neighbor constrained smoothing.  
 */
void 
BrainModelSurface::landmarkNeighborConstrainedSmoothing(const float strength, 
                                                        const int iterations, 
                                                        const std::vector<bool>& landmarkNodeFlag,
                                                        const int smoothNeighborsEveryX,
                                                        const int projectToSphereEveryXIterations)
{  
   PreferencesFile* pf = brainSet->getPreferencesFile();
   int numThreads = pf->getMaximumNumberOfThreads();
//   numThreads = 0;
//   if (numThreads > 0) {
//      BrainModelSurfaceSmoothing smoothObject(brainSet,
//                                              this,
//                                              BrainModelSurfaceSmoothing::SMOOTHING_TYPE_LANDMARK_NEIGHBOR_CONSTRAINED,
//                                              strength,
//                                              iterations,
//                                              0,
//                                              smoothNeighborsEveryX,
//                                              NULL,
//                                              &landmarkNodeFlag,
//                                              projectToSphereEveryXIterations,
//                                              numThreads);
//      try {
//         smoothObject.execute();
//      }
//      catch (BrainModelAlgorithmException&) {
//      }
//
//      return;
//   }

   const float sphereRadius = getSphericalSurfaceRadius();
   
   const float inverseStrength = 1.0 - strength;
   
   const int numNodes = getNumberOfNodes();
   const TopologyHelper* helper = topology->getTopologyHelper(false, true, true);

   std::vector<NodeInfo> nodeInfo(numNodes);
   
   appendToCoordinateFileComment("Landmark Neighbor Constrained Smoothing: ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(strength));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(iterations));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(smoothNeighborsEveryX));
   appendToCoordinateFileComment("\n");
   
   //
   // Set node info for all nodes
   //
   for (int i = 0; i < numNodes; i++) {
      //
      // Get neighbors for this node
      //
      //std::vector<int> neighbors;
      //helper->getNodeNeighbors(i, neighbors);
      //const int numNeighbors = static_cast<int>(neighbors.size());
      int numNeighbors;
      const int* neighbors = helper->getNodeNeighbors(i, numNeighbors);
      const float floatNumNeighbors = numNeighbors;

      //
      // Is this node a landmarkd
      //
      if (landmarkNodeFlag[i]) {
         nodeInfo[i].nodeType = NodeInfo::NODE_LANDMARK;
         
         //
         // Calculate offset of this landmark node from the average of neighbors
         //
         float neighborSum[3] = { 0.0, 0.0, 0.0 };
         for (int j = 0; j < numNeighbors; j++) {
            const float* xyz = coordinates.getCoordinate(neighbors[j]);
            neighborSum[0] += xyz[0];
            neighborSum[1] += xyz[1];
            neighborSum[2] += xyz[2];
         }
         const float* xyz = coordinates.getCoordinate(i);
         nodeInfo[i].offset[0] = xyz[0] - (neighborSum[0] / floatNumNeighbors);
         nodeInfo[i].offset[1] = xyz[1] - (neighborSum[1] / floatNumNeighbors);
         nodeInfo[i].offset[2] = xyz[2] - (neighborSum[2] / floatNumNeighbors);
      }
      else {
         //
         // See if any of nodes neighbors are landmarks
         //
         for (int j = 0; j < numNeighbors; j++) {
            const int n = neighbors[j];
            if (landmarkNodeFlag[n]) {
               nodeInfo[i].numLandmarkNeighbors++;
            }
         }
         if (nodeInfo[i].numLandmarkNeighbors > 0) {
            nodeInfo[i].nodeType = NodeInfo::NODE_LANDMARK_NEIGHBOR;
         }
      }
   }
   
   //
   // Apply offsets to landmark neighbors
   //
   for (int i = 0; i < numNodes; i++) {
      if (nodeInfo[i].nodeType == NodeInfo::NODE_LANDMARK_NEIGHBOR) {
         //
         // Get neighbors for this node
         //
         //std::vector<int> neighbors;
         //helper->getNodeNeighbors(i, neighbors);
         //const int numNeighbors = static_cast<int>(neighbors.size());
         int numNeighbors;
         const int* neighbors = helper->getNodeNeighbors(i, numNeighbors);
         
         //
         // Get average of landmark neighbor offsets
         //
         float avg[3] = { 0.0, 0.0, 0.0 };
         for (int j = 0; j < numNeighbors; j++) {
            const int n = neighbors[j];
            if (nodeInfo[n].nodeType == NodeInfo::NODE_LANDMARK) {
               avg[0] += nodeInfo[n].offset[0];
               avg[1] += nodeInfo[n].offset[1];
               avg[2] += nodeInfo[n].offset[2];
            }
         }
         
         //
         // Offset this landmark neighbor
         //
         const float floatNumLandmarkNeighbors = nodeInfo[i].numLandmarkNeighbors;
         float xyz[3];
         coordinates.getCoordinate(i, xyz);
         xyz[0] += (avg[0] / floatNumLandmarkNeighbors);
         xyz[1] += (avg[1] / floatNumLandmarkNeighbors);
         xyz[2] += (avg[2] / floatNumLandmarkNeighbors);
         coordinates.setCoordinate(i, xyz);
      }
   }
   
   //
   // smooth for specified number of iterations
   //
   int smoothNeighborCounter = 1;
   for (int iter = 1; iter <= iterations; iter++) {
      
      for (int i = 0; i < numNodes; i++) {
         //
         // Get the coordinate
         //
         coordinates.getCoordinate(i, nodeInfo[i].xyz);
         
         //
         // Get the neighbors for this node
         //
         //std::vector<int> neighbors;
         //helper->getNodeNeighbors(i, neighbors);
         //const int numNeighbors = static_cast<int>(neighbors.size());
         int numNeighbors;
         const int* neighbors = helper->getNodeNeighbors(i, numNeighbors);
         const float floatNumNeighbors = numNeighbors;
         
         //
         // Only smooth nodes with neighbors
         //
         if (numNeighbors > 0) {
            //
            // Do not smooth landmark nodes
            // smooth landmark neighbors every "smoothNeighborsEveryX" iterations
            // smooth normal nodes every iteration
            //
            if ((nodeInfo[i].nodeType == NodeInfo::NODE_NORMAL) ||
                ((nodeInfo[i].nodeType == NodeInfo::NODE_LANDMARK_NEIGHBOR) &&
                 (smoothNeighborCounter == smoothNeighborsEveryX))) {
               //
               // Get average of neighbors coordinates
               //
               float neighborAverage[3] = { 0.0, 0.0, 0.0 };
               for (int j = 0; j < numNeighbors; j++) {
                  const int n = neighbors[j];
                  const float* xyz = coordinates.getCoordinate(n);
                  neighborAverage[0] += xyz[0];
                  neighborAverage[1] += xyz[1];
                  neighborAverage[2] += xyz[2];
               }
               neighborAverage[0] /= floatNumNeighbors;
               neighborAverage[1] /= floatNumNeighbors;
               neighborAverage[2] /= floatNumNeighbors;
               
               //
               // Smooth the node
               //
               nodeInfo[i].xyz[0] = nodeInfo[i].xyz[0] * inverseStrength
                                  + strength * neighborAverage[0];
               nodeInfo[i].xyz[1] = nodeInfo[i].xyz[1] * inverseStrength
                                  + strength * neighborAverage[1];
               nodeInfo[i].xyz[2] = nodeInfo[i].xyz[2] * inverseStrength
                                  + strength * neighborAverage[2];
            }
         }
      }
         
      //
      // Update the neighbor smoothing counter
      //
      if (smoothNeighborCounter >= smoothNeighborsEveryX) {
         smoothNeighborCounter = 1;
      }
      else {
         smoothNeighborCounter++;
      }
      
      //
      // If the surface should be projected to a sphere
      //
      if (projectToSphereEveryXIterations > 0) {
         if ((iter % projectToSphereEveryXIterations) == 0) {
            for (int j = 0; j < numNodes; j++) {
               MathUtilities::setVectorLength(nodeInfo[j].xyz, sphereRadius);
            }
         }
      }

      //
      // Set the coordinates
      //
      for (int i = 0; i < numNodes; i++) {
         coordinates.setCoordinate(i, nodeInfo[i].xyz);
      }
      
      //
      // If the surface should be projected to a sphere
      //
      //if (projectToSphereEveryXIterations > 0) {
      //   if ((iter % projectToSphereEveryXIterations) == 0) {
      //      convertToSphereWithRadius(sphereRadius);
      //   }
      //}
      
      //
      // Update the displayed brain model
      //
      brainSet->drawBrainModel(this, iter);
   }                 
   coordinates.clearDisplayList();
}

/**
 * Convert to a sphere with the specified area.
 * Disconnected nodes are placed at origin.
 * Assumes surface is already centered at origin.
 */
void
BrainModelSurface::convertToSphereWithSurfaceArea(const float desiredSphereAreaIn)
{
   float desiredSphereArea = desiredSphereAreaIn;
   
   //
   // If area not set just use the surface area
   //
   if (desiredSphereArea <= 0.0) {
      desiredSphereArea = getSurfaceArea();
   }
   
   //
   // Area sphere is 4 * PI * Radius * Radius
   //
   const float radius = std::sqrt(desiredSphereArea / (4.0 * M_PI));
   
   //appendToCoordinateFileComment("Convert to sphere with area: ");
   //appendToCoordinateFileComment(StringUtilities::fromNumber(desiredSphereAreaIn));
   //appendToCoordinateFileComment("\n");
   
   convertToSphereWithRadius(radius);
}

/**
 * Convert to a sphere with the specified radius.
 * Disconnected nodes are placed at origin.
 * Assumes surface is already centered at origin.
 */
void
BrainModelSurface::convertToSphereWithRadius(const float radius,
                                             const int startNodeIndexIn,
                                             const int endNodeIndexIn)
{
   const int numNodes = getNumberOfNodes();
   const TopologyHelper* helper = topology->getTopologyHelper(false, true, false);
   
   //
   // Set start and end nodes if needed
   //
   int startNodeIndex = startNodeIndexIn;
   if (startNodeIndex < 0) {
      startNodeIndex = 0;
   }
   int endNodeIndex = endNodeIndexIn;
   if (endNodeIndex < 0) {
      endNodeIndex = numNodes;
   }
   
   for (int i = startNodeIndex; i < endNodeIndex; i++) {
      float p[3];
      if (helper->getNodeHasNeighbors(i)) {
         coordinates.getCoordinate(i, p);
         const float prad = std::sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
         if (prad > 0.0) {
            const float scale = radius / prad;
            p[0] *= scale;
            p[1] *= scale;
            p[2] *= scale;
         }
      }
      else {
         p[0] = p[1] = p[2] = 0.0;
      }
      coordinates.setCoordinate(i, p);
   }
   
   //
   // set the surface type to a sphere
   //
   setSurfaceType(SURFACE_TYPE_SPHERICAL);
   
   //
   // Reset the viewing transformations
   //
   resetViewingTransformations();
   
//   appendToCoordinateFileComment("Convert to sphere with radius: ");
//   appendToCoordinateFileComment(StringUtilities::fromNumber(radius));
//   appendToCoordinateFileComment("\n");
}

/**
 * move the surface so that the midpoint of the surface is at the origin.
 */
void 
BrainModelSurface::translateMidpointToOrigin()
{
   /*
    * Get bounds
    */
   float bounds[6];
   this->getBounds(bounds);
   
   /*
    * Get center of sphere
    */
   float center[] = {
      ((bounds[0] + bounds[1]) / 2.0f),
      ((bounds[2] + bounds[3]) / 2.0f),
      ((bounds[4] + bounds[5]) / 2.0f)
   };
   
   /*
    * Set translation
    */
   TransformationMatrix tm;
   tm.translate(-center[0], -center[1], -center[2]);

   this->applyTransformationMatrix(tm);
}

/**
 * Convert an ellipsoidal surface to a sphere (if area is zero the ellipsoidal area will be used).
 */
void
BrainModelSurface::convertEllipsoidToSphereWithSurfaceArea(const float desiredSphereAreaIn)
{
   //
   // Get a topology helper
   //
   const TopologyHelper* th = topology->getTopologyHelper(false, true, false);
   
   //
   // Determine surface area if necessary
   //
   float desiredSurfaceArea = desiredSphereAreaIn;
   if (desiredSurfaceArea <= 0.0) {
      desiredSurfaceArea = getSurfaceArea();
   }
   
   //
   // Determine radius of the output sphere.
   // Note: Sphere surface area = 4 * PI * Radius * Radius
   //
   const float sphereRadius = std::sqrt(desiredSurfaceArea / (4.0 * M_PI));
   
   //
   // Determine lengths of the axes
   //
   float bounds[6];
   coordinates.getBounds(bounds);
   const float A = (fabs(bounds[0]) + fabs(bounds[1])) * 0.5;
   const float B = (fabs(bounds[2]) + fabs(bounds[3])) * 0.5;
   const float C = (fabs(bounds[4]) + fabs(bounds[5])) * 0.5;
   
   //
   // Convert the coordinates from ellipsoid to sphere
   //
   const float aSquared = A * A;
   const float bSquared = B * B;
   const float cSquared = C * C;
   const int numCoords = getNumberOfNodes();
   for (int i = 0; i < numCoords; i++) {
      float xyz[3] = { 0.0, 0.0, 0.0 };
      if (th->getNodeHasNeighbors(i)) {
         coordinates.getCoordinate(i, xyz);
         
         //
         // "unitize" the ellipsoidal coordinates
         //
         //  x*x   y*y   z*z
         //  --- + --- + --- = 1.0
         //  A*A   B*B   C*C
         //
         const float t1 = (xyz[0]*xyz[0]) / (aSquared);
         const float t2 = (xyz[1]*xyz[1]) / (bSquared);
         const float t3 = (xyz[2]*xyz[2]) / (cSquared);
         const float f = std::sqrt(t1 + t2 + t3);
         if (f != 0.0) {
            xyz[0] /= f;
            xyz[1] /= f;
            xyz[2] /= f;
         }
         
         //
         // Push coordinate onto the sphere
         //
         xyz[0] = (sphereRadius * xyz[0]) / A;
         xyz[1] = (sphereRadius * xyz[1]) / B;
         xyz[2] = (sphereRadius * xyz[2]) / C;
      }
      coordinates.setCoordinate(i, xyz);
   }
   
   //
   // set the surface type to a sphere
   //
   setSurfaceType(SURFACE_TYPE_SPHERICAL);
   
   //
   // Reset the viewing transformations
   //
   resetViewingTransformations();

   appendToCoordinateFileComment("Convert to ellipoid with area: ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(desiredSphereAreaIn));
   appendToCoordinateFileComment("\n");
}

/**
 * convert to an ellipsoid surface.
 */
void 
BrainModelSurface::convertToEllipsoid()
{
   //
   // Get a topology helper
   //
   const TopologyHelper* th = topology->getTopologyHelper(false, true, false);
   
   //
   // Translate the surface to its center of mass
   //
   translateToCenterOfMass();
   
   //
   // Determine lengths of the axes
   //
   float bounds[6];
   coordinates.getBounds(bounds);
   const float A = (fabs(bounds[0]) + fabs(bounds[1])) * 0.5;
   const float B = (fabs(bounds[2]) + fabs(bounds[3])) * 0.5;
   const float C = (fabs(bounds[4]) + fabs(bounds[5])) * 0.5;
   const float aSquared = A * A;
   const float bSquared = B * B;
   const float cSquared = C * C;

   //
   // "normalize" the ellipsoidal coordinates
   //
   const int numCoords = getNumberOfNodes();
   for (int i = 0; i < numCoords; i++) {
      float xyz[3] = { 0.0, 0.0, 0.0 };
      if (th->getNodeHasNeighbors(i)) {
         coordinates.getCoordinate(i, xyz);
         
         //  x*x   y*y   z*z
         //  --- + --- + --- = 1.0
         //  A*A   B*B   C*C
         //
         const float t1 = (xyz[0]*xyz[0]) / (aSquared);
         const float t2 = (xyz[1]*xyz[1]) / (bSquared);
         const float t3 = (xyz[2]*xyz[2]) / (cSquared);
         const float f = std::sqrt(t1 + t2 + t3);
         if (f != 0.0) {
            xyz[0] /= f;
            xyz[1] /= f;
            xyz[2] /= f;
         }
      }
      coordinates.setCoordinate(i, xyz);
   }
   
   //
   // Scale the surface to its original major axis size
   //
   coordinates.getBounds(bounds);
   const float newA = (fabs(bounds[0]) + fabs(bounds[1])) * 0.5;
   if (newA > 0.0) {
      const float scaleFactor = A / newA;
      TransformationMatrix tm;
      tm.scale(scaleFactor, scaleFactor, scaleFactor);
   }
   
   setSurfaceType(SURFACE_TYPE_ELLIPSOIDAL);

   appendToCoordinateFileComment("Convert to ellipsoid.");
}

/**
 * Rotate a surface so that the node is on the positive Z-axis (facing user).
 */
void 
BrainModelSurface::orientNodeToPositiveScreenZ(const int nodeNumber, 
                                               const int surfaceViewNumber)
{
   bool flatFlag = false;
   switch (surfaceType) {
      case SURFACE_TYPE_RAW:
      case SURFACE_TYPE_FIDUCIAL:
      case SURFACE_TYPE_INFLATED:
      case SURFACE_TYPE_VERY_INFLATED:
      case SURFACE_TYPE_SPHERICAL:
      case SURFACE_TYPE_ELLIPSOIDAL:
      case SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         break;
      case SURFACE_TYPE_FLAT:
      case SURFACE_TYPE_FLAT_LOBAR:
         flatFlag = true;
         break;
      case SURFACE_TYPE_HULL:
      case SURFACE_TYPE_UNKNOWN:
      case SURFACE_TYPE_UNSPECIFIED:
         break;
   }
   
   const int numNodes = getNumberOfNodes();
   if ((nodeNumber > 0) && (nodeNumber < numNodes)) {
      //
      // Get the node's coordinate
      //
      float vect[3];
      coordinates.getCoordinate(nodeNumber, vect);
      
      if (flatFlag) {
         setToStandardView(surfaceViewNumber, VIEW_RESET);
         setTranslation(surfaceViewNumber, vect);
      }
      else {
         //
         // Normalize the point
         //
         MathUtilities::normalize(vect);
         
         // assume a vector goes from the  center of gravity in the
         // positive z direction so it would be something like (0, 0, 1).
         // The dot product of the positive Z vector and "vect" is just vect[2]
         // since the x & y components of the vector down positive Z are (0, 0).
         // The arc-cosine of the dot product is the angle between the
         // two vectors.
         const float angle = std::acos(vect[2]) * MathUtilities::radiansToDegrees();

         // vector perpendicular (90 degrees ccw) to the vect in X/Y plane
         double perp[3];
         perp[0] = -vect[1];
         perp[1] = vect[0];
         perp[2] = 0.0;
         MathUtilities::normalize(perp);

         //
         // Create a transformation matrix for the rotation and apply it to the surface
         //
         TransformationMatrix tm;
         tm.rotate(angle, perp);
         float matrix[16];
         tm.getMatrix(matrix);
         setRotationMatrix(surfaceViewNumber, matrix);
      }
   }
}

/**
 * compress the front face of a surface.
 */
void 
BrainModelSurface::compressFrontFace(const float compressionFactor,
                                     const int surfaceViewNumber)
{
   //
   // Save current rotation matrix
   //
   //float matrix[16];
   //getRotationMatrix(surfaceViewNumber, matrix);
   
   //
   // Apply the current rotation matrix
   //
   TransformationMatrix rot;
   rot.setMatrix(rotationMatrix[surfaceViewNumber]);
   rot.rotate(TransformationMatrix::ROTATE_Y_AXIS, 180.0);  // backface gets rotated
   rot.transpose();  // I'm not sure why this works JWH perhaps OpenGL backwards from VTK.
   applyTransformationMatrix(rot);
   
   //
   // Compress the front face
   // 
   const SURFACE_TYPES st = getSurfaceType();
   convertSphereToCompressedMedialWall(compressionFactor);
   setSurfaceType(st);
   
   //
   // Apply the current rotation matrix
   //
   rot.identity();
   rot.rotate(TransformationMatrix::ROTATE_Y_AXIS, 180.0);  // backface gets rotated
   rot.transpose();  // I'm not sure why this works JWH perhaps OpenGL backwards from VTK.
   applyTransformationMatrix(rot);
   
   //
   // Set to default view
   //
   setToStandardView(surfaceViewNumber, BrainModelSurface::VIEW_RESET);

   //
   // Apply inverse of rotation matrix
   //
   //rot.transpose();
   //applyTransformationMatrix(rot);
   
   //
   // Restore rotation matrix
   //
   //setRotationMatrix(surfaceViewNumber, matrix);

   coordinates.clearDisplayList();
}

/**
 * convert a sphere to a compressed medial wall surface.
 */
void 
BrainModelSurface::convertSphereToCompressedMedialWall(const float compressionFactor)
{
   const float piDiv2 = M_PI / 2.0;
   
   //
   // Get a topology helper
   //
   const TopologyHelper* th = topology->getTopologyHelper(false, true, false);
   
   //
   // get the spherical surface radius
   //
   const float radius = getSphericalSurfaceRadius();
   
   //
   // Compress the sphere's nodes
   //
   const int numNodes = getNumberOfNodes();
   for (int i = 0; i < numNodes; i++) {
      float xyz[3] = { 0.0, 0.0, 0.0 };
      if (th->getNodeHasNeighbors(i)) {
         
         //
         // Get the coordinate and its distance from the sphere center
         //
         coordinates.getCoordinate(i, xyz);
         
         //
         // Normalize the spherical coordinate
         //
         const float length = MathUtilities::normalize(xyz);
         if (length > 0.0) {
            //
            // Convert cartesian coordinates to spherical coordinates
            //
            double phi = std::acos(xyz[2]);
            const double theta = std::atan2(xyz[1], xyz[0]);
            
            //
            // Compress the node around the sphere
            //
            if (phi < (piDiv2 * compressionFactor)) {
               phi /= compressionFactor;
            }
            else {
               phi = (phi + M_PI * (1.0 - compressionFactor)) / (2.0 - compressionFactor);
            }
            
            //
            // Convert spherical coordinates back to cartesian
            //
            xyz[0] = radius * std::cos(theta) * std::sin(phi);
            xyz[1] = radius * std::sin(theta) * std::sin(phi);
            xyz[2] = radius * std::cos(phi);
         }
      }
      //
      // Replace the coordinate
      //
      coordinates.setCoordinate(i, xyz);
   }
   
   setSurfaceType(SURFACE_TYPE_COMPRESSED_MEDIAL_WALL);
   
   appendToCoordinateFileComment("Convert to compressed medial wall with compression factor: ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(compressionFactor));
   appendToCoordinateFileComment("\n");
}
      
/**
 * Convert a spherical surface to flat.
 */
void
BrainModelSurface::convertSphereToFlat()
{
   const float piDiv2 = M_PI / 2.0;
   //
   // Get a topology helper
   //
   const TopologyHelper* th = topology->getTopologyHelper(false, true, false);
   
   const int numNodes = getNumberOfNodes();
   for (int i = 0; i < numNodes; i++) {
      float x = 0.0, y = 0.0, z = 0.0;
      if (th->getNodeHasNeighbors(i)) {
         
         //
         // Get the coordinate and its distance from the sphere center
         //
         coordinates.getCoordinate(i, x, y, z);
         const float radius = std::sqrt(x*x + y*y + z*z);
         if (radius > 0.0) {
            //
            //
            //
            float zmult = 1.0;
            if (z <= 0.0) {
               const float t1 = (z*z) / (radius*radius);
               zmult = std::pow((1.0 - t1), -0.25);
            }
            const float rad = radius * zmult * std::acos(z / radius);
            float phi;
            if (fabs(y) > 0.00001) {
               phi = std::atan(x/y);
            }
            else {
               phi = piDiv2;
            }
            const float u = (x / fabs(x)) * rad * fabs(std::sin(phi));
            const float v = (y / fabs(y)) * rad * fabs(std::cos(phi));
            
            x = u;
            y = v;
            z = 0.0;
         }
      }
      coordinates.setCoordinate(i, x, y, z);
   }
   
   //
   // set the surface type to a sphere
   //
   setSurfaceType(SURFACE_TYPE_FLAT);
   
   //
   // Reset the viewing transformations
   //
   resetViewingTransformations();
   
   appendToCoordinateFileComment("Convert Sphere to Flat\n");
   
   //
   // Surface is cartesian non-standard
   //
   coordinates.setHeaderTag(AbstractFile::headerTagCoordFrameID, "CN");
}

/**
 * apply surface shape to a surface (typically a flat or spherical surface).
 */
void 
BrainModelSurface::applyShapeToSurface(const SurfaceShapeFile& ssf,
                                       const int shapeColumn,
                                       const float shapeMultiplier)
{
   //
   // Make sure shape data valid
   //
   const int numNodes = getNumberOfNodes();
   if (numNodes != ssf.getNumberOfNodes()) {
      return;
   }
   if ((shapeColumn < 0) || (shapeColumn >= ssf.getNumberOfColumns())) {
      return;
   }
   
   //
   // Get radius of sphere
   //
   const float radius = getSphericalSurfaceRadius();
   
   //
   // Apply relief to nodes
   //
   for (int i = 0; i < numNodes; i++) {
      float xyz[3];
      coordinates.getCoordinate(i, xyz);
      
      const float offset = ssf.getValue(i, shapeColumn) * shapeMultiplier;
      switch (surfaceType) {
         case SURFACE_TYPE_RAW:
            break;
         case SURFACE_TYPE_FIDUCIAL:
            break;
         case SURFACE_TYPE_INFLATED:
            break;
         case SURFACE_TYPE_VERY_INFLATED:
            break;
         case SURFACE_TYPE_SPHERICAL:
            if (radius > 0.0) {
               const float scale = (radius + offset) / radius;
               xyz[0] *= scale;
               xyz[1] *= scale;
               xyz[2] *= scale;
            }
            break;
         case SURFACE_TYPE_ELLIPSOIDAL:
            break;
         case SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
            break;
         case SURFACE_TYPE_FLAT:
            xyz[2] += offset;
            break;
         case SURFACE_TYPE_FLAT_LOBAR:
            xyz[2] += offset;
            break;
         case SURFACE_TYPE_HULL:
            break;
         case SURFACE_TYPE_UNKNOWN:
            break;
         case SURFACE_TYPE_UNSPECIFIED:
            break;
      }
      
      coordinates.setCoordinate(i, xyz);
   }
   
   //
   // Update normals
   //
   computeNormals();
}
                               
/**
 * inflate the surface
 */
void 
BrainModelSurface::inflate(const int smoothingIterations,
                           const int inflationIterations,
                           const float inflationFactorIn)
{
   appendToCoordinateFileComment("Inflated: ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(smoothingIterations));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(inflationIterations));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(inflationFactorIn));
   appendToCoordinateFileComment("\n");
   
   //
   // See if a progress dialog should be displayed
   //
   QProgressDialog* progressDialog = NULL;
   QWidget* progressDialogParent = brainSet->getProgressDialogParent();
   if (progressDialogParent != NULL) {
      progressDialog = new QProgressDialog("Inflate Surface",
                                           "Cancel",
                                           0,
                                           inflationIterations + 1,
                                           progressDialogParent);
      progressDialog->setWindowTitle("Inflate Surface");
      progressDialog->setValue(0);
      progressDialog->show();
   }
   
   const float inflationFactor = inflationFactorIn; // * 0.01;
   
   //
   // Move center of mass to origin
   //
   translateToCenterOfMass();
   
   //
   // Find node furthest from the origin
   //
   const int numNodes = getNumberOfNodes();
   float radius = 0.0;
   for (int i = 0; i < numNodes; i++) {
      const float* xyz = coordinates.getCoordinate(i);
      const float dist = xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2];
      radius = std::max(radius, dist);
   }
   radius = std::sqrt(radius);
   
   //
   // Get number of smoothing iterations per inflation iterations
   //
   const int numSmoothPerInflate = smoothingIterations / inflationIterations;
   
   //
   // Smooth and inflate the surface
   //
   for (int iter = 0; iter < inflationIterations; iter++) {
      //
      // Set progress dialog
      //
      if (progressDialog != NULL) {
         //
         // See if progress dialog was cancelled
         //
         if (progressDialog->wasCanceled()) {
            break;
         }
         //
         // update progress dialog
         //
         progressDialog->setValue(iter + 1);
         progressDialog->setLabelText("Inflating");
         qApp->processEvents();  // note: qApp is global in QApplication
      }

      
      //
      // Smooth the surface
      //
      arealSmoothing(1.0, numSmoothPerInflate, 0);
      
      //
      // Move center of mass to origin
      //
      translateToCenterOfMass();
      
      //
      // Inflate the surface
      //
      for (int i = 0; i < numNodes; i++) {
         float p[3];
         coordinates.getCoordinate(i, p);
         const double radiusPoint = std::sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
         double factor = 1.0 + ( (inflationFactor - 1.0)
                                 * (1.0 - (radiusPoint / radius)) );       
         p[0] *= factor;
         p[1] *= factor;
         p[2] *= factor;
         coordinates.setCoordinate(i, p);
      }
   }
   
   //       
   // Remove progress dialog
   //     
   if (progressDialog != NULL) {
      progressDialog->setValue(inflationIterations + 2);
      delete progressDialog;
      qApp->processEvents();  // note: qApp is global in QApplication
   }
}

/**
 * Create the inflated and ellipsoid surfaces starting from a fiducial surface.
 * The inflated and ellipsoid surfaces are added to the brain set.
 * Use iteration scaling when the surface contains a large number of nodes.  
 * If the surface contains 150,000 nodes, try an iterations scale of 2.5.
 */
void
BrainModelSurface::createInflatedAndEllipsoidFromFiducial(const bool createInflated,
                                                          const bool createVeryInflated,
                                                          const bool createEllipsoid,
                                                          const bool createSphere,
                                                          const bool createCompressedMedialWall,
                                                          const bool enableFingerSmoothing,
                                                          const bool scaleToMatchFiducialArea,
                                                          const float iterationsScaleIn,
                                                          MetricFile* metricMeasurementsFileOut,
                                                          const float compressionFactorIn) const
{
   if ((createInflated == false) &&
       (createVeryInflated == false) &&
       (createEllipsoid == false) &&
       (createSphere == false) &&
       (createCompressedMedialWall == false)) {
      return;
   }
   
   const int numNodes = getNumberOfNodes();
   
   //
   // Free surfer surfaces have lots of nodes (150,000) versus Caret (75,000)
   // so the number of iterations may need to be increased
   //
   float iterationScale = 1.0;
   if (iterationsScaleIn != 0.0) {
      iterationScale = iterationsScaleIn;
   }
   
   //
   // Fiducial surface is "this" surface
   //
   const BrainModelSurface* fiducialSurface = this;
   const float fiducialSurfaceArea = fiducialSurface->getSurfaceArea();
   
   //
   // Copy the surface
   //
   BrainModelSurface* lowSmoothSurface = new BrainModelSurface(*fiducialSurface);
   
   //
   // Create the "low smooth" surface
   //
   MetricFile metricMeasureFile1;
   lowSmoothSurface->inflateSurfaceAndSmoothFingers(fiducialSurface,
                                                   1,     // number of cycles
                                                   0.2,   // regular smoothing strength
                                  static_cast<int>(50 * iterationScale),    // regular smoothing iterations
                                                   1.0,   // inflation factor
                                                   3.0,   // finger compress/stretch threshold
                                                   1.0,   // finger smoothing strength
                                                   0,     // finger smoothing iterations
                                                   &metricMeasureFile1);
   
   if (DebugControl::getDebugOn()) {
      if (metricMeasurementsFileOut != NULL) {
         metricMeasurementsFileOut->append(metricMeasureFile1);
      }
   }

   //
   // Find the "gaussian_neg" column
   //
   int lowSmoothGaussNegColumn = -1;
   //if (metricMeasureFile1 != NULL) {
      lowSmoothGaussNegColumn = metricMeasureFile1.getColumnWithName("gaussian_neg");
   //}

   //
   // High smooth and low smooth metric files
   //
   MetricFile highSmoothMetric;
   MetricFile lowSmoothMetric;
   MetricFile compStretchMetric;
   
   //
   // Get the gaussian neg for the low smooth
   //
   if (lowSmoothGaussNegColumn >= 0) {
      lowSmoothMetric.setNumberOfNodesAndColumns(numNodes, 1);
      lowSmoothMetric.setColumnName(0, "gaussian_neg.LowSmooth");
      for (int i = 0; i < numNodes; i++) {
         lowSmoothMetric.setValue(i, 0, metricMeasureFile1.getValue(i, lowSmoothGaussNegColumn));
      }
   }
   
   //
   // Copy the surface
   //
   BrainModelSurface* inflatedSurface = new BrainModelSurface(*lowSmoothSurface);
   inflatedSurface->setSurfaceType(BrainModelSurface::SURFACE_TYPE_INFLATED);
   if (createInflated) {
      CoordinateFile* cf = inflatedSurface->getCoordinateFile();
      cf->makeDefaultFileName(inflatedSurface->getSurfaceTypeName()); 
      brainSet->addBrainModel(inflatedSurface);
   }
   
   //
   // Create the inflated surface
   //
   int fingerSmoothingIterations = 0;
   if (enableFingerSmoothing) {
      fingerSmoothingIterations = static_cast<int>(30 * iterationScale);
   }
   MetricFile metricMeasureFile2;
   inflatedSurface->inflateSurfaceAndSmoothFingers(fiducialSurface,
                                                   2,     // number of cycles
                                                   1.0,   // regular smoothing strength
                                  static_cast<int>(30 * iterationScale),    // regular smoothing iterations
                                                   1.4,   // inflation factor
                                                   3.0,   // finger compress/stretch threshold
                                                   1.0,   // finger smoothing strength
                                                   fingerSmoothingIterations,
                                                   &metricMeasureFile2);   
   if (scaleToMatchFiducialArea) {
      inflatedSurface->scaleSurfaceToArea(fiducialSurfaceArea, false);
   }
   
   //
   // If the very inflated surface should be created
   //
   if (createVeryInflated) {
      //
      // Copy the surface
      //
      BrainModelSurface* veryInflatedSurface = new BrainModelSurface(*inflatedSurface);
      veryInflatedSurface->setSurfaceType(BrainModelSurface::SURFACE_TYPE_VERY_INFLATED);
      CoordinateFile* cf = veryInflatedSurface->getCoordinateFile();
      cf->makeDefaultFileName(veryInflatedSurface->getSurfaceTypeName()); 
      brainSet->addBrainModel(veryInflatedSurface);
      
      //
      // Create the very inflated surface 
      //
      MetricFile metricMeasureFile3;
      veryInflatedSurface->inflateSurfaceAndSmoothFingers(fiducialSurface,
                                                      4,     // number of cycles
                                                      1.0,   // regular smoothing strength
                                     static_cast<int>(30 * iterationScale),    // regular smoothing iterations
                                                      1.1,   // inflation factor
                                                      3.0,   // finger compress/stretch threshold
                                                      1.0,   // finger smoothing strength
                                                      0,     // finger smoothing iterations
                                                      &metricMeasureFile3);
      if (scaleToMatchFiducialArea) {
         veryInflatedSurface->scaleSurfaceToArea(fiducialSurfaceArea, false);
      }
   }
   
   //
   // If the ellipsoid should be created
   //
   BrainModelSurface* highSmoothSurface = NULL;
   BrainModelSurface* ellipsoidSurface = NULL;
   if (createEllipsoid || createSphere || createCompressedMedialWall) {   
      //
      // Copy the surface
      //
      highSmoothSurface = new BrainModelSurface(*inflatedSurface);
      
      //
      // Create the high smooth surface 
      //
      int fingerSmoothingIterations = 0;
      if (enableFingerSmoothing) {
         fingerSmoothingIterations = static_cast<int>(60 * iterationScale);
      }
      MetricFile metricMeasureFile4;
      highSmoothSurface->inflateSurfaceAndSmoothFingers(fiducialSurface,
                                                      6,     // number of cycles
                                                      1.0,   // regular smoothing strength
                                     static_cast<int>(60 * iterationScale),    // regular smoothing iterations
                                                      1.6,   // inflation factor
                                                      3.0,   // finger compress/stretch threshold
                                                      1.0,   // finger smoothing strength
                                                      fingerSmoothingIterations,    // finger smoothing iterations
                                                      &metricMeasureFile4);
      //
      // Find the "compressed" column
      //
      int highSmoothedCompressedColumn = -1;
      //if (metricMeasurementsFile != NULL) {
         highSmoothedCompressedColumn = metricMeasureFile4.getColumnWithName("compressed");
      //}
      
      //
      // Get the compressed for the high smooth
      //
      if (highSmoothedCompressedColumn >= 0) {
         highSmoothMetric.setNumberOfNodesAndColumns(numNodes, 1);
         highSmoothMetric.setColumnName(0, "compressed.HighSmooth");
         for (int i = 0; i < numNodes; i++) {
            highSmoothMetric.setValue(i, 0, metricMeasureFile4.getValue(i, highSmoothedCompressedColumn));
         }
      }
   
      //
      // Copy the surface
      //
      ellipsoidSurface = new BrainModelSurface(*highSmoothSurface);
      ellipsoidSurface->setSurfaceType(BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL);
      if (createEllipsoid) {
         CoordinateFile* cf = ellipsoidSurface->getCoordinateFile();
         cf->makeDefaultFileName(ellipsoidSurface->getSurfaceTypeName()); 
         brainSet->addBrainModel(ellipsoidSurface);
      }   
      //
      // Create the ellipsoid surface
      //
      MetricFile metricMeasureFile5;
      ellipsoidSurface->inflateSurfaceAndSmoothFingers(fiducialSurface,
                                                      6,     // number of cycles
                                                      1.0,   // regular smoothing strength
                                     static_cast<int>(50 * iterationScale),    // regular smoothing iterations
                                                      1.4,   // inflation factor
                                                      4.0,   // finger compress/stretch threshold
                                                      1.0,   // finger smoothing strength
                                                      fingerSmoothingIterations,     // finger smoothing iterations
                                                      &metricMeasureFile5);
      if (scaleToMatchFiducialArea) {
         ellipsoidSurface->scaleSurfaceToArea(fiducialSurfaceArea, false);
      }
      
      //
      // Find the "average_cosi" column
      //
      int averageCosiColumn = -1;
      //if (metricMeasurementsFile != NULL) {
         averageCosiColumn = metricMeasureFile5.getColumnWithName("average_cosi");
      //}
      
      //
      // Get the compressed for the high smooth
      //
      if (averageCosiColumn >= 0) {
         compStretchMetric.setNumberOfNodesAndColumns(numNodes, 1);
         compStretchMetric.setColumnName(0, "Ellipsoid_CompressedOrStretched");
         for (int i = 0; i < numNodes; i++) {
            compStretchMetric.setValue(i, 0, metricMeasureFile5.getValue(i, averageCosiColumn));
         }
      }
   }
   
   BrainModelSurface* sphereSurface = NULL;
   if (createSphere || createCompressedMedialWall) {
      //
      // Copy the surface
      //
      sphereSurface = new BrainModelSurface(*ellipsoidSurface);
      sphereSurface->setSurfaceType(BrainModelSurface::SURFACE_TYPE_SPHERICAL);
      sphereSurface->convertEllipsoidToSphereWithSurfaceArea(fiducialSurfaceArea);
      CoordinateFile* cf = sphereSurface->getCoordinateFile();
      cf->makeDefaultFileName(sphereSurface->getSurfaceTypeName()); 
      if (createSphere) {
         brainSet->addBrainModel(sphereSurface);
      }
   }
   
   if (createCompressedMedialWall) {
      //
      // Copy the surface
      //
      BrainModelSurface* cmwSurface = new BrainModelSurface(*sphereSurface);
      cmwSurface->setSurfaceType(BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL);
      CoordinateFile* cf = cmwSurface->getCoordinateFile();
      cf->makeDefaultFileName(cmwSurface->getSurfaceTypeName()); 
      
      cmwSurface->setToStandardView(0, VIEW_MEDIAL);
      cmwSurface->applyCurrentView(0, true, true, true);
      TransformationMatrix tm;
      tm.rotate(TransformationMatrix::ROTATE_X_AXIS, -27.0);
      cmwSurface->applyTransformationMatrix(tm);
      cmwSurface->convertSphereToCompressedMedialWall(compressionFactorIn); //0.95);   //0.85);
      cmwSurface->projectCoordinatesToPlane(
                   BrainModelSurface::COORDINATE_PLANE_MOVE_POSITIVE_Z_TO_ZERO);
      cmwSurface->computeNormals();

      brainSet->addBrainModel(cmwSurface);    
   }
   
   delete lowSmoothSurface;
   if (highSmoothSurface != NULL) {
      delete highSmoothSurface;
   }
   if (ellipsoidSurface != NULL) {
      if (createEllipsoid == false) {
         delete ellipsoidSurface;
      }
   }
   if (sphereSurface != NULL) {
      if (createSphere == false) {
         delete sphereSurface;
      }
   }
   if (createInflated == false) {
      delete inflatedSurface;
   }
   
   if (metricMeasurementsFileOut != NULL) {
      if (lowSmoothMetric.getNumberOfColumns() > 0) {
         metricMeasurementsFileOut->append(lowSmoothMetric);
      }
      if (highSmoothMetric.getNumberOfColumns() > 0) {
         metricMeasurementsFileOut->append(highSmoothMetric);
      }
      if (compStretchMetric.getNumberOfColumns() > 0) {
         metricMeasurementsFileOut->append(compStretchMetric);
      }
   }
}

/**
 * Inflate surface and smooth fingers
 */
void
BrainModelSurface::inflateSurfaceAndSmoothFingers(const BrainModelSurface* fiducialSurfaceIn,
                                                  const int numberSmoothingCycles,
                                                  const float regularSmoothingStrength,
                                                  const int regularSmoothingIterations,
                                                  const float inflationFactorIn,
                                                  const float compressStretchThreshold,
                                                  const float fingerSmoothingStrength,
                                                  const int fingerSmoothingIterations,
                                                  MetricFile* metricMeasurementsFile)
{
   if (fiducialSurfaceIn == NULL) {
      std::cout << "ERROR: BrainModelSurface::inflateSurfaceAndSmoothFingers - No fiducial surface as input." << std::endl;
      return;
   }
   const int numNodes = getNumberOfNodes();
   if (numNodes <= 0) {
      return;
   }
   
   appendToCoordinateFileComment("Inflate Surface and Smooth Fingers: ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(numberSmoothingCycles));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(regularSmoothingStrength));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(regularSmoothingIterations));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(inflationFactorIn));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(compressStretchThreshold));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(fingerSmoothingStrength));
   appendToCoordinateFileComment(" ");
   appendToCoordinateFileComment(StringUtilities::fromNumber(fingerSmoothingIterations));
   appendToCoordinateFileComment("\n");
   
   const float inflationFactor = inflationFactorIn - 1.0;
   
   QTime timer;
   timer.start();

   //
   // Copy the fiducial surface since it will be modified (translated to center of mass)
   //
   BrainModelSurface* fiducialSurface = new BrainModelSurface(*fiducialSurfaceIn);
   
   //
   // Step 2: Translate AUX coord origin to center of gravity
   //  Caret Menu: Operate->Surface Translation->Translate To Center of Mass
   translateToCenterOfMass();
   
   //
   // Translate the fiducial to center of mass
   //
   fiducialSurface->translateToCenterOfMass();
   
   //
   // Get bounds of fiducial surface
   //
   float fiducialBounds[6];
   fiducialSurface->getBounds(fiducialBounds);
   const float xdiff = fiducialBounds[1] - fiducialBounds[0];
   const float ydiff = fiducialBounds[3] - fiducialBounds[2];
   const float zdiff = fiducialBounds[5] - fiducialBounds[4];
   
   //
   // Step 5: Calculate surface area of fiducial surface
   //
   const float fiducialSurfaceArea = fiducialSurface->getSurfaceArea();
   
   //float* arealCompression = new float[numNodes];
   float* averageCompressedStretched = new float[numNodes];
   float* maximumLinearDistortion = new float[numNodes];
   float* averageArealCompression = new float[numNodes];
   float* compressedStretched = new float[numNodes];
   float* stretching = new float[numNodes];
   
   float surfaceAreaRatio = 0.0;
   
   //
   // The surface's coordinates
   //
   CoordinateFile* surfaceCoords = getCoordinateFile();
   const float* surfacePos = surfaceCoords->getCoordinate(0);
   const CoordinateFile* fiducialCoords = fiducialSurface->getCoordinateFile();
   const float* fiducialPos = fiducialCoords->getCoordinate(0);
   
   //
   // Step 6: Main Smoothing Cycle
   // Note: No smoothing takes place in the "+1" cycle, just metric calculation
   //
   for (int cycles = 0; cycles < (numberSmoothingCycles + 1); cycles++) {
   
      if (cycles < numberSmoothingCycles) {
         //
         // Step 6a: Apply Smoothing to AUX coord
         //  Caret Menu: Operate->Smoothing->Smoothing...
         //
         arealSmoothing(regularSmoothingStrength, 
                        regularSmoothingIterations,
                        1);

         //
         // Step 6b: Incrementally Inflate AUX Surface by Ellipsoidal Projection
         //
         for (int i = 0; i < numNodes; i++) {
            float xyz[3];
            surfaceCoords->getCoordinate(i, xyz);
            const float x = xyz[0] / xdiff;
            const float y = xyz[1] / ydiff;
            const float z = xyz[2] / zdiff;

            const float r = std::sqrt(x*x + y*y + z*z);

            const float k = 1.0 + inflationFactor * (1.0 - r);

            xyz[0] *= k;
            xyz[1] *= k;
            xyz[2] *= k;
            surfaceCoords->setCoordinate(i, xyz);
         }
      }
      
      // Step 6c: Calculate surface area of this surface
      const float inflatedSurfaceArea = getSurfaceArea();
      
      //
      // Ratio of inflated and spherical surfaces
      //
      surfaceAreaRatio = inflatedSurfaceArea / fiducialSurfaceArea;
      
      //
      // Topology helper for neighbors
      //
      const TopologyHelper* th = getTopologyFile()->getTopologyHelper(false, true, false);
   
      //
      // Step 6d: Calculate compress/stretched value for each node
      //
      for (int i = 0; i < numNodes; i++) {
      
         //
         // Get position of node in both this and fiducial surface
         //
         //float nodePos[3];
         //surfaceCoords->getCoordinate(i, nodePos);
         const float* nodePos = &surfacePos[i * 3];
         //float nodeFiducialPos[3];
         //fiducialCoords->getCoordinate(i, nodeFiducialPos);
         const float* nodeFiducialPos = &fiducialPos[i * 3];
         
         maximumLinearDistortion[i] = 0.0;
         //arealCompression[i]        = 0.0;
         averageArealCompression[i] = 0.0;
         
         //float tileArea = 0.0;
         //float fiducialTileArea = 0.0;
         float numValidNeighbors = 0;
         
         //
         // Get neighbors for this node
         //
         int numNeighbors;
         const int* neighbors = th->getNodeNeighbors(i, numNeighbors);
         
         //    
         // log(2.0) to avoid recomputation each iteration
         // 
         //const double log2 = log(2.0);

         //
         // Loop through the neighbors
         //
         for (int j = 0; j < numNeighbors; j++) {
            const int neighbor = neighbors[j];
            
            //const float* neighPos = surfaceCoords->getCoordinate(neighbor);
            const float* neighPos = &surfacePos[neighbor * 3];
            //const float* neighFiducialPos = fiducialCoords->getCoordinate(neighbor);
            const float* neighFiducialPos = &fiducialPos[neighbor * 3];
            
            //
            // calculate maximum linear distortion on aux and ref surface
            //
            float dx = neighPos[0] - nodePos[0];
            float dy = neighPos[1] - nodePos[1];
            float dz = neighPos[2] - nodePos[2];
            float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
            dx = neighFiducialPos[0] - nodeFiducialPos[0];
            dy = neighFiducialPos[1] - nodeFiducialPos[1];
            dz = neighFiducialPos[2] - nodeFiducialPos[2];
            float fiducialDist = std::sqrt(dx*dx + dy*dy + dz*dz);
            if (fiducialDist > 0.0) {
               const float ratio = dist / fiducialDist;
               if (ratio > maximumLinearDistortion[i]) {
                  maximumLinearDistortion[i] = ratio;
               }
            }
            
            //
            // Next neighbor
            //
            int jNext = j + 1;
            if (jNext >= numNeighbors) {
               jNext = 0;
            }
            const int nextNeighbor = neighbors[jNext];
            
            //
            // compute area of tiles on aux and ref surfaces
            //
            //3/4/05 tileArea += getTileArea(i, neighbor, nextNeighbor);
            //3/4/05 fiducialTileArea = fiducialSurface->getTileArea(i, neighbor, nextNeighbor);
            const float tileArea = getTileArea(i, neighbor, nextNeighbor);
            const float fiducialTileArea = fiducialSurface->getTileArea(i, neighbor, nextNeighbor);
                                 
            //
            // average areal compression of tiles associated with node
            //
            float distort = 0.0;
            if (tileArea > 0.0) {
               distort = fiducialTileArea / tileArea;
            }
            else {
               if (fiducialTileArea != 0.0) {
                  distort = 10000.0;  // big dist since denominator zero
               }
               else {
                  distort = 1.0;  // if both zero then use assume same area
               }
            }
            
            //
            // Zero will cause -inf
            //
            if (distort < 0.00000001) {
               distort = 0.00000001;
            }
            
            averageArealCompression[i] += distort; //arealCompression[i] += distort;  //log(distort) / log2;
            numValidNeighbors += 1.0;
         }
         
         if (numValidNeighbors > 0) {
            //arealCompression[i] /= numValidNeighbors;
            averageArealCompression[i] /= numValidNeighbors;
         }
      
         //
         // compressed/stretched for node
         //
         compressedStretched[i] = maximumLinearDistortion[i]
                                * averageArealCompression[i]  //arealCompression[i] 
                                * surfaceAreaRatio;
         
         //
         // stretching for node
         //
         stretching[i] = maximumLinearDistortion[i]
                                * std::sqrt(averageArealCompression[i]  //arealCompression[i] 
                                       * surfaceAreaRatio);
      }
      
      //
      // average compressed/stretched for all nodes by averaging with neighbors
      //
      for (int i = 0; i < numNodes; i++) {
         averageCompressedStretched[i] = compressedStretched[i];
         if (DebugControl::getDebugOn()) {
            if ((i % 1000) == 0) {
               std::cout << "comp-stretch " << i << " " << compressedStretched[i] << std::endl;
            }
         }
         int numNeighbors;
         const int* neighbors = th->getNodeNeighbors(i, numNeighbors);
         if (numNeighbors > 0) {
            for (int j = 0; j < numNeighbors; j++) {
               const int n = neighbors[j];
               averageCompressedStretched[i] += compressedStretched[n];
            }
            averageCompressedStretched[i] /= ((float)(numNeighbors + 1));
         }
      }

      //
      // average areal compression for all nodes by averaging with neighbors
      //
/*
      for (int i = 0; i < numNodes; i++) {
         averageArealCompression[i] = arealCompression[i];
         int numNeighbors;
         const int* neighbors = th->getNodeNeighbors(i, numNeighbors);
         if (numNeighbors > 0) {
            for (int j = 0; j < numNeighbors; j++) {
               const int n = neighbors[j];
               averageArealCompression[i] += arealCompression[n];
            }
            averageArealCompression[i] /= ((float)(numNeighbors + 1));
         }
      }
*/
      //
      // Step 6e: Flag highly compressed/stretched nodes for targeted smoothing
      //
      int numDistortionAboveThreshold = 0;
      float maxDistortion = -std::numeric_limits<float>::max();
      float minDistortion =  std::numeric_limits<float>::max();
      std::vector<bool> needSmoothing(numNodes);
      for (int i = 0; i < numNodes; i++) {
         if (averageCompressedStretched[i] > compressStretchThreshold) {
            numDistortionAboveThreshold++;
            needSmoothing[i] = true;
         }
         else {
            needSmoothing[i] = false;
         }
         if (averageCompressedStretched[i] > maxDistortion) {
            maxDistortion = averageCompressedStretched[i];
         }
         if (averageCompressedStretched[i] < minDistortion) {
            minDistortion = averageCompressedStretched[i];
         }
      }  
       
      if (DebugControl::getDebugOn()) {
         std::cout << "Distortion Info:" << endl;
         std::cout << "   " << numDistortionAboveThreshold << " of " << numNodes
              << " points are above "
              << "threshold = " << compressStretchThreshold << endl;
         std::cout << "   minimum distortion: " << minDistortion << endl;
         std::cout << "   maximum distortion: " << maxDistortion << endl;
      }
      
      if (cycles < numberSmoothingCycles) {
         //
         // Step 6f: Targeted smoothing
         //
         arealSmoothing(fingerSmoothingStrength,
                        fingerSmoothingIterations,
                        1,
                        &needSmoothing);
      }
   }
      
   computeNormals();

   //
   // If metrics should be created
   //
   if (metricMeasurementsFile != NULL) {
      //
      // Column names
      //
      const QString averageCosiColumnName("average_cosi");
      const QString compressedColumnName("compressed");
      const QString stretchedColumnName("stretched");
      const QString gaussianNegColumnName("gaussian_neg");
      
      //
      // See if columns exist
      //
      int averageCosiColumn = metricMeasurementsFile->getColumnWithName(averageCosiColumnName);
      int compressedColumn  = metricMeasurementsFile->getColumnWithName(compressedColumnName);
      int stretchedColumn   = metricMeasurementsFile->getColumnWithName(stretchedColumnName);
      int gaussianNegColumn = metricMeasurementsFile->getColumnWithName(gaussianNegColumnName);
      
      //
      // Determine how many new columns are needed
      //
      int newColumnsNeeded = 0;
      if (averageCosiColumn < 0) newColumnsNeeded++;
      if (compressedColumn < 0) newColumnsNeeded++;
      if (stretchedColumn < 0) newColumnsNeeded++;
      if (gaussianNegColumn < 0) newColumnsNeeded++;
      
      //
      // Add columns to the metric file
      //
      if (newColumnsNeeded > 0) {
         if (metricMeasurementsFile->getNumberOfColumns() == 0) {
            metricMeasurementsFile->setNumberOfNodesAndColumns(numNodes, newColumnsNeeded);
         }
         else {
            metricMeasurementsFile->addColumns(newColumnsNeeded);
         }
      }
      
      //
      // Get the column numbers for the new data
      //
      int newCnt = 1;
      if (gaussianNegColumn < 0) {
         gaussianNegColumn = metricMeasurementsFile->getNumberOfColumns() - newCnt;
         newCnt++;
      }
      if (stretchedColumn < 0) {
         stretchedColumn = metricMeasurementsFile->getNumberOfColumns() - newCnt;
         newCnt++;
      }
      if (compressedColumn < 0) {
         compressedColumn = metricMeasurementsFile->getNumberOfColumns() - newCnt;
         newCnt++;
      }
      if (averageCosiColumn < 0) {
         averageCosiColumn = metricMeasurementsFile->getNumberOfColumns() - newCnt;
         newCnt++;
      }
      
      //
      // Name the columns
      //
      metricMeasurementsFile->setColumnName(averageCosiColumn, averageCosiColumnName);
      metricMeasurementsFile->setColumnName(compressedColumn,  compressedColumnName);
      metricMeasurementsFile->setColumnName(stretchedColumn,   stretchedColumnName);
      metricMeasurementsFile->setColumnName(gaussianNegColumn, gaussianNegColumnName);
      
      //
      // Set the average cosi column
      //
      for (int i = 0; i < numNodes; i++) {
         metricMeasurementsFile->setValue(i, averageCosiColumn, averageCompressedStretched[i]);
         metricMeasurementsFile->setValue(i, compressedColumn, averageArealCompression[i] * surfaceAreaRatio);
         metricMeasurementsFile->setValue(i, stretchedColumn, stretching[i]);
      }
      
      //
      // Compute gaussian curvature
      // 
      SurfaceShapeFile ssf;
      const QString gaussColumnName("gauss");
      BrainModelSurfaceCurvature bmss(brainSet,
                                      this,
                                      &ssf,
                                      BrainModelSurfaceCurvature::CURVATURE_COLUMN_DO_NOT_GENERATE,
                                      BrainModelSurfaceCurvature::CURVATURE_COLUMN_CREATE_NEW,
                                      "",
                                      gaussColumnName);
      float* tempArray = new float[numNodes];
      for (int i = 0; i < numNodes; i++) {
         tempArray[i] = 0.0; //-1.0;  // was 0.0 3/3/05
      }
      try {
         //
         // Generate curvature
         //
         bmss.execute();
         
         //
         // Compute negative gaussian curvature
         //
         const int gaussColumn = ssf.getColumnWithName(gaussColumnName);
         if (gaussColumn >= 0) {
            for (int i = 0; i < numNodes; i++) {
               const float gauss = ssf.getValue(i, gaussColumn);
               if ((gauss < 0.0) && (averageArealCompression[i] > 0.0)) {
                  tempArray[i] = -gauss;
                  tempArray[i] *= averageArealCompression[i];
               }
            }
         }
      }
      catch (BrainModelAlgorithmException&) {
         // curvature calculation failed
      }
      for (int i = 0; i < numNodes; i++) {
         metricMeasurementsFile->setValue(i, gaussianNegColumn, tempArray[i]);
      }
      
      //
      // Log 10 scale the values
      //
      metricMeasurementsFile->scaleColumnLog10(averageCosiColumn);
      metricMeasurementsFile->scaleColumnLog10(compressedColumn);
      metricMeasurementsFile->scaleColumnLog10(stretchedColumn);
      metricMeasurementsFile->scaleColumnLog10(gaussianNegColumn);
      
      delete[] tempArray;
   }
   
   delete fiducialSurface;
   
   //delete[] arealCompression;
   delete[] averageCompressedStretched;
   delete[] maximumLinearDistortion;
   delete[] averageArealCompression;
   delete[] compressedStretched;
   delete[] stretching;
   
   if (DebugControl::getDebugOn()) {
      std::cout << "Total time: " << (static_cast<float>(timer.elapsed()) / 1000.0) << std::endl;
   }
}

/**
 * translate a surface to its center of mass.
 */
void 
BrainModelSurface::translateToCenterOfMass()
{
//   appendToCoordinateFileComment("Translate to Center of Mass");
   
   float com[3];
   getCenterOfMass(com);
   TransformationMatrix tm;
   tm.translate(-com[0], -com[1], -com[2]);
   applyTransformationMatrix(tm);
   coordinates.clearDisplayList();
}

/**
 * Get the center of mass for a surface.
 */      
void
BrainModelSurface::getCenterOfMass(float centerOfMass[3]) const
{
   const TopologyFile* tf = getTopologyFile();
   const TopologyHelper* th = tf->getTopologyHelper(false, true, false);

   const int numNodes = getNumberOfNodes();
   double cx = 0.0, cy = 0.0, cz = 0.0;
   float numNodesWithNeighbors = 0.0;
   for (int i = 0; i < numNodes; i++) {
      if (th->getNodeHasNeighbors(i)) {
         const float* xyz = coordinates.getCoordinate(i);
         cx += xyz[0];
         cy += xyz[1];
         cz += xyz[2];
         numNodesWithNeighbors += 1.0;
      }
   }
   if (numNodesWithNeighbors > 0.0) {
      centerOfMass[0] = cx / numNodesWithNeighbors;
      centerOfMass[1] = cy / numNodesWithNeighbors;
      centerOfMass[2] = cz / numNodesWithNeighbors;
   }
   else {
      centerOfMass[0] = 0.0;
      centerOfMass[1] = 0.0;
      centerOfMass[2] = 0.0;
   }
}

/**
 * Get the radius of a spherical surface (assumes surface is a sphere
 * with its center at the origin).
 */
float
BrainModelSurface::getSphericalSurfaceRadius() const
{
   const int numNodes = getNumberOfNodes();
   const TopologyHelper* helper = topology->getTopologyHelper(false, true, false);
   
   for (int i = 0; i < numNodes; i++) {
      float p[3];
      if (helper->getNodeHasNeighbors(i)) {
         coordinates.getCoordinate(i, p);
         const float radius = std::sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
         return radius;
      }
   }
   return 0.0;
}

/**
 * Orient a spherical surface so that a group of nodes identified by paint are placed
 * on the negative Z axis.  Returns true if matching nodes are NOT found.
 */
bool
BrainModelSurface::orientPaintedNodesToNegativeZAxis(const PaintFile* pf,
                                                     const std::vector<QString> paintNames,
                                                     const int paintColumn,
                                                     QString& errorMessage)
{
   errorMessage = "";
   
   //
   // Make sure paint column is valid
   //
   if ((paintColumn < 0) && (paintColumn >= pf->getNumberOfColumns())) {
      errorMessage = "Paint column number is invalid.";
      return false;
   }
   
   //
   // convert paint names into paint indices
   //
   std::vector<int> paintIndices;
   for (int i = 0; i < static_cast<int>(paintNames.size()); i++) {
      const int index = pf->getPaintIndexFromName(paintNames[i]);
      if (index >= 0) {
         paintIndices.push_back(index);
      }
   }
   if (paintIndices.size() == 0) {
      std::ostringstream str;
      str << "Paint names (";
      for (unsigned int i = 0; i < paintNames.size(); i++) {
         if (i > 0) {
            str << ", ";
         }
         str << paintNames[i].toAscii().constData();
      }
      str << ") needed" << "\n";
      str << "for orienting the surface were not found.";
      errorMessage = str.str().c_str();
      return true;
   }
   
   //
   // find the center of gravity of the painted nodes
   //
   double sumCOG[3] = { 0.0, 0.0, 0.0 };
   const int numNodes = getNumberOfNodes();
   float numMatchingNodes = 0.0;
   for (int i = 0; i < numNodes; i++) {
      const float* xyz = coordinates.getCoordinate(i);
      const int paintIndex = pf->getPaint(i, paintColumn);
      if (std::find(paintIndices.begin(), paintIndices.end(), paintIndex) != paintIndices.end()) {
         sumCOG[0] += xyz[0];
         sumCOG[1] += xyz[1];
         sumCOG[2] += xyz[2];
         numMatchingNodes += 1.0;
      }
   }
   
   //
   //  Make sure nodes were found
   //
   if (numMatchingNodes == 0.0) {
      errorMessage = "No matching paint names found:";
      for (int i = 0; i < static_cast<int>(paintNames.size()); i++) {
         errorMessage.append(" ");
         errorMessage.append(paintNames[i]);
      }
      return true;
   }
   
   //
   // Place the center of gravity of the matching nodes on the negative Z axis
   //
   const float cog[3] = { (sumCOG[0] / numMatchingNodes),
                          (sumCOG[1] / numMatchingNodes),
                          (sumCOG[2] / numMatchingNodes) };
   orientPointToNegativeZAxis(cog);
   
   return false;
}

/**
 * Orient a sphere so that the point is placed on the negative Z axis.
 * Assumes that the surface is center is at the origin.
 */
void 
BrainModelSurface::orientPointToNegativeZAxis(const float pIn[3])
{
   float vect[3] = { pIn[0], pIn[1], pIn[2] };
   
   //
   // Normalize the point
   //
   MathUtilities::normalize(vect);
   
   // assume a vector goes from the  center of gravity in the
   // negative z direction so it would be something like (0, 0, -1).
   // The dot product of the negative Z vector and "vect" is just -vect[2]
   // since the x & y components of the vector down negative Z are (0, 0).
   // The arc-cosine of the dot product is the angle between the
   // two vectors.
   const float angle = std::acos(-vect[2]) * MathUtilities::radiansToDegrees();

   // vector perpendicular (90 degrees ccw) to the vect in X/Y plane
   double perp[3];
   perp[0] = -vect[1];
   perp[1] = vect[0];
   perp[2] = 0.0;
   MathUtilities::normalize(perp);

   //
   // Create a transformation matrix for the rotation and apply it to the surface
   //
   TransformationMatrix tm;
   tm.rotate(angle, perp);
   applyTransformationMatrix(tm);
   coordinates.clearDisplayList();
}

/**
 * orient a sphere so that the point is placed on the positive Z axis.
 */
void 
BrainModelSurface::orientPointToPositiveZAxis(const float p[3])
{
   //
   // Put on negative Z
   //
   orientPointToNegativeZAxis(p);
   
   //
   // Rotate 180 about Y so point on positive Z-Axis
   //
   TransformationMatrix tm;
   tm.rotate(TransformationMatrix::ROTATE_Y_AXIS, 180.0);
   applyTransformationMatrix(tm);
}

/**
 * push (save) the coordinates
 */
void 
BrainModelSurface::pushCoordinates()
{
   pushPopCoordinates.clear();
   
   const int numNodes = getNumberOfNodes();
   for (int i = 0; i < numNodes; i++) {
      float x, y, z;
      coordinates.getCoordinate(i, x, y, z);
      pushPopCoordinates.push_back(x);
      pushPopCoordinates.push_back(y);
      pushPopCoordinates.push_back(z);
   }
}

/**
 * pop (restore) the coordinates (must have done a push prior to this call)
 */
void 
BrainModelSurface::popCoordinates()
{
   const int numNodes = getNumberOfNodes();
   if (numNodes == static_cast<int>(pushPopCoordinates.size() / 3)) {
      for (int i = 0; i < numNodes; i++) {
         coordinates.setCoordinate(i, &pushPopCoordinates[i*3]);
      }
   }
   else {
      std::cout << "Number of nodes incorrect in BrainModelSurface::popCoordinates() at "
                << __LINE__ << " in " << __FILE__ << std::endl;
   }
   coordinates.clearDisplayList();
}

/**
 * project the coordinates to a plane 
 */
void 
BrainModelSurface::projectCoordinatesToPlane(const COORDINATE_PLANE plane)
{
   const unsigned long modFlag = coordinates.getModified();

   switch (plane) {
      case COORDINATE_PLANE_NONE:
         break;
      case COORDINATE_PLANE_MOVE_POSITIVE_X_TO_ZERO:
      case COORDINATE_PLANE_MOVE_NEGATIVE_X_TO_ZERO:
      case COORDINATE_PLANE_MOVE_POSITIVE_Y_TO_ZERO:
      case COORDINATE_PLANE_MOVE_NEGATIVE_Y_TO_ZERO:
      case COORDINATE_PLANE_MOVE_POSITIVE_Z_TO_ZERO:
      case COORDINATE_PLANE_MOVE_NEGATIVE_Z_TO_ZERO:
         {
            pushCoordinates();
            
            const int numNodes = getNumberOfNodes();
            for (int i = 0; i < numNodes; i++) {
               float x, y, z;
               coordinates.getCoordinate(i, x, y, z);

               switch (plane) {
                  case COORDINATE_PLANE_NONE:
                     break;
                  case COORDINATE_PLANE_MOVE_POSITIVE_X_TO_ZERO:
                     x = std::min(x, 0.0f);
                     break;
                  case COORDINATE_PLANE_MOVE_NEGATIVE_X_TO_ZERO:
                     x = std::max(x, 0.0f);
                     break;
                  case COORDINATE_PLANE_MOVE_POSITIVE_Y_TO_ZERO:
                     y = std::min(y, 0.0f);
                     break;
                  case COORDINATE_PLANE_MOVE_NEGATIVE_Y_TO_ZERO:
                     y = std::max(y, 0.0f);
                     break;
                  case COORDINATE_PLANE_MOVE_POSITIVE_Z_TO_ZERO:
                     z = std::min(z, 0.0f);
                     break;
                  case COORDINATE_PLANE_MOVE_NEGATIVE_Z_TO_ZERO:
                     z = std::max(z, 0.0f);
                     break;
                  case COORDINATE_PLANE_RESTORE:
                     break;
               }
               
               coordinates.setCoordinate(i, x, y, z);
            }
         }
         break;
      case COORDINATE_PLANE_RESTORE:
         popCoordinates();
         break;
   }
/*   
   const int numNodes = getNumberOfNodes();
   switch (plane) {
      case COORDINATE_PLANE_MOVE_POSITIVE_Z_TO_ZERO:
         pushCoordinates();
         for (int i = 0; i < numNodes; i++) {
            float x, y, z;
            coordinates.getCoordinate(i, x, y, z);
            if (z > 0.0) {
               z = 0.0;
               coordinates.setCoordinate(i, x, y, z);
            }
         }
         break;
      case COORDINATE_PLANE_MOVE_NEGATIVE_Z_TO_ZERO:
         pushCoordinates();
         for (int i = 0; i < numNodes; i++) {
            float x, y, z;
            coordinates.getCoordinate(i, x, y, z);
            if (z < 0.0) {
               z = 0.0;
               coordinates.setCoordinate(i, x, y, z);
            }
         }
         break;
      case COORDINATE_PLANE_RESTORE:
         popCoordinates();
         break;
   }
*/   
   coordinates.setModifiedCounter(modFlag);
}

/**
 * Align to standard orientation (flat or spherical).
 */
void 
BrainModelSurface::alignToStandardOrientation(const BrainModelSurface* fiducialSurface,
                                              const BorderProjection* centralSulcusBorderProjection,
                                              const bool generateSphericalLatitudeLongitude,
                                              const bool scaleToFiducialArea)
{
   if (fiducialSurface == NULL) {
      return;
   }
   if (centralSulcusBorderProjection == NULL) {
      return;
   }
   
   //
   // Unproject the central sulcus border to the fiducial surface
   //
   const CoordinateFile* fiducialCoordinateFile = fiducialSurface->getCoordinateFile();
   Border centralSulcusBorder;
   centralSulcusBorderProjection->unprojectBorderProjection(
      fiducialCoordinateFile,
      getTopologyFile()->getTopologyHelper(false, true, false),
      centralSulcusBorder);
   
   const int numLinks = centralSulcusBorder.getNumberOfLinks();
   if (numLinks < 2) {
      return;
   }

   //
   // Get position of first link on the FIDUCIAL surface
   //
   const float* firstLinkXYZ = centralSulcusBorder.getLinkXYZ(0);
                                                                  
   //
   // Get position of last link on the FIDUCIAL surface
   //
   const float* lastLinkXYZ = centralSulcusBorder.getLinkXYZ(numLinks - 1);
                                                
   //
   // Determine ventral and dorsal ends of central sulcus
   // 
   int ventralNodeNumber = 
      fiducialCoordinateFile->getCoordinateIndexClosestToPoint(firstLinkXYZ);
   int dorsalNodeNumber =
      fiducialCoordinateFile->getCoordinateIndexClosestToPoint(lastLinkXYZ);
   if (firstLinkXYZ[2] > lastLinkXYZ[2]) {
      std::swap(ventralNodeNumber, dorsalNodeNumber);
   }  
   
   //
   // Align the surface
   //
   alignToStandardOrientation(ventralNodeNumber,
                              dorsalNodeNumber,
                              generateSphericalLatitudeLongitude,
                              scaleToFiducialArea);
}
      
/**
 * Align to standard orientation (flat or spherical).
 */
void 
BrainModelSurface::alignToStandardOrientation(const int ventralTipCentralSulcusNode, 
                                              const int dorsalMedialTipCentralSulcusNode,
                                              const bool generateSphericalLatitudeLongitude,
                                              const bool scaleToFiducialArea)
{
   if ((ventralTipCentralSulcusNode >= 0) &&
       (dorsalMedialTipCentralSulcusNode >= 0)) {
       
      const double leftHemAngleFromVertical = 75.0;
      const double rightHemAngleFromVertical = 105;
      
      //
      // Aligning flat surface
      //
      if ((getSurfaceType() == SURFACE_TYPE_FLAT) ||
          (getSurfaceType() == SURFACE_TYPE_FLAT_LOBAR)) {
         
         //
         // Position of medial and ventral tip nodes
         //
         const float* ventralTipPos = coordinates.getCoordinate(ventralTipCentralSulcusNode);
         const float* medialTipPos  = coordinates.getCoordinate(dorsalMedialTipCentralSulcusNode);
   
         //
         // angle from ventral tip to medial tip in degrees
         //
         const double dx = medialTipPos[0] - ventralTipPos[0];  
         const double dy = medialTipPos[1] - ventralTipPos[1];      
         const double angle = std::atan2(dy, dx) * MathUtilities::radiansToDegrees();
         
         //
         // Set rotation angle appropriate for hemisphere
         //
         float rotateAngle;
         if (structure.getType() == Structure::STRUCTURE_TYPE_CORTEX_LEFT) {
            rotateAngle = leftHemAngleFromVertical - angle;
         }
         else {
            rotateAngle = rightHemAngleFromVertical - angle;
         }
         
         //
         // Transform the surface
         //
         TransformationMatrix tm;
         tm.translate(-ventralTipPos[0], -ventralTipPos[1], 0.0f);
         applyTransformationMatrix(tm);
         const float* transVentralTipPos = coordinates.getCoordinate(ventralTipCentralSulcusNode);
         tm.identity();
         tm.rotate(TransformationMatrix::ROTATE_Z_AXIS, rotateAngle);
         applyTransformationMatrix(tm);
         
         if (DebugControl::getDebugOn()) {
            std::cout << "Surface Alignment: " << std::endl;
            std::cout << "   Ventral Node: " << ventralTipCentralSulcusNode << std::endl;
            std::cout << "   Ventral Pos: " << ventralTipPos[0] << " "
                                 << ventralTipPos[1] << " "
                                 << ventralTipPos[2] << std::endl;
            std::cout << "   Dorsal Node: " << dorsalMedialTipCentralSulcusNode << std::endl;
            std::cout << "   Dorsal Pos: " << medialTipPos[0] << " "
                                 << medialTipPos[1] << " "
                                 << medialTipPos[2] << std::endl;
            std::cout << "   Rotate Angle: " << rotateAngle << std::endl;
            std::cout << "   Ventral Pos After Translate: " << transVentralTipPos[0] << " "
                                 << transVentralTipPos[1] << " "
                                 << transVentralTipPos[2] << std::endl;
            const float* newVentralTipPos = coordinates.getCoordinate(ventralTipCentralSulcusNode);
            std::cout << "   Ventral Pos After Rotate: " << newVentralTipPos[0] << " "
                                 << newVentralTipPos[1] << " "
                                 << newVentralTipPos[2] << std::endl;
         }
         
         //
         // If surface should be scaled to match the area of the fiducial surface
         //
         if (scaleToFiducialArea) {
            if (brainSet != NULL) {
               const BrainModelSurface* fiducial = brainSet->getActiveFiducialSurface();
               if (fiducial == NULL) {
                  fiducial = brainSet->getBrainModelSurfaceOfType(BrainModelSurface::SURFACE_TYPE_FIDUCIAL);
               }
               if (fiducial != NULL) {
                  const float fiducialArea = fiducial->getSurfaceArea(getTopologyFile());
                  scaleSurfaceToArea(fiducialArea, true);
               }
            }
         }
         
         //
         // Surface is now cartesian standard
         //
         coordinates.setHeaderTag(AbstractFile::headerTagCoordFrameID, "CS"); 
      }
      else if (getSurfaceType() == SURFACE_TYPE_SPHERICAL) {
         //
         // Place the ventral tip node on the positive Z-Axis.  Use the negative
         // of the node since the method places the point on the negative Z-Axis.
         //
         orientPointToNegativeZAxis(coordinates.getCoordinate(ventralTipCentralSulcusNode));
         
         //
         // Rotate so ventral tip on positive Z-Axis
         //
         TransformationMatrix tm;
         tm.rotate(TransformationMatrix::ROTATE_Y_AXIS, 180.0);
         applyTransformationMatrix(tm);
         
         //
         // Position of medial and ventral tip nodes
         //
         const float* ventralTipPos = coordinates.getCoordinate(ventralTipCentralSulcusNode);
         const float* medialTipPos  = coordinates.getCoordinate(dorsalMedialTipCentralSulcusNode);
   
         //
         // angle from ventral tip to medial tip in degrees
         //
         const double dx = medialTipPos[0] - ventralTipPos[0];  
         const double dy = medialTipPos[1] - ventralTipPos[1];      
         const double angle = std::atan2(dy, dx) * MathUtilities::radiansToDegrees();
         
         //
         // Set rotation angle appropriate for hemisphere
         //
         float rotateAngle;
         if (structure.getType() == Structure::STRUCTURE_TYPE_CORTEX_LEFT) {
            rotateAngle = leftHemAngleFromVertical - angle;
         }
         else {
            rotateAngle = rightHemAngleFromVertical - angle;
         }
         //
         // Transform the surface
         //
         tm.identity();
         tm.rotate(TransformationMatrix::ROTATE_Z_AXIS, rotateAngle);
         applyTransformationMatrix(tm);
         
         //
         // Generate latitude/longitude if requested
         //
         if (generateSphericalLatitudeLongitude) {
            createLatitudeLongitude(brainSet->getLatLonFile(), 
                                    -1,
                                    "Created by Standard Orientation",
                                    false,
                                    false);
         }

         tm.identity();
         if (structure.getType() == Structure::STRUCTURE_TYPE_CORTEX_LEFT) {
            tm.rotate(TransformationMatrix::ROTATE_Y_AXIS, 270);
            applyTransformationMatrix(tm);
            tm.identity();
            tm.rotate(TransformationMatrix::ROTATE_X_AXIS, 90.0);
            applyTransformationMatrix(tm);
         }
         else {
            tm.rotate(TransformationMatrix::ROTATE_Y_AXIS, 90.0);
            applyTransformationMatrix(tm);
            tm.identity();
            tm.rotate(TransformationMatrix::ROTATE_X_AXIS, 90.0);
            applyTransformationMatrix(tm);
         }
         
         //
         // If surface should be scaled to match the area of the fiducial surface
         //
         if (scaleToFiducialArea) {
            if (brainSet != NULL) {
               const BrainModelSurface* fiducial = brainSet->getActiveFiducialSurface();
               if (fiducial != NULL) {
                  const float fiducialArea = fiducial->getSurfaceArea(getTopologyFile());
                  convertToSphereWithSurfaceArea(fiducialArea);
               }
            }
         }
         
         //
         // Surface is now spherical standard
         //
         coordinates.setHeaderTag(AbstractFile::headerTagCoordFrameID, "SS"); 
      }
   }
   coordinates.clearDisplayList();
}

/**
 * Create deformation field vectors.
 *
 * For each node, the vector originates at the node's "this" surface coordinate.  The
 * vector tip ends at the deformed surfaces's coordinate which is projected to "this" 
 * surface.  This the tip is at a barycentric coordinate so that it may be shown on 
 * any surface configuration.
 */
void
BrainModelSurface::createDeformationField(const BrainModelSurface* deformedSurfaceIn,
                                          const int columnNumberIn,
                                          const QString& columnName,
                                          DeformationFieldFile& dff) const 
{
   const int numNodes = getNumberOfNodes();
   if (numNodes != deformedSurfaceIn->getNumberOfNodes()) {
      return;
   }
   
   //
   // Make a copy of the deformed surface since it will get modified in this method
   //
   BrainModelSurface deformedSurface(*deformedSurfaceIn);
   
   //
   // Scale deformed sphere to same radius as normal sphere
   //
   deformedSurface.convertToSphereWithRadius(getSphericalSurfaceRadius());
   
   //
   // Setup deformation field file
   //
   int columnNumber = columnNumberIn;
   if (dff.getNumberOfColumns() <= 0) {
      dff.setNumberOfNodesAndColumns(numNodes, 1);
      columnNumber = 0;
   }
   else if (dff.getNumberOfNodes() != numNodes) {
      return;
   }
   else if ((columnNumber < 0) || (columnNumber >= dff.getNumberOfColumns())) {
      dff.addColumns(1);
      columnNumber = dff.getNumberOfColumns() - 1;
   }
   dff.setColumnName(columnNumber, columnName);
   
   dff.setDeformedTopologyFileName(columnNumber,
                                   FileUtilities::basename(getTopologyFile()->getFileName()));
   dff.setTopologyFileName(columnNumber,
                           FileUtilities::basename(getTopologyFile()->getFileName()));
   const CoordinateFile* deformedCoords = deformedSurface.getCoordinateFile();
   dff.setDeformedCoordinateFileName(columnNumber,
                                     FileUtilities::basename(deformedCoords->getFileName()));
   
   const CoordinateFile* origCoords = getCoordinateFile();
   dff.setPreDeformedCoordinateFileName(columnNumber,
                                        FileUtilities::basename(origCoords->getFileName()));
   dff.setCoordinateFileName(columnNumber,
                             FileUtilities::basename(origCoords->getFileName()));
   
   //
   // Create a point projector
   //
   BrainModelSurfacePointProjector bspp(this,
                                        BrainModelSurfacePointProjector::SURFACE_TYPE_HINT_SPHERE,
                                        false);
   
   //
   // for each node
   //
   for (int i = 0; i < numNodes; i++) {
      const float* xyz = deformedCoords->getCoordinate(i);
      
      //
      // Project deformed node position on original coord file
      //
      int nearestNode = -1;
      int tileNodes[3];
      float tileAreas[3];
      const int tile = bspp.projectBarycentric(xyz, nearestNode, tileNodes, tileAreas);
      
      if (tile < 0) {
         if (nearestNode >= 0) {
            tileNodes[0] = nearestNode;
            tileNodes[1] = nearestNode;
            tileNodes[2] = nearestNode;
            tileAreas[0] = 1.0;
            tileAreas[1] = 1.0;
            tileAreas[2] = 1.0;
         }
      }
      DeformationFieldNodeInfo* dffi = dff.getDeformationInfo(i, columnNumber);
      dffi->setData(tileNodes, tileAreas);
      
/*      
      if ((i > 20000) && (i < 20100)) {
         float pos[3];
         BrainModelSurfacePointProjector::unprojectPoint(tileNodes, tileAreas, origCoords, pos);
         std::cout << "before proj: " << xyz[0] << ", " << xyz[1] << ", " << xyz[2]
                   << " after: " << pos[0] << ", " << pos[1] << ", " << pos[2] << std::endl;
      }
*/
   }
}      

/**
 * Create deformation field vectors for an ATLAS surface.
 * 
 * For each node in "this" surface:
 *  1) Project it to the indivDeformSurface producing barycentric coordinates
 *  2) Apply the barycentric coordinates using the indivSourceSurface
 *  3) Project back to "this" surface and store the barycentric coordinates.
 */
void
BrainModelSurface::createDeformationField(const BrainModelSurface* indivSourceSurfaceIn,
                                          const BrainModelSurface* indivDeformSurfaceIn,
                                          const int columnNumberIn,
                                          const QString& columnName,
                                          DeformationFieldFile& dff) const 
{
   if (indivSourceSurfaceIn->getNumberOfNodes() != indivDeformSurfaceIn->getNumberOfNodes()) {
      return;
   }
   
   //
   // Make a copy of the indiv surfaces since they will get modified in this method
   //
   BrainModelSurface indivSourceSurface(*indivSourceSurfaceIn);
   BrainModelSurface indivDeformedSurface(*indivDeformSurfaceIn);
   
   //
   // Scale indiv spheres to same radius as "this" sphere
   //
   indivSourceSurface.convertToSphereWithRadius(getSphericalSurfaceRadius());
   indivDeformedSurface.convertToSphereWithRadius(getSphericalSurfaceRadius());
   
   const int numNodes = getNumberOfNodes();

   //
   // Setup deformation field file
   //
   int columnNumber = columnNumberIn;
   if (dff.getNumberOfColumns() <= 0) {
      dff.setNumberOfNodesAndColumns(numNodes, 1);
      columnNumber = 0;
   }
   else if (dff.getNumberOfNodes() != numNodes) {
      return;
   }
   else if ((columnNumber < 0) || (columnNumber >= dff.getNumberOfColumns())) {
      dff.addColumns(1);
      columnNumber = dff.getNumberOfColumns() - 1;
   }
   dff.setColumnName(columnNumber, columnName);

   const TopologyFile* indivTopoFile = indivDeformedSurface.getTopologyFile();
   if (indivTopoFile != NULL) {    
      dff.setDeformedTopologyFileName(columnNumber,
                                      FileUtilities::basename(indivTopoFile->getFileName()));
   }
   
   if (topology == NULL) {
      return;
   }
   dff.setTopologyFileName(columnNumber,
                        FileUtilities::basename(topology->getFileName()));

   const CoordinateFile* deformedCoords = indivDeformedSurface.getCoordinateFile();
   dff.setDeformedCoordinateFileName(columnNumber,
                                     FileUtilities::basename(deformedCoords->getFileName()));
   
   const CoordinateFile* indivCoords = indivSourceSurface.getCoordinateFile();
   dff.setPreDeformedCoordinateFileName(columnNumber,
                                        FileUtilities::basename(indivCoords->getFileName()));
   const CoordinateFile* coords = getCoordinateFile();
   dff.setCoordinateFileName(columnNumber,
                             FileUtilities::basename(coords->getFileName()));
   
   //
   // Create a point projector for indiv deformed surface
   //
   BrainModelSurfacePointProjector indivDeformedProjector(&indivDeformedSurface,
                                        BrainModelSurfacePointProjector::SURFACE_TYPE_HINT_SPHERE,
                                        false);
   
   //
   // Create a point projector for "this" surface
   //
   BrainModelSurfacePointProjector thisProjector(this,
                                        BrainModelSurfacePointProjector::SURFACE_TYPE_HINT_SPHERE,
                                        false);
   
   //
   // Topology Helper
   //
   const TopologyHelper* th = topology->getTopologyHelper(false, true, false);
   
   //
   // for each node
   //
   for (int i = 0; i < numNodes; i++) {
      int tileNodes[3] = { -1, -1, -1 };
      float tileAreas[3] = { 0.0, 0.0, 0.0 };

      //
      // If node in this surface has neighbors
      //
      if (th->getNodeHasNeighbors(i)) {
         //
         // Get coordinate in this surface
         //   
         float xyz[3];
         coords->getCoordinate(i, xyz);
         
         //
         // Project node onto deformed surface
         //
         int nearestNode = -1;
         const int tile = indivDeformedProjector.projectBarycentric(xyz, nearestNode, tileNodes, tileAreas);
         
         if (tile < 0) {
            if (nearestNode >= 0) {
               tileNodes[0] = nearestNode;
               tileNodes[1] = nearestNode;
               tileNodes[2] = nearestNode;
               tileAreas[0] = 1.0;
               tileAreas[1] = 1.0;
               tileAreas[2] = 1.0;
            }
         }
         
         //
         // If not projected correctly
         //
         if (tileNodes[0] > 0) {
            //
            // Unproject onto indiv source surface
            //
            BrainModelSurfacePointProjector::unprojectPoint(tileNodes,
                                                            tileAreas,
                                                            indivCoords,
                                                            xyz);
            
            //
            // Project onto this surface
            //
            thisProjector.projectBarycentric(xyz, nearestNode, tileNodes, tileAreas);
         }
      }
      
      //
      // Add to deformation field
      //
      DeformationFieldNodeInfo* dffi = dff.getDeformationInfo(i, columnNumber);
      dffi->setData(tileNodes, tileAreas);
   }
}

/**
 * simplify the surface to a fewer number of polygons.
 */
vtkPolyData* 
BrainModelSurface::simplifySurface(const int maxPolygons) const
{
   //
   // See if polygons need to be reduced
   //
   const int numTriangles = topology->getNumberOfTiles();
   if (numTriangles < maxPolygons) {
      //
      // Number of polygons is already less than desired
      //
      return NULL;
   }
   
   //
   // Convert to vtk's poly data format
   //
   vtkPolyData* inputPolyData = convertToVtkPolyData();
   
   //
   // If conversion to vtk's polydata format successful
   //
   if (inputPolyData != NULL) {
      //
      // Set up to Decimate the polygon data
      //
      vtkDecimatePro* decimater = vtkDecimatePro::New();
#ifdef HAVE_VTK6
      decimater->SetInputData(inputPolyData);
#else
      decimater->SetInput(inputPolyData);
#endif
      const float reduction = 1.0 - ((float)maxPolygons /
                                     (float)numTriangles);
      if (DebugControl::getDebugOn()) {
         std::cout << "Reduction is " << reduction * 100.0 << "%" << std::endl;
      }
      const double errorVal = VTK_DOUBLE_MAX; //1.0;
#ifdef HAVE_VTK6
      decimater->SetInputData(inputPolyData);
#else
      decimater->SetInput(inputPolyData);
#endif
      decimater->SetTargetReduction(reduction);
      decimater->PreserveTopologyOff(); //On();
      decimater->SetFeatureAngle(30);
      decimater->SplittingOff();
      decimater->PreSplitMeshOff();
      decimater->SetMaximumError(errorVal);
      decimater->BoundaryVertexDeletionOff();
      decimater->SetDegree(25);
      decimater->AccumulateErrorOn();
      decimater->SetAbsoluteError(errorVal);
      decimater->SetErrorIsAbsolute(1);
      
      //
      // Compute normals on the surface
      //
      vtkPolyDataNormals* vtkNormals = vtkPolyDataNormals::New();
#ifdef HAVE_VTK6
      vtkNormals->SetInputConnection(decimater->GetOutputPort());
#else
      vtkNormals->SetInput(decimater->GetOutput());
#endif
      vtkNormals->SplittingOff();
      vtkNormals->ConsistencyOn();
      vtkNormals->ComputePointNormalsOn();
      vtkNormals->NonManifoldTraversalOn();
      vtkNormals->Update();

      vtkPolyData* polyOut = vtkPolyData::New();
      polyOut->DeepCopy(vtkNormals->GetOutput());
      
      decimater->Delete();
      vtkNormals->Delete();
      inputPolyData->Delete();
      
      return polyOut;
   }
   
   return NULL;
}      

/**
 * get node closest to point.
 */
int 
BrainModelSurface::getNodeClosestToPoint(const float pointXYZ[3]) const
{
   int nearestNodeNumber = -1;
   float nearestNodeDistance = std::numeric_limits<float>::max();;
   
   const TopologyHelper* th = topology->getTopologyHelper(false, true, false);
   const int num = coordinates.getNumberOfNodes();
   for (int i = 0; i < num; i++) {
      if (th->getNodeHasNeighbors(i)) {
         const float* xyz = coordinates.getCoordinate(i);
         const float dist = MathUtilities::distanceSquared3D(xyz,
                                                             pointXYZ);
         if (dist < nearestNodeDistance) {
            nearestNodeNumber = i;
            nearestNodeDistance = dist;
         } 
      }
   }
   
   return nearestNodeNumber;
}      

/**
 * Create a string of c-language arrays containing vertices, normals, triangles.
 */
QString
BrainModelSurface::convertToCLanguageArrays() const
{
   //
   // Get the number of vertices and triangles
   //
   const CoordinateFile* cf = this->getCoordinateFile();
   const int numVertices = cf->getNumberOfCoordinates();
   const TopologyFile* tf = this->getTopologyFile();
   const int numTriangles = tf->getNumberOfTiles();
   if ((numVertices <= 0) ||
       (numTriangles <= 0)) {
      return "";
   }

   //
   // Estimate storage to avoid excessive memory reallocations
   //
   const int charactersPerVertex = 8;
   const int charactersPerTriangle = 4;
   const int estimatedStorage = charactersPerVertex * 2 * numVertices
                              + charactersPerTriangle * numTriangles;
   QString s;
   s.reserve(estimatedStorage);

   //
   // Wrap a text stream around the string
   //
   QTextStream stream(&s, QIODevice::WriteOnly);

   //
   // Add number of vertices and triangles to string
   //
   stream << "const int numVertices = " << numVertices << ";\n";
   stream << "const int numTriangles = " << numTriangles << ";\n";
   stream << "\n";

   //
   // Write the vertices
   //
   stream << "float vertices[] = {\n";
   for (int i = 0; i < numVertices; i++) {
      const float* xyz = cf->getCoordinate(i);
      stream << "   " << xyz[0] << ", " << xyz[1] << ", " << xyz[2];
      if ((i + 1) < numVertices) {
         stream << ",";
      }
      stream << "\n";
   }
   stream << "};\n\n";

   //
   // Write the normals
   //
   stream << "float normals[] = {\n";
   for (int i = 0; i < numVertices; i++) {
      const float* xyz = this->getNormal(i);
      stream << "   " << xyz[0] << ", " << xyz[1] << ", " << xyz[2];
      if ((i + 1) < numVertices) {
         stream << ",";
      }
      stream << "\n";
   }
   stream << "};\n\n";

   //
   // Write the triangles
   //
   stream << "int triangles[] = {\n";
   for (int i = 0; i < numTriangles; i++) {
      const int* tile = tf->getTile(i);
      stream << "   " << tile[0] << ", " << tile[1] << ", " << tile[2];
      if ((i + 1) < numTriangles) {
         stream << ",";
      }
      stream << "\n";
   }
   stream << "};\n\n";

   return s;
}


