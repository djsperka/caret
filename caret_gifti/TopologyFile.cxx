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

#include <cstdio>
#include <iostream>
#include <stack>
#include <QMutexLocker>

#include "DebugControl.h"
#include "FileUtilities.h"
#include "GiftiCommon.h"
#include "MniObjSurfaceFile.h"
#include "NodeRegionOfInterestFile.h"
#include "PaintFile.h"
#include "SpecFile.h"
#include "TopologyFile.h"
#include "TopologyHelper.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkTriangleFilter.h"

/**
 * The constructor.
 */
TopologyFile::TopologyFile()
   : GiftiDataArrayFile("Topology File",
                        GiftiCommon::intentTopologyTriangles,
                        GiftiDataArray::DATA_TYPE_INT32, 
                        SpecFile::getTopoFileExtension(), 
                        FILE_FORMAT_ASCII, 
                        FILE_IO_READ_AND_WRITE, 
                        FILE_IO_READ_AND_WRITE, 
                        FILE_IO_NONE,
                        FILE_IO_NONE,
                        false)
{
   topologyHelper = NULL;
   clear();
}

/**
 * The copy constructor.
 */
TopologyFile::TopologyFile(const TopologyFile& tf)
   : GiftiDataArrayFile(tf)
{
   copyHelperTopology(tf);
}

/**
 * The destructor.
 */
TopologyFile::~TopologyFile()
{
   clear();
}

/**
 * copy helper used by copy constructor and assignment operator.
 */
void 
TopologyFile::copyHelperTopology(const TopologyFile& tf)
{
   topologyHelper = NULL;
   topologyHelperNeedsRebuild = false;
   nodeSections = tf.nodeSections;
   numberOfNodes = tf.numberOfNodes;
   setFileName("");
   setModified();
}

/**
 * get the number of tiles.
 */
int 
TopologyFile::getNumberOfTiles() const 
{ 
   if (dataArrays.empty()) {
      return 0;
   }
   return dataArrays[0]->getNumberOfRows();
}

/**
 * assignment operator.
 */
TopologyFile& 
TopologyFile::operator=(const TopologyFile& tf)
{
   if (&tf != this) {
      GiftiDataArrayFile::operator=(tf);
      copyHelperTopology(tf);
   }
   
   return *this;
}
      
/**
 * Add a tile.
 */
void
TopologyFile::addTile(const int v1, const int v2, const int v3)
{
   
   if (getNumberOfDataArrays() == 0) {
      std::vector<int> dim;
      dim.push_back(1);
      dim.push_back(3);
      GiftiDataArray* nda = new GiftiDataArray(this,
                                               GiftiCommon::intentTopologyTriangles,
                                               GiftiDataArray::DATA_TYPE_INT32,
                                               dim);
      addDataArray(nda);
      int32_t* tiles = nda->getDataPointerInt();
      tiles[0] = v1;
      tiles[1] = v2;
      tiles[2] = v3;
   }
   else {
      const int numRows = dataArrays[0]->getNumberOfRows();
      dataArrays[0]->addRows(1);
      int32_t* tiles = dataArrays[0]->getDataPointerInt();
      const int indx = numRows * 3;
      tiles[indx]   = v1;
      tiles[indx+1] = v2;
      tiles[indx+2] = v3;
   }
   topologyHelperNeedsRebuild = true;
   setModified();
   
   // add one to node since node number range is [0..N-1]
   numberOfNodes = std::max(v1+1, numberOfNodes);
   numberOfNodes = std::max(v2+1, numberOfNodes);
   numberOfNodes = std::max(v3+1, numberOfNodes);
}

/**
 * Add a tile.
 */
void
TopologyFile::addTile(const int v[3])
{
   addTile(v[0], v[1], v[2]);
}

/**
 * Clear the Topology File.
 */
void
TopologyFile::clear()
{
   GiftiDataArrayFile::clear();
   numberOfNodes = 0;
   //nodes.clear();
   nodeSections.clear();
   setHeaderTag(AbstractFile::headerTagPerimeterID, "UNKNOWN");
   if (topologyHelper != NULL) {
      delete topologyHelper;
      topologyHelper = NULL;
   }
   topologyHelperNeedsRebuild = true;
}

/**
 * Set the number of nodes in coordinate files using this topology file.
 */
void
TopologyFile::setNumberOfNodes(const int num) 
{
   // Note: this "set method" does not modify the topology file so do not call setModified()
   numberOfNodes = std::max(num, numberOfNodes);
   topologyHelperNeedsRebuild = true;
}

/**
 * Get the perimeter ID from a topology type
 */
QString 
TopologyFile::getPerimeterIDFromTopologyType(TOPOLOGY_TYPES tt)
{
   QString pid("UNKNOWN");
   
   switch(tt) {
      case TOPOLOGY_TYPE_CLOSED:
         pid = "CLOSED";
         break;
      case TOPOLOGY_TYPE_OPEN:
         pid = "OPEN";
         break;
      case TOPOLOGY_TYPE_CUT:
         pid = "CUT";
         break;
      case TOPOLOGY_TYPE_LOBAR_CUT:
         pid = "LOBAR_CUT";
         break;
      case TOPOLOGY_TYPE_UNKNOWN:
      case TOPOLOGY_TYPE_UNSPECIFIED:
         pid = "UNKNOWN";
         break;
   }
   return pid;
}

/** 
 * Get spec file tag from topology type
 */
QString
TopologyFile::getSpecFileTagFromTopologyType(const TOPOLOGY_TYPES tt)
{
   QString tag(SpecFile::getUnknownTopoFileMatchTag());
   
   switch(tt) {
      case TOPOLOGY_TYPE_CLOSED:
         tag = SpecFile::getClosedTopoFileTag();
         break;
      case TOPOLOGY_TYPE_OPEN:
         tag = SpecFile::getOpenTopoFileTag();
         break;
      case TOPOLOGY_TYPE_CUT:
         tag = SpecFile::getCutTopoFileTag();
         break;
      case TOPOLOGY_TYPE_LOBAR_CUT:
         tag = SpecFile::getLobarCutTopoFileTag();
         break;
      case TOPOLOGY_TYPE_UNKNOWN:
      case TOPOLOGY_TYPE_UNSPECIFIED:
         tag = SpecFile::getUnknownTopoFileMatchTag();
         break;
   }
   
   return tag;
}

/**
 * get the topology type from a perimeter ID
 */
TopologyFile::TOPOLOGY_TYPES 
TopologyFile::getTopologyTypeFromPerimeterID(const QString pid)
{
   TOPOLOGY_TYPES tt = TOPOLOGY_TYPE_UNKNOWN;
   if (pid == "CLOSED") {
      tt = TOPOLOGY_TYPE_CLOSED;
   }
   else if (pid == "OPEN") {
      tt = TOPOLOGY_TYPE_OPEN;
   }
   else if (pid == "CUT") {
      tt = TOPOLOGY_TYPE_CUT;
   }
   else if (pid == "LOBAR_CUT") {
      tt = TOPOLOGY_TYPE_LOBAR_CUT;
   }
   return tt;
}

/**
 * Set the topology type.
 */
void
TopologyFile::setTopologyType(const TOPOLOGY_TYPES tt)
{
   setModified();
   QString topoTypeName;

   switch(tt) {
      case TOPOLOGY_TYPE_CLOSED:
        topoTypeName = "CLOSED";
        break;
      case TOPOLOGY_TYPE_OPEN:
        topoTypeName = "OPEN";
        break;
      case TOPOLOGY_TYPE_CUT:
        topoTypeName = "CUT";
        break;
      case TOPOLOGY_TYPE_LOBAR_CUT:
        topoTypeName = "LOBAR_CUT";
        break;
      case TOPOLOGY_TYPE_UNKNOWN:
         topoTypeName = "UNKNOWN";
         break;
      case TOPOLOGY_TYPE_UNSPECIFIED:
      default:
        topoTypeName = "UNSPECIFIED";
        break;
   }
   setHeaderTag(AbstractFile::headerTagPerimeterID, topoTypeName);
}

/**
 * Get a descriptive name of a file (type followed by basename)
 */
QString
TopologyFile::getDescriptiveName() const
{
   QString name(getTopologyTypeName());
   const QString fn(getFileName());
   if (fn.isEmpty() == false) {
      name.append(" ");
      name.append(FileUtilities::basename(fn));
   }
   return name;
}

/**
 * Get the name of the type of this topology file.
 */
QString
TopologyFile::getTopologyTypeName() const
{
   const QString topoTypeName = getHeaderTag(AbstractFile::headerTagPerimeterID);
   if (topoTypeName.isEmpty()) {
      return "UNKNOWN";
   }
   return topoTypeName;
}

/**
 * Get the topology type
 */
TopologyFile::TOPOLOGY_TYPES
TopologyFile::getTopologyType() const
{
   const QString topoTypeName = getHeaderTag(AbstractFile::headerTagPerimeterID);
   if (topoTypeName == "CLOSED") {
      return TOPOLOGY_TYPE_CLOSED;
   }
   else if (topoTypeName == "OPEN") {
      return TOPOLOGY_TYPE_OPEN;
   } 
   else if (topoTypeName == "CUT") {
      return TOPOLOGY_TYPE_CUT;
   }
   else if (topoTypeName == "LOBAR_CUT") {
      return TOPOLOGY_TYPE_LOBAR_CUT;
   }
   else if (topoTypeName == "UNKNOWN") {
      return TOPOLOGY_TYPE_UNKNOWN;
   }
   else {
      return TOPOLOGY_TYPE_UNSPECIFIED;
   }
}

/**
 * delete tiles that use both of these nodes.
 */
void 
TopologyFile::deleteTilesWithEdge(const int node1, const int node2)
{
   const int numTiles = getNumberOfTiles();
   if (numTiles <= 0) {
      return;
   }
   
   std::vector<int> rowsToDelete;
   
   for (int i = 0; i < numTiles; i++) {
      int v1, v2, v3;
      getTile(i, v1, v2, v3);
      
      bool keepTile = true;
      
      if ((v1 == node1) &&
          (v2 == node2)) {
         keepTile = false;
      }
      else if ((v2 == node1) &&
               (v1 == node2)) {
         keepTile = false;
      }
      else if ((v2 == node1) &&
               (v3 == node2)) {
         keepTile = false;
      }
      else if ((v3 == node1) &&
               (v2 == node2)) {
         keepTile = false;
      }
      else if ((v1 == node1) &&
               (v3 == node2)) {
         keepTile = false;
      }
      else if ((v3 == node1) &&
               (v1 == node2)) {
         keepTile = false;
      }
      
      if (keepTile == false) {
         rowsToDelete.push_back(i);
      }
   }   
   
   if (rowsToDelete.empty() == false) {
      dataArrays[0]->deleteRows(rowsToDelete);
      topologyHelperNeedsRebuild = true;
   }
}      

/*
 ** Delete tiles that use any of the marked nodes
 */
void TopologyFile::deleteTilesWithMarkedNodes(const std::vector<bool>& markedNodes)
{
   const int numNodes = static_cast<int>(markedNodes.size());
   const int numTiles = getNumberOfTiles();
   if (numTiles <= 0) {
      return;
   }
   
   std::vector<int> rowsToDelete;
   
   for (int i = 0; i < numTiles; i++) {
      int v1, v2, v3;
      getTile(i, v1, v2, v3);
      
      bool deleteTile = false;
      if (v1 < numNodes) {
         if (markedNodes[v1]) {
            deleteTile = true;
         }
      }
      if (v2 < numNodes) {
         if (markedNodes[v2]) {
            deleteTile = true;
         }
      }
      if (v3 < numNodes) {
         if (markedNodes[v3]) {
            deleteTile = true;
         }
      }
      
      if (deleteTile) {
         rowsToDelete.push_back(i);
      }
   }   
   
   if (rowsToDelete.empty() == false) {
      dataArrays[0]->deleteRows(rowsToDelete);
      topologyHelperNeedsRebuild = true;
   }
}      

/**
 * Delete tiles with the specified indices.
 */
void
TopologyFile::deleteTiles(const std::vector<int>& tilesToDelete)
{
   if (tilesToDelete.empty() == false) {
      dataArrays[0]->deleteRows(tilesToDelete);
      topologyHelperNeedsRebuild = true;
   }
}

/**
 * Get Edges with 3 or more tiles assigned to them.
 */
void
TopologyFile::getDegenerateEdges(int& numDegenerateEdges, 
                   std::vector<int>& nodesUsedByDegenerateEdges) const
{
   numDegenerateEdges = 0;
   nodesUsedByDegenerateEdges.clear();
   
   std::set<int> degenNodes;
   
   const TopologyHelper* th = getTopologyHelper(true, true, true);
   std::set<TopologyEdgeInfo> edges = th->getEdgeInfo();
   
   for (std::set<TopologyEdgeInfo>::iterator iter = edges.begin();
        iter != edges.end(); iter++) {
      if (iter->getEdgeUsedByMoreThanTwoTriangles()) {
         numDegenerateEdges++;
         int n1, n2;
         iter->getNodes(n1, n2);
         degenNodes.insert(n1);
         degenNodes.insert(n2);
      }
   }
   
   nodesUsedByDegenerateEdges.insert(nodesUsedByDegenerateEdges.end(),
                                     degenNodes.begin(), degenNodes.end());
}

/**
 * Get the euler count (V - E + F = 2 for closed surface).
 * For more info: http://mathworld.wolfram.com/PoincareFormula.html
 *                http://mathworld.wolfram.com/EulerCharacteristic.html
 */
void 
TopologyFile::getEulerCount(const bool twoDimFlag, int& numFaces, 
                            int& numVertices, int& numEdges,
                            int& eulerCount, int& numHoles, int& numObjects) const
{
   numFaces = getNumberOfTiles();
   const TopologyHelper* th = getTopologyHelper(true, true, false);
   numVertices = 0;
   const int numNodes = th->getNumberOfNodes();
   for (int i = 0; i < numNodes; i++) {
      const int numNeighbors = th->getNodeNumberOfNeighbors(i);
      if (numNeighbors > 0) {
         numVertices++;
      }
   }
   numObjects = getNumberOfDisjointObjects();
   numEdges = th->getNumberOfEdges();
   eulerCount = numVertices - numEdges + numFaces;
   if (twoDimFlag) {
      numHoles = 1 - eulerCount;
   }
   else {
      numHoles = 1 - (eulerCount / 2);   //(eulerCount - 2) / (-2);
   }
}

/**
 * get non-manifold nodes (nodes whose triangles do not share and edge).
 */
void 
TopologyFile::getNonManifoldNodes(std::vector<int>& nodesOut) const
{
   nodesOut.clear();

   //
   // Get this topology file's topology helper (DO NOT DELETE IT)
   //
   const TopologyHelper* topologyHelper = getTopologyHelper(true, true, true);

   const int numNodes = getNumberOfNodes();
   std::vector<int> edgesPerNode(numNodes);
   topologyHelper->getNumberOfBoundaryEdgesForAllNodes(edgesPerNode);
   
   //
   // Loop through nodes
   //
   for (int i = 0; i < numNodes; i++) {
      //
      // Non-manifold nodes have 4 or more boundary edges
      //
      if (edgesPerNode[i] >= 4) {
         nodesOut.push_back(i);
      }
   }
}

/**
 * get number of disjoint objects.
 */
int 
TopologyFile::getNumberOfDisjointObjects() const
{
   std::vector<int> islandRoot, islandNumNodes, islandRootNeighbor;
      
   const int numSurfaceObjects = findIslands(islandRoot, islandNumNodes, islandRootNeighbor);
   
   return numSurfaceObjects;
}
      
/**
 * Find islands (number of disjoint groups of nodes).
 * Returns number disjoint connected pieces of surface.
 * Return value of 1 indicates no islands (one connected piece of surface).
 * Return value of 0 indicates no topology.
 * Return value greater than 1 indicates islands.
 *
 * islandRootNode - contains a node in the piece of surface and the number of 
 *                  elements is the number returned by this method.
 * islandNumNodes - is the number of nodes in the piece of surface and the 
 *                  number of elements is the number returned by this method.
 * nodeRootNeighbor - is the "islandRootNode" for the node "i" and the number
 *                    of elements is the number of nodes in the surface.
 */
int
TopologyFile::findIslands(std::vector<int>& islandRootNode,
                          std::vector<int>& islandNumNodes,
                          std::vector<int>& nodeRootNeighbor) const
{
   islandRootNode.clear();
   islandNumNodes.clear();
   
   //
   // Get this topology file's topology helper (DO NOT DELETE IT)
   //
   const TopologyHelper* topologyHelper = getTopologyHelper(false, true, false);
   
   const int numNodes = topologyHelper->getNumberOfNodes();
   
   if (numNodes == 0) {
      return 0;
   }
   
   if (numNodes == 1) {
      for (int i = 0; i < numNodes; i++) {
         if (topologyHelper->getNodeHasNeighbors(i)) {
            islandRootNode.push_back(i);
            islandNumNodes.push_back(1);
            nodeRootNeighbor.push_back(i);
            break;
         }
      }
      return static_cast<int>(islandRootNode.size());
   }
   
   nodeRootNeighbor.resize(numNodes);
   std::fill(nodeRootNeighbor.begin(), nodeRootNeighbor.end(), -1);

   std::vector<int> numConnectedNeighbors(numNodes, 0);
   std::vector<int> visited(numNodes, 0);
   
   //
   // Mark all nodes without neighbors as visited
   //
   for (int i = 0; i < numNodes; i++) {
      if (topologyHelper->getNodeHasNeighbors(i) == false) {
         visited[i] = 1;
      }
   } 
 
   //
   // Search the surface marking all connected nodes.
   //
   for (int n = 0; n < numNodes; n++) {
      if (visited[n] == 0) {
         
         const int nodeNumberIn = n;
         const int origNeighbor = n;
         
         std::stack<int> st;
         st.push(nodeNumberIn);
         
         while(!st.empty()) {
            const int nodeNumber = st.top();
            st.pop();
            
            if (visited[nodeNumber] == 0) {
               visited[nodeNumber] = 1;
               nodeRootNeighbor[nodeNumber] = origNeighbor;
               numConnectedNeighbors[origNeighbor]++;
               std::vector<int> neighbors;
               topologyHelper->getNodeNeighbors(nodeNumber, neighbors);
               for (int i = 0; i < static_cast<int>(neighbors.size()); i++) {
                  const int node = neighbors[i];
                  if (visited[node] == 0) {
                     st.push(node);
                  }
               }
            }
         }
      }
   }
   
   //
   // set the islands
   //
   for (int j = 0; j < numNodes; j++) {
      if (numConnectedNeighbors[j] > 0) {
         islandRootNode.push_back(j);
         islandNumNodes.push_back(numConnectedNeighbors[j]);
         if (DebugControl::getDebugOn()) {
            std::cout << j << " is connected to " << numConnectedNeighbors[j]
                      << " nodes." << std::endl;
         }
      }
   }   
   
   return static_cast<int>(islandRootNode.size());
}

/**
 * disconnect nodes that are in the region of interest.
 */
void 
TopologyFile::disconnectNodesInRegionOfInterest(const NodeRegionOfInterestFile& roiFile) throw (FileException)
{
   //
   // Determine nodes that should be disconnected
   //
   const int numNodes = roiFile.getNumberOfNodes();
   std::vector<bool> disconnectNodeFlags(numNodes, false);
   for (int i = 0; i < numNodes; i++) {
      if (roiFile.getNodeSelected(i)) {
         disconnectNodeFlags[i] = true;
      }
   }
   
   //
   // Disonnect the nodes by removing tiles
   //
   deleteTilesWithMarkedNodes(disconnectNodeFlags);
}

/**
 * disconnect nodes that are labeled with the specified name and in the specified column.
 */
void 
TopologyFile::disconnectNodesUsingPaint(const PaintFile& paintFile,
                                        const int paintColumn,
                                        const QString& paintName) throw (FileException)
{
   //
   // Check inputs
   //
   if ((paintColumn < 0) ||
       (paintColumn >= paintFile.getNumberOfColumns())) {
      throw FileException("Invalid paint column number");
   }   
   if (paintName.isEmpty()) {
      throw FileException("Paint name is empty.");
   }
   const int paintIndex = paintFile.getPaintIndexFromName(paintName);
   if (paintIndex < 0) {
      throw FileException("Paint name not found in paint file.");
   }
   
   //
   // Determine nodes that should be disconnected
   //
   const int numNodes = paintFile.getNumberOfNodes();
   std::vector<bool> disconnectNodeFlags(numNodes, false);
   for (int i = 0; i < numNodes; i++) {
      if (paintFile.getPaint(i, paintColumn) == paintIndex) {
         disconnectNodeFlags[i] = true;
      }
   }
   
   //
   // Disonnect the nodes by removing tiles
   //
   deleteTilesWithMarkedNodes(disconnectNodeFlags);
}
                                     
/**
 * Disconnect islands (retain largest number of connected nodes; disconnect others).
 * Returns the number of islands that were disconnected.
 */
int 
TopologyFile::disconnectIslands()
{
   //
   // Find the islands
   //
   std::vector<int> islandRootNode;
   std::vector<int> islandNumNodes;
   std::vector<int> nodeRootNeighbor;
   const int numPieces = findIslands(islandRootNode, islandNumNodes, nodeRootNeighbor);
   
   //
   // See if there are any islands
   //
   if (numPieces <= 1) {
      return 0;
   }
   
   const int numNodes = static_cast<int>(nodeRootNeighbor.size());

   //
   // find node with most connected neighbors
   //
   int mostNeighborsNode = -1;
   int mostNeighbors = 0;
   for (int j = 0; j < numPieces; j++) {
      if (islandNumNodes[j] > 0) {
         if (DebugControl::getDebugOn()) {
            std::cout << islandRootNode[j] << " is connected to " << islandNumNodes[j]
                      << " nodes." << std::endl;
         }
      }
      if (islandNumNodes[j] > mostNeighbors) {
         mostNeighborsNode = islandRootNode[j];
         mostNeighbors     = islandNumNodes[j];
      }
   }
   
   if (DebugControl::getDebugOn()) {
      std::cout << mostNeighborsNode << " has the most neighbors = "
                << mostNeighbors << std::endl;
   }
   
   //
   // Disconnect all nodes that are not 
   // connected to the node with the most connected neighbors
   //
   std::vector<bool> markedNodes(numNodes, false);
   if (mostNeighborsNode >= 0) {
      for (int i = 0; i < numNodes; i++) {
         if (nodeRootNeighbor[i] != mostNeighborsNode) {
            markedNodes[i] = true;
         }
      }
   }
   deleteTilesWithMarkedNodes(markedNodes);   
   topologyHelperNeedsRebuild = true;
   setModified();
   
   return numPieces - 1;
}

/**
 * Find corner tiles (corners are tiles that have one or more nodes that are only in one tile).
 */
void
TopologyFile::findCornerTiles(const int numCornerNodes, std::vector<int>& cornerTiles) const
{
   cornerTiles.clear();
   
   //
   // Topology helper needs to be updated each cycle since tiles get deleted
   //
   const TopologyHelper* th = getTopologyHelper(false, true, false);

   //
   // Check all nodes
   //
   const int numTiles = getNumberOfTiles();
   for (int i = 0; i < numTiles; i++) {
      //
      // Get the nodes in the tile
      //
      int nodes[3];
      getTile(i, nodes);
      
      //
      // count the number of corner nodes in the tile
      //
      int numCornerNodesInTile = 0;
      for (int j = 0; j < 3; j++) {
         if (th->getNodeNumberOfNeighbors(nodes[j]) == 2) {
            numCornerNodesInTile++;
         }
      }
      
      //
      // If only one tile is used by the node then it must be a corner node
      //
      if (numCornerNodesInTile > 0) {
         if (numCornerNodesInTile >= numCornerNodes) {
            cornerTiles.push_back(i);
         }
      }
   }      
}
 
/**
 * Remove corners (corners are tiles that have one or more nodes that are only in one tile).
 */
int 
TopologyFile::removeCornerTiles(const int numCornerNodes)
{
   int tilesDeletedCount = 0;
   
   //
   // May need multiple iterations due to corners revealing additional corners
   //
   bool cornersFound = true;
   while (cornersFound) {
      cornersFound = false;
      
      //
      // Topology helper needs to be updated each cycle since tiles get deleted
      //
      const TopologyHelper* th = getTopologyHelper(false, true, false);
   
      //
      // Keep track of tiles that need to be deleted
      //
      std::vector<int> tilesToDelete;

      //
      // Check all nodes
      //
      const int numTiles = getNumberOfTiles();
      for (int i = 0; i < numTiles; i++) {
         //
         // Get the nodes in the tile
         //
         int nodes[3];
         getTile(i, nodes);
         
         //
         // count the number of corner nodes in the tile
         //
         int numCornerNodesInTile = 0;
         for (int j = 0; j < 3; j++) {
            if (th->getNodeNumberOfNeighbors(nodes[j]) == 2) {
               numCornerNodesInTile++;
            }
         }
         
         //
         // If only one tile is used by the node then it must be a corner node
         //
         if (numCornerNodesInTile > 0) {
            if (numCornerNodesInTile >= numCornerNodes) {
               tilesToDelete.push_back(i);
            }
         }
      }
      
      //
      // If corner tiles are found
      //
      if (tilesToDelete.empty() == false) {
         deleteTiles(tilesToDelete);
         cornersFound = true;
         tilesDeletedCount += tilesToDelete.size();
      }
   }
   
   if (DebugControl::getDebugOn()) {
      std::cout << tilesDeletedCount << " corner tiles were deleted." << std::endl;
   }
   
   return tilesDeletedCount;
}

/*
int 
TopologyFile::removeCornerTiles(const int numCornerNodes)
{
   int tilesDeletedCount = 0;
   
   //
   // May need multiple iterations due to corners revealing additional corners
   //
   bool cornersFound = true;
   while (cornersFound) {
      cornersFound = false;
      
      //
      // Topology helper needs to be updated each cycle since tiles get deleted
      //
      const TopologyHelper* th = getTopologyHelper(false, true, false);
   
      //
      // Keep track of tiles that need to be deleted
      //
      std::vector<int> tilesToDelete;
   
      //
      // Check all nodes
      //
      const int numNodes = th->getNumberOfNodes();
      for (int i = 0; i < numNodes; i++) {
         //
         // Get the tiles used by the node
         //
         std::vector<int> tilesUsedByNode;
         th->getNodeTiles(i, tilesUsedByNode);
         
         //
         // If only one tile is used by the node then it must be a corner node
         //
         if (tilesUsedByNode.size() == 1) {
            tilesToDelete.push_back(tilesUsedByNode[0]);
         }
      }
      
      //
      // If corner tiles are found
      //
      if (tilesToDelete.isEmpty() == false) {
         deleteTiles(tilesToDelete);
         cornersFound = true;
         tilesDeletedCount += tilesToDelete.size();
      }
   }
   
   if (DebugControl::getDebugOn()) {
      std::cout << tilesDeletedCount << " corner tiles were deleted." << std::endl;
   }
   
   topologyHelperNeedsRebuild = true;
   setModified();

   return tilesDeletedCount;
}
*/

/*
int 
TopologyFile::disconnectIslands()
{
   //
   // Get this topology file's topology helper (DO NOT DELETE IT)
   //
   const TopologyHelper* topologyHelper = getTopologyHelper(false, true, false);
   
   const int numNodes = topologyHelper->getNumberOfNodes();
   if (numNodes < 2) {
      return 0;
   }
   
   std::vector<int> baseNeighbor(numNodes, -1);
   std::vector<int> numConnectedNeighbors(numNodes, 0);
   std::vector<int> visited(numNodes, 0);
   
   //
   // Mark all nodes without neighbors as visited
   //
   for (int i = 0; i < numNodes; i++) {
      if (topologyHelper->getNodeHasNeighbors(i) == false) {
         visited[i] = 1;
      }
   } 
 
   //
   // Search the surface marking all connected nodes.
   //
   for (int n = 0; n < numNodes; n++) {
      if (visited[n] == 0) {
         
         const int nodeNumberIn = n;
         const int origNeighbor = n;
         
         std::stack<int> st;
         st.push(nodeNumberIn);
         
         while(!st.isEmpty()) {
            const int nodeNumber = st.top();
            st.pop();
            
            if (visited[nodeNumber] == 0) {
               visited[nodeNumber] = 1;
               baseNeighbor[nodeNumber] = origNeighbor;
               numConnectedNeighbors[origNeighbor]++;
               std::vector<int> neighbors;
               topologyHelper->getNodeNeighbors(nodeNumber, neighbors);
               for (int i = 0; i < static_cast<int>(neighbors.size()); i++) {
                  const int node = neighbors[i];
                  if (visited[node] == 0) {
                     st.push(node);
                  }
               }
            }
         }
      }
   }
   
   //
   // find node with most connected neighbors
   //
   int mostNeighborsNode = -1;
   int mostNeighbors = 0;
   for (int j = 0; j < numNodes; j++) {
      if (numConnectedNeighbors[j] > 0) {
         if (DebugControl::getDebugOn()) {
            std::cout << j << " is connected to " << numConnectedNeighbors[j]
                      << " nodes." << std::endl;
         }
      }
      if (numConnectedNeighbors[j] > mostNeighbors) {
         mostNeighborsNode = j;
         mostNeighbors = numConnectedNeighbors[j];
      }
   }
   
   if (DebugControl::getDebugOn()) {
      std::cout << mostNeighborsNode << " has the most neighbors = "
                << mostNeighbors << std::endl;
   }
   
   //
   // Disconnect all nodes that are not 
   // connected to the node with the most connected neighbors
   //
   std::vector<bool> markedNodes(numNodes, false);
   if (mostNeighborsNode >= 0) {
      for (int i = 0; i < numNodes; i++) {
         if (baseNeighbor[i] != mostNeighborsNode) {
            markedNodes[i] = true;
         }
      }
   }
   deleteTilesWithMarkedNodes(markedNodes);   
   topologyHelperNeedsRebuild = true;
   setModified();
}
*/

/**
 * Flip the orientation of all tiles
 */
void
TopologyFile::flipTileOrientation()
{
   const int numTiles = getNumberOfTiles();
   int v1, v2, v3;
   for (int i = 0; i < numTiles; i++) {
      getTile(i, v1, v2, v3);
      setTile(i, v3, v2, v1);
   }
   setModified();
   topologyHelperNeedsRebuild = true;
}

/**
 * Set the number of Topology Nodes (all zeros for data)
 */
/*
void
TopologyFile::setNumberOfTopologyNodes(const int numNodes)
{
   TopologyNode tp(0, TopologyNode::INTERIOR);
   nodes.resize(numNodes, tp);
   setModified();
   topologyHelperNeedsRebuild = true;
}
*/

/**
 * Get the topology information for a node.
 */
/*
const TopologyNode* 
TopologyFile::getTopologyNode(const int nodeNumber) const
{
   return &nodes[nodeNumber];
}
*/

/**
 * get a Tile.
 */
const int* 
TopologyFile::getTile(const int indx) const 
{ 
   const int* tiles = dataArrays[0]->getDataPointerInt();
   return &tiles[indx * 3]; 
}

/**
 * Get a tiles vertices.
 */
void
TopologyFile::getTile(const int tileNumber, int& v1, int& v2, int& v3) const
{
   int32_t* tiles = dataArrays[0]->getDataPointerInt();
   v1 = tiles[tileNumber * 3];
   v2 = tiles[tileNumber * 3 + 1];
   v3 = tiles[tileNumber * 3 + 2];
}

/**
 * Get a tiles vertices.
 */
void
TopologyFile::getTile(const int tileNumber, int vertices[3]) const
{
   int32_t* tiles = dataArrays[0]->getDataPointerInt();
   vertices[0] = tiles[tileNumber * 3];
   vertices[1] = tiles[tileNumber * 3 + 1];
   vertices[2] = tiles[tileNumber * 3 + 2];
}

/**
 * Find the tile (that is not notTileNum) with the two specified vertices.
 */
int 
TopologyFile::getTileWithVertices(const int v1, const int v2, 
                                  const int notTileNum) const
{
   const int num = getNumberOfTiles();
   for (int i = 0; i < num; i++) {
      if (i != notTileNum) {
         int p1, p2, p3;
         getTile(i, p1, p2, p3);

         if (((v1 == p1) && (v2 == p2)) ||
             ((v1 == p2) && (v2 == p1))) {
            return i;
         }
         if (((v1 == p2) && (v2 == p3)) ||
             ((v1 == p3) && (v2 == p2))) {
            return i;
         }
         if (((v1 == p1) && (v2 == p3)) ||
             ((v1 == p3) && (v2 == p1))) {
            return i;
         }
      }
   }

   return -1;
}

/**
 * Get the topology helper.  Users should not store the pointer to the topology 
 * helper since it may be modified at any time.  Call this method to get the 
 * pointer and then use it immediately.
 */
const TopologyHelper*
TopologyFile::getTopologyHelper(const bool needEdgeInfo,
                                const bool needNodeInfo,
                                const bool needNodeInfoSorted) const
{
   QMutexLocker locked(&gettingTopoHelper);//lock BEFORE testing whether rebuild is needed
   if (topologyHelper == NULL) {
      topologyHelperNeedsRebuild = true;
   }

   if (topologyHelperNeedsRebuild == false) {
      if (needEdgeInfo) {
         if (topologyHelper->getEdgeInfoValid() == false) {
            topologyHelperNeedsRebuild = true;
         }
      }
      if (needNodeInfo) {
         if (topologyHelper->getNodeInfoValid() == false) {
            topologyHelperNeedsRebuild = true;
         }
      }
      if (needNodeInfoSorted) {
         if (topologyHelper->getNodeSortedInfoValid() == false) {
            topologyHelperNeedsRebuild = true;
         }
      }
   }
   if (topologyHelperNeedsRebuild) {
      if (topologyHelper != NULL) {
         delete topologyHelper;
      }
      topologyHelper = new TopologyHelper(this, needEdgeInfo, needNodeInfo, 
                                          needNodeInfoSorted);
      topologyHelperNeedsRebuild = false;
   }
   
   return topologyHelper;
}

/**
 * Get the section numbers for the nodes.  Returns true if the sections are valid else false.
 */
bool
TopologyFile::getNodeSections(std::vector<int>& nodeSectionsOut) const
{
   nodeSectionsOut = nodeSections;
   if (nodeSectionsOut.empty()) {
      return false;
   }
   return true;
}
/**
 * set a tiles vertices.
 */
void
TopologyFile::setTile(const int tileNumber, const int v1, const int v2, const int v3)
{
   int32_t* tiles = dataArrays[0]->getDataPointerInt();
   tiles[tileNumber * 3]     = v1;
   tiles[tileNumber * 3 + 1] = v2;
   tiles[tileNumber * 3 + 2] = v3;
   setModified();
   topologyHelperNeedsRebuild = true;
   // add one to node since node number range is [0..N-1]
   numberOfNodes = std::max(v1+1, numberOfNodes);
   numberOfNodes = std::max(v2+1, numberOfNodes);
   numberOfNodes = std::max(v3+1, numberOfNodes);
}

/**
 * set a tiles vertices.
 */
void
TopologyFile::setTile(const int tileNumber, const int v[3])
{
   setTile(tileNumber, v[0], v[1], v[2]);
}

/**
 * Set the number of tiles (all vertices are zero)
 */
void
TopologyFile::setNumberOfTiles(const int numTiles)
{
   std::vector<int> dim;
   dim.push_back(numTiles);
   dim.push_back(3);
   if (dataArrays.empty()) {
      GiftiDataArray* nda = new GiftiDataArray(this,
                                               GiftiCommon::intentTopologyTriangles,
                                               GiftiDataArray::DATA_TYPE_INT32,
                                               dim);
      addDataArray(nda);
   }
   else {
      dataArrays[0]->setDimensions(dim);
   }

   setModified();
   topologyHelperNeedsRebuild = true;
}

/**
 * see if the topology file is equivalent to this one (contains exact same tiles).
 */
bool 
TopologyFile::equivalent(const TopologyFile& tf) const
{
   //
   // Compare type of topology
   //
   if (getTopologyType() != tf.getTopologyType()) {
      return false;
   }
   
   //
   // Check number of tiles
   //
   const int numTiles = getNumberOfTiles();
   if (numTiles != tf.getNumberOfTiles()) {
      return false;
   }
   
   //
   // Get pointers to the tiles
   //
   const int32_t* myTiles = dataArrays[0]->getDataPointerInt();
   const int32_t* otherTiles = tf.dataArrays[0]->getDataPointerInt();

   //
   // Compare all tile vertices
   //
   const int numVertices = numTiles * 3;
   for (int i = 0; i < numVertices; i++) {
      if (myTiles[i] != otherTiles[i]) {
         return false;
      }
   }
   
   //
   // Files must be the same
   //
   return true;
}

/**
 * get the topology from a vtk poly data file
 */
void
TopologyFile::importFromVtkFile(vtkPolyData* polyDataIn)
{
   clear();
   
   vtkPolyData* polyData = polyDataIn;
   if (DebugControl::getDebugOn()) {
      std::cout << "Before Triangle Filter:\n"
                << "   Topology Import Polydata Strips: " << polyData->GetNumberOfStrips()
                << "\n"
                << "   Topology Import Polydata Polygons: " << polyData->GetNumberOfPolys()
                << "\n";
   }
   
   //
   // Convert triangle strips to triangles (if necessary)
   //
   vtkTriangleFilter* triangleFilter = NULL;
   if (polyData->GetNumberOfStrips() > 0) {
      triangleFilter = vtkTriangleFilter::New();
#ifdef HAVE_VTK6
      triangleFilter->SetInputData(polyDataIn);
#else
      triangleFilter->SetInput(polyDataIn);
#endif
      triangleFilter->Update();
      polyData = triangleFilter->GetOutput();
   }
   if (DebugControl::getDebugOn()) {
      std::cout << "After Triangle Filter:\n"
                << "   Topology Import Polydata Strips: " << polyData->GetNumberOfStrips()
                << "\n"
                << "   Topology Import Polydata Polygons: " << polyData->GetNumberOfPolys()
                << "\n";
   }
   
   std::vector<int> triangles;
   
   vtkCellArray* polys = polyData->GetPolys();
   vtkIdType npts;
   vtkIdType* pts;
   for (polys->InitTraversal(); polys->GetNextCell(npts,pts); ) {
      if (npts == 3) {
         triangles.push_back(static_cast<int>(pts[0]));
         triangles.push_back(static_cast<int>(pts[1]));
         triangles.push_back(static_cast<int>(pts[2]));
      }
      else {
         std::cout << "ERROR: VTK surface contains polygon with " << npts 
                   << " vertices." << std::endl;
      }
   }
   
   const int numTrianglesRead = static_cast<int>(triangles.size()) / 3;
   setNumberOfTiles(numTrianglesRead);
   for (int i = 0; i < numTrianglesRead; i++) {
      setTile(i, &triangles[i*3]);
   }
   if (DebugControl::getDebugOn()) {
      std::cout << "Topology contains " << numTrianglesRead << " triangles." << std::endl;
   }
   
   setModified();
   topologyHelperNeedsRebuild = true;
   
   if (triangleFilter != NULL) {
      triangleFilter->Delete();
   }
}

/**
 * get the topology from a MNI OBJ surface file.
 */
void 
TopologyFile::importFromMniObjSurfaceFile(const MniObjSurfaceFile& mni) throw (FileException)
{
   clear();
   
   const int numberOfTriangles = mni.getNumberOfTriangles();
   if (numberOfTriangles > 0) {
      setNumberOfTiles(numberOfTriangles);
      for (int i = 0; i < numberOfTriangles; i++) {
         const int* triangle = mni.getTriangle(i);
         setTile(i, triangle[0], triangle[1], triangle[2]);
      }
   }
   appendToFileComment(" Imported from ");
   appendToFileComment(FileUtilities::basename(mni.getFileName()));
   setModified();
   topologyHelperNeedsRebuild = true;
}
      
/**
 * get the topology from a brain voyager file
 */
void 
TopologyFile::importFromBrainVoyagerFile(const BrainVoyagerFile& bvf)
{
   clear();
   
   const int numberOfTriangles = bvf.getNumberOfTriangles();
   if (numberOfTriangles > 0) {
      setNumberOfTiles(numberOfTriangles);
      for (int i = 0; i < numberOfTriangles; i++) {
         int triangle[3];
         bvf.getTriangle(i, triangle);
         setTile(i, triangle[0], triangle[1], triangle[2]);
      }
   }
   appendToFileComment(" Imported from ");
   appendToFileComment(FileUtilities::basename(bvf.getFileName()));
   setModified();
   topologyHelperNeedsRebuild = true;
}

/**
 * get the topology from a free surfer surface file
 */
void 
TopologyFile::importFromFreeSurferSurfaceFile(const FreeSurferSurfaceFile& fssf,
                                              const TopologyFile* closedTopologyFile)
{
   clear();
   
   const int numberOfTriangles = fssf.getNumberOfTriangles();
   //
   // Binary patch surfaces have no triangles so obtain topology from the closed
   // topology file which should have been read from the "orig" free surfer surface.
   //
   if (numberOfTriangles <= 0) {
      if (closedTopologyFile == NULL) {
         throw FileException(filename, "This surface has no topology (triangles) and there \n"
                                       "is no closed topology previously loaded.  Try loading\n"
                                       "the \"orig\" surface prior to loading this surface.");
      }
      else if (closedTopologyFile->getNumberOfTiles() <= 0) {
         throw FileException(filename, "This surface has no topology (triangles) and there \n"
                                       "is no closed topology previously loaded.  Try loading\n"
                                       "the \"orig\" surface prior to loading this surface.");
      }
      else {
         //
         // Copy from closed topology any tiles that have its three nodes
         // in the patch
         //
         const int maxNodes = std::max(closedTopologyFile->getNumberOfNodes(),
                                       fssf.getNumberOfVertices());
         std::vector<bool> nodeUsed(maxNodes, false);
         for (int i = 0; i < fssf.getNumberOfVertices(); i++) {
            nodeUsed[fssf.getVertexNumber(i)] = true;
         }
         for (int i = 0; i < closedTopologyFile->getNumberOfTiles(); i++) {
            int v1, v2, v3;
            closedTopologyFile->getTile(i, v1, v2, v3);
            if (nodeUsed[v1] && nodeUsed[v2] && nodeUsed[v3]) {
               addTile(v1, v2, v3);
            }
         }
      }
   }
   else {
      setNumberOfTiles(numberOfTriangles);
      for (int i = 0; i < numberOfTriangles; i++) {
         int triangle[3];
         fssf.getTriangle(i, triangle);
         setTile(i, triangle[0], triangle[1], triangle[2]);
      }
   }
   appendToFileComment(" Imported from ");
   appendToFileComment(FileUtilities::basename(fssf.getFileName()));
   setModified();
   topologyHelperNeedsRebuild = true;
}

/**
 * export topology to a free surfer surface file
 */
void 
TopologyFile::exportToFreeSurferSurfaceFile(FreeSurferSurfaceFile& fssf)
{
   const int numberOfTriangles = getNumberOfTiles();
   for (int i = 0; i < numberOfTriangles; i++) {
      int v[3];
      getTile(i, v);
      fssf.setTriangle(i, v);
   }
}

/**
 * Read topology file version 0.
 */
void
TopologyFile::readFileDataVersion0(QTextStream& stream, 
                                   const QString& firstLineRead) throw (FileException)
{
   QString line = firstLineRead;
   
   //readLine(stream, line);
   numberOfNodes = line.toInt();
   //setNumberOfTopologyNodes(numberOfNodes);

   nodeSections.resize(numberOfNodes, 0);
   
   for (int i = 0; i < numberOfNodes; i++) {
      readLine(stream, line);
      
      int nodeNumber;
      int numNeighbors;
      int sectionNumber;
      int contourNumber;
      int pointNumber;
      int categoryInt;
     
      if (sscanf(line.toAscii().constData(), "%d %d %d %d %d %d", 
             &nodeNumber, &numNeighbors, &sectionNumber,
             &contourNumber, &pointNumber, &categoryInt) != 6)
      {
         throw FileException(filename, "Truncated topology file detected");
      }
      nodeSections[i] = sectionNumber;
/* using sscanf is faster
      QTextIStream(&line) >> nodeNumber
                          >> numNeighbors
                          >> sectionNumber
                          >> contourNumber
                          >> pointNumber
                          >> categoryInt;
*/
    //  TopologyNode::CATEGORIES category = (TopologyNode::CATEGORIES)categoryInt;
      
    //  nodes[i].setData(sectionNumber, category);
      
      //
      // Neighbor data no longer used so read and ignore it
      //
      for (int j = 0; j < numNeighbors; j++) {
         readLine(stream, line);
      }
   }
   
   readTilesAscii(stream, true);
}

/**
 * Read the tiles
 */
void
TopologyFile::readTilesAscii(QTextStream& stream, 
                             const bool clockwiseOrientation) throw (FileException)
{
   QString line;
   readLine(stream, line);
   const int numTiles = line.toInt();
   if (numTiles < 0) {
      throw FileException(filename, "Number of tiles is less than zero");
   }
   setNumberOfTiles(numTiles);
   
   if (numTiles > 0) {
      int* tilePtr = dataArrays[0]->getDataPointerInt();
      
      //
      // Read tiles (Note that version 0 stores tiles with Clockwise orientation so switch tile order)
      //
      int t1, t2, t3;
      for (int j = 0; j < numTiles; j++) {
         // readLine & sscanf is faster than    "stream >> t1 >> t2 >> t3;"
         readLine(stream, line);
         if (sscanf(line.toAscii().constData(), "%d %d %d", &t1, &t2, &t3) != 3)
         {
            throw FileException(filename, "Truncated topology file detected");
         }
         
         const int j3 = j * 3;
         if (clockwiseOrientation) {
            tilePtr[j3]     = t3;
            tilePtr[j3 + 1] = t2;
            tilePtr[j3 + 2] = t1;
         }
         else {
            tilePtr[j3]     = t1;
            tilePtr[j3 + 1] = t2;
            tilePtr[j3 + 2] = t3;
         }
         
         // add one to node since node number range is [0..N-1]
         numberOfNodes = std::max(numberOfNodes, t1+1);
         numberOfNodes = std::max(numberOfNodes, t2+1);
         numberOfNodes = std::max(numberOfNodes, t3+1);
      }
   }
   
   setModified();
   topologyHelperNeedsRebuild = true;
}

/**
 * Read the tiles
 */
void
TopologyFile::readTilesBinary(QDataStream& binStream) throw (FileException)
{
   int numTiles = 0;
   binStream >> numTiles;
   
   if (numTiles < 0) {
      throw FileException(filename, "Number of tiles is less than zero");
   }
   setNumberOfTiles(numTiles);
   
   if (numTiles > 0) {
      int* tilePtr = dataArrays[0]->getDataPointerInt();
      
      //
      // Read tiles 
      //
      int t1, t2, t3;
      for (int j = 0; j < numTiles; j++) {
         binStream >> t1 >> t2 >> t3;
         
         const int j3 = j * 3;
         tilePtr[j3]     = t1;
         tilePtr[j3 + 1] = t2;
         tilePtr[j3 + 2] = t3;
         
         // add one to node since node number range is [0..N-1]
         numberOfNodes = std::max(numberOfNodes, t1+1);
         numberOfNodes = std::max(numberOfNodes, t2+1);
         numberOfNodes = std::max(numberOfNodes, t3+1);
      }   
   }
   
   setModified();
   topologyHelperNeedsRebuild = true;
}

/**
 * Read topology file version 1.
 */
void
TopologyFile::readFileDataVersion1(QFile& file,
                                   QTextStream& stream,
                                   QDataStream& binStream) throw (FileException)
{
   switch (getFileReadType()) {
      case FILE_FORMAT_ASCII:
         readTilesAscii(stream, false);
         break;
      case FILE_FORMAT_BINARY:
#ifdef QT4_FILE_POS_BUG
         {
            qint64 offset = findBinaryDataOffsetQT4Bug(file, "tag-version 1");
            if (offset > 0) {
               offset++;  
               file.seek(offset);
            }            
         }
#endif
         readTilesBinary(binStream);
         break;
      case FILE_FORMAT_XML:
         throw FileException(filename, "Reading in XML format not supported.");
         break;
      case FILE_FORMAT_XML_BASE64:
         throw FileException(filename, "XML Base64 not supported.");
         break;
      case FILE_FORMAT_XML_GZIP_BASE64:
         throw FileException(filename, "XML GZip Base64 not supported.");
         break;
      case FILE_FORMAT_XML_EXTERNAL_BINARY:
         throw FileException(filename, "XML External Binary not supported.");
         break;      
      case FILE_FORMAT_OTHER:
         throw FileException(filename, "Reading in Other format not supported.");
         break;
      case FILE_FORMAT_COMMA_SEPARATED_VALUE_FILE:
         throw FileException(filename, "Comma Separated Value File Format not supported.");
         break;
   }
}

/**
 * Read Topology File's data.  May throw FileException.
 */
void
TopologyFile::readLegacyFileData(QFile& file, QTextStream& stream,
                                 QDataStream& binStream) throw (FileException)
{
   //
   // Should reading data be skipped ?
   //
   if (getReadMetaDataOnlyFlag()) {
      return;
   }

   // get starting position of data
   //const QIODevice::Offset startOfData = file.at();
   
   int version = 0;
   
   // read first line to see if it is version 1 or later
   QString line, versionStr, versionNumberStr;
   readTagLine(stream, line, versionStr, versionNumberStr);
   if (versionStr == tagFileVersion) {
      switch(versionNumberStr.toInt()) {
         case 1:
            version = 1;
            break;
         default:
            throw FileException(filename, "Unknown version of topology file");
      }
   }

   switch(version) {
      case 1:
         {
            //file.seek(stream.pos());
            const qint64 pos = this->getQTextStreamPosition(stream);
            file.seek(pos);
            readFileDataVersion1(file, stream, binStream);
         }
         break;
      default:
/*
         throw FileException(getFileName(), 
                   "Due to a bug in QT4, Caret is unable to read version 0 topology.\n"
                   "files at this time.  To get around this problem use caret_file_convert."
                   "  caret_file_convert -binary file.topo\n"
                   "  caret_file_convert -text file.topo");
*/
         //file.at(startOfData);
         readFileDataVersion0(stream, line);
         break;
   }
   topologyHelperNeedsRebuild = true;
}

void
TopologyFile::writeLegacyFileData(QTextStream& stream, QDataStream& binStream)
                                                             throw (FileException)
{
   stream << tagFileVersion << " 1" << "\n";

   const int numTiles = getNumberOfTiles();

   switch (getFileWriteType()) {
      case FILE_FORMAT_ASCII:
         {
            stream << numTiles << "\n";
            for (int j = 0; j < numTiles; j++) {
               int t1, t2, t3;
               getTile(j, t1, t2, t3);
               stream << t1 << " " << t2 << " " << t3 << "\n";
            }
         }
         break;
      case FILE_FORMAT_BINARY:
#ifdef QT4_FILE_POS_BUG
         setBinaryFilePosQT4Bug();
#else  // QT4_FILE_POS_BUG
         //
         // still a bug in QT 4.2.2
         //
         setBinaryFilePosQT4Bug();
#endif // QT4_FILE_POS_BUG
         {
            binStream << numTiles;
            for (int j = 0; j < numTiles; j++) {
               int t1, t2, t3;
               getTile(j, t1, t2, t3);
               binStream << t1 << t2 << t3;
            }
         }
         break;
      case FILE_FORMAT_XML:
         throw FileException(filename, "Writing in XML format not supported.");
         break;
      case FILE_FORMAT_XML_BASE64:
         throw FileException(filename, "XML Base64 not supported.");
         break;
      case FILE_FORMAT_XML_GZIP_BASE64:
         throw FileException(filename, "XML GZip Base64 not supported.");
         break;
      case FILE_FORMAT_XML_EXTERNAL_BINARY:
         throw FileException(filename, "XML External Binary not supported.");
         break;      
      case FILE_FORMAT_OTHER:
         throw FileException(filename, "Writing in Other format not supported.");
         break;
      case FILE_FORMAT_COMMA_SEPARATED_VALUE_FILE:
         throw FileException(filename, "Comma Separated Value File Format not supported.");
         break;
   }
   
}

/**
 * Update the file's metadata for Caret6.
 */
void
TopologyFile::updateMetaDataForCaret6()
{
   AbstractFile::updateMetaDataForCaret6();

   switch (this->getTopologyType()) {
      case TOPOLOGY_TYPE_CLOSED:
         this->setHeaderTag(GiftiCommon::metaDataNameTopologicalType,
                            "Closed");
         break;
      case TOPOLOGY_TYPE_OPEN:
         this->setHeaderTag(GiftiCommon::metaDataNameTopologicalType,
                            "Open");
         break;
      case TOPOLOGY_TYPE_CUT:
         this->setHeaderTag(GiftiCommon::metaDataNameTopologicalType,
                            "Cut");
         break;
      case TOPOLOGY_TYPE_LOBAR_CUT:
         this->setHeaderTag(GiftiCommon::metaDataNameTopologicalType,
                            "Cut");
         break;
      case TOPOLOGY_TYPE_UNKNOWN:
      case TOPOLOGY_TYPE_UNSPECIFIED:
         this->setHeaderTag(GiftiCommon::metaDataNameTopologicalType,
                            "Closed");
         break;
   }

   this->removeHeaderTag("perimeter_id");
}

/**
 * Write the file's memory in caret6 format to the specified name.
 */
QString
TopologyFile::writeFileInCaret6Format(const QString& filenameIn, Structure structure,const ColorFile* colorFileIn, const bool useCaret6ExtensionFlag) throw (FileException)
{
   QString name = filenameIn;
   if (useCaret6ExtensionFlag) {
      name = FileUtilities::replaceExtension(filenameIn, ".topo",
                                     SpecFile::getGiftiTopologyFileExtension());
   }
   this->setFileWriteType(AbstractFile::FILE_FORMAT_XML_GZIP_BASE64);
   this->writeFile(name);

   return name;
}

