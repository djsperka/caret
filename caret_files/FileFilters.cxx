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

#include <QImageReader>
#include <QImageWriter>

#include "FileFilters.h"

/**
 * file filters for opening images.
 */
void 
FileFilters::getImageOpenFileFilters(QStringList& fileFiltersOut,
                                     QStringList& fileExtensionsOut)
{
   fileFiltersOut.clear();
   fileExtensionsOut.clear();
   
   for (int i = 0; i < QImageReader::supportedImageFormats().count(); i++) {
      QString str = QString(QImageReader::supportedImageFormats().at(i));
      QString strLower = str.toLower();
      
      const QString f = (str
                         + " Image File (*."
                         + strLower
                         + ")");
      fileFiltersOut << f;
      fileExtensionsOut << strLower;
   }
}

/**
 * file filter containing all image open formats.
 */
QString 
FileFilters::getImageOpenFileFilter()
{
   QStringList filterList;
   
   for (int i = 0; i < QImageReader::supportedImageFormats().count(); i++) {
      QString str = QString(QImageReader::supportedImageFormats().at(i));
      QString strLower = str.toLower();
      filterList << ("*." + strLower);
   }
   
   const QString filters = ("Image File ("
                            + filterList.join(" ")
                            + ")");
   return filters;
}

/**
 * file filters for saving images.
 */
void 
FileFilters::getImageSaveFileFilters(QStringList& fileFiltersOut,
                                     QStringList& fileExtensionsOut)
{
   fileFiltersOut.clear();
   fileExtensionsOut.clear();
   
   for (int i = 0; i < QImageWriter::supportedImageFormats().count(); i++) {
      QString str = QString(QImageWriter::supportedImageFormats().at(i));
      QString strLower = str.toLower();
      
      const QString f = (str
                         + " Image File (*."
                         + strLower
                         + ")");
      fileFiltersOut << f;
      fileExtensionsOut << strLower;
   }
}

/**
 * file filter containing all image save formats.
 */
QString 
FileFilters::getImageSaveFileFilter()
{
   QStringList filterList;
   
   for (int i = 0; i < QImageWriter::supportedImageFormats().count(); i++) {
      QString str = QString(QImageWriter::supportedImageFormats().at(i));
      QString strLower = str.toLower();
      filterList << ("*." + strLower);
   }
   
   const QString filters = ("Image File ("
                            + filterList.join(" ")
                            + ")");
   return filters;
}

/**
 * get all file filters.
 */
void 
FileFilters::getAllFileFilters(QStringList& allFileFiltersOut)
{
   allFileFiltersOut.clear();
   
   allFileFiltersOut.append(getAreaColorFileFilter());
   allFileFiltersOut.append(getArealEstimationFileFilter());
   allFileFiltersOut.append(getBorderRawFileFilter());
   allFileFiltersOut.append(getBorderFiducialFileFilter());
   allFileFiltersOut.append(getBorderInflatedFileFilter());
   allFileFiltersOut.append(getBorderVeryInflatedFileFilter());
   allFileFiltersOut.append(getBorderSphericalFileFilter());
   allFileFiltersOut.append(getBorderEllipsoidalFileFilter());
   allFileFiltersOut.append(getBorderCompressedFileFilter());
   allFileFiltersOut.append(getBorderFlatFileFilter());
   allFileFiltersOut.append(getBorderFlatLobarFileFilter());
   allFileFiltersOut.append(getBorderHullFileFilter());
   allFileFiltersOut.append(getBorderUnknownFileFilter());
   allFileFiltersOut.append(getBorderGenericFileFilter());
   allFileFiltersOut.append(getBorderVolumeFileFilter());
   allFileFiltersOut.append(getBorderColorFileFilter());
   allFileFiltersOut.append(getBorderProjectionFileFilter());
   allFileFiltersOut.append(getCellFileFilter());
   allFileFiltersOut.append(getCellColorFileFilter());
   allFileFiltersOut.append(getCellProjectionFileFilter());
   allFileFiltersOut.append(getCellVolumeFileFilter());
   allFileFiltersOut.append(getCocomacFileFilter());
   allFileFiltersOut.append(getContourFileFilter());
   allFileFiltersOut.append(getContourCellFileFilter());
   allFileFiltersOut.append(getContourCellColorFileFilter());
   allFileFiltersOut.append(getCoordinateRawFileFilter());
   allFileFiltersOut.append(getCoordinateFiducialFileFilter());
   allFileFiltersOut.append(getCoordinateInflatedFileFilter());
   allFileFiltersOut.append(getCoordinateVeryInflatedFileFilter());
   allFileFiltersOut.append(getCoordinateSphericalFileFilter());
   allFileFiltersOut.append(getCoordinateEllipsoidalFileFilter());
   allFileFiltersOut.append(getCoordinateCompressedFileFilter());
   allFileFiltersOut.append(getCoordinateFlatFileFilter());
   allFileFiltersOut.append(getCoordinateFlatLobarFileFilter());
   allFileFiltersOut.append(getCoordinateHullFileFilter());
   allFileFiltersOut.append(getCoordinateUnknownFileFilter());
   allFileFiltersOut.append(getCoordinateGenericFileFilter());
   allFileFiltersOut.append(getCutsFileFilter());
   allFileFiltersOut.append(getDeformationFieldFileFilter());
   allFileFiltersOut.append(getDeformationMapFileFilter());
   allFileFiltersOut.append(getFociFileFilter());
   allFileFiltersOut.append(getFociColorFileFilter());
   allFileFiltersOut.append(getFociProjectionFileFilter());
   allFileFiltersOut.append(getGeodesicDistanceFileFilter());
   allFileFiltersOut.append(getImageSaveFileFilter());
   allFileFiltersOut.append(getLatitudeLongitudeFileFilter());
   allFileFiltersOut.append(getMetricFileFilter());
   allFileFiltersOut.append(getMetricShapeFileFilter());
   allFileFiltersOut.append(getPaintFileFilter());
   allFileFiltersOut.append(getPaletteFileFilter());
   allFileFiltersOut.append(getParamsFileFilter());
   allFileFiltersOut.append(getProbAtlasFileFilter());
   allFileFiltersOut.append(getRgbPaintFileFilter());
   allFileFiltersOut.append(getRegionOfInterestFileFilter());
   allFileFiltersOut.append(getSceneFileFilter());
   allFileFiltersOut.append(getSectionFileFilter());
   allFileFiltersOut.append(getSurfaceShapeFileFilter());
   allFileFiltersOut.append(getTopographyFileFilter());
   allFileFiltersOut.append(getTopologyClosedFileFilter());
   allFileFiltersOut.append(getTopologyOpenFileFilter());
   allFileFiltersOut.append(getTopologyCutFileFilter());
   allFileFiltersOut.append(getTopologyCutLobarFileFilter());
   allFileFiltersOut.append(getTopologyUnknownFileFilter());
   allFileFiltersOut.append(getTopologyGenericFileFilter());
   allFileFiltersOut.append(getTransformationMatrixFileFilter());
   allFileFiltersOut.append(getTransformationDataFileFilter());
   allFileFiltersOut.append(getTrajectoryFileFilter());
   allFileFiltersOut.append(getVolumeAnatomyFileFilter());
   allFileFiltersOut.append(getVolumeFunctionalFileFilter());
   allFileFiltersOut.append(getVolumePaintFileFilter());
   allFileFiltersOut.append(getVolumeProbAtlasFileFilter());
   allFileFiltersOut.append(getVolumeRgbFileFilter());
   allFileFiltersOut.append(getVolumeSegmentationFileFilter());
   allFileFiltersOut.append(getVolumeVectorFileFilter());
   allFileFiltersOut.append(getVolumeGenericFileFilter());
   allFileFiltersOut.append(getVolumeFileAfniFilter());
   allFileFiltersOut.append(getVolumeMultiFileAfniFilter());
   allFileFiltersOut.append(getVolumeFileNiftiReadFilter());
   allFileFiltersOut.append(getVolumeFileNiftiWriteFilter());
   allFileFiltersOut.append(getVolumeMultiFileNiftiFilter());
   allFileFiltersOut.append(getVolumeFileSpmMedxFilter());
   allFileFiltersOut.append(getVolumeFileWuNilFilter());
   allFileFiltersOut.append(getVtkModelFileFilter());
   allFileFiltersOut.append(getVocabularyFileFilter());
   allFileFiltersOut.append(getWustlRegionFileFilter());
   allFileFiltersOut.append(getStudyCollectionFileFilter());
   allFileFiltersOut.append(getStudyMetaDataFileFilter());
   allFileFiltersOut.append(getSpecFileFilter());
   allFileFiltersOut.append(getMdPlotFileFilter());
   allFileFiltersOut.append(getNeurolucidaFileFilter());
   allFileFiltersOut.append(getBrainVoyagerFileFilter());
   allFileFiltersOut.append(getVtkSurfaceFileFilter());
   allFileFiltersOut.append(getVtkXmlSurfaceFileFilter());
   allFileFiltersOut.append(getByuSurfaceFileFilter());
   allFileFiltersOut.append(getStlSurfaceFileFilter());
   allFileFiltersOut.append(getStudio3DSurfaceFileFilter());
   allFileFiltersOut.append(getWavefrontObjFileFilter());
   allFileFiltersOut.append(getVrmlSurfaceFileFilter());
   allFileFiltersOut.append(getSumaRgbFileFilter());
   allFileFiltersOut.append(getTextFileFilter());
   allFileFiltersOut.append(getCommaSeparatedValuesFileFilter());
   allFileFiltersOut.append(getOpenInventorSurfaceFileFilter());
   allFileFiltersOut.append(getFreeSurferAsciiSurfaceFileFilter());
   allFileFiltersOut.append(getFreeSurferBinarySurfaceFileFilter());
   allFileFiltersOut.append(getFreeSurferAsciiCurvatureFileFilter());
   allFileFiltersOut.append(getFreeSurferBinaryCurvatureFileFilter());
   allFileFiltersOut.append(getFreeSurferAsciiFunctionalFileFilter());
   allFileFiltersOut.append(getFreeSurferBinaryFunctionalFileFilter());
   allFileFiltersOut.append(getFreeSurferAsciiLabelFileFilter());
   allFileFiltersOut.append(getAnalyzeVolumeFileFilter());
   allFileFiltersOut.append(getMincVolumeFileFilter());
   allFileFiltersOut.append(getMniObjSurfaceFileFilter());
   allFileFiltersOut.append(getVtkVolumeFileFilter());
   allFileFiltersOut.append(getRawVolumeFileFilter());
   allFileFiltersOut.append(getGiftiCoordinateFileFilter());
   allFileFiltersOut.append(getGiftiTopologyFileFilter());
   allFileFiltersOut.append(getGiftiSurfaceFileFilter());
   allFileFiltersOut.append(getGiftiFunctionalFileFilter());
   allFileFiltersOut.append(getGiftiShapeFileFilter());
   allFileFiltersOut.append(getGiftiLabelFileFilter());
   allFileFiltersOut.append(getGiftiVectorFileFilter());
   allFileFiltersOut.append(getSureFitVectorFileFilter());
   allFileFiltersOut.append(getPDFFileFilter());
   allFileFiltersOut.append(getHTMLFileFilter());
   allFileFiltersOut.append(getZipFileFilter());
   allFileFiltersOut.append(getPythonFileFilter());
   allFileFiltersOut.append(getPerlFileFilter());
   allFileFiltersOut.append(getBourneShellFileFilter());
   allFileFiltersOut.append(getCShellFileFilter());
   allFileFiltersOut.append(getWindowsBATFileFilter());
   allFileFiltersOut.append(getCaretScriptFileFilter());
   
   std::sort(allFileFiltersOut.begin(), allFileFiltersOut.end());
   allFileFiltersOut.insert(allFileFiltersOut.begin(), getAnyFileFilter());
}

/**
 * get all file Caret filters.
 */
void 
FileFilters::getAllCaretFileFilters(QStringList& allFileFiltersOut)
{
   allFileFiltersOut.clear();
   
   allFileFiltersOut.append(getAreaColorFileFilter());
   allFileFiltersOut.append(getArealEstimationFileFilter());
   allFileFiltersOut.append(getBorderGenericFileFilter());
   allFileFiltersOut.append(getBorderColorFileFilter());
   allFileFiltersOut.append(getBorderProjectionFileFilter());
   allFileFiltersOut.append(getCellFileFilter());
   allFileFiltersOut.append(getCellColorFileFilter());
   allFileFiltersOut.append(getCellProjectionFileFilter());
   allFileFiltersOut.append(getCocomacFileFilter());
   allFileFiltersOut.append(getContourFileFilter());
   allFileFiltersOut.append(getContourCellFileFilter());
   allFileFiltersOut.append(getContourCellColorFileFilter());
   allFileFiltersOut.append(getCoordinateGenericFileFilter());
   allFileFiltersOut.append(getCutsFileFilter());
   allFileFiltersOut.append(getDeformationFieldFileFilter());
   allFileFiltersOut.append(getDeformationMapFileFilter());
   allFileFiltersOut.append(getFociFileFilter());
   allFileFiltersOut.append(getFociColorFileFilter());
   allFileFiltersOut.append(getFociProjectionFileFilter());
   allFileFiltersOut.append(getGeodesicDistanceFileFilter());
   allFileFiltersOut.append(getLatitudeLongitudeFileFilter());
   allFileFiltersOut.append(getMetricFileFilter());
   allFileFiltersOut.append(getPaintFileFilter());
   allFileFiltersOut.append(getPaletteFileFilter());
   allFileFiltersOut.append(getParamsFileFilter());
   allFileFiltersOut.append(getProbAtlasFileFilter());
   allFileFiltersOut.append(getRgbPaintFileFilter());
   allFileFiltersOut.append(getRegionOfInterestFileFilter());
   allFileFiltersOut.append(getSceneFileFilter());
   allFileFiltersOut.append(getSectionFileFilter());
   allFileFiltersOut.append(getSpecFileFilter());
   allFileFiltersOut.append(getStudyMetaDataFileFilter());
   allFileFiltersOut.append(getSurfaceShapeFileFilter());
   allFileFiltersOut.append(getTopographyFileFilter());
   allFileFiltersOut.append(getTopologyGenericFileFilter());
   allFileFiltersOut.append(getTransformationMatrixFileFilter());
   allFileFiltersOut.append(getVolumeGenericFileFilter());
   allFileFiltersOut.append(getWustlRegionFileFilter());
   allFileFiltersOut.append(getCaretScriptFileFilter());
   
   std::sort(allFileFiltersOut.begin(), allFileFiltersOut.end());
   
}
