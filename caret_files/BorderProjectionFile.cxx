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

#include <algorithm>

#include "XmlGenericWriter.h"
#include <cmath>
#include <limits>

#include "BorderFile.h"
#define __BORDER_PROJECTION_MAIN__
#include "BorderProjectionFile.h"
#undef __BORDER_PROJECTION_MAIN__
#include "ColorFile.h"
#include "CoordinateFile.h"
#include "FileUtilities.h"
#include "MathUtilities.h"
#include "SpecFile.h"
#include "TopologyHelper.h"
#include "Caret6ProjectedItem.h"

/**
 * Constructor.
 */
BorderProjectionLink::BorderProjectionLink(const int sectionIn,
                                           const int verticesIn[3],
                                           const float areasIn[3],
                                           const float radiusIn)
{
   borderProjectionFile = NULL;
   setData(sectionIn, verticesIn, areasIn, radiusIn);
}

/**
 * Destructor.
 */
BorderProjectionLink::~BorderProjectionLink()
{
}
      
/// determine if two border projection links are the same
bool 
BorderProjectionLink::operator==(const BorderProjectionLink& bpl) const
{
   for (int i = 0; i < 3; i++) {
      if (vertices[i] != bpl.vertices[i]) {
         return false;
      }
      if (areas[i] != bpl.areas[i]) {
         return false;
      }
   }
   return true;
}

/**
 * Set the Link's data.
 */
void 
BorderProjectionLink::setData(const int sectionIn,
                              const int verticesIn[3],
                              const float areasIn[3],
                              const float radiusIn)
{
   section     = sectionIn;
   vertices[0] = verticesIn[0];
   vertices[1] = verticesIn[1];
   vertices[2] = verticesIn[2];
   areas[0]    = areasIn[0];
   areas[1]    = areasIn[1];
   areas[2]    = areasIn[2];
   radius      = radiusIn;
   
   if (borderProjectionFile != NULL) {
      borderProjectionFile->setModified();
   }
}
                       
/**
 * Get the link's data.
 */
void 
BorderProjectionLink::getData(int& sectionOut,
                              int verticesOut[3],
                              float areasOut[3],
                              float& radiusOut) const
{
   sectionOut     = section;
   verticesOut[0] = vertices[0];
   verticesOut[1] = vertices[1];
   verticesOut[2] = vertices[2];
   areasOut[0]    = areas[0];
   areasOut[1]    = areas[1];
   areasOut[2]    = areas[2];
   radiusOut      = radius;
}

/**
 * unproject a border link.
 */
void 
BorderProjectionLink::unprojectLink(const CoordinateFile* unprojectCoordFile,
                                  float xyzOut[3]) const
{
   const float totalArea = areas[0] + areas[1] + areas[2];
   if (totalArea > 0.0) {
      const float* v1 = unprojectCoordFile->getCoordinate(vertices[0]);
      const float* v2 = unprojectCoordFile->getCoordinate(vertices[1]);
      const float* v3 = unprojectCoordFile->getCoordinate(vertices[2]);
      xyzOut[0] = (v1[0] * areas[1] + v2[0] * areas[2] + v3[0] * areas[0])
                     / totalArea;
      xyzOut[1] = (v1[1] * areas[1] + v2[1] * areas[2] + v3[1] * areas[0])
                     / totalArea;
      xyzOut[2] = (v1[2] * areas[1] + v2[2] * areas[2] + v3[2] * areas[0])
                     / totalArea;
   }
}

/**
 * get center of gravity.
 */
void 
BorderProjection::getCenterOfGravity(const CoordinateFile* coordFile,
                                     float centerOfGravityOut[3]) const
{
   centerOfGravityOut[0] = 0.0;
   centerOfGravityOut[1] = 0.0;
   centerOfGravityOut[2] = 0.0;
   
   if (coordFile == NULL) {
      return;
   }
   
   const int numLinks = getNumberOfLinks();
   if (numLinks <= 0) {
      return;
   }
   
   //
   // Loop through links and compute center of gravity
   //
   double sum[3] = { 0.0, 0.0, 0.0 };
   for (int i = 0; i < numLinks; i++) {
      //
      // Get link XYZ
      //
      float linkXYZ[3];
      links[i].unprojectLink(coordFile,
                             linkXYZ);
      sum[0] += linkXYZ[0];
      sum[1] += linkXYZ[1];
      sum[2] += linkXYZ[2];
   }
                         
   //
   // Set COG
   //
   centerOfGravityOut[0] = sum[0] / numLinks;
   centerOfGravityOut[1] = sum[1] / numLinks;
   centerOfGravityOut[2] = sum[2] / numLinks;
}      

/**
 * remove links from border that are not within the specified extent.
 */
void 
BorderProjection::removeLinksOutsideExtent(const CoordinateFile* unprojectCoordFile,
                                           const float extent[6])
{
   //
   // Keeps track of links that are NOT nibbled
   //
   std::vector<BorderProjectionLink> linkTemp;
   
   //
   // Loop through links
   //
   const int numLinks = getNumberOfLinks();
   for (int i = 0; i < numLinks; i++) {
      //
      // Get link XYZ
      //
      float linkXYZ[3];
      links[i].unprojectLink(unprojectCoordFile,
                             linkXYZ);
                             
      //
      // Keep link if it is within extent
      //
      if ((linkXYZ[0] >= extent[0]) &&
          (linkXYZ[0] <= extent[1]) &&
          (linkXYZ[1] >= extent[2]) &&
          (linkXYZ[1] <= extent[3]) &&
          (linkXYZ[2] >= extent[4]) &&
          (linkXYZ[2] <= extent[5])) {
         linkTemp.push_back(links[i]);
      }
   }   

   //
   // were links removed
   //
   if (links.size() != linkTemp.size()) {
      links = linkTemp;
      if (borderProjectionFile != NULL) {
         borderProjectionFile->setModified();
      }
   }
}

/**
 * get the link number nearest to a coordinate.
 */
int 
BorderProjection::getLinkNumberNearestToCoordinate(const CoordinateFile* cf,
                                                   const float xyz[3]) const
{
   int nearestLinkNumber = -1;
   float nearestDistanceSQ = std::numeric_limits<float>::max();
   
   const int numLinks = getNumberOfLinks();
   for (int i = 0; i < numLinks; i++) {
      float pos[3];
      getBorderProjectionLink(i)->unprojectLink(cf, pos);
      const float distSQ = MathUtilities::distanceSquared3D(xyz, pos);
      if (distSQ < nearestDistanceSQ) {
         nearestDistanceSQ = distSQ;
         nearestLinkNumber = i;
      }
   }
   
   return nearestLinkNumber;
}

/**
 * get the link number furthest from a coordinate.
 */
int 
BorderProjection::getLinkNumberFurthestFromCoordinate(const CoordinateFile* cf,
                                                      const float xyz[3]) const
{
   int furthestLinkNumber = -1;
   float furthestDistanceSQ = -1.0;
   
   const int numLinks = getNumberOfLinks();
   for (int i = 0; i < (numLinks - 1); i++) {
      float pos[3];
      getBorderProjectionLink(i)->unprojectLink(cf, pos);
      const float distSQ = MathUtilities::distanceSquared3D(xyz, pos);
      if (distSQ > furthestDistanceSQ) {
         furthestDistanceSQ = distSQ;
         furthestLinkNumber = i;
      }
   }
   
   return furthestLinkNumber;
}

/**
 * split a border that is approximately linear in shape.
 * Returns true if successful.
 */
bool 
BorderProjection::splitClosedBorderProjection(const CoordinateFile* cf,
                                              const int startingLinkNumber,
                                              const QString& newNameSuffix,
                                              BorderProjection& halfOneOut,
                                              BorderProjection& halfTwoOut,
                                              const int endingLinkNumber)
{  
   const int numLinks = getNumberOfLinks();
   if (numLinks < 3) {
      return false;
   }
   
   //
   // Get coordinate of starting link
   //
   float startXYZ[3];
   getBorderProjectionLink(startingLinkNumber)->unprojectLink(cf, startXYZ);
   
   //
   // Get the border link furthest from the starting link
   //
   const int splitLinkNumber = 
      ((endingLinkNumber >= 0) ?
         endingLinkNumber:
         getLinkNumberFurthestFromCoordinate(cf, startXYZ));
   
   //
   // Set indices for splitting
   //
   int oneEnd = splitLinkNumber - 1;
   if (oneEnd < 0) {
      oneEnd = getNumberOfLinks() - 1;
   }
   int twoEnd = startingLinkNumber - 1;
   if (twoEnd < 0) {
      twoEnd = getNumberOfLinks() - 1;
   }
   
   //
   // Create the new borders
   //
   halfOneOut = getSubSetOfBorderProjectionLinks(startingLinkNumber, 
                                                 oneEnd);
   halfTwoOut = getSubSetOfBorderProjectionLinks(splitLinkNumber, 
                                                 twoEnd);
   
   //
   // Set the names of the output projections
   //
   halfOneOut.setName(getName() + newNameSuffix);
   halfTwoOut.setName(getName() + newNameSuffix);
   
   return true;
}                                       

/**
 * unproject a border.
 */
void 
BorderProjection::unprojectBorderProjection(const CoordinateFile* cf,
                                            Border& borderOut) const
{
   borderOut.clearLinks();
   borderOut.setName(getName());
   
   const int numLinks = getNumberOfLinks();
   for (int i = 0; i < numLinks; i++) {
      const BorderProjectionLink* bpl = getBorderProjectionLink(i);
      float xyz[3];
      bpl->unprojectLink(cf, xyz);
      borderOut.addBorderLink(xyz);
   }
}
                           
/**
 * unproject a border.
 */
void 
BorderProjection::unprojectBorderProjection(const CoordinateFile* cf,
                                            const TopologyHelper* th,
                                            Border& borderOut) const
{
   borderOut.clearLinks();
   borderOut.setName(getName());
   
   const int numLinks = getNumberOfLinks();
   for (int i = 0; i < numLinks; i++) {
      const BorderProjectionLink* bpl = getBorderProjectionLink(i);
      if (th->getNodeHasNeighbors(bpl->vertices[0]) &&
          th->getNodeHasNeighbors(bpl->vertices[1]) &&
          th->getNodeHasNeighbors(bpl->vertices[2])) {
         float xyz[3];
         bpl->unprojectLink(cf, xyz);
         borderOut.addBorderLink(xyz);
      }
   }
}

/**
 * get the center of gravity for a border.
 */
bool 
BorderProjection::getCenterOfGravity(const CoordinateFile* cf,
                                     const TopologyHelper* th,
                                     float cogXYZOut[3]) const
{
   //
   // Unproject to get border
   //
   Border border;
   unprojectBorderProjection(cf, th, border);
   
   //
   // get border's COG
   //
   const bool valid = border.getCenterOfGravity(cogXYZOut);
   return valid;
}                           
                           
/**
 * change the starting link of a closed border so it is close to a point.
 */
void 
BorderProjection::changeStartingLinkOfClosedBorderToBeNearPoint(const CoordinateFile* cf,
                                                                const float pointXYZ[3])
{
   //
   // Unproject to a border
   //
   Border border;
   unprojectBorderProjection(cf, border);
   
   //
   // Find link nearest point
   //
   int closestLinkNumber = -1;
   float closestLinkDistance = std::numeric_limits<float>::max();
   const int numLinks = border.getNumberOfLinks();
   for (int i = 0; i < numLinks; i++) {
      float xyz[3];
      border.getLinkXYZ(i, xyz);
      const float dist = MathUtilities::distanceSquared3D(xyz, pointXYZ);
      if (dist < closestLinkDistance) {
         closestLinkDistance = dist;
         closestLinkNumber = i;
      }
   }
   
   //
   // If no links or closest link is first link
   //
   if (closestLinkNumber <= 0) {
      return;
   }
   
   //
   // Reorganize the links
   //
   const std::vector<BorderProjectionLink> linksCopy = links;
   links.clear();
   for (int i = closestLinkNumber; i < numLinks; i++) {
      addBorderProjectionLink(linksCopy[i]);
   }
   for (int i = 0; i < closestLinkNumber; i++) {
      addBorderProjectionLink(linksCopy[i]);
   }
}
      
/**
 * remove links in border before/after link nearest to point.
 */
void 
BorderProjection::removeLinksBeforeAfterLinkNearestPoint(const CoordinateFile* cf,
                                                         const float pointXYZ[3],
                                                         const bool removeAfterFlag,
                                                         const bool removeBeforeFlag)
{
   //
   // Unproject to a border
   //
   Border border;
   unprojectBorderProjection(cf, border);
   
   //
   // Find link nearest point
   //
   int closestLinkNumber = -1;
   float closestLinkDistance = std::numeric_limits<float>::max();
   const int numLinks = border.getNumberOfLinks();
   for (int i = 0; i < numLinks; i++) {
      float xyz[3];
      border.getLinkXYZ(i, xyz);
      const float dist = MathUtilities::distanceSquared3D(xyz, pointXYZ);
      if (dist < closestLinkDistance) {
         closestLinkDistance = dist;
         closestLinkNumber = i;
      }
   }
   
   //
   // If no links or closest link is first link
   //
   if (closestLinkNumber <= 0) {
      return;
   }
   
   //
   // keep needed the links
   //
   const std::vector<BorderProjectionLink> linksCopy = links;
   links.clear();
   if (removeBeforeFlag == false) {
      for (int i = 0; i < closestLinkNumber; i++) {
         addBorderProjectionLink(linksCopy[i]);
      }
   }
   if (removeAfterFlag == false) {
      for (int i = closestLinkNumber + 1; i < numLinks; i++) {
         addBorderProjectionLink(linksCopy[i]);
      }
   }
}
      
/**
 * remove links from border greater than specified distances of point
 * if a specified distance is zero or less it is ignored.
 */
void 
BorderProjection::removeLinksAwayFromPoint(const CoordinateFile* unprojectCoordFile,
                                           const float pointXYZ[3],
                                           const float xDistance,
                                           const float yDistance,
                                           const float zDistance,
                                           const float straightLineDistance)
{
   //
   // Squared nibble distance
   //
   float squaredStraightLineDistance = straightLineDistance * straightLineDistance;
   
   //
   // Keeps track of links that are NOT nibbled
   //
   std::vector<BorderProjectionLink> linkTemp;
   
   //
   // Loop through links
   //
   const int numLinks = getNumberOfLinks();
   for (int i = 0; i < numLinks; i++) {
      //
      // Get link XYZ
      //
      float linkXYZ[3];
      links[i].unprojectLink(unprojectCoordFile,
                                            linkXYZ);
                                            
      //
      // intially keep the link
      //
      bool keepLinkFlag = true;
      
      //
      // Get distance components
      //
      const float dx = std::fabs(pointXYZ[0] - linkXYZ[0]);
      const float dy = std::fabs(pointXYZ[1] - linkXYZ[1]);
      const float dz = std::fabs(pointXYZ[2] - linkXYZ[2]);
      
      //
      // Check distances
      //
      if (dx > xDistance) {
         keepLinkFlag = false;
      }
      else if (dy > yDistance) {
         keepLinkFlag = false;
      }
      else if (dz > zDistance) {
         keepLinkFlag = false;
      }
      else {
         //
         // Distance Squared from point to link 
         //
         const float distanceSquared = dx*dx + dy*dy + dz*dz;
         if (distanceSquared > squaredStraightLineDistance) {
            keepLinkFlag = false;
         }
      }
                                                          
      //
      // Should this link be kept
      //
      if (keepLinkFlag) {
         linkTemp.push_back(links[i]);
      }
   }
   
   //
   // were links removed
   //
   if (links.size() != linkTemp.size()) {
      links = linkTemp;
      if (borderProjectionFile != NULL) {
         borderProjectionFile->setModified();
      }
   }
}

/**
 * remove links from border within specified distance of point.
 * if a specified distance is zero or less it is ignored
 */
void 
BorderProjection::removeLinksNearPoint(const CoordinateFile* unprojectCoordFile,
                                       const float pointXYZ[3],
                                       const float xDistance,
                                       const float yDistance,
                                       const float zDistance,
                                       const float straightLineDistance)
{
   //
   // Squared nibble distance
   //
   float squaredStraightLineDistance = straightLineDistance * straightLineDistance;
   
   //
   // Keeps track of links that are NOT nibbled
   //
   std::vector<BorderProjectionLink> linkTemp;
   
   //
   // Loop through links
   //
   const int numLinks = getNumberOfLinks();
   for (int i = 0; i < numLinks; i++) {
      //
      // Get link XYZ
      //
      float linkXYZ[3];
      links[i].unprojectLink(unprojectCoordFile,
                                            linkXYZ);
                                            
      //
      // intially keep the link
      //
      bool keepLinkFlag = true;
      
      //
      // Get distance components
      //
      const float dx = std::fabs(pointXYZ[0] - linkXYZ[0]);
      const float dy = std::fabs(pointXYZ[1] - linkXYZ[1]);
      const float dz = std::fabs(pointXYZ[2] - linkXYZ[2]);
      
      //
      // Check distances
      //
      if (dx < xDistance) {
         keepLinkFlag = false;
      }
      else if (dy < yDistance) {
         keepLinkFlag = false;
      }
      else if (dz < zDistance) {
         keepLinkFlag = false;
      }
      else {
         //
         // Distance Squared from point to link 
         //
         const float distanceSquared = dx*dx + dy*dy + dz*dz;
         if (distanceSquared < squaredStraightLineDistance) {
            keepLinkFlag = false;
         }
      }
                                                          
      //
      // Should this link be kept
      //
      if (keepLinkFlag) {
         linkTemp.push_back(links[i]);
      }
   }
   
   //
   // were links removed
   //
   if (links.size() != linkTemp.size()) {
      links = linkTemp;
      if (borderProjectionFile != NULL) {
         borderProjectionFile->setModified();
      }
   }
}
                                
/**
 * Remove the border projection link at the specified index
 */
void
BorderProjection::removeBorderProjectionLink(const int linkNumber)
{
   if (linkNumber < getNumberOfLinks()) {
      links.erase(links.begin() + linkNumber);
      if (borderProjectionFile != NULL) {
         borderProjectionFile->setModified();
      }
   }
}

/**
 * reverse the order of the border projection links.
 */
void 
BorderProjection::reverseOrderOfBorderProjectionLinks()
{
   if (links.empty() == false) {
      std::reverse(links.begin(), links.end());
      if (borderProjectionFile != NULL) {
         borderProjectionFile->setModified();
      }
   }
}
      
/** 
 * Constructor.
 */
BorderProjection::BorderProjection(const QString& nameIn, const float* centerIn3, 
               const float samplingDensityIn, const float varianceIn,
               const float topographyIn, const float arealUncertaintyIn)
{
   uniqueID = uniqueIDSource++;
   borderProjectionFile = NULL;
   borderColorIndex = 0;
   float center[3] = { 0.0, 0.0, 0.0 };
   if (centerIn3 != NULL) {
      center[0] = centerIn3[0];
      center[1] = centerIn3[1];
      center[2] = centerIn3[2];
   }
   setData(nameIn, center, samplingDensityIn,
           varianceIn, topographyIn, arealUncertaintyIn);
}

/**
 * Destructor
 */
BorderProjection::~BorderProjection()
{
}

/**
 * determine if two border projections are the same.
 */
bool 
BorderProjection::operator==(const BorderProjection& bp) const
{
   const int numLinks = getNumberOfLinks();
   if (numLinks != bp.getNumberOfLinks()) {
      return false;
   }
   
   if (getName() != bp.getName()) {
      return  false;
   }
   
   for (int i = 0; i < numLinks; i++) {
      const BorderProjectionLink* b1 = getBorderProjectionLink(i);
      const BorderProjectionLink* b2 = bp.getBorderProjectionLink(i);
      if ((*b1 == *b2) == false) {
         return false;
      }
   }
   
   return true;
}

/** 
 * Get the data from a border.
 */
void 
BorderProjection::getData(QString& nameOut, float centerOut[3], 
                float& samplingDensityOut, float& varianceOut,
                float& topographyOut, float& arealUncertaintyOut) const
{
   nameOut = name;
   centerOut[0] = center[0];
   centerOut[1] = center[1];
   centerOut[2] = center[2];
   samplingDensityOut = samplingDensity;
   varianceOut = variance;
   topographyOut = topographyValue;
   arealUncertaintyOut = arealUncertainty;
}                  

/** 
 * Set the data in a border.
 */
void
BorderProjection::setData(const QString& nameIn, const float centerIn[3], 
                const float samplingDensityIn, const float varianceIn,
                const float topographyIn, const float arealUncertaintyIn)
{
   name = nameIn;
   center[0] = centerIn[0];
   center[1] = centerIn[1];
   center[2] = centerIn[2];
   samplingDensity = samplingDensityIn;
   variance = varianceIn;
   topographyValue = topographyIn;
   arealUncertainty = arealUncertaintyIn;
   
   if (borderProjectionFile != NULL) {
      borderProjectionFile->setModified();
   }
}

/**
 * append a border projection to this border projection.
 */
void 
BorderProjection::append(const BorderProjection& bp)
{
   const int num = bp.getNumberOfLinks();
   for (int i = 0; i < num; i++) {
      addBorderProjectionLink(*(bp.getBorderProjectionLink(i)));
   }
}
      
/**
 * add border projection link directly on node.
 */
void 
BorderProjection::addBorderProjectionLinkOnNode(const int nodeNumber)
{
   const int nodes[3] = { nodeNumber, nodeNumber, nodeNumber };
   const float areas[3] = { 0.333, 0.333, 0.334 };
   BorderProjectionLink bpl(0,
                            nodes,
                            areas,
                            1.0);
   addBorderProjectionLink(bpl);
}
      
/** 
 * Add a link to the border.
 */
void
BorderProjection::addBorderProjectionLink(const BorderProjectionLink& bl)
{
   //
   // Make sure border projection link is valid
   //
   if ((bl.vertices[0] >= 0) || (bl.vertices[1] >= 0) || (bl.vertices[2] >= 0.0)) {
      links.push_back(bl);
      const int indx = static_cast<int>(links.size()) - 1;
      links[indx].borderProjectionFile = borderProjectionFile;
   }
}

/**
 * insert a border projection link before the specified link number (use number of links for end).
 */
void 
BorderProjection::insertBorderProjectionLink(const int linkIndex,
                                             const BorderProjectionLink& bl)
{
   if (linkIndex >= getNumberOfLinks()) {
      addBorderProjectionLink(bl);
   }
   else if (linkIndex < getNumberOfLinks()) {
      links.insert(links.begin() + linkIndex,
                   bl);
      links[linkIndex].borderProjectionFile = borderProjectionFile;
   }
}

/**
 * get a subset of the links as a border projection.
 */
BorderProjection
BorderProjection::getSubSetOfBorderProjectionLinks(const int startLinkNumber,
                                                  const int endLinkNumber) const
{
   BorderProjection bp("Subset of " + getName());
   
   if (startLinkNumber < endLinkNumber) {
      for (int i = startLinkNumber; i <= endLinkNumber; i++) {
         bp.addBorderProjectionLink(*getBorderProjectionLink(i));
      }
   }
   else {
      const int numLinks = getNumberOfLinks() - 1;
      for (int i = startLinkNumber; i < numLinks; i++) {
         bp.addBorderProjectionLink(*getBorderProjectionLink(i));
      }
      for (int i = 0; i <= endLinkNumber; i++) {
         bp.addBorderProjectionLink(*getBorderProjectionLink(i));
      }
   }
   
   return bp;
}                                                        
                                 
/**
 * insert a border projection link before the specified link number (use number of links for end).
 */
void 
BorderProjection::insertBorderProjectionLinkOnNode(const int linkIndex,
                                                   const int nodeNumber)
{
   const int nodes[3] = { nodeNumber, nodeNumber, nodeNumber };
   const float areas[3] = { 0.333, 0.333, 0.334 };
   BorderProjectionLink bpl(0,
                            nodes,
                            areas,
                            1.0);
   insertBorderProjectionLink(linkIndex, bpl);
}                                       

/**
 * remove duplicate border projection links.
 */
void 
BorderProjection::removeDuplicateBorderProjectionLinks()      
{
   const int num = getNumberOfLinks();
   if (num > 0) {
      std::vector<int> linksToDelete;
      for (int i = 0; i < (num - 1); i++) {
         const BorderProjectionLink* bpl = getBorderProjectionLink(i);
         for (int j = i + 1; j < num; j++) {
            const BorderProjectionLink* bpl2 = getBorderProjectionLink(j);
            if (*bpl == *bpl2) {
               linksToDelete.push_back(j);
            }
         }
      }
      
      std::unique(linksToDelete.begin(), linksToDelete.end());
      std::sort(linksToDelete.begin(), linksToDelete.end());
      std::reverse(linksToDelete.begin(), linksToDelete.end());
      const int numToDelete = static_cast<int>(linksToDelete.size());
      for (int i = 0; i < numToDelete; i++) {
         removeBorderProjectionLink(linksToDelete[i]);
      }
   }
}

/**
 * remove the last border projection link.
 */
void 
BorderProjection::removeLastBorderProjectionLink()
{
   const int num = getNumberOfLinks();
   if (num > 0) {
      removeBorderProjectionLink(num - 1);
   }
}
      
/**
 * Constructor.
 */
BorderProjectionFile::BorderProjectionFile()
   : AbstractFile("Border Projection File",
                  SpecFile::getBorderProjectionFileExtension())
{
   clear();
}

/**
 * Destructor.
 */
BorderProjectionFile::~BorderProjectionFile()
{
   clear();
}

/**
 * Append a border projection file to this one.
 */
void
BorderProjectionFile::append(BorderProjectionFile& bpf)
{
   const int numProj = bpf.getNumberOfBorderProjections();
   for (int i = 0; i < numProj; i++) {
      BorderProjection* bp = bpf.getBorderProjection(i);
      addBorderProjection(*bp);
   }
   
   //
   // transfer the file's comment
   //
   appendFileComment(bpf);
}

/**
 * Assign colors to these border projections.
 */
void 
BorderProjectionFile::assignColors(const ColorFile& colorFile)
{
   const int numProj = getNumberOfBorderProjections();
   for (int i = 0; i < numProj; i++) {
      BorderProjection* bp = getBorderProjection(i);
      bool match;
      bp->setBorderColorIndex(colorFile.getColorIndexByName(bp->getName(), match));
   }
}

/**
 * Clear the border projection file's data.
 */
void
BorderProjectionFile::clear()
{
   clearAbstractFile();
   BorderProjection::uniqueIDSource = 1;
   links.clear();
}

/**
 * add a border projection to the file.
 */
void
BorderProjectionFile::addBorderProjection(const BorderProjection& b)
{
   links.push_back(b);
   const int indx = static_cast<int>(links.size()) - 1;
   links[indx].borderProjectionFile = this;
   for (int i = 0; i < links[indx].getNumberOfLinks(); i++) {
      BorderProjectionLink* bpl = links[indx].getBorderProjectionLink(i);
      bpl->borderProjectionFile = this;
   }
   setModified();
}

/**
 * get the indices of border projections that area duplicates of other border projections.
 */
void 
BorderProjectionFile::getDuplicateBorderProjectionIndices(
                               std::vector<int>& duplicateBorderProjIndices) const
{
   duplicateBorderProjIndices.clear();
   
   const int numBorders = getNumberOfBorderProjections();
   for (int i = 0; i < (numBorders - 1); i++) {
      const BorderProjection* b = getBorderProjection(i);
      
      for (int j = i + 1; j < numBorders; j++) {
         const BorderProjection* b2 = getBorderProjection(j);
         if (*b == *b2) {
            duplicateBorderProjIndices.push_back(j);
         }
      }
   }
}
      
/**
 * get first specified border by name.
 */
BorderProjection* 
BorderProjectionFile::getFirstBorderProjectionByName(const QString& name)
{
   const int numBorders = getNumberOfBorderProjections();
   for (int i = 0; i < numBorders; i++) {
      BorderProjection* b = getBorderProjection(i);
      if (b->getName() == name) {
         return b;
      }
   }
   
   return NULL;
}

/**
 * get first specified border by name.
 */
const BorderProjection* 
BorderProjectionFile::getFirstBorderProjectionByName(const QString& name) const
{
   const int numBorders = getNumberOfBorderProjections();
   for (int i = 0; i < numBorders; i++) {
      const BorderProjection* b = getBorderProjection(i);
      if (b->getName() == name) {
         return b;
      }
   }
   
   return NULL;
}

/**
 * get last specified border by name.
 */
BorderProjection* 
BorderProjectionFile::getLastBorderProjectionByName(const QString& name)
{
   const int numBorders = getNumberOfBorderProjections();
   for (int i = (numBorders - 1); i >= 0; i--) {
      BorderProjection* b = getBorderProjection(i);
      if (b->getName() == name) {
         return b;
      }
   }
   
   return NULL;
}

/**
 * get last specified border by name.
 */
const BorderProjection* 
BorderProjectionFile::getLastBorderProjectionByName(const QString& name) const
{
   const int numBorders = getNumberOfBorderProjections();
   for (int i = (numBorders - 1); i >= 0; i--) {
      const BorderProjection* b = getBorderProjection(i);
      if (b->getName() == name) {
         return b;
      }
   }
   
   return NULL;
}

/**
 * Get the border projection with the unique ID
 */
BorderProjection*
BorderProjectionFile::getBorderProjectionWithUniqueID(const int uniqueID)
{
   const int indx = getBorderProjectionIndexWithUniqueID(uniqueID);
   if (indx >= 0) {
      return getBorderProjection(indx);
   }
   return NULL;
}

/**
 * Get the index of the border projection with the unique ID
 */
int
BorderProjectionFile::getBorderProjectionIndexWithUniqueID(const int uniqueID) const
{
   const int num = getNumberOfBorderProjections();
   for (int i = 0; i < num; i++) {
      const BorderProjection* b = getBorderProjection(i);
      if (b->getUniqueID() == uniqueID) {
         return i;
      }
   }
   return -1;
}

/**
 * Remove the border projection with the specified ID
 */
void
BorderProjectionFile::removeBorderProjectionWithUniqueID(const int uniqueID)
{
   const int indx = getBorderProjectionIndexWithUniqueID(uniqueID);
   if (indx >= 0) {
      removeBorderProjection(indx);
      setModified();
   }
}

/**
 * Remove the border projection at the specified index
 */
void
BorderProjectionFile::removeBorderProjection(const int indx)
{
   if (indx < getNumberOfBorderProjections()) {
      links.erase(links.begin() + indx);
      setModified();
   }
}

/**
 * get the index of a border projection.
 */
int 
BorderProjectionFile::getBorderProjectionIndex(const BorderProjection* bp) const
{
   const int num = getNumberOfBorderProjections();
   for (int i = 0; i < num; i++) {
      if (bp == getBorderProjection(i)) {
         return i;
      }
   }
   return -1;
}
      
/**
 * get the border projection with the largest number of links.
 */
BorderProjection* 
BorderProjectionFile::getBorderProjectionWithLargestNumberOfLinks()      
{
   const int num = getNumberOfBorderProjections();
   int mostLinks = -1;
   BorderProjection* mostLinksBJ = NULL;
   for (int i = 0; i < num; i++) {
      BorderProjection* bj = getBorderProjection(i);
      if (bj->getNumberOfLinks() > mostLinks) {
         mostLinks = bj->getNumberOfLinks();
         mostLinksBJ = bj;
      }
   }
   
   return mostLinksBJ;
}
   
/**
 * reverse order of links in all border projections.
 */
void 
BorderProjectionFile::reverseOrderOfAllBorderProjections()
{
   const int num = getNumberOfBorderProjections();
   for (int i = 0; i < num; i++) {
      BorderProjection* bp = getBorderProjection(i);
      bp->reverseOrderOfBorderProjectionLinks();
   }
   setModified();
}
      
/**
 * remove borders with the specified name.
 */
void 
BorderProjectionFile::removeBordersWithName(const QString& nameIn)
{
   std::vector<int> indicesOfBorderToDelete;
   
   const int num = getNumberOfBorderProjections();
   for (int i = 0; i < num; i++) {
      BorderProjection* bp = getBorderProjection(i);
      const QString s = bp->getName();
      if (s == nameIn) {
         indicesOfBorderToDelete.push_back(i);
      }
   }
   
   removeBordersWithIndices(indicesOfBorderToDelete);
}

/** 
 * remove borders with the specified indices.
 */
void 
BorderProjectionFile::removeBordersWithIndices(const std::vector<int>& borderProjectionIndicesIn)
{
   std::vector<int> borderProjectionIndices = borderProjectionIndicesIn;
   std::sort(borderProjectionIndices.begin(), borderProjectionIndices.end());
   const int num = static_cast<int>(borderProjectionIndices.size());
   for (int i = (num - 1); i >= 0; i--) {
      removeBorderProjection(borderProjectionIndices[i]);
   }
}

/**
 * compare a file for unit testing (returns true if "within tolerance").
 */
bool 
BorderProjectionFile::compareFileForUnitTesting(const AbstractFile* af,
                                                const float tolerance,
                                                QString& messageOut) const
{
   const BorderProjectionFile* bpf = dynamic_cast<const BorderProjectionFile*>(af);
   if (bpf == NULL) {
      messageOut = "File for comparison is not a border projection file.";
      return false;
   }
   
   const int numProj = getNumberOfBorderProjections();
   if (numProj != bpf->getNumberOfBorderProjections()) {
      messageOut = "Files have a different number of border projections.";
      return false;
   }
   
   for (int i = 0; i < numProj; i++) {
      const BorderProjection* bp1 = getBorderProjection(i);
      const BorderProjection* bp2 = bpf->getBorderProjection(i);
      
      if (bp1->getName() != bp2->getName()) {
         messageOut = "Border " 
                    + QString::number(i)
                    + " names to not match "
                    + bp1->getName()
                    + " and " 
                    + bp2->getName();
         return false;
      }
      
      const int numLinks = bp1->getNumberOfLinks();
      if (numLinks != bp2->getNumberOfLinks()) {
         messageOut = "Border Projection has a different number of links.";
         return false;
      }
      
      for (int j = 0; j < numLinks; j++) {
         const BorderProjectionLink* link1 = bp1->getBorderProjectionLink(j);
         const BorderProjectionLink* link2 = bp2->getBorderProjectionLink(j);
         
         int s1, s2;
         int v1[3], v2[3];
         float a1[3], a2[3];
         float r1, r2;
         link1->getData(s1, v1, a1, r1);
         link2->getData(s2, v2, a2, r2);
         
         if ((v1[0] != v2[0]) ||
             (v1[1] != v2[1]) ||
             (v1[2] != v2[2])) {
            messageOut = "border link has non-matching vertices";
            return false;
         }
         
         const float da1 = std::fabs(a1[0] - a2[0]);
         const float da2 = std::fabs(a1[1] - a2[1]);
         const float da3 = std::fabs(a1[2] - a2[2]);
         if ((da1 > tolerance) ||
             (da2 > tolerance) ||
             (da3 > tolerance)) {
            messageOut = "border link has non-matching areas";
            return false;
         }
      }
   }
   
   return true;
}
      
/** 
 * Read the border projection file data.
 */
void 
BorderProjectionFile::readFileData(QFile& /*file*/, 
                                   QTextStream& stream, 
                                   QDataStream&,
                                   QDomElement& /* rootElement */) throw (FileException)
{ 
   //
   // Should reading data be skipped ?
   //
   if (getReadMetaDataOnlyFlag()) {
      return;
   }
   QString line;
   
   // 1st line contains number of borders
   readLine(stream, line);
   const int numBorders = line.toInt();
   
   for (int i = 0; i < numBorders; i++) {
      readLine(stream, line);
      int borderNumber;
      int numLinks = 0;
      QString borderName;
      float samplingDensity = 25.0, variance = 1.0, topography = 0.0, uncertainty = 1.0;
      
      QTextStream(&line, QIODevice::ReadOnly) >> borderNumber
                         >> numLinks
                         >> borderName
                         >> samplingDensity
                         >> variance
                         >> topography
                         >> uncertainty;
      
      // line containing the center (ignore)
      float center[3] = { 0.0, 0.0, 0.0 };
      readLine(stream, line);
      
      // create the border
      BorderProjection borderProj(borderName, center, samplingDensity, variance, 
                                 topography, uncertainty);
      
      for (int j = 0; j < numLinks; j++) {
         readLine(stream, line);
         
         int sectionNumber;
         int vertices[3];
         float areas[3];
         float radius = 0.0;
         
         QTextStream(&line, QIODevice::ReadOnly) >> vertices[0]
                             >> vertices[1]
                             >> vertices[2]
                             >> sectionNumber
                             >> areas[0]
                             >> areas[1]
                             >> areas[2]
                             >> radius;

         BorderProjectionLink bl(sectionNumber, vertices, areas, radius);
         bl.borderProjectionFile = this;
         borderProj.addBorderProjectionLink(bl);
      }
      
      addBorderProjection(borderProj);
   }
}

/** 
 * Write the border projection file data.
 */
void 
BorderProjectionFile::writeFileData(QTextStream& stream, QDataStream&,
                                 QDomDocument& /* xmlDoc */,
                                    QDomElement& /* rootElement */) throw (FileException)
{
   int numBorders = getNumberOfBorderProjections();

   //
   // only write borders that have links
   //
   int numBordersToWrite = 0;
   for (int i = 0; i < numBorders; i++) {
      if (getBorderProjection(i)->getNumberOfLinks() > 0) {
         numBordersToWrite++;
      }
   }
   stream << numBordersToWrite << "\n";
   
   for (int i = 0; i < numBorders; i++) {
      BorderProjection* border = getBorderProjection(i);
      const int numLinks = border->getNumberOfLinks();
      
      //
      // Only write borders with links
      //
      if (numLinks <= 0) {
         continue;
      }
      
      // write border number, number links, name, sampling, variance, topography
      QString name;
      float center[3];
      float samplingDensity, variance, topography, uncertainty;
      border->getData(name, center, samplingDensity, variance, 
                      topography, uncertainty);
      stream << i << " "
             << numLinks << " "
             << name << " "
             << samplingDensity << " "
             << variance << " "
             << topography << " "
             << uncertainty << "\n";
              
      // write border center (unused)
      stream << center[0] << " " << center[1] << " " << center[2] << "\n";
      
      for (int j = 0; j < numLinks; j++) {
         BorderProjectionLink* bl = border->getBorderProjectionLink(j);
         
         // write vertices, section, and areas
         float areas[3];
         float radius;
         int vertices[3];
         int section;
         bl->getData(section, vertices, areas, radius);
         stream << vertices[0] << " "
                << vertices[1] << " "
                << vertices[2] << " "
                << section << " "
                << areas[0] << " "
                << areas[1] << " "
                << areas[2] << " "
                << radius << "\n";
      }
   }
}

/**
 * Write the file's memory in caret6 format to the specified name.
 */
QString
BorderProjectionFile::writeFileInCaret6Format(const QString& filenameIn, Structure structure,const ColorFile* colorFileIn, const bool useCaret6ExtensionFlag) throw (FileException)
{
   int numBorders = this->getNumberOfBorderProjections();
   if (numBorders <= 0) {
      throw FileException("Contains no borders");
   }
   QFile file(filenameIn);
   if (AbstractFile::getOverwriteExistingFilesAllowed() == false) {
      if (file.exists()) {
         throw FileException("file exists and overwrite is prohibited.");
      }
   }
   if (file.open(QFile::WriteOnly) == false) {
      throw FileException("Unable to open for writing");
   }
   QTextStream stream(&file);

   XmlGenericWriter xmlWriter(stream);
   xmlWriter.writeStartDocument();

   XmlGenericWriterAttributes attributes;
   attributes.addAttribute("CaretFileType", "BorderProjection");
   attributes.addAttribute("xmlns:xsi",
                           "http://www.w3.org/2001/XMLSchema-instance");
   attributes.addAttribute("xsi:noNamespaceSchemaLocation",
                           "http://brainvis.wustl.edu/caret6/xml_schemas/BorderProjectionFileSchema.xsd");
   attributes.addAttribute("Version", "6.0");
   xmlWriter.writeStartElement("CaretDataFile", attributes);

   this->writeHeaderXMLWriter(xmlWriter);

   GiftiLabelTable labelTable;
   if (colorFileIn != NULL) {
      labelTable.createLabelsFromColors(*colorFileIn);
   }
   labelTable.writeAsXML(xmlWriter);

   for (int i = 0; i < numBorders; i++) {
      BorderProjection* bp = this->getBorderProjection(i);
      int numLinks = bp->getNumberOfLinks();
      if (numLinks > 0) {

         XmlGenericWriterAttributes attributes;
         attributes.addAttribute("Index", i);
         xmlWriter.writeStartElement("BorderProjection", attributes);

         xmlWriter.writeElementCData("Name", bp->getName());

         QString structureName = "Invalid";
         if (structure.isLeftCortex()) {
            structureName = "CortexLeft";
         }
         else if (structure.isRightCortex()) {
            structureName = "CortexRight";
         }
         else if (structure.isCerebellum()) {
            structureName = "Cerebellum";
         }
         xmlWriter.writeElementCharacters("Structure",
                                          structureName);

         for (int j = 0; j < numLinks; j++) {
            BorderProjectionLink* bpl = bp->getBorderProjectionLink(j);
            int section, nodes[3];
            float areas[3], radius;
            bpl->getData(section, nodes, areas, radius);

            //
            // Area and node indices do not match in Caret5 but
            // they do in Caret6
            //  In Caret5:  node-index area-index
            //                   0         1
            //                   1         2
            //                   2         0
            //
            // But caret5 was clockwise order and we want
            // counter clockwise so
            //
            //    node-index  area-index
            //        2            0
            //        1            2
            //        0            1
            //
            //
            int nodesCaret6[3];
            float areasCaret6[3];
            areasCaret6[0] = areas[0];
            areasCaret6[1] = areas[2];
            areasCaret6[2] = areas[1];
            nodesCaret6[0] = nodes[2];
            nodesCaret6[1] = nodes[1];
            nodesCaret6[2] = nodes[0];

            xmlWriter.writeStartElement("Projection");
            xmlWriter.writeElementCharacters("Nodes", nodesCaret6, 3);
            xmlWriter.writeElementCharacters("Areas", areasCaret6, 3);
            xmlWriter.writeEndElement();
         }

         xmlWriter.writeEndElement();
      }
   }

   xmlWriter.writeEndElement();

   xmlWriter.writeEndDocument();
   
   file.close();

   return filenameIn;
}

/**
 * Write the file's memory in caret7 format to the specified name.
 */
QString
BorderProjectionFile::writeFileInCaret7Format(const QString& filenameIn, 
                                              Structure structure,
                                              const ColorFile* colorFileIn, 
                                              const bool useCaret7ExtensionFlag) throw (FileException)
{
    int numBorders = this->getNumberOfBorderProjections();
    if (numBorders <= 0) {
        throw FileException("Contains no borders");
    }

    QString name = filenameIn;
    if (useCaret7ExtensionFlag) {
        name = FileUtilities::replaceExtension(filenameIn, ".borderproj",
                                               ".border");
    }
    QFile file(name);
    if (AbstractFile::getOverwriteExistingFilesAllowed() == false) {
        if (file.exists()) {
            throw FileException("file exists and overwrite is prohibited.");
        }
    }
    if (file.open(QFile::WriteOnly) == false) {
        throw FileException("Unable to open for writing");
    }
    QTextStream stream(&file);
    
    XmlGenericWriter xmlWriter(stream);
    xmlWriter.writeStartDocument();
    
    XmlGenericWriterAttributes attributes;
    attributes.addAttribute("Version", "1.0");
    xmlWriter.writeStartElement("BorderFile", attributes);
    
    this->writeHeaderXMLWriter(xmlWriter);
    
    GiftiLabelTable labelTable;
    if (colorFileIn != NULL) {
        labelTable.createLabelsFromColors(*colorFileIn);
    }
    labelTable.writeAsXML(xmlWriter);
    
    for (int i = 0; i < numBorders; i++) {
        BorderProjection* bp = this->getBorderProjection(i);
        int numLinks = bp->getNumberOfLinks();
        if (numLinks > 0) {
            
            QString className = "";
            QString colorName = "BLACK";
            const QString borderName = bp->getName();
            const int labelIndex = labelTable.getBestMatchingLabelIndex(borderName);
            if (labelIndex >= 0) {
                className = labelTable.getLabel(labelIndex);
                if (className.isEmpty() == false) {
                    colorName = "CLASS";
                }
            }
            
            xmlWriter.writeStartElement("Border");
            
            xmlWriter.writeElementCharacters("Name", 
                                             borderName);
            
            xmlWriter.writeElementCharacters("ClassName", 
                                             className);
            xmlWriter.writeElementCharacters("ColorName",
                                             colorName);
            
            QString structureName = "Invalid";
            if (structure.isLeftCortex()) {
                structureName = "CORTEX_LEFT";
            }
            else if (structure.isRightCortex()) {
                structureName = "CORTEX_RIGHT";
            }
            else if (structure.isCerebellum()) {
                structureName = "CEREBELLUM";
            }
            
            for (int j = 0; j < numLinks; j++) {
                BorderProjectionLink* bpl = bp->getBorderProjectionLink(j);
                int section, nodes[3];
                float areas[3], radius;
                bpl->getData(section, nodes, areas, radius);
                
                //
                // Area and node indices do not match in Caret5 but
                // they do in Caret6
                //  In Caret5:  node-index area-index
                //                   0         1
                //                   1         2
                //                   2         0
                //
                // But caret5 was clockwise order and we want
                // counter clockwise so
                //
                //    node-index  area-index
                //        2            0
                //        1            2
                //        0            1
                //
                //
                int nodesCaret6[3];
                float areasCaret6[3];
                areasCaret6[0] = areas[0];
                areasCaret6[1] = areas[2];
                areasCaret6[2] = areas[1];
                nodesCaret6[0] = nodes[2];
                nodesCaret6[1] = nodes[1];
                nodesCaret6[2] = nodes[0];
                
                xmlWriter.writeStartElement("SurfaceProjectedItem");
                xmlWriter.writeElementCharacters("Structure", structureName);
                xmlWriter.writeStartElement("ProjectionBarycentric");
                xmlWriter.writeElementCharacters("TriangleNodes", nodesCaret6, 3);
                xmlWriter.writeElementCharacters("TriangleAreas", areasCaret6, 3);
                xmlWriter.writeEndElement();
                xmlWriter.writeEndElement();
            }
            
            xmlWriter.writeEndElement();
        }
    }
    
    xmlWriter.writeEndElement();
    
    xmlWriter.writeEndDocument();
    
    file.close();
    
    return name;
}

