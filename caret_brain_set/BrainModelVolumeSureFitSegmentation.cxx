
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
#include <sstream>

#include <QDateTime>
#include <QDir>

#include "AreaColorFile.h"
#include "BorderColorFile.h"
#include "BorderProjectionFile.h"
#include "BrainSet.h"
#include "BrainModelOpenGL.h"
#include "BrainModelSurface.h"
#include "BrainModelSurfaceBorderLandmarkIdentification.h"
#include "BrainModelSurfaceCurvature.h"
#include "BrainModelSurfaceOverlay.h"
#include "BrainModelSurfaceSulcalDepthWithNormals.h"
#include "BrainModelSurfaceTopologyCorrector.h"
#include "BrainModelVolumeGradient.h"
#include "BrainModelVolumeNearToPlane.h"
#include "BrainModelVolumeSureFitErrorCorrection.h"
#include "BrainModelVolumeSureFitSegmentation.h"
#include "BrainModelVolumeToSurfaceConverter.h"
#include "BrainModelVolumeTopologyGraphCorrector.h"
#include "DebugControl.h"
#include "DisplaySettingsBorders.h"
#include "DisplaySettingsSurfaceShape.h"
#include "PaintFile.h"
#include "ParamsFile.h"
#include "SceneFile.h"
#include "StatisticHistogram.h"
#include "SurfaceShapeFile.h"
#include "TopologyFile.h"
#include "VocabularyFile.h"

/**
 * Constructor.  Call execute() after this constructor.
 */
BrainModelVolumeSureFitSegmentation::BrainModelVolumeSureFitSegmentation(BrainSet* bs,
                                   const VolumeFile* anatomyVolumeIn,
                                   const VolumeFile* segmentationVolumeIn,
                                   const VolumeFile::FILE_READ_WRITE_TYPE typeOfVolumeFilesToWriteIn,
                                   const int acIJKIn[3],
                                   const int partialHemispherePaddingIn[6],
                                   const float wmPeakIn,
                                   const float gmPeakIn,
                                   const float midThreshOverrideIn,
                                   const Structure::STRUCTURE_TYPE structureIn,
                                   const bool disconnectEyeFlagIn,
                                   const bool disconnectHindBrainFlagIn,
                                   const bool disconnectHindBrainHiThreshFlagIn,
                                   const bool cutCorpusCallosumFlagIn,
                                   const bool segmentAnatomyFlagIn,
                                   const bool fillVentriclesFlagIn,
                                   const ERROR_CORRECTION_METHOD errorCorrectionMethodIn,
                                   const bool generateRawAndFidualSurfacesFlagIn,
                                   const bool maximumPolygonsFlagIn,
                                   const bool generateTopologicallyCorrectFiducialSurfaceFlagIn,
                                   const bool generateInflatedSurfaceFlagIn,
                                   const bool generateVeryInflatedSurfaceFlagIn,
                                   const bool generateEllipsoidSurfaceFlagIn,
                                   const bool generateSphericalSurfaceFlagIn,
                                   const bool generateCompressedMedialWallSurfaceFlagIn,
                                   const bool generateHullSurfaceFlagIn,
                                   const bool generateDepthCurvatureGeographyFlagIn,
                                   const bool identifyRegisterFlattenLandmarksFlagIn,
                                   const bool autoSaveFilesFlagIn)
   : BrainModelAlgorithm(bs)
{
   typeOfVolumeFilesToWrite = typeOfVolumeFilesToWriteIn;
   disconnectEyeFlag = disconnectEyeFlagIn;
   disconnectHindBrainFlag = disconnectHindBrainFlagIn;
   disconnectHindBrainHiThreshFlag = disconnectHindBrainHiThreshFlagIn;
   cutCorpusCallosumFlag = cutCorpusCallosumFlagIn;
   if (segmentAnatomyFlagIn) {
      generateInnerBoundaryFlag = true;
      generateOuterBoundaryFlag = true;
      generateSegmentationFlag  = true;
   }
   else {
      generateInnerBoundaryFlag = false;
      generateOuterBoundaryFlag = false;
      generateSegmentationFlag  = false;
   }
   fillVentriclesFlag = fillVentriclesFlagIn;
   errorCorrectionMethod = errorCorrectionMethodIn;
   generateRawAndFidualSurfacesFlag = generateRawAndFidualSurfacesFlagIn;
   generateTopologicallyCorrectFiducialSurfaceFlag = generateTopologicallyCorrectFiducialSurfaceFlagIn;
   generateInflatedSurfaceFlag = generateInflatedSurfaceFlagIn;
   generateVeryInflatedSurfaceFlag = generateVeryInflatedSurfaceFlagIn;
   generateEllipsoidSurfaceFlag = generateEllipsoidSurfaceFlagIn;
   generateSphericalSurfaceFlag = generateSphericalSurfaceFlagIn;
   generateCompressedMedialWallSurfaceFlag = generateCompressedMedialWallSurfaceFlagIn;
   generateHullSurfaceFlag = generateHullSurfaceFlagIn;
   generateDepthCurvatureGeographyFlag = generateDepthCurvatureGeographyFlagIn;
   identifyRegisterFlattenLandmarksFlag = identifyRegisterFlattenLandmarksFlagIn;
   autoSaveFilesFlag = autoSaveFilesFlagIn;
   maximumPolygonsFlag = maximumPolygonsFlagIn;
   
   extractMaskFlag = true;
   
   anatomyVolume = NULL;
   if (anatomyVolumeIn != NULL) {
      anatomyVolume = new VolumeFile(*anatomyVolumeIn);
      anatomyVolume->setFileWriteType(typeOfVolumeFilesToWrite);
   }
   segmentationVolume = NULL;
   if (segmentationVolumeIn != NULL) {
      segmentationVolume = new VolumeFile(*segmentationVolumeIn);
      segmentationVolume->setFileWriteType(typeOfVolumeFilesToWrite);
   }
   segmentationVentriclesFilledVolume = NULL;
   whiteMatterThreshNoEyeVolume = NULL;
   whiteMatterThreshNoEyeFloodVolume = NULL;
   cerebralWmNoBstemFill = NULL;
   innerMask1Volume = NULL;
   gradIntensityVolume = NULL;
   eyeFatSculptVolume = NULL;
   gmILevelVolume = NULL;
   outerMaskVolume = NULL;
   wmThreshFloodVolume = NULL;
   inTotalVolume = NULL;
   inTotalThinWMVolume = NULL;
   outTotalVolume = NULL;
   thinWMOrNearVentricleHCMask = NULL;
   ventGradLevelBlurVolume = NULL;
   inTotalBlur1Volume = NULL;
   outTotalBlur1Volume = NULL;
   cerebralWMErodeVolume = NULL;
   hindbrainFloodVolume = NULL;
      
   fiducialSurface = NULL;
   
   gradPiaLevelVec = NULL;
   gradThinWMlevelVecFile = NULL;
   gradIntensityVecFile = NULL;
   gradInTotalThinWMVecFile = NULL;
   gradGWlevelVecFile = NULL;
   outGradPialLevelGMGradOutITMagVecFile = NULL;
   
   acIJK[0] = acIJKIn[0];
   acIJK[1] = acIJKIn[1];
   acIJK[2] = acIJKIn[2];
   wmPeak   = wmPeakIn;
   cgmPeak  = gmPeakIn;
   wmThresh = (wmPeak + cgmPeak) * 0.5;
   if (midThreshOverrideIn > 0.0) {
      wmThresh = midThreshOverrideIn;
   }
   
   structure = structureIn;
   
   for (int i = 0; i < 6; i++) {
      partialHemispherePadding[i] = partialHemispherePaddingIn[i];
   }
   
   volumeMask = NULL;
   whiteMatterMaximum = 0.0;
}
                             
/**
 * Constructor used for identifying sulci only.  
 * Call executeIdentifySulci() after this constructor.
 */
BrainModelVolumeSureFitSegmentation::BrainModelVolumeSureFitSegmentation(BrainSet* bs,
                                                 const Structure::STRUCTURE_TYPE structureIn,
                                                 const VolumeFile::FILE_READ_WRITE_TYPE typeOfVolumeFilesToWriteIn,
                                                 const bool generateHullSurfaceFlagIn)
   : BrainModelAlgorithm(bs)
{
   typeOfVolumeFilesToWrite = typeOfVolumeFilesToWriteIn;
   autoSaveFilesFlag = true;
   anatomyVolume = NULL;
   segmentationVentriclesFilledVolume = NULL;
   whiteMatterThreshNoEyeVolume = NULL;
   whiteMatterThreshNoEyeFloodVolume = NULL;
   cerebralWmNoBstemFill = NULL;
   innerMask1Volume = NULL;
   gradIntensityVolume = NULL;
   eyeFatSculptVolume = NULL;
   gmILevelVolume = NULL;
   outerMaskVolume = NULL;
   wmThreshFloodVolume = NULL;
   inTotalVolume = NULL;
   inTotalThinWMVolume = NULL;
   outTotalVolume = NULL;
   thinWMOrNearVentricleHCMask = NULL;
   ventGradLevelBlurVolume = NULL;
   inTotalBlur1Volume = NULL;
   outTotalBlur1Volume = NULL;
   cerebralWMErodeVolume = NULL;
   hindbrainFloodVolume = NULL;
      
   gradPiaLevelVec = NULL;
   gradThinWMlevelVecFile = NULL;
   gradIntensityVecFile = NULL;
   gradInTotalThinWMVecFile = NULL;
   gradGWlevelVecFile = NULL;
   outGradPialLevelGMGradOutITMagVecFile = NULL;
   
   generateHullSurfaceFlag = generateHullSurfaceFlagIn;

   if (bs->getNumberOfVolumeSegmentationFiles() <= 0) {
      throw BrainModelAlgorithmException("No segmentation volume is loaded in brain set.");
   }
   else if (bs->getNumberOfVolumeSegmentationFiles() > 1) {
      throw BrainModelAlgorithmException("More than one segmentation volumes are loaded in brain set.");
   }
   segmentationVolume = new VolumeFile(*(bs->getVolumeSegmentationFile(0)));

   fiducialSurface = bs->getBrainModelSurfaceOfType(BrainModelSurface::SURFACE_TYPE_FIDUCIAL);
   if (fiducialSurface == NULL) {
      throw BrainModelAlgorithmException("No fiducial surface is loaded in brain set.");
   }
   
   acIJK[0] = 0;
   acIJK[1] = 0;
   acIJK[2] = 0;
   structure = structureIn;
   
   for (int i = 0; i < 6; i++) {
      partialHemispherePadding[i] = 0;
   }
   
   volumeMask = NULL;
   whiteMatterMaximum = 0.0;
}

/**
 * Destructor.
 */
BrainModelVolumeSureFitSegmentation::~BrainModelVolumeSureFitSegmentation()
{
   freeAllFilesInMemory();
   
   if (DebugControl::getDebugOn() == false) {
      QDir dir;
      dir.rmdir(segmentationDebugFilesSubDirectory);
   }
}

/**
 * get error correction methods and names.
 */
void 
BrainModelVolumeSureFitSegmentation::getErrorCorrectionMethodsAndNames(
                            std::vector<QString>& namesOut,
                            std::vector<ERROR_CORRECTION_METHOD>& methodsOut)
{
   namesOut.clear();   methodsOut.clear();
   namesOut.push_back("NONE");  methodsOut.push_back(ERROR_CORRECTION_METHOD_NONE); 
   namesOut.push_back("GRAPH");  methodsOut.push_back(ERROR_CORRECTION_METHOD_GRAPH); 
   namesOut.push_back("SUREFIT");  methodsOut.push_back(ERROR_CORRECTION_METHOD_SUREFIT); 
   namesOut.push_back("SUREFIT_THEN_GRAPH");  methodsOut.push_back(ERROR_CORRECTION_METHOD_SUREFIT_AND_GRAPH); 
   namesOut.push_back("GRAPH_THEN_SUREFIT");  methodsOut.push_back(ERROR_CORRECTION_METHOD_GRAPH_AND_SUREFIT); 
}                                  

/**
 * set the volume mask applied prior to inner and outer boundary determination.
 */
void 
BrainModelVolumeSureFitSegmentation::setVolumeMask(const VolumeFile* volumeMaskIn)
{
   volumeMask = new VolumeFile(*volumeMaskIn);
}
      
/**
 * set white maximum (values larger than this are excluded prior to inner and outer boundary determination.
 */
void 
BrainModelVolumeSureFitSegmentation::setWhiteMatterMaximum(const float whiteMatterMaximumIn)
{
   whiteMatterMaximum = whiteMatterMaximumIn;
}
      
/**
 * Free all volumes and vector files in memory.
 */
void
BrainModelVolumeSureFitSegmentation::freeAllFilesInMemory()
{
   freeVolumeInMemory(anatomyVolume);
   freeVolumeInMemory(whiteMatterThreshNoEyeVolume);
   freeVolumeInMemory(whiteMatterThreshNoEyeFloodVolume);
   freeVolumeInMemory(cerebralWmNoBstemFill);
   freeVolumeInMemory(innerMask1Volume);
   freeVolumeInMemory(gradIntensityVolume);
   freeVolumeInMemory(eyeFatSculptVolume);
   freeVolumeInMemory(gmILevelVolume);
   freeVolumeInMemory(outerMaskVolume);
   freeVolumeInMemory(wmThreshFloodVolume);
   freeVolumeInMemory(inTotalVolume);
   freeVolumeInMemory(inTotalThinWMVolume);
   freeVolumeInMemory(outTotalVolume);
   freeVolumeInMemory(thinWMOrNearVentricleHCMask);
   freeVolumeInMemory(ventGradLevelBlurVolume);
   freeVolumeInMemory(inTotalBlur1Volume);
   freeVolumeInMemory(outTotalBlur1Volume);
   freeVolumeInMemory(cerebralWMErodeVolume);
   freeVolumeInMemory(segmentationVolume);
   freeVolumeInMemory(segmentationVentriclesFilledVolume);
   freeVolumeInMemory(hindbrainFloodVolume);
   freeVolumeInMemory(volumeMask);
   
   freeVectorInMemory(gradPiaLevelVec);
   freeVectorInMemory(gradThinWMlevelVecFile);
   freeVectorInMemory(gradIntensityVecFile);
   freeVectorInMemory(gradInTotalThinWMVecFile);
   freeVectorInMemory(gradGWlevelVecFile);
   freeVectorInMemory(outGradPialLevelGMGradOutITMagVecFile);
}

/**
 * free a volume file in memory.
 */
void
BrainModelVolumeSureFitSegmentation::freeVolumeInMemory(VolumeFile* &vf)
{
   if (vf != NULL) {
      delete vf;
      vf = NULL;
   }
}

/**
 * free a vector file in memory.
 */
void
BrainModelVolumeSureFitSegmentation::freeVectorInMemory(SureFitVectorFile* &vf)
{
   if (vf != NULL) {
      delete vf;
      vf = NULL;
   }
}

/**
 * execute the algorithm for only identifying sulci.
 */
void 
BrainModelVolumeSureFitSegmentation::executeIdentifySulci() throw (BrainModelAlgorithmException)
{
   if (segmentationVolume == NULL) {
      throw BrainModelAlgorithmException("Segmentation volume is NULL");
   }
   
   switch (structure) {
      case Structure::STRUCTURE_TYPE_CORTEX_LEFT:
      case Structure::STRUCTURE_TYPE_CORTEX_RIGHT:
         break;
      case Structure::STRUCTURE_TYPE_CORTEX_BOTH:
      case Structure::STRUCTURE_TYPE_CEREBELLUM:
      case Structure::STRUCTURE_TYPE_CEREBELLUM_OR_CORTEX_LEFT:
      case Structure::STRUCTURE_TYPE_CEREBELLUM_OR_CORTEX_RIGHT:
      case Structure::STRUCTURE_TYPE_CORTEX_LEFT_OR_CEREBELLUM:
      case Structure::STRUCTURE_TYPE_CORTEX_RIGHT_OR_CEREBELLUM:
      case Structure::STRUCTURE_TYPE_CEREBRUM_CEREBELLUM:
      case Structure::STRUCTURE_TYPE_SUBCORTICAL:
      case Structure::STRUCTURE_TYPE_ALL:
      case Structure::STRUCTURE_TYPE_INVALID:
         QString msg("Struture must be either \"");
         msg += Structure::convertTypeToString(Structure::STRUCTURE_TYPE_CORTEX_LEFT);
         msg += "\" or \"";
         msg += Structure::convertTypeToString(Structure::STRUCTURE_TYPE_CORTEX_RIGHT);
         msg += "\".";
         throw BrainModelAlgorithmException(msg);
         break;
   }
   
   //
   // Create the segmentation debug files directory
   //
   segmentationDebugFilesSubDirectory = "SEGMENTATION_DEBUG_VOLUMES";
   QDir debugDir(segmentationDebugFilesSubDirectory);
   if (debugDir.exists() == false) {
      QDir temp(".");
      temp.mkdir(segmentationDebugFilesSubDirectory);
   }

   //
   // Determine AC voxel
   //
   float zeros[3] = { 0.0, 0.0, 0.0 };
   segmentationVolume->convertCoordinatesToVoxelIJK(zeros, acIJK);
   
   //
   // setup parameters for AC offsets and stuff
   //
   getParameters();
   
   //
   // Generate depth, curvature, and geography
   //
   generateDepthCurvatureGeography(segmentationVolume);
   
   freeAllFilesInMemory();
}
      
/**
 * execute the algorithm.
 */
void 
BrainModelVolumeSureFitSegmentation::execute() throw (BrainModelAlgorithmException)
{
   if ((anatomyVolume == NULL) && (segmentationVolume == NULL)) {
      throw BrainModelAlgorithmException("No anatomy or segmentation volume available.");
   }
   
   QTime timer;
   timer.start();
   
   if (DebugControl::getDebugOn()) {
      if (anatomyVolume != NULL) {
         float mins, maxs;
         anatomyVolume->getMinMaxVoxelValues(mins, maxs);
         std::cout << "Anatomy Volume min/max voxels: " << mins << ", " << maxs << std::endl;
      }
   }

   //
   // Create the segmentation debug files directory
   //
   segmentationDebugFilesSubDirectory = "SEGMENTATION_DEBUG_VOLUMES";
   QDir debugDir(segmentationDebugFilesSubDirectory);
   if (debugDir.exists() == false) {
      QDir temp(".");
      temp.mkdir(segmentationDebugFilesSubDirectory);
   }

   try {   
      //
      // Read parameters from paramters file
      //
      getParameters();
      
      int PROGRESS_PROGRESS_DISCONNECT_EYE = -1;
      int PROGRESS_DISCONNECT_HIND_BRAIN = -1;
      int PROGRESS_CUT_CORPUS_CALLOSUM = -1;
      int PROGRESS_GENERATE_INNER_BOUNDARY = -1;
      int PROGRESS_GENERATE_OUTER_BOUNDARY = -1;
      int PROGRESS_GENERATE_LAYER_4 = -1;
      int PROGRESS_FILL_VENTRICLES = -1;
      int PROGRESS_AUTOMATIC_ERROR_CORRECTION = -1;
      int PROGRESS_GENERATE_SURFACE = -1;
      int PROGRESS_GENERATE_INFLATED_ELLIPSOID_SURFACE = -1;
      int PROGRESS_GENERATE_SULCAL_ID = -1;
      int PROCESS_GENERATE_REGISTER_FLATTEN_LANDMARKS = -1;
      
      int numSteps = 1;
      //
      // Check inputs
      //
      if (disconnectEyeFlag) {
         if (anatomyVolume == NULL) {
            throw BrainModelAlgorithmException("Cannot disconnect eye.  No Anatomy Volume.");
         }
         PROGRESS_PROGRESS_DISCONNECT_EYE = numSteps;
         numSteps++;
      }
      if (disconnectHindBrainFlag) {
         if (anatomyVolume == NULL) {
            throw BrainModelAlgorithmException("Cannot hindbrain.  No Anatomy Volume.");
         }
         PROGRESS_DISCONNECT_HIND_BRAIN = numSteps;
         numSteps++;
      }
      if (cutCorpusCallosumFlag) {
         if (anatomyVolume == NULL) {
            throw BrainModelAlgorithmException("Cannot cut corpus callosum.  No Anatomy Volume.");
         }
         PROGRESS_CUT_CORPUS_CALLOSUM = numSteps;
         numSteps++;
      }
      
      if (anatomyVolume != NULL) {
         if (generateInnerBoundaryFlag) {
            PROGRESS_GENERATE_INNER_BOUNDARY = numSteps;
            numSteps++;
         }
         if (generateOuterBoundaryFlag) {
            PROGRESS_GENERATE_OUTER_BOUNDARY = numSteps;
            numSteps++;
         }
         if (generateSegmentationFlag) {
            PROGRESS_GENERATE_LAYER_4 = numSteps;
            numSteps++;
         }
      }
      
      if (fillVentriclesFlag) {
         PROGRESS_FILL_VENTRICLES = numSteps;
         numSteps++;
      }
      
      switch (errorCorrectionMethod) {
         case ERROR_CORRECTION_METHOD_NONE:
            break;
         case ERROR_CORRECTION_METHOD_GRAPH:
         case ERROR_CORRECTION_METHOD_SUREFIT:
         case ERROR_CORRECTION_METHOD_SUREFIT_AND_GRAPH:
         case ERROR_CORRECTION_METHOD_GRAPH_AND_SUREFIT:
            PROGRESS_AUTOMATIC_ERROR_CORRECTION = numSteps;
            numSteps++;
            break;
      }
      
      if (generateRawAndFidualSurfacesFlag) {
         PROGRESS_GENERATE_SURFACE = numSteps;
         numSteps++;
      }
      if (generateInflatedSurfaceFlag ||
          generateVeryInflatedSurfaceFlag ||
          generateEllipsoidSurfaceFlag ||
          generateSphericalSurfaceFlag ||
          generateCompressedMedialWallSurfaceFlag) {
         if (generateRawAndFidualSurfacesFlag == false) {
            throw BrainModelAlgorithmException("You must create raw and fiducial if you want "
                                               "inflated and/or ellipsoid surfaces.");
         }
         PROGRESS_GENERATE_INFLATED_ELLIPSOID_SURFACE = numSteps;
         numSteps++;
      }
      if (generateDepthCurvatureGeographyFlag) {
         if (generateRawAndFidualSurfacesFlag == false) {
            throw BrainModelAlgorithmException("You must create raw and fiducial if you want "
                                               "to generate depth, curvature, and geography.");
         }
         PROGRESS_GENERATE_SULCAL_ID = numSteps;
         numSteps++;
      }
      if (identifyRegisterFlattenLandmarksFlag) {
         if (generateInflatedSurfaceFlag == false) {
            throw BrainModelAlgorithmException("You must generate an inflated surface"
                                               " if you want to generate registration and flattening landmark borders.");
         }
         if (generateVeryInflatedSurfaceFlag == false) {
            throw BrainModelAlgorithmException("You must generate a very inflated surface"
                                               " if you want to generate registration and flattening landmark borders.");
         }
         if (generateEllipsoidSurfaceFlag == false) {
            throw BrainModelAlgorithmException("You must generate an ellipsoid surface"
                                               " if you want to generate registration and flattening landmark borders.");
         }
         //if (generateSphericalSurfaceFlag == false) {
         //   throw BrainModelAlgorithmException("You must generate a spherical surface"
         //                                      " if you want to generate registration and flattening landmarks borders.");
         //}
         if (generateDepthCurvatureGeographyFlag == false) {
            throw BrainModelAlgorithmException("You must  generate depth, curvature, and geography"
                                               " if you want to generate registration and flattening landmark borders.");
         }
         PROCESS_GENERATE_REGISTER_FLATTEN_LANDMARKS = numSteps;
         numSteps++;
      }
      createProgressDialog("Segmentation Processing",
                           numSteps,
                           "segmentationProgressDialog");
      
      //
      // descriptive name for segmentation volume (only set if segmentation is performed)
      //
      QString segmentationVolumeDescription;
      
      //
      // Should anatomy volume be processed
      //
      if (anatomyVolume != NULL) {
         //
         // make volume write float
         // 
         anatomyVolume->setVoxelDataType(VolumeFile::VOXEL_DATA_TYPE_FLOAT);
         
         //
         // Disconnect the eye
         //
         if (disconnectEyeFlag) {
            updateProgressDialog("Disconnecting the eye.",
                                     PROGRESS_PROGRESS_DISCONNECT_EYE);
            disconnectEye();
            
            if ((disconnectHindBrainFlag   == false) &&
                (cutCorpusCallosumFlag     == false) &&
                (generateInnerBoundaryFlag == false) &&
                (generateOuterBoundaryFlag == false) &&
                (generateSegmentationFlag  == false)) {
               segmentationVolume = new VolumeFile(*whiteMatterThreshNoEyeVolume);
               segmentationVolumeDescription = "EyeAndSkullDisconnected";
            }
         }
         
         //
         // Disconnect the hind brain
         //
         if (disconnectHindBrainFlag) {
            updateProgressDialog("Disconnecting the hind brain.",
                                     PROGRESS_DISCONNECT_HIND_BRAIN);
            disconnectHindBrain();
            
            if ((cutCorpusCallosumFlag     == false) &&
                (generateInnerBoundaryFlag == false) &&
                (generateOuterBoundaryFlag == false) &&
                (generateSegmentationFlag  == false)) {
               segmentationVolume = new VolumeFile(*cerebralWmNoBstemFill);
               segmentationVolumeDescription = "HindBrainDisconnected";
            }
         }
         else {
            if (disconnectEyeFlag) {
               cerebralWmNoBstemFill = whiteMatterThreshNoEyeFloodVolume;
            }
            else {
               //
               // Needed if disconnecting hind brain skipped
               //
               //cerebralWmNoBstemFill = new VolumeFile(*anatomyVolume);
               //cerebralWmNoBstemFill->setAllVoxels(0.0);
            }
         }
         
         //
         // cut the corpus callosum
         //
         if (cutCorpusCallosumFlag) {
            updateProgressDialog("Cutting the corpus callosum.",
                                     PROGRESS_CUT_CORPUS_CALLOSUM);
            cutCorpusCallossum();
            
            if ((generateInnerBoundaryFlag == false) &&
                (generateOuterBoundaryFlag == false) &&
                (generateSegmentationFlag  == false)) {
               segmentationVolume = new VolumeFile(*cerebralWMErodeVolume);
               segmentationVolumeDescription = "CorpusCallossumCut";
            }
         }

         //
         // Apply optional mask and white matter maximum
         //
         applyVolumeMaskAndWhiteMatterMaximum();

         //
         // Update params file with gray/white peaks
         //
         ParamsFile* paramsFile = brainSet->getParamsFile();
         if (paramsFile != NULL) {
            paramsFile->setParameter(ParamsFile::keyCGMpeak, cgmPeak);
            paramsFile->setParameter(ParamsFile::keyWMpeak, wmPeak);
            try {
               paramsFile->writeFile(paramsFile->getFileName());
            }
            catch (FileException&) {
            }
         }
         
         //
         // generate the inner boundary
         //
         if (generateInnerBoundaryFlag) {
            updateProgressDialog("Determining the inner boundary.",
                                     PROGRESS_GENERATE_INNER_BOUNDARY);
            generateInnerBoundary();
         }

         //
         // generate the outer boundary
         //
         if (generateOuterBoundaryFlag) {
            updateProgressDialog("Determining the outer boundary.",
                                     PROGRESS_GENERATE_OUTER_BOUNDARY);
            generateOuterBoundary();
         }

         //
         // Generate the segmentation
         //
         if (generateSegmentationFlag) {
            updateProgressDialog("Determining layer 4.",
                                     PROGRESS_GENERATE_LAYER_4);
            
            generateSegmentation();
            segmentationVolumeDescription = "Segmentation";
         }
      }
      
      //
      // Was the segmentation volume created ?
      //
      if ((segmentationVolume != NULL) && 
          (segmentationVolumeDescription.isEmpty() == false)) {
         //
         // make volume write float
         // 
         segmentationVolume->setVoxelDataType(VolumeFile::VOXEL_DATA_TYPE_FLOAT);

         //
         // Add it to the brain set
         //
         VolumeFile* temp = new VolumeFile(*segmentationVolume);
         temp->setFileWriteType(typeOfVolumeFilesToWrite);
         temp->makeDefaultFileName(segmentationVolumeDescription);
         temp->setDescriptiveLabel(segmentationVolumeDescription);
         brainSet->addVolumeFile(VolumeFile::VOLUME_TYPE_SEGMENTATION,
                                 temp,
                                 "",
                                 true,
                                 false);
                                 
         //
         // Save the segmentation volume ?
         //
         if (autoSaveFilesFlag) {
            try {
               brainSet->writeVolumeFile(temp->getFileName(),
                                         temp->getFileWriteType(),
                                         VolumeFile::VOLUME_TYPE_SEGMENTATION,
                                         temp);
            }
            catch (FileException& e) {
               addToWarningMessages(e.whatQString()); 
            }
         }
      }
      
      //
      // Is there a segmentation volume ?
      //
      if (segmentationVolume != NULL) {
         VolumeFile* segmentVolumeForProcessing = new VolumeFile(*segmentationVolume);
         
         //
         // make volume write float
         // 
         segmentVolumeForProcessing->setVoxelDataType(VolumeFile::VOXEL_DATA_TYPE_FLOAT);
         
         //
         // If ventricles should be filled
         //
         if (fillVentriclesFlag) {
            updateProgressDialog("Filling the ventricles.",
                                     PROGRESS_FILL_VENTRICLES);
            
            //
            // Fill the ventricles
            //
            fillVentricles();
            
            //
            // Were ventricles filled ?
            //
            if (segmentationVentriclesFilledVolume != NULL) {
               //
               // Add it to the brain set
               //
               VolumeFile* temp = new VolumeFile(*segmentationVentriclesFilledVolume);
               temp->setFileWriteType(typeOfVolumeFilesToWrite);
               temp->makeDefaultFileName("Segmentation_vent");
               temp->setDescriptiveLabel("Segmentation_vent");
               brainSet->addVolumeFile(VolumeFile::VOLUME_TYPE_SEGMENTATION,
                                       temp,
                                       "",
                                       true,
                                       false);
                                       
               delete segmentVolumeForProcessing;
               segmentVolumeForProcessing = new VolumeFile(*temp);
            
               //
               // Save the segmentation with ventricles filled volume ?
               //
               if (autoSaveFilesFlag) {
                  try {
                     brainSet->writeVolumeFile(temp->getFileName(),
                                               typeOfVolumeFilesToWrite,
                                               VolumeFile::VOLUME_TYPE_SEGMENTATION,
                                               temp);
                  }
                  catch (FileException& e) {
                     addToWarningMessages(e.whatQString()); 
                  }
               }
            }
         }
         
         //
         // Should errors be automatically corrected
         //
         if (errorCorrectionMethod != ERROR_CORRECTION_METHOD_NONE) {
            //
            // If a segmentation was generated from the anatomy volume
            //
            if ((anatomyVolume != NULL) && generateSegmentationFlag) {
               //
               // Generate a surface so that the user can see it and determine if
               // there was a problem such failure to remove the skull
               //
               generateRawAndFiducialSurfaces(segmentVolumeForProcessing);
               
               //
               // Cause surface to be displayed
               //
               brainSet->drawBrainModel(brainSet->getNumberOfBrainModels() - 1, 0);
               allowEventsToProcess();
            }
            
            updateProgressDialog("Automatic Error Correction.",
                                     PROGRESS_AUTOMATIC_ERROR_CORRECTION);
            
            //
            // Do error correction
            //
            VolumeFile* correctedVolume = NULL;
            
            QString defaultCorrectedName("Segment_ErrorCorrected");
            switch (errorCorrectionMethod) {
               case ERROR_CORRECTION_METHOD_NONE:
                  break;
               case ERROR_CORRECTION_METHOD_GRAPH:
                  defaultCorrectedName = "Segment_GraphErrorCorrected";
                  correctedVolume = graphBasedErrorCorrection(segmentVolumeForProcessing);
                  break;
               case ERROR_CORRECTION_METHOD_SUREFIT:
                  defaultCorrectedName = "Segment_SureFitErrorCorrected";
                  correctedVolume = sureFitAutomaticErrorCorrection(segmentVolumeForProcessing);
                  break;
               case ERROR_CORRECTION_METHOD_SUREFIT_AND_GRAPH:
                  {
                     correctedVolume = sureFitAutomaticErrorCorrection(segmentVolumeForProcessing);
                     VolumeFile* volumeToDelete = correctedVolume;
                     correctedVolume = graphBasedErrorCorrection(correctedVolume);
                     defaultCorrectedName = "Segment_SureFit_GraphErrorCorrected";
                     if (volumeToDelete != NULL) {
                        delete volumeToDelete;
                     }
                  }
                  break;
               case ERROR_CORRECTION_METHOD_GRAPH_AND_SUREFIT:
                  {
                     correctedVolume = graphBasedErrorCorrection(segmentVolumeForProcessing);
                     VolumeFile* volumeToDelete = correctedVolume;
                     correctedVolume = sureFitAutomaticErrorCorrection(correctedVolume);
                     defaultCorrectedName = "Segment_Graph_SureFitErrorCorrected";
                     if (volumeToDelete != NULL) {
                        delete volumeToDelete;
                     }
                  }
                  break;
            }
            
            if (correctedVolume != NULL) {
               correctedVolume->setFileWriteType(typeOfVolumeFilesToWrite);
               correctedVolume->makeDefaultFileName(defaultCorrectedName);
               correctedVolume->setDescriptiveLabel(defaultCorrectedName);
               
               //
               // Add the corrected volume to the brain set
               //
               brainSet->addVolumeFile(VolumeFile::VOLUME_TYPE_SEGMENTATION,
                                       correctedVolume,
                                       correctedVolume->getFileName(),
                                       true,
                                       true);
   
               //
               // Save the error corrected segmentation volume ?
               //
               if (autoSaveFilesFlag) {
                  try {
                     brainSet->writeVolumeFile(correctedVolume->getFileName(),
                                               typeOfVolumeFilesToWrite,
                                               VolumeFile::VOLUME_TYPE_SEGMENTATION,
                                               correctedVolume);
                  }
                  catch (FileException& e) {
                     addToWarningMessages(e.whatQString()); 
                  }
               }
               
               delete segmentVolumeForProcessing;
               segmentVolumeForProcessing = new VolumeFile(*correctedVolume);
            }
         }
                  
         //
         // If the surface should be generated
         //
         if (generateRawAndFidualSurfacesFlag) {
            updateProgressDialog("Generating the surface.",
                                     PROGRESS_GENERATE_SURFACE);
            
            //
            // Does the volume need to be padded for surface reconstruction
            // Only need to do this if generating an ellipsoid too
            //
            if (generateEllipsoidSurfaceFlag) {
               if ((partialHemispherePadding[0] != 0) ||
                   (partialHemispherePadding[1] != 0) ||
                   (partialHemispherePadding[2] != 0) ||
                   (partialHemispherePadding[3] != 0) ||
                   (partialHemispherePadding[4] != 0) ||
                   (partialHemispherePadding[5] != 0)) {
                  //
                  // Pad the segmentation
                  //
                  segmentVolumeForProcessing->padSegmentation(partialHemispherePadding, true);
                  segmentVolumeForProcessing->clearModified();
                  
                  //
                  // Update volume dimensions
                  //
                  segmentVolumeForProcessing->getDimensions(xDim, yDim, zDim);
               }
            }
            
            //
            // Generate the raw and fiducial surfaces
            //
            updateProgressDialog("Generating the raw and fiducial surfaces.",
                                     PROGRESS_GENERATE_SURFACE);
            generateRawAndFiducialSurfaces(segmentVolumeForProcessing);

            //
            // If fiducial surface should be corrected
            //
            if (generateTopologicallyCorrectFiducialSurfaceFlag) {
               updateProgressDialog("Correcting fiducial surface topology.",
                                     PROGRESS_GENERATE_SURFACE);
               generateTopologicallyCorrectFiducialSurface();
            }
            
            //
            // If inflated and ellipsoid surfaces should be generated
            //
            if (generateInflatedSurfaceFlag ||
                generateVeryInflatedSurfaceFlag ||
                generateEllipsoidSurfaceFlag ||
                generateSphericalSurfaceFlag||
                generateCompressedMedialWallSurfaceFlag) {
               updateProgressDialog("Generating the inflated, ellipsoid, and/or spherical surfaces.",
                                        PROGRESS_GENERATE_INFLATED_ELLIPSOID_SURFACE);
               
               //
               // Generate the inflated and ellipsoid surfaces
               //
               generateInflatedAndEllipsoidSurfaces();
            }
            
            //
            // Auto save topology and coordinate files
            //
            if (autoSaveFilesFlag) {
               for (int i = 0; i < brainSet->getNumberOfTopologyFiles(); i++) {
                  TopologyFile* tf = brainSet->getTopologyFile(i);
                  try {
                     brainSet->writeTopologyFile(tf->getFileName(),
                                                 tf->getTopologyType(),
                                                 tf);
                  }
                  catch (FileException& e) {
                     addToWarningMessages(e.whatQString()); 
                  }
               }
               
               for (int i = 0; i < brainSet->getNumberOfBrainModels(); i++) {
                  BrainModelSurface* bms = brainSet->getBrainModelSurface(i);
                  if (bms != NULL) {
                     CoordinateFile* cf = bms->getCoordinateFile();
                     try {
                        brainSet->writeCoordinateFile(cf->getFileName(),
                                                      bms->getSurfaceType(),
                                                      cf,
                                                      true);
                     }
                     catch (FileException& e) {
                        addToWarningMessages(e.whatQString()); 
                     }
                  }
               }
            }
            
            //
            // If sulci ID should be generated
            //
            if (generateDepthCurvatureGeographyFlag) {
               updateProgressDialog("Generating depth, curvature, and geography.",
                                        PROGRESS_GENERATE_SULCAL_ID);

               //
               // generate depth, curvature, and geography
               //
               generateDepthCurvatureGeography(segmentVolumeForProcessing);
            }
            
            //
            // If registration and flattening landmarks should be generated
            //
            if (identifyRegisterFlattenLandmarksFlag) {
               updateProgressDialog("Generating registration and flattening landmarks.",
                                    PROCESS_GENERATE_REGISTER_FLATTEN_LANDMARKS);
                                    
               generateRegistrationFlatteningLandmarkBorders();
            }
            
            //
            // Generate default scenes
            //
            generateDefaultScenes();
            
         }         
         if (segmentVolumeForProcessing != NULL) {
            delete segmentVolumeForProcessing;
            segmentVolumeForProcessing = NULL;
         }
      }
      
      if (DebugControl::getDebugOn()) {
         std::cout << "Time to perform segmentation: "
                   << (static_cast<float>(timer.elapsed()) / 1000.0)
                   << std::endl;
      }
   }
   catch (BrainModelAlgorithmException& e) {
      freeAllFilesInMemory();
      removeProgressDialog();
      throw BrainModelAlgorithmException(e.whatQString());
   }
   catch (FileException& e) {
      freeAllFilesInMemory();
      removeProgressDialog();
      throw BrainModelAlgorithmException(e.whatQString());
   }
   
   freeAllFilesInMemory();
   removeProgressDialog();
}

/**
 * apply volume mask and white matter maximum.
 */
void 
BrainModelVolumeSureFitSegmentation::applyVolumeMaskAndWhiteMatterMaximum() throw (BrainModelAlgorithmException)
{
   if (volumeMask != NULL) {
      //
      // Exclude voxels that fall outside optional mask
      //
      cerebralWMErodeVolume->maskWithVolume(volumeMask);
      innerMask1Volume->maskWithVolume(volumeMask);
      outerMaskVolume->maskWithVolume(volumeMask);
   }     
   
   if (whiteMatterMaximum > 0.0) {
      //
      // Turn off all voxels brighter than white matter maximum
      //
      int dim[3];
      anatomyVolume->getDimensions(dim);
      for (int i = 0; i < dim[0]; i++) {
         for (int j = 0; j < dim[1]; j++) {
            for (int k = 0; k < dim[2]; k++) {
               if (anatomyVolume->getVoxel(i, j, k, 0) > whiteMatterMaximum) {
                  cerebralWMErodeVolume->setVoxel(i, j, k, 0, 0.0);
                  innerMask1Volume->setVoxel(i, j, k, 0, 0.0);
                  outerMaskVolume->setVoxel(i, j, k, 0, 0.0);
               }
            }
         }
      }
   }
}
      
/**
 * do graph-based automatic error correction.
 */
VolumeFile* 
BrainModelVolumeSureFitSegmentation::graphBasedErrorCorrection(VolumeFile* vf)
{
   BrainModelVolumeTopologyGraphCorrector corrector(brainSet,
                BrainModelVolumeTopologyGraphCorrector::CORRECTION_MODE_NORMAL,
                vf);
   try {
      corrector.execute();
   }
   catch (BrainModelAlgorithmException& e) {
      throw e;
   }
   
   VolumeFile* volumeOut = NULL;
   if (corrector.getCorrectedSegmentationVolumeFile() != NULL) {
      volumeOut = new VolumeFile(*corrector.getCorrectedSegmentationVolumeFile());
   }
   return volumeOut;
}
      
/**
 * do automatic error correction.
 */
VolumeFile* 
BrainModelVolumeSureFitSegmentation::sureFitAutomaticErrorCorrection(VolumeFile* vf)
{
   VolumeFile *vol = new VolumeFile(*vf);
   
   //
   // Load the radial position map volume
   //
   VolumeFile radialPosVolume;
   try {
      switch (typeOfVolumeFilesToWrite) {
         case VolumeFile::FILE_READ_WRITE_TYPE_RAW:
            throw FileException("ERROR: RadioPositionMap wants to be read in RAW");
            break;
         case VolumeFile::FILE_READ_WRITE_TYPE_AFNI:
            radialPosVolume.readFile("RadialPositionMap+orig.HEAD");
            break;
         case VolumeFile::FILE_READ_WRITE_TYPE_ANALYZE:
            radialPosVolume.readFile("RadialPositionMap+orig.hdr");
            break;
         case VolumeFile::FILE_READ_WRITE_TYPE_NIFTI:
         case VolumeFile::FILE_READ_WRITE_TYPE_NIFTI_GZIP:
            if (QFile::exists("RadialPositionMap+orig.nii.gz")) {
               radialPosVolume.readFile("RadialPositionMap+orig.nii.gz");
            }
            else {
               radialPosVolume.readFile("RadialPositionMap+orig.nii");
            }
            break;
         case VolumeFile::FILE_READ_WRITE_TYPE_SPM_OR_MEDX: 
            radialPosVolume.readFile("RadialPositionMap+orig.hdr");
            break;
         case VolumeFile::FILE_READ_WRITE_TYPE_WUNIL:
            radialPosVolume.readFile("RadialPositionMap+orig.ifh");
            break;
         case VolumeFile::FILE_READ_WRITE_TYPE_UNKNOWN:
            throw FileException("ERROR: RadioPositionMap wants to be read in UNKNOWN");
            break;
      }
   }
   catch (FileException& e) {
      throw BrainModelAlgorithmException("Unable to find volume file \"RadialPositionMap+orig.*\"");
   }
   
   //
   // Run error correction
   //
   BrainModelVolumeSureFitErrorCorrection sfec(brainSet,
                                               vol,
                                               &radialPosVolume,
                                               typeOfVolumeFilesToWrite,
                                               acIJK,
                                               (Hem == 0),
                                               DebugControl::getDebugOn());
                                               
   try {
      sfec.execute();
   }
   catch (BrainModelAlgorithmException& e) {
      throw e;
   }
   
   //
   // Get the error corrected volume
   //
   if (vol != NULL) {
      delete vol;
   }
   vol = NULL;
   if (sfec.getOutputVolume() != NULL) {
      vol = new VolumeFile(*sfec.getOutputVolume());
   }
   
   return vol;
}

/**
 * Generate depth, curvature, and geography.
 */
void 
BrainModelVolumeSureFitSegmentation::generateDepthCurvatureGeography(const VolumeFile* segmentVolIn)
{
   //
   // Expand around edges with empty slices
   //
   VolumeFile segmentVolumeExpanded(*segmentVolIn);
   int expDim[3];
   segmentVolumeExpanded.getDimensions(expDim);
   const int expSlices = 7;
   const int resizeCrop[6] = { 
      -expSlices, expDim[0] + expSlices,
      -expSlices, expDim[1] + expSlices,
      -expSlices, expDim[2] + expSlices
   };
   segmentVolumeExpanded.resize(resizeCrop);
   writeDebugVolume(segmentVolumeExpanded, "SegmentExpandedForDepthCurveGeom");
   
   //
   // Add area colors if needed
   //
   AreaColorFile* areaColorFile = brainSet->getAreaColorFile();
   bool match = false;
   areaColorFile->getColorIndexByName("???", match);
   if (match == false) {
      areaColorFile->addColor("???", 170, 170, 170);
   }
   match = false;
   areaColorFile->getColorIndexByName("SUL", match);
   if (match == false) {
      areaColorFile->addColor("SUL", 130, 130, 130);
   }
   match = false;
   areaColorFile->getColorIndexByName("CENTRAL", match);
   if (match == false) {
      areaColorFile->addColor("CENTRAL", 255, 255, 0);
   }
   match = false;
   areaColorFile->getColorIndexByName("CALCARINE", match);
   if (match == false) {
      areaColorFile->addColor("CALCARINE", 255, 100, 0);
   }
   match = false;
   areaColorFile->getColorIndexByName("CUT.FACE", match);
   if (match == false) {
      areaColorFile->addColor("CUT.FACE", 255, 0, 0);
   }
   if (autoSaveFilesFlag) {
      if (areaColorFile->getModified()) {
         try {
            if (QFile::exists(areaColorFile->getFileName())) {
               brainSet->writeAreaColorFile(areaColorFile->getFileName());
            }
            else {
               brainSet->writeAreaColorFile(areaColorFile->makeDefaultFileName("Initial"));
            }
         }
         catch (FileException& e) {
            addToWarningMessages(e.whatQString()); 
         }
      }
   }
   
   //
   // Generate the hull volume and the cerebral hull VTK file
   // This creates the cerebral hull volume too.
   //
   brainSet->generateCerebralHullVtkFile(&segmentVolumeExpanded, false);
       
   VolumeFile* cerebralHullVolume = NULL;
   const int num = brainSet->getNumberOfVolumeSegmentationFiles() - 1;
   if (num >= 0) {
      //
      // Cerebral hull volume is newest segmentation volume
      //
      cerebralHullVolume = brainSet->getVolumeSegmentationFile(num);

      //
      // Should cerebral hull volume be saved
      //
      if (autoSaveFilesFlag) {
         try {
            cerebralHullVolume->setFileWriteType(typeOfVolumeFilesToWrite);
            cerebralHullVolume->makeDefaultFileName("CerebralHull");
            cerebralHullVolume->setDescriptiveLabel("CerebralHull");
            cerebralHullVolume->setFileWriteType(typeOfVolumeFilesToWrite);
            brainSet->writeVolumeFile(cerebralHullVolume->getFileName(),
                                      typeOfVolumeFilesToWrite,
                                      cerebralHullVolume->getVolumeType(),
                                      cerebralHullVolume);
         }
         catch (FileException& e) {
            addToWarningMessages(e.whatQString());
         }
      }
   }
   
   //
   // cerebral hull surface
   BrainModelSurface* hullSurface = NULL;
   CoordinateFile* hullCoordFile = NULL;
   if (generateHullSurfaceFlag) {
      hullSurface = new BrainModelSurface(*fiducialSurface);
      hullCoordFile = hullSurface->getCoordinateFile();
      hullCoordFile->makeDefaultFileName("CerebralHull");
      brainSet->addBrainModel(hullSurface);
   }
   
   //
   //
   // Create the sulcal depth map in the surface shape file 
   //
   const int hullSmoothingIterations = 5;
   const int depthSmoothingIterations = 100;
   SurfaceShapeFile* ssf = brainSet->getSurfaceShapeFile();
   BrainModelSurfaceSulcalDepthWithNormals bmssd(brainSet,
                                    fiducialSurface,
                                    brainSet->getCerebralHullFileName(),
                                    ssf,
                                    hullSmoothingIterations,
                                    depthSmoothingIterations,
                                    BrainModelSurfaceSulcalDepthWithNormals::DEPTH_COLUMN_CREATE_NEW,
                                    BrainModelSurfaceSulcalDepthWithNormals::DEPTH_COLUMN_CREATE_NEW,
                                    "Depth",
                                    "Smoothed Depth",
                                    hullCoordFile);
   bmssd.execute();
   
   //
   // Add curvature to the surface shape file
   //
   BrainModelSurfaceCurvature bmsc(brainSet,
                                   fiducialSurface,
                                   ssf,
                                   BrainModelSurfaceCurvature::CURVATURE_COLUMN_CREATE_NEW,
                                   BrainModelSurfaceCurvature::CURVATURE_COLUMN_CREATE_NEW,
                                   "Folding (Mean Curvature)",
                                   "Gaussian Curvature");
   bmsc.execute();
   
   //
   // Should files be saved
   //
   if (autoSaveFilesFlag) {
      try {
         if (QFile::exists(ssf->getFileName())) {
            brainSet->writeSurfaceShapeFile(ssf->getFileName());
         }
         else {
            brainSet->writeSurfaceShapeFile(ssf->makeDefaultFileName("Initial"));
         }
      }
      catch (FileException& e) {
         addToWarningMessages(e.whatQString());
      }
         
      //
      // Save hull surface too
      //
      if (hullSurface != NULL) {
         try {
            brainSet->writeCoordinateFile(hullCoordFile->getFileName(),
                                          BrainModelSurface::SURFACE_TYPE_HULL,
                                          hullCoordFile);
         }
         catch (FileException& e) {
            addToWarningMessages(e.whatQString());
         }
      }
   }

   //#VolMorphOps.py 0 3 CerebralHull.mnc CerebralHull.erode.3
   //SureFitOps.VolMorphOps (0, 3, data)
   //che3fname = "%s/%s" % (SulDirectory, "CerebralHull.erode.3.mnc")
   //WriteNetCDFFile (che3fname, data, xdim, ydim, zdim)
   VolumeFile data(*cerebralHullVolume);
   data.doVolMorphOps(0, 3);
   writeDebugVolume(data, "CerebralHull.erode.3");
   VolumeFile cerebralHullErode3(data);
   
   //#VolMorphOps.py 0 1 CerebralHull.erode.3.mnc CerebralHull.erode.4
   //SureFitOps.VolMorphOps (0, 1, data)
   //che4fname = "%s/%s" % (SulDirectory, "CerebralHull.erode.4.mnc")
   //WriteNetCDFFile (che4fname, data, xdim, ydim, zdim)
   data.doVolMorphOps(0, 1);
   writeDebugVolume(data, "CerebralHull.erode.4");
   VolumeFile cerebralHullErode4(data);
   
   //#VolMorphOps.py 0 6 CerebralHull.erode.4.mnc CerebralHull.erode.10
   //SureFitOps.VolMorphOps (0, 6, data)
   //che10fname = "%s/%s" % (SulDirectory, "CerebralHull.erode.10.mnc")
   //WriteNetCDFFile (che10fname, data, xdim, ydim, zdim)
   data.doVolMorphOps(0, 6);
   writeDebugVolume(data, "CerebralHull.erode.10");
   VolumeFile cerebralHullErode10(data);

   //#CombineVols.py subrect CerebralHull.erode.3.mnc $Segment_file BuriedCortex.3deep
   //vol = volume.Volume (che3fname)
   //data = vol.VoxData3D
   //SureFitOps.CombineVols ("subrect", data, segdata, segdata)
   //bc3fname = "%s/%s.BuriedCortex.3deep.mnc" % 
   //        (SulDirectory, ReadParams.GetFilePrefix ())
   //WriteNetCDFFile (bc3fname, data, xdim, ydim, zdim)
   VolumeFile segData(segmentVolumeExpanded);
   data = cerebralHullErode3;
   VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                                            &data,
                                            &segData,
                                            &segData,
                                            &data);
   data.stretchVoxelValues();
   writeDebugVolume(data, "BuriedCortex.3deep");
   VolumeFile bc3vol(data);
        
     //#VolMorphOps.py 1 0 BuriedCortex.3deep.mnc Sulci.3.dilate
     //SureFitOps.VolMorphOps (1, 0, data)
     //fname = "%s/%s" % (SulDirectory, "Sulci.3.dilate.mnc")
     //WriteNetCDFFile (fname, data, xdim, ydim, zdim)
   data.doVolMorphOps(1, 0);
   writeDebugVolume(data, "Sulci.3.dilate");
   
   //  #IntersectVolumeWithSurface.py $fname Sulci.3.dilate.mnc SUL 1 -0.5 -0.5 -0.5
   //  paintstring = "SUL"
   //  col_number = 0
   //  xoffset = yoffset = zoffset = -0.5
   //  SureFitOps.IntersectVolumeWithSurface (specfname, data, paintstring, col_number, xoffset, yoffset, zoffset)
   //  oldpaintfile = "%s.paint" % paintstring
   //  os.rename (oldpaintfile, paintfile)
   //  #2002-12: want this paint in both specific sulci and geography columns
   //  paintstring = "SUL"
   //  col_number = 1
   //  xoffset = yoffset = zoffset = -0.5
   //  SureFitOps.IntersectVolumeWithSurface (specfname, data, paintstring, col_number, xoffset, yoffset, zoffset)
   //  oldpaintfile = "%s.paint" % paintstring
   //  os.rename (oldpaintfile, paintfile)

   //
   // 27 Jan 2006 per DVE
   // To determine geography, use the cerebral hull eroded three times
   //
   const BrainModelSurface* rawBMS = brainSet->getBrainModelSurfaceOfType(
                                                 BrainModelSurface::SURFACE_TYPE_RAW);
   if (rawBMS == NULL) {
      throw BrainModelAlgorithmException("Unable to find raw surface for sulci ID");
   }
   
   const bool identifyCalcarineAndCentralFlag = false;
   int numPaintColumns = 0;
   int sulciIDColumnNumber = -1;
   if (identifyCalcarineAndCentralFlag) {
      sulciIDColumnNumber = numPaintColumns++;
   }
   const int geographyColumnNumber = numPaintColumns++;
   const CoordinateFile* rawCoordFile = rawBMS->getCoordinateFile();
   PaintFile* pf = brainSet->getPaintFile();
   pf->setNumberOfNodesAndColumns(brainSet->getNumberOfNodes(), numPaintColumns);
   if (sulciIDColumnNumber >= 0) {
      pf->setColumnName(sulciIDColumnNumber, "Sulci ID");
   }
   pf->setColumnName(geographyColumnNumber, "Geography");
   if (sulciIDColumnNumber >= 0) {
      pf->assignPaintColumnWithVolumeFile(&cerebralHullErode3, //&data,
                                          rawCoordFile,
                                          sulciIDColumnNumber,
                                          "SUL");
   }
   pf->assignPaintColumnWithVolumeFile(&cerebralHullErode3, //&data,
                                       rawCoordFile,
                                       geographyColumnNumber,
                                       "SUL");
                                       
   //
   // Set CUT.FACE paint ID for padded volumes
   //
   assignPaddedCutFaceNodePainting(rawCoordFile,
                                   &segmentVolumeExpanded,
                                   pf,
                                   geographyColumnNumber);

   if (identifyCalcarineAndCentralFlag) {
      //  #CombineVols.py subrect CerebralHull.erode.4.mnc $Segment_file BuriedCortex.4deep
      //  vol = volume.Volume (che4fname)
      //  data = vol.VoxData3D
      //  SureFitOps.CombineVols ("subrect", data, segdata, segdata)
      //  bc4fname = "%s/%s" % (SulDirectory, "BuriedCortex.4deep.mnc")
      //  WriteNetCDFFile (bc4fname, data, xdim, ydim, zdim)
      data = cerebralHullErode4;
      VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                                               &data,
                                               &segData,
                                               &segData,
                                               &data);
      data.stretchVoxelValues();
      writeDebugVolume(data, "BuriedCortex.4deep");
      VolumeFile buriedCortex4Deep(data);
      
      //if (acIJK[1] > (yDim - oldPadPosY)) {
      if (acIJK[1] > (yDim - partialHemispherePadding[3])) { 
         if (DebugControl::getDebugOn()) {
            std::cout << "AC forward of anterior wall; skipping central sulcus." << std::endl;
         }
      }
      else {
         const int xAC_CeSlat = acIJK[0] + Hem3 * 40;
         const int xAC_CeSmed = acIJK[1] + Hem3 * 10;
         const int CeSxlow = xAC_CeSlat * Hem2 + xAC_CeSmed * Hem;
         const int CeSxhigh = xAC_CeSlat * Hem + xAC_CeSmed * Hem2;
         const int CeSymin = acIJK[1] - 40;
         const int CeSymax = acIJK[1] - 5;
         const int CeSzmin = acIJK[2] + 40;
         const int CeSzmax = acIJK[2] + 80;
         // #FillBiggestObject.py BuriedCortex.4deep.mnc CentralSulcus.4below $CeSxlow $CeSxhigh $CeSymin $CeSymax $CeSzmin $CeSzmax 
         // x1 = CeSxlow
         // x2 = CeSxhigh
         // y1 = CeSymin
         // y2 = CeSymax
         // z1 = CeSzmin
         // z2 = CeSzmax
         // fboseed, data = FillBiggestObject (data, x1, x2, y1, y2, z1, z2)
         // fname = "%s/%s" % (SulDirectory, "CentralSulcus.4below.mnc")
         // WriteNetCDFFile (fname, data, xdim, ydim, zdim)
         data.fillBiggestObjectWithinMask(CeSxlow, CeSxhigh, CeSymin, CeSymax, CeSzmin, CeSzmax, 255, 255);
         writeDebugVolume(data, "CentralSulcus.4below");

         // #Sculpt.py 0 5 0 $ncol 0 $nrow 0 $nslices CentralSulcus.4below.mnc BuriedCortex.3deep.mnc CentralSulcus
         // x1 = 0
         // x2 = xdim
         // y1 = 0
         // y2 = ydim
         // z1 = 0
         // z2 = zdim
         // SureFitOps.Sculpt (0, 5, 0, 0, 0, x1, x2, y1, y2, z1, z2, data, bc3vol.VoxData3D)
         // fname = "%s/%s.CentralSulcus.mnc" % 
         //         (SulDirectory, ReadParams.GetFilePrefix ())
         // WriteNetCDFFile (fname, data, xdim, ydim, zdim)
         // if refreshslice == 1:
         //         SureSliceFcn.SetSecondVolume (data)
         int seed[3] = { 0, 0, 0 };
         int extent[6] = { 0, xDim, 0, yDim, 0, zDim };
         data.sculptVolume(0, &bc3vol, 5, seed, extent);
         writeDebugVolume(data, "CentralSulcus");
         
         // #FindLimits.py CentralSulcus.mnc Limits.CeS
         // fname = "%s/%s.Limits.CeS" % 
         //         (SulDirectory, ReadParams.GetFilePrefix ())
         // CesLim = SureFitOps.FindLimits (fname, data)
         data.findLimits("CentralSulculs.limits", extent);
         
         // #VolMorphOps.py 1 0 CentralSulcus.mnc CentralSulcus.dilate
         // SureFitOps.VolMorphOps (1, 0, data)
         // csdfname = "%s/%s.CentralSulcus.dilate.mnc" % 
         // (SulDirectory, ReadParams.GetFilePrefix ())
         // WriteNetCDFFile (csdfname, data, xdim, ydim, zdim)
         data.doVolMorphOps(1, 0);
         writeDebugVolume(data, "CentralSulcus.dilate");

         // #IntersectVolumeWithSurface.py $fname CentralSulcus.dilate.mnc CENTRAL 1 -0.5 -0.5 -0.5
         // paintstring = "CENTRAL"
         // col_number = 0
         // SureFitOps.IntersectVolumeWithSurface (specfname, data, paintstring, col_number, xoffset, yoffset, zoffset)
         // oldpaintfile = "%s.paint" % paintstring
         // os.rename (oldpaintfile, paintfile)
         pf->assignPaintColumnWithVolumeFile(&data,
                                          rawCoordFile,
                                          sulciIDColumnNumber,
                                          "CENTRAL");
      }
      //  #CombineVols.py subrect CerebralHull.erode.10.mnc $Segment_file BuriedCortex.10deep
      //  vol = volume.Volume (che10fname)
      //  data = vol.VoxData3D
      //  SureFitOps.CombineVols ("subrect", data, segdata, segdata)
      //  bc10fname = "%s/%s.BuriedCortex.10deep.mnc" % 
      //          (SulDirectory, ReadParams.GetFilePrefix ())
      //  WriteNetCDFFile (bc10fname, data, xdim, ydim, zdim)
      data = cerebralHullErode10;
      VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                                               &data,
                                               &segData,
                                               &segData,
                                               &data);
      data.stretchVoxelValues();
      writeDebugVolume(data, "BuriedCortex.10deep");

      //  #VolMorphOps.py 1 0 BuriedCortex.10deep.mnc Sulci.10.dilate
      //  SureFitOps.VolMorphOps (1, 0, data)
      //  s10dfname = "%s/%s" % (SulDirectory, "Sulci.10.dilate.mnc")
      //  WriteNetCDFFile (s10dfname, data, xdim, ydim, zdim)
      data.doVolMorphOps(1, 0);
      writeDebugVolume(data, "Sulci.10.dilate");
      
      //     #MaskVol.py  BuriedCortex.4deep.mnc BuriedCortex.4deep.CaSmask $CaSxlow $CaSxhigh $CaSymin $CaSymax $CaSzmin $CaSzmax
      //     x1 = CaSxlow     
      //     x2 = CaSxhigh
      //     y1 = CaSymin
      //     y2 = CaSymax
      //     z1 = CaSzmin
      //     z2 = CaSzmax
      //     vol = volume.Volume (bc4fname)
      //     data = vol.VoxData3D
      //     SureFitOps.MaskVol (x1, x2, y1, y2, z1, z2, data)
      //     fname = "%s/%s" % (SulDirectory, "BuriedCortex.4deep.CaSmask.mnc") 
      //     WriteNetCDFFile (fname, data, xdim, ydim, zdim)
      const int xAC_CaSlat = acIJK[0] + Hem3 * 30;
      const int xAC_CaSmed = acIJK[0];
      const int CaSxlow  = xAC_CaSlat * Hem2 + xAC_CaSmed * Hem;
      const int CaSxhigh = xAC_CaSlat * Hem  + xAC_CaSmed * Hem2;
      const int CaSymin  = acIJK[1] - 100;
      const int CaSymax  = acIJK[1] - 70;
      const int CaSzmin  = acIJK[2];
      const int CaSzmax  = acIJK[2] + 30;
      data = buriedCortex4Deep;
      int extent[6];
      extent[0] = CaSxlow;
      extent[1] = CaSxhigh;
      extent[2] = CaSymin;
      extent[3] = CaSymax;
      extent[4] = CaSzmin;
      extent[5] = CaSzmax;
      data.maskVolume(extent);
      writeDebugVolume(data, "BuriedCortex.4deep.CaSmask");
           
      //  #FillBiggestObject.py BuriedCortex.4deep.CaSmask.mnc CalcarineSulcus.4below $CaSxlow $CaSxhigh $CaSymin $CaSymax $CaSzmin $CaSzmax 
      //  fboseed, data = FillBiggestObject (data, x1, x2, y1, y2, z1, z2)
      //  fname = "%s/%s" % (SulDirectory, "CalcarineSulcus.4below.mnc")
      //  WriteNetCDFFile (fname, data, xdim, ydim, zdim)
      data.fillBiggestObjectWithinMask(extent, 255, 255);
      writeDebugVolume(data, "CalcarineSulcus.4below");

      //  #Sculpt.py 0 8 0 $ncol 0 $nrow 0 $nslices CalcarineSulcus.4below.mnc BuriedCortex.3deep.mnc CalcarineSulcus
      //  x1 = 0
      //  x2 = xdim
      //  y1 = 0
      //  y2 = ydim
      //  z1 = 0
      //  z2 = zdim
      //  SureFitOps.Sculpt (0, 8, 0, 0, 0, x1, x2, y1, y2, z1, z2, data, bc3vol.VoxData3D)
      //  fname = "%s/%s.CalcarineSulcus.mnc" % 
      //          (SulDirectory, ReadParams.GetFilePrefix ())
      //  WriteNetCDFFile (fname, data, xdim, ydim, zdim)
      extent[0] = 0;
      extent[1] = xDim;
      extent[2] = 0;
      extent[3] = yDim;
      extent[4] = 0;
      extent[5] = zDim;
      int seed[3];
      seed[0] = 0;
      seed[1] = 0;
      seed[2] = 0; //hghjhj
      data.sculptVolume(0, &bc3vol, 8, seed, extent);
      writeDebugVolume(data, "CalcarineSulcus");
      
      //  #FindLimits.py CalcarineSulcus.mnc Limits.CaS
      //  fname = "%s/%s.Limits.CaS" % 
      //          (SulDirectory, ReadParams.GetFilePrefix ())
      //  CesLim = SureFitOps.FindLimits (fname, data)
      data.findLimits("Cas.Limits", extent);

      //     #VolMorphOps.py 1 0 CalcarineSulcus.mnc CalcarineSulcus.dilate
      //     SureFitOps.VolMorphOps (1, 0, data)
      //     fname = "%s/%s.CalcarineSulcus.dilate.mnc" % 
      //             (SulDirectory, ReadParams.GetFilePrefix ())
      //     WriteNetCDFFile (fname, data, xdim, ydim, zdim)
      data.doVolMorphOps(1, 0);
      writeDebugVolume(data, "CalcarineSulcus.dilate");

      //  #IntersectVolumeWithSurface.py $fname CalcarineSulcus.dilate.mnc CALCARINE 1 -0.5 -0.5 -0.5
      //  paintstring = "CALCARINE"
      //  col_number = 0
      //  SureFitOps.IntersectVolumeWithSurface (specfname, data, paintstring, col_number, xoffset, yoffset, zoffset)
      //  oldpaintfile = "%s.paint" % paintstring
      //  os.rename (oldpaintfile, paintfile)
      //  geographyfname = re.sub(".Surface.", ".geography.", specfname)
      //  geographyfname = re.sub(".spec", ".paint", geographyfname)
      //  os.rename (paintfile, geographyfname)
      pf->assignPaintColumnWithVolumeFile(&data,
                                          rawCoordFile,
                                          sulciIDColumnNumber,
                                          "CALCARINE");   
   }
   
   //
   // Should the paint file be saved
   //
   if (autoSaveFilesFlag) {
      PaintFile* pf = brainSet->getPaintFile();
      try {
         if (QFile::exists(pf->getFileName())) {
            brainSet->writePaintFile(pf->getFileName());
         }
         else {
            brainSet->writePaintFile(pf->makeDefaultFileName("Initial"));
         }
      }
      catch (FileException& e) {
         addToWarningMessages(e.whatQString());
      }
   }
}

/**
 * generate default scenes.
 */
void 
BrainModelVolumeSureFitSegmentation::generateDefaultScenes() throw (BrainModelAlgorithmException)
{
   //
   // Update display settings and other things in brain set
   //
   brainSet->postSpecFileReadInitializations();
   
   //
   // Set underlay to surface shape
   //
   BrainModelSurfaceOverlay* underlay = brainSet->getSurfaceUnderlay();
   underlay->setOverlay(-1, BrainModelSurfaceOverlay::OVERLAY_SURFACE_SHAPE);
   DisplaySettingsSurfaceShape* dsss = brainSet->getDisplaySettingsSurfaceShape();
   const SurfaceShapeFile* ssf = brainSet->getSurfaceShapeFile();
   const int underlayOverlayNumber = underlay->getOverlayNumber();
   if (ssf->getSulcalDepthColumnNumber() >= 0) {
      dsss->setSelectedDisplayColumn(-1, underlayOverlayNumber, ssf->getSulcalDepthColumnNumber());
   }
   else if (ssf->getMeanCurvatureColumnNumber() >= 0) {
      dsss->setSelectedDisplayColumn(-1, underlayOverlayNumber, ssf->getMeanCurvatureColumnNumber());
   }
   
   //
   // Turn on display of borders
   //
   brainSet->getDisplaySettingsBorders()->setDisplayBorders(true);
   
   //
   // For each window, main and viewing
   //
   std::vector<SceneFile::SceneClass> windowSceneClasses;
            
   //
   // Lateral view of very inflated in Main Window
   //
   BrainModelSurface* veryInflatedSurface = 
      brainSet->getBrainModelSurfaceOfType(BrainModelSurface::SURFACE_TYPE_VERY_INFLATED);
   if (veryInflatedSurface == NULL) {
      return;
   }

   //
   // Set the view
   //

   const int mainWindowGeometry[4] = { 50, 50, 600, 600 };
   const int viewWindowGeometry[4] = { 600, 50, 400, 400 };
   const int graphicsSizeX = 512;
   const int graphicsSizeY = 512;
   const int mainWindowGraphicsSize[2] = { -1, -1 }; 
   const int viewWindowGraphicsSize[2] = { graphicsSizeX, graphicsSizeY };
   if (brainSet->getProgressDialogParent() == NULL) {
      double orthoRight, orthoTop;
      BrainModelOpenGL::getDefaultOrthoRightAndTop(graphicsSizeX,
                                                   graphicsSizeY,
                                                   orthoRight,
                                                   orthoTop);
      brainSet->setDefaultScaling(orthoRight, orthoTop);
   }

   //
   // create the lateral view in the main window
   //
   SceneFile::SceneClass mainWindowSceneClass("");
   veryInflatedSurface->setToStandardView(BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW, 
                                      BrainModel::VIEW_LATERAL);
   brainSet->saveSceneForBrainModelWindow(BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW,
                                         mainWindowGeometry,
                                         mainWindowGraphicsSize,
                                         veryInflatedSurface,
                                         false,
                                         mainWindowSceneClass);
   windowSceneClasses.push_back(mainWindowSceneClass);

   //
   // create the medial view in the view window
   //
   SceneFile::SceneClass viewWindowSceneClass("");
   veryInflatedSurface->setToStandardView(BrainModel::BRAIN_MODEL_VIEW_AUX_WINDOW_2, 
                                      BrainModel::VIEW_MEDIAL);
   brainSet->saveSceneForBrainModelWindow(BrainModel::BRAIN_MODEL_VIEW_AUX_WINDOW_2,
                                         viewWindowGeometry,
                                         viewWindowGraphicsSize,
                                         veryInflatedSurface,
                                         false,
                                         viewWindowSceneClass);
   windowSceneClasses.push_back(viewWindowSceneClass);

   QString errorMessage;
   QString warningMessage;
   brainSet->saveScene(brainSet->getSceneFile(),
                       windowSceneClasses,
                       "Lateral/Medial Views of Landmarks",
                       false,
                       errorMessage,
                       warningMessage);
   errorMessage.append(warningMessage);
   if (errorMessage.isEmpty() == false) {
      throw BrainModelAlgorithmException(errorMessage);
   }
   
   //
   // Save scene file
   //
   if (autoSaveFilesFlag) {
      SceneFile* sf = brainSet->getSceneFile();
      try {
         if (QFile::exists(sf->getFileName())) {
            brainSet->writeSceneFile(sf->getFileName());
         }
         else {
            brainSet->writeSceneFile(sf->makeDefaultFileName("Initial"));
         }
      }
      catch (FileException& e) {
         addToWarningMessages(e.whatQString());
      }
   }
}

/**
 * generate landmarks borders for flattening and registration.
 */
void 
BrainModelVolumeSureFitSegmentation::generateRegistrationFlatteningLandmarkBorders() throw (BrainModelAlgorithmException)
{
   BorderProjectionFile borderProjectionFile;
   BrainModelSurfaceBorderLandmarkIdentification 
      bmsbli(brainSet,
             brainSet->getStereotaxicSpace(),
             anatomyVolume,
             brainSet->getBrainModelSurfaceOfType(BrainModelSurface::SURFACE_TYPE_FIDUCIAL),
             brainSet->getBrainModelSurfaceOfType(BrainModelSurface::SURFACE_TYPE_INFLATED),
             brainSet->getBrainModelSurfaceOfType(BrainModelSurface::SURFACE_TYPE_VERY_INFLATED),
             brainSet->getBrainModelSurfaceOfType(BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL),
             brainSet->getSurfaceShapeFile(),
             brainSet->getSurfaceShapeFile()->getSulcalDepthColumnNumber(),
             brainSet->getPaintFile(),
             brainSet->getPaintFile()->getGeographyColumnNumber(),
             brainSet->getAreaColorFile(),
             &borderProjectionFile,
             brainSet->getBorderColorFile(),
             brainSet->getVocabularyFile(),
             BrainModelSurfaceBorderLandmarkIdentification::OPERATION_ID_ALL);
   bmsbli.execute();
   
   brainSet->getBorderSet()->copyBordersFromBorderProjectionFile(&borderProjectionFile);

   brainSet->getDisplaySettingsBorders()->setDisplayBorders(true);
   
   //
   // Save data files
   //
   if (autoSaveFilesFlag) {
      AreaColorFile* acf = brainSet->getAreaColorFile();
      if (acf->getModified()) {
         try {
            if (QFile::exists(acf->getFileName())) {
               brainSet->writeAreaColorFile(acf->getFileName());
            }
            else {
               brainSet->writeAreaColorFile(acf->makeDefaultFileName("Initial"));
            }
         }
         catch (FileException& e) {
            addToWarningMessages(e.whatQString()); 
         }
      }

      PaintFile* pf = brainSet->getPaintFile();
      try {
         if (QFile::exists(pf->getFileName())) {
            brainSet->writePaintFile(pf->getFileName());
         }
         else {
            brainSet->writePaintFile(pf->makeDefaultFileName("Initial"));
         }
      }
      catch (FileException& e) {
         addToWarningMessages(e.whatQString());
      }
      
      VocabularyFile* vocabularyFile = brainSet->getVocabularyFile();
      if (vocabularyFile->getModified()) {
         try {
            if (QFile::exists(vocabularyFile->getFileName())) {
               brainSet->writeVocabularyFile(vocabularyFile->getFileName());
            }
            else {
               brainSet->writeVocabularyFile(vocabularyFile->makeDefaultFileName("Initial"));
            }
         }
         catch (FileException& e) {
            addToWarningMessages(e.whatQString()); 
         }
      }
      
      BorderColorFile* borderColorFile = brainSet->getBorderColorFile();
      if (borderColorFile->getModified()) {
         try {
            if (QFile::exists(borderColorFile->getFileName())) {
               brainSet->writeBorderColorFile(borderColorFile->getFileName());
            }
            else {
               brainSet->writeBorderColorFile(borderColorFile->makeDefaultFileName("Initial"));
            }
         }
         catch (FileException& e) {
            addToWarningMessages(e.whatQString()); 
         }
      }
      
      if (brainSet->getBorderSet()->getNumberOfBorders() > 0) {
         BorderProjectionFile bpf;
         const QString name = bpf.makeDefaultFileName("LANDMARKS");
         try {
            brainSet->writeBorderProjectionFile(name, "", "");
         }
         catch (FileException& e) {
            addToWarningMessages(e.whatQString()); 
         }
      }
   }
}

/**
 * assign paint for padded CUT.FACE nodes.
 */
void 
BrainModelVolumeSureFitSegmentation::assignPaddedCutFaceNodePainting(const CoordinateFile* cf,
                                                                     const VolumeFile* segmentVol,
                                                                     PaintFile* pf,
                                                                     const int columnToAssign)
{
   if ((partialHemispherePadding[0] > 0) ||
       (partialHemispherePadding[1] > 0) ||
       (partialHemispherePadding[2] > 0) ||
       (partialHemispherePadding[3] > 0) ||
       (partialHemispherePadding[4] > 0) ||
       (partialHemispherePadding[5] > 0)) {

      //
      // Make a copy of the segmentation volume
      //
      VolumeFile padVol(*segmentVol);
      
      //
      // Turn on all voxels (all padding)
      //
      padVol.setAllVoxels(1);
      
      //
      // Turn off voxels that are not in padding region
      //
      const int iStart = partialHemispherePadding[0];
      const int iEnd   = xDim - partialHemispherePadding[1];
      const int jStart = partialHemispherePadding[2];
      const int jEnd   = yDim - partialHemispherePadding[3];
      const int kStart = partialHemispherePadding[4];
      const int kEnd   = zDim - partialHemispherePadding[5];
      for (int i = iStart; i < iEnd; i++) {
         for (int j = jStart; j < jEnd; j++) {
            for (int k = kStart; k < kEnd; k++) {
               padVol.setVoxel(i, j, k, 0, 0);
            }
         }
      }
      
      //
      // Intersect volume with coordinates to for CUT.FACE in geography column
      //
      pf->assignPaintColumnWithVolumeFile(&padVol,
                                          cf,
                                          columnToAssign,
                                          "CUT.FACE");
   }
}
                                           
/**
 * generate the surfaces.
 */
void 
BrainModelVolumeSureFitSegmentation::generateRawAndFiducialSurfaces(VolumeFile* vf) throw (BrainModelAlgorithmException)
{
   //
   // Clear surface files from the spec file
   //
   try {
      SpecFile sf;
      sf.readFile(brainSet->getSpecFileName());
      sf.clearFiles(false, true, false, true);
      sf.writeFile(sf.getFileName());
   }
   catch (FileException& e) {
      std::cout << "ERROR: clearing surface files from the spec file." << std::endl;
   }
   
   //
   // Set reconstruction mode
   //
   BrainModelVolumeToSurfaceConverter::RECONSTRUCTION_MODE reconMode = 
      BrainModelVolumeToSurfaceConverter::RECONSTRUCTION_MODE_SUREFIT_SURFACE;
   if (maximumPolygonsFlag) {
      reconMode = BrainModelVolumeToSurfaceConverter::RECONSTRUCTION_MODE_SUREFIT_SURFACE_MAXIMUM_POLYGONS;
   }
   
   //
   // Generate the surface
   //
   BrainModelVolumeToSurfaceConverter bmvsc(brainSet,
                      vf,
                      reconMode,
                      brainSet->getStructure() == (Structure::STRUCTURE_TYPE_CORTEX_RIGHT),
                      brainSet->getStructure() == (Structure::STRUCTURE_TYPE_CORTEX_LEFT));
   bmvsc.execute();
   
   //
   // name surfaces
   //
   for (int i = 0; i < brainSet->getNumberOfBrainModels(); i++) {
      BrainModelSurface* bms = brainSet->getBrainModelSurface(i);
      if (bms != NULL) {
         CoordinateFile* cf = bms->getCoordinateFile();
         cf->makeDefaultFileName(bms->getSurfaceTypeName());
         
         if (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL) {
            fiducialSurface = bms;
         }
      }
   }
   
   //
   // Name topology files
   //
   for (int i = 0; i < brainSet->getNumberOfTopologyFiles(); i++) {
      TopologyFile* tf = brainSet->getTopologyFile(i);
      tf->makeDefaultFileName(tf->getTopologyTypeName());
   }
   
   if (fiducialSurface == NULL) {
      throw BrainModelAlgorithmException("Generation of fiducial surface failed.");
   }
}

/**
 * create a fiducial surface that is topologically correct.
 */
void 
BrainModelVolumeSureFitSegmentation::generateTopologicallyCorrectFiducialSurface()
{
   if (fiducialSurface != NULL) {
      const TopologyFile* tf = fiducialSurface->getTopologyFile();
      if (tf != NULL) {
         //
         // An Euler count of 2 means that there are no topological errors in the surface.
         // If that is the case, there is no need to do correction.
         //
         int faces, vertices, edges, eulerCount, holes, objects;
         tf->getEulerCount(false, faces, vertices, edges, eulerCount, holes, objects);
         if (eulerCount == 2) {
            return;
         }
         
         //
         // Correct the surface
         //
         BrainModelSurfaceTopologyCorrector bmstc(brainSet,
                                                  fiducialSurface);
         try {
            bmstc.execute();
            
            //
            // Use corrected fiducial surface for further processing
            //
            fiducialSurface = bmstc.getPointerToNewSurface();
         }
         catch (BrainModelAlgorithmException& e) {
            //
            // Let user know correction failed but do not abort segmentation process
            //
            std::cout << "WARNING: surface topology correction failed." << std::endl;
            std::cout << "         " << e.whatQString().toAscii().constData() << std::endl;
            return;
         }
      }
   }
   // be sure to set the member fiducialSurface !!!!
}
      
/**
 * generate the inflated and ellipsoid surfaces.
 */
void 
BrainModelVolumeSureFitSegmentation::generateInflatedAndEllipsoidSurfaces() throw (BrainModelAlgorithmException)
{
   fiducialSurface->createInflatedAndEllipsoidFromFiducial(generateInflatedSurfaceFlag,
                                                           generateVeryInflatedSurfaceFlag,
                                                           generateEllipsoidSurfaceFlag,
                                                           generateSphericalSurfaceFlag,
                                                           generateCompressedMedialWallSurfaceFlag,
                                                           true,
                                                           true,
                                                           1.0,
                                                           NULL);
}
      
/**
 * disconnect the eye.
 */
void
BrainModelVolumeSureFitSegmentation::disconnectEye() throw (BrainModelAlgorithmException)
{
   if (DebugControl::getDebugOn()) {
   	std::cout << "#### DisconnectEye begin ####" << std::endl;
   }
	int seed[3]={0,0,0};
	//int extent[6]={0,0,0,0,0,0};
   //int ACx = acIJK[0];
	int ACy = acIJK[1];
	int ACz = acIJK[2];

   //
   // Copy the input volume
   //
   VolumeFile* volume = new VolumeFile(*anatomyVolume);
   
	//DISCONNECT EYE SECTION:
	//5 if ACz > 0, enter loop:
	bool EyeFatDisconnectRound2 = true;
	bool EyeFound = true;

	if (( ACz > 0) && ( ACy < yDim )) {
		for (int i = 1 ; i < 9 ; i++) {
			const float WMhiThreshTemp = wmThresh + (10 * i);
			if ( WMhiThreshTemp > 255) {
            if (DebugControl::getDebugOn()) {
   				std::cout << "REACHED THRESH LIMIT" << std::endl;
            }
				break;
			}
         
         //
         // Copy the input volume
         //
         delete volume;
         volume = new VolumeFile(*anatomyVolume);

         //
         // Threshold the input volume
         //
         volume->thresholdVolume(static_cast<int>(WMhiThreshTemp));

         //
         // Find the biggest object within the specified region
         //
         VoxelIJK vijk(seed);
         volume->findBiggestObjectWithinMask(xAC_15_40_low, 
                                             xAC_15_40_high, 
                                             acIJK[1] + 20, 
                                             yDim, 
                                             0, 
                                             acIJK[2] - 15, 
                                             255.0,
                                             255.0,
                                             vijk);
         
         vijk.getIJK(seed);
			if ((seed[0] == -1) || (seed[1] == -1) || (seed[2] == -1)) {
            if (DebugControl::getDebugOn()) {
   				std::cout << "NO EYE FOUND" << std::endl;
            }
				EyeFound = false;
				EyeFatDisconnectRound2 = false;
				writeDebugVolume(volume, "EyeFat.Flood");
				break;
			}
         
         //
         // Fill the biggest object
         //
         volume->floodFillWithVTK(seed, 255, 255, 0);
         writeDebugVolume(volume, "EyeFat.TestFlood");

         //
         // Find non-zero voxel extent
         //
         int extent[6];
         volume->findLimits("EyeFat.TestFlood.limits", extent);
         
			if (extent[5] < (acIJK[2] + 20)) {
            if (DebugControl::getDebugOn()) {
               std::cout << "EYE DISCONNECTED FROM CEREBRUM" << std::endl;
            }
				writeDebugVolume(volume, "EyeFat.Flood");
				EyeFatDisconnectRound2 = false;
				break;
			}
		}
	} //  if (( ACz > 0) && ( ACy < ydim )) {
   else {
      if (DebugControl::getDebugOn()) {
   		std::cout << "No need to disconnect eye (out of volume range)." << std::endl;
      }
		EyeFound = false;
		EyeFatDisconnectRound2 = false;
	}

	if (EyeFatDisconnectRound2) {
		//1/26/2001  EyeFatDisconnectRound2 if previous loop failed to disconnect eye
		// e.g., brl19_mpr_n4_111_t88_gfc_8bit.L.full.sMRI.mnc
      if (DebugControl::getDebugOn()) {
		   std::cout << "EYE FAT NOT DISCONNECTED; TRYING SECOND PASS" << std::endl;
      }
      
		for (int i = 1; i < 9; i++) {
			const float WMhiThreshTemp = wmThresh + (10 * i);
			if ( WMhiThreshTemp > 255) {
            if (DebugControl::getDebugOn()) {
   				std::cout << "REACHED THRESH LIMIT BEFORE EYE DISCONNECTED." << std::endl;
            }
            throw BrainModelAlgorithmException("REACHED THRESH LIMIT BEFORE EYE DISCONNECTED.");
			}
         
         //
         // Copy the input volume
         //
         delete volume;
         volume = new VolumeFile(*anatomyVolume);

         //
         // Threshold the volume
         //         
			volume->thresholdVolume(static_cast<int>(WMhiThreshTemp));
         
         //
         // Set extent for masking the volume
         //
         int extent[6] = {
            xAClow,
            xAChigh,
            acIJK[1],
            acIJK[1] + 35,
            acIJK[2] - 30,
            acIJK[2]
         };
         
         //
         // Mask the volume to within the extent
         //
         volume->maskVolume(extent);
         
         //
         // Stretch the volume values to full range of 0 - 255
         //
         volume->stretchVoxelValues();
         
         writeDebugVolume(volume, "WMhiThreshTemp");
         
         
         //
         // Find the biggest object within the specified region
         //
         VoxelIJK voxelIJK(seed);
         volume->findBiggestObjectWithinMask(xAC_15_40_low, 
                                             xAC_15_40_high, 
                                             acIJK[1] + 20, 
                                             yDim, 
                                             0, 
                                             acIJK[2] - 15,
                                             255,
                                             255, 
                                             voxelIJK);
         voxelIJK.getIJK(seed);
			if (( seed[0] == -1 ) || ( seed[1] == -1 ) || ( seed[2] == -1 )) {
            if (DebugControl::getDebugOn()) {
   				std::cout << "NO EYE FOUND" << std::endl;
            }
				writeDebugVolume(volume, "EyeFat.Flood");
				break;
			}
         
         //
         // Flood fille the volume
         //
         volume->floodFillWithVTK(seed, 255, 255, 0);
			writeDebugVolume(volume, "EyeFat.TestFlood");
         
         //
         // Find non-zero voxel extent
         //
         volume->findLimits("EyeFat.TestFlood.limits", extent);
         
			if ( extent[5] < ACz-5 ) {
				if (DebugControl::getDebugOn()) {
               std::cout << "EYE DISCONNECTED FROM CEREBRUM" << std::endl;
            }
            writeDebugVolume(volume, "EyeFat.Flood");
				break;
			}
			if ( i==8 ) { 
				std::cout << "ERROR IN EYE FAT SEGMENTATION" << std::endl;
				throw BrainModelAlgorithmException("ERROR IN EYE FAT SEGMENTATION");
			}	
		}
	}
   

	if ( EyeFound ) {
		//SmearAxis.py EyeFat.Flood.mnc EyeFat.SmearVent 2 5 -1 1
		//%doSmearAxis (2, 5, -1, 1, voxdataflat, xdim, ydim, zdim);
      volume->smearAxis(VolumeFile::VOLUME_AXIS_Z, 5, -1, 1);
      volume->stretchVoxelValues();
		writeDebugVolume(volume, "EyeFat.SmearVent");
      
		//%unsigned char* efsvdata=new unsigned char [num_voxels];
		//%for ( i=0 ; i<num_voxels ; i++ ) efsvdata[i]=voxdataflat[i];
      VolumeFile volumeCopy(*volume);
      
		//ShiftAxis.py EyeFat.Flood.mnc EyeFat.SlideVent 2 10 -1
		//%ShiftAxis (xdim, ydim, zdim, voxdataflat, 2, -10);
		//%write_minc ("EyeFat.SlideVent.mnc", voxdataflat, xdim, ydim, zdim);
      volume->shiftAxis(VolumeFile::VOLUME_AXIS_Z, -10);
      
		//SmearAxis.py EyeFat.SlideVent.mnc EyeFat.SlideVent_SmearXpos 0 5 1 1
		//%doSmearAxis (0, 5, 1, 1, voxdataflat, xdim, ydim, zdim);
		//%write_minc ("EyeFat.SlideVent_SmearXpos.mnc", voxdataflat, xdim, ydim, zdim);
      volume->smearAxis(VolumeFile::VOLUME_AXIS_X, 5, 1, 1);
      volume->stretchVoxelValues();
      writeDebugVolume(volume, "EyeFat.SlideVent_SmearXpos");
      
		//SmearAxis.py EyeFat.SlideVent_SmearXpos.mnc EyeFat.SlideVent_SmearXpos_Xneg 0 5 -1 1
		//%doSmearAxis (0, 5, -1, 1, voxdataflat, xdim, ydim, zdim);
		//%write_minc ("EyeFat.SlideVent_SmearXpos_Xneg.mnc", voxdataflat, xdim, ydim, zdim);
      volume->smearAxis(VolumeFile::VOLUME_AXIS_X, 5, -1, 1);
      volume->stretchVoxelValues(); // 10/29
      writeDebugVolume(volume, "EyeFat.SlideVent_SmearXpos_Xneg");
      
		//CombineVols.py OR EyeFat.SmearVent.mnc EyeFat.SlideVent_SmearXpos_Xneg.mnc EyeFat.SmearVent_MedLat
		//%CombineVols ("OR", voxdataflat, efsvdata, efsvdata, xdim, ydim, zdim);
		//%write_minc ("EyeFat.SmearVent_MedLat.mnc", voxdataflat, xdim, ydim, zdim);
		//%delete [] efsvdata;
      VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_OR,
                                   volume, &volumeCopy, NULL, volume); 
      volume->stretchVoxelValues(); //10/29
      writeDebugVolume(volume, "EyeFat.SmearVent_MedLat");
            
		//3/24/2003: Added mask for Wieser's YL_tlrc.L.full.sMRI.mnc missing temporal pole
      int extent[6];
		extent[0]=0;
		extent[1]=xDim;
		extent[2]=ACy+25;
		extent[3]=yDim;
		extent[4]=0;
		extent[5]=zDim;
		//%MaskVolume (voxdataflat, xdim, ydim, zdim, extent);
      volume->maskVolume(extent); 
      volume->stretchVoxelValues(); // 10/29
      
		//InvertThresh.py ../$fname CSF.thresh CSFThresh
		//%unsigned char* csftdata=new unsigned char [num_voxels];
		//%for ( i=0 ; i<num_voxels ; i++ ) csftdata[i]=inputdata[i];
		//%InverseThresholdVolume (csftdata, int(CSFThresh), xdim, ydim, zdim);
		//%write_minc ("CSF.thresh.mnc", csftdata, xdim, ydim, zdim);
      VolumeFile volumeCSF(*anatomyVolume);
      volumeCSF.inverseThresholdVolume(static_cast<int>(csfThresh));
      writeDebugVolume(&volumeCSF, "CSF.thresh");

		//8 Sculpt.py 2 2 xMedLimit_40_low xMedLimit_40_high ACy nrow 0 `expr ACz + 20` EyeFat.SmearVent_MedLat.mnc CSF.thresh.mnc EyeFat.sculpt
		seed[0] = 0;
      seed[1] = 0;
      seed[2] = 0;
		extent[0] = xMedLimit_50_low;
		extent[1] = xMedLimit_50_high;
		extent[2] = ACy;
		extent[3] = yDim;
		extent[4] = 0;
		extent[5] = ACz + 20;
		//%Sculpt (2, 2, seed, extent, voxdataflat, csftdata, xdim, ydim, zdim);
		//%write_minc ("EyeFat.sculpt.mnc", voxdataflat, xdim, ydim, zdim);
      volume->sculptVolume(2, &volumeCSF, 2, seed, extent);
      volume->stretchVoxelValues();
      writeDebugVolume(volume, "EyeFat.sculpt");
      
		//%delete [] csftdata;	//Thresh.py ../fname WM.thresh.mnc WMThresh
	} else {
		//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=0;
		//%write_minc ("EyeFat.sculpt.mnc", voxdataflat, xdim, ydim, zdim);
      volume->setAllVoxels(0.0);
      writeDebugVolume(volume, "EyeFat.sculpt");
	}
   
   //
   // Copy for use in hind brain removal
   //
   eyeFatSculptVolume = new VolumeFile(*volume);
   
	//Thresh.py ../$fname WM.thresh.mnc WMThresh
	//%ThresholdVolume (inputdata, int(WMThresh), xdim,ydim,zdim);
	//%write_minc ("WhiteMatter.Thresholded.mnc", inputdata, xdim, ydim, zdim);   
   VolumeFile* inputVolume = new VolumeFile(*anatomyVolume);
   inputVolume->thresholdVolume(static_cast<int>(wmThresh));
   writeDebugVolume(inputVolume, "WhiteMatter.Thresholded");
   
	//CombineVols.py subrect WM.thresh.mnc EyeFat.sculpt.mnc WM.thresh_noEye
	//%CombineVols ("subrect", inputdata, voxdataflat, voxdataflat, xdim, ydim, zdim);
	//%write_minc ("WM.thresh_noEye.mnc", inputdata, xdim, ydim, zdim);   
   VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                                               inputVolume, volume, volume, inputVolume);
   inputVolume->stretchVoxelValues();
   writeDebugVolume(inputVolume, "WM.thresh_noEye");
   whiteMatterThreshNoEyeVolume = new VolumeFile(*inputVolume);
   
	//%FindBiggestObjectWithinMask (inputdata, xdim, ydim, zdim,
	//%	0, xdim, 0, ydim, 0, zdim, seed);
	//%vtkFloodFill (seed, inputdata, 255, 255, 0, xdim, ydim, zdim);
	//&write_minc ("WM.thresh_noEye.flood.mnc", inputdata, xdim, ydim, zdim); 
   VolumeFile floodVolume(*inputVolume);
   VoxelIJK seedIJK(seed);
   floodVolume.findBiggestObjectWithinMask(0, xDim, 0, yDim, 0, zDim,
                                            255.0, 255.0, seedIJK);
   if (seedIJK.getI() < 0) {
      throw BrainModelAlgorithmException(
         "findBiggestObjectWithinMask() failed to find biggest object when\n"
         "trying to create intermediate volume \"WM.thresh_noEye.flood\".");
   }
   floodVolume.floodFillWithVTK(seedIJK, 255, 255, 0);
   writeDebugVolume(&floodVolume, "WM.thresh_noEye.flood");
   whiteMatterThreshNoEyeFloodVolume = new VolumeFile(floodVolume);
   
   delete volume;
   
   if (DebugControl::getDebugOn()) {
   	std::cout << "#### DisconnectEye done ####" << std::endl;
   }
}

/**
 * disconnect the hind brain.
 */
void
BrainModelVolumeSureFitSegmentation::disconnectHindBrain() throw (BrainModelAlgorithmException)
{
	if (DebugControl::getDebugOn()) {
      std::cout << "#### DisconnectHindbrain begin ####" << std::endl;
   }
   
	int ACx = acIJK[0];
	int ACy = acIJK[1];
	int ACz = acIJK[2];
	//%float WMhiThreshTemp;

	// HINDBRAIN LOOP
	//%unsigned char* inputdata=new unsigned char [num_voxels];
	//%unsigned char* voxdataflat=new unsigned char [num_voxels];
	//%unsigned char* hbdata=new unsigned char [num_voxels];
	//%unsigned char* wmtnedata=new unsigned char [num_voxels];
	//%read_minc_file (mincfile, inputdata);
	//%if ( lp.skipeye ) {for ( i=0 ; i<num_voxels ; i++ ) wmtnedata[i]=inputdata[i];}
	//%else {read_minc_file ("WM.thresh_noEye.mnc", wmtnedata);}

   //
   // Copy input volume
   //
   VolumeFile inputData(*anatomyVolume);
   VolumeFile wmtnedata(inputData);
   if (disconnectEyeFlag) {
      wmtnedata = *whiteMatterThreshNoEyeVolume;
   }
   else {
      wmtnedata.thresholdVolume(static_cast<int>(wmThresh));
      writeDebugVolume(wmtnedata, "WhiteMatter.Thresholded");
   }
   VolumeFile hbdata(inputData);
   VolumeFile voxdataflat(wmtnedata);
   
   freeVolumeInMemory(whiteMatterThreshNoEyeFloodVolume);
   
	bool hbloopflag = false;
	bool hbnull = false;
	//%int seed[3] = { 0, 0, 0};
	//%int extent[6] = { 0, 0, 0, 0, 0, 0 };
   if (DebugControl::getDebugOn()) {
   	std::cout << "ACz=" << ACz << " ; ACy=" << ACy << " ; ydim=" << yDim << std::endl;
   }
	if (ACz > 0) {
      hbloopflag = true;
   }
	if (yDim < (ACy-60)) {
      hbloopflag = false;
   }
	if ((yDim < (ACy-40)) && (zDim < 10)) {
      hbloopflag = false;
   }

	if ( hbloopflag ) {
		// PREPARE CUT PLANES

		//2 MakePlane.py Hem3 ACx 1 ACy 1 ACz -20 2  ../fname CutForHindbrain
		//%unsigned char* cutdata1=new unsigned char [num_voxels];
		//%for ( i=0 ; i<num_voxels ; i++ ) cutdata1[i]=voxdataflat[i]=0;
		float ymax = ACy;
		if (ymax > yDim) {
         ymax = yDim;
      }
		//%MakePlane (cutdata1, lp.Hem3, float (ACx), 1.0, 
		//%		ymax, 1.0, float (ACz), -20.0, 2.0, xdim,ydim,zdim);
		//%write_minc ("CutForHindbrain.mnc", cutdata1, xdim, ydim, zdim);   
      VolumeFile cutdata1(wmtnedata);
      cutdata1.setAllVoxels(0.0);
      cutdata1.makePlane(Hem3, ACx, 1.0, 
		                   ymax, 1.0, ACz, -20.0, 2.0);
      voxdataflat = cutdata1;   // 10mar05
      writeDebugVolume(&cutdata1, "CutForHindbrain");
      // 10mar05 voxdataflat.setAllVoxels(0.0);

		//4 MaskVol.py CutForHindbrain.mnc CutForHindbrain.mask xMedLimit_20_low xMedLimit_20_high 0 nrow 0 `expr ACz + 10
		int extent[6];
		extent[0] = xMedLimit_20_low;
		extent[1] = xMedLimit_20_high;
		extent[2] = 0;
		extent[3] = yDim;
		extent[4] = 0;
		extent[5] = ACz + 10;
		cutdata1.maskVolume(extent);
      cutdata1.stretchVoxelValues();

		//11/11/2002 - DVE: add lower, wider cut x2+10, z2=ACz
		extent[1] = xMedLimit_20_high + 10;
		extent[5] = ACz;
		voxdataflat.maskVolume(extent);
      voxdataflat.stretchVoxelValues();

		//%CombineVols ("OR", cutdata1, voxdataflat, voxdataflat, xdim, ydim, zdim);
		//end 11/11/2002 additions
		//%write_minc ("CutForHindbrain.mask.mnc", cutdata1, xdim, ydim, zdim); 
      VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_OR,
                                                  &cutdata1,
                                                  &voxdataflat,
                                                  &voxdataflat,
                                                  &cutdata1); 
      cutdata1.stretchVoxelValues();
      writeDebugVolume(&cutdata1, "CutForHindbrain.mask");

		//03/16/2001: DVE added next two steps, used later in hindbrain loop
		//MakePlane.py 0 0 0 0 1 ACz 0 10  ../fname HorizontalCut
		//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=0;
		//%MakePlane (voxdataflat, 0.0, 0.0, 0.0, 0.0, 1.0, float(ACz), 0.0, 10.0, xdim,ydim,zdim);
		//%write_minc ("HorizontalCut.mnc", voxdataflat, xdim, ydim, zdim);   
      voxdataflat.setAllVoxels(0.0);
      voxdataflat.makePlane(0.0, 0.0, 0.0, 0.0, 1.0, ACz, 0.0, 10.0);
      writeDebugVolume(&voxdataflat, "HorizontalCut");

		//MaskVol.py HorizontalCut.mnc ContralateralCutforHB [limits below]
		extent[0] = xMedLimit_low;
		extent[1] = xMedLimit_high;
		extent[2] = ACy - 40;
		extent[3] = ACy;
		extent[4] = 0;
		extent[5] = ACz + 10;
		voxdataflat.maskVolume(extent);
      voxdataflat.stretchVoxelValues();
		//%write_minc ("ContralateralCutforHB.mnc", voxdataflat, xdim, ydim, zdim);  
      writeDebugVolume(&voxdataflat, "ContralateralCutforHB");

		// 03/16/2001: DVE added next step (Todd twin contralateral issue)
		//%CombineVols ("OR", cutdata1, voxdataflat, voxdataflat, xdim, ydim, zdim);
		//%write_minc ("CutForHindbrain.both.mnc", cutdata1, xdim, ydim, zdim); 
      VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_OR,
                                                  &cutdata1,
                                                  &voxdataflat,
                                                  &voxdataflat,
                                                  &cutdata1);
      cutdata1.stretchVoxelValues();
      writeDebugVolume(&cutdata1, "CutForHindbrain.both");
      
		for (int i = 1 ; i < 9 ; i++) {
         if (DebugControl::getDebugOn()) {
            std::cout << "Thresh Loop: " << i << std::endl;
         }
         
			const float WMhiThreshTemp = wmThresh + 4 * (i - 2);
			if ( WMhiThreshTemp > 255.0) {
				std::cout << "REACHED THRESH LIMIT BEFORE HINDBRAIN DISCONNECTED" << std::endl;
            throw BrainModelAlgorithmException("REACHED THRESH LIMIT BEFORE HINDBRAIN DISCONNECTED");
			}
			//%for ( j=0 ; j<num_voxels ; j++ ) voxdataflat[j]=inputdata[j];
			//%ThresholdVolume (voxdataflat, int(WMhiThreshTemp), xdim,ydim,zdim);
         voxdataflat = inputData;
         voxdataflat.thresholdVolume(static_cast<int>(WMhiThreshTemp));
         
			//CombineVols.py subrect WM.HiThreshTemp.mnc CutForHindbrain.mask.mnc WMthresh_HindbrainCut
			//%CombineVols ("subrect", voxdataflat, cutdata1, cutdata1, xdim, ydim, zdim);
			//%write_minc ("WMthresh_HindbrainCut.mnc", voxdataflat, xdim, ydim, zdim);  
         VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                                                     &voxdataflat,
                                                     &cutdata1,
                                                     &cutdata1,
                                                     &voxdataflat);
         voxdataflat.stretchVoxelValues();
         std::ostringstream str;
         str.str("");
         str << "WMthresh_HindbrainCut"
             << i;
         writeDebugVolume(&voxdataflat, str.str().c_str());
         
			//%for ( j=0 ; j<num_voxels ; j++ ) hbdata[j]=voxdataflat[j];
			//FillBiggestObject.py WMthresh_HindbrainCut.mnc Hindbrain.TestFlood.mnc xMedLimit_20_low xMedLimit_20_high 0 ACy 0 `expr ACz/2
			//%seed[0]=seed[1]=seed[2]=0;
			//%FindBiggestObjectWithinMask (hbdata, xdim, ydim, zdim, lp.xMedLimit_20_low, lp.xMedLimit_20_high, ACy-70, ACy, 0, ACz/2, seed);
			//%if (( seed[0] == -1 ) || ( seed[1] == -1 ) || ( seed[2] == -1 )) {
			//%	printf("NO HINDBRAIN FOUND\n");
			//%	hbnull=TRUE;
			//%	delete [] cutdata1;
			//%	break;
			//%}
			//%vtkFloodFill (seed, hbdata, 255, 255, 0, xdim, ydim, zdim);
			//%write_minc ("Hindbrain.TestFlood.mnc", hbdata, xdim, ydim, zdim);
         hbdata = voxdataflat;
         VoxelIJK seedVoxel(0, 0, 0);
         hbdata.findBiggestObjectWithinMask(xMedLimit_20_low,
                                             xMedLimit_20_high, 
                                             ACy - 70, 
                                             ACy, 
                                             0, 
                                             ACz / 2, 
                                             255.0,
                                             255.0,
                                             seedVoxel);
         if ((seedVoxel.getI() == -1) ||
             (seedVoxel.getJ() == -1) ||
             (seedVoxel.getK() == -1)) {
				if (DebugControl::getDebugOn()) {
               std::cout << "NO HINDBRAIN FOUND" << std::endl;
            }
				hbnull = true;
				break;
			}
         hbdata.floodFillWithVTK(seedVoxel, 255, 255, 0);
         str.str("");
         str << "Hindbrain.TestFlood"
             << i;
         writeDebugVolume(&hbdata, str.str().c_str());
         
			//FindLimits ncol nrow nslices Hindbrain.TestFlood.mnc.raw
			//%FindLimits (xdim, ydim, zdim, hbdata, "Hindbrain.TestFlood.limits", extent);
         str.str("");
         str << "Hindbrain.TestFlood."
             << i
             << ".limits";
         hbdata.findLimits(str.str().c_str(), extent);
			if ( extent[5] < ACz+20 ) {
            if (DebugControl::getDebugOn()) {
               std::cout << "HINDBRAIN DISCONNECTED FROM CEREBRUM" << std::endl;
            }
				//%delete [] cutdata1;
				//%delete [] inputdata;
				//%write_minc ("Hindbrain.Flood.mnc", hbdata, xdim, ydim, zdim);
            writeDebugVolume(&hbdata, "Hindbrain.Flood");
            if (hindbrainFloodVolume != NULL) {
               delete hindbrainFloodVolume;
            }
            hindbrainFloodVolume = new VolumeFile(hbdata);
            
				//Sculpt.py  0 3 xMedLimit_20_low xMedLimit_20_high 0 ACy 0 ACz Hindbrain.Flood.mnc WM.thresh_noEye.mnc Hindbrain_sculpt
				//%seed[0]=seed[1]=seed[2]=0;
            VoxelIJK voxelSeed(0, 0, 0);
				extent[0] = xMedLimit_20_low;
				extent[1] = xMedLimit_20_high;
				extent[2] = 0;
				extent[3] = ACy;
				extent[4] = 0;
				extent[5] = ACz;
				//%Sculpt (0, 3, seed, extent, hbdata, wmtnedata, xdim, ydim, zdim);
				//%write_minc ("Hindbrain_sculpt.mnc", hbdata, xdim, ydim, zdim);
            int seedTemp[3];
            voxelSeed.getIJK(seedTemp);
            hbdata.sculptVolume(0, &wmtnedata, 3, seedTemp, extent);
            hbdata.stretchVoxelValues();
            writeDebugVolume(&hbdata, "Hindbrain_sculpt");
            
				//VolMorphOps.py 2 1 Hindbrain_sculpt.mnc Hindbrain_sculpt_DilErode
		      //%		DoVolMorphOps (xdim, ydim, zdim, hbdata, 2, 1);
		      //%		write_minc ("Hindbrain_sculpt_DilErode.mnc", hbdata, xdim, ydim, zdim);
            hbdata.doVolMorphOps(2, 1);
            writeDebugVolume(&hbdata, "Hindbrain_sculpt_DilErode");
				break;
			}
         
			//%if ( i==8 ) { 
			//%	printf ("ERROR IN HINDBRAIN SEGMENTATION\n");
			//%	delete [] cutdata1;
			//%	return(EXIT_FAILURE);
			//%}
         if (i == 8) {
            std::cout << "ERROR IN HINDBRAIN SEGMENTATION" << std::endl;
            throw BrainModelAlgorithmException("ERROR IN HINDBRAIN SEGMENTATION");
         }
		}
	} 
   else {
		hbnull = true;
	}
	if (hbnull) {
      if (DebugControl::getDebugOn()) {
         std::cout << "SETTING HINDBRAIN TO NULL VOLUME" << std::endl;
      }
      //%for ( i=0 ; i<num_voxels ; i++ ) hbdata[i]=0;
      //%write_minc ("Hindbrain.Flood.mnc", hbdata, xdim, ydim, zdim);
      //%write_minc ("Hindbrain_sculpt.mnc", hbdata, xdim, ydim, zdim);
      //%write_minc ("Hindbrain_sculpt_DilErode.mnc", hbdata, xdim, ydim, zdim);
      hbdata.setAllVoxels(0.0);
      writeDebugVolume(&hbdata, "Hindbrain.Flood");
      if (hindbrainFloodVolume != NULL) {
         delete hindbrainFloodVolume;
      }
      hindbrainFloodVolume = new VolumeFile(hbdata);
      writeDebugVolume(&hbdata, "Hindbrain_sculpt");
      writeDebugVolume(&hbdata, "Hindbrain_sculpt_DilErode");
      
      //Thresh.py ../fname WM.thresh.mnc WMThresh
      //%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=inputdata[i];
      //%delete [] inputdata;
      // cp WhiteMatter.Thresholded.mnc WMthresh_HindbrainCut.mnc
      //%ThresholdVolume (voxdataflat, int(WMThresh), xdim,ydim,zdim);
      //%write_minc ("WMthresh_HindbrainCut.mnc", voxdataflat, xdim, ydim, zdim);   
      voxdataflat = inputData;
      voxdataflat.thresholdVolume(static_cast<int>(wmThresh));
      writeDebugVolume(&voxdataflat, "WMthresh_HindbrainCut");
	}
   
	//CombineVols.py subrect WMthresh_HindbrainCut.mnc EyeFat.sculpt.mnc WMHiThresh_noEye_noHB
	//%unsigned char* efsdata=new unsigned char [num_voxels];
	//%if ( lp.skipeye ) {for ( i=0 ; i<num_voxels ; i++ ) efsdata[i]=0;}
	//%else {read_minc_file ("EyeFat.sculpt.mnc", efsdata);}
	//%CombineVols ("OR", voxdataflat, efsdata, efsdata, xdim, ydim, zdim);
	//%write_minc ("WMHiThresh_noEye_noHB.mnc", voxdataflat, xdim, ydim, zdim);
	//%delete [] efsdata;
   if (disconnectEyeFlag == false) {
      if (eyeFatSculptVolume == NULL) {
         eyeFatSculptVolume = new VolumeFile(voxdataflat);
      }
      eyeFatSculptVolume->setAllVoxels(0.0);
   }
   //
   // Added on 8/9/05 per Donna's Au 8, 2005 2:45PM email
   //
   if (disconnectHindBrainHiThreshFlag) {
      VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,  // 10mar2005
                                               &voxdataflat,
                                               eyeFatSculptVolume,
                                               eyeFatSculptVolume,
                                               &voxdataflat);
   }
   else {
      if (disconnectEyeFlag) {
         VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,  // 10mar2005
                                                  whiteMatterThreshNoEyeVolume,
                                                  hindbrainFloodVolume,
                                                  hindbrainFloodVolume,
                                                  &voxdataflat);
      }
      else {
         VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,  // 10mar2005
                                                  &wmtnedata,
                                                  hindbrainFloodVolume,
                                                  hindbrainFloodVolume,
                                                  &voxdataflat);
      }
   }
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(&voxdataflat, "WMHiThresh_noEye_noHB");
   
   freeVolumeInMemory(eyeFatSculptVolume);
   freeVolumeInMemory(hindbrainFloodVolume);
   freeVolumeInMemory(whiteMatterThreshNoEyeVolume);
   
	//FillBiggestObject.py WMHiThresh_noEye_noHB.mnc WMHiThresh_noEye_noHB.final xMedLimit_20_low xMedLimit_20_high `expr ACy - 20` `expr ACy + 40` ACz `expr ACz + 30
	//12/8/2000 DVE: change y1 to `expr min[ACy, ymin] - 20`
	//%seed[0]=seed[1]=seed[2]=0;
	//%int lowy=ACy;
	//%if (lp.Ymin < ACy) lowy=lp.Ymin;
	//%FindBiggestObjectWithinMask (voxdataflat, xdim, ydim, zdim, lp.xMedLimit_20_low, lp.xMedLimit_20_high, lowy-20, ACy+40, ACz, ACz+30, seed);
	//%vtkFloodFill (seed, voxdataflat, 255, 255, 0, xdim, ydim, zdim);
	//%write_minc ("WMHiThresh_noEye_noHB.final.mnc", voxdataflat, xdim, ydim, zdim);
   VoxelIJK voxelSeed(0, 0, 0);
   int lowy = ACy;
   if (15 < ACy) {
      lowy = 15;
   } 
//   if (yMin < ACy) {
//      lowy = yMin;
//   }
   voxdataflat.findBiggestObjectWithinMask(xMedLimit_20_low, xMedLimit_20_high, 
                                           lowy-20, ACy+40, ACz, ACz+30, 
                                           255.0, 255.0,
                                           voxelSeed);
   if (voxelSeed.getI() < 0) {
      throw BrainModelAlgorithmException(
         "findBiggestObjectWithinMask() failed to find biggest object when\n"
         "trying to create intermediate volume \"WMHiThresh_noEye_noHB.final\".");
   }
   voxdataflat.floodFillWithVTK(voxelSeed, 255, 255, 0);
   writeDebugVolume(&voxdataflat, "WMHiThresh_noEye_noHB.final");

	//05/15/2001: Sculpt.py 0 3 $xAClow $xAChigh 0 $ydim 0 $zdim WMHiThresh_noEye_noHB.final.mnc WM.thresh_noEye.mnc CerebralWM_FirstSculpt.mnc
   int extent[6];
	extent[0] = xAClow;
	extent[1] = xAChigh;
	extent[2] = 0;
	extent[3] = yDim;
	extent[4] = 0;
	extent[5] = zDim;
	//%unsigned char* cwmfsdata=new unsigned char [num_voxels];
	//%for ( i=0 ; i<num_voxels ; i++ ) cwmfsdata[i]=voxdataflat[i];
	//%Sculpt (0, 3, seed, extent, cwmfsdata, wmtnedata, xdim, ydim, zdim);
	//%write_minc ("CerebralWM_FirstSculpt.mnc", cwmfsdata, xdim, ydim, zdim);
   VolumeFile cwmfsdata(voxdataflat);
   int seedTemp[3];
   voxelSeed.getIJK(seedTemp);
   cwmfsdata.sculptVolume(0, &wmtnedata, 3, seedTemp, extent);
   voxelSeed.setIJK(seedTemp);
   cwmfsdata.stretchVoxelValues();
   writeDebugVolume(&cwmfsdata, "CerebralWM_FirstSculpt");
   
   
	//Sculpt.py 0 12 $xAClow $xAChigh 0 $ydim `expr $ACz + 20` $zdim WMHiThresh_noEye_noHB.final.mnc WM.thresh_noEye.mnc CerebralWM_ExtraDorsalsculpt
	extent[4]=ACz+20;
	//%Sculpt (0, 12, seed, extent, voxdataflat, wmtnedata, xdim, ydim, zdim);
	//%write_minc ("CerebralWM_ExtraDorsalsculpt.mnc", voxdataflat, xdim, ydim, zdim);
   voxelSeed.getIJK(seedTemp);
   voxdataflat.sculptVolume(0, &wmtnedata, 12, seedTemp, extent);
   voxelSeed.setIJK(seedTemp);
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(&voxdataflat, "CerebralWM_ExtraDorsalsculpt");
   
	//11 JULY 02 Sculpt.py 0 10 `expr $xAClow - 10` `expr $xAChigh + 10`  0 $nrow 0 $nslices CerebralWM_FirstSculpt.mnc WM.thresh_noEye.mnc CerebralWM_FirstSculpt_MedialSculpt.mnc
	extent[0] = xAClow-10;
	extent[1] = xAChigh+10;
	extent[2] = 0;
	extent[3] = yDim;
	extent[4] = 0;
	extent[5] = zDim;
	//%Sculpt (0, 10, seed, extent, cwmfsdata, wmtnedata, xdim, ydim, zdim);
	//%write_minc ("CerebralWM_FirstSculpt_MedialSculpt.mnc", cwmfsdata, xdim, ydim, zdim);
	//%delete [] wmtnedata;
   voxelSeed.getIJK(seedTemp);
   cwmfsdata.sculptVolume(0, &wmtnedata, 10, seedTemp, extent);
   voxelSeed.setIJK(seedTemp);
   cwmfsdata.stretchVoxelValues();
   writeDebugVolume(&cwmfsdata, "CerebralWM_FirstSculpt_MedialSculpt");
   
	//CombineVols.py OR CerebralWM_ExtraDorsalsculpt.mnc CerebralWM_FirstSculpt.mnc CerebralWM_sculpt.mnc
	//%CombineVols ("OR", voxdataflat, cwmfsdata, cwmfsdata, xdim, ydim, zdim);
	//%write_minc ("CerebralWM_sculpt.mnc", voxdataflat, xdim, ydim, zdim);
	//%delete [] cwmfsdata;
   VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_OR,
                                               &voxdataflat,
                                               &cwmfsdata,
                                               &cwmfsdata,
                                               &voxdataflat);
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(&voxdataflat, "CerebralWM_sculpt");
   
	//CombineVols.py subrect CerebralWM_sculpt.mnc Hindbrain_sculpt_DilErode.mnc CerebralWM_noBstem
	//%CombineVols ("subrect", voxdataflat, hbdata, hbdata, xdim, ydim, zdim);
	//%write_minc ("CerebralWM_noBstem.fill.mnc", voxdataflat, xdim, ydim, zdim);
	//%delete [] hbdata;
	//%delete [] voxdataflat;
   VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                                               &voxdataflat,
                                               &hbdata,
                                               &hbdata,
                                               &voxdataflat);
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(&voxdataflat, "CerebralWM_noBstem.fill");
   cerebralWmNoBstemFill = new VolumeFile(voxdataflat);
   
	if (DebugControl::getDebugOn()) {
      std::cout << "#### DisconnectHindbrain done ####" << std::endl;
   }
}      

/**
 * generate the corpus callosum slice (assumes AC at center)
 * estimate white matter peak if invalid.
 */
void 
BrainModelVolumeSureFitSegmentation::generateCorpusCallosumSlice(VolumeFile& anatomyVolumeFileIn,
                                        VolumeFile& corpusCallosumVolumeFileOut,
                                        const Structure& structure,
                                        const float grayMatterPeakIn,
                                        const float whiteMatterPeakIn,
                                        const bool looseMask) throw (BrainModelAlgorithmException)
{
   float grayMatterPeak  = grayMatterPeakIn;
   float whiteMatterPeak = whiteMatterPeakIn;
   const float acXYZ[3] = { 0.0, 0.0, 0.0 };
   float voxMin, voxMax;
   anatomyVolumeFileIn.getMinMaxVoxelValues(voxMin, voxMax);
   int dimExtent[6];
   float coordExtent[6];
   int i, j, k, count, total;
   int acIJK[3];
   bool redo;
   if ((grayMatterPeak <= 0) ||
       (whiteMatterPeak <= 0)) {
      const StatisticHistogram* hist = anatomyVolumeFileIn.getHistogram();
      int grayPeakBucketNumber, whitePeakBucketNumber,
          grayMinimumBucketNumber, whiteMaximumBucketNumber,
          grayWhiteBoundaryBucketNumber, csfPeakBucketNumber;
      hist->getGrayWhitePeakEstimates(grayPeakBucketNumber, whitePeakBucketNumber,
                                      grayMinimumBucketNumber, whiteMaximumBucketNumber,
                                      grayWhiteBoundaryBucketNumber, csfPeakBucketNumber);
      if (grayMatterPeak <= 0) {
         grayMatterPeak  = hist->getDataValueForBucket(grayPeakBucketNumber);
      }
      if (whiteMatterPeak <= 0) {
         whiteMatterPeak = hist->getDataValueForBucket(whitePeakBucketNumber);
      }
      delete hist;
   }
   
   int redoCounter = 0;
   float threshhold = (grayMatterPeak + whiteMatterPeak) * 0.5;
   do
   {
      redo = false;
      redoCounter++;
      if (DebugControl::getDebugOn()) {
         std::cout << "Redo " << redoCounter
                   << ": Gray: " << grayMatterPeak
                   << " White: " << whiteMatterPeak
                   << " Threshold: " << threshhold
                   << std::endl;
      }
      
      corpusCallosumVolumeFileOut = anatomyVolumeFileIn;
      corpusCallosumVolumeFileOut.thresholdVolume(threshhold);
   
      corpusCallosumVolumeFileOut.convertCoordinatesToVoxelIJK(acXYZ, acIJK);
      
      int Hem = 0;
      if (structure.getType() == Structure::STRUCTURE_TYPE_CORTEX_RIGHT) {
         Hem = 1;
      }
      const int Hem1   = Hem;
      const int Hem2   = 1 - Hem1;
      const int HemDbl = 2 * Hem1;
      const int Hem3   = HemDbl - 1;
      const int xAC_1 = acIJK[0] + Hem3 * 1;
      const int xAC_1_low  = xAC_1 * Hem2 + acIJK[0] * Hem1;
      const int xAC_1_high = xAC_1 * Hem1 + acIJK[0] * Hem2;
      
      //MaskVol.py WM.thresh.mnc CC.slice xAC_1_low xAC_1_high `expr ACy - 50` `expr ACy + 40` ACz `expr ACz + 40
      int extent[6];
      extent[0] = xAC_1_low;
      extent[1] = xAC_1_high;
      extent[2] = acIJK[1] - 50;
      extent[3] = acIJK[1] + 40;
      extent[4] = acIJK[2];
      extent[5] = acIJK[2] + 40;
      //%MaskVolume (ccdata, xdim, ydim, zdim, extent);
      //%write_minc ("CC.slice.mnc", ccdata, xdim, ydim, zdim);
      if (looseMask)
      {/* allow 711-2* origin to not be exactly on the AC */
         extent[2] -= 10;
         extent[3] += 10;
         extent[4] -= 10;
         extent[5] += 10;
      }

      if (DebugControl::getDebugOn()) {
         try {
            QString name = "CC_BeforeMask_" + QString::number(redoCounter) + ".nii.gz";
            corpusCallosumVolumeFileOut.setDescriptiveLabel(name);
            corpusCallosumVolumeFileOut.writeFile(name);
         }
         catch (FileException e) {
         }
      }
      corpusCallosumVolumeFileOut.maskVolume(extent);
      corpusCallosumVolumeFileOut.stretchVoxelValues();
      if (DebugControl::getDebugOn()) {
         try {
            QString name = "CC_AfterMask_" + QString::number(redoCounter) + ".nii.gz";
            corpusCallosumVolumeFileOut.setDescriptiveLabel(name);
            corpusCallosumVolumeFileOut.writeFile(name);
         }
         catch (FileException e) {
         }
      }
   
                 
      //FillBiggestObject.py CC.slice.mnc CC.slice.fill.mnc xAC_1_low xAC_1_high  `expr ACy - 50` `expr ACy + 40` ACz `expr ACz + 40
      //%FindBiggestObjectWithinMask (ccdata, xdim, ydim, zdim, extent[0],extent[1],extent[2],extent[3],extent[4],extent[5], seed);
      //%vtkFloodFill (seed, ccdata, 255, 255, 0, xdim, ydim, zdim);
      //%write_minc ("CorpusCallosumSlice.mnc", ccdata, xdim, ydim, zdim);
   
      VoxelIJK voxelSeed(0, 0, 0);
      corpusCallosumVolumeFileOut.findBiggestObjectWithinMask(extent, 255, 255, voxelSeed);
      if (voxelSeed.getI() < 0) {
         if (grayMatterPeakIn > 0.0f && whiteMatterPeakIn > 0.0f)
         {
            throw BrainModelAlgorithmException(
               "findBiggestObjectWithinMask() failed to find biggest object when\n"
               "trying to create intermediate volume \"CorpusCallosumSlice\".");
         } else {
            //cout << "threshhold " << threshhold << ": findBiggestObjectWithinMask() failed" << endl;
            voxMax = threshhold;
            redo = true;
         }
      }
      if (!redo)
      {
         corpusCallosumVolumeFileOut.floodFillWithVTK(voxelSeed, 255, 255, 0);
         if (DebugControl::getDebugOn()) {
            try {
               QString name = "CC_FloodFill_" + QString::number(redoCounter) + ".nii.gz";
               corpusCallosumVolumeFileOut.setDescriptiveLabel(name);
               corpusCallosumVolumeFileOut.writeFile(name);
            }
            catch (FileException e) {
            }
         }
         corpusCallosumVolumeFileOut.setDescriptiveLabel("CorpusCallosumSlice");
         
         //
         // Get extent of volume
         //
         corpusCallosumVolumeFileOut.getNonZeroVoxelExtent(dimExtent,
                                                    coordExtent);//doesn't use normal "one-after" array size conventions
         if (dimExtent[0] == -1)
         {
            //cout << "threshhold " << threshhold << ": no nonzero extent found" << endl;
            voxMax = threshhold;
            redo = true;
         }
      }
      if (!redo && fabs(coordExtent[3] - coordExtent[2]) < 50.0f)
      {
         //cout << "threshhold " << threshhold << ": y coord extent too small" << endl;
         voxMax = threshhold;
         redo = true;
      }
      if (!redo && fabs((coordExtent[3] - coordExtent[2]) * (coordExtent[5] - coordExtent[4])) > 2900.0f)
      {
         //cout << "threshhold " << threshhold << ": yz extent too large" << endl;
         voxMin = threshhold;
         redo = true;
      }
      if (!redo && fabs(coordExtent[5] - coordExtent[4]) > 45.0f)
      {//can only be tripped when looseMask is true
         //cout << "threshhold " << threshhold << ": z coord extent too big" << endl;
         voxMin = threshhold;
         redo = true;
      }
      if (!redo)
      {
         i = (dimExtent[0] + dimExtent[1]) / 2;//only need to look at the middle slice
         count = 0;
         total = (dimExtent[3] - dimExtent[2] + 1) * (dimExtent[5] - dimExtent[4] + 1);
         for (j = dimExtent[2]; j <= dimExtent[3]; ++j)
         {
            for (k = dimExtent[4]; k <= dimExtent[5]; ++k)
            {
               if (corpusCallosumVolumeFileOut.getVoxel(i, j, k) > 0.0f)
               {
                  ++count;
               }
            }
         }
         //cout << "threshhold " << threshhold << ": " << count << " / " << total;
         if (count < total * 0.25f)
         {
            //cout << ", too few voxels" << endl;
            voxMax = threshhold;
            redo = true;
         } else {
            if (count > total * 0.55f)
            {
               //cout << ", too many voxels" << endl;
               voxMin = threshhold;
               redo = true;
            } else {
               //cout << ", acceptable" << endl;
            }
         }
      }
      threshhold = (voxMin + voxMax + threshhold) / 3.0f;//its like bisection search, except it moves slower so it doesnt try crazy values
      if (redo && voxMax != 0.0f && voxMin / voxMax > 0.98f)
      {//warning: give up check expects voxels to have minimum of 0
         //cout << "giving up" << endl;
         redo = false;
      }
   } while (grayMatterPeakIn <= 0 && whiteMatterPeakIn <= 0 && redo);
}

/**
 * cut the corpus callossum.
 */
void 
BrainModelVolumeSureFitSegmentation::cutCorpusCallossum() throw (BrainModelAlgorithmException)
{
	//%printf("#### CutCorpusCallossum begin ####\n");
   if (DebugControl::getDebugOn()) {
      std::cout << "#### CutCorpusCallossum begin ####" << std::endl;
   }
   
	//%int i, j;
	//%int seed[3]={0,0,0};
	//%int extent[6]={0,0,0,0,0,0};
	//%int xdim=lp.xdim;
	//%int ydim=lp.ydim;
	//%int zdim=lp.zdim;
	//%int num_voxels=xdim*ydim*zdim;
	//%int ACy=lp.ACy;
	//%int ACz=lp.ACz;
	//%float WMThresh=lp.WMThresh;
	//%unsigned char* ccdata=new unsigned char [num_voxels];
	//%read_minc_file (mincfile, ccdata);
	//Thresh.py ../$fname WM.thresh.mnc WMThresh
	//%ThresholdVolume (ccdata, int(WMThresh), xdim,ydim,zdim);

   VolumeFile ccdata = *anatomyVolume;
   ccdata.thresholdVolume(static_cast<int>(wmThresh));

   //
   // Were both eye removal and hindbrain removal skipped ?
   //
   if ((disconnectEyeFlag == false) &&
       (disconnectHindBrainFlag == false)) {
      cerebralWmNoBstemFill = new VolumeFile(ccdata); 
   }
   
	//MaskVol.py WM.thresh.mnc CC.slice xAC_1_low xAC_1_high `expr ACy - 50` `expr ACy + 40` ACz `expr ACz + 40
              int extent[6];
	extent[0] = xAC_1_low;
	extent[1] = xAC_1_high;
	extent[2] = acIJK[1] - 50;
	extent[3] = acIJK[1] + 40;
	extent[4] = acIJK[2];
	extent[5] = acIJK[2] + 40;
   if (DebugControl::getDebugOn()) {
	   std::cout << "CC LIMITS: "
                << extent[0] << "-" << extent[1] << "-"
                << extent[2] << "-" << extent[3] << "-"
                << extent[4] << "-" << extent[5] << std::endl; 
   }
	//%MaskVolume (ccdata, xdim, ydim, zdim, extent);
	//%write_minc ("CC.slice.mnc", ccdata, xdim, ydim, zdim);
   ccdata.maskVolume(extent);
   ccdata.stretchVoxelValues();
   writeDebugVolume(&ccdata, "CC.slice");

              
	//FillBiggestObject.py CC.slice.mnc CC.slice.fill.mnc xAC_1_low xAC_1_high  `expr ACy - 50` `expr ACy + 40` ACz `expr ACz + 40
	//%FindBiggestObjectWithinMask (ccdata, xdim, ydim, zdim, extent[0],extent[1],extent[2],extent[3],extent[4],extent[5], seed);
	//%vtkFloodFill (seed, ccdata, 255, 255, 0, xdim, ydim, zdim);
	//%write_minc ("CorpusCallosumSlice.mnc", ccdata, xdim, ydim, zdim);

   VoxelIJK voxelSeed(0, 0, 0);
   ccdata.findBiggestObjectWithinMask(extent, 255, 255, voxelSeed);
   if (voxelSeed.getI() < 0) {
      throw BrainModelAlgorithmException(
         "findBiggestObjectWithinMask() failed to find biggest object when\n"
         "trying to create intermediate volume \"CorpusCallosumSlice\".");
   }
   ccdata.floodFillWithVTK(voxelSeed, 255, 255, 0);
   //writeDebugVolume(&ccdata, "CorpusCallosumSlice");
   
   //
   // Now save the corpus callosum slice volume (30 July 2007)
   // which will be used for sulcal ID
   //
   {
      QString fileNameWritten, dataFileNameWritten;
      ccdata.setDescriptiveLabel("CorpusCallosumSlice");
      VolumeFile::writeVolumeFile(&ccdata,
                               typeOfVolumeFilesToWrite,
                               "CorpusCallosumSlice",
                               false,
                               fileNameWritten,
                               dataFileNameWritten);
   }

	//%FindLimits (xdim, ydim, zdim, ccdata, "CC.slice.fill.limits", extent);
   ccdata.findLimits("CC.slice.fill.limits", extent);

	// TO-DO: Write CC* values to params file
	int CCpost=extent[2];
	int CCant=extent[3];
	int CCvent=extent[4];
	int CCdors=extent[5];
	//%printf("CCpost=%d ; CCant=%d ; CCvent=%d ; CCdors=%d\n", CCpost, CCant, CCvent, Ccdors);
   if (DebugControl::getDebugOn()) {
      std::cout << "CCpost=" << CCpost << " ; "
                 << "CCant=" << CCant << " ; "
                 << "CCvent=" << CCvent << " ; "
                 << "CCdors=" << CCdors << std::endl;
   }
   
	//24 VolMorphOps.py 3 0 CerebralWM_noBstem.fill.mnc CerebralWM_noBstem.dilate.mnc
	//%unsigned char* cwmnbsfdata=new unsigned char [num_voxels];
	//%unsigned char* cwmnbsfdilatedata=new unsigned char [num_voxels];
	//%read_minc_file ("CerebralWM_noBstem.fill.mnc", cwmnbsfdata);
	//%for ( i=0 ; i<num_voxels ; i++ ) cwmnbsfdilatedata[i]=cwmnbsfdata[i];
	//%DoVolMorphOps (xdim, ydim, zdim, cwmnbsfdilatedata, 3, 0);
	//%write_minc ("CerebralWM_noBstem.dilate.mnc", cwmnbsfdilatedata, xdim, ydim, zdim);
   VolumeFile cwmnbsfdata = *cerebralWmNoBstemFill;
   VolumeFile cwmnbsfdilatedata = cwmnbsfdata;
   cwmnbsfdilatedata.doVolMorphOps(3, 0);
   writeDebugVolume(cwmnbsfdilatedata, "CerebralWM_noBstem.dilate");

	//25 MaskVol.py CerebralWM_noBstem.dilate.mnc CCslab.1 xAC_1_low xAC_1_high `expr CCpost - 4` `expr CCant + 4` `expr ACz - 6` `expr CCdors + 3
	extent[0] = xAC_1_low;
	extent[1] = xAC_1_high;
	extent[2] = CCpost - 4;
	extent[3] = CCant + 4;
	extent[4] = acIJK[2] - 6;
	extent[5] = CCdors + 3;
	//%for ( i=0 ; i<num_voxels ; i++ ) ccdata[i]=cwmnbsfdilatedata[i];
	//%MaskVolume (ccdata, xdim, ydim, zdim, extent);
	//%write_minc ("CCslab.1.mnc", ccdata, xdim, ydim, zdim);
   ccdata = cwmnbsfdilatedata;
   ccdata.maskVolume(extent);
   ccdata.stretchVoxelValues();
   writeDebugVolume(ccdata, "Ccslab.1");


	//26 MaskVol.py CerebralWM_noBstem.dilate.mnc CCslab.2 xAC_1_low xAC_1_high `expr CCpost - 4` `expr ACy + 10` 0 `expr CCdors + 3
	extent[0] = xAC_1_low;
	extent[1] = xAC_1_high;
	extent[2] = CCpost - 4;
	extent[3] = acIJK[1] + 10;
	extent[4] = 0;
	extent[5] = CCdors + 3;
	//%unsigned char* cc2data=new unsigned char [num_voxels];
	//%for ( i=0 ; i<num_voxels ; i++ ) cc2data[i]=cwmnbsfdilatedata[i];
	//%MaskVolume (cc2data, xdim, ydim, zdim, extent);
	//%write_minc ("CCslab.2.mnc", cc2data, xdim, ydim, zdim);
   VolumeFile cc2data = cwmnbsfdilatedata;
   cc2data.maskVolume(extent);
   cc2data.stretchVoxelValues();
   writeDebugVolume(cc2data, "Ccslab.2");

	//%delete [] cwmnbsfdilatedata;
	//27 CombineVols.py OR CCslab.1.mnc CCslab.2.mnc CCslab.notched
	//%CombineVols ("OR", ccdata, cc2data, cc2data, xdim, ydim, zdim);
	//%write_minc ("CCslab.notched.mnc", ccdata, xdim, ydim, zdim);
   VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_OR,
                          &ccdata,
                          &cc2data,
                          &cc2data,
                          &ccdata);
   ccdata.stretchVoxelValues();
   writeDebugVolume(ccdata, "Ccslab.notched");

	//28 CombineVols.py subrect CerebralWM_noBstem.fill.mnc CCslab.notched.mnc CerebralWM.CCcut.mnc
	//%CombineVols ("subrect", cwmnbsfdata, ccdata, ccdata, xdim, ydim, zdim);
	//%write_minc ("CerebralWM.CCcut.mnc", cwmnbsfdata, xdim, ydim, zdim);
   VolumeFile::performMathematicalOperation(
            VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
            &cwmnbsfdata,
            &ccdata,
            &ccdata,
            &cwmnbsfdata);
   cwmnbsfdata.stretchVoxelValues();
   writeDebugVolume(cwmnbsfdata, "CerebralWM.CCcut");

	//%delete [] ccdata;
	//29 FillBiggestObject.py CerebralWM.CCcut.mnc CerebralWhiteMatter.mnc xAC_20_low xAC_20_high CCpost CCant ACz CCdors
	extent[0] = xAC_20_low;
	extent[1] = xAC_20_high;
	if ( CCant > CCpost) {
		extent[2] = CCpost;
		extent[3] = CCant;
	} else {
		extent[2] = 0;
		extent[3] = yDim;
	}
	if ( CCdors > acIJK[2] ) {
		extent[4] = acIJK[2];
		extent[5] = CCdors;
	} else {
		extent[4] = 0;
		extent[5] = zDim;
	}
	//%FindBiggestObjectWithinMask (cwmnbsfdata, xdim, ydim, zdim, extent[0],extent[1],extent[2],extent[3],extent[4],extent[5], seed);
	//%vtkFloodFill (seed, cwmnbsfdata, 255, 255, 0, xdim, ydim, zdim);
	//%unsigned char* cwmdata=new unsigned char [num_voxels];
	//%for ( j=0 ; j<num_voxels ; j++ ) cwmdata[j]=cwmnbsfdata[j];
	// TO-DO: Insert subject prefix from params
	//%write_minc ("CerebralWhiteMatter.mnc", cwmdata, xdim, ydim, zdim);
   cwmnbsfdata.findBiggestObjectWithinMask(extent, 255, 255, voxelSeed);
   if (voxelSeed.getI() < 0) {
      throw BrainModelAlgorithmException(
         "findBiggestObjectWithinMask() failed to find biggest object when\n"
         "trying to create intermediate volume \"CerebralWhiteMatter\".");
   }
   cwmnbsfdata.floodFillWithVTK(voxelSeed, 255, 255, 0);
   VolumeFile cwmdata = cwmnbsfdata;
   writeDebugVolume(cwmdata, "CerebralWhiteMatter");

	//30 VolMorphOps.py 0 2 CerebralWhiteMatter.mnc CerebralWM.erode.mnc
	//%DoVolMorphOps (xdim, ydim, zdim, cwmnbsfdata, 0, 2);
	//%write_minc ("CerebralWM.erode.mnc", cwmnbsfdata, xdim, ydim, zdim);
	//31 MakeShell.py 0 2 CerebralWhiteMatter.mnc CerebralWM.shell.mnc
	//%for ( j=0 ; j<num_voxels ; j++ ) cwmnbsfdata[j]=cwmdata[j];
	//%MakeShell (cwmnbsfdata, xdim, ydim, zdim, 0, 2);
	//%write_minc ("CerebralWM.shell.mnc", cwmnbsfdata, xdim, ydim, zdim);
   cwmnbsfdata.doVolMorphOps(0, 2);
   writeDebugVolume(&cwmnbsfdata, "CerebralWM.erode");
   cerebralWMErodeVolume = new VolumeFile(cwmnbsfdata);
   cwmnbsfdata = cwmdata;
   cwmnbsfdata.makeShellVolume(0, 2);
   cwmnbsfdata.stretchVoxelValues();
   writeDebugVolume(cwmnbsfdata, "CerebralWM.shell");

	//32 MakeShell.py 3 3 CerebralWhiteMatter.mnc InnerMask.1
	//%for ( j=0 ; j<num_voxels ; j++ ) cwmnbsfdata[j]=cwmdata[j];
	//%MakeShell (cwmnbsfdata, xdim, ydim, zdim, 3, 3);
	//%write_minc ("InnerMask.1.mnc", cwmnbsfdata, xdim, ydim, zdim);
   cwmnbsfdata = cwmdata;

   //
   // 7/24/2008 DLD added next three lines to reduce chimp occipital smearing
   //
   int dilerodes = 2;
   if (brainSet->getSpecies().isHuman()) { 
      dilerodes = 3; 
   }
   cwmnbsfdata.makeShellVolume(dilerodes, dilerodes);
   //cwmnbsfdata.makeShellVolume(3, 3);
   cwmnbsfdata.stretchVoxelValues();
   writeDebugVolume(cwmnbsfdata, "InnerMask.1");
   innerMask1Volume = new VolumeFile(cwmnbsfdata);

	//33 MakeShell.py 6 1 CerebralWhiteMatter.mnc OuterMask.1
	//%for ( j=0 ; j<num_voxels ; j++ ) cwmnbsfdata[j]=cwmdata[j];
	//%MakeShell (cwmnbsfdata, xdim, ydim, zdim, 6, 1);
	//%write_minc ("OuterMask.1.mnc", cwmnbsfdata, xdim, ydim, zdim);
   cwmnbsfdata = cwmdata;
   
   //
   // 7/24/2008 DLD added next three lines to reduce chimp occipital smearing
   //
   int dilations = 4;
   if (brainSet->getSpecies().isHuman()) {
      dilations = 6;
   }
   cwmnbsfdata.makeShellVolume(dilations, 1);
   cwmnbsfdata.stretchVoxelValues();
   writeDebugVolume(cwmnbsfdata, "OuterMask.1");
   outerMaskVolume = new VolumeFile(cwmnbsfdata);
   
	//%printf("#### CutCorpusCallossum done ####\n");
	//%delete [] cwmdata;
	//%delete [] cwmnbsfdata;
	//%printf("#### CutCorpusCallossum end ####\n");
	//%return 0;

   if (DebugControl::getDebugOn()) {
      std::cout << "#### CutCorpusCallossum done ####" << std::endl;
      std::cout << "#### CutCorpusCallossum end ####" << std::endl;
   }
}

/**
 * generate the inner boundary.
 */
void 
BrainModelVolumeSureFitSegmentation::generateInnerBoundary() throw (BrainModelAlgorithmException)
{
	//%printf("#### GenerateInnerBoundary begin ####\n");
   if (DebugControl::getDebugOn()) {
      std::cout << "#### GenerateInnerBoundary begin ####" << std::endl;
   }
   
	//%int i, extent[6];
	//%int xdim=lp.xdim;
	//%int ydim=lp.ydim;
	//%int zdim=lp.zdim;
	//%int ACy=lp.ACy;
	//%int ACz=lp.ACz;
	//%int num_voxels=xdim*ydim*zdim;
	//%float WMpeak=lp.WMpeak;
	//%float CGMpeak=lp.CGMpeak;


	//%unsigned char* inputdata=new unsigned char [num_voxels];
	//%read_minc_file (mincfile, inputdata);
	//1
	//%unsigned char* voxdataflat=new unsigned char [num_voxels];
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=inputdata[i];
   VolumeFile inputdata = *anatomyVolume;
   VolumeFile voxdataflat = inputdata;

	//%Vector vec("", xdim, ydim, zdim);
	//%Grad (1, "Intensity.grad", voxdataflat, vec.X, vec.Y, vec.Z, vec.Mag, xdim, ydim, zdim);
   SureFitVectorFile vec(xDim, yDim, zDim);
   BrainModelVolumeGradient* bmvg = new BrainModelVolumeGradient(brainSet,
                                                                 1,
                                                                 true,
                                                                 false,
                                                                 &voxdataflat,
                                                                 &voxdataflat,
                                                                 &vec);
   bmvg->execute();
   delete bmvg;
   bmvg = NULL;
   voxdataflat.stretchVoxelValues();
   gradIntensityVecFile = new SureFitVectorFile(vec);
   writeDebugVector(vec, "Intensity.grad");
   
	//2
	//%unsigned char* intgraddata=new unsigned char [num_voxels];
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=intgraddata[i]=(unsigned char)vec.Mag[i];
	//%write_minc ("Intensity.grad.mnc", voxdataflat, xdim, ydim, zdim);
   VolumeFile intgraddata(voxdataflat);
   const int numVoxels = intgraddata.getTotalNumberOfVoxels();
   for (int i = 0; i < numVoxels; i++) {
      const unsigned char value = static_cast<unsigned char>(vec.getMagnitudeWithFlatIndex(i));
      voxdataflat.setVoxelWithFlatIndex(i, 0, value);
      intgraddata.setVoxelWithFlatIndex(i, 0, value);
   }
   writeDebugVolume(voxdataflat, "Intensity.grad");
   gradIntensityVolume = new VolumeFile(voxdataflat);
   
	//3
	//%float GWdiff=WMpeak-CGMpeak;
	//%float GWgradpeak=GWdiff/2;
	//%float GWgradlow=GWgradpeak/2;
	//%float GWgradhigh=GWgradpeak*2;
	//%float GWgradsignum=1.5;
	//%ClassifyIT (GWgradpeak, GWgradlow, GWgradhigh, GWgradsignum, voxdataflat, xdim, ydim, zdim);
	//%write_minc ("Grad.GWlevel.mnc", voxdataflat, xdim, ydim, zdim);
   const float gwDiff = wmPeak - cgmPeak;
   const float gwGradPeak = gwDiff / 2.0;
   const float gwGradLow = gwGradPeak / 2.0;
   const float gwGradHigh = gwGradPeak * 2.0;
   const float gwGradSignum = 1.5;
   voxdataflat.classifyIntensities(gwGradPeak, gwGradLow, gwGradHigh, gwGradSignum);
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(voxdataflat, "Grad.GWlevel");
   
	//4
	//%CombineVectorVolume ("replacemag", xdim, ydim, zdim, vec.X, vec.Y, vec.Z, vec.Mag, voxdataflat);
	//%vec.WriteRaw ("Grad.GWlevel.vec");
	//NEW 2000-10-14 begin
	//%float ThinWMgradpeak=GWdiff/4.0;
	//%float ThinWMgradlow=ThinWMgradpeak/2.0;
	//%float ThinWMgradhigh=ThinWMgradpeak*2.0;
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=intgraddata[i];
	//%ClassifyIT (ThinWMgradpeak, ThinWMgradlow, ThinWMgradhigh, GWgradsignum, voxdataflat, xdim, ydim, zdim);
	//%write_minc ("Grad.ThinWMlevel.mnc", voxdataflat, xdim, ydim, zdim);
	//%CombineVectorVolume ("replacemag", xdim, ydim, zdim, vec.X, vec.Y, vec.Z, vec.Mag, voxdataflat);
	//%vec.WriteRaw ("Grad.ThinWMlevel.vec");
   vec.combineWithVolumeOperation(SureFitVectorFile::COMBINE_VOLUME_REPLACE_MAGNITUDE_WITH_VOLUME,
                        &voxdataflat);
   writeDebugVector(vec, "Grad.GWlevel");
   gradGWlevelVecFile = new SureFitVectorFile(vec);
   const float thinWMGradPeak = gwDiff / 4.0;
   const float thinWMGradLow  = thinWMGradPeak / 2.0;
   const float thinWMGradHigh = thinWMGradPeak * 2.0;
   voxdataflat = intgraddata;
   voxdataflat.classifyIntensities(thinWMGradPeak, thinWMGradLow, thinWMGradHigh, gwGradSignum);
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(voxdataflat, "Grad.ThinWMlevel");
   vec.combineWithVolumeOperation(SureFitVectorFile::COMBINE_VOLUME_REPLACE_MAGNITUDE_WITH_VOLUME,
                        &voxdataflat);
   writeDebugVector(vec, "Grad.ThinWMlevel");
   gradThinWMlevelVecFile = new SureFitVectorFile(vec);
   
	//5
	//%float PiaGradpeak=CGMpeak*2/3;
	//%float PiaGradlow=PiaGradpeak/2;
	//%float PiaGradhigh=PiaGradpeak*3/2;
	//%float PiaGradsignum=1.0;
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=intgraddata[i];
	//%ClassifyIT (PiaGradpeak, PiaGradlow, PiaGradhigh, PiaGradsignum, voxdataflat, xdim, ydim, zdim);
	//%write_minc ("Grad.PiaLevel.mnc", voxdataflat, xdim, ydim, zdim);
   const float piaGradPeak = cgmPeak * 2.0 / 3.0;
   const float piaGradLow  = piaGradPeak / 2.0;
   const float piaGradHigh = piaGradPeak * 3.0 / 2.0;
   const float piaGradSignum = 1.0;
   voxdataflat = intgraddata;
   voxdataflat.classifyIntensities(piaGradPeak, piaGradLow, piaGradHigh, piaGradSignum);
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(voxdataflat, "Grad.PiaLevel");
	
   //6
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=inputdata[i];
	//%ClassifyIT (CGMpeak, lp.CGMlow, lp.CGMhigh, lp.CGMsignum, voxdataflat, xdim, ydim, zdim);
	//ClassifyIT (CGMpeak, 0,0,0, voxdataflat, xdim, ydim, zdim);
	//%write_minc ("GM.ILevel.mnc", voxdataflat, xdim, ydim, zdim);
   voxdataflat = inputdata;
   voxdataflat.classifyIntensities(cgmPeak, cgmLow, cgmHigh, cgmSignum);
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(voxdataflat, "GM.ILevel");
   gmILevelVolume = new VolumeFile(voxdataflat);
   
	//7
	//%Grad (1, "GM.grad", voxdataflat, vec.X, vec.Y, vec.Z, vec.Mag, xdim, ydim, zdim);
   //{
   //   for (int i = 30000; i < 30100; i++) {
   //      std::cout << "Voxel " << i << ": " << voxdataflat.getVoxelWithFlatIndex(i) << std::endl;
   //   }
   //}
   bmvg = new BrainModelVolumeGradient(brainSet,
                                       1,
                                       true,
                                       false,
                                       &voxdataflat,
                                       &voxdataflat,
                                       &vec);
   bmvg->execute();
   //{
   //   float vx, vy, vz;
   //   vec.getVectorWithFlatIndex(30028, vx, vy, vz);
   //   std::cout << "vec 30028 : " << vx << ", " << vy << ", " << vz << ", "
   //             << vec.getMagnitudeWithFlatIndex(30028) << std::endl;
   //}
   delete bmvg;
   bmvg = NULL;
   voxdataflat.stretchVoxelValues();
   writeDebugVector(vec, "GM.grad");

	//8
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=(unsigned char)vec.Mag[i];
	//%write_minc ("GM.grad.mnc", voxdataflat, xdim, ydim, zdim);
   vec.copyMagnitudeToVolume(&voxdataflat);
   writeDebugVolume(voxdataflat, "GM.grad");
   
	//9
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=inputdata[i];
	//%delete [] inputdata;
	//%ClassifyIT (lp.InITpeak, lp.InITlow, lp.InIThigh, lp.InITsignum, voxdataflat, xdim, ydim, zdim);
	//%write_minc ("InIT.mnc", voxdataflat, xdim, ydim, zdim);
   voxdataflat = inputdata;
   voxdataflat.classifyIntensities(inITPeak, inITLow, inITHigh, inITSignum);
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(voxdataflat, "InIT");
    
	//10
	//%CombineVectorVolume ("replacemag", xdim, ydim, zdim, vec.X, vec.Y, vec.Z, vec.Mag, voxdataflat);
	//%vec.WriteRaw ("GmgradVec_InITmag.vec");
   vec.combineWithVolumeOperation(SureFitVectorFile::COMBINE_VOLUME_REPLACE_MAGNITUDE_WITH_VOLUME,
                        &voxdataflat);
   writeDebugVector(vec, "GmgradVec_InITmag");

	//11
	//%int maskflag=0;
	//%int operation=1 ; //dotsqrtrectminus_secondnormal
	//%unsigned char* maskdata=new unsigned char [num_voxels];
	//%for ( i=0 ; i<num_voxels ; i++ ) maskdata[i]=0;
	//%Vector gwvec("Grad.GWlevel.vec", xdim, ydim, zdim);
	//%NewCombineVecs (maskflag, operation, "In.Total", xdim, ydim, zdim, 
	//%	vec.X, vec.Y, vec.Z, vec.Mag, gwvec.X, gwvec.Y, gwvec.Z, gwvec.Mag, maskdata);
	//Note: Unlike grad, output isn't returned in either vec -- to file only.
	//%gwvec.Delete();
   SureFitVectorFile gradInTotal(xDim, yDim, zDim);
   SureFitVectorFile::combineVectorFiles(false, 
                                  SureFitVectorFile::COMBINE_OPERATION_DOT_SQRT_RECT_MINUS,
                                  &vec,
                                  gradGWlevelVecFile,
                                  NULL,
                                  &gradInTotal);
   //for (int jj = 30000; jj < 30100; jj++) {
   //   float x1, y1, z1, m1, x2, y2, z2, m2, x3, y3, z3, m3;
   //   vec.getVectorWithFlatIndex(jj, x1, y1, z1);
   //   m1 = vec.getMagnitudeWithFlatIndex(jj);
   //   gradGWlevelVecFile.getVectorWithFlatIndex(jj, x2, y2, z2);
   //   m2 = gradGWlevelVecFile.getMagnitudeWithFlatIndex(jj);
   //   gradGWlevelVecFile.getVectorWithFlatIndex(jj, x3, y3, z3);
   //   m3 = gradGWlevelVecFile.getMagnitudeWithFlatIndex(jj);
   //   std::cout << jj << ": vec ("
   //             << x1 << ", " << y1 << ", " << z1 << ", " << m1 << ") gradGWlevelVecFile ("
   //             << x2 << ", " << y2 << ", " << z2 << ", " << m2 << ") gradInTotal ("
   //             << x3 << ", " << y3 << ", " << z3 << ", " << m3 << ")" << std::endl;
   //}
   writeDebugVector(gradInTotal, "In.Total");
   
   freeVectorInMemory(gradGWlevelVecFile);
   
	//12
	//%unsigned char* intotaldata=new unsigned char [num_voxels];
	//%vec.fname="In.Total.vec";
	//%vec.ReadRaw();
	//%for ( i=0 ; i<num_voxels ; i++ ) intotaldata[i]=(unsigned char)vec.Mag[i];
	//%write_minc ("In.Total.mnc", intotaldata, xdim, ydim, zdim);
   // ************* WHERE IS "In.Total.vec" WRITTEN !!!! ***************
   vec = gradInTotal;
   VolumeFile intotaldata(voxdataflat);
   vec.copyMagnitudeToVolume(&intotaldata);
   writeDebugVolume(intotaldata, "In.Total");
   inTotalVolume = new VolumeFile(intotaldata);
   
	//13
	//%float sigmaN=1.0;
	//%float sigmaW=1.5;
	//%float offset=1.0;
	//%int downflag=0;
	//%int gradsign=-1;
	//%if ( extractmaskflag == 1 ){
	//%	read_minc_file ("InnerMask.1.mnc", maskdata);
	//%} else {
	//%	for ( i=0 ; i<num_voxels ; i++ ) maskdata[i]=0;
	//%}
	//%vec.fname="Grad.ThinWMlevel.vec";
	//%vec.ReadRaw();
	//%NewNear2Planes ("ThinWM.mnc", sigmaN, sigmaW, offset, downflag, 
	//%	gradsign, extractmaskflag, maskdata, vec.X, vec.Y, vec.Z, vec.Mag, 
	//%	voxdataflat, xdim, ydim, zdim);
	//%unsigned char* twmdata=new unsigned char [num_voxels];
	//%for ( i=0 ; i<num_voxels ; i++ ) twmdata[i]=voxdataflat[i];
   const float sigmaN = 1.0;
   const float sigmaW = 1.5;
   const float offset = 1.0;
   const bool downflag = false;
   const int gradsign = -1;
   VolumeFile maskdata(voxdataflat);
   maskdata.setAllVoxels(0.0);
   if (extractMaskFlag) {
      maskdata = *innerMask1Volume;
   }
   vec = *gradThinWMlevelVecFile;
   BrainModelVolumeNearToPlane* bmvntp =
       new BrainModelVolumeNearToPlane(brainSet,
                                       &vec,
                                       sigmaN,
                                       sigmaW,
                                       offset,
                                       downflag,
                                       gradsign,
                                       extractMaskFlag,
                                       &maskdata,
                                       &voxdataflat);
   try {
      bmvntp->execute();
   }
   catch (BrainModelAlgorithmException& e) {
      throw BrainModelAlgorithmException(e.whatQString());
   }
   delete bmvntp;
   voxdataflat.stretchVoxelValues();
   VolumeFile twmdata(voxdataflat);
     
   freeVectorInMemory(gradThinWMlevelVecFile);
   
	//14
	//%BlurFil(voxdataflat, xdim, ydim, zdim);
	//%write_minc ("ThinWM.blur.mnc", voxdataflat, xdim, ydim, zdim);
   voxdataflat.blur();
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(voxdataflat, "ThinWM.blur");
   
	//15 This part extensively revised by DVE 6/5/2000
	//%for ( i=0 ; i<num_voxels ; i++ ) maskdata[i]=voxdataflat[i];
	//CombineVols ("mult", voxdataflat, maskdata, maskdata, xdim, ydim, zdim);
	//%write_minc ("ThinWM.blur.square.mnc", voxdataflat, xdim, ydim, zdim);
	//%unsigned char* twmbsdata=new unsigned char [num_voxels];
	//%for ( i=0 ; i<num_voxels ; i++ ) twmbsdata[i]=voxdataflat[i];
	//ClassifyIT.py Intensity.grad.mnc VentGradLevel $VentGradpeak $VentGradlow $VentGradhigh $VentGradsignum 
	//%float WM_VentDiff=WMpeak-(CGMpeak/2.0);
	//%float VentGradpeak=WM_VentDiff/2;
	//%float VentGradlow=VentGradpeak*2/3;
	//%float VentGradhigh=VentGradpeak*2;
	//%float VentGradsignum=1.5;
	//for ( i=0 ; i<num_voxels ; i++ ) maskdata[i]=intgraddata[i];
	//%delete [] intgraddata;
   maskdata = voxdataflat;
   VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_MULTIPLY,
                                               &voxdataflat,
                                               &maskdata,
                                               &maskdata,
                                               &voxdataflat);
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(voxdataflat, "ThinWM.blur.square");
   VolumeFile twmbsdata = voxdataflat;
   const float wmVentDiff = wmPeak - (cgmPeak / 2.0);
   const float ventGradPeak = wmVentDiff / 2.0;
   const float ventGradLow = ventGradPeak * 2.0 / 3.0;
   const float ventGradHigh = ventGradPeak * 2.0;
   const float ventGradSignum = 1.5;
   maskdata = intgraddata;
   
	//%ClassifyIT (VentGradpeak, VentGradlow, VentGradhigh, VentGradsignum, maskdata, xdim, ydim, zdim);
	//%write_minc ("VentGradLevel.mnc", maskdata, xdim, ydim, zdim);
	//BlurFil.py VentGradLevel.mnc VentGradLevel.blur 
	//%BlurFil (maskdata, xdim, ydim, zdim);
	//%write_minc ("VentGradLevel.blur.mnc", maskdata, xdim, ydim, zdim);
	//BlurFil.py VentGradLevel.blur.mnc VentGradLevel.blur2 
	//%BlurFil (maskdata, xdim, ydim, zdim);
	//%write_minc ("VentGradLevel.blur2.mnc", maskdata, xdim, ydim, zdim);
   maskdata.classifyIntensities(ventGradPeak, ventGradLow, ventGradHigh, ventGradSignum);
   maskdata.stretchVoxelValues();
   writeDebugVolume(maskdata, "VentGradLevel");
   maskdata.blur();
   maskdata.stretchVoxelValues();
   writeDebugVolume(maskdata, "VentGradLevel.blur");
   maskdata.blur();
   maskdata.stretchVoxelValues();
   writeDebugVolume(maskdata, "VentGradLevel.blur2");

	//MaskVol.py VentGradLevel.blur2.mnc VentGradLevel.blur.mask $xAC_10_40_low $xAC_10_40_high `expr $ACy - 90` $ACy `expr $ACz - 20` `expr $ACz + 20`
	//%int xAC_1_low, xAC_1_high;
	const int xAC_1_low = xAC_40 * Hem2 + xAC_10 * Hem1;
	const int xAC_1_high = xAC_40 * Hem1 + xAC_10 * Hem2;
   int extent[6];
   extent[0] = xAC_1_low;
   extent[1] = xAC_1_high;
	//extent[2] = acIJK[1] - 90;  // 2004-09-27 optical smearing Kathleen Handon, Veronica Smith
   extent[2] = acIJK[1] - 75;
	extent[3] = acIJK[1];
	extent[4] = acIJK[2] - 20;
	extent[5] = acIJK[2] + 20;
	//%MaskVolume (maskdata, xdim, ydim, zdim, extent);
	//%write_minc ("VentGradLevel.blur.mask.mnc", maskdata, xdim, ydim, zdim);
   maskdata.maskVolume(extent);
   maskdata.stretchVoxelValues();
   writeDebugVolume(maskdata, "VentGradLevel.blur.mask");
   ventGradLevelBlurVolume = new VolumeFile(maskdata);

	//CombineVols.py mult ThinWM.blur.square.mnc VentGradLevel.blur.mask.mnc ThinWM_NearVentricle
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=twmbsdata[i];
	//%CombineVols ("mult", voxdataflat, maskdata, maskdata, xdim, ydim, zdim);
	//%write_minc ("ThinWM_NearVentricle.mnc", voxdataflat, xdim, ydim, zdim);
	//%delete [] maskdata;
   voxdataflat = twmbsdata;
   VolumeFile::performMathematicalOperation(
            VolumeFile::VOLUME_MATH_OPERATION_MULTIPLY,
            &voxdataflat,
            &maskdata,
            &maskdata,
            &voxdataflat);
    voxdataflat.stretchVoxelValues();
    writeDebugVolume(voxdataflat, "ThinWM_NearVentricle");


	//CombineVols.py max ThinWM.blur.square.mnc ThinWM_NearVentricle.mnc ThinWM_OrNearVentricle.blur 
	//%CombineVols ("max", twmbsdata, voxdataflat, voxdataflat, xdim, ydim, zdim);
	//%write_minc ("ThinWM_OrNearVentricle.blur.mnc", twmbsdata, xdim, ydim, zdim);
   VolumeFile::performMathematicalOperation(
            VolumeFile::VOLUME_MATH_OPERATION_MAX,
            &twmbsdata,
            &voxdataflat,
            &voxdataflat,
            &twmbsdata);
   twmbsdata.stretchVoxelValues();
   writeDebugVolume(twmbsdata, "ThinWM_OrNearVentricle.blue");

	//%delete [] twmbsdata;
	//14 oct 00: ADD:CombineVols.py max ThinWM.mnc ThinWM_NearVentricle.mnc ThinWM_OrNearVentricle
	//%CombineVols ("max", twmdata, voxdataflat, voxdataflat, xdim, ydim, zdim);
	//%write_minc ("ThinWM_OrNearVentricle.mnc", twmdata, xdim, ydim, zdim);
   VolumeFile::performMathematicalOperation(
               VolumeFile::VOLUME_MATH_OPERATION_MAX,
               &twmdata,
               &voxdataflat,
               &voxdataflat,
               &twmdata);
   twmdata.stretchVoxelValues();
   writeDebugVolume(twmdata, "ThinWM_OrNearVentricle");

	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=twmdata[i];
   voxdataflat = twmdata;

	// MaskVol.py ThinWM_OrNearVentricle.mnc ThinWM_OrNearVentricle.HCmask 
	//	$xAC_10_40_low $xAC_10_40_high `expr $ACy - 45` `expr $ACy + 20` 0 $ACz
	extent[0] = xAC_1_low;
	extent[1] = xAC_1_high;
	extent[2] = acIJK[1] - 45;
	extent[3] = acIJK[1] + 20;
	extent[4] = 0;
	extent[5] = acIJK[2];
	//%MaskVolume (twmdata, xdim, ydim, zdim, extent);
	//%write_minc ("ThinWM_OrNearVentricle.HCmask.mnc", twmdata, xdim, ydim, zdim);
	//%delete [] twmdata;
   twmdata.maskVolume(extent);
   twmdata.stretchVoxelValues();
   writeDebugVolume(twmdata, "ThinWM_OrNearVentricle.HCmask");
   thinWMOrNearVentricleHCMask = new VolumeFile(twmdata);
   
	// 13 oct 00 REPLACE: CombineVols.py max ThinWM_NearVentricle.mnc In.Total.mnc In.Total_ThinWM.mnc 
	// WITH: CombineVols.py max ThinWM_OrNearVentricle.mnc In.Total.mnc In.Total_ThinWM.mnc
	//%CombineVols ("max", voxdataflat, intotaldata, intotaldata, xdim, ydim, zdim);
	//%write_minc ("In.Total_ThinWM.mnc", voxdataflat, xdim, ydim, zdim);
	//%write_minc ("InnerBoundary.mnc", voxdataflat, xdim, ydim, zdim);
	//%delete [] intotaldata;
   VolumeFile::performMathematicalOperation(
                  VolumeFile::VOLUME_MATH_OPERATION_MAX,
                   &voxdataflat,
                   &intotaldata,
                   &intotaldata,
                   &voxdataflat);
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(&voxdataflat, "In.Total_ThinWM");
   inTotalThinWMVolume = new VolumeFile(voxdataflat);
   writeDebugVolume(&voxdataflat, "InnerBoundary");


	//%vec.fname="Intensity.grad.vec";  // NEVER WAS WRITTEN BEFORE THIS
	//%vec.ReadRaw();
	//%CombineVectorVolume ("replacemag", xdim, ydim, zdim, vec.X, vec.Y, vec.Z, vec.Mag, voxdataflat);
	//%vec.WriteRaw ("In.Total_ThinWM.vec");
	//%vec.Delete();
   vec = *gradIntensityVecFile;
   vec.combineWithVolumeOperation(SureFitVectorFile::COMBINE_VOLUME_REPLACE_MAGNITUDE_WITH_VOLUME,
                                  &voxdataflat);
   writeDebugVector(vec, "In.Total_ThinWM");
   gradInTotalThinWMVecFile = new SureFitVectorFile(vec);

	//17
	//%BlurFil (voxdataflat, xdim, ydim, zdim);
	//%write_minc ("In.Total.blur1.mnc", voxdataflat, xdim, ydim, zdim);
   
   //
   // 7/24/2008 DLD added next line to reduce chimp occipital smearing
   //
   if (brainSet->getSpecies().isHuman() == false) { 
      voxdataflat = *inTotalVolume; 
   }
   voxdataflat.blur();
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(voxdataflat, "In.Total.blur1");
   inTotalBlur1Volume = new VolumeFile(voxdataflat);
   
	//%delete [] voxdataflat;
	//%printf("#### GenerateInnerBoundary end ####\n");
	//%return 0;
   if (DebugControl::getDebugOn()) {
      std::cout << "#### GenerateInnerBoundary end ####" << std::endl;
   }
}

/**
 * generate the outer boundary.
 */
void 
BrainModelVolumeSureFitSegmentation::generateOuterBoundary() throw (BrainModelAlgorithmException)
{
	//%printf("#### GenerateOuterBoundary begin ####\n");
   if (DebugControl::getDebugOn()) {
      std::cout << "#### GenerateOuterBoundary ####" << std::endl;
   }
   
	//%int i;
	//%int xdim=lp.xdim;
	//%int ydim=lp.ydim;
	//%int zdim=lp.zdim;
	//%int num_voxels=xdim*ydim*zdim;
	//%unsigned char* outitdata=new unsigned char [num_voxels];
	//%read_minc_file (mincfile, outitdata);
   VolumeFile outitdata = *anatomyVolume;

	//1
	//%ClassifyIT (lp.OutITpeak, lp.OutITlow, lp.OutIThigh, lp.OutITsignum, outitdata, xdim, ydim, zdim);
	//%write_minc ("OutIT.mnc", outitdata, xdim, ydim, zdim);
   outitdata.classifyIntensities(outITPeak, outITLow, outITHigh, outITSignum);
   outitdata.stretchVoxelValues();
   writeDebugVolume(outitdata, "OutIT");

	//2	
	//%float PiaGradpeak=lp.CGMpeak*2/3.0;
	//%float PiaGradlow=PiaGradpeak/2.0;
	//%float PiaGradhigh=PiaGradpeak*3/2.0;
	//%float PiaGradsignum=1.0;
	//%unsigned char* voxdataflat=new unsigned char [num_voxels];
	//%read_minc_file ("Intensity.grad.mnc", voxdataflat);
	//%ClassifyIT (PiaGradpeak, PiaGradlow, PiaGradhigh, PiaGradsignum, 
	//%	voxdataflat, xdim, ydim, zdim);
	//%write_minc ("Grad.PiaLevel.mnc", voxdataflat, xdim, ydim, zdim);
   const float piaGradPeak = cgmPeak * 2.0 / 3.0;
   const float piaGradLow  = piaGradPeak / 2.0;
   const float piaGradHigh = piaGradPeak * 3.0 / 2.0;
   const float piaGradSignum = 1.0;
   VolumeFile voxdataflat = *gradIntensityVolume;
   voxdataflat.classifyIntensities(piaGradPeak, piaGradLow, piaGradHigh, piaGradSignum);
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(voxdataflat, "Grad.PiaLevel");

   freeVolumeInMemory(gradIntensityVolume);
   
	//3
	//%Vector vec("Intensity.grad.vec", xdim, ydim, zdim);
	//%CombineVectorVolume ("replacemag", xdim, ydim, zdim, vec.X, vec.Y, vec.Z, vec.Mag, voxdataflat);
	//%vec.WriteRaw ("Grad.PiaLevel.vec");
   SureFitVectorFile vec = *gradIntensityVecFile;
   vec.combineWithVolumeOperation(SureFitVectorFile::COMBINE_VOLUME_REPLACE_MAGNITUDE_WITH_VOLUME,
                                  &voxdataflat);
   writeDebugVector(vec, "Grad.PiaLevel");
   gradPiaLevelVec = new SureFitVectorFile(vec);
   
	//4
	//%unsigned char* soliddata=new unsigned char [num_voxels];
	//%for ( i=0 ; i<num_voxels ; i++ ) soliddata[i]=255;
	//%write_minc ("SolidWhite.mnc", soliddata, xdim, ydim, zdim);
   VolumeFile soliddata(voxdataflat);
   soliddata.setAllVoxels(255.0);
   writeDebugVolume(soliddata, "SolidWhite");

	//5
	//%read_minc_file ("GM.ILevel.mnc", voxdataflat);
	//%CombineVols ("subrect", soliddata, voxdataflat, voxdataflat, xdim, ydim, zdim);
	//%write_minc ("InvertGM.ILevel.mnc", soliddata, xdim, ydim, zdim);
   voxdataflat = *gmILevelVolume;
   VolumeFile::performMathematicalOperation(
                    VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                    &soliddata,
                    &voxdataflat,
                    &voxdataflat,
                    &soliddata);
   soliddata.stretchVoxelValues();
   writeDebugVolume(soliddata, "InvertGM.ILevel");
   
   freeVolumeInMemory(gmILevelVolume);
   
	//6
	//%Grad (1, "InvertGM.grad", soliddata, vec.X, vec.Y, vec.Z, vec.Mag, xdim, ydim, zdim);
   BrainModelVolumeGradient* bmvg = new BrainModelVolumeGradient(brainSet,
                                                  1,
                                                  true,
                                                  false,
                                                  &soliddata,
                                                  &soliddata,
                                                  &vec);
   try {
      bmvg->execute();
   }
   catch (BrainModelAlgorithmException& e) {
      throw BrainModelAlgorithmException(e.whatQString());
   }
   writeDebugVector(vec, "InvertGM.grad");
   delete bmvg;
   bmvg = NULL;
   soliddata.stretchVoxelValues();
   
	//7
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=(unsigned char)vec.Mag[i];
	//%write_minc ("InvertGM.grad.mnc", voxdataflat, xdim, ydim, zdim);
   vec.copyMagnitudeToVolume(&voxdataflat);
   writeDebugVolume(voxdataflat, "InvertGM.grad");

	//8
	//%CombineVectorVolume ("replacemag", xdim, ydim, zdim, vec.X, vec.Y, vec.Z, vec.Mag, outitdata);
	//%vec.WriteRaw ("InvertGMgradVec_OutITmag.vec");
	//%delete [] outitdata;	delete [] soliddata;
   vec.combineWithVolumeOperation(SureFitVectorFile::COMBINE_VOLUME_REPLACE_MAGNITUDE_WITH_VOLUME,
                                  &outitdata);
   writeDebugVector(vec, "InvertGMgradVec_OutITmag");

	//9
	//%int maskflag=0;
	//%int operation=1 ; //dotsqrtrectminus_secondnormal
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=0;
	//%Vector piavec("Grad.PiaLevel.vec", xdim, ydim, zdim);
	//%NewCombineVecs (maskflag, operation, "Out.GradPialLevel_GMgrad_OutITmag", xdim, ydim, zdim, 
	//%	vec.X, vec.Y, vec.Z, vec.Mag, piavec.X, piavec.Y, piavec.Z, piavec.Mag, voxdataflat);
	//Note: Unlike grad, output isn't returned in either vec -- to file only.
	//%piavec.Delete();
	//%vec.fname="Out.GradPialLevel_GMgrad_OutITmag.vec";
	//%vec.ReadRaw();
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=(unsigned char)vec.Mag[i];
	//%write_minc ("Out.GradPialLevel_GMgrad_OutITmag.mnc", voxdataflat, xdim, ydim, zdim);
   voxdataflat.setAllVoxels(0.0);
   SureFitVectorFile piaVec = *gradPiaLevelVec;
   SureFitVectorFile gradPialLevelOutITMag(xDim, yDim, zDim);
   SureFitVectorFile::combineVectorFiles(false,
                                  SureFitVectorFile::COMBINE_OPERATION_DOT_SQRT_RECT_MINUS,
                                  &vec,
                                  &piaVec,
                                  &voxdataflat,
                                  &gradPialLevelOutITMag);
   writeDebugVector(gradPialLevelOutITMag, "Out.GradPialLevel_GMgrad_OutITmag");
   outGradPialLevelGMGradOutITMagVecFile = new SureFitVectorFile(gradPialLevelOutITMag);
   gradPialLevelOutITMag.copyMagnitudeToVolume(&voxdataflat);
   writeDebugVolume(voxdataflat, "Out.GradPialLevel_GMgrad_OutITmag");

   freeVectorInMemory(gradPiaLevelVec);
   
	//10
	//%vec.fname="Intensity.grad.vec";
	//%vec.ReadRaw();
	//%CombineVectorVolume ("replacemag", xdim, ydim, zdim, vec.X, vec.Y, vec.Z, vec.Mag, voxdataflat);
	//%vec.WriteRaw ("Out.GradPialLevel_GMgrad_OutITmag.vec");
   vec = *gradIntensityVecFile;
   vec.combineWithVolumeOperation(SureFitVectorFile::COMBINE_VOLUME_REPLACE_MAGNITUDE_WITH_VOLUME,
                                  &voxdataflat);
   writeDebugVector(vec, "Out.GradPialLevel_GMgrad_OutITmag");
                                  
	//11
	//%char * outnear2infname="Out.Near2In.mnc";
	//%float sigmaN=1.4;
	//%float sigmaW=1.5;
	//%float offset=2.2;
	//%int downflag=0;
	//%int gradsign=1;
	//%unsigned char* maskdata=new unsigned char [num_voxels];
	//%if ( extractmaskflag == TRUE ) read_minc_file ("OuterMask.1.mnc", maskdata);
	//%else for ( i=0 ; i<num_voxels ; i++ ) maskdata[i]=0;
	//%vec.fname="In.Total_ThinWM.vec";
	//%vec.ReadRaw();
	//%NewNear2Planes (outnear2infname, sigmaN, sigmaW, offset, downflag, 
	//%	gradsign, extractmaskflag, maskdata, vec.X, vec.Y, vec.Z, vec.Mag, 
	//%	voxdataflat, xdim, ydim, zdim);
	//%write_minc (outnear2infname, voxdataflat, xdim, ydim, zdim);
   const float sigmaN = 1.4;
   const float sigmaW = 1.5;
   const float offset = 2.2;
   const int gradSign = 1;
   const bool downFlag = false;
   VolumeFile maskdata = *outerMaskVolume;
   if (extractMaskFlag == false) {
      maskdata.setAllVoxels(0.0);
   }
   vec = *gradInTotalThinWMVecFile;
   BrainModelVolumeNearToPlane* bmvntp = new BrainModelVolumeNearToPlane(brainSet,
                                                    &vec,
                                                    sigmaN,
                                                    sigmaW,
                                                    offset,
                                                    downFlag,
                                                    gradSign,
                                                    extractMaskFlag,
                                                    &maskdata,
                                                    &voxdataflat);
   bmvntp->execute();
   delete bmvntp;
   bmvntp = NULL;
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(voxdataflat, "Out.Near2In");

   freeVolumeInMemory(outerMaskVolume);
   freeVectorInMemory(gradInTotalThinWMVecFile);
   
	//%for ( i=0 ; i<num_voxels ; i++ ) soliddata[i]=255;
	//%if ( extractmaskflag == TRUE ) {
		//%unsigned char* ontidata=new unsigned char [num_voxels];
		//%for ( i=0 ; i<num_voxels ; i++ ) ontidata[i]=voxdataflat[i];
		//13 oct 00 Replace CombineVols.py sqrt Out.Near2In.mnc SolidWhite.mnc Out.Near2In.sqrt:
		// with next three steps:
		// CombineVols.py subrect SolidWhite.mnc ../CEREBRAL.WHITE.MATTER/CerebralWM_noBstem.fill.mnc NOT_CerebralWM
		//%read_minc_file ("CerebralWM_noBstem.fill.mnc", voxdataflat);
		//%CombineVols ("subrect", soliddata, voxdataflat, voxdataflat, xdim, ydim, zdim);
		//%write_minc ("NOT_CerebralWM.mnc", soliddata, xdim, ydim, zdim);
		//CombineVols.py subrect NOT_CerebralWM.mnc ../INNER.BOUNDARY/In.Total_ThinWM.mnc NOT_InTotal_ThinWM_orCerebralWM.mnc
		//%read_minc_file ("In.Total_ThinWM.mnc", maskdata);
		//%CombineVols ("subrect", soliddata, maskdata, maskdata, xdim, ydim, zdim);
		//%write_minc ("NOT_InTotal_ThinWM_orCerebralWM.mnc", soliddata, xdim, ydim, zdim);
		// CombineVols.py sqrt Out.Near2In.mnc NOT_InTotal_ThinWM_orCerebralWM.mnc Out.Near2In.sqrt
		//%CombineVols ("sqrt", ontidata, soliddata, soliddata, xdim, ydim, zdim);
		//%write_minc ("Out.Near2In.sqrt.mnc", ontidata, xdim, ydim, zdim);
		//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=ontidata[i];
		//%delete [] ontidata;
	//%} else {
		// 2/15/2001: Restored for segment full volume:
		//12 CombineVols.py sqrt Out.Near2In.mnc SolidWhite.mnc Out.Near2In.sqrt
		//%CombineVols ("sqrt", voxdataflat, soliddata, soliddata, xdim, ydim, zdim);
		//%write_minc ("Out.Near2In.sqrt.mnc", voxdataflat, xdim, ydim, zdim);
	//%}
	//%delete [] soliddata;
   soliddata.setAllVoxels(255.0);
   if (extractMaskFlag) {
      VolumeFile ontidata(voxdataflat);
      voxdataflat = *cerebralWmNoBstemFill;
      VolumeFile::performMathematicalOperation(
                      VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                      &soliddata,
                      &voxdataflat,
                      &voxdataflat,
                      &soliddata);
      soliddata.stretchVoxelValues();
      writeDebugVolume(soliddata, "NOT_CerebralWM");
      maskdata = *inTotalThinWMVolume;
      VolumeFile::performMathematicalOperation(
                      VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                      &soliddata,
                      &maskdata,
                      &maskdata,
                      &soliddata);
      soliddata.stretchVoxelValues();
      writeDebugVolume(soliddata, "NOT_InTotal_ThinWM_orCerebralWM");
      VolumeFile::performMathematicalOperation(
                       VolumeFile::VOLUME_MATH_OPERATION_SQRT,
                       &ontidata,
                       &soliddata,
                       &soliddata,
                       &ontidata);
      ontidata.stretchVoxelValues();
      writeDebugVolume(ontidata, "Out.Near2In.sqrt");
      voxdataflat = ontidata;
   }
   else {
      VolumeFile::performMathematicalOperation(
                       VolumeFile::VOLUME_MATH_OPERATION_SQRT,
                       &voxdataflat,
                       &soliddata,
                       &soliddata,
                       &voxdataflat);
      voxdataflat.stretchVoxelValues();
      writeDebugVolume(voxdataflat, "Out.Near2In.sqrt");
   }
   
   freeVolumeInMemory(cerebralWmNoBstemFill);
   
   freeVolumeInMemory(inTotalThinWMVolume);
   
	//13, 14 Changed to following on 14 OCT 00:
	//CombineVols.py subrect  Out.Near2In.sqrt.mnc ../INNER.BOUNDARY/ThinWM_OrNearVentricle.HCmask.mnc	Out.Near2In.sqrt_notThinWM.HCmask
	//CombineVecVol.py ../INNER.BOUNDARY/Intensity.grad.vec Out.Near2In Out.Near2In.sqrt_notThinWM.HCmask.mnc
	//%read_minc_file ("ThinWM_OrNearVentricle.HCmask.mnc", maskdata);
	//%CombineVols ("subrect", voxdataflat, maskdata, maskdata, xdim, ydim, zdim);
	//%write_minc ("Out.Near2In.sqrt_notThinWM.HCmask.mnc", voxdataflat, xdim, ydim, zdim);
	//%vec.fname="Intensity.grad.vec";
	//%vec.ReadRaw();
	//%CombineVectorVolume ("replacemag", xdim, ydim, zdim, vec.X, vec.Y, vec.Z, vec.Mag, voxdataflat);
	//%vec.WriteRaw ("Out.Near2In.vec");
   //
   // 7/24/2008 DLD added species condition to reduce chimp occipital smearing
   //
   if (brainSet->getSpecies().isHuman()) {
      maskdata = *thinWMOrNearVentricleHCMask;
      VolumeFile::performMathematicalOperation(
                        VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                        &voxdataflat,
                        &maskdata,
                        &maskdata,
                        &voxdataflat);
      voxdataflat.stretchVoxelValues();
   }
   writeDebugVolume(voxdataflat, "Out.Near2In.sqrt_notThinWM.HCmask");
   vec = *gradIntensityVecFile;
   vec.combineWithVolumeOperation(SureFitVectorFile::COMBINE_VOLUME_REPLACE_MAGNITUDE_WITH_VOLUME,
                                  &voxdataflat);
   writeDebugVector(vec, "Out.Near2In");
           
   freeVolumeInMemory(thinWMOrNearVentricleHCMask);
   
	//16
	//%maskflag=0;
	//%operation=2; //2vec
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=0;
	//%Vector ogpvec("Out.GradPialLevel_GMgrad_OutITmag.vec", xdim, ydim, zdim);
	//%NewCombineVecs (maskflag, operation, "Out.Total", xdim, ydim, zdim, 
	//%	ogpvec.X, ogpvec.Y, ogpvec.Z, ogpvec.Mag, vec.X, vec.Y, vec.Z, vec.Mag, voxdataflat);
	//Note: Unlike grad, output isn't returned in either vec -- to file only.
	//%ogpvec.Delete();
   voxdataflat.setAllVoxels(0.0);
   SureFitVectorFile ogpvec = *outGradPialLevelGMGradOutITMagVecFile;
   SureFitVectorFile outTotalVec(xDim, yDim, zDim);
   SureFitVectorFile::combineVectorFiles(false,
                                  SureFitVectorFile::COMBINE_OPERATION_2_VEC_NORMAL,
                                  &ogpvec,
                                  &vec,
                                  &voxdataflat,
                                  &outTotalVec);
   writeDebugVector(outTotalVec, "Out.Total");
                                  
   freeVectorInMemory(outGradPialLevelGMGradOutITMagVecFile);
   
	//17
	//%vec.fname="Out.Total.vec";
	//%vec.ReadRaw();
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=(unsigned char)vec.Mag[i];
	//%write_minc ("Out.Total.mnc", voxdataflat, xdim, ydim, zdim);
   vec = outTotalVec;
   vec.copyMagnitudeToVolume(&voxdataflat);
   writeDebugVolume(voxdataflat, "Out.Total");
   outTotalVolume = new VolumeFile(voxdataflat);

	//New step added 6/5/2000:
	//%read_minc_file ("VentGradLevel.blur.mask.mnc", maskdata);
	//%CombineVols ("subrect", voxdataflat, maskdata, maskdata, xdim, ydim, zdim);
	//%write_minc ("Out.Total_notVentricle.mnc", voxdataflat, xdim, ydim, zdim);
	//%write_minc ("OuterBoundary.mnc", voxdataflat, xdim, ydim, zdim);
   
   //
   // 7/24/2008 DLD added species condition to reduce chimp occipital smearing
   //
   if (brainSet->getSpecies().isHuman()) {
      maskdata = *ventGradLevelBlurVolume;
      VolumeFile::performMathematicalOperation(
                        VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                        &voxdataflat,
                        &maskdata,
                        &maskdata,
                        &voxdataflat);
      voxdataflat.stretchVoxelValues();
   }
   writeDebugVolume(voxdataflat, "Out.Total_notVentricle");
   writeDebugVolume(voxdataflat, "OuterBoundary");
   
	//18
	//%BlurFil(voxdataflat, xdim, ydim, zdim);
	//%write_minc ("Out.Total.blur1.mnc", voxdataflat, xdim, ydim, zdim);
   voxdataflat.blur();
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(voxdataflat, "Out.Total.blur1");
   outTotalBlur1Volume = new VolumeFile(voxdataflat);

	// Clean up
	//%delete [] maskdata;
	//%delete [] voxdataflat;
	//%printf("#### GenerateOuterBoundary end ####\n");
	//%return 0;
   if (DebugControl::getDebugOn()) {
      std::cout << "#### GenerateOuterBoundary end ####" << std::endl;
   }
}

/**
 * generate the segmentation.
 */
void 
BrainModelVolumeSureFitSegmentation::generateSegmentation() throw (BrainModelAlgorithmException)
{
	//%printf("#### GenerateSegmentation begin ####\n");
   if (DebugControl::getDebugOn()) {
      std::cout  << "#### GenerateSegmentation begin ####" << std::endl;
   }
   
	//%int i, seed[3];
	//%int xdim=lp.xdim;
	//%int ydim=lp.ydim;
	//%int zdim=lp.zdim;
	//%int num_voxels=xdim*ydim*zdim;
	//%float WMThresh=lp.WMThresh;
	//%unsigned char* voxdataflat=new unsigned char [num_voxels];
	//%unsigned char* maskdata=new unsigned char [num_voxels];
   VolumeFile voxdataflat(*anatomyVolume);
   VolumeFile maskdata(*anatomyVolume);

	//%if ( extractmaskflag == TRUE ) {
	//%	read_minc_file ("In.Total.blur1.mnc", voxdataflat);
	//%	read_minc_file ("InnerMask.1.mnc", maskdata);
	//%	CombineVols ("mult", voxdataflat, maskdata, maskdata, xdim, ydim, zdim);
	//%	write_minc ("In.Total.blur.mask.mnc", voxdataflat, xdim, ydim, zdim);
	//%	read_minc_file ("CerebralWM.erode.mnc", maskdata);
	//%} 
   //%else {
	//%	//1 Thresh.py ../$fname WM.thresh.mnc WMThresh
	//%	read_minc_file (mincfile, maskdata);
	//%	ThresholdVolume (maskdata, int(WMThresh), xdim,ydim,zdim);
	//%	write_minc ("WhiteMatter.Thresholded.mnc", maskdata, xdim, ydim, zdim);
	//%	//2 FillBiggestObject.py WM.thresh.mnc WM.thresh.flood.mnc 0 ncol 0 nrow 0 nslices
	//%	FindBiggestObjectWithinMask (voxdataflat, xdim, ydim, zdim, 0, xdim, 0, ydim, 0, zdim, seed);
	//%	if (( seed[0] == -1 ) || ( seed[1] == -1 ) || ( seed[2] == -1 )) {
	//%		printf("Exiting: Null volume\n");
	//%		return 1;
	//%	}
	//%	vtkFloodFill (seed, maskdata, 255, 255, 0, xdim, ydim, zdim);
	//%	write_minc ("WM.thresh.flood.mnc", maskdata, xdim, ydim, zdim);
	//%	//3 VolMorphOps.py 0 2 WM.thresh.flood.mnc WM.thresh.erode.mnc
	//%	DoVolMorphOps (xdim, ydim, zdim, maskdata, 0, 2);
	//%	write_minc ("WM.thresh.erode.mnc", maskdata, xdim, ydim, zdim);
	//%	read_minc_file ("In.Total.blur1.mnc", voxdataflat);
	//%}
   if (extractMaskFlag) {
      voxdataflat = *inTotalBlur1Volume;
      maskdata    = *innerMask1Volume;
      VolumeFile::performMathematicalOperation(
                        VolumeFile::VOLUME_MATH_OPERATION_MULTIPLY,
                        &voxdataflat,
                        &maskdata,
                        &maskdata,
                        &voxdataflat);
      voxdataflat.stretchVoxelValues();
      writeDebugVolume(voxdataflat, "In.Total.blur.mask");
      maskdata = *cerebralWMErodeVolume;
   }
   else {
      maskdata = *anatomyVolume;
      maskdata.thresholdVolume(static_cast<int>(wmThresh));
      writeDebugVolume(maskdata, "WhiteMatter.Thresholded");
      VoxelIJK voxelSeed(0, 0, 0);
      voxdataflat.findBiggestObjectWithinMask(0, xDim, 
                                              0, yDim,
                                              0, zDim,
                                              255.0, 255.0,
                                              voxelSeed);
      if (voxelSeed.getI() < 0) {
         throw BrainModelAlgorithmException(
            "findBiggestObjectWithinMask() failed to find biggest object when\n"
            "trying to create intermediate volume \"WM.thresh.flood\".");
      }
      maskdata.floodFillWithVTK(voxelSeed, 255, 255, 0);
      writeDebugVolume(maskdata, "WM.thresh.flood");
      wmThreshFloodVolume = new VolumeFile(maskdata);
      maskdata.doVolMorphOps(0, 2);
      writeDebugVolume(maskdata, "WM.thresh.erode");
      voxdataflat = *inTotalBlur1Volume;
   }

   freeVolumeInMemory(innerMask1Volume);
   freeVolumeInMemory(inTotalBlur1Volume);
   
	//4 CombineVolsDiffRatio.py ../INNER.BOUNDARY/In.Total.blur1.mnc ../OUTER.BOUNDARY/Out.Total.blur1.mnc WM.thresh.erode.mnc InOutDiff.mag.mnc
	//%unsigned char* outtotalblurdata=new unsigned char [num_voxels];
	//%read_minc_file ("Out.Total.blur1.mnc", outtotalblurdata);
   VolumeFile outtotalblurdata = *outTotalBlur1Volume;

   freeVolumeInMemory(outTotalBlur1Volume);
   
	//%CombineVols ("diffratio", voxdataflat, outtotalblurdata, maskdata, xdim, ydim, zdim);
	//%write_minc ("InOutDiff.mag.mnc", voxdataflat, xdim, ydim, zdim);
	//%write_minc ("RadialPositionMap.Initial.mnc", voxdataflat, xdim, ydim, zdim);
	//%delete [] outtotalblurdata;
	//%unsigned char* rpmdata=new unsigned char [num_voxels];
	//%for ( i=0 ; i<num_voxels ; i++ ) rpmdata[i]=voxdataflat[i];
   VolumeFile::performMathematicalOperation(
             VolumeFile::VOLUME_MATH_OPERATION_DIFFRATIO,
             &voxdataflat,
             &outtotalblurdata,
             &maskdata,
             &voxdataflat);
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(voxdataflat, "InOutDiff.mag");
   writeDebugVolume(voxdataflat, "RadialPositionMap.Initial");
   VolumeFile rpmdata = voxdataflat;

	//5 Thresh.py InOutDiff.mag.mnc InOutDiff.thresh.mnc InOutDiffThresh
	//%int InOutDiffThresh=150;
	//%ThresholdVolume (voxdataflat, InOutDiffThresh, xdim,ydim,zdim);
	//%write_minc ("InOutDiff.thresh.mnc", voxdataflat, xdim, ydim, zdim);
	//%unsigned char* inoutdiffthreshdata=new unsigned char [num_voxels];
	//%for ( i=0 ; i<num_voxels ; i++ ) inoutdiffthreshdata[i]=voxdataflat[i];
   const int inOutDiffThresh = 150;
   voxdataflat.thresholdVolume(static_cast<int>(inOutDiffThresh));
   writeDebugVolume(voxdataflat, "InOutDiff.thresh");
   VolumeFile inoutdiffthreshdata = voxdataflat;

	//6 FillBiggestObject.py InOutDiff.thresh.mnc InOutDiff.flood.mnc 0 ncol 0 nrow 0 nslices
	//%FindBiggestObjectWithinMask (voxdataflat, xdim, ydim, zdim, 0, xdim, 0, ydim, 0, zdim, seed);
	//%if (( seed[0] == -1 ) || ( seed[1] == -1 ) || ( seed[2] == -1 )) {
	//%	printf("Exiting: Null volume\n");
	//%	return 1;
	//%}
	//%vtkFloodFill (seed, voxdataflat, 255, 255, 0, xdim, ydim, zdim);
	//%write_minc ("InOutDiff.flood.mnc", voxdataflat, xdim, ydim, zdim);
   VoxelIJK voxelSeed(0, 0, 0);
   voxdataflat.findBiggestObjectWithinMask(0, xDim,
                                           0, yDim,
                                           0, zDim,
                                           255.0, 255.0,
                                           voxelSeed);
   if (voxelSeed.getI() < 0) {
      throw BrainModelAlgorithmException(
         "findBiggestObjectWithinMask() failed to find biggest object when\n"
         "trying to create intermediate volume \"InOutDiff.flood\".");
   }
   voxdataflat.floodFillWithVTK(voxelSeed, 255, 255, 0);
   writeDebugVolume(voxdataflat, "InOutDiff.flood");
   
	//7 Grad.py 1 InTotal.grad ../INNER.BOUNDARY/In.Total.mnc
	//%read_minc_file ("In.Total.mnc", voxdataflat);
	//%Vector vec("", xdim, ydim, zdim);
	//%Grad (1, "InTotal.grad", voxdataflat, vec.X, vec.Y, vec.Z, vec.Mag, xdim, ydim, zdim);
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=(unsigned char)vec.Mag[i];
	//%write_minc ("InTotal.grad.mnc", voxdataflat, xdim, ydim, zdim);
   voxdataflat = *inTotalVolume;
   SureFitVectorFile vec(xDim, yDim, zDim);
   BrainModelVolumeGradient* bmvg = new BrainModelVolumeGradient(
                                           brainSet,
                                           1,
                                           true,
                                           false,
                                           &voxdataflat,
                                           &voxdataflat,
                                           &vec);
   bmvg->execute();
   delete bmvg;
   bmvg = NULL;
   writeDebugVector(vec, "InTotal.grad");
   SureFitVectorFile inTotalGradVector = vec;
   vec.copyMagnitudeToVolume(&voxdataflat);
   writeDebugVolume(voxdataflat, "InTotal.grad");

   freeVolumeInMemory(inTotalVolume);
   
	//9 Grad.py 1 OutTotal.grad ../OUTER.BOUNDARY/Out.Total.mnc
	//%read_minc_file ("Out.Total.mnc", voxdataflat);
	//%Grad (1, "OutTotal.grad", voxdataflat, vec.X, vec.Y, vec.Z, vec.Mag, xdim, ydim, zdim);
   voxdataflat = *outTotalVolume;
   bmvg = new BrainModelVolumeGradient(brainSet,
                                           1,
                                           true,
                                           false,
                                           &voxdataflat,
                                           &voxdataflat,
                                           &vec);
   bmvg->execute();
   delete bmvg;
   bmvg = NULL;
   writeDebugVector(vec, "OutTotal.grad");

   freeVolumeInMemory(outTotalVolume);
   
	//10 ViewVector.py  OutTotal.grad.vec ../$fname OutTotal.grad
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=(unsigned char)vec.Mag[i];
	//%write_minc ("OutTotal.grad.mnc", voxdataflat, xdim, ydim, zdim);
   vec.copyMagnitudeToVolume(&voxdataflat);
   writeDebugVolume(voxdataflat, "OutTotal.grad");
   
	//11 CombineVectors dotsqrtrectminus ncol nrow nslices OutTotal.grad.vec InTotal.grad.vec OutInOppositeGrad.raw 0 
	//%int maskflag=0;
	//%int operation=1; //dotsqrtrectminus_secondnormal
	//%for ( i=0 ; i<num_voxels ; i++ ) maskdata[i]=0;
	//%Vector invec("InTotal.grad.vec", xdim, ydim, zdim);
	//%NewCombineVecs (maskflag, operation, "OutInOppositeGrad", xdim, ydim, zdim, 
	//%	vec.X, vec.Y, vec.Z, vec.Mag, invec.X, invec.Y, invec.Z, invec.Mag, voxdataflat);
	//%invec.Delete();
	//%vec.fname="OutInOppositeGrad.vec";
	//%vec.ReadRaw();
	//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=(unsigned char)vec.Mag[i];
	//%write_minc ("OutInOppositeGrad.mnc", voxdataflat, xdim, ydim, zdim);
   maskdata.setAllVoxels(0.0);
   SureFitVectorFile invec = inTotalGradVector;
   SureFitVectorFile outInOppositeGradVector(xDim, yDim, zDim);
   SureFitVectorFile::combineVectorFiles(false,
                                  SureFitVectorFile::COMBINE_OPERATION_DOT_SQRT_RECT_MINUS,
                                  &vec,
                                  &invec,
                                  &voxdataflat,
                                  &outInOppositeGradVector);
   writeDebugVector(outInOppositeGradVector, "OutInOppositeGrad");
   vec = outInOppositeGradVector;
   vec.copyMagnitudeToVolume(&voxdataflat);
   writeDebugVolume(voxdataflat, "OutInOppositeGrad");

	//12 CombineVecVol.py ../INNER.BOUNDARY/Intensity.grad.vec OutInOppositeGrad_In OutInOppositeGrad.mnc 
	//%vec.fname="Intensity.grad.vec";
	//%vec.ReadRaw();
	//%CombineVectorVolume ("replacemag", xdim, ydim, zdim, vec.X, vec.Y, vec.Z, vec.Mag, voxdataflat);
	//%vec.WriteRaw ("OutInOppositeGrad_In.vec");
   vec = *gradIntensityVecFile;
   vec.combineWithVolumeOperation(SureFitVectorFile::COMBINE_VOLUME_REPLACE_MAGNITUDE_WITH_VOLUME,
                                  &voxdataflat);
   writeDebugVector(vec, "OutInOppositeGrad_In");
   
	//13 MakeShell.py 2 1 InOutDiff.thresh.mnc OuterMask.3
	//%MakeShell (inoutdiffthreshdata, xdim, ydim, zdim, 2, 1);
	//%write_minc ("OuterMask.3.mnc", inoutdiffthreshdata, xdim, ydim, zdim);
   inoutdiffthreshdata.makeShellVolume(2, 1);
   inoutdiffthreshdata.stretchVoxelValues();
   writeDebugVolume(inoutdiffthreshdata, "OuterMask.3.mnc");

   freeVectorInMemory(gradIntensityVecFile);
   
	//14 NewNear2Planes.py OuterMask.3.mnc PialTrough OutInOppositeGrad_In.vec 0.800 1.50000 1.0000  0 1 1
	//%float sigmaN=0.8;
	//%float sigmaW=1.5;
	//%float offset=1.0;
	//%int downflag=0;
	//%int gradsign=1;
	//%maskflag=1;
	//%NewNear2Planes ("PialTrough.mnc", sigmaN, sigmaW, offset, downflag, 
	//%	gradsign, maskflag, inoutdiffthreshdata, vec.X, vec.Y, vec.Z, vec.Mag, 
	//%	voxdataflat, xdim, ydim, zdim);
	//%write_minc ("PialTrough.mnc", voxdataflat, xdim, ydim, zdim);
	//%delete [] inoutdiffthreshdata;
	//%vec.Delete();
   const float sigmaN = 0.8;
   const float sigmaW = 1.5;
   const float offset = 1.0;
   BrainModelVolumeNearToPlane* bmvntp = new BrainModelVolumeNearToPlane(
                                                brainSet,
                                                &vec,
                                                sigmaN,
                                                sigmaW,
                                                offset,
                                                false,
                                                1,
                                                true,
                                                &inoutdiffthreshdata,
                                                &voxdataflat);
   bmvntp->execute();
   delete bmvntp;
   bmvntp = NULL;
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(voxdataflat, "PialTrough");
                                                
	//15 VolMorphOps.py 1 0 CerebralWM.flood.mnc CerebralWM.flood.dilate.mnc
	//%if ( extractmaskflag == TRUE ) read_minc_file ("CerebralWM.erode.mnc", maskdata);
	//%else read_minc_file ("WM.thresh.flood.mnc", maskdata);
	//%DoVolMorphOps (xdim, ydim, zdim, maskdata, 1, 0);
	//%write_minc ("CerebralWM.flood.dilate.mnc", maskdata, xdim, ydim, zdim);
   if (extractMaskFlag) {
      maskdata = *cerebralWMErodeVolume;
   }
   else {
      maskdata = *wmThreshFloodVolume;
   }
   maskdata.doVolMorphOps(1, 0);
   writeDebugVolume(maskdata, "CerebralWM.flood.dilate");

   freeVolumeInMemory(wmThreshFloodVolume);
   freeVolumeInMemory(cerebralWMErodeVolume);
   
	//16 CombineVols.py subrect PialTrough.mnc CerebralWM.flood.dilate.mnc PialTrough_noCerebralWM.mnc
	//%CombineVols ("subrect", voxdataflat, maskdata, maskdata, xdim, ydim, zdim);
	//%write_minc ("PialTrough_noCerebralWM.mnc", voxdataflat, xdim, ydim, zdim);
	// New step added 6/5/2000:
	//%read_minc_file ("VentGradLevel.blur.mask.mnc", maskdata);
	//%CombineVols ("subrect", voxdataflat, maskdata, maskdata, xdim, ydim, zdim);
	//%write_minc ("PialTrough_Trimmed.mnc", voxdataflat, xdim, ydim, zdim);
	//%delete [] maskdata;
   VolumeFile::performMathematicalOperation(
                    VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                    &voxdataflat,
                    &maskdata,
                    &maskdata,
                    &voxdataflat);
   voxdataflat.stretchVoxelValues();
   writeDebugVolume(voxdataflat, "PialTrough_noCerebralWM");
   
   //
   // 7/24/2008 DLD added species condition to reduce chimp occipital smearing 
   //
   if (brainSet->getSpecies().isHuman()) {
      maskdata = *ventGradLevelBlurVolume;
      VolumeFile::performMathematicalOperation(
                       VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                       &voxdataflat,
                       &maskdata,
                       &maskdata,
                       &voxdataflat);
      voxdataflat.stretchVoxelValues();
   }
   writeDebugVolume(voxdataflat, "PialTrough_Trimmed");
   
   freeVolumeInMemory(ventGradLevelBlurVolume);
   
	//17 CombineVols.py subrect InOutDiff.mag.mnc PialTrough_Trimmed.mnc InOutDiff_noPialTrough.mnc
	//%CombineVols ("subrect", rpmdata, voxdataflat, voxdataflat, xdim, ydim, zdim);
	//%write_minc ("InOutDiff_noPialTrough.mnc", rpmdata, xdim, ydim, zdim);
	//%write_minc ("RadialPositionMap.mnc", rpmdata, xdim, ydim, zdim);
	//%delete [] voxdataflat;
   VolumeFile::performMathematicalOperation(
                    VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                    &rpmdata,
                    &voxdataflat,
                    &voxdataflat,
                    &rpmdata);
   rpmdata.stretchVoxelValues();
   writeDebugVolume(&rpmdata, "InOutDiff_noPialTrough");
   try {
      QString fileNameWritten, dataFileNameWritten;
      rpmdata.setDescriptiveLabel("RadialPositionMap");
      VolumeFile::writeVolumeFile(&rpmdata,
                                  typeOfVolumeFilesToWrite,
                                  "RadialPositionMap",
                                  false,
                                  fileNameWritten,
                                  dataFileNameWritten);
      
      //rpmdata.writeFile("RadialPositionMap+orig.HEAD");
      //brainSet->writeVolumeFile("RadialPositionMap+orig.HEAD",
      //                          VolumeFile::FILE_READ_WRITE_TYPE_AFNI,
      //                          VolumeFile::VOLUME_TYPE_FUNCTIONAL,
      //                          &rpmdata);
   }
   catch (FileException& e) {
      throw BrainModelAlgorithmException(e.whatQString());
   }
   
	//18 Thresh.py InOutDiff_noPialTrough.mnc InOutDiff_noPialTrough.thresh.mnc InOutDiffThresh
	//%ThresholdVolume (rpmdata, InOutDiffThresh, xdim,ydim,zdim);
	//%write_minc ("InOutDiff_noPialTrough.thresh.mnc", rpmdata, xdim, ydim, zdim);
   rpmdata.thresholdVolume(static_cast<int>(inOutDiffThresh));
   writeDebugVolume(rpmdata, "InOutDiff_noPialTrough.thresh");

	//19 FillBiggestObject.py InOutDiff_noPialTrough.thresh.mnc InOutDiff_final.mnc 0 ncol 0 nrow 0 nslices 
	//%FindBiggestObjectWithinMask (rpmdata, xdim, ydim, zdim, 0, xdim, 0, ydim, 0, zdim, seed);
	//%if (( seed[0] == -1 ) || ( seed[1] == -1 ) || ( seed[2] == -1 )) {
	//%	printf("Exiting: Null volume\n");
	//%	return 1;
	//%}
	//%vtkFloodFill (seed, rpmdata, 255, 255, 0, xdim, ydim, zdim);
	//%write_minc ("InOutDiff_final.mnc", rpmdata, xdim, ydim, zdim);
	// Added 10/13/2000: FillHoles.py InOutDiff_final.mnc InOutDiff_final.fill.mnc
	//%NewDoFillHoles (xdim, ydim, zdim, rpmdata);
	//%write_minc ("InOutDiff_final.fill.mnc", rpmdata, xdim, ydim, zdim);
	//%write_minc ("Segment.mnc", rpmdata, xdim, ydim, zdim);
   rpmdata.findBiggestObjectWithinMask(0, xDim,
                                       0, yDim,
                                       0, zDim,
                                       255.0, 255.0,
                                       voxelSeed);
   if (voxelSeed.getI() < 0) {
      throw BrainModelAlgorithmException(
         "findBiggestObjectWithinMask() failed to find biggest object when\n"
         "trying to create intermediate volume \"InOutDiff_final\".");
   }
   rpmdata.floodFillWithVTK(voxelSeed, 255, 255, 0);
   writeDebugVolume(rpmdata, "InOutDiff_final");
   rpmdata.fillSegmentationCavities();
   writeDebugVolume(rpmdata, "InOutDiff_final.fill");
   writeDebugVolume(rpmdata, "Segment");
   segmentationVolume = new VolumeFile(rpmdata);

	//TO-DO: volume.AddMincHeaderLine ("correction_cycle", "0", segfinalfname)
	//20 (moved/changed after 21 10/25/2000):
	//MakeShell.py 1 0 Segmentation.mnc Segmentation.shell.mnc
	//%MakeShell (rpmdata, xdim, ydim, zdim, 1, 0);
	//%write_minc ("Segmentation.shell.mnc", rpmdata, xdim, ydim, zdim);
	//%delete [] rpmdata;
   rpmdata.makeShellVolume(1, 0);
   rpmdata.stretchVoxelValues();
   writeDebugVolume(rpmdata, "Segmentation.shell");
	//TO-DO: GenerateVolumeSpecFile ()
	//TO-DO: Clean up files

	//%printf("#### GenerateSegmentation end ####\n");
	//%return 0;
   if (DebugControl::getDebugOn()) {
	   std::cout << "#### GenerateSegmentation end ####" << std::endl;
   }
}

/**
 * fill ventricles.
 */
void 
BrainModelVolumeSureFitSegmentation::fillVentricles() throw (BrainModelAlgorithmException)
{
	if (DebugControl::getDebugOn()) {
      std::cout << "#### FillVentricles begin ####" << std::endl;
   }
	//1 VentricleThreshTemp=$CSFThresh
	//%int VentricleThreshTemp=int(lp.CSFThresh)+15;
   int ventricleThreshTemp = static_cast<int>(csfThresh) + 15;
   
	//2 MakePlane.py 1 $ACx 0 $ACy 0 $ACz 0 2 $Segment_file MidlineSlab
	//%int xdim=lp.xdim;
	//%int ydim=lp.ydim;
	//%int zdim=lp.zdim;
	//%int ACx=lp.ACx;
	//%int ACy=lp.ACy;
	//%int ACz=lp.ACz;
	//%int num_voxels=xdim*ydim*zdim;
	//%unsigned char* inputdata=new unsigned char [num_voxels];
	//%unsigned char* voxdataflat=new unsigned char [num_voxels];
	//%unsigned char* segdata=new unsigned char [num_voxels];
	//%read_minc_file (mincfile, inputdata);
	//%read_minc_file (segfname, segdata);
	//%unsigned char* plane=new unsigned char [num_voxels];
	//%MakePlane (plane, 1.0, float(ACx), 0.0, float(ACy), 0.0, 
	//%	float(ACz), 0.0, 2.0, xdim, ydim, zdim);
	//%write_minc ("MidlineSlab.mnc", plane, xdim, ydim, zdim);
   VolumeFile inputdata = *anatomyVolume;
   VolumeFile voxdataflat = inputdata;
   voxdataflat.setAllVoxels(0.0);
   VolumeFile segdata = *segmentationVolume;
   VolumeFile plane = inputdata;
   plane.setAllVoxels(0.0);
   plane.makePlane(1.0, acIJK[0], 0.0, acIJK[1], 0.0, acIJK[2], 0.0, 2.0);
   plane.stretchVoxelValues();
   writeDebugVolume(plane, "MidlineSlab");
   
	//3 MaskVol.py  MidlineSlab.mnc MidlineSlab.mask $xAC_1_low $xAC_1_high `expr $ACy - 40` `expr $ACy + 30` 0 `expr $ACz + 30`
	//%int extent[6];
	//%extent[0]=lp.xAC_1_low;
	//%extent[1]=lp.xAC_1_high;
	//%extent[2]=ACy-40;
	//%extent[3]=ACy+30;
	//%extent[4]=0;
	//%extent[5]=ACz+30;
	//%MaskVolume (plane, xdim, ydim, zdim, extent);
	//%write_minc ("MidlineSlab.mask.mnc", plane, xdim, ydim, zdim);
   int extent[6];
   extent[0] = xAC_1_low;
   extent[1] = xAC_1_high;
   extent[2] = acIJK[1] - 40;
   extent[3] = acIJK[1] + 30;
   extent[4] = 0;
   extent[5] = acIJK[2] + 30;
   plane.maskVolume(extent);
   plane.stretchVoxelValues();
   writeDebugVolume(plane, "MidlineSlab.mask");

	//4 while [ $VentricleFoundFlag == 0 ] ; do 
	//       sh -x NEW.TEST_VENTRICLE_LIMITS.sh ; . Volume_ID_limits ;
   //  done
	//%char filename[256];
	//%int seed[3], i, VentricleFoundFlag=FALSE;
	//%while ( VentricleFoundFlag==FALSE ) {
   
   int seed[3] = { -1, -1, -1 };
   bool ventricleFoundFlag = false;
   while (ventricleFoundFlag == false) {
		//%for ( i=0 ; i<num_voxels ; i++ ) voxdataflat[i]=inputdata[i];
		//%VentricleThreshTemp=VentricleThreshTemp-5;
      voxdataflat = inputdata;
      ventricleThreshTemp -= 5;

		//2 InvertThresh.py $fname Ventricle.Temp.Thresh.mnc $VentricleThreshTemp
		//%InverseThresholdVolume (voxdataflat, VentricleThreshTemp, xdim, ydim, zdim);
		//%sprintf (filename, "Ventricle.Temp.Thresh%d.mnc", VentricleThreshTemp);
		//TO-DO: push filename onto cleanup stack 
		//%write_minc (filename, voxdataflat, xdim, ydim, zdim);
      voxdataflat.inverseThresholdVolume(static_cast<int>(ventricleThreshTemp));
      std::ostringstream str;
      str << "Ventricle.Temp.Thresh"
          << ventricleThreshTemp;
      writeDebugVolume(voxdataflat, str.str().c_str());
      
		//3 MaskVol.py Ventricle.Temp.Thresh.mnc Ventricle.Temp.Thresh.PadMask $PadNegX `expr $ncol - $PadPosX` $PadNegY `expr $nrow - $PadPosY`	$PadNegZ `expr $nslices - $PadPosZ`
		//%extent[0]=lp.OldPadNegX;
		//%extent[1]=xdim-lp.OldPadPosX;
		//%extent[2]=lp.OldPadNegY;
		//%extent[3]=ydim-lp.OldPadPosY;
		//%extent[4]=lp.OldPadNegZ;
		//%extent[5]=zdim-lp.OldPadPosZ;
		//%MaskVolume (voxdataflat, xdim, ydim, zdim, extent);
		//%write_minc ("Ventricle.Temp.Thresh.PadMask.mnc", voxdataflat, xdim, ydim, zdim);
      
      //extent[0] = oldPadNegX;
      //extent[1] = xDim - oldPadPosX;
      //extent[2] = oldPadNegY;
      //extent[3] = yDim - oldPadPosY;
      //extent[4] = oldPadNegZ;
      //extent[5] = zDim - oldPadPosZ;
      extent[0] = partialHemispherePadding[0];
      extent[1] = xDim - partialHemispherePadding[1];
      extent[2] = partialHemispherePadding[2];
      extent[3] = yDim - partialHemispherePadding[3];
      extent[4] = partialHemispherePadding[4];
      extent[5] = zDim - partialHemispherePadding[5];
      voxdataflat.maskVolume(extent);
      voxdataflat.stretchVoxelValues();
      writeDebugVolume(voxdataflat, "Ventricle.Temp.Thresh.PadMask");

		//4 CombineVols.py subrect Ventricle.Temp.Thresh.PadMask.mnc MidlineSlab.mask.mnc Ventricle.Temp.Thresh_MidlineCut
		//%CombineVols ("subrect", voxdataflat, plane, plane, xdim, ydim, zdim);
		//%sprintf (filename, "Ventricle.Temp.Thresh%d_MidlineCut.mnc", VentricleThreshTemp);
		//TO-DO: push filename onto cleanup stack 
		//%write_minc (filename, voxdataflat, xdim, ydim, zdim);
      VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE,
                                               &voxdataflat,
                                               &plane,
                                               &plane,
                                               &voxdataflat);
      voxdataflat.stretchVoxelValues();
      str.str("");
      str << "Ventricle.Temp.Thresh"
          << ventricleThreshTemp
          << "_MidlineCut";
      writeDebugVolume(voxdataflat, str.str().c_str());

		//5 FillBiggestObject.py Ventricle.Temp.Thresh_MidlineCut.mnc Ventricle.TestFlood $xAC_20_low $xAC_20_high `expr $ACy - 20` `expr $ACy + 20` `expr $ACz + 20` `expr $ACz + 40`
		//%extent[0]=lp.xAC_20_low;
		//%extent[1]=lp.xAC_20_high;
		//%extent[2]=ACy-20;
		//%extent[3]=ACy+20;
		//%extent[4]=ACz+20;
		//%extent[5]=ACz+40;
		//%FindBiggestObjectWithinMask (voxdataflat, xdim, ydim, zdim, extent[0],extent[1],extent[2],extent[3],extent[4],extent[5], seed);
		//%if (( seed[0] == -1 ) || ( seed[1] == -1 ) || ( seed[2] == -1 )) {
		//%	printf("Ventricle not found within limits; exiting FillVentricles\n");
		//%	break;
		//%}
		//%vtkFloodFill (seed, voxdataflat, 255, 255, 0, xdim, ydim, zdim);
		//%sprintf (filename, "Ventricle.TestFlood%d.mnc", VentricleThreshTemp);
		//TO-DO: push filename onto cleanup stack 
		//%write_minc (filename, voxdataflat, xdim, ydim, zdim);
      extent[0] = xAC_20_low;
      extent[1] = xAC_20_high;
      extent[2] = acIJK[1] - 20;
      extent[3] = acIJK[1] + 20;
      extent[4] = acIJK[2] + 20;
      extent[5] = acIJK[2] + 40;
      VoxelIJK seedIJK(seed);
      voxdataflat.findBiggestObjectWithinMask(extent, 255, 255, seedIJK);
      seed[0] = seedIJK.getI();
      seed[1] = seedIJK.getJ();
      seed[2] = seedIJK.getK();
      if ((seed[0] < 0) ||
          (seed[1] < 0) ||
          (seed[2] < 0)) {
         if (DebugControl::getDebugOn()) {
            std::cout << "Ventricle not found within limits; exiting FillVentricles" << std::endl;
         }
         break;
      }
      voxdataflat.floodFillWithVTK(seedIJK, 255, 255, 0);
      str.str("");
      str << "Ventricle.TestFlood"
          << ventricleThreshTemp;
      writeDebugVolume(voxdataflat, str.str().c_str());

		//6 FindLimits.py Ventricle.TestFlood.mnc Ventricle.limits
		//%FindLimits (xdim, ydim, zdim, voxdataflat, "Ventricle.limits", extent);
      voxdataflat.findLimits("Ventricle.limits", extent);

		//7 if [ $LimitZmin -gt `expr $ACz - 20` ] ; then echo "VENTRICLE DISCONNECTED FROM CSF" ; exit; fi
		//%if (extent [4] > (ACz-20)) {VentricleFoundFlag=TRUE;}
		//%else {printf("VENTRICLE NOT YET DISCONNECTED\n");}
      if (extent[4] > (acIJK[2] - 20)) {
         ventricleFoundFlag = true;
      }
      else {
         if (DebugControl::getDebugOn()) {
            std::cout << "VENTRICLE NOT YET DISCONNECTED" << std::endl;
         }
      }
	}
   
	if (ventricleFoundFlag == false) {
		//%printf("VENTRICLE FAILED TO DISCONNECT FROM CSF\n");
      if (DebugControl::getDebugOn()) {
         std::cout << "VENTRICLE FAILED TO DISCONNECT FROM CSF" << std::endl;
      }
	} 
   else {
		//%printf("VENTRICLE DISCONNECTED FROM CSF\n");
      if (DebugControl::getDebugOn()) {
         std::cout << "VENTRICLE DISCONNECTED FROM CSF" << std::endl;
      }
		//5 cp Ventricle.TestFlood.mnc Ventricle.flood.mnc
		//%write_minc ("Ventricle.flood.mnc", voxdataflat, xdim, ydim, zdim);
      writeDebugVolume(voxdataflat, "Ventricle.flood");

		//6 Sculpt.py 2 5 $xAClow $xAChigh 0 $nrow 0 $nslices Ventricle.flood.mnc Segment_file Ventricle_notSegmentation
		//%for ( i=0 ; i<3 ; i++ ) seed[i]=0;
		//%extent[0]=lp.xAClow;
		//%extent[1]=lp.xAChigh;
		//%extent[2]=0;
		//%extent[3]=ydim;
		//%extent[4]=0;
		//%extent[5]=zdim;
		//%Sculpt (2, 5, seed, extent, voxdataflat, segdata, xdim, ydim, zdim);
		//%write_minc ("Ventricle_notSegmentation.mnc", voxdataflat, xdim, ydim, zdim);
      seed[0] = 0;
      seed[1] = 0;
      seed[2] = 0;
      extent[0] = xAClow;
      extent[1] = xAChigh;
      extent[2] = 0;
      extent[3] = yDim;
      extent[4] = 0;
      extent[5] = zDim;
      voxdataflat.sculptVolume(2, &segdata, 5, seed, extent);
      voxdataflat.stretchVoxelValues();
      writeDebugVolume(voxdataflat, "Ventricle_notSegmentation");

		//7 CombineVols.py OR $Segment_file Ventricle_notSegmentation.mnc Segmentation_Ventricle
		//%CombineVols ("OR", segdata, voxdataflat, voxdataflat, xdim, ydim, zdim);
		//%QString segventfname=segfname;
		//%segventfname.replace (segventfname.find(".mnc"),4,"_vent.mnc");
		//%write_minc ((char *)segventfname, segdata, xdim, ydim, zdim);
      VolumeFile::performMathematicalOperation(VolumeFile::VOLUME_MATH_OPERATION_OR,
                                               &segdata,
                                               &voxdataflat,
                                               &voxdataflat,
                                               &segdata);
      segdata.stretchVoxelValues();
      writeDebugVolume(segdata, "Segment_vent");
      segmentationVentriclesFilledVolume = new VolumeFile(segdata);
	}
/*   
	delete [] inputdata;
	delete [] voxdataflat;
	delete [] segdata;
	delete [] plane;
*/   
   if (DebugControl::getDebugOn()) {
	   std::cout << "#### FillVentricles end ####" << std::endl;
   }
}

/**
 * write the vector file for debugging.
 */
void 
BrainModelVolumeSureFitSegmentation::writeDebugVector(SureFitVectorFile& vf, 
                              const QString& nameIn) throw (BrainModelAlgorithmException)
{
   if (DebugControl::getDebugOn()) {
      try {
         QString name;
         QDir intermedDir(segmentationDebugFilesSubDirectory);
         if (intermedDir.exists()) {
            name.append(segmentationDebugFilesSubDirectory);
            name.append("/");
         }
         name.append(nameIn);
         name.append(SpecFile::getSureFitVectorFileExtension());
         vf.writeFile(name);
         std::cout << "Write Debug Vector File: " << name.toAscii().constData() << std::endl;
      }
      catch (FileException& e) {
         throw (BrainModelAlgorithmException(e.whatQString()));
      }
   }
}

/**
 * write the volume.
 */
void 
BrainModelVolumeSureFitSegmentation::writeDebugVolume(VolumeFile& vf, 
                               const QString& nameIn) throw (BrainModelAlgorithmException)
{
   writeDebugVolume(&vf, nameIn);
}
      
/**
 * write the volume.
 */
void 
BrainModelVolumeSureFitSegmentation::writeDebugVolume(VolumeFile* vf, 
                               const QString& nameIn) throw (BrainModelAlgorithmException)
{
   if (DebugControl::getDebugOn()) {
      try {
         vf->setDescriptiveLabel(nameIn);
         QString name;
         QDir intermedDir(segmentationDebugFilesSubDirectory);
         if (intermedDir.exists()) {
            name.append(segmentationDebugFilesSubDirectory);
            name.append("/");
         }
         name.append(nameIn);
         
         QString fileNameWritten;
         QString dataFileNameWritten;
         VolumeFile::writeVolumeFile(vf,
                                     typeOfVolumeFilesToWrite,
                                     name,
                                     false,
                                     fileNameWritten,
                                     dataFileNameWritten);
                                     
         //name.append(SpecFile::getAfniVolumeFileExtension());
         //vf->writeFile(name);
         std::cout << "Write Debug Volume File: " << fileNameWritten.toAscii().constData() << std::endl;
      }
      catch (FileException& e) {
         throw (BrainModelAlgorithmException(e.whatQString()));
      }
   }
}      

/**
 * get parameters from the parameters file.
 */
void 
BrainModelVolumeSureFitSegmentation::getParameters() throw (BrainModelAlgorithmException)
{
   QString msg;


   if (structure == Structure::STRUCTURE_TYPE_INVALID) {
/*
      int hem;
      if (pf->getParameter(ParamsFile::keyHem, hem)) {
         if (hem == 0) {
            hemisphere = Structure::STRUCTURE_TYPE_CORTEX_LEFT;
         }
         else if(hem == 1) {
            hemisphere = Structure::STRUCTURE_TYPE_CORTEX_RIGHT;
         }
      }
*/      
      if (structure == Structure::STRUCTURE_TYPE_INVALID) {
         msg.append("Unable to determine structure.\n");
      }
   }
   
   if (msg.isEmpty() == false) {
      throw BrainModelAlgorithmException(msg);
   }
   
   //
   // Get the volume's dimensions
   //
   if (anatomyVolume != NULL) {
      anatomyVolume->getDimensions(xDim, yDim, zDim);
   }
   else {
      segmentationVolume->getDimensions(xDim, yDim, zDim);
   }
   
   //
   // Some structure flags
   //
   Hem = 0;
   if (structure == Structure::STRUCTURE_TYPE_CORTEX_RIGHT) {
      Hem = 1;
   }
   Hem1   = Hem;
   Hem2   = 1 - Hem1;
   HemDbl = 2 * Hem1;
   Hem3   = HemDbl - 1;
   
   //
   // Some AC relationships
   //
   xAC_1 = acIJK[0] + Hem3 * 1;
	xAClow = acIJK[0] * Hem1;
	xAChigh = xDim * Hem1 + acIJK[0] * Hem2;
	xAC_10 = acIJK[0] + Hem3 * 10;
   xAC_15 = acIJK[0] + Hem3 * 15;
   xAC_20 = acIJK[0] + Hem3 * 20;
   xAC_40 = acIJK[0] + Hem3 * 40;
   xAC_1_low  = xAC_1 * Hem2 + acIJK[0] * Hem1;
	xAC_1_high = xAC_1 * Hem1 + acIJK[0] * Hem2;
   xAC_15_40_low  = xAC_40 * Hem2 + xAC_15 * Hem1;
   xAC_15_40_high = xAC_40 * Hem1 + xAC_15 * Hem2;
   xAC_20_low  = xAC_20 * Hem2 + acIJK[0] * Hem1;
	xAC_20_high = xAC_20 * Hem1 + acIJK[0] * Hem2;
   xAC_50 = acIJK[0] + Hem3 * 50;
	xMedLimit_50_low  = xAC_50 * Hem2;
	xMedLimit_50_high = xAC_50 * Hem1 + xDim * Hem2;
   xMedLimit_20_low  = xAC_20 * Hem2;
   xMedLimit_20_high = xAC_20 * Hem1 + xDim * Hem2;
   xMedLimit_low  = acIJK[0] * Hem2;
	xMedLimit_high = acIJK[0] * Hem1 + xDim * Hem2;

	inITPeak= (wmPeak + cgmPeak) / 2.0;
	inITLow = cgmPeak;
	inITHigh = wmPeak;
	inITSignum = 2.0;

	cgmLow = cgmPeak / 2.0;
	cgmHigh = inITPeak;
	cgmSignum = 1.3;

   csfThresh = cgmPeak / 2.0;

   outITPeak = cgmPeak / 2.0;
   outITLow  = csfThresh * 0.5;
   outITHigh = cgmPeak;
   outITSignum = 2.0;
   
   if (DebugControl::getDebugOn()) {
      std::cout << "Hem: " << Hem << std::endl;
      std::cout << "Hem2: " << Hem2 << std::endl;
      std::cout << "HemDbl: " << HemDbl << std::endl;
      std::cout << "Hem3: " << Hem3 << std::endl;
      std::cout << "xAC_1: " << xAC_1 << std::endl;
      std::cout << "xAC_10: " << xAC_10 << std::endl;
      std::cout << "xAC_15: " << xAC_15 << std::endl;
      std::cout << "xAC_20: " << xAC_20 << std::endl;
      std::cout << "xAC_40: " << xAC_40 << std::endl;
      std::cout << "xAC_50: " << xAC_50 << std::endl;
      std::cout << "xAC_1_low: " << xAC_1_low << std::endl;
      std::cout << "xAC_1_high: " << xAC_1_high << std::endl;
      std::cout << "xAC_20_low: " << xAC_20_low << std::endl;
      std::cout << "xAC_20_high: " << xAC_20_high << std::endl;
      std::cout << "xAC_15_40_low: " << xAC_15_40_low << std::endl;
      std::cout << "xAC_15_40_high: " << xAC_15_40_high << std::endl;
      std::cout << "xMedLimit_20_low: " << xMedLimit_20_low << std::endl;
      std::cout << "xMedLimit_20_high: " << xMedLimit_20_high << std::endl;
      std::cout << "xMedLimit_50_low: " << xMedLimit_50_low << std::endl;
      std::cout << "xMedLimit_50_high: " << xMedLimit_50_high << std::endl;
      std::cout << "xAClow: " << xAClow << std::endl;
      std::cout << "xAChigh: " << xAChigh << std::endl;
      std::cout << "xMedLimit_low: " << xMedLimit_low << std::endl;
      std::cout << "xMedLimit_high: " << xMedLimit_high << std::endl;
      std::cout << "AC: " << acIJK[0] << ", " << acIJK[1] << ", " << acIJK[2] << std::endl;
      std::cout << "wmPeak: " << wmPeak << std::endl;
      std::cout << "cgmPeak: " << cgmPeak << std::endl;
      std::cout << "wmThresh: " << wmThresh << std::endl;
   }
}
      
