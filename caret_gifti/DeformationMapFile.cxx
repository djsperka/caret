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

#define DEFORMATION_MAP_FILE_DEFINE
#include "DeformationMapFile.h"
#undef  DEFORMATION_MAP_FILE_DEFINE

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

#include "FileUtilities.h"
#include "SpecFile.h"

/**
 * Constructor
 */
DeformMapNodeData::DeformMapNodeData()
{
   tileNodes[0] = -1;
   tileNodes[1] = -1;
   tileNodes[2] = -1;
   tileBarycentric[0] = 0.0;
   tileBarycentric[1] = 0.0;
   tileBarycentric[2] = 0.0;
}

/**
 * Constructor
 */
DeformationMapFile::DeformationMapFile()
   : AbstractFile("Deformation Map File", 
                  SpecFile::getDeformationMapFileExtension(),
                  true, 
                  FILE_FORMAT_ASCII, 
                  FILE_IO_READ_AND_WRITE, 
                  FILE_IO_READ_AND_WRITE, 
                  FILE_IO_NONE,
                  FILE_IO_NONE)
{
   clear();
}

/**
 * Destructor
 */
DeformationMapFile::~DeformationMapFile()
{
   clear();
}

/**
 * compare a file for unit testing (returns true if "within tolerance").
 */
bool 
DeformationMapFile::compareFileForUnitTesting(const AbstractFile* af,
                                              const float tolerance,
                                              QString& messageOut) const
{
   messageOut = ""; 
    
   const DeformationMapFile* dmf = dynamic_cast<const DeformationMapFile*>(af);
   if (dmf == NULL) {
      messageOut += "ERROR: File for comparison is not a Deformation Map File.\n";
      return false;
   }
   
   const int numNodes = getNumberOfNodes();
   if (numNodes != dmf->getNumberOfNodes()) {
      messageOut += "ERROR number of nodes do not match for file comparison.";
      return false;
   }
   
   for (int i = 0; i < numNodes; i++) {
      int tilesMe[3], tilesOther[3];
      float areasMe[3], areasOther[3];
      getDeformDataForNode(i, tilesMe, areasMe);
      dmf->getDeformDataForNode(i, tilesOther, areasOther);
      
      for (int i = 0; i < 3; i++) {
         if (tilesMe[i] != tilesOther[i]) {
            messageOut += "ERROR: There are tiles that do not match between the files.\n";
            return false;
         }
         const float diff = std::fabs((float)(areasMe[i] != areasOther[i]));
         if (diff > tolerance) {
            messageOut +="ERROR: tile areas are not within tolerance.";
            return false;
         }
      }
   }
   
   return true;
}

/**
 * clear the contents of the file
 */
void
DeformationMapFile::clear()
{
   clearAbstractFile();
   
   flatOrSphereSelection       = DEFORMATION_TYPE_SPHERE;
   borderResampleType          = BORDER_RESAMPLING_VALUE;
   borderResampleValue         = 10.0;   //15.0;
   sphericalNumberOfStages     = 1;
   for (int i = 0; i < MAX_SPHERICAL_STAGES; i++) {
      sphericalNumberOfCycles[i]     = 3; //1;
      sphereResolution[i]            = 4610;   //18434;
       for (int j = 0; j < MAX_SPHERICAL_CYCLES; j++) {
          smoothingStrength[i][j]           = 1.0;
          smoothingCycles[i][j]             = 100;
          smoothingIterations[i][j]         = 20;
          smoothingNeighborIterations[i][j] = 10;
          smoothingFinalIterations[i][j]    = 5;

          morphingCycles[i][j]              = 1;
          morphingLinearForce[i][j]         = 0.3;
          morphingAngularForce[i][j]        = 0.4;
          morphingStepSize[i][j]            = 0.5;
          morphingLandmarkStepSize[i][j]    = 0.5;
          morphingIterations[i][j]          = 80;
          morphingSmoothIterations[i][j]    = 5;

          landmarkVectorSmoothingIteratons[i][j] = 10;
          landmarkVectorDisplacementFactor[i][j] = 1.0;
          landmarkVectorEndpointFactor[i] = 1.0;
       }

       smoothingStrength[i][0]           = 1.0;
       smoothingCycles[i][0]             = 100;
       smoothingIterations[i][0]         = 20;
       smoothingNeighborIterations[i][0] = 10;
       smoothingFinalIterations[i][0]    = 30;
       morphingCycles[i][0]              = 1;
       morphingLinearForce[i][0]         = 0.3;
       morphingAngularForce[i][0]        = 0.6;
       morphingStepSize[i][0]            = 0.5;
       morphingLandmarkStepSize[i][0]    = 0.5;
       morphingIterations[i][0]          = 300;
       morphingSmoothIterations[i][0]    = 20;

       smoothingStrength[i][1]           = 1.0;
       smoothingCycles[i][1]             = 100;
       smoothingIterations[i][1]         = 20;
       smoothingNeighborIterations[i][1] = 10;
       smoothingFinalIterations[i][1]    = 5;
       morphingCycles[i][1]              = 1;
       morphingLinearForce[i][1]         = 0.3;
       morphingAngularForce[i][1]        = 0.6;
       morphingStepSize[i][1]            = 0.5;
       morphingLandmarkStepSize[i][1]    = 0.5;
       morphingIterations[i][1]          = 300;
       morphingSmoothIterations[i][1]    = 5;

       smoothingStrength[i][2]           = 1.0;
       smoothingCycles[i][2]             = 50;
       smoothingIterations[i][2]         = 20;
       smoothingNeighborIterations[i][2] = 10;
       smoothingFinalIterations[i][2]    = 1;
       morphingCycles[i][2]              = 1;
       morphingLinearForce[i][2]         = 0.3;
       morphingAngularForce[i][2]        = 0.6;
       morphingStepSize[i][2]            = 0.5;
       morphingLandmarkStepSize[i][2]    = 0.5;
       morphingIterations[i][2]          = 300;
       morphingSmoothIterations[i][2]    = 2;
   }

   fiducialSphereRatio = 0.5;
   fiducialSphereRatioEnabled = true;
   
   flatSubSamplingTiles = 900;
   flatBeta = 0.00001;
   flatVarMult = 1.0;
   flatNumIters = 20;
   
   fileVersion = DeformationMapFile::DEFAULT_DEFORMATION_MAP_VERSION;
   
   deformData.clear();
 
   const QString noValue("");
   setSourceDirectory(noValue);
   setSourceSpecFileName(noValue);
   setSourceBorderFileName(noValue, BORDER_FILE_UNKNOWN);
   setSourceClosedTopoFileName(noValue);
   setSourceCutTopoFileName(noValue);
   setSourceFiducialCoordFileName(noValue);
   setSourceSphericalCoordFileName(noValue);
   setSourceFlatCoordFileName(noValue);
   setSourceDeformedSphericalCoordFileName(noValue);
   setSourceDeformedFlatCoordFileName(noValue);
   setSourceResampledFlatCoordFileName(noValue);
   setSourceResampledDeformedFlatCoordFileName(noValue);
   setSourceResampledCutTopoFileName(noValue);
   
   setTargetDirectory(noValue);
   setTargetSpecFileName(noValue);
   for (int i = 0; i < MAX_SPHERICAL_STAGES; i++) {
      setTargetBorderFileName(i, noValue, BORDER_FILE_UNKNOWN);
   }
   setTargetClosedTopoFileName(noValue);
   setTargetCutTopoFileName(noValue);
   setTargetFiducialCoordFileName(noValue);
   setTargetSphericalCoordFileName(noValue);
   setTargetFlatCoordFileName(noValue);
   
   setOutputSpecFileName(noValue);
   
   inverseDeformationFlag = false;
   deleteIntermediateFilesFlag = true;
   deformBothWaysFlag = true;
   metricDeformationType = METRIC_DEFORM_NEAREST_NODE;
   deformedFileNamePrefix = "deformed_";
   deformedColumnNamePrefix = "deformed_";
   smoothDeformedSurfacesFlag = true;

   pauseForCrossoversConfirmation = false;
}

/**
 * Get the deformation for a node.
 */
void 
DeformationMapFile::getDeformDataForNode(const int nodeNumber, 
                                        int tileNodesOut[3],
                                        float tileBarycentricOut[3]) const
{
   for (int i = 0; i < 3; i++) {
      tileNodesOut[i]       = deformData[nodeNumber].tileNodes[i];
      tileBarycentricOut[i] = deformData[nodeNumber].tileBarycentric[i];
   }
}

/**
 * Set the deformation for a node.
 */
void 
DeformationMapFile::setDeformDataForNode(const int nodeNumber, 
                                        const int tileNodesIn[3],
                                        const float tileBarycentricIn[3])
{
   for (int i = 0; i < 3; i++) {
      deformData[nodeNumber].tileNodes[i]       = tileNodesIn[i];
      deformData[nodeNumber].tileBarycentric[i] = tileBarycentricIn[i];
   }
   setModified();
}

/**
 * Set the number of nodes for the file.
 */
void
DeformationMapFile::setNumberOfNodes(const int numNodesIn)
{
   deformData.resize(numNodesIn);
   setModified();
}

/**
 * Make a filname relative to that "path".
 */
void
DeformationMapFile::makeFileRelative(const QString& path,
                                     QString& name)
{
   if (name.isEmpty() == false) {
      //printf("... %s is now: ", name);
      if (name[0] != '/') {
         return;
      }
      const QString filename(FileUtilities::basename(name));
      QString filepath(FileUtilities::dirname(name));
      QString result;
      FileUtilities::relativePath(filepath, path, result);
      if (result.isEmpty() == false) {
          QString s(result);
          s.append("/");
          s.append(filename);
          name = s;
      }
      else {
         name = filename;
      }
      //printf("%s\n", name);
   }
}

/**
 * Make the source files relative to a path.
 */
void
DeformationMapFile::makeSourceFilesRelativeToPath(const QString& path)
{
   makeFileRelative(path, sourceSpecFileName);
   makeFileRelative(path, sourceBorderFileName);
   makeFileRelative(path, sourceClosedTopoFileName);
   makeFileRelative(path, sourceCutTopoFileName);
   makeFileRelative(path, sourceFiducialCoordFileName);
   makeFileRelative(path, sourceSphericalCoordFileName);
   makeFileRelative(path, sourceDeformedSphericalCoordFileName);
   makeFileRelative(path, sourceDeformedFlatCoordFileName);
   makeFileRelative(path, sourceFlatCoordFileName);
   makeFileRelative(path, sourceResampledFlatCoordFileName);
   makeFileRelative(path, sourceResampledDeformedFlatCoordFileName);
   makeFileRelative(path, sourceResampledCutTopoFileName);
   setModified();
}
 
/**
 * Make the target files relative to a path.
 */
void
DeformationMapFile::makeTargetFilesRelativeToPath(const QString& path)
{
   makeFileRelative(path, targetSpecFileName);
   for (int i = 0; i < MAX_SPHERICAL_STAGES; i++) {
      makeFileRelative(path, targetBorderFileName[i]);
   }
   makeFileRelative(path, targetClosedTopoFileName);
   makeFileRelative(path, targetCutTopoFileName);
   makeFileRelative(path, targetFiducialCoordFileName);
   makeFileRelative(path, targetSphericalCoordFileName);
   makeFileRelative(path, targetFlatCoordFileName);
   makeFileRelative(path, outputSpecFileName);
   setModified();
}
 
/**
 * make file name's relative paths.
 */
void 
DeformationMapFile::makePathsRelative(const QString& path)
{
   makeSourceFilesRelativeToPath(path);
   makeTargetFilesRelativeToPath(path);
}

/**
 * Swap the source files with the target files.
 */
void
DeformationMapFile::swapSourceAndTargetFiles()
{
   std::swap(sourceDirectoryName,          targetDirectoryName);
   std::swap(sourceSpecFileName,           targetSpecFileName);
   std::swap(sourceBorderFileName,         targetBorderFileName[0]);
   std::swap(sourceBorderFileType,         targetBorderFileType[0]);
   std::swap(sourceClosedTopoFileName,     targetClosedTopoFileName);
   std::swap(sourceCutTopoFileName,        targetCutTopoFileName);
   std::swap(sourceFiducialCoordFileName,  targetFiducialCoordFileName);
   std::swap(sourceSphericalCoordFileName, targetSphericalCoordFileName);
   std::swap(sourceFlatCoordFileName,      targetFlatCoordFileName);
}

/**
 * Read the deformation map file's data.
 */
void
DeformationMapFile::readFileData(QFile& file, QTextStream& stream, QDataStream& binStream,
                                  QDomElement& /* rootElement */) 
                                                    throw (FileException)
{
   //
   // first section of deformation file contains a list of files that
   // were used to perform deformation.
   //
   bool readingFiles = true;
   QString lastLineRead;
   while(readingFiles) {
      //
      // Read a line from the file
      //
      QString tag, tagValue;
      readTagLine(stream, lastLineRead, tag, tagValue);
      
      if (tag == startOfDataTag) {
         readingFiles = false;
      }
      else if (tag == deformMapFileVersionTag) {
         fileVersion = QString(tagValue).toInt();
         if (fileVersion > DEFAULT_DEFORMATION_MAP_VERSION) {
            throw FileException(
               "The deformation map file is from a newer version of Caret.  "
               "You need to update your Caret software.");
         }
      }
      else if (tag == deformedFileNamePrefixTag) {
         setDeformedFileNamePrefix(tagValue);
      }
      else if (tag == deformedColumnNamePrefixTag) {
         setDeformedColumnNamePrefix(tagValue);
      }
      else if (tag == sourceDirectoryTag) {
         setSourceDirectory(tagValue);
      }
      else if (tag == sourceSpecTag) {
         setSourceSpecFileName(tagValue);
      }
      else if (tag == sourceBorderTag) {
         setSourceBorderFileName(tagValue, BORDER_FILE_UNKNOWN);
      }
      else if (tag == sourceClosedTopoTag) {
         setSourceClosedTopoFileName(tagValue);
      }
      else if (tag == sourceCutTopoTag) {
         setSourceCutTopoFileName(tagValue);
      }
      else if (tag == sourceFiducialCoordTag) {
         setSourceFiducialCoordFileName(tagValue);
      }
      else if (tag == sourceSphericalCoordTag) {
         setSourceSphericalCoordFileName(tagValue);
      }
      else if (tag == sourceDeformedSphericalCoordTag) {
         setSourceDeformedSphericalCoordFileName(tagValue);
      }
      else if (tag == sourceDeformedFlatCoordTag) {
         setSourceDeformedFlatCoordFileName(tagValue);
      }
      else if (tag == sourceFlatCoordTag) {
         setSourceFlatCoordFileName(tagValue);
      }
      else if (tag == sourceResampledFlatCoordTag) {
         setSourceResampledFlatCoordFileName(tagValue);
      }
      else if (tag == sourceResampledDeformedFlatCoordTag) {
         setSourceResampledDeformedFlatCoordFileName(tagValue);
      }
      else if (tag == sourceResampledCutTopoTag) {
         setSourceResampledCutTopoFileName(tagValue);
      }
      else if (tag == targetDirectoryTag) {
         setTargetDirectory(tagValue);
      }
      else if (tag == targetSpecTag) {
         setTargetSpecFileName(tagValue);
      }
      else if (tag == targetBorderTag) {
         if (fileVersion >= 3) {
            int index = 0;
            QString name;
            QTextStream stream(&tagValue, QIODevice::ReadOnly);
            stream >> index >> name;
            setTargetBorderFileName(index, name, BORDER_FILE_UNKNOWN);
         }
         else {
            setTargetBorderFileName(0, tagValue, BORDER_FILE_UNKNOWN);
         }
      }
      else if (tag == targetClosedTopoTag) {
         setTargetClosedTopoFileName(tagValue);
      }
      else if (tag == targetCutTopoTag) {
         setTargetCutTopoFileName(tagValue);
      }
      else if (tag == targetFiducialCoordTag) {
         setTargetFiducialCoordFileName(tagValue);
      }
      else if (tag == targetSphericalCoordTag) {
         setTargetSphericalCoordFileName(tagValue);
      }
      else if (tag == targetFlatCoordTag) {
         setTargetFlatCoordFileName(tagValue);
      }
      else if (tag == outputSpecFileTag) {
         setOutputSpecFileName(tagValue);
      }
      else if (tag == sphereResolutionTag) {
         if (fileVersion >= 3) {
            int stageNumber = 0;
            int resolution = 4610;
            QTextStream textStream(&tagValue, QIODevice::ReadOnly);
            textStream >> stageNumber >> resolution;
            sphereResolution[stageNumber] = resolution;
         }
         else {
            sphereResolution[0] = tagValue.toInt();
         }
      }
      else if (tag == flatOrSphereSelectionTag) {
         if (tagValue == DeformationFlatValue) {
            flatOrSphereSelection = DEFORMATION_TYPE_FLAT;
         }
         else if (tagValue == DeformationSphereValue) {
            flatOrSphereSelection = DEFORMATION_TYPE_SPHERE;
         }
         else if (tagValue == DeformationSphereMultiStageVectorValue) {
            flatOrSphereSelection = DEFORMATION_TYPE_SPHERE_MULTI_STAGE_VECTOR;
         }
         else if (tagValue == DeformationSphereSingleStageVectorValue) {
            flatOrSphereSelection = DEFORMATION_TYPE_SPHERE_SINGLE_STAGE_VECTOR;
         }
         else {
            QString msg("ERROR invalid deformation flat/sphere value ");
            msg.append(tagValue);
            throw FileException(filename, msg);
         }
      }
      else if (tag == borderResampleTag) {
         QTextStream textStream(&tagValue, QIODevice::ReadOnly);
         int resampleTypeInt;
         textStream >> resampleTypeInt
              >> borderResampleValue;
         borderResampleType = static_cast<BORDER_RESAMPLING_TYPE>(resampleTypeInt);
      }
      else if (tag == sphericalNumberOfStagesTag) {
         sphericalNumberOfStages = tagValue.toInt();
      }
      else if (tag == sphericalNumberOfCyclesTag) {
         if (fileVersion >= 3) {
            int stageNumber = 0;
            int cycles = 3;
            QTextStream textStream(&tagValue, QIODevice::ReadOnly);
            textStream >> stageNumber >> cycles;
            sphericalNumberOfCycles[stageNumber] = cycles;
         }
         else {
            sphericalNumberOfCycles[0] = tagValue.toInt();
         }
      }
      else if (tag == smoothingParamtersTag) {
         if (fileVersion >= 3) {
            int stageNumber;
            int cycleNumber;
            float strength;
            int cycles;
            int iterations;
            int neighborIterations;
            int finalIterations;

            QTextStream textStream(&tagValue, QIODevice::ReadOnly);
            textStream >> stageNumber
                 >> cycleNumber
                 >> strength
                 >> cycles
                 >> iterations
                 >> neighborIterations
                 >> finalIterations;

            smoothingStrength[stageNumber][cycleNumber]           = strength;
            smoothingCycles[stageNumber][cycleNumber]             = cycles;
            smoothingIterations[stageNumber][cycleNumber]         = iterations;
            smoothingNeighborIterations[stageNumber][cycleNumber] = neighborIterations;
            smoothingFinalIterations[stageNumber][cycleNumber]    = finalIterations;
         }
         else if (fileVersion > 0) {
            int cycleNumber;
            float strength;
            int cycles;
            int iterations;
            int neighborIterations;
            int finalIterations;

            QTextStream textStream(&tagValue, QIODevice::ReadOnly);
            textStream >> cycleNumber
                 >> strength
                 >> cycles
                 >> iterations
                 >> neighborIterations
                 >> finalIterations;

            smoothingStrength[0][cycleNumber]           = strength;
            smoothingCycles[0][cycleNumber]             = cycles;
            smoothingIterations[0][cycleNumber]         = iterations;
            smoothingNeighborIterations[0][cycleNumber] = neighborIterations;
            smoothingFinalIterations[0][cycleNumber]    = finalIterations;
         }
         else {
            QTextStream textStream(&tagValue, QIODevice::ReadOnly);
            textStream >> smoothingStrength[0][0]
                 >> smoothingCycles[0][0]
                 >> smoothingIterations[0][0]
                 >> smoothingNeighborIterations[0][0]
                 >> smoothingFinalIterations[0][0];
         }
      }
      else if (tag == landmarkVectorParametersTag) {
         if (fileVersion >= 3) {
             QTextStream textStream(&tagValue, QIODevice::ReadOnly);
             int stageNumber = 0;
             int cycleNumber = 0;
             textStream >> stageNumber;
             textStream >> cycleNumber;
             textStream >> landmarkVectorSmoothingIteratons[stageNumber][cycleNumber];
             textStream >> landmarkVectorDisplacementFactor[stageNumber][cycleNumber];
         }
         else {
             QTextStream textStream(&tagValue, QIODevice::ReadOnly);
             int cycleNumber = 0;
             textStream >> cycleNumber;
             if (textStream.atEnd() == false) {
                textStream >> landmarkVectorSmoothingIteratons[0][cycleNumber];
                textStream >> landmarkVectorDisplacementFactor[0][cycleNumber];
             }
             else {
                for (int mm = 0; mm < MAX_SPHERICAL_CYCLES; mm++) {
                   landmarkVectorSmoothingIteratons[0][mm] = cycleNumber;
                }
             }
         }
      }
      else if (tag == landmarkVectorStageParametersTag) {
         QTextStream textStream(&tagValue, QIODevice::ReadOnly);
         int stageNumber = 0;
         textStream >> stageNumber;
         textStream >> landmarkVectorEndpointFactor[stageNumber];
      }
      else if (tag == morphingParametersTag) {
         if (fileVersion >= 3) {
            int stageNumber;
            int cycleNumber;
            int cycles;
            float linearForce;
            float angularForce;
            float stepSize;
            float landmarkStepSize;
            int iterations;
            int smoothIterations;

            QTextStream textStream(&tagValue, QIODevice::ReadOnly);
            textStream >> stageNumber
                 >> cycleNumber
                 >> cycles
                 >> linearForce
                 >> angularForce
                 >> stepSize
                 >> landmarkStepSize
                 >> iterations
                 >> smoothIterations;

            morphingCycles[stageNumber][cycleNumber] = cycles;
            morphingLinearForce[stageNumber][cycleNumber] = linearForce;
            morphingAngularForce[stageNumber][cycleNumber] = angularForce;
            morphingStepSize[stageNumber][cycleNumber] = stepSize;
            morphingLandmarkStepSize[stageNumber][cycleNumber] = landmarkStepSize;
            morphingIterations[stageNumber][cycleNumber] = iterations;
            morphingSmoothIterations[stageNumber][cycleNumber] = smoothIterations;
         }
         else if (fileVersion > 0) {
            int cycleNumber;
            int cycles;
            float linearForce;
            float angularForce;
            float stepSize;
            float landmarkStepSize;
            int iterations;
            int smoothIterations;

            QTextStream textStream(&tagValue, QIODevice::ReadOnly);
            textStream >> cycleNumber
                 >> cycles
                 >> linearForce
                 >> angularForce
                 >> stepSize
                 >> landmarkStepSize
                 >> iterations
                 >> smoothIterations;

            morphingCycles[0][cycleNumber] = cycles;
            morphingLinearForce[0][cycleNumber] = linearForce;
            morphingAngularForce[0][cycleNumber] = angularForce;
            morphingStepSize[0][cycleNumber] = stepSize;
            morphingLandmarkStepSize[0][cycleNumber] = landmarkStepSize;
            morphingIterations[0][cycleNumber] = iterations;
            morphingSmoothIterations[0][cycleNumber] = smoothIterations;
         }
         else {
            //
            // Some older files may only have 5 parameters
            //
            morphingCycles[0][0] = 1;
            morphingLandmarkStepSize[0][0] = morphingStepSize[0][0];
            morphingSmoothIterations[0][0] = 0;
            
            QTextStream textStream(&tagValue, QIODevice::ReadOnly);
            textStream >> morphingCycles[0][0]
                 >> morphingLinearForce[0][0]
                 >> morphingAngularForce[0][0]
                 >> morphingStepSize[0][0]
                 >> morphingLandmarkStepSize[0][0]
                 >> morphingIterations[0][0]
                 >> morphingSmoothIterations[0][0];
         }
      }
      else if (tag == inverseDeformationFlagTag) {
         inverseDeformationFlag = (tagValue == "true");
      }
      else if (tag == pauseForCrossoversConfirmationTag) {
         pauseForCrossoversConfirmation = (tagValue == "true");
      }
      else if (tag == flatParametersTag) {
         QTextStream textStream(&tagValue, QIODevice::ReadOnly);
         textStream >> flatSubSamplingTiles
               >> flatBeta
               >> flatVarMult
               >> flatNumIters;
      }
      else if (tag == startOfDataTag) {
            readingFiles = false;
      }
      else if (tag == sphereFiducialSphereRatioTag) {
         QTextStream istr(&tagValue, QIODevice::ReadOnly);
         QString enabled;
         istr >> enabled >> fiducialSphereRatio;
         fiducialSphereRatioEnabled = (enabled == "true");
      }
      else if (tag == smoothDeformedSurfaceTag) {
         smoothDeformedSurfacesFlag = (tagValue == "true");
      }
      else {
         std::cout << "WARNING unrecognzied deformation map file tag: " 
                   << tag.toAscii().constData() << std::endl;
      }
   }
   
   //
   // Should reading data be skipped ?
   //
   if (getReadMetaDataOnlyFlag()) {
      return;
   }

   switch (getFileReadType()) {
      case FILE_FORMAT_ASCII:
         {
            int numNodes;
            stream >> numNodes;
            setNumberOfNodes(numNodes);
            
            for (int i = 0; i < numNodes; i++) {
               int nodeNumber;
               int nodes[3];
               float areas[3];
               stream >> nodeNumber
                      >> nodes[0]
                      >> nodes[1]
                      >> nodes[2]
                      >> areas[0]
                      >> areas[1]
                      >> areas[2];
               setDeformDataForNode(nodeNumber, nodes, areas);
            }
         }
         break;
      case FILE_FORMAT_BINARY:
         {
#ifdef QT4_FILE_POS_BUG
            qint64 offset = findBinaryDataOffsetQT4Bug(file, lastLineRead.toAscii().constData());
            if (offset > 0) {
               offset++;
               file.seek(offset);
            }
#endif // QT4_FILE_POS_BUG
            //
            // Needed for QT 4.2.2
            //
            binStream.device()->seek(stream.pos());
            
            int numNodes;
            binStream >> numNodes;
            setNumberOfNodes(numNodes);
            
            for (int i = 0; i < numNodes; i++) {
               int nodes[3];
               float areas[3];
               binStream >> nodes[0]
                         >> nodes[1]
                         >> nodes[2]
                         >> areas[0]
                         >> areas[1]
                         >> areas[2];
               setDeformDataForNode(i, nodes, areas);
            }
         }
         break;
      case FILE_FORMAT_XML:
         throw FileException(filename, "Reading in XML format not supported.");           
         break;
      case FILE_FORMAT_XML_BASE64:
         throw FileException(filename, "Reading XML Base64 not supported.");
         break;
      case FILE_FORMAT_XML_GZIP_BASE64:
         throw FileException(filename, "Reading XML GZip Base64 not supported.");
         break;
      case FILE_FORMAT_XML_EXTERNAL_BINARY:
         throw FileException(filename, "Reading XML External Binary not supported.");
         break;      
      case FILE_FORMAT_OTHER:
         throw FileException(filename, "Reading in Other format not supported.");
         break;
      case FILE_FORMAT_COMMA_SEPARATED_VALUE_FILE:
         throw FileException(filename, "Reading Comma Separated Value File Format not supported.");
         break;
   }
}

/**
 * Write a tag/value pair and make it relative to a directory
 */
void
DeformationMapFile::writeFileTagRelative(QTextStream& stream,
                                         const QString& directory,
                                         const QString& tag,
                                         const QString& valueIn)
{
   QString value = valueIn;
   
   //
   // Only do so for version 2 and later files
   //
   if (fileVersion >= 2) {
      if (directory.isEmpty() == false) {
         makeFileRelative(directory, value);
      }
   }
   stream << tag << " "
          << value << "\n";
}

/**
 * Write a tag/value pair and make it relative to a directory
 */
void
DeformationMapFile::writeFileTagRelative(QTextStream& stream,
                                         const QString& directory,
                                         const QString& tag,
                                         const QString valueIn[],
                                         const int index)
{
   QString value = valueIn[index];

   //
   // Only do so for version 2 and later files
   //
   if (fileVersion >= 2) {
      if (directory.isEmpty() == false) {
         makeFileRelative(directory, value);
      }
   }
   stream << tag << " "
          << index << " "
          << value << "\n";
}

/**
 * Write the file's data.
 */
void
DeformationMapFile::writeFileData(QTextStream& stream, QDataStream& binStream,
                                 QDomDocument& /* xmlDoc */,
                                  QDomElement& /* rootElement */) throw (FileException)
{
   fileVersion = DEFAULT_DEFORMATION_MAP_VERSION;
   stream << deformMapFileVersionTag << " " << fileVersion << "\n";

   switch(flatOrSphereSelection) {
      case DEFORMATION_TYPE_FLAT:
         stream << flatOrSphereSelectionTag << " "
                << DeformationFlatValue << "\n";
         break;
      case DEFORMATION_TYPE_SPHERE:
         stream << flatOrSphereSelectionTag << " "
                << DeformationSphereValue << "\n";
         break;
      case DEFORMATION_TYPE_SPHERE_MULTI_STAGE_VECTOR:
         stream << flatOrSphereSelectionTag << " "
                << DeformationSphereMultiStageVectorValue << "\n";
         break;
      case DEFORMATION_TYPE_SPHERE_SINGLE_STAGE_VECTOR:
         stream << flatOrSphereSelectionTag << " "
                << DeformationSphereSingleStageVectorValue << "\n";
         break;
   }
   
   stream << deformedFileNamePrefixTag << " " << deformedFileNamePrefix << "\n";
   stream << deformedColumnNamePrefixTag << " " << deformedColumnNamePrefix << "\n";
   
   stream << sourceDirectoryTag << " "
          << sourceDirectoryName << "\n";

   writeFileTagRelative(stream, sourceDirectoryName, 
                        sourceSpecTag, sourceSpecFileName);
   writeFileTagRelative(stream, sourceDirectoryName, 
                        sourceBorderTag, sourceBorderFileName);
   writeFileTagRelative(stream, sourceDirectoryName, 
                        sourceClosedTopoTag, sourceClosedTopoFileName);
   writeFileTagRelative(stream, sourceDirectoryName, 
                        sourceCutTopoTag, sourceCutTopoFileName);
   writeFileTagRelative(stream, sourceDirectoryName, 
                        sourceFiducialCoordTag, sourceFiducialCoordFileName);
   writeFileTagRelative(stream, sourceDirectoryName, 
                        sourceSphericalCoordTag, sourceSphericalCoordFileName);
   writeFileTagRelative(stream, sourceDirectoryName, 
                        sourceDeformedSphericalCoordTag, sourceDeformedSphericalCoordFileName);
   writeFileTagRelative(stream, sourceDirectoryName, 
                        sourceDeformedFlatCoordTag, sourceDeformedFlatCoordFileName);
   writeFileTagRelative(stream, sourceDirectoryName, 
                        sourceFlatCoordTag, sourceFlatCoordFileName);
   writeFileTagRelative(stream, sourceDirectoryName, 
                        sourceResampledFlatCoordTag, sourceResampledFlatCoordFileName);
   writeFileTagRelative(stream, sourceDirectoryName, 
                        sourceResampledDeformedFlatCoordTag, sourceResampledDeformedFlatCoordFileName);
   writeFileTagRelative(stream, sourceDirectoryName, 
                        sourceResampledCutTopoTag, sourceResampledCutTopoFileName);
   
   for (int i = 0; i < sphericalNumberOfStages; i++) {
      stream << sphereResolutionTag << " " << i << " " << sphereResolution[i] << "\n";
   }
   stream << borderResampleTag << " " 
          << static_cast<int>(borderResampleType) << " " 
          << borderResampleValue << "\n";
   stream << sphericalNumberOfStagesTag << " "
          << sphericalNumberOfStages << "\n";
   for (int i = 0; i < sphericalNumberOfStages; i++) {
      stream << sphericalNumberOfCyclesTag << " " << i << " "
             << sphericalNumberOfCycles[i] << "\n";
   }
   for (int i = 0; i < sphericalNumberOfStages; i++) {
       for (int j = 0; j < sphericalNumberOfCycles[i]; j++) {
          stream << smoothingParamtersTag << " " << i << " " << j << " "
                 << smoothingStrength[i][j] << " "
                 << smoothingCycles[i][j] << " "
                 << smoothingIterations[i][j] << " "
                 << smoothingNeighborIterations[i][j] << " "
                 << smoothingFinalIterations[i][j] << "\n";
          stream << morphingParametersTag << " " << i << " " << j << " "
                 << morphingCycles[i][j] << " "
                 << morphingLinearForce[i][j] << " "
                 << morphingAngularForce[i][j] << " "
                 << morphingStepSize[i][j] << " "
                 << morphingLandmarkStepSize[i][j] << " "
                 << morphingIterations[i][j] << " "
                 << morphingSmoothIterations[i][j] << "\n";
          stream << landmarkVectorParametersTag << " " << i << " " << j << " "
                 << landmarkVectorSmoothingIteratons[i][j] << " "
                 << landmarkVectorDisplacementFactor[i][j] << "\n";
       }
       stream << landmarkVectorStageParametersTag << " " << i << " "
              << landmarkVectorEndpointFactor[i] << "\n";
   }

   stream << flatParametersTag << " " 
          << flatSubSamplingTiles << " " 
          << flatBeta << " " 
          << flatVarMult << " " 
          << flatNumIters << "\n";
   
   stream << targetDirectoryTag << " "
          << targetDirectoryName << "\n";
   
   stream << smoothDeformedSurfaceTag << (smoothDeformedSurfacesFlag ? " true" : " false") << "\n";
   
   writeFileTagRelative(stream, targetDirectoryName, 
                        targetSpecTag, targetSpecFileName);
   for (int i = 0; i < sphericalNumberOfStages; i++) {
      writeFileTagRelative(stream, targetDirectoryName,
                           targetBorderTag, targetBorderFileName, i);
   }
   writeFileTagRelative(stream, targetDirectoryName, 
                        targetClosedTopoTag, targetClosedTopoFileName);
   writeFileTagRelative(stream, targetDirectoryName, 
                        targetCutTopoTag, targetCutTopoFileName);
   writeFileTagRelative(stream, targetDirectoryName, 
                        targetSphericalCoordTag, targetSphericalCoordFileName);
   writeFileTagRelative(stream, targetDirectoryName, 
                        targetFiducialCoordTag, targetFiducialCoordFileName);
   writeFileTagRelative(stream, targetDirectoryName, 
                        targetFlatCoordTag, targetFlatCoordFileName);
   writeFileTagRelative(stream, targetDirectoryName, 
                        outputSpecFileTag, outputSpecFileName);
      
   QString ratioString("false");
   if (fiducialSphereRatioEnabled) {
      ratioString = "true";
   }
   stream << sphereFiducialSphereRatioTag << " "
          << ratioString << " "
          << fiducialSphereRatio << "\n";

   stream << pauseForCrossoversConfirmationTag
          << (pauseForCrossoversConfirmation ? " true" : " false")
          << "\n";

   if (inverseDeformationFlag) {
      stream << inverseDeformationFlagTag << " true\n";
   }
   else {
      stream << inverseDeformationFlagTag << " false\n";
   }
   stream << startOfDataTag << "\n";
   
   const int numNodes = getNumberOfNodes();

   switch (getFileWriteType()) {
      case FILE_FORMAT_ASCII:
         stream << numNodes << "\n";
         
         for (int i = 0; i < numNodes; i++) {
            int nodes[3];
            float areas[3];
            getDeformDataForNode(i, nodes, areas);
            stream << i << " "  
                   << nodes[0] << " "  
                   << nodes[1] << " "  
                   << nodes[2] << " " 
                   << areas[0] << " " 
                   << areas[1] << " " 
                   << areas[2] << "\n";
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
         binStream << static_cast<qint32>(numNodes);
         
         for (int i = 0; i < numNodes; i++) {
            int nodes[3];
            float areas[3];
            getDeformDataForNode(i, nodes, areas);
            binStream << static_cast<qint32>(nodes[0])  
                      << static_cast<qint32>(nodes[1])  
                      << static_cast<qint32>(nodes[2]) 
                      << areas[0]
                      << areas[1] 
                      << areas[2];
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
         throw FileException(filename, "Writing XML External Binary not supported.");
         break;      
      case FILE_FORMAT_OTHER:
         throw FileException(filename, "Writing in Other format not supported.");
         break;
      case FILE_FORMAT_COMMA_SEPARATED_VALUE_FILE:
         throw FileException(filename, "Writing Comma Separated Value File Format not supported.");
         break;
   }
}
