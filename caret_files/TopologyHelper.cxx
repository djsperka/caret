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

#include "TopologyFile.h"
#include "TopologyHelper.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkTriangleFilter.h"

/**
 * Comparison operator for EdgeInfo
 */
bool 
operator<(const TopologyEdgeInfo& e1, const TopologyEdgeInfo& e2)
{
   if (e1.node1 == e2.node1) {
      return (e1.node2 < e2.node2);
   }
   return (e1.node1 < e2.node1);
}

/**
 * Constructor.  This will contruct the topological information for the TopologyFile.
 *   "buildEdgeInfo" will allow queries to be made about which tiles are used by an edge.
 *   "buildNodeInfo" will allow queries to be made about the tiles and node neighbors of
 *   nodes.  If "sortNodeInfo" is also set the tiles and node neighbors will be sorted
 *   around each node.
 */
TopologyHelper::TopologyHelper(const TopologyFile* tf,
                               const bool buildEdgeInfo,
                               const bool buildNodeInfo,
                               const bool sortNodeInfo)
{
   nodeSortedInfoBuilt = false;
   nodeInfoBuilt       = false;
   edgeInfoBuilt = false;
   
   const int numTiles = tf->getNumberOfTiles();
   if (numTiles <= 0) {
      return;
   }
   
   if (buildNodeInfo) {
      int maxNodeNum = -1;
   
         //
      // Get the number of nodes
      //
      for (int j = 0; j < numTiles; j++) {
         int n1, n2, n3;
         tf->getTile(j, n1, n2, n3);
         if (n1 > maxNodeNum) maxNodeNum = n1;
         if (n2 > maxNodeNum) maxNodeNum = n2;
         if (n3 > maxNodeNum) maxNodeNum = n3;
      }
      
      //
      // Node indices start at zero so need to increment
      //
      maxNodeNum++; 
      
      //
      // There may be nodes that have been disconnected and not associated with any tiles
      //
      maxNodeNum = std::max(maxNodeNum, tf->getNumberOfNodes());
      
      //
      // Initialize the node structures
      //
      nodes.reserve(maxNodeNum);
      for (int i = 0; i < maxNodeNum; i++) {
         nodes.push_back(NodeInfo(i));
      }
   }
   
   //
   // Keep track of the tiles
   //
   for (int j = 0; j < numTiles; j++) {
      int n1, n2, n3;
      tf->getTile(j, n1, n2, n3);
      if (buildNodeInfo) {
         if (sortNodeInfo) {
            nodes[n1].addNeighbors(j, n2, n3);
            nodes[n2].addNeighbors(j, n3, n1);
            nodes[n3].addNeighbors(j, n1, n2);
         }
         else {
            nodes[n1].addNeighbor(n2);
            nodes[n1].addNeighbor(n3);
            nodes[n2].addNeighbor(n1);
            nodes[n2].addNeighbor(n3);
            nodes[n3].addNeighbor(n1);
            nodes[n3].addNeighbor(n2);
            nodes[n1].addTile(j);
            nodes[n2].addTile(j);
            nodes[n3].addTile(j);
         }
      }
      
      if (buildEdgeInfo) {
         addEdgeInfo(j, n1, n2);
         addEdgeInfo(j, n2, n3);
         addEdgeInfo(j, n3, n1);
      }
   }

   if (buildEdgeInfo) {
      edgeInfoBuilt = true;
   }
   
   if (buildNodeInfo) {
      nodeInfoBuilt = true;
      if (sortNodeInfo) {
         nodeSortedInfoBuilt = true;
         for (unsigned int k = 0; k < nodes.size(); k++) {
            nodes[k].sortNeighbors();
         }
      }
   }
}

/**
 * Constructor.  This will contruct the topological information for the TopologyFile.
 *   "buildEdgeInfo" will allow queries to be made about which tiles are used by an edge.
 *   "buildNodeInfo" will allow queries to be made about the tiles and node neighbors of
 *   nodes.  If "sortNodeInfo" is also set the tiles and node neighbors will be sorted
 *   around each node.
 */
TopologyHelper::TopologyHelper(vtkPolyData* vtkIn,
                               const bool buildEdgeInfo,
                               const bool buildNodeInfo,
                               const bool sortNodeInfo)
{
   vtkPolyData* vtk = vtkIn;
   
   //
   // convert triangle strips to triangles if necessary       
   //
   vtkTriangleFilter *triangleFilter = NULL;
   if (vtk->GetNumberOfStrips() > 0) {
      triangleFilter = vtkTriangleFilter::New();
      triangleFilter->SetInput(vtk);
      triangleFilter->Update();
      vtk = triangleFilter->GetOutput();
   }

   nodeSortedInfoBuilt = false;
   nodeInfoBuilt       = false;
   edgeInfoBuilt = false;
   
   //const int numTiles = tf->getNumberOfTiles();
   //if (numTiles <= 0) {
   //   return;
  // }
   
   if (buildNodeInfo) {
      vtkCellArray* polys = vtk->GetPolys();
      int npts;
      int* pts;
      const int maxNodeNum = vtk->GetNumberOfPoints();  //-1;
      for (polys->InitTraversal(); polys->GetNextCell(npts,pts); ) {
         if (npts != 3) {
            std::cerr << " Polygon is not a triangle in TopologyHelper, ignored"      
                      << std::endl;
         }
         //maxNodeNum = std::max(pts[0], maxNodeNum);
         //maxNodeNum = std::max(pts[1], maxNodeNum);
         //maxNodeNum = std::max(pts[2], maxNodeNum);
      }
      
      //
      // Node indices start at zero so need to increment
      //
      //maxNodeNum++; 
      
      //
      // Initialize the node structures
      //
      for (int i = 0; i < maxNodeNum; i++) {
         nodes.push_back(NodeInfo(i));
      }
   }
   
   //
   // Keep track of the tiles
   //
   vtkCellArray* polys = vtk->GetPolys();
   int cellId = 0;
   int npts;
   int* pts;
   for (polys->InitTraversal(); polys->GetNextCell(npts,pts); cellId++) {
      if (npts != 3) {
         std::cerr << " Polygon is not a triangle in TopologyHelper"      
                     << std::endl;
         return;
      }
      int n1 = pts[0];
      int n2 = pts[1];
      int n3 = pts[2];
      if (buildNodeInfo) {
         if (sortNodeInfo) {
            nodes[n1].addNeighbors(cellId, n2, n3);
            nodes[n2].addNeighbors(cellId, n3, n1);
            nodes[n3].addNeighbors(cellId, n1, n2);
         }
         else {
            nodes[n1].addNeighbor(n2);
            nodes[n1].addNeighbor(n3);
            nodes[n2].addNeighbor(n1);
            nodes[n2].addNeighbor(n3);
            nodes[n3].addNeighbor(n1);
            nodes[n3].addNeighbor(n2);
            nodes[n1].addTile(cellId);
            nodes[n2].addTile(cellId);
            nodes[n3].addTile(cellId);
         }
      }
      
      if (buildEdgeInfo) {
         addEdgeInfo(cellId, n1, n2);
         addEdgeInfo(cellId, n2, n3);
         addEdgeInfo(cellId, n3, n1);
      }
   }

   if (buildEdgeInfo) {
      edgeInfoBuilt = true;
   }
   
   if (buildNodeInfo) {
      nodeInfoBuilt = true;
      if (sortNodeInfo) {
         nodeSortedInfoBuilt = true;
         for (unsigned int k = 0; k < nodes.size(); k++) {
            nodes[k].sortNeighbors();
         }
      }
   }
}

/**
 * Destructor.
 */
TopologyHelper::~TopologyHelper()
{
   nodes.clear();
   topologyEdges.clear();
}

/**
 * Keep track of edges
 */
void
TopologyHelper::addEdgeInfo(const int tileNum, const int node1, const int node2)
{
   TopologyEdgeInfo edge(tileNum, node1, node2);
   
   std::set<TopologyEdgeInfo>::iterator iter = topologyEdges.find(edge);
   if (iter == topologyEdges.end()) {
      topologyEdges.insert(edge);
   }
   else {
      TopologyEdgeInfo& e = (TopologyEdgeInfo&)(*iter);
      e.addTile(tileNum);
      //iter->addTile(tileNum);
   }
}

/**
 * Sort a nodes neighbors
 */
void 
TopologyHelper::NodeInfo::sortNeighbors() 
{
   if ((edges.size() > 0) && sortMe) {
      //
      // Nodes that are on the edge (boundary) of a cut surface must be treated specially when sorting.
      // In this case the sorting needs to start with the link whose first node is not used
      // in any other link.  This is true if all tiles are consistently oriented (and they
      // must be).
      //
      int startEdge = -1;
      for (unsigned int k = 0; k < edges.size(); k++) {
         const int nodeNum = edges[k].node1;
         bool foundNode = false;
         
         //
         // See if nodeNum is used in any other edges
         //
         for (unsigned int m = 0; m < edges.size(); m++) {
            if (m != k) {
               if (edges[m].containsNode(nodeNum) == true) {
                  foundNode = true;
                  break;
               }
            }
         }
         
         if (foundNode == false) {
            if (startEdge >= 0) {
               //std::cerr << "INFO: multiple starting edges for node " << nodeNumber << std::endl;
            }
            else {
               startEdge = k;
            }
         }
      }
      
      bool flag = false;
      if ((nodeNumber >= 44508) && (nodeNumber <= 44507)) { // modify for debugging
         std::cerr << "Start Edge for node " << nodeNumber << " is " << startEdge << std::endl;
         for (unsigned int k = 0; k < edges.size(); k++) {
            std::cerr << "   " << edges[k].node1 << " " << edges[k].node2 << std::endl;
         }
         flag = true;
      }

      // startEdge is less than zero if the node's links are all interior links 
      if (startEdge < 0) {
         startEdge = 0;
      }
      
      int currentNode  = edges[startEdge].node1;
      int nextNeighbor = edges[startEdge].node2;
      neighbors.push_back(currentNode);
      tiles.push_back(edges[startEdge].tileNumber);
      const int firstNode = currentNode;
      
      for (unsigned int i = 1; i < edges.size(); i++) {
         neighbors.push_back(nextNeighbor);
         // find edge with node "nextNeighbor" but without node "currentNode"
         const NodeEdgeInfo* e = findEdgeWithPoint(nextNeighbor, currentNode);
         if (e != NULL) {
            tiles.push_back(e->tileNumber);
            currentNode = nextNeighbor;
            nextNeighbor = e->getOtherNode(nextNeighbor);
            if (nextNeighbor < 0) {
               //std::cerr << "INFO: Unable to find neighbor of"
               //            <<  nextNeighbor << std::endl;
            }
         }
         else {
            nextNeighbor = -1;
            //std::cerr << "INFO: Unable to find edge for for node " 
            //            << nodeNumber << std::endl;
            break;
         }
      }
      
      if ((nextNeighbor != firstNode) && (nextNeighbor >= 0)) {
         neighbors.push_back(nextNeighbor);
      }
      
      if (flag) {
         std::cerr << "Node " << nodeNumber << " neighbors: ";
         for (unsigned int n = 0; n < neighbors.size(); n++) {
            std::cerr << "  " << neighbors[n];
         }
         std::cerr << std::endl;
         std::cerr << "Node " << nodeNumber << " tiles: ";
         for (unsigned int n = 0; n < tiles.size(); n++) {
            std::cerr << "  " << tiles[n];
         }
         std::cerr << std::endl;
      }
   }
   edges.clear();
}

/**
 * See if a node has any neighbors
 */
bool
TopologyHelper::getNodeHasNeighbors(const int nodeNum) const
{
   if ((nodeNum >= 0) && (nodeNum < static_cast<int>(nodes.size()))) {
      return (nodes[nodeNum].neighbors.empty() == false);
   }
   return false;
}

/**
 * Get the number of neighbors for a node.
 */
int 
TopologyHelper::getNodeNumberOfNeighbors(const int nodeNum) const
{
   if ((nodeNum >= 0) && (nodeNum < static_cast<int>(nodes.size()))) {
      return nodes[nodeNum].neighbors.size();
   }
   return 0;
}

/**
 * Get the number of boundary edges used by node.
 */
void 
TopologyHelper::getNumberOfBoundaryEdgesForAllNodes(std::vector<int>& numBoundaryEdgesPerNode) const
{
   //
   // should already be allocated
   //
   const int numNodes = getNumberOfNodes();
   if (static_cast<int>(numBoundaryEdgesPerNode.size()) < numNodes) {
      numBoundaryEdgesPerNode.resize(numNodes);
   }
   std::fill(numBoundaryEdgesPerNode.begin(), numBoundaryEdgesPerNode.end(), 0);
   
   //
   // Loop through all edges
   //
   for (std::set<TopologyEdgeInfo>::const_iterator iter = topologyEdges.begin(); 
        iter != topologyEdges.end(); iter++) {
      //
      // Get the tiles
      //
      int tile1, tile2;
      iter->getTiles(tile1, tile2);
      
      //
      // If edges has only one tile
      //
      if ((tile1 >= 0) && (tile2 < 0)) {
         //
         // Increment the count for the node
         //
         int node1, node2;
         iter->getNodes(node1, node2);
         numBoundaryEdgesPerNode[node1]++;
         numBoundaryEdgesPerNode[node2]++;
      }
   }
}
      
/**
 * Get the maximum number of neighbors of all nodes.
 */
int
TopologyHelper::getMaximumNumberOfNeighbors() const
{
   unsigned int maxNeighbors = 0;
   int numNodes = getNumberOfNodes();
   for (int i = 0; i < numNodes; i++) {
      const unsigned int num = nodes[i].neighbors.size();
      if (num > maxNeighbors) {
         maxNeighbors = num;
      }
   }
   return maxNeighbors;
}

/**
 * Get the neighboring nodes for a node
 */
void
TopologyHelper::getNodeNeighbors(const int nodeNum,
                                 std::vector<int>& neighborsOut) const
{
   if ((nodeNum >= 0) && (nodeNum < static_cast<int>(nodes.size()))) {
      neighborsOut = nodes[nodeNum].neighbors;
   }
   else {
      neighborsOut.clear();
   }
}

/**
 * Get the neighbors of a node to a specified depth.
 */
void 
TopologyHelper::getNodeNeighborsToDepth(const int rootNode, 
                                        const int depth,
                                        std::vector<int>& neighborsOut) const
{
   neighborsOut.clear();
   
   const int numNodes = getNumberOfNodes();
   std::vector<int> nodeVisited(numNodes, 0);
   
   std::set<int> nodesMarked;
   nodesMarked.insert(rootNode);
   
   for (int dp = 0; dp < depth; dp++) {
      std::set<int> newNodes;
      
      for (std::set<int>::iterator it = nodesMarked.begin();
           it != nodesMarked.end(); it++) {
         const int node = *it; 
         
         if (nodeVisited[node] == 0) {
            nodeVisited[node] = 1;
            //
            // Get all of the nodes neighbors
            //
            const std::vector<int>& neighbors = nodes[node].neighbors;
            const unsigned int numNeighs = neighbors.size();
            
            for (unsigned int j = 0; j < numNeighs; j++) {
               const int n = neighbors[j];
               if (nodeVisited[n] == 0) {
                  newNodes.insert(n);
               }
            }
         }
      }
      
      nodesMarked.insert(newNodes.begin(), newNodes.end());
   }
   
   for (std::set<int>::iterator it = nodesMarked.begin();
        it != nodesMarked.end(); it++) {
      const int node = *it; 
      if (node != rootNode) {
         neighborsOut.push_back(node);
      }
   }
   
/*   
   const int numNodes = getNumberOfNodes();

   //
   // Neighbors found to depth
   //
   std::vector<int> neighborsFound(numNodes, 0);
   
   //
   // Mark root node as found
   //
   neighborsFound[rootNode] = 1;
   
   //
   // Find nodes to the specified depth
   //
   for (int dp = 0; dp <= depth; dp++) {
      for (int i = 0; i < numNodes; i++) {
         //
         // If this node is not marked yet
         //
         if (neighborsFound[i] == 0) {
            //
            // Get all of the nodes neighbors
            //
            int numNeighs = 0;
            const int* nodeNeighbors = getNodeNeighbors(i, numNeighs);
            
            //
            // Add nodes neighbors for next iteration
            //
            for (int k = 0; k < numNeighs; k++) {
               //
               // If a neighbor is marked
               //
               const int n = nodeNeighbors[k];
               if (neighborsFound[n]) {
                  //
                  // Mark me
                  //
                  neighborsFound[i] = 1;
                  break;
               }
            }
         }
      }
   }
   
   for (int i = 0; i < numNodes; i++) {
      if (i != rootNode) {
         if (neighborsFound[i]) {
            neighborsOut.push_back(i);
         }
      }
   }   
*/

/*   
   //
   // Neighbors found to depth
   //
   std::set<int> neighborsFound;
   
   //
   // Keep track of nodes visited so far
   //
   std::vector<int> visited(numNodes, false);
   
   //
   // List of nodes to search at each depth iteration
   //
   std::vector<int> nodesToSearch;
   nodesToSearch.push_back(rootNode);
   
   //
   // Find nodes to the specified depth
   //
   for (int dp = 0; dp <= depth; dp++) {
      //
      // Nodes for next depth iteration (use set so list is unique)
      //
      std::set<int> nodesForNextIteration;
      
      //
      // For nodes that are to be searched
      //
      const int numToSearch = static_cast<int>(nodesToSearch.size());
      for (int i = 0; i < numToSearch; i++) {
         //
         // Node that is to be searched
         //
         const int node = nodesToSearch[i];
         
         //
         // If this node has not already been visited
         //
         if (visited[node] == false) {
            //
            // Mark this node visited
            //
            visited[node] = true;
            
            //
            // Add it to the list of neighbors
            //
            neighborsFound.insert(node);
            
            //
            // Get all of the nodes neighbors
            //
            int numNeighs = 0;
            const int* nodeNeighbors = getNodeNeighbors(node, numNeighs);
            
            //
            // Add nodes neighbors for next iteration
            //
            for (int k = 0; k < numNeighs; k++) {
               const int n = nodeNeighbors[k];
               if (visited[n] == false) {
                  nodesForNextIteration.insert(n);
               }
            }
         }
      }
      
      //
      // Update list of neighbors to search for next iteration
      //
      nodesToSearch.clear();
      nodesToSearch.insert(nodesToSearch.end(),
                           nodesForNextIteration.begin(), nodesForNextIteration.end());
   }

   //
   // Set the output neighbors exluding the input node
   //
   for (std::set<int>::iterator iter = neighborsFound.begin();
        iter != neighborsFound.end(); iter++) {
      const int n = *iter;
      if (n != rootNode) {
         neighborsOut.push_back(n);
      }
   }
*/
}
      
/**
 * Get the neighboring nodes for a node.  Returns a pointer to an array
 * containing the neighbors.
 */
const int*
TopologyHelper::getNodeNeighbors(const int nodeNum, int& numNeighborsOut) const
{
   if ((nodeNum >= 0) && (nodeNum < static_cast<int>(nodes.size()))) {
      numNeighborsOut = static_cast<int>(nodes[nodeNum].neighbors.size());
      if (numNeighborsOut <= 0) {
         return NULL;
      }
      return &nodes[nodeNum].neighbors[0];
   }
   numNeighborsOut = 0;
   return NULL;
}

/**
 * Get the tiles that use this node.
 */
void
TopologyHelper::getNodeTiles(const int nodeNum, std::vector<int>& tilesOut) const
{
   if ((nodeNum >= 0) && (nodeNum < static_cast<int>(nodes.size()))) {
      tilesOut = nodes[nodeNum].tiles;
   }
   else {
      tilesOut.clear();
   }
}

