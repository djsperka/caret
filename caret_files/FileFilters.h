
#ifndef __FILE_FILTERS_H__
#define __FILE_FILTERS_H__

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

#include "SpecFile.h"

/// class for file filters
class FileFilters {
   public:
      /// get all file filters
      static void getAllFileFilters(QStringList& allFileFiltersOut);
      
      /// get all caret file filters
      static void getAllCaretFileFilters(QStringList& allFileFiltersOut);
      
      /// file filer for  file
      static QString getAnyFileFilter() { return "Any File (*)"; }
      
      /// file filer for  file
      static QString getAreaColorFileFilter() { return QString("Area Color File (*%1)").arg(SpecFile::getAreaColorFileExtension()); }
      
      /// file filer for  file
      static QString getArealEstimationFileFilter() { return QString("Areal Estimation Files (*%1)").arg(SpecFile::getArealEstimationFileExtension()); }
      
      /// file filer for  file
      static QString getBorderRawFileFilter() { return QString("Border Files RAW (*%1)").arg(SpecFile::getBorderFileExtension()); }
      
      /// file filer for  file
      static QString getBorderFiducialFileFilter() { return QString("Border Files FIDUCIAL (*%1)").arg(SpecFile::getBorderFileExtension()); }
      
      /// file filer for  file
      static QString getBorderInflatedFileFilter() { return QString("Border Files INFLATED (*%1)").arg(SpecFile::getBorderFileExtension()); }
      
      /// file filer for  file
      static QString getBorderVeryInflatedFileFilter() { return QString("Border Files VERY INFLATED (*%1)").arg(SpecFile::getBorderFileExtension()); }
      
      /// file filer for  file
      static QString getBorderSphericalFileFilter() { return QString("Border Files SPHERICAL (*%1)").arg(SpecFile::getBorderFileExtension()); }
      
      /// file filer for  file
      static QString getBorderEllipsoidalFileFilter() { return QString("Border Files ELLIPSOIDAL (*%1)").arg(SpecFile::getBorderFileExtension()); }
      
      /// file filer for  file
      static QString getBorderCompressedFileFilter() { return QString("Border Files COMPRESSED (*%1)").arg(SpecFile::getBorderFileExtension()); }
      
      /// file filer for  file
      static QString getBorderFlatFileFilter() { return QString("Border Files FLAT (*%1)").arg(SpecFile::getBorderFileExtension()); }
      
      /// file filer for  file
      static QString getBorderFlatLobarFileFilter() { return QString("Border Files FLAT LOBAR (*%1)").arg(SpecFile::getBorderFileExtension()); }
      
      /// file filer for  file
      static QString getBorderHullFileFilter() { return QString("Border Files HULL (*%1)").arg(SpecFile::getBorderFileExtension()); }
      
      /// file filer for  file
      static QString getBorderUnknownFileFilter() { return QString("Border Files UNKNOWN (*%1)").arg(SpecFile::getBorderFileExtension()); }
      
      /// file filer for  file
      static QString getBorderGenericFileFilter() { return QString("Border Files - Surface (*%1)").arg(SpecFile::getBorderFileExtension()); }
      
      /// file filer for  file
      static QString getBorderVolumeFileFilter() { return QString("Border Files - Volume (*%1)").arg(SpecFile::getBorderFileExtension()); }
      
      /// file filer for  file
      static QString getBorderColorFileFilter() { return QString("Border Color Files (*%1)").arg(SpecFile::getBorderColorFileExtension()); }
      
      /// file filer for  file
      static QString getBorderProjectionFileFilter() { return QString("Border Projection Files (*%1)").arg(SpecFile::getBorderProjectionFileExtension()); }
      
      /// file filer for  file
      static QString getCellFileFilter() { return QString("Cell Files (*%1)").arg(SpecFile::getCellFileExtension()); }
      
      /// file filer for  file
      static QString getCellColorFileFilter() { return QString("Cell Color Files (*%1)").arg(SpecFile::getCellColorFileExtension()); }
      
      /// file filer for  file
      static QString getCellProjectionFileFilter() { return QString("Cell Projection Files (*%1)").arg(SpecFile::getCellProjectionFileExtension()); }
      
      /// file filer for  file
      static QString getCellVolumeFileFilter() { return QString("Cell Files - Volume (*%1)").arg(SpecFile::getCellFileExtension()); }
      
      /// file filer for  file
      static QString getCocomacFileFilter() { return QString("CoCoMac Files (*%1 *.xml)").arg(SpecFile::getCocomacConnectivityFileExtension()); }
      
      /// file filer for  file
      static QString getContourFileFilter() { return QString("Contour Files (*%1)").arg(SpecFile::getContourFileExtension()); }
      
      /// file filer for  file
      static QString getContourCellFileFilter() { return QString("Contour Cell Files (*%1)").arg(SpecFile::getContourCellFileExtension()); }
      
      /// file filer for  file
      static QString getContourCellColorFileFilter() { return QString("Contour Cell Color Files (*%1)").arg(SpecFile::getContourCellColorFileExtension()); }
      
      /// file filer for  file
      static QString getCoordinateRawFileFilter() { return QString("Coordinate Files RAW (*%1)").arg(SpecFile::getCoordinateFileExtension()); }
      
      /// file filer for  file
      static QString getCoordinateFiducialFileFilter() { return QString("Coordinate Files FIDUCIAL (*%1)").arg(SpecFile::getCoordinateFileExtension()); }
      
      /// file filer for  file
      static QString getCoordinateInflatedFileFilter() { return QString("Coordinate Files INFLATED (*%1)").arg(SpecFile::getCoordinateFileExtension()); }
      
      /// file filer for  file
      static QString getCoordinateVeryInflatedFileFilter() { return QString("Coordinate Files VERY INFLATED (*%1)").arg(SpecFile::getCoordinateFileExtension()); }
      
      /// file filer for  file
      static QString getCoordinateSphericalFileFilter() { return QString("Coordinate Files SPHERICAL (*%1)").arg(SpecFile::getCoordinateFileExtension()); }
      
      /// file filer for  file
      static QString getCoordinateEllipsoidalFileFilter() { return QString("Coordinate Files ELLIPSOIDAL (*%1)").arg(SpecFile::getCoordinateFileExtension()); }
      
      /// file filer for  file
      static QString getCoordinateCompressedFileFilter() { return QString("Coordinate Files COMPRESSED (*%1)").arg(SpecFile::getCoordinateFileExtension()); }
      
      /// file filer for  file
      static QString getCoordinateFlatFileFilter() { return QString("Coordinate Files FLAT (*%1)").arg(SpecFile::getCoordinateFileExtension()); }
      
      /// file filer for  file
      static QString getCoordinateFlatLobarFileFilter() { return QString("Coordinate Files FLAT LOBAR (*%1)").arg(SpecFile::getCoordinateFileExtension()); }
      
      /// file filer for  file
      static QString getCoordinateHullFileFilter() { return QString("Coordinate Files HULL (*%1)").arg(SpecFile::getCoordinateFileExtension()); }
      
      /// file filer for  file
      static QString getCoordinateUnknownFileFilter() { return QString("Coordinate Files UNKNOWN (*%1)").arg(SpecFile::getCoordinateFileExtension()); }
      
      /// file filer for  file
      static QString getCoordinateGenericFileFilter() { return QString("Coordinate Files (*%1)").arg(SpecFile::getCoordinateFileExtension()); }
      
      /// file filer for  file
      static QString getCutsFileFilter() { return QString("Cuts Files (*%1)").arg(SpecFile::getCutsFileExtension()); }

      /// file filer for  file
      static QString getDeformationFieldFileFilter() { return QString("Deformation Field Files (*%1)").arg(SpecFile::getDeformationFieldFileExtension()); }
      
      /// file filer for  file
      static QString getDeformationMapFileFilter() { return QString("Deformation Map Files (*%1)").arg(SpecFile::getDeformationMapFileExtension()); }
      
      /// file filer for  file
      static QString getFociFileFilter() { return QString("Foci Files (*%1 *%2)").arg(SpecFile::getFociFileExtension()).arg(
                                                                                      SpecFile::getFociFileExtension() + ".csv"); }
      
      /// file filer for  file
      static QString getFociColorFileFilter() { return QString("Foci Color Files (*%1)").arg(SpecFile::getFociColorFileExtension()); }
      
      /// file filer for  file
      static QString getFociProjectionFileFilter() { return QString("Foci Projection Files (*%1)").arg(SpecFile::getFociProjectionFileExtension()); }
      
      /// file filer for  file
      static QString getFociSearchFileFilter() { return QString("Foci Search Files (*%1)").arg(SpecFile::getFociSearchFileExtension()); }
      
      /// file filer for  file
      static QString getGeodesicDistanceFileFilter() { return QString("Geodesic Distance Files (*%1)").arg(SpecFile::getGeodesicDistanceFileExtension()); }
      
      /// file filer for  file
      static QString getLatitudeLongitudeFileFilter() { return QString("Latitude Longitude Files (*%1)").arg(SpecFile::getLatLonFileExtension()); }
      
      /// file filer for  file
      static QString getMetricFileFilter() { return QString("Metric Files (*%1)").arg(SpecFile::getMetricFileExtension()); }
      
      /// file filer for metric or shape as metric file
      static QString getMetricOrShapeAsMetricFileFilter() { return QString("Metric [or Shape] Files (*%1 *%2 *%3 *%4)").arg(SpecFile::getMetricFileExtension()).arg(SpecFile::getSurfaceShapeFileExtension()).arg(SpecFile::getGiftiFunctionalFileExtension()).arg(SpecFile::getGiftiShapeFileExtension()); }
      
      /// file filer for  file
      static QString getMetricShapeFileFilter() { return QString("Metric/Shape Files (*%1 *%2)").arg(SpecFile::getMetricFileExtension()).arg(SpecFile::getSurfaceShapeFileExtension()); }
      
      /// file filer for  file
      static QString getPaintFileFilter() { return QString("Paint Files (*%1)").arg(SpecFile::getPaintFileExtension()); }
      
      /// file filer for  file
      static QString getPaletteFileFilter() { return QString("Palette Files (*%1 *.pal)").arg(SpecFile::getPaletteFileExtension()); }
      
      /// file filer for  file
      static QString getParamsFileFilter() { return QString("Params Files (*%1)").arg(SpecFile::getParamsFileExtension()); }
      
      /// file filer for  file
      static QString getProbAtlasFileFilter() { return QString("Probabilistic Atlas Files (*%1 *atlas.paint*)").arg(SpecFile::getProbabilisticAtlasFileExtension()); }
      
      /// file filer for  file
      static QString getRgbPaintFileFilter() { return QString("RGB Paint Files (*%1)").arg(SpecFile::getRgbPaintFileExtension()); }
      
      /// file filer for  file
      static QString getRegionOfInterestFileFilter() { return QString("Region of Interest Files (*%1)").arg(SpecFile::getRegionOfInterestFileExtension()); }
      
      /// file filer for  file
      static QString getSceneFileFilter() { return QString("Scene Files (*%1)").arg(SpecFile::getSceneFileExtension()); }
      
      /// file filer for  file
      static QString getSectionFileFilter() { return QString("Section Files (*%1)").arg(SpecFile::getSectionFileExtension()); }
      
      /// file filer for  file
      static QString getSurfaceShapeFileFilter() { return QString("Surface Shape Files (*%1)").arg(SpecFile::getSurfaceShapeFileExtension()); }
      
      /// file filer for shape or metric as shape file
      static QString getSurfaceShapeOrMetricAsShapeFileFilter() { return QString("Surface Shape [or Metric] Files (*%1 *%2 *%3 *%4)").arg(SpecFile::getMetricFileExtension()).arg(SpecFile::getSurfaceShapeFileExtension()).arg(SpecFile::getGiftiFunctionalFileExtension()).arg(SpecFile::getGiftiShapeFileExtension()); }
      
      /// file filer for  file
      static QString getTopographyFileFilter() { return QString("Topography Files (*%1)").arg(SpecFile::getTopographyFileExtension()); }
      
      /// file filer for  file
      static QString getTopologyClosedFileFilter() { return QString("Topology Files CLOSED (*%1)").arg(SpecFile::getTopoFileExtension()); }
      
      /// file filer for  file
      static QString getTopologyOpenFileFilter() { return QString("Topology Files OPEN (*%1)").arg(SpecFile::getTopoFileExtension()); }
      
      /// file filer for  file
      static QString getTopologyCutFileFilter() { return QString("Topology Files CUT (*%1)").arg(SpecFile::getTopoFileExtension()); }
      
      /// file filer for  file
      static QString getTopologyCutLobarFileFilter() { return QString("Topology Files CUT LOBAR (*%1)").arg(SpecFile::getTopoFileExtension()); }
      
      /// file filer for  file
      static QString getTopologyUnknownFileFilter() { return QString("Topology Files UNKNOWN (*%1)").arg(SpecFile::getTopoFileExtension()); }
      
      /// file filer for  file
      static QString getTopologyGenericFileFilter() { return QString("Topology Files (*%1)").arg(SpecFile::getTopoFileExtension()); }

      /// file filer for  file
      static QString getTrajectoryFileFilter() { return QString("Electrode Trajectory Files (*%1)").arg(SpecFile::getTrajectoryFileExtension()); }

      /// file filer for  file
      static QString getTransformationMatrixFileFilter() { return QString("Transformation Matrix Files (*%1)").arg(SpecFile::getTransformationMatrixFileExtension()); }

      /// file filer for  file
      static QString getTransformationDataFileFilter() { return QString("Transformation Data Files (*%1 *%3 *.vtp *%4 *%5)"
                                                   ).arg(SpecFile::getCellFileExtension()).arg(
                                                         SpecFile::getVtkModelFileExtension()).arg(
                                                         SpecFile::getContourFileExtension()).arg(
                                                         SpecFile::getContourCellFileExtension()); }

      /// file filer for  file
      static QString getVolumeAnatomyFileFilter() { return QString("Volume Anatomy Files (*%1 *%2 *%3 *%4 *%5)").arg(
                                                  SpecFile::getAfniVolumeFileExtension()).arg(
                                                  SpecFile::getAnalyzeVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiGzipVolumeFileExtension()).arg(
                                                  SpecFile::getWustlVolumeFileExtension()); } 
      
      /// file filer for  file
      static QString getVolumeFunctionalFileFilter() { return QString("Volume Functional Files (*%1 *%2 *%3 *%4 *%5)").arg(
                                                  SpecFile::getAfniVolumeFileExtension()).arg(
                                                  SpecFile::getAnalyzeVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiGzipVolumeFileExtension()).arg(
                                                  SpecFile::getWustlVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getVolumePaintFileFilter() { return QString("Volume Paint Files (*%1 *%2 *%3 *%4 *%5)").arg(
                                                  SpecFile::getAfniVolumeFileExtension()).arg(
                                                  SpecFile::getAnalyzeVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiGzipVolumeFileExtension()).arg(
                                                  SpecFile::getWustlVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getVolumeProbAtlasFileFilter() { return QString("Volume Prob Atlas Files (*%1 *%2 *%3 *%4 *%5)").arg(
                                                  SpecFile::getAfniVolumeFileExtension()).arg(
                                                  SpecFile::getAnalyzeVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiGzipVolumeFileExtension()).arg(
                                                  SpecFile::getWustlVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getVolumeRgbFileFilter() { return QString("Volume Rgb Files (*%1 *%2 *%3 *%4 *%5)").arg(
                                                  SpecFile::getAfniVolumeFileExtension()).arg(
                                                  SpecFile::getAnalyzeVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiGzipVolumeFileExtension()).arg(
                                                  SpecFile::getWustlVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getVolumeSegmentationFileFilter() { return QString("Volume Segmentation Files (*%1 *%2 *%3 *%4 *%5)").arg(
                                                  SpecFile::getAfniVolumeFileExtension()).arg(
                                                  SpecFile::getAnalyzeVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiGzipVolumeFileExtension()).arg(
                                                  SpecFile::getWustlVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getVolumeVectorFileFilter() { return QString("Volume Vector Files (*%1 *%2 *%3 *%4 *%5)").arg(
                                                  SpecFile::getAfniVolumeFileExtension()).arg(
                                                  SpecFile::getAnalyzeVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiGzipVolumeFileExtension()).arg(
                                                  SpecFile::getWustlVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getVolumeGenericFileFilter() { return QString("Volume Files (*%1 *%2 *%3 *%4 *%5)").arg(
                                                  SpecFile::getAfniVolumeFileExtension()).arg(
                                                  SpecFile::getAnalyzeVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiVolumeFileExtension()).arg(
                                                  SpecFile::getNiftiGzipVolumeFileExtension()).arg(
                                                  SpecFile::getWustlVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getVolumeFileAfniFilter() { return QString("Volume File - AFNI (*%1)").arg(SpecFile::getAfniVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getVolumeMultiFileAfniFilter() { return QString("Volume Files MULTIPLE - AFNI (*%1)").arg(SpecFile::getAfniVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getVolumeFileNiftiReadFilter() { return QString("Volume Files - NIFTI (*%1 *%2 %3)").arg(SpecFile::getAnalyzeVolumeFileExtension()).arg(
                                                                    SpecFile::getNiftiVolumeFileExtension()).arg(
                                                                    SpecFile::getNiftiGzipVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getVolumeFileNiftiWriteFilter() { return QString("Volume Files - NIFTI (*%1 *%2)").arg(SpecFile::getNiftiVolumeFileExtension()).arg(
                                                                    SpecFile::getNiftiGzipVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getVolumeMultiFileNiftiFilter() { return QString("Volume Files MULTIPLE - NIFTI (*%1 *%2)").arg(SpecFile::getNiftiVolumeFileExtension()).arg(
                                                                    SpecFile::getNiftiGzipVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getVolumeFileSpmMedxFilter() { return QString("Volume Files - SPM and MEDx (*%1)").arg(SpecFile::getAnalyzeVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getVolumeFileWuNilFilter() { return QString("Volume Files - WUNIL (*%1)").arg(SpecFile::getWustlVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getVtkModelFileFilter() { return QString("VTK Model File (*%1 *.vtp)").arg(SpecFile::getVtkModelFileExtension()); }
      
      /// file filer for  file
      static QString getVocabularyFileFilter() { return QString("Vocabulary File (*%1)").arg(SpecFile::getVocabularyFileExtension()); }
      
      /// file filer for  file
      static QString getWustlRegionFileFilter() { return QString("Wustl Region File (*%1)").arg(SpecFile::getWustlRegionFileExtension()); }

      /// file filer for  file
      static QString getStudyCollectionFileFilter() { return QString("Study Collection File (*%1)").arg(SpecFile::getStudyCollectionFileExtension()); }
      
      /// file filer for  file
      static QString getStudyMetaDataFileFilter() { return QString("Study Metadata File (*%1)").arg(SpecFile::getStudyMetaDataFileExtension()); }
      
      /// file filer for  file
      static QString getSpecFileFilter() { return QString("Spec File (*%1)").arg(SpecFile::getSpecFileExtension()); }
      
      /// file filer for  file
      static QString getMdPlotFileFilter() { return QString("MD Plot File (*%1)").arg(SpecFile::getMDPlotFileExtension()); }
      
      /// file filer for  file
      static QString getNeurolucidaFileFilter() { return QString("Neurolucida File (*%1)").arg(SpecFile::getNeurolucidaFileExtension()); }
      
      /// file filer for  file
      static QString getBrainVoyagerFileFilter() { return QString("Brain Voyager File (*%1)").arg(SpecFile::getBrainVoyagerFileExtension()); }
      
      /// file filer for  file
      static QString getVtkSurfaceFileFilter() { return QString("VTK Surface File (*%1)").arg(SpecFile::getVtkModelFileExtension()); }
      
      /// file filer for  file
      static QString getVtkXmlSurfaceFileFilter() { return "VTK XML Surface File (*.vtp)"; }
      
      /// file filer for  file
      static QString getByuSurfaceFileFilter() { return "BYU Surface File (*.byu)"; }
      
      /// file filer for  file
      static QString getStlSurfaceFileFilter() { return "STL Surface File (*.stl)"; }
      
      /// file filer for  file
      static QString getStudio3DSurfaceFileFilter() { return "3D Studio Max Ascii Scene (*.ase)"; }
      
      /// file filer for  file
      static QString getWavefrontObjFileFilter() { return "Wavefront OBJ Surface File (*.obj)"; }
      
      /// file filer for  file
      static QString getVrmlSurfaceFileFilter() { return "VRML Surface File (*.wrl)"; }
      
      /// file filer for  file
      static QString getSumaRgbFileFilter() { return "SUMA RGB file (*.col)"; }
      
      /// file filer for  file
      static QString getTextFileFilter() { return "Text File (*.txt *.text)"; }
      
      /// file filer for  file
      static QString getCommaSeparatedValuesFileFilter() { return QString("Comma Separated Value File (*%1)").arg(SpecFile::getCommaSeparatedValueFileExtension()); }
      
      /// file filer for  file
      static QString getOpenInventorSurfaceFileFilter() { return "Open Inventor Surface File (*.iv)"; }
      
      /// file filer for  file
      static QString getFreeSurferAsciiSurfaceFileFilter() { return QString("Free Surfer Ascii Surface File (*%1)").arg(SpecFile::getFreeSurferAsciiSurfaceFileExtension()); }
      
      /// file filer for  file
      static QString getFreeSurferBinarySurfaceFileFilter() { return QString("Free Surfer Binary Surface File (*)"); }
      
      /// file filer for  file
      static QString getFreeSurferAsciiCurvatureFileFilter() { return 
             QString("Free Surface Ascii Curvature File As Surface Shape (*%1)").arg(SpecFile::getFreeSurferAsciiCurvatureFileExtension()); }
      
      /// file filer for  file
      static QString getFreeSurferBinaryCurvatureFileFilter() { return 
             QString("Free Surface Binary Curvature File As Surface Shape (*%1)").arg(SpecFile::getFreeSurferBinaryCurvatureFileExtension()); }
      
      /// file filer for  file
      static QString getFreeSurferAsciiFunctionalFileFilter() { return 
             QString("Free Surfer Ascii Functional File As Metric(*%1)").arg(SpecFile::getFreeSurferAsciiFunctionalFileExtension()); }
      
      /// file filer for  file
      static QString getFreeSurferBinaryFunctionalFileFilter() { return
             QString("Free Surfer Binary Functional File As Metric(*%1)").arg(SpecFile::getFreeSurferBinaryFunctionalFileExtension()); }
      
      /// file filer for  file
      static QString getFreeSurferAsciiLabelFileFilter() { return 
             QString("Free Surfer Ascii Label File As Paint(*%1)").arg(SpecFile::getFreeSurferLabelFileExtension()); }
             
      /// file filer for  file
      static QString getAnalyzeVolumeFileFilter() { return QString("Analyze Volume File (*%1)").arg(SpecFile::getAnalyzeVolumeFileExtension()); }
      
      /// file filer for  file
      static QString getMincVolumeFileFilter() { return QString("MINC Volume File (*%1)").arg(SpecFile::getMincVolumeFileExtension()); }

      /// file filter for file
      static QString getMniObjSurfaceFileFilter() { return QString("MNI OBJ Surface File (*%1 *%1.txt)").arg(SpecFile::getMniObjeSurfaceFileExtension()); }
      
      /// file filer for  file
      static QString getVtkVolumeFileFilter() { return QString("VTK Volume File (*%1)").arg(SpecFile::getVtkModelFileExtension()); }
      
      /// file filer for  file
      static QString getRawVolumeFileFilter() { return "Raw Volume File (*)"; }

      /// file filer for  file
      static QString getGiftiGenericFileFilter() { return QString("GIfTI Generic File (*%1)").arg(SpecFile::getGiftiGenericFileExtension()); }
      
      /// file filer for  file
      static QString getGiftiCoordinateFileFilter() { return QString("GIfTI Coordinate File (*%1)").arg(SpecFile::getGiftiCoordinateFileExtension()); }
      
      /// file filer for  file
      static QString getGiftiFunctionalFileFilter() { return QString("GIfTI Functional File (*%1)").arg(SpecFile::getGiftiFunctionalFileExtension()); }
      
      /// file filer for  file
      static QString getGiftiLabelFileFilter() { return QString("GIfTI Label File (*%1)").arg(SpecFile::getGiftiLabelFileExtension()); }

      /// file filer for  file
      static QString getGiftiRgbaFileFilter() { return QString("GIfTI RGBA File (*%1)").arg(SpecFile::getGiftiRgbaFileExtension()); }
      
      /// file filer for  file
      static QString getGiftiShapeFileFilter() { return QString("GIfTI Shape File (*%1)").arg(SpecFile::getGiftiShapeFileExtension()); }
      
      /// file filer for  file
      static QString getGiftiSurfaceFileFilter() { return QString("GIfTI Surface File (*%1)").arg(SpecFile::getGiftiSurfaceFileExtension()); }
      
      /// file filer for  file
      static QString getGiftiTensorFileFilter() { return QString("GIfTI Tensor File (*%1)").arg(SpecFile::getGiftiTensorFileExtension()); }
      
      /// file filer for  file
      static QString getGiftiTimeSeriesFileFilter() { return QString("GIfTI Time Series File (*%1)").arg(SpecFile::getGiftiTimeSeriesFileExtension()); }
      
      /// file filer for  file
      static QString getGiftiTopologyFileFilter() { return QString("GIfTI Topology File (*%1)").arg(SpecFile::getGiftiTopologyFileExtension()); }
      
      /// file filter for GIFTI vector file 
      static QString getGiftiVectorFileFilter() { return QString("GIFTI Vector File (*%1)").arg(SpecFile::getGiftiVectorFileExtension()); }

      /// file filters for opening images
      static void getImageOpenFileFilters(QStringList& fileFiltersOut,
                                          QStringList& fileExtensionsOut);

      /// file filter containing all image open formats
      static QString getImageOpenFileFilter();
      
      /// file filters for saving images
      static void getImageSaveFileFilters(QStringList& fileFiltersOut,
                                          QStringList& fileExtensionsOut);
      
      /// file filter containing all image save formats
      static QString getImageSaveFileFilter();
      
      /// file filter for vector file
      static QString getSureFitVectorFileFilter() { return QString("Vector File (%1)").arg(SpecFile::getSureFitVectorFileExtension()); }
      
      /// file filter for Caret Script File
      static QString getCaretScriptFileFilter() { return QString("Caret Script File (*%1)").arg(SpecFile::getCaretScriptFileExtension()); }
     
       /// file filter for PDF file
      static QString getPDFFileFilter() { return "PDF File (*.pdf)"; }
      
      /// file filter for HTML file
      static QString getHTMLFileFilter() { return "HTML File (*.html)"; }
      
      /// file filter for zip file
      static QString getZipFileFilter() { return QString("Zip File (*%1)").arg(SpecFile::getZipFileExtension()); }
      
      /// file filter for python script
      static QString getPythonFileFilter() { return "Python Script File (*.py)"; }
      
      /// file filter for perl script
      static QString getPerlFileFilter() { return "Perl Script File (*.pl)"; }
      
      /// file filter for bourne shell script
      static QString getBourneShellFileFilter() { return "Bourne Shell Script File (*.sh)"; }
      
      /// file filter for C Shell script
      static QString getCShellFileFilter() { return "C-Shell Script File (*.csh)"; }
      
      /// file filter for Windows BAT script
      static QString getWindowsBATFileFilter() { return "Windows DOS Script File (*.bat)"; }      
};

#endif // __FILE_FILTERS_H__

