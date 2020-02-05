
#ifndef __GUI_FILES_MODIFIED_H__
#define __GUI_FILES_MODIFIED_H__

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
 *  ualong with CARET; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*LICENSE_END*/

/// This class is used to assist with updating the GUI when data files are modified.
class GuiFilesModified {
   public:
      /// Constructor
      GuiFilesModified();

      /// Destructor
      ~GuiFilesModified();
      
      /// get all files were modified
      bool getAllFilesModified() const;
      
      /// set the status for all files
      void setStatusForAll(const bool status);
      
      /// set area color has been modified
      
      /// set  has been modified
      void setAreaColorModified() { areaColor = true; }
      
      /// set areal estimation has been modified
      void setArealEstimationModified() { arealEstimation = true; }
      
      /// set border has been modified
      void setBorderModified() { border = true; }
      
      /// set border color has been modified
      void setBorderColorModified() { borderColor = true; }
      
      /// set cell has been modified
      void setCellModified() { cell = true; }
      
      /// set cell color has been modified
      void setCellColorModified() { cellColor = true; }
      
      /// set cell projection has been modified
      void setCellProjectionModified() { cellProjection = true; }
      
      /// set cocomac has been modified
      void setCocomacModified() { cocomac = true; }
      
      /// get contour has been modified
      bool getContourModified() const { return contour; }
      
      /// set contour has been modified
      void setContourModified() { contour = true; }
      
      /// set contour cell has been modified
      void setContourCellModified() { contourCell = true; }
      
      /// set contour cell color has been modified
      void setContourCellColorModified() { contourCellColor = true; }
      
      /// get the coordinate has been modified
      bool getCoordinateModified() const { return coordinate; }
      
      /// set coordinate has been modified
      void setCoordinateModified() { coordinate = true; }
      
      /// set cut has been modified
      void setCutModified() { cut = true; }
      
      /// set deformation map has been modified
      void setDeformationMapModified() { deformationMap = true; }
      
      /// set deformation field has been modified
      void setDeformationFieldModified() { deformationField = true; }
      
      /// set foci has been modified
      void setFociModified() { foci = true; }
      
      /// set foci color has been modified
      void setFociColorModified() { fociColor = true; }
      
      /// set foci projection has been modified
      void setFociProjectionModified() { fociProjection = true; }
      
      /// set foci search has been modified
      void setFociSearchModified() { fociSearch = true; }
      
      /// set geodesic has been modified
      void setGeodesicModified() { geodesic = true; }
      
      /// set images has been modified
      void setImagesModified() { images = true; }
      
      /// get images has been modified
      bool getImagesModified() const { return images; }
      
      /// set lat lon has been modified
      void setLatLonModified() { latLon = true; }
      
      /// set metric has been modified
      void setMetricModified() { metric = true; }
      
      /// set paint has been modified
      void setPaintModified() { paint = true; }
      
      /// set palette has been modified
      void setPaletteModified() { palette = true; }
      
      /// set parameters has been modified
      void setParameterModified() { parameter = true; }
      
      /// set probabilisitic atlas has been modified
      void setProbabilisticAtlasModified() { probabilisticAtlas = true; }
      
      /// set rgb paint has been modified
      void setRgbPaintModified() { rgbPaint = true; }
      
      /// set scene has been modified
      void setSceneModified() { scene = true; }
      
      /// set section has been modified
      void setSectionModified() { section = true; }
      
      /// set study collection has been modified
      void setStudyCollectionModified() { studyCollection = true; }
      
      /// set study metadata has been modified
      void setStudyMetaDataModified() { studyMetaData = true; }
      
      /// set surface shape has been modified
      void setSurfaceShapeModified() { surfaceShape = true; }
      
      /// set vector has been modified
      void setVectorModified() { vector = true; }
      
      /// set topography has been modified
      void setTopographyModified() { topography = true; }
      
      /// get topology has been modified
      bool getTopologyModified() const { return topology; }
      
      /// set topology has been modified
      void setTopologyModified() { topology = true; }
      
      /// set transformation matrix has been modified
      void setTransformationMatrixModified() { transformationMatrix = true; }
      
      /// set transformation data file has been modified
      void setTransformationDataModified() { transformationData = true; }
      
      /// get transformation data file has been modified
      bool getTransformationDataModified() const { return transformationData; }
      
      /// get volume has been modified
      bool getVolumeModified() const { return volume; }
      
      /// set volume has been modified
      void setVolumeModified() { volume = true; }
      
      /// get vtk model has been modified
      bool getVtkModelModified() const { return vtkModel; }
      
      /// set vtk model has been modified
      void setVtkModelModified() { vtkModel = true; }
      
      /// set vocabulary modified
      void setVocabularyModified() { vocabulary = true; }
      
      /// get vocabulary modfiied
      bool getVocabularyModified() { return vocabulary; }
      
      /// set wustl region has been modified
      void setWustlRegionModified() { wustlRegion = true; }

      /// set trajectory data modified
      void setTrajectoryModified() { trajectory = true; }

      /// set inhibit coordinate file surface default scaling
      void setInhibitSurfaceDefaultScaling() { inhibitDefaultSurfaceScaling = true; }
      
   protected:
      bool areaColor;
      bool arealEstimation;
      bool border;
      bool borderColor;
      bool cell;
      bool cellColor;
      bool cellProjection;
      bool cocomac;
      bool contour;
      bool contourCell;
      bool contourCellColor;
      bool coordinate;
      bool cut;
      bool deformationField;
      bool deformationMap;
      bool foci;
      bool fociColor;
      bool fociProjection;
      bool fociSearch;
      bool geodesic;
      bool images;
      bool latLon;
      bool metric;
      bool paint;
      bool palette;
      bool parameter;
      bool probabilisticAtlas;
      bool rgbPaint;
      bool scene;
      bool section;
      bool studyCollection;
      bool studyMetaData;
      bool surfaceShape;
      bool vector;
      bool topography;
      bool topology;
      bool transformationMatrix;
      bool transformationData;
      bool vocabulary;
      bool volume;
      bool vtkModel;
      bool wustlRegion;
      bool trajectory;
      
      bool inhibitDefaultSurfaceScaling;
      friend class GuiMainWindow;
};

#endif // __GUI_FILES_MODIFIED_H__      
