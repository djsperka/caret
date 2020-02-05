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


#ifndef __SPEC_FILE_H__
#define __SPEC_FILE_H__

#include "AbstractFile.h"
#include "Category.h"
#include "SceneFile.h"
#include "Species.h"
#include "StereotaxicSpace.h"
#include "Structure.h"

/// SpecFile - This class is used to read, write, and manipulate a specification file.
class SpecFile : public AbstractFile {
   public:
   
      /// True and False for SpecFile.   We would prefer to use "bool" but
      /// std::vector<bool> is treated specially by the compiler to pack the
      /// bools into as little space as possible.  This prevents taking the
      /// address of an element so we have to define our own type.
      /// See page 158 of "The C++ Standard Library" by Josuttis.
      enum SPEC_FILE_BOOL {
         SPEC_FALSE = false,
         SPEC_TRUE = true
      };
      
      /// Sorting methods
      enum SORT {
         SORT_NONE,
         SORT_DATE,
         SORT_NAME
      };
      
      /// brain structure types
      
      /// class for storing info about a file type
      class Entry {
         public:
            // type of file
            enum FILE_TYPE {
               FILE_TYPE_SURFACE,
               FILE_TYPE_VOLUME,
               FILE_TYPE_OTHER
            };
            
            /// the files in the entry
            class Files {
               public:
                  // constructor
                  Files(const QString& filenameIn,
                           const QString& dataFileNameIn,
                           const Structure& structureIn,
                           const SPEC_FILE_BOOL selectionStatus = SPEC_TRUE);
                           
                  // destructor
                  ~Files();
                  
                  // comparison operator
                  bool operator<(const Files& e) const;
            
                  // sort the files
                  static void setSortMethod(const SpecFile::SORT sortMethodIn);
            
                  /// name of file
                  QString filename;
                  
                  /// name of data file (used by volumes with separate header and data files)
                  QString dataFileName;
                  
                  /// file is selected
                  SPEC_FILE_BOOL selected;
                  
                  /// structure with which file is associated
                  Structure structure;
             
                  /// how to sort
                  static SpecFile::SORT sortMethod;
            };
            

            // constructor
            Entry();
                  
            // destructor
            ~Entry();
            
            // comparison operator
            bool operator<(const Entry& e) const
               { return descriptiveName < e.descriptiveName; }

            // initialize
            void initialize(const QString& descriptiveNameIn,
                            const QString& specFileTagIn,
                            const FILE_TYPE fileTypeIn,
                            const bool specFilePatternMatchFlagIn = false);
                  
            // add if tag matches (returns true if tag matched)
            bool addFile(const QString& tagReadIn,
                         const QString& filenameIn,
                         const QString& dataFileNameIn,
                         const Structure& structureIn);
            
            // clear selection status for a file
            void clearSelectionStatus(const QString& filenameIn);
            
            // deselect files from other spec .
            void deselectFilesOtherSpec(const SpecFile& otherSpecFile);

            // add path to files
            void prependPath(const QString& path,
                             const bool ignoreFilesWithAbsPaths);
                             
            // get all files (including data files) to input vector
            void getAllFiles(std::vector<QString>& allFiles);
            
            // get all files (excluding data files) to input vector
            void getAllFilesNoDataFile(std::vector<QString>& allFiles);

            // clear all members except type and tag
            void clear(const bool removeFilesFromDiskToo);
            
            /// get number or files
            int getNumberOfFiles() const { return files.size(); }
            
            // get number of files selected
            int getNumberOfFilesSelected() const;
            
            /// get name of file
            QString getFileName(const int indx) const { return files[indx].filename; }
            
            /// set name of file
            void setFileName(const int indx, const QString& name) {
               files[indx].filename = name;
            }

            /// get name of data file (used by volumes with separate header and data files)
            QString getDataFileName(const int indx) const { return files[indx].dataFileName; }
            
            /// get file is selected
            SPEC_FILE_BOOL getSelected(const int indx) const { return files[indx].selected; }
            
            /// get structure with which file is associated
            Structure getStructure(const int indx) { return files[indx].structure; }
            
            /// get structure with which file is associated
            const Structure getStructure(const int indx) const { return files[indx].structure; }
            
            /// set the structure
            void setStructure(const int indx, const Structure& s) {
               files[indx].structure = s;
            }

            // Returns true if all of myFiles are in otherFiles.  
            bool isSubset(const SpecFile& otherSpecFile,
                          QString& errorMessage) const;
                          
            // set selection based upon file name (returns true if selected)
            bool setSelected(const QString& name,
                             const bool addIfNotFound,
                             const Structure& structureIn);
                             
            // make sure all files exist and are readable
            void validate(QString& errorMessage) const;
            
            // clean up this entry (remove entries for files that do not exist)
            bool cleanup();
            
            // set all file selections
            void setAllSelections(const SPEC_FILE_BOOL selStatus);
            
            // remove paths from all files
            void removePaths();
            
            // save scene
            void saveScene(SceneFile::SceneClass& sc,
                           const bool selectedFilesOnlyFlag);
                           
            // sort the files
            void sort(const SORT sortMethod);
            
            // write the files
            void writeFiles(QTextStream& stream,
                            QDomDocument& xmlDoc,
                            QDomElement& rootElement,
                            const FILE_FORMAT fileFormat,
                            const int fileVersionNumber,
                            const bool writeOnlySelectedFiles) throw (FileException);
            
            /// get the descriptive name
            QString getDescriptiveName() const { return descriptiveName; }
            
            /// get the spec file tag
            QString getSpecFileTag() const { return specFileTag; }
            
            /// descriptive name
            QString descriptiveName;
            
            /// type of file
            FILE_TYPE fileType;
            
            /// spec file tag
            QString specFileTag;
            
            /// the files
            std::vector<Files> files;
            
            /// pattern match (this tag is subset of that found in spec file)
            bool specFilePatternMatchFlag;
      };
      
      // Constructor
      SpecFile();

      // Copy constructor
      SpecFile(const SpecFile& sf);
      
      // Destructor
      ~SpecFile();
      
      // assignment operator
      SpecFile& operator=(const SpecFile& sf);
      
      // append a spec file to this one (files only)
      void append(const SpecFile& sf);
            
      // Clear the spec file
      void clear();
      
      /// get the version of the spec file
      int getFileVersion() const { return fileVersion; }
      
      // clean the spec file (remove entries for files that do not exist)
      bool cleanSpecFile();
      
      // remove files from the spec file and possibly the files themselves on the disk
      void clearFiles(const bool clearVolumeFiles, 
                      const bool clearSurfaceFiles, 
                      const bool clearOtherFiles,
                      const bool removeFilesFromDiskToo);
      
      /// returns true if the file is isEmpty (contains no data)
      bool empty() const { return getFileName().isEmpty();  }
      
      /// set sorting performed when a spec file is read
      void setSorting(const SORT sort) { sorting = sort; }
      
      // Get the number of selected files
      int getNumberOfSelectedFiles() const;
      
      // see if only scene files are selected
      bool onlySceneFilesSelected() const;

      // get the number of entries
      int getNumberOfEntries() const;

      // get the entry at the specified index
      Entry* getEntry(int indx);

      // get all entries
      void getAllEntries(std::vector<Entry>& allEntriesOut);
            
      // convert all data files to the specified data type
      void convertAllDataFilesToType(const std::vector<AbstractFile::FILE_FORMAT> ft,
                                     const bool printInfoToStdout);
      
      // get all of the data files in the spec file
      void getAllDataFilesInSpecFile(std::vector<QString>& allFiles,
                                     const bool includeVolumeDataFiles = true) const;
      
      // validate a spec file - all files exist and are readable (true if valid)
      bool validate(QString& errorMessage) const;
      
      // add a file to the spec file basing spec file tag off of file extension
      // returns true if added
      bool addUnknownTypeOfFileToSpecFile(const QString& fileName);
      
      // add a file to the spec file (returns true if file was written)
      bool addToSpecFile(const QString& tag, 
                         const QString& value1,
                         const QString& value2,
                         const bool writeSpecFileIfChanged);
      
      // select or deselect all files
      void setAllFileSelections(const SPEC_FILE_BOOL status); 
      
      // set just fiducial and flat coord/topo selected
      void setDefaultFilesFiducialAndFlat();

      // set the specified files for metric mapping
      // returns an error message (isEmpty if no error)
      QString setFilesForMetricMapping(const QString& topoFileName,
                                           const QString& coordFileName,
                                           const QString& metricFileName,
                                          const Structure structureIn);

      // Set the specified topology and coordinate files as selected.
      void setTopoAndCoordSelected(const QString& topoName,
                                   const QString& coordName,
                                   const Structure structureIn);

      // Set the specified topology and coordinate files as selected.
      void setTopoAndCoordSelected(const QString& topoName,
                                   const std::vector<QString>& coordNames,
                                   const Structure structureIn);

      /// set to write only files that are selected to the spec file
      void setWriteOnlySelectedFiles(const bool selOnly) { writeOnlySelectedFiles = selOnly; }
   
      // get the species
      Species getSpecies() const;
      
      // set the species
      void setSpecies(const Species& speciesIn);
      
      // get the subject
      QString getSubject() const;
      
      // set the subject
      void setSubject(const QString& subjectIn);
      
      // get the space
      StereotaxicSpace getSpace() const;
      
      // set the space
      void setSpace(const StereotaxicSpace& spaceIn);
      
      // get the structure
      Structure getStructure() const;
      
      // set the structure
      void setStructure(const Structure& st);
      
      // get the category
      Category getCategory() const;
      
      // set the category
      void setCategory(const Category& cat);
      
      // set current directory to path of spec file
      void setCurrentDirectoryToSpecFileDirectory();

      // set the files selected for use by deformation
      void setDeformationSelectedFiles(const QString& closedTopoName,
                                       const QString& cutTopoName,
                                       const QString& fiducialCoordName,
                                       const QString& sphericalCoordName,
                                       const QString& flatCoordName,
                                       const QString& borderOrBorderProjName,
                                       const bool addIfNotFound,
                                       const Structure& structureIn);
       
      // set the files selected for multi-resolution morphing
      void setMultiresolutionMorphingFiles(const QString& topoFileName,
                                           const QString& fiducialFileName,
                                           const QString& flatOrSphereFileName,
                                           const Structure& structureIn);
      // Remove paths from all files.
      void removePathsFromAllFiles();
      
      // Prepend paths to a vector of file names 
      void prependPathsToAllFiles(const QString& path,
                                  const bool ignoreFilesWithAbsPaths);


      // sort all of the different files types by name
      void sortAllFilesByName();
      
      // Sort all of the different files types by date
      void sortAllFilesByDate();
      
      // Select files for use by WebCaret.
      void setWebFileSelections();

      // read the spec file tags that are children of the element
      void readTagsFromXML(QDomElement& topElement) throw (FileException);
      
      // write the spec file tags as children of the element
      void writeTagsToXML(QDomDocument& xmlDoc,
                          QDomElement& rootElement) throw (FileException);
         
      // read the spec file from a scene
      void showScene(const SceneFile::Scene& scene, QString& errorMessage);
      
      // write the spec file to a scene
      void saveScene(SceneFile::Scene& scene, const bool selectedFilesOnlyFlag);
      
      // deselect any of "my files" if they are selected in the "other spec"
      void deselectFilesSelectedInOtherSpecFile(SpecFile& otherSpecFile);
      
      // Returns true if "this" spec file is a subset of the "other" spec file.
      // In other words, all of the files listed in this spec file are in 
      // the other spec file.
      bool isSubsetOfOtherSpecFile(const SpecFile& otherSpecFile, QString& errorMessage) const;

      // clear selection status for file name/selection pair
      void clearSelectionStatus(const std::vector<QString>& specNames,
                                       std::vector<SpecFile::SPEC_FILE_BOOL>& selectionStatus,
                                       const QString fileName);
                                       
      /// write the file's memory in caret6 format to the specified name
      virtual QString writeFileInCaret6Format(const QString& filenameIn, Structure structure,const ColorFile* colorFileIn, const bool useCaret6ExtensionFlag) throw (FileException);

    /// write the file's memory in caret7 format to the specified name
    virtual QString writeFileInCaret7Format(const QString& filenameIn, 
                                            Structure structure,
                                            const ColorFile* colorFileIn, 
                                            const bool useCaret7ExtensionFlag) throw (FileException);
    
      // convert to caret6 spec file tag, returns empty string if tag invalid for Caret6
      static QString convertToCaret6SpecFileTag(const QString& tagCaret5);

    // convert to caret6 spec file tag, returns empty string if tag invalid for Caret7
    static QString convertToCaret7DataFileType(const QString& tagCaret5);
    
      // IMPORTANT if new extensions added, update the method
      //    addUnknownTypeOfFileToSpecFile()
      static QString getTopoFileExtension() { return ".topo"; }
      static QString getCoordinateFileExtension() { return ".coord"; }
      static QString getTransformationMatrixFileExtension() { return ".matrix"; }
      static QString getLatLonFileExtension() { return ".latlon"; }
      static QString getSectionFileExtension() { return ".section"; }
      static QString getPaintFileExtension() { return ".paint"; }
      static QString getRegionOfInterestFileExtension() { return ".roi"; }
      static QString getProbabilisticAtlasFileExtension() { return ".atlas"; }
      static QString getAreaColorFileExtension() { return ".areacolor"; }
      static QString getRgbPaintFileExtension() { return ".RGB_paint"; }
      static QString getBorderFileExtension() { return ".border"; }
      static QString getBorderColorFileExtension() { return ".bordercolor"; }
      static QString getBorderProjectionFileExtension() { return ".borderproj"; }
      static QString getPaletteFileExtension() { return ".palette"; }
      static QString getTopographyFileExtension() { return ".topography"; }
      static QString getCellFileExtension() { return ".cell"; }
      static QString getCellColorFileExtension() { return ".cell_color"; }
      static QString getCellProjectionFileExtension() { return ".cellproj"; }
      static QString getContourFileExtension() { return ".contours"; }
      static QString getContourCellColorFileExtension() { return ".contour_cell_color"; }
      static QString getContourCellFileExtension() { return ".contour_cells"; }
      static QString getMetricFileExtension() { return ".metric"; }
      static QString getSurfaceShapeFileExtension() { return ".surface_shape"; }
      static QString getCocomacConnectivityFileExtension() { return ".cocomac.xml"; }
      static QString getArealEstimationFileExtension() { return ".areal_estimation"; }
      static QString getCutsFileExtension() { return ".cuts"; }
      static QString getFociFileExtension() { return ".foci"; }
      static QString getFociColorFileExtension() { return ".focicolor"; }
      static QString getFociProjectionFileExtension() { return ".fociproj"; }
      static QString getFociSearchFileExtension() { return ".focisearch"; }
      static QString getParamsFileExtension() { return ".params"; }
      static QString getDeformationMapFileExtension() { return ".deform_map"; }
      static QString getDeformationFieldFileExtension() { return ".deform_field"; }
      static QString getVtkModelFileExtension() { return ".vtk"; }
      static QString getGeodesicDistanceFileExtension() { return ".geodesic"; }
      static QString getAtlasSurfaceDirectoryFileExtension() { return ".directory"; }
      static QString getBrainVoyagerFileExtension() { return ".srf"; }
      static QString getAtlasSpaceFileExtension() { return ".atlas_space.xml"; }
      static QString getFreeSurferAsciiCurvatureFileExtension() { return ".curv.asc"; }
      static QString getFreeSurferBinaryCurvatureFileExtension() { return ".curv"; }
      static QString getFreeSurferAsciiFunctionalFileExtension() { return ".w.asc"; }
      static QString getFreeSurferBinaryFunctionalFileExtension() { return ".w"; }
      static QString getFreeSurferLabelFileExtension() { return ".label"; }
      static QString getFreeSurferAsciiSurfaceFileExtension() { return ".asc"; }
      static QString getFreeSurferBinarySurfaceFileExtension() { return ""; } // no extension
      static QString getSumaRgbFileExtension() { return ".col"; }
      static QString getPreferencesFileExtension() { return "caret5_preferences"; }
      static QString getSpecFileExtension() { return ".spec"; }
      static QString getAnalyzeVolumeFileExtension() { return ".hdr"; }
      static QString getAfniVolumeFileExtension() { return ".HEAD"; }
      static QString getWustlVolumeFileExtension() { return ".ifh"; }
      static QString getMincVolumeFileExtension() { return ".mnc"; }
      static QString getNiftiVolumeFileExtension() { return ".nii"; }
      static QString getNiftiGzipVolumeFileExtension() { return ".nii.gz"; }
      static QString getSceneFileExtension() { return ".scene"; }
      static QString getSureFitVectorFileExtension() { return ".vec"; }
      static QString getWustlRegionFileExtension() { return ".wustl_txt"; }
      static QString getLimitsFileExtension() { return ".limits"; }
      static QString getMDPlotFileExtension() { return ".mdo"; }
      static QString getGiftiGenericFileExtension() { return ".gii"; }
      static QString getGiftiCoordinateFileExtension() { return ".coord.gii"; }
      static QString getGiftiFunctionalFileExtension() { return ".func.gii"; }
      static QString getGiftiLabelFileExtension() { return ".label.gii"; }
      static QString getGiftiRgbaFileExtension() { return ".rgba.gii"; }
      static QString getGiftiShapeFileExtension() { return ".shape.gii"; }
      static QString getGiftiSurfaceFileExtension() { return ".surf.gii"; }
      static QString getGiftiTensorFileExtension() { return ".tensor.gii"; }
      static QString getGiftiTimeSeriesFileExtension() { return ".time.gii"; }
      static QString getGiftiTopologyFileExtension() { return ".topo.gii"; }
      static QString getGiftiVectorFileExtension() { return ".vector.gii"; }
      static QString getCommaSeparatedValueFileExtension() { return ".csv"; }
      static QString getVocabularyFileExtension() { return ".vocabulary"; }
      static QString getStudyCollectionFileExtension() { return ".study_collection"; }
      static QString getStudyMetaDataFileExtension() { return ".study"; }
      static QString getXmlFileExtension() { return ".xml"; }
      static QString getTextFileExtension() { return ".txt"; }
      static QString getNeurolucidaFileExtension() { return ".xml"; }
      static QString getCaretScriptFileExtension() { return ".script"; }
      static QString getMniObjeSurfaceFileExtension() { return ".obj"; }
      static QString getZipFileExtension() { return ".zip"; }
      static QString getMultiResMorphFileExtension() { return ".morph"; }
      static QString getTrajectoryFileExtension() { return ".traj"; }

      // IMPORTANT if new extensions added, update the method
      //    addUnknownTypeOfFileToSpecFile()
      
      /// all spec file "Entry" DO NOT CLEAR
      std::vector<Entry*> allEntries;
      
      // volume files have both header and data file
      Entry volumeFunctionalFile;
      
      Entry volumePaintFile;
      
      Entry volumeProbAtlasFile;
      
      Entry volumeRgbFile;
      
      Entry volumeSegmentationFile;
      
      Entry volumeAnatomyFile;
      
      Entry volumeVectorFile;
      
      Entry closedTopoFile;
      Entry openTopoFile;
      Entry cutTopoFile;
      Entry lobarCutTopoFile;
      Entry unknownTopoFile;
      
      Entry rawCoordFile;
      Entry fiducialCoordFile;
      Entry inflatedCoordFile;
      Entry veryInflatedCoordFile;
      Entry sphericalCoordFile;
      Entry ellipsoidCoordFile;
      Entry compressedCoordFile;
      Entry flatCoordFile;
      Entry lobarFlatCoordFile;
      Entry hullCoordFile;
      Entry unknownCoordFile;
      
      Entry rawSurfaceFile;
      Entry fiducialSurfaceFile;
      Entry inflatedSurfaceFile;
      Entry veryInflatedSurfaceFile;
      Entry sphericalSurfaceFile;
      Entry ellipsoidSurfaceFile;
      Entry compressedSurfaceFile;
      Entry flatSurfaceFile;
      Entry lobarFlatSurfaceFile;
      Entry hullSurfaceFile;
      Entry unknownSurfaceFile;
      
      Entry averageFiducialCoordFile;
      
      Entry latLonFile;
      
      Entry sectionFile;
      
      Entry sceneFile;
      
      Entry imageFile;
      
      Entry transformationMatrixFile;
      
      Entry paintFile;
      Entry areaColorFile;
      Entry rgbPaintFile;
      
      Entry vectorFile;

      Entry rawBorderFile;
      Entry fiducialBorderFile;
      Entry inflatedBorderFile;
      Entry veryInflatedBorderFile;
      Entry sphericalBorderFile;
      Entry ellipsoidBorderFile;
      Entry compressedBorderFile;
      Entry flatBorderFile;
      Entry lobarFlatBorderFile;
      Entry hullBorderFile;
      Entry unknownBorderFile;
      Entry volumeBorderFile;
      
      Entry borderColorFile;
      Entry borderProjectionFile;
      Entry momcTemplateFile;
      Entry momcTemplateMatchFile;
      
      Entry paletteFile;
     
      Entry topographyFile;
      
      Entry cellFile;
      Entry cellColorFile;
      Entry cellProjectionFile;
      Entry volumeCellFile;
      
      Entry contourFile;
      
      Entry contourCellFile;
      
      Entry contourCellColorFile;
      
      Entry atlasFile;
      
      Entry metricFile;
      
      Entry surfaceShapeFile;
      
      Entry cocomacConnectivityFile;
      
      Entry arealEstimationFile;
      
      Entry cutsFile;
      
      Entry fociFile;
      Entry fociColorFile;
      Entry fociProjectionFile;
      Entry fociSearchFile;
      
      Entry paramsFile;
      Entry deformationMapFile;
      Entry deformationFieldFile;
      Entry cerebralHullFile;
      
      Entry    vtkModelFile;
      
      Entry    geodesicDistanceFile;
      
      Entry    wustlRegionFile;
      
      Entry    transformationDataFile;
      
      Entry studyMetaDataFile;
      Entry studyCollectionFile;
      
      Entry vocabularyFile;
      
      Entry documentFile;
      
      Entry scriptFile;
      
      Entry trajectoryFile;

      static QString getVolumeFunctionalFileTag()   { return "volume_functional_file"; }
      static QString getVolumePaintFileTag()        { return "volume_paint_file"; }
      static QString getVolumeProbAtlasFileTag() { return "volume_prob_atlas_file"; }
      static QString getVolumeRgbFileTag()     { return "volume_rgb_file"; }
      static QString getVolumeSegmentationFileTag() { return "volume_segmentation_file"; }
      static QString getVolumeAnatomyFileTag()   { return "volume_anatomy_file"; }
      static QString getVolumeVectorFileTag()    { return "volume_vector_file"; }
      
      static QString getClosedTopoFileTag()     { return "CLOSEDtopo_file"; }
      static QString getOpenTopoFileTag()       { return "OPENtopo_file"; }
      static QString getCutTopoFileTag()        { return "CUTtopo_file"; }
      static QString getLobarCutTopoFileTag()   { return "LOBAR_CUTtopo_file"; }
      static QString getUnknownTopoFileMatchTag() { return "topo_file"; }
      
      static QString getTopoFlagTag()           { return "topo_flag"; }
      
      static QString getRawCoordFileTag()        { return "RAWcoord_file"; }
      static QString getFiducialCoordFileTag()   { return "FIDUCIALcoord_file"; }
      static QString getInflatedCoordFileTag()   { return "INFLATEDcoord_file"; }
      static QString getVeryInflatedCoordFileTag()   { return "VERY_INFLATEDcoord_file"; }
      static QString getSphericalCoordFileTag()  { return "SPHERICALcoord_file"; }
      static QString getEllipsoidCoordFileTag()  { return "ELLIPSOIDcoord_file"; }
      static QString getCompressedCoordFileTag() { return "COMPRESSED_MEDIAL_WALLcoord_file"; }
      static QString getFlatCoordFileTag()       { return "FLATcoord_file"; }
      static QString getLobarFlatCoordFileTag()  { return "LOBAR_FLATcoord_file"; }
      static QString getHullCoordFileTag()       { return "HULLcoord_file"; }
      static QString getUnknownCoordFileMatchTag() { return "coord_file"; }
      static QString getAverageFiducialCoordFileTag() { return "AVERAGE_FIDUCIALcoord_file"; }
      
      static QString getRawSurfaceFileTag()        { return "RAWsurface_file"; }
      static QString getFiducialSurfaceFileTag()   { return "FIDUCIALsurface_file"; }
      static QString getInflatedSurfaceFileTag()   { return "INFLATEDsurface_file"; }
      static QString getVeryInflatedSurfaceFileTag()   { return "VERY_INFLATEDsurface_file"; }
      static QString getSphericalSurfaceFileTag()  { return "SPHERICALsurface_file"; }
      static QString getEllipsoidSurfaceFileTag()  { return "ELLIPSOIDsurface_file"; }
      static QString getCompressedSurfaceFileTag() { return "COMPRESSED_MEDIAL_WALLsurface_file"; }
      static QString getFlatSurfaceFileTag()       { return "FLATsurface_file"; }
      static QString getLobarFlatSurfaceFileTag()  { return "LOBAR_FLATsurface_file"; }
      static QString getHullSurfaceFileTag()       { return "HULLsurface_file"; }
      static QString getUnknownSurfaceFileMatchTag() { return "surface_file"; }
      
      static QString getLatLonFileTag() { return "lat_lon_file"; }
      static QString getSectionFileTag() { return "section_file"; }
      static QString getTransformationMatrixFileTag() { return "transformation_matrix_file"; }
      static QString getTransformationDataFileTag() { return "transformation_data_file"; }
      
      static QString getPaintFileTag() { return "paint_file"; }
      static QString getAreaColorFileTag() { return "area_color_file"; }
      static QString getRgbPaintFileTag() { return "RGBpaint_file"; }
      static QString getVectorFileTag() { return "vector_file"; }
      
      static QString getRawBorderFileTag()        { return "RAWborder_file"; }
      static QString getFiducialBorderFileTag()   { return "FIDUCIALborder_file"; }
      static QString getInflatedBorderFileTag()   { return "INFLATEDborder_file"; }
      static QString getVeryInflatedBorderFileTag()   { return "VERY_INFLATEDborder_file"; }
      static QString getSphericalBorderFileTag()  { return "SPHERICALborder_file"; }
      static QString getEllipsoidBorderFileTag()  { return "ELLIPSOIDborder_file"; }
      static QString getCompressedBorderFileTag() { return "COMPRESSED_MEDIAL_WALLborder_file"; }
      static QString getFlatBorderFileTag()  { return "FLATborder_file"; }
      static QString getLobarFlatBorderFileTag() { return "LOBAR_FLATborder_file"; }
      static QString getHullBorderFileTag() { return "HULLborder_file"; }
      static QString getUnknownBorderFileMatchTag() { return "border_file"; }
      static QString getVolumeBorderFileTag() { return "VOLUMEborder_file"; }
      
      static QString getBorderColorFileTag() { return "border_color_file"; }
      static QString getBorderProjectionFileTag() { return "borderproj_file"; }
      
      static QString getPaletteFileTag() { return "palette_file"; }
      
      static QString getTopographyFileTag() { return "topography_file"; }
      static QString getTrajectoryFileTag() { return "electrode_trajectory_file"; }
      static QString getCellFileTag() { return "cell_file"; }
      static QString getCellColorFileTag() { return "cell_color_file"; }
      static QString getCellProjectionFileTag() { return "cellproj_file"; }
      static QString getVolumeCellFileTag() { return "volume_cell_file"; }
      
      static QString getContourFileTag() { return "contour_file"; }
      static QString getContourCellFileTag() { return "contour_cell_file"; }
      static QString getContourCellColorFileTag() { return "contour_cell_color_file"; }
      
      static QString getFociFilterFlagTag()     { return "foci_filter_flag"; }
      static QString getFociFileTag()           { return "foci_file"; }
      static QString getFociColorFileTag()      { return "foci_color_file"; }
      static QString getFociProjectionFileTag() { return "fociproj_file"; }
      static QString getFociSearchFileTag()     { return "foci_search_file"; }
      
      static QString getAtlasFileTag() { return "atlas_file"; }
      
      static QString getMetricFileTag() { return "metric_file"; }
      static QString getSurfaceShapeFileTag() { return "surface_shape_file"; }
      
      static QString getCocomacConnectivityFileTag() { return "cocomac_connect_file"; }
      
      static QString getArealEstimationFileTag() { return "areal_estimation_file"; }
      
      static QString getCutsFileTag() { return "cuts_file"; }
      
      static QString getParamsFileTag() { return "params_file"; }
      
      static QString getDeformationMapFileTag() { return "deform_map_file"; }
      
      static QString getDeformationFieldFileTag() { return "deform_field_file"; }
      
      static QString getCerebralHullFileTag() { return "CEREBRAL_HULLvtk_file"; }
      
      static QString getVtkModelFileTag() { return "vtk_model_file"; }
      
      static QString getGeodesicDistanceFileTag() { return "geodesic_distance_file"; }
      
      static QString getImageFileTag() { return "image_file"; }
      static QString getSceneFileTag() { return "scene_file"; }
      static QString getScriptFileTag() { return "script_file"; }
      
      static QString getStudyCollectionFileTag() { return "study_collection_file"; }
      static QString getStudyMetaDataFileTag() { return "study_metadata_file"; }
      
      static QString getVocabularyFileTag() { return "vocabulary_file"; }
      
      static QString getWustlRegionFileTag() { return "wustl_region_file"; }
      
      static QString getDocumentFileTag() { return "document_file"; }
      static QString getFlatBorderFileTagName() { return "FLAT"; }
      static QString getSphericalBorderFileTagName() { return "SPHERICAL"; }

   protected:
      // copy helper used by copy constructor and operator=
      void copyHelperSpecFile(const SpecFile& sf);
      
      // Read the spec file data
      void readFileData(QFile& file, QTextStream& stream, QDataStream& binStream,
                                  QDomElement& /* rootElement */) throw (FileException);
      
      // Write the spec file data
      void writeFileData(QTextStream& stream, QDataStream& binStream,
                                 QDomDocument& /* xmlDoc */,
                                  QDomElement& /* rootElement */) throw (FileException);
      
      // process a tag/value pair while reading spec file
      bool processTag(const std::vector<QString>& tokens);
      
      /// version of the spec file
      int fileVersion;
      
      /// sorting performed when a spec file is read
      SORT sorting;
      
      // Write a set of files associated with a tag
      void writeFiles(QTextStream& stream, 
                      QDomDocument& xmlDoc,
                      QDomElement& rootElement,
                      const QString& tagName, 
                      const std::vector<QString>& names,
                      const std::vector<SPEC_FILE_BOOL>& selected) const;
      
      // Write a set of files (two names) associated with a tag
      void writeFiles(QTextStream& stream, 
                      QDomDocument& xmlDoc,
                      QDomElement& rootElement,
                      const QString& tagName, 
                      const std::vector<QString>& names,
                      const std::vector<QString>& names2,
                      const std::vector<SPEC_FILE_BOOL>& selected) const;
      
      // Write a set of tags with files
      void writeFiles(QTextStream& stream, 
                      QDomDocument& xmlDoc,
                      QDomElement& rootElement,
                      const std::vector<QString>& tagName, 
                      const std::vector<QString>& names,
                      const std::vector<SPEC_FILE_BOOL>& selected) const;
      
      // Write a file associated with a tag
      void writeFiles(QTextStream& stream, 
                      QDomDocument& xmlDoc,
                      QDomElement& rootElement,
                      const QString& tagName, 
                      const QString& name,
                      const SPEC_FILE_BOOL selected) const;
      
      // update the allEntries vector
      void updateAllEntries();
      
      static QString getXmlFileTagName1() { return "file1"; }
         
      static QString getXmlFileTagName2() { return "file2"; }
       
      static QString getXmlFileTagStructure() { return "structure"; }

      /// only write selected files flag
      bool writeOnlySelectedFiles;
      
      std::vector<QString> otherTags;
      std::vector<QString> otherTagsValues;

      // NOTE: BE SURE TO UPDATE copyHelperSpecFile() if new *******************
      // variables are added !!!!***********************************************
};  // class

#endif // __SPEC_FILE_H__

#ifdef __SPEC_FILE_DEFINE__
   SpecFile::SORT SpecFile::Entry::Files::sortMethod; //  = SpecFile::SORT_DATE;
#endif // __SPEC_FILE_DEFINE__
