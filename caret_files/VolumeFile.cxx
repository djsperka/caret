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
/*****===================================================================*****/
/*****     Sample functions to deal with NIFTI-1 and ANALYZE files       *****/
/*****...................................................................*****/
/*****            This code is released to the public domain.            *****/
/*****...................................................................*****/
/*****  Author: Robert W Cox, SSCC/DIRP/NIMH/NIH/DHHS/USA/EARTH          *****/
/*****  Date:   August 2003                                              *****/
/*****...................................................................*****/
/*****  Neither the National Institutes of Health (NIH), nor any of its  *****/
/*****  employees imply any warranty of usefulness of this software for  *****/
/*****  any purpose, and do not assume any liability for damages,        *****/
/*****  incidental or otherwise, caused by any use of this document.     *****/
/*****===================================================================*****/


#include <QGlobalStatic>  // needed for Q_OS_WIN32
#ifdef Q_OS_WIN32     // required for M_PI in <cmath>
#define _USE_MATH_DEFINES
#define NOMINMAX
#endif
#include <QTime>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <iostream>
#include <memory>
#include <sstream>
#include <stack>

#include <QProcess>

#define __VOLUME_FILE_MAIN_H__
#include "VolumeFile.h"
#undef __VOLUME_FILE_MAIN_H__

#include "BorderFile.h"
#include "ByteSwapping.h"
#include "DebugControl.h"
#include "FileUtilities.h"
#include "MathUtilities.h"
#include "NiftiFileHeader.h"
#include "ParamsFile.h"
#include "SpecFile.h"
#include "StatisticDataGroup.h"
#include "StatisticHistogram.h"
#include "StringUtilities.h"
#include "TopologyFile.h"
#include "TransformationMatrixFile.h"
#include "SystemUtilities.h"
#include "SureFitVectorFile.h"
#include "VolumeITKImage.h"
#include "VolumeModification.h"

#ifdef HAVE_MINC
#include "minc_cpp.h"
#endif

#include "caret_uniformize.h"
#include "mayo_analyze.h"
#include "nifti1.h"
#include "vtkCleanPolyData.h"
#include "vtkFloatArray.h"
#include "vtkImageCast.h"
#include "vtkImageFlip.h"
#include "vtkImageGaussianSmooth.h"
#include "vtkImageResample.h"
#include "vtkImageReslice.h"
#include "vtkImageSeedConnectivity.h"
#include "vtkImageShrink3D.h"
#include "vtkMarchingCubes.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkPolyDataNormals.h"
#include "vtkPolyDataWriter.h"
#include "vtkStructuredPoints.h"
#include "vtkStructuredPointsReader.h"
#include "vtkStructuredPointsWriter.h"
#include "vtkTransform.h"
#include "vtkTriangle.h"
#include "vtkTriangleFilter.h"
#include "vtkUnsignedCharArray.h"
#include "NiftiCaretExtension.h"


/**
 * Constructor.
 */
VolumeFile::VolumeFile()
   : AbstractFile("Volume File", 
                  SpecFile::getNiftiGzipVolumeFileExtension(),
                  false, 
                  FILE_FORMAT_OTHER, 
                  FILE_IO_NONE, 
                  FILE_IO_NONE, 
                  FILE_IO_NONE,
                  FILE_IO_NONE,
                  FILE_IO_NONE,
                  FILE_IO_READ_AND_WRITE)
{
   voxels     = NULL;
   voxelColoring  = NULL;
   voxelToSurfaceDistances = NULL;
   clear();
}

/**
 * Copy Constructor.
 */
VolumeFile::VolumeFile(const VolumeFile& vf)
   : AbstractFile("Volume File", 
                  vf.getDefaultFileNameExtension(),
                  false, FILE_FORMAT_OTHER,
                  FILE_IO_NONE, 
                  FILE_IO_NONE, 
                  FILE_IO_NONE,
                  FILE_IO_READ_AND_WRITE)
{
   voxels     = NULL;
   voxelColoring  = NULL;
   voxelToSurfaceDistances = NULL;
   copyVolumeData(vf);
}

/**
 * Assignment operator.
 */
VolumeFile&
VolumeFile::operator=(const VolumeFile& vf)
{
   if (this != &vf) {
      // do not do this since this volume already has data in it: voxels     = NULL;
      // do not do this since this volume already has data in it: voxelColoring  = NULL;
      // do not do this since this volume already has data in it: voxelToSurfaceDistances = NULL;
      copyVolumeData(vf);
   }
   
   return *this;
}

/**
 * constructor creates volume from a vector file.
 */
VolumeFile::VolumeFile(const SureFitVectorFile& vf)
   : AbstractFile("Volume File", 
                  vf.getDefaultFileNameExtension(),
                  false, 
                  FILE_FORMAT_OTHER, 
                  FILE_IO_NONE, 
                  FILE_IO_NONE, 
                  FILE_IO_NONE,
                  FILE_IO_READ_AND_WRITE)
{
   voxels     = NULL;
   voxelColoring  = NULL;
   voxelToSurfaceDistances = NULL;

   int dim[3];
   vf.getDimensions(dim);
   const ORIENTATION orient[3] = { 
      ORIENTATION_LEFT_TO_RIGHT,
      ORIENTATION_POSTERIOR_TO_ANTERIOR,
      ORIENTATION_INFERIOR_TO_SUPERIOR
   };
   const float org[3] = { 0.0, 0.0, 0.0 };
   const float space[3] = { 1.0, 1.0, 1.0 };
   initialize(VOXEL_DATA_TYPE_VECTOR,
              dim,
              orient,
              org,
              space,
              true,
              true);
              
   for (int i = 0; i < dim[0]; i++) {
      for (int j = 0; j < dim[1]; j++) {
         for (int k = 0; k < dim[2]; k++) {
            float xyz[3];
            vf.getVector(i, j, k, xyz);
            setVoxel(i, j, k, 0, xyz[0]);
            setVoxel(i, j, k, 1, xyz[1]);
            setVoxel(i, j, k, 2, xyz[2]);
            setVoxel(i, j, k, 3, vf.getMagnitude(i, j, k));
         }
      }
   }
}
      

/**
 * copy volume data (used by copy contructor and assignment operator).
 */
void 
VolumeFile::copyVolumeData(const VolumeFile& vf,
                           const bool copyVoxelData)
{
   clear();
   
   //
   // Copy parent's data
   //
   copyHelperAbstractFile(vf);
   
   //
   // Copy member variables
   //
   volumeType = vf.volumeType;
   fileReadType = vf.fileReadType;
   setFileWriteType(vf.fileWriteType);
   dataFileName = "";
   voxelDataType = vf.voxelDataType;
   niftiReadDataOffset = vf.niftiReadDataOffset;
   spmAcPosition[0] = vf.spmAcPosition[0];
   spmAcPosition[1] = vf.spmAcPosition[1];
   spmAcPosition[2] = vf.spmAcPosition[2];
   scaleSlope  = vf.scaleSlope;
   scaleOffset = vf.scaleOffset;
   orientation[0] = vf.orientation[0];
   orientation[1] = vf.orientation[1];
   orientation[2] = vf.orientation[2];
   subVolumeNames = vf.subVolumeNames;
   numberOfSubVolumes = vf.numberOfSubVolumes;
   volumeDataReadMode = vf.volumeDataReadMode;
   afniHeader = vf.afniHeader;
   wunilHeader = vf.wunilHeader;
   regionNames = vf.regionNames;
   dataFileWasZippedFlag = vf.dataFileWasZippedFlag;
   
   //
   // Setup volume data
   //
   int dim[3];
   vf.getDimensions(dim);
   ORIENTATION orient[3];
   vf.getOrientation(orient);
   float org[3];
   vf.getOrigin(org);
   float space[3];
   vf.getSpacing(space);   
   initialize(voxelDataType, dim, orient, org, space, false);
   
   //
   // Copy the voxels
   //
   if (copyVoxelData) {
      const int num = getTotalNumberOfVoxelElements();
      for (int i = 0; i < num; i++) {
         voxels[i] = vf.voxels[i];
      }
   }
   
   minimumVoxelValue = vf.minimumVoxelValue;
   maximumVoxelValue = vf.maximumVoxelValue;
   minMaxVoxelValuesValid = vf.minMaxVoxelValuesValid;
   
   minMaxTwoPercentVoxelValue = vf.minMaxTwoPercentVoxelValue;
   minMaxNinetyEightPercentVoxelValue = vf.minMaxNinetyEightPercentVoxelValue;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = vf.minMaxTwoToNinetyEightPercentVoxelValuesValid;
   
   descriptiveLabel = vf.descriptiveLabel;
   
   niftiIntentCodeAndParamString = vf.niftiIntentCodeAndParamString;
   niftiIntentCode = vf.niftiIntentCode;
   niftiIntentName = vf.niftiIntentName;
   niftiIntentParameter1 = vf.niftiIntentParameter1;
   niftiIntentParameter2 = vf.niftiIntentParameter2;
   niftiIntentParameter3 = vf.niftiIntentParameter3;
   niftiTR = vf.niftiTR;
   niftiSFormTransformationMatrix = vf.niftiSFormTransformationMatrix;
   niftiQFormTransformationMatrix = vf.niftiQFormTransformationMatrix;
   
   regionNameHighlighted = vf.regionNameHighlighted;
   
   //studyMetaDataLinkSet = vf.studyMetaDataLinkSet;

   allocateVoxelColoring();
   
   std::ostringstream defaultName;
   defaultName << "copy_of"
               << vf.getFileName().toAscii().constData();
   filename = defaultName.str().c_str();

   setModified();
}
      
/**
 * set the volume file's type for writing.
 */
void 
VolumeFile::setFileWriteType(const FILE_READ_WRITE_TYPE ft) 
{ 
   fileWriteType = ft; 
   
   switch (fileWriteType) {
      case FILE_READ_WRITE_TYPE_RAW:
         setDefaultFileNameExtension(".vol");
         break;
      case FILE_READ_WRITE_TYPE_AFNI:
         setDefaultFileNameExtension(SpecFile::getAfniVolumeFileExtension());
         break;
      case FILE_READ_WRITE_TYPE_ANALYZE:
         setDefaultFileNameExtension(SpecFile::getAnalyzeVolumeFileExtension());
         break;
      case FILE_READ_WRITE_TYPE_NIFTI:
         setDefaultFileNameExtension(SpecFile::getNiftiVolumeFileExtension());
         break;
      case FILE_READ_WRITE_TYPE_NIFTI_GZIP:
         setDefaultFileNameExtension(SpecFile::getNiftiGzipVolumeFileExtension());
         break;
      case FILE_READ_WRITE_TYPE_WUNIL:
         setDefaultFileNameExtension(SpecFile::getWustlVolumeFileExtension());
         break;
      case FILE_READ_WRITE_TYPE_SPM_OR_MEDX:
         setDefaultFileNameExtension(SpecFile::getAnalyzeVolumeFileExtension());
         break;
      case FILE_READ_WRITE_TYPE_UNKNOWN:
         setDefaultFileNameExtension(".vol");
         break;
   }

}
      
/**
 * get the name of the file.
 */
QString 
VolumeFile::getFileName(const QString& description) const 
{ 
   QString name = AbstractFile::getFileName(description);
   
   if (description.isEmpty() == false) {
      QString ext(".");
      ext.append(FileUtilities::filenameExtension(name));
      
      if (ext == ".vol") {
         switch (fileWriteType) {
            case FILE_READ_WRITE_TYPE_RAW:
               ext = ".vol";
               break;
            case FILE_READ_WRITE_TYPE_AFNI:
               ext = SpecFile::getAfniVolumeFileExtension();
               break;
            case FILE_READ_WRITE_TYPE_ANALYZE:
               ext = SpecFile::getAnalyzeVolumeFileExtension();
               break;
            case FILE_READ_WRITE_TYPE_NIFTI:
               ext = SpecFile::getNiftiVolumeFileExtension();
               break;
            case FILE_READ_WRITE_TYPE_NIFTI_GZIP:
               ext = SpecFile::getNiftiGzipVolumeFileExtension();
               break;
            case FILE_READ_WRITE_TYPE_WUNIL:
               ext = SpecFile::getWustlVolumeFileExtension();
               break;
            case FILE_READ_WRITE_TYPE_SPM_OR_MEDX:
               ext = SpecFile::getAnalyzeVolumeFileExtension();
               break;
            case FILE_READ_WRITE_TYPE_UNKNOWN:
               ext = ".vol";
               break;
         }
      }
         
      QString newName;
      
      //
      // Add directory, if needed, to new file name
      //
      QString bn(FileUtilities::dirname(name));
      if ((bn.isEmpty() == false)  &&
          (bn != ".")) {
         newName = bn;
         newName.append("/");
      }

      //
      // Assemble file name
      //
      const QString nameNoExt(FileUtilities::filenameWithoutExtension(name));
      newName.append(nameNoExt);
      if (fileWriteType == FILE_READ_WRITE_TYPE_AFNI) {
         //
         // Afni likes "+orig" in name
         //
         if (nameNoExt.indexOf('+') == -1) {
            newName.append("+orig");
         }
      }
      newName.append(ext);
      
      name = newName;
   }
   
   return name;
}

/**
 * get number of voxel elements (dims * components per voxel).
 */
int 
VolumeFile::getTotalNumberOfVoxelElements() const
{
   const int num = getTotalNumberOfVoxels() 
                 * numberOfComponentsPerVoxel;
   return num;
}      

/**
 * get number of voxel elements (dims only, does not include components per voxel)
 */
int 
VolumeFile::getTotalNumberOfVoxels() const
{
   const int num = dimensions[0] * dimensions[1] * dimensions[2];
   return num;
}      

/**
 * set the data type.
 */
void 
VolumeFile::setVoxelDataType(const VOXEL_DATA_TYPE vdt) 
{
   voxelDataType = vdt; 
   
   switch (voxelDataType) {
      case VOXEL_DATA_TYPE_UNKNOWN:
      case VOXEL_DATA_TYPE_CHAR:
      case VOXEL_DATA_TYPE_CHAR_UNSIGNED:
      case VOXEL_DATA_TYPE_SHORT:
      case VOXEL_DATA_TYPE_SHORT_UNSIGNED:
      case VOXEL_DATA_TYPE_INT:
      case VOXEL_DATA_TYPE_INT_UNSIGNED:
      case VOXEL_DATA_TYPE_LONG:
      case VOXEL_DATA_TYPE_LONG_UNSIGNED:
      case VOXEL_DATA_TYPE_FLOAT:
      case VOXEL_DATA_TYPE_DOUBLE:
         numberOfComponentsPerVoxel = 1;
         break;
      case VOXEL_DATA_TYPE_RGB_VOXEL_INTERLEAVED:
      case VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED:
         numberOfComponentsPerVoxel = 3;
         break;
      case VOXEL_DATA_TYPE_VECTOR:
         numberOfComponentsPerVoxel = 4;
         break;
   }
}

/**
 * get the sub volume names.
 */
void 
VolumeFile::getSubVolumeNames(std::vector<QString>& names) const 
{ 
   names = subVolumeNames;
   if (names.empty()) {
      names.resize(std::max(1, numberOfSubVolumes), FileUtilities::basename(getFileName()));
   }
}

/**
 * Initialize - creates a volume and allocates memory for the specified dimensions.
 */
void
VolumeFile::initialize(const VOXEL_DATA_TYPE vdt, const int dim[3],
                       const ORIENTATION orient[3],
                       const float org[3], const float space[3],
                       const bool doClear,
                       const bool allocateVoxelData)
{
   if (doClear) {
      clear();
   }
   
   if (voxels != NULL) {
      delete[] voxels;
      voxels = NULL;
   }
   
   minimumVoxelValue = 0.0;
   maximumVoxelValue = 0.0;
   minMaxVoxelValuesValid = false;
   
   minMaxTwoPercentVoxelValue = 0.0;
   minMaxNinetyEightPercentVoxelValue = 0.0;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
   
   setVoxelDataType(vdt);
   setDimensions(dim);
   setOrientation(orient);
   setOrigin(org);
   setSpacing(space);
   
   if (allocateVoxelData) {
      const int num = getTotalNumberOfVoxelElements();
      voxels = new float[num];
      for (int i = 0; i < num; i++) {
         voxels[i] = 0.0;
      }
      allocateVoxelColoring();
   }
   
   filename = getFileName("");
/*
   static int nameCounter = 0;
   std::ostringstream defaultName;
   defaultName << "volume_"
               << nameCounter;
   nameCounter++;
   filename = defaultName.str().c_str();
*/   
   setModified();
}

/**
 * Destructor.
 */
VolumeFile::~VolumeFile()
{
   clear();
}

/**
 * get all volume types and names (anatomy, functional, etc).
 */
void 
VolumeFile::getAllVolumeTypesAndNames(std::vector<VOLUME_TYPE>& typesOut,
                                      std::vector<QString>& namesOut,
                                      const bool addUnknown,
                                      const bool addROI)
{
   typesOut.clear();
   namesOut.clear();
   
   typesOut.push_back(VOLUME_TYPE_ANATOMY);
   namesOut.push_back("Anatomy");
   typesOut.push_back(VOLUME_TYPE_FUNCTIONAL);
   namesOut.push_back("Functional");
   typesOut.push_back(VOLUME_TYPE_PAINT);
   namesOut.push_back("Paint");
   typesOut.push_back(VOLUME_TYPE_PROB_ATLAS);
   namesOut.push_back("Probabilistic Atlas");
   typesOut.push_back(VOLUME_TYPE_RGB);
   namesOut.push_back("RGB");
   if (addROI) {
      typesOut.push_back(VOLUME_TYPE_ROI);
      namesOut.push_back("ROI");
   }
   typesOut.push_back(VOLUME_TYPE_SEGMENTATION);
   namesOut.push_back("Segmentation");
   typesOut.push_back(VOLUME_TYPE_VECTOR);
   namesOut.push_back("Vector");
   if (addUnknown) {
      typesOut.push_back(VOLUME_TYPE_UNKNOWN);
      namesOut.push_back("Unknown");
   }
   
   //
   // This switch statement is here so that it will cause a compilation warning
   // if a new volume type is added but not update above and below!!!!
   //
   VOLUME_TYPE vt = VOLUME_TYPE_ANATOMY;
   switch (vt) {
      case VOLUME_TYPE_ANATOMY:
      case VOLUME_TYPE_FUNCTIONAL:
      case VOLUME_TYPE_PAINT:
      case VOLUME_TYPE_PROB_ATLAS:
      case VOLUME_TYPE_RGB:
      case VOLUME_TYPE_ROI:
      case VOLUME_TYPE_SEGMENTATION:
      case VOLUME_TYPE_VECTOR:
      case VOLUME_TYPE_UNKNOWN:
         break;
   }
}
 
/*
 * assemble type name with number of bits for type
 */
static QString
voxelDataTypeHelper(const QString& nameIn, const int numBytes)
{
   QString name(nameIn);
   
   const int numBits = numBytes * 8;
   name += " (";
   name += QString::number(numBits);
   name += " bits)";
   
   return name;
}

/**
 * get all voxel data types and names (float, int, etc).
 */
void 
VolumeFile::getAllVoxelDataTypesAndNames(std::vector<VOXEL_DATA_TYPE>& typesOut,
                                          std::vector<QString>& namesOut,
                                          const bool addUnknown)
{
   typesOut.clear();
   namesOut.clear();
   
   typesOut.push_back(VOXEL_DATA_TYPE_CHAR);
   namesOut.push_back(voxelDataTypeHelper("Char", sizeof(char)));

   typesOut.push_back(VOXEL_DATA_TYPE_CHAR_UNSIGNED);
   namesOut.push_back(voxelDataTypeHelper("Unsigned Char", sizeof(unsigned char)));

   typesOut.push_back(VOXEL_DATA_TYPE_SHORT);
   namesOut.push_back(voxelDataTypeHelper("Short", sizeof(short)));

   typesOut.push_back(VOXEL_DATA_TYPE_SHORT_UNSIGNED);
   namesOut.push_back(voxelDataTypeHelper("Unsigned Short", sizeof(unsigned short)));

   typesOut.push_back(VOXEL_DATA_TYPE_INT);
   namesOut.push_back(voxelDataTypeHelper("int", sizeof(int)));

   typesOut.push_back(VOXEL_DATA_TYPE_INT_UNSIGNED);
   namesOut.push_back(voxelDataTypeHelper("Unsigned Int", sizeof(unsigned int)));

   if (sizeof(long long) == 8) {
      typesOut.push_back(VOXEL_DATA_TYPE_LONG);
      namesOut.push_back(voxelDataTypeHelper("Long", sizeof(long long)));

      typesOut.push_back(VOXEL_DATA_TYPE_LONG_UNSIGNED);
      namesOut.push_back(voxelDataTypeHelper("Unsigned Long", sizeof(unsigned long long)));
   }

   typesOut.push_back(VOXEL_DATA_TYPE_FLOAT);
   namesOut.push_back(voxelDataTypeHelper("Float", sizeof(float)));

   typesOut.push_back(VOXEL_DATA_TYPE_DOUBLE);
   namesOut.push_back(voxelDataTypeHelper("Double", sizeof(double)));

   typesOut.push_back(VOXEL_DATA_TYPE_RGB_VOXEL_INTERLEAVED);
   namesOut.push_back(voxelDataTypeHelper("RGB Voxel Interleaved", sizeof(unsigned char)));

   typesOut.push_back(VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED);
   namesOut.push_back(voxelDataTypeHelper("RGB Slice Interleaved", sizeof(unsigned char)));

   typesOut.push_back(VOXEL_DATA_TYPE_VECTOR);
   namesOut.push_back(voxelDataTypeHelper("Vector", sizeof(float)));

   if (addUnknown) {
      typesOut.push_back(VOXEL_DATA_TYPE_UNKNOWN);
      namesOut.push_back("Unknown");
   }
   
   //
   // This code is just here so that the compiler will complain
   // if a new voxel data type is added
   //
   VOXEL_DATA_TYPE vdt = VOXEL_DATA_TYPE_UNKNOWN;
   switch (vdt) {
      case VOXEL_DATA_TYPE_UNKNOWN:
      case VOXEL_DATA_TYPE_CHAR:
      case VOXEL_DATA_TYPE_CHAR_UNSIGNED:
      case VOXEL_DATA_TYPE_SHORT:
      case VOXEL_DATA_TYPE_SHORT_UNSIGNED:
      case VOXEL_DATA_TYPE_INT:
      case VOXEL_DATA_TYPE_INT_UNSIGNED:
      case VOXEL_DATA_TYPE_LONG:
      case VOXEL_DATA_TYPE_LONG_UNSIGNED:
      case VOXEL_DATA_TYPE_FLOAT:
      case VOXEL_DATA_TYPE_DOUBLE:
      case VOXEL_DATA_TYPE_RGB_VOXEL_INTERLEAVED:
      case VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED:
      case VOXEL_DATA_TYPE_VECTOR:
         break;
   }
}
                                            
/**
 * get the volume's descriptive label.
 */
QString 
VolumeFile::getDescriptiveLabel() const 
{ 
   if (descriptiveLabel.isEmpty()) {
      return FileUtilities::basename(getFileName());
   }
   
   return descriptiveLabel; 
}

/**
 * get the minimum and maximum values for a data type.
 */
void 
VolumeFile::getDataTypeMinMaxValues(const VOXEL_DATA_TYPE vdt,
                                    double& minValueForDataType,
                                    double& maxValueForDataType)
{
   minValueForDataType = 0.0;
   maxValueForDataType = 0.0;
   
   switch (vdt) {
      case VOXEL_DATA_TYPE_UNKNOWN:
         minValueForDataType = 0.0;
         maxValueForDataType = 0.0;
         break;
      case VOXEL_DATA_TYPE_CHAR:
         minValueForDataType = std::numeric_limits<char>::min();
         maxValueForDataType = std::numeric_limits<char>::max();
         break;
      case VOXEL_DATA_TYPE_CHAR_UNSIGNED:
         minValueForDataType = std::numeric_limits<unsigned char>::min();
         maxValueForDataType = std::numeric_limits<unsigned char>::max();
         break;
      case VOXEL_DATA_TYPE_SHORT:
         minValueForDataType = std::numeric_limits<short>::min();
         maxValueForDataType = std::numeric_limits<short>::max();
         break;
      case VOXEL_DATA_TYPE_SHORT_UNSIGNED:
         minValueForDataType = std::numeric_limits<unsigned short>::min();
         maxValueForDataType = std::numeric_limits<unsigned short>::max();
         break;
      case VOXEL_DATA_TYPE_INT:
         minValueForDataType = std::numeric_limits<int>::min();
         maxValueForDataType = std::numeric_limits<int>::max();
         break;
      case VOXEL_DATA_TYPE_INT_UNSIGNED:
         minValueForDataType = std::numeric_limits<unsigned int>::min();
         maxValueForDataType = std::numeric_limits<unsigned int>::max();
         break;
      case VOXEL_DATA_TYPE_LONG:
         minValueForDataType = std::numeric_limits<long long>::min();
         maxValueForDataType = std::numeric_limits<long long>::max();
         break;
      case VOXEL_DATA_TYPE_LONG_UNSIGNED:
         minValueForDataType = std::numeric_limits<unsigned long long>::min();
         maxValueForDataType = std::numeric_limits<unsigned long long>::max();
         break;
      case VOXEL_DATA_TYPE_FLOAT:
         minValueForDataType = -std::numeric_limits<float>::max();
         maxValueForDataType = std::numeric_limits<float>::max();
         break;
      case VOXEL_DATA_TYPE_DOUBLE:
         minValueForDataType = -std::numeric_limits<double>::max();
         maxValueForDataType = std::numeric_limits<double>::max();
         break;
      case VOXEL_DATA_TYPE_RGB_VOXEL_INTERLEAVED:
         minValueForDataType = std::numeric_limits<unsigned char>::min();
         maxValueForDataType = std::numeric_limits<unsigned char>::max();
         break;
      case VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED:
         minValueForDataType = std::numeric_limits<unsigned char>::min();
         maxValueForDataType = std::numeric_limits<unsigned char>::max();
         break;
      case VOXEL_DATA_TYPE_VECTOR:
         minValueForDataType = -std::numeric_limits<float>::max();
         maxValueForDataType = std::numeric_limits<float>::max();
         break;
   }
}                                   

/**
 * rescale voxel values (input min/max mapped to output min/max).
 */
void 
VolumeFile::rescaleVoxelValues(const float inputMinimum,
                               const float inputMaximum,
                               const float outputMinimum,
                               const float outputMaximum)
{
   float inputDiff  = inputMaximum - inputMinimum;
   if (inputDiff == 0.0) {
      inputDiff = 1.0;
   }
   const float outputDiff = outputMaximum - outputMinimum;
   const int num = getTotalNumberOfVoxelElements();
   for (int i = 0; i < num; i++) {
      float value = voxels[i];
      if (value <= inputMinimum) {
         value = outputMinimum;
      }
      else if (value >= inputMaximum) {
         value = outputMaximum;
      }
      else {
         const float normalized = (value - inputMinimum) / inputDiff;
         value = normalized * outputDiff + outputMinimum;
      }
      voxels[i] = value;
   }
   
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}                              
      
/**
 * Scale the voxel values.
 */
void
VolumeFile::scaleVoxelValues(const float scale, const float minimumValueAllowed,
                                const float maximumValueAllowed)
{
   const int num = getTotalNumberOfVoxelElements();
   for (int i = 0; i < num; i++) {
      float value = voxels[i];
      value *= scale;
      value = std::min(value, maximumValueAllowed);
      value = std::max(value, minimumValueAllowed);
      voxels[i] = value;
   }
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 * Get the ranges of the voxel values for a specific component.
 */
void
VolumeFile::getMinMaxVoxelValues(float& minVoxelValue, float& maxVoxelValue) 
{
   if (minMaxVoxelValuesValid == false) {
      const int numVoxelElements = getTotalNumberOfVoxelElements();
      if (numVoxelElements <= 0) {
         minimumVoxelValue = 0.0;
         maximumVoxelValue = 0.0;
      }
      else {
         minimumVoxelValue =  std::numeric_limits<float>::max();
         maximumVoxelValue = -std::numeric_limits<float>::max();
         
         for (int i = 0; i < numVoxelElements; i++) {
            const float value = voxels[i];
            minimumVoxelValue = std::min(minimumVoxelValue, value);
            maximumVoxelValue = std::max(maximumVoxelValue, value);
         }
      }
      minMaxVoxelValuesValid = true;
   }
   minVoxelValue = minimumVoxelValue;
   maxVoxelValue = maximumVoxelValue;
}

/**
 * Get the value of the 2% and 98% voxels.
 */
void
VolumeFile::getTwoToNinetyEightPercentMinMaxVoxelValues(float& minVoxelValue, float& maxVoxelValue) 
{
   if (minMaxTwoToNinetyEightPercentVoxelValuesValid == false) {
      float minVoxel = 0.0;
      float maxVoxel = 255.0;
      std::vector<int> histogram;
      const int histoSize = 256;
      getHistogram(histoSize, histogram, minVoxel, maxVoxel);
      const float range = maxVoxel - minVoxel;

      minMaxTwoPercentVoxelValue = 0.0;
      minMaxNinetyEightPercentVoxelValue = 255.0;
         
      if (range != 0.0) {
         const float percent = 0.02;

         //
         // Skip the first percent of voxels
         //
         int sum = 0;
         const int numVoxels = getTotalNumberOfVoxels();
         int percentage = static_cast<int>(numVoxels * percent);
         for (int i = 0; i < histoSize; i++) {
            sum += histogram[i];
            if (sum >= percentage) {
               minMaxTwoPercentVoxelValue = (static_cast<float>(i) / static_cast<float>(histoSize)) 
                        * range + minVoxel;
               break;
            }
         }
         
         //
         // Skip the first percent of voxels
         //
         sum = 0;
         for (int i = histoSize - 1; i >= 0; i--) {
            sum += histogram[i];
            if (sum >= percentage) {
               minMaxNinetyEightPercentVoxelValue = (static_cast<float>(i) / static_cast<float>(histoSize)) 
                        * range + minVoxel;
               break;
            }
         }
      }
      minMaxTwoToNinetyEightPercentVoxelValuesValid = true;
   }
   minVoxelValue = minMaxTwoPercentVoxelValue;
   maxVoxelValue = minMaxNinetyEightPercentVoxelValue;
}

/**
 * get a histogram of the voxels (assumes one component per voxel).
 * User must delete the returned Histogram.
 */
StatisticHistogram* 
VolumeFile::getHistogram(const int numBuckets,
                         const float excludeLeftPercent,
                         const float excludeRightPercent) const
{
   std::vector<float> values;
   const int numVoxels = getTotalNumberOfVoxels();
   for (int i = 0; i < numVoxels; i++) {
      values.push_back(getVoxelWithFlatIndex(i));
   }
   
   StatisticHistogram* hist = new StatisticHistogram(numBuckets,
                                                     excludeLeftPercent,
                                                     excludeRightPercent);
   StatisticDataGroup sdg(&values, StatisticDataGroup::DATA_STORAGE_MODE_POINT);
   hist->addDataGroup(&sdg);
   try {
      hist->execute();
   }
   catch (StatisticException&) {
   }
   
   return hist;
}

/**
 * get a histogram of the voxels.
 * "minVoxelValue" is the minimum voxel value found
 * "maxVoxelValue" is the maximum voxel value found
 */
void 
VolumeFile::getHistogram(const int numBuckets,
                         std::vector<int>& histogram,
                         float& minVoxelValue,
                         float& maxVoxelValue) 
{
   getMinMaxVoxelValues(minVoxelValue, maxVoxelValue);
   
   const int numVoxels = getTotalNumberOfVoxels();
   const int numComponents = getNumberOfComponentsPerVoxel();
   
   histogram.resize(numBuckets);
   std::fill(histogram.begin(), histogram.end(), 0);
   
   const float range = maxVoxelValue - minVoxelValue;
   if (range == 0.0) {
      return;
   }
   
   const int bucketMax = numBuckets - 1;
   
   for (int i = 0; i < numVoxels; i++) {
      const int indx = i * numComponents;
      float f = ((voxels[indx] - minVoxelValue) / range) 
              * numBuckets;
      int intValue = static_cast<int>(f + 0.5);  // round into an int
      intValue = std::min(intValue, bucketMax);
      intValue = std::max(intValue, 0);
      histogram[intValue]++;
   }
}

/**
 * Allocate voxel coloring.
 */
void
VolumeFile::allocateVoxelColoring()
{
   if (voxelColoring != NULL) {
      delete[] voxelColoring;
      voxelColoring = NULL;
   }
   
   if (voxels != NULL) {
      const int num = getTotalNumberOfVoxels();
      if (num > 0) {
         voxelColoring = new unsigned char[num * 4];
      }
   }
   setVoxelColoringInvalid();
}

/**
 * get the label of an orientation
 */
QString 
VolumeFile::getOrientationLabel(const ORIENTATION orient)
{
   QString s;
   
   switch(orient) {
      case ORIENTATION_UNKNOWN:
         s = "Unknown";
         break;
      case ORIENTATION_RIGHT_TO_LEFT:
         s = "Right to Left";
         break;
      case ORIENTATION_LEFT_TO_RIGHT:
         s = "Left to Right";
         break;
      case ORIENTATION_POSTERIOR_TO_ANTERIOR:
         s = "Posterior to Anterior";
         break;
      case ORIENTATION_ANTERIOR_TO_POSTERIOR:
         s = "Anterior to Posterior";
         break;
      case ORIENTATION_INFERIOR_TO_SUPERIOR:
         s = "Inferior to Superior";
         break;
      case ORIENTATION_SUPERIOR_TO_INFERIOR:
         s = "Superior to Inferior";
         break;
      default:
         s = "Invalid Value";
         break;
   }
   return s;
}
      

/**
 * Clear the file's contents.
 */
void
VolumeFile::clear()
{
   clearAbstractFile();
   
   if (voxels != NULL) {
      delete[] voxels;
      voxels = NULL;
   }
   
   if (voxelColoring != NULL) {
       delete[] voxelColoring;
       voxelColoring = NULL;
   }
   voxelColoringValid = false;
   
   dimensions[0] = 0;
   dimensions[1] = 0;
   dimensions[2] = 0;
   origin[0] = 0.0;
   origin[1] = 0.0;
   origin[2] = 0.0;
   spacing[0] = 0.0;
   spacing[1] = 0.0;
   spacing[2] = 0.0;

   numberOfComponentsPerVoxel = 0;
   
   dataFileWasZippedFlag = false;
   volumeType = VOLUME_TYPE_ANATOMY;  //VOLUME_TYPE_UNKNOWN;
   dataFileName = "";
   fileReadType  = FILE_READ_WRITE_TYPE_NIFTI_GZIP;
   setFileWriteType(FILE_READ_WRITE_TYPE_NIFTI_GZIP);
   numberOfSubVolumes = 0;
   orientation[0]   = ORIENTATION_UNKNOWN;
   orientation[1]   = ORIENTATION_UNKNOWN;
   orientation[2]   = ORIENTATION_UNKNOWN;
   spmAcPosition[0] = 0.0;
   spmAcPosition[1] = 0.0;
   spmAcPosition[2] = 0.0;
   niftiReadDataOffset = 0;
   voxelDataType = VOXEL_DATA_TYPE_UNKNOWN;
   scaleSlope.clear();
   scaleOffset.clear();
   subVolumeNames.clear();
   afniHeader.clear();
   wunilHeader.clear();
   regionNames.clear();
   
   if (voxelToSurfaceDistances != NULL) {
      delete[] voxelToSurfaceDistances;
      voxelToSurfaceDistances = NULL;
   }
   voxelToSurfaceDistancesValid = false;
   
   minimumVoxelValue = 0.0;
   maximumVoxelValue = 0.0;
   minMaxVoxelValuesValid = false;
   
   minMaxTwoPercentVoxelValue = 0.0;
   minMaxNinetyEightPercentVoxelValue = 0.0;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
   
   descriptiveLabel = "";
   niftiIntentCodeAndParamString = "";
   niftiIntentName = "";
   niftiIntentCode = 0;
   niftiIntentParameter1 = 0.0;
   niftiIntentParameter2 = 0.0;
   niftiIntentParameter3 = 0.0;
   niftiTR = 0.0;
   niftiSFormTransformationMatrix.identity();
   niftiQFormTransformationMatrix.identity();
   
   setVoxelDataType(VOXEL_DATA_TYPE_FLOAT);
   
   clearRegionHighlighting();
   
   //studyMetaDataLinkSet.clear();
}

/**
 * get the string describing the type of volume (anatomy, functional, etc).
 */
QString 
VolumeFile::getVolumeTypeDescription() const
{
   QString s;
   switch (volumeType) {
      case VOLUME_TYPE_ANATOMY:
         s = "Anatomy";
         break;
      case VOLUME_TYPE_FUNCTIONAL:
         s = "Functional";
         break;
      case VOLUME_TYPE_PAINT:
         s = "Paint";
         break;
      case VOLUME_TYPE_PROB_ATLAS:
         s = "Prob Atlas";
         break;
      case VOLUME_TYPE_RGB:
         s = "RGB";
         break;
      case VOLUME_TYPE_ROI:
         s = "ROI";
         break;
      case VOLUME_TYPE_SEGMENTATION:
         s = "Segmentation";
         break;
      case VOLUME_TYPE_VECTOR:
         s = "Vector";
         break;
      case VOLUME_TYPE_UNKNOWN:
         s = "Unknown";
         break;
   }
   
   return s;
}
      
/**
 * get a region name by its index.
 */
QString 
VolumeFile::getRegionNameFromIndex(const int index) const
{
   if ((index >= 0) && (index < getNumberOfRegionNames())) {
      return regionNames[index];
   }
   return "";
}

/**
 * get the index of a region name (returns -1 if not found)
 */
int 
VolumeFile::getRegionIndexFromName(const QString& name) const
{
   const int numNames = getNumberOfRegionNames();
   for (int i = 0; i < numNames; i++) {
      if (name == regionNames[i]) {
         return i;
      }
   }
   return -1;
}

/**
 * Set a region name.
 */
void
VolumeFile::setRegionName(int index, QString& name)
{
   if (index >= static_cast<int>(this->regionNames.size())) {
       this->regionNames.resize(index + 1, "");
   }
   this->regionNames[index] = name;
   this->setModified();
}

/**
 * remove all region names.
 */
void 
VolumeFile::deleteAllRegionNames()
{
   regionNames.clear();
}

/**
 * Create region names for voxels that do not index into a region.
 */
void
VolumeFile::createRegionNamesForVoxelsThatDoNotIndexIntoRegionNames()
{
   int numVoxels = this->getTotalNumberOfVoxels();
   for (int iv = 0; iv < numVoxels; iv++) {
       int voxel = static_cast<int>(this->voxels[iv]);

       if (voxel >= 0) {
           QString name = this->getRegionNameFromIndex(voxel);
           if (name.isEmpty()) {
               if (voxel > 0) {
                  name = "Region_" + QString::number(voxel);
               }
               else {
                  name = "???";
               }
               this->setRegionName(voxel, name);
           }
       }
   }
}

/**
 * add a region name (returns its index)
 */
int 
VolumeFile::addRegionName(const QString& name)
{
   const int index = getRegionIndexFromName(name);
   if (index >= 0) {
      return index;
   }
   
   //
   // WU NIL volume file cannot have a voxel value of 1.  So, the
   // zero index is "not in a region", and index one is unused.
   //
   if (regionNames.size() == 0) {
      regionNames.push_back("???");
      regionNames.push_back("???_not_used");
   }
   else if (regionNames.size() == 1) {
      if (regionNames[0] == "???_not_used") {
         regionNames.push_back("???_not_used1");
      }
      else {
         regionNames.push_back("???_not_used");
      }
   }
   
   //
   // Name could be one of the ??? names
   //
   const int index2 = getRegionIndexFromName(name);
   if (index2 >= 0) {
      return index2;
   }

   regionNames.push_back(name);

   
   return (regionNames.size() - 1);
}      

/**
 * set highlight a region name.
 */
void 
VolumeFile::setHighlightRegionName(const QString& name,
                                   const bool highlightItFlag)
{
   const int indx = getRegionIndexFromName(name);
   if (indx >= 0) {
      std::vector<int>::iterator iter = std::find(regionNameHighlighted.begin(),
                                                  regionNameHighlighted.end(),
                                                  indx);
      if (highlightItFlag) {
         if (iter == regionNameHighlighted.end()) {
            regionNameHighlighted.push_back(indx);
         }
      }
      else {
         if (iter != regionNameHighlighted.end()) {
            regionNameHighlighted.erase(iter);
         }
      }
      
      setVoxelColoringInvalid();
   }
}

/**
 * get a region is highlighted.
 */
bool 
VolumeFile::getHighlightRegionNameByIndex(const int indx) const
{
   const bool exists = (std::find(regionNameHighlighted.begin(),
                                  regionNameHighlighted.end(),
                                  indx) != regionNameHighlighted.end());
   return exists;
}

/**
 * clear region highlighting.
 */
void 
VolumeFile::clearRegionHighlighting()
{
   regionNameHighlighted.clear();
   setVoxelColoringInvalid();
}

/**
 * synchronize the region names in the volumes (index X is always region Y).
 */
void 
VolumeFile::synchronizeRegionNames(std::vector<VolumeFile*>& volumeFiles)
{
   const int numVolumes = static_cast<int>(volumeFiles.size());
   if (numVolumes <= 1) {
      return;
   }
   
   VolumeFile* firstVolume = volumeFiles[0];
   firstVolume->clearRegionHighlighting();
   
   //
   // Use region table from first volume file
   //
   for (int i = 1; i < numVolumes; i++) {
      //
      // Add region names for volume "i" to the first volume and build
      // translation from volume "i" to first volume region names
      //
      VolumeFile* vf = volumeFiles[i];
      const int numRegionNames = vf->getNumberOfRegionNames();
      std::vector<int> regionIndexUpdater(numRegionNames, 0);
      for (int j = 0; j < numRegionNames; j++) {
         regionIndexUpdater[j] = firstVolume->addRegionName(vf->getRegionNameFromIndex(j));
      }
      
      //
      // Update volume "i" voxels with new region indices
      //
      const int numVoxels = vf->getTotalNumberOfVoxels();
      for (int k = 0; k < numVoxels; k++) {
         vf->voxels[k] = regionIndexUpdater[static_cast<int>(vf->voxels[k])];
      }
      
      vf->clearRegionHighlighting();
   }
   
   //
   // replace the region names and invalidate coloring
   //
   for (int i = 1; i < numVolumes; i++) {
      volumeFiles[i]->regionNames = firstVolume->regionNames;
      volumeFiles[i]->setVoxelColoringInvalid();
   }
}      

/**
 * get the voxel distance (used by surface and volume rendering).
 */
float* 
VolumeFile::getVoxelToSurfaceDistances()
{
   if (voxelToSurfaceDistances == NULL) {
      if (voxels != NULL) {
         int dim[3];
         getDimensions(dim);
         const int num = dim[0] * dim[1] * dim[2];
         if (num > 0) {
            voxelToSurfaceDistances = new float[num];
            for (int i = 0; i < num; i++) {
               voxelToSurfaceDistances[i] = 0.0;
            }
         }
      }
   }
   return voxelToSurfaceDistances;
}

/**
 * get axis from string.
 */
VolumeFile::VOLUME_AXIS 
VolumeFile::getAxisFromString(const QString& sIn)
{
   const QString& s = sIn.toUpper();
   
   if (s == "X") {
      return VOLUME_AXIS_X;
   }
   else if (s == "Y") {
      return VOLUME_AXIS_Y;
   }
   else if (s == "Z") {
      return VOLUME_AXIS_Z;
   }
   else if (s == "ALL") {
      return VOLUME_AXIS_ALL;
   }
   else if (s == "OBLIQUE") {
      return VOLUME_AXIS_OBLIQUE;
   }
   else if (s == "X-OBLIQUE") {
      return VOLUME_AXIS_OBLIQUE_X;
   }
   else if (s == "Y-OBLIQUE") {
      return VOLUME_AXIS_OBLIQUE_Y;
   }
   else if (s == "Z-OBLIQUE") {
      return VOLUME_AXIS_OBLIQUE_Z;
   }
   else if (s == "ALL-OBLIQUE") {
      return VOLUME_AXIS_OBLIQUE_ALL;
   }
   else {
      return VOLUME_AXIS_UNKNOWN;
   }
}

/**
 * Get the label of an axis enumerated type.
 */
QString
VolumeFile::getAxisLabel(const VOLUME_AXIS axis)
{
   switch (axis) {
      case VOLUME_AXIS_X:
         return "X";
         break;
      case VOLUME_AXIS_Y:
         return "Y";
         break;
      case VOLUME_AXIS_Z:
         return "Z";
         break;
      case VOLUME_AXIS_ALL:
         return "ALL";
         break;
      case VOLUME_AXIS_OBLIQUE:
         return "OBLIQUE";
         break;
      case VOLUME_AXIS_OBLIQUE_X:
         return "X-OBLIQUE";
         break;
      case VOLUME_AXIS_OBLIQUE_Y:
         return "Y-OBLIQUE";
         break;
      case VOLUME_AXIS_OBLIQUE_Z:
         return "Z-OBLIQUE";
         break;
      case VOLUME_AXIS_OBLIQUE_ALL:
         return "ALL-OBLIQUE";
         break;
      case VOLUME_AXIS_UNKNOWN:
         return "UNKNOWN";
         break;
   }
   return "UNKNOWN";   
}

/**
 * get the size of a slice (number of voxels and components).
 */
int 
VolumeFile::getSizeOfSlice(const VOLUME_AXIS axis) const
{
   int sliceSize = 0;
   
   switch (axis) {
      case VOLUME_AXIS_X:
         sliceSize = dimensions[1] * dimensions[2] * getNumberOfComponentsPerVoxel();
         break;
      case VOLUME_AXIS_Y:
         sliceSize = dimensions[0] * dimensions[2] * getNumberOfComponentsPerVoxel();
         break;
      case VOLUME_AXIS_Z:
         sliceSize = dimensions[0] * dimensions[1] * getNumberOfComponentsPerVoxel();
         break;
      case VOLUME_AXIS_ALL:
         break;
      case VOLUME_AXIS_OBLIQUE:
         break;
      case VOLUME_AXIS_OBLIQUE_X:
         break;
      case VOLUME_AXIS_OBLIQUE_Y:
         break;
      case VOLUME_AXIS_OBLIQUE_Z:
         break;
      case VOLUME_AXIS_OBLIQUE_ALL:
         break;
      case VOLUME_AXIS_UNKNOWN:
         break;
   }
   
   return sliceSize;
}

/**
 * get a slice from a volume (sliceVoxelsOut should be size getSizeOfSlice()).
 */
void 
VolumeFile::getSlice(const VOLUME_AXIS axis,
                     const int sliceNumber,
                     const SLICE_DATA_ORDER dataOrder,
                     float* sliceVoxelsOut) const
{
   const int dimI = dimensions[0];
   const int dimJ = dimensions[1];
   const int dimK = dimensions[2];
   const int numComp = getNumberOfComponentsPerVoxel();
   
   switch (axis) {
      case VOLUME_AXIS_X:
         if (dataOrder == SLICE_DATA_ORDER_COLUMN) {
            int ctr = 0;
            for (int j = 0; j < dimJ; j++) {
               for (int k = 0; k < dimK; k++) {
                  for (int m = 0; m < numComp; m++) {
                     sliceVoxelsOut[ctr] = getVoxel(sliceNumber, j, k, m);
                     ctr++;
                  }
               }
            }
         }
         else {
            int ctr = 0;
            for (int k = 0; k < dimK; k++) {
               for (int j = 0; j < dimJ; j++) {
                  for (int m = 0; m < numComp; m++) {
                     sliceVoxelsOut[ctr] = getVoxel(sliceNumber, j, k, m);
                     ctr++;
                  }
               }
            }
         }
         break;
      case VOLUME_AXIS_Y:
         if (dataOrder == SLICE_DATA_ORDER_COLUMN) {
            int ctr = 0;
            for (int i = 0; i < dimI; i++) {
               for (int k = 0; k < dimK; k++) {
                  for (int m = 0; m < numComp; m++) {
                     sliceVoxelsOut[ctr] = getVoxel(i, sliceNumber, k, m);
                     ctr++;
                  }
               }
            }
         }
         else {
            int ctr = 0;
            for (int k = 0; k < dimK; k++) {
               for (int i = 0; i < dimI; i++) {
                  for (int m = 0; m < numComp; m++) {
                     sliceVoxelsOut[ctr] = getVoxel(i, sliceNumber, k, m);
                     ctr++;
                  }
               }
            }
         }
         break;
      case VOLUME_AXIS_Z:
         if (dataOrder == SLICE_DATA_ORDER_COLUMN) {
            int ctr = 0;
            for (int i = 0; i < dimI; i++) {
               for (int j = 0; j < dimJ; j++) {
                  for (int m = 0; m < numComp; m++) {
                     sliceVoxelsOut[ctr] = getVoxel(i, j, sliceNumber, m);
                     ctr++;
                  }
               }
            }
         }
         else {
            int ctr = 0;
            for (int j = 0; j < dimJ; j++) {
               for (int i = 0; i < dimI; i++) {
                  for (int m = 0; m < numComp; m++) {
                     sliceVoxelsOut[ctr] = getVoxel(i, j, sliceNumber, m);
                     ctr++;
                  }
               }
            }
         }
         break;
      case VOLUME_AXIS_ALL:
         break;
      case VOLUME_AXIS_OBLIQUE:
         break;
      case VOLUME_AXIS_OBLIQUE_X:
         break;
      case VOLUME_AXIS_OBLIQUE_Y:
         break;
      case VOLUME_AXIS_OBLIQUE_Z:
         break;
      case VOLUME_AXIS_OBLIQUE_ALL:
         break;
      case VOLUME_AXIS_UNKNOWN:
         break;
   }
}
              
/**
 * set a slice from a volume (sliceVoxelsIn should be size getSizeOfSlice()).
 */
void 
VolumeFile::setSlice(const VOLUME_AXIS axis,
                     const int sliceNumber,
                     const SLICE_DATA_ORDER dataOrder,
                     const float* sliceVoxelsIn)
{
   const int dimI = dimensions[0];
   const int dimJ = dimensions[1];
   const int dimK = dimensions[2];
   const int numComp = getNumberOfComponentsPerVoxel();
   
   switch (axis) {
      case VOLUME_AXIS_X:
         if (dataOrder == SLICE_DATA_ORDER_COLUMN) {
            int ctr = 0;
            for (int j = 0; j < dimJ; j++) {
               for (int k = 0; k < dimK; k++) {
                  for (int m = 0; m < numComp; m++) {
                     setVoxel(sliceNumber, j, k, m, sliceVoxelsIn[ctr]);
                     ctr++;
                  }
               }
            }
         }
         else {
            int ctr = 0;
            for (int k = 0; k < dimK; k++) {
               for (int j = 0; j < dimJ; j++) {
                  for (int m = 0; m < numComp; m++) {
                     setVoxel(sliceNumber, j, k, m, sliceVoxelsIn[ctr]);
                     ctr++;
                  }
               }
            }
         }
         break;
      case VOLUME_AXIS_Y:
         if (dataOrder == SLICE_DATA_ORDER_COLUMN) {
            int ctr = 0;
            for (int i = 0; i < dimI; i++) {
               for (int k = 0; k < dimK; k++) {
                  for (int m = 0; m < numComp; m++) {
                     setVoxel(i, sliceNumber, k, m, sliceVoxelsIn[ctr]);
                     ctr++;
                  }
               }
            }
         }
         else {
            int ctr = 0;
            for (int k = 0; k < dimK; k++) {
               for (int i = 0; i < dimI; i++) {
                  for (int m = 0; m < numComp; m++) {
                     setVoxel(i, sliceNumber, k, m, sliceVoxelsIn[ctr]);
                     ctr++;
                  }
               }
            }
         }
         break;
      case VOLUME_AXIS_Z:
         if (dataOrder == SLICE_DATA_ORDER_COLUMN) {
            int ctr = 0;
            for (int i = 0; i < dimI; i++) {
               for (int j = 0; j < dimJ; j++) {
                  for (int m = 0; m < numComp; m++) {
                     setVoxel(i, j, sliceNumber, m, sliceVoxelsIn[ctr]);
                     ctr++;
                  }
               }
            }
         }
         else {
            int ctr = 0;
            for (int j = 0; j < dimJ; j++) {
               for (int i = 0; i < dimI; i++) {
                  for (int m = 0; m < numComp; m++) {
                     setVoxel(i, j, sliceNumber, m, sliceVoxelsIn[ctr]);
                     ctr++;
                  }
               }
            }
         }
         break;
      case VOLUME_AXIS_ALL:
         break;
      case VOLUME_AXIS_OBLIQUE:
         break;
      case VOLUME_AXIS_OBLIQUE_X:
         break;
      case VOLUME_AXIS_OBLIQUE_Y:
         break;
      case VOLUME_AXIS_OBLIQUE_Z:
         break;
      case VOLUME_AXIS_OBLIQUE_ALL:
         break;
      case VOLUME_AXIS_UNKNOWN:
         break;
   }
}
                    
/**
 * Rotate the volume clockwise about an axis while loooking down the axis
 * from the positive end of the axis to the negative end of the axis.
 */
void
VolumeFile::rotate(const VOLUME_AXIS axis)
{
   if (DebugControl::getDebugOn()) {
      std::cout << "VolumeFile rotating about axis: " << getAxisLabel(axis).toAscii().constData() << std::endl;
   }
   
   //
   // Copy current volume into the "old" volume
   //
   VolumeFile oldVolume = *this;

   //
   // Get information on current volume
   //
   int oldDim[3];
   oldVolume.getDimensions(oldDim);
   float oldSpacing[3];
   oldVolume.getSpacing(oldSpacing);
   float oldOrigin[3];
   oldVolume.getOrigin(oldOrigin);
   ORIENTATION oldOrientation[3];
   oldVolume.getOrientation(oldOrientation);
   
   //
   // Does this volume have a valid orientation.  If not do not adjust spacing and origin.
   //
   bool validOrientation = false;
   switch (volumeSpace) {
      case VOLUME_SPACE_COORD_LPI:
         validOrientation = isValidOrientation(orientation);
         break;
      case VOLUME_SPACE_VOXEL_NATIVE:
         break;
   }
   
   //
   // Setup new dimensions and let spacing follow
   //
   // NOTE: IF THE VOLUME IS ROTATED USING EDIT VOLUME ATTRIBUTES, THE VOLUME MAY
   // APPEAR "UPSIDE DOWN".  THIS MAY BE CORRECTED BY FLIPPING THE SIGNS OF THE
   // ORIGIN AND SPACING FOR ONE OF THE AXIS.
   //
   int newDim[3] = { dimensions[0], dimensions[1], dimensions[2] };
   float newSpacing[3] = { oldSpacing[0], oldSpacing[1], oldSpacing[2] };
   float newOrigin[3] = { oldOrigin[0], oldOrigin[1], oldOrigin[2] };
   ORIENTATION newOrientation[3] = { oldOrientation[0], oldOrientation[1], oldOrientation[2] };
   switch (axis) {
      case VOLUME_AXIS_X:
         newDim[0] = oldDim[0];
         newDim[1] = oldDim[2];
         newDim[2] = oldDim[1];
         newSpacing[0] = oldSpacing[0];
         newSpacing[1] = oldSpacing[2];
         newSpacing[2] = oldSpacing[1];
         if (validOrientation) {
            newSpacing[2] = -oldSpacing[1];
            newOrigin[0]  = oldOrigin[0];
            newOrigin[1]  = oldOrigin[2];
            newOrigin[2]  = (oldDim[1] * oldSpacing[1] + oldOrigin[1]);
            newOrientation[VOLUME_AXIS_X] = oldOrientation[VOLUME_AXIS_X];
            newOrientation[VOLUME_AXIS_Y] = oldOrientation[VOLUME_AXIS_Z];
            newOrientation[VOLUME_AXIS_Z] = getInverseOrientation(oldOrientation[VOLUME_AXIS_Y]);
         }
         break;
      case VOLUME_AXIS_Y:
         newDim[0] = oldDim[2];
         newDim[1] = oldDim[1];
         newDim[2] = oldDim[0];
         newSpacing[0] = oldSpacing[2];
         newSpacing[1] = oldSpacing[1];
         newSpacing[2] = oldSpacing[0];
         if (validOrientation) {
            newSpacing[2] = -oldSpacing[0];
            newOrigin[0]  = oldOrigin[2];
            newOrigin[1]  = oldOrigin[1];
            newOrigin[2]  = (oldDim[0] * oldSpacing[0] + oldOrigin[0]);
            newOrientation[VOLUME_AXIS_X] = oldOrientation[VOLUME_AXIS_Z];
            newOrientation[VOLUME_AXIS_Y] = oldOrientation[VOLUME_AXIS_Y];
            newOrientation[VOLUME_AXIS_Z] = getInverseOrientation(oldOrientation[VOLUME_AXIS_X]);
         }
         break;
      case VOLUME_AXIS_Z:
         newDim[0] = oldDim[1];
         newDim[1] = oldDim[0];
         newDim[2] = oldDim[2];
         newSpacing[0] = oldSpacing[1];
         newSpacing[1] = oldSpacing[0];
         newSpacing[2] = oldSpacing[2];
         if (validOrientation) {
            newSpacing[1] = -oldSpacing[0];
            newOrigin[0]  = oldOrigin[1];
            newOrigin[1]  = (oldDim[0] * oldSpacing[0] + oldOrigin[0]);
            newOrigin[2]  = oldOrigin[2];
            newOrientation[VOLUME_AXIS_X] = oldOrientation[VOLUME_AXIS_Y];
            newOrientation[VOLUME_AXIS_Y] = getInverseOrientation(oldOrientation[VOLUME_AXIS_X]);
            newOrientation[VOLUME_AXIS_Z] = oldOrientation[VOLUME_AXIS_Z];
         }
         break;
      case VOLUME_AXIS_ALL:
         std::cout << "ALL axis not supported for rotate() in VolumeFile." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE:
         std::cout << "OBLIQUE axis not supported for rotate() in VolumeFile." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE_X:
         std::cout << "X OBLIQUE axis not supported for rotate() in VolumeFile." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE_Y:
         std::cout << "Y OBLIQUE axis not supported for rotate() in VolumeFile." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE_Z:
         std::cout << "Z OBLIQUE axis not supported for rotate() in VolumeFile." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE_ALL:
         std::cout << "ALL OBLIQUE axis not supported for rotate() in VolumeFile." << std::endl;
         return;
         break;
      case VOLUME_AXIS_UNKNOWN:
         std::cout << "UNKNOWN axis not supported for rotate() in VolumeFile." << std::endl;
         return;
         break;
   }
   
   //
   // Create a new structured points and initialize it
   //
   setDimensions(newDim);
   setOrigin(newOrigin);
   setSpacing(newSpacing);
   setOrientation(newOrientation);

   if (voxels != NULL) {
      //
      // Rotate the voxels into the new volume
      //
      switch (axis) {
         case VOLUME_AXIS_X:
            {
               //
               // Size of slice and memory for slice
               //
               const int sliceSize = getSizeOfSlice(VOLUME_AXIS_Z);
               float* sliceVoxels = new float[sliceSize];
               
               //
               // Fill slices into new volume along K-axis going min to max
               //
               for (int k = 0; k < newDim[2]; k++) {
                  //
                  // Slice from old volume along J-axis going max to min
                  //
                  const int oldSliceNum = oldDim[1] - k - 1;
                  
                  //
                  // Get the slice from the old volume
                  //
                  oldVolume.getSlice(VOLUME_AXIS_Y, oldSliceNum, SLICE_DATA_ORDER_ROW, sliceVoxels);
                  
                  //
                  // Put slice in new volume
                  //
                  setSlice(VOLUME_AXIS_Z, k, SLICE_DATA_ORDER_ROW, sliceVoxels);
               }
                              
               delete[] sliceVoxels;
            }
            break;
         case VOLUME_AXIS_Y:
            {
               //
               // Size of slice and memory for slice
               //
               const int sliceSize = getSizeOfSlice(VOLUME_AXIS_Z);
               float* sliceVoxels = new float[sliceSize];
               
               //
               // Fill slices into new volume along K-axis going min to max
               //
               for (int k = 0; k < newDim[2]; k++) {
                  //
                  // Slice from old volume along I-axis going max to min
                  //
                  const int oldSliceNum = oldDim[0] - k - 1;
                  
                  //
                  // Get the slice from the old volume
                  //
                  oldVolume.getSlice(VOLUME_AXIS_X, oldSliceNum, SLICE_DATA_ORDER_COLUMN, sliceVoxels);
                  
                  //
                  // Put slice in new volume
                  //
                  setSlice(VOLUME_AXIS_Z, k, SLICE_DATA_ORDER_ROW, sliceVoxels);
               }
                              
               delete[] sliceVoxels;
            }
            break;
         case VOLUME_AXIS_Z:
            {
               //
               // Size of slice and memory for slice
               //
               const int sliceSize = getSizeOfSlice(VOLUME_AXIS_Y);
               float* sliceVoxels = new float[sliceSize];
               
               //
               // Fill slices into new volume along J-axis going min to max
               //
               for (int j = 0; j < newDim[1]; j++) {
                  //
                  // Slice from old volume along I-axis going max to min
                  //
                  const int oldSliceNum = oldDim[0] - j - 1;   //j;
                  
                  //
                  // Get the slice from the old volume
                  //
                  oldVolume.getSlice(VOLUME_AXIS_X, oldSliceNum, SLICE_DATA_ORDER_ROW, sliceVoxels);
                  
                  //
                  // Put slice in new volume
                  //
                  setSlice(VOLUME_AXIS_Y, j, SLICE_DATA_ORDER_ROW, sliceVoxels);
               }
                              
               delete[] sliceVoxels;
            }
/*
            for (int k = 0; k < newDim[2]; k++) {
               for (int j = 0; j < newDim[1]; j++) {
                  for (int i = 0; i < newDim[0]; i++) {
                     int newIndx = i + j * newDim[0] + k * newDim[0] * newDim[1];
                     const int oi = (oldDim[0] - 1) - j;
                     const int oj = i;
                     const int ok = k;
                     int oldIndx = oi + oj * oldDim[0] + ok * oldDim[0] * oldDim[1];
                     
                     for (int m = 0; m < numComponents; m++) {
                        voxels[newIndx + m] = oldVoxels[oldIndx + m];
                     }
                  }
               }
            }
*/
            break;
         case VOLUME_AXIS_ALL:
            std::cout << "ALL axis not supported for rotate() in VolumeFile." << std::endl;
            return;
            break;
         case VOLUME_AXIS_OBLIQUE:
            std::cout << "OBLIQUE axis not supported for rotate() in VolumeFile." << std::endl;
            return;
            break;
         case VOLUME_AXIS_OBLIQUE_X:
            std::cout << "X OBLIQUE axis not supported for rotate() in VolumeFile." << std::endl;
            return;
            break;
         case VOLUME_AXIS_OBLIQUE_Y:
            std::cout << "Y OBLIQUE axis not supported for rotate() in VolumeFile." << std::endl;
            return;
            break;
         case VOLUME_AXIS_OBLIQUE_Z:
            std::cout << "Z OBLIQUE axis not supported for rotate() in VolumeFile." << std::endl;
            return;
            break;
         case VOLUME_AXIS_OBLIQUE_ALL:
            std::cout << "ALL OBLIQUE axis not supported for rotate() in VolumeFile." << std::endl;
            return;
            break;
         case VOLUME_AXIS_UNKNOWN:
            std::cout << "UNKNOWN axis not supported for rotate() in VolumeFile." << std::endl;
            return;
            break;
      }
   }
   
   allocateVoxelColoring();

   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 * Determine if a voxel index is valid.
 */
bool
VolumeFile::getVoxelIndexValid(const VoxelIJK& v) const
{
   return getVoxelIndexValid(v.getIJK());
}

/**
 * Determine if a voxel index is valid.
 */
bool
VolumeFile::getVoxelIndexValid(const int ijk[3]) const
{
   if (ijk[0] < 0) return false;
   if (ijk[0] >= dimensions[0]) return false;
   if (ijk[1] < 0) return false;
   if (ijk[1] >= dimensions[1]) return false;
   if (ijk[2] < 0) return false;
   if (ijk[2] >= dimensions[2]) return false;
   
   return true;
}

/**
 * Determine if a voxel index is valid.
 */
bool
VolumeFile::getVoxelIndexValid(const int i, const int j, const int k) const
{
   if (i < 0) return false;
   if (i >= dimensions[0]) return false;
   if (j < 0) return false;
   if (j >= dimensions[1]) return false;
   if (k < 0) return false;
   if (k >= dimensions[2]) return false;
   
   return true;
}

/**
 * get a voxel with a flat index.
 */
float 
VolumeFile::getVoxelWithFlatIndex(const int indx, const int component) const
{
   return voxels[indx*numberOfComponentsPerVoxel + component];
}

/**
 * set a voxel with a flat index.
 */
void 
VolumeFile::setVoxelWithFlatIndex(const int indx, const int component, const float value)
{
   voxels[indx*numberOfComponentsPerVoxel + component] = value;
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}
      
/**
 * get the coordinate of a voxel.
 */
void 
VolumeFile::getVoxelCoordinate(const int ijk[3], 
                               float coord[3]) const
{
   getVoxelCoordinate(ijk[0], ijk[1], ijk[2], coord);
}

/**
 * get the coordinate at the center of the voxel.
 */
void 
VolumeFile::getVoxelCoordinate(const int i, const int j, const int k,
                               float coord[3]) const
{
   coord[0] = origin[0] + spacing[0] * i;
   coord[1] = origin[1] + spacing[1] * j;
   coord[2] = origin[2] + spacing[2] * k;
/*
   if (centerOfVoxelFlag) {
      coord[0] += spacing[0] * 0.5;
      coord[1] += spacing[1] * 0.5;
      coord[2] += spacing[2] * 0.5;
   }
*/
}

/// get the coordinate at the center of the voxel
void 
VolumeFile::getVoxelCoordinate(const VoxelIJK& v, 
                               float coord[3]) const
{
   getVoxelCoordinate(v.ijkv[0], v.ijkv[1], v.ijkv[2], coord);
}

/**
 * get a voxel.
 */
float 
VolumeFile::getVoxel(const int i, const int j, const int k, const int component) const
{
   const int indx = getVoxelDataIndex(i, j, k, component);
   return voxels[indx];
}

/**
 * get a voxel.
 */
float 
VolumeFile::getVoxel(const int ijk[3], const int component) const
{
   const int indx = getVoxelDataIndex(ijk, component);
   return voxels[indx];
}
      
/**
 * get a voxel.
 */
float 
VolumeFile::getVoxel(const VoxelIJK& v, const int component) const
{
   const int indx = getVoxelDataIndex(v.getIJK(), component);
   return voxels[indx];
}
      
/**
 * Get the value at a voxel.  Returns true if a valid voxel index.
 * "voxelValue" should be allocated by the user and have
 * "getNumberOfComponentsPerVoxel" number of voxels.
 */
bool
VolumeFile::getVoxelAllComponents(const int ijk[3], float* voxelValue) const
{
   if (getVoxelIndexValid(ijk)) {
      if (voxels != NULL) {
         const int indx = getVoxelDataIndex(ijk);
         for (int i = 0; i < numberOfComponentsPerVoxel; i++) {
            voxelValue[i] = voxels[indx + i];
         }
         return true;
      }
   }
   return false;
}

/**
 * get the index of this voxel ignoring the components).
 */
int 
VolumeFile::getVoxelNumber(const int i, const int j, const int k) const
{
   const int indx = i + j * dimensions[0] + k * dimensions[0] * dimensions[1];
   return indx;
}

/** 
 * get the index of this voxel ignoring the components).
 */
int 
VolumeFile::getVoxelNumber(const VoxelIJK& v) const
{
   return getVoxelNumber(v.ijkv[0], v.ijkv[1], v.ijkv[2]);
}

/**
 * get the index of this voxel ignoring the components).
 */
int 
VolumeFile::getVoxelNumber(const int ijk[3]) const
{
   return getVoxelNumber(ijk[0], ijk[1], ijk[2]);
}
          
/**
 * Get the value at a voxel.  Returns true if a valid voxel index.
 * "voxelValue" should be allocated by the user and have
 * "getNumberOfComponentsPerVoxel" number of voxels.
 */
bool
VolumeFile::getVoxelAllComponents(const int i, const int j, const int k, float* voxelValue) const
{
   int ijk[3] = { i, j, k };
   return getVoxelAllComponents(ijk, voxelValue);
}

/**
 * Set the value at a voxel.  
 * "voxelValue" should have "getNumberOfComponentsPerVoxel" number of voxels.
 */
void
VolumeFile::setVoxelAllComponents(const int ijk[3], const float* voxelValue) 
{
   if (getVoxelIndexValid(ijk)) {
      if (voxels != NULL) {
         const int indx = getVoxelDataIndex(ijk);
         for (int i = 0; i < numberOfComponentsPerVoxel; i++) {
            voxels[indx + i] = voxelValue[i];
         }
         setModified();
         minMaxVoxelValuesValid = false;
         minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
         
         //
         // Set color invalid for this voxel
         //
         if (voxelColoring != NULL) {
            const int indx = getVoxelColorIndex(ijk);
            voxelColoring[indx+3] = VOXEL_COLOR_STATUS_INVALID;
         }
      }
   }
}

/**
 * set all of the voxels in a slice.
 */
void 
VolumeFile::setAllVoxelsInSlice(const VOLUME_AXIS axis, const int sliceNumber,
                                const float value)
{
   if (sliceNumber < 0) {
      return;
   }
   
   int iMin = 0;
   int iMax = dimensions[0] - 1;
   int jMin = 0;
   int jMax = dimensions[1] - 1;
   int kMin = 0;
   int kMax = dimensions[2] - 1;
   switch (axis) {
      case VOLUME_AXIS_X:
         if (sliceNumber > iMax) {
            return;
         }
         iMin = sliceNumber;
         iMax = sliceNumber;
         break;
      case VOLUME_AXIS_Y:
         if (sliceNumber > jMax) {
            return;
         }
         jMin = sliceNumber;
         jMax = sliceNumber;
         break;
      case VOLUME_AXIS_Z:
         if (sliceNumber > kMax) {
            return;
         }
         kMin = sliceNumber;
         kMax = sliceNumber;
         break;
      case VOLUME_AXIS_ALL:
      case VOLUME_AXIS_OBLIQUE:
      case VOLUME_AXIS_OBLIQUE_X:
      case VOLUME_AXIS_OBLIQUE_Y:
      case VOLUME_AXIS_OBLIQUE_Z:
      case VOLUME_AXIS_OBLIQUE_ALL:
      case VOLUME_AXIS_UNKNOWN:
         return;
         break;
   }
   
   const int numComp = getNumberOfComponentsPerVoxel();
   for (int i = iMin; i <= iMax; i++) {
      for (int j = jMin; j <= jMax; j++) {
         for (int k = kMin; k <= kMax; k++) {
            for (int m = 0; m < numComp; m++) {
               setVoxel(i, j, k, m, value);
            }
         }
      }
   }
}                               

/**
 * Set the value at a voxel.
 */
void
VolumeFile::setVoxel(const int ijk[3], const int component, const float voxelValue) 
{
   if (getVoxelIndexValid(ijk)) {
      if (voxels != NULL) {
         const int indx = getVoxelDataIndex(ijk);
         voxels[indx + component] = voxelValue;
         setModified();
         minMaxVoxelValuesValid = false;
         minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
         
         //
         // Set color invalid for this voxel
         //
         if (voxelColoring != NULL) {
            const int indx = getVoxelColorIndex(ijk);
            voxelColoring[indx+3] = VOXEL_COLOR_STATUS_INVALID;
         }
      }
   }
}

/**
 * set a voxel at the specified index.
 */
void 
VolumeFile::setVoxel(const VoxelIJK& v, const int component, const float voxelValue)
{
   const int ijk[3] = { v.getI(), v.getJ(), v.getK() };
   setVoxel(ijk, component, voxelValue);
}
      
/**
 * set the value at a voxel.
 * "voxelValue" should be have "getNumberOfComponentsPerVoxel" number of voxels.
 */
void
VolumeFile::setVoxelAllComponents(const int i, const int j, const int k, const float* voxelValue) 
{
   int ijk[3] = { i, j, k };
   setVoxelAllComponents(ijk, voxelValue);
}

/**
 * set the value at a voxel.
 */
void
VolumeFile::setVoxel(const int i, const int j, const int k, const int component,
                     const float voxelValue) 
{
   int ijk[3] = { i, j, k };
   setVoxel(ijk, component, voxelValue);
}

/**
 * set a bunch of voxels using one dimensional index.
 */
void 
VolumeFile::setVoxel(const std::vector<int> indicies, const float voxelValue)
{
   const int num = static_cast<int>(indicies.size());
   
   if (voxels != NULL) {
      if (numberOfComponentsPerVoxel > 0) {
         for (int i = 0; i < num; i++) {
            const int id = indicies[i];
            for (int j = 0; j < numberOfComponentsPerVoxel; j++) {
               voxels[id+j] = voxelValue;
            }
            
            //
            // Set color invalid for this voxel
            //
            if (voxelColoring != NULL) {
               const int id4 = id * 4;
               voxelColoring[id4+3] = VOXEL_COLOR_STATUS_INVALID;
            }
         }
         setModified();
         minMaxVoxelValuesValid = false;
         minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
      }
   }
}
      
/**
 * set all voxels within a rectangle.
 */
void 
VolumeFile::setAllVoxelsInRectangle(const int extentIn[6],
                                    const float value)
{
   int extent[6] = {
      extentIn[0],
      extentIn[1],
      extentIn[2],
      extentIn[3],
      extentIn[4],
      extentIn[5]
   };
   
   clampVoxelDimension(VOLUME_AXIS_X, extent[0]);
   clampVoxelDimension(VOLUME_AXIS_X, extent[1]);
   clampVoxelDimension(VOLUME_AXIS_Y, extent[2]);
   clampVoxelDimension(VOLUME_AXIS_Y, extent[3]);
   clampVoxelDimension(VOLUME_AXIS_Z, extent[4]);
   clampVoxelDimension(VOLUME_AXIS_Z, extent[5]);

   for (int i = extent[0]; i < extent[1]; i++) {
      for (int j = extent[2]; j < extent[3]; j++) {
         for (int k = extent[4]; k < extent[5]; k++) {
            for (int m = 0; m < numberOfComponentsPerVoxel; m++) {
               setVoxel(i, j, k, m, value);
            }
         }
      }
   }
}

/**
 * set all of the voxels to a value.
 */
void 
VolumeFile::setAllVoxels(const float value)
{
   const int num = getTotalNumberOfVoxelElements();
   for (int i = 0; i < num; i++) {
      voxels[i] = value;
   }
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
   setVoxelColoringInvalid();
}
      
/**
 * Set the colors for a voxel.  This should only be used for editing the volume so 
 * the colors match the voxel value.  Note the 4th byte denotes whether or not
 * the voxel should be displayed and if the coloring is valid.
 */
void
VolumeFile::setVoxelColor(const int ijk[3], const unsigned char rgb[4])
{
   if (getVoxelIndexValid(ijk)) {
      if (voxelColoring != NULL) {
         const int indx = getVoxelColorIndex(ijk);
         voxelColoring[indx]   = rgb[0];
         voxelColoring[indx+1] = rgb[1];
         voxelColoring[indx+2] = rgb[2];
         voxelColoring[indx+3] = rgb[3];
      }
   }
}

/**
 * Set the colors for a voxel.  This should only be used for editing the volume so 
 * the colors match the voxel value.  Note the 4th byte denotes whether or not
 * the voxel should be displayed and if the coloring is valid.
 */
void
VolumeFile::setVoxelColor(const int i, const int j, const int k, const unsigned char rgb[4])
{
   if (getVoxelIndexValid(i, j, k)) {
      if (voxelColoring != NULL) {
         const int indx = getVoxelColorIndex(i, j, k);
         voxelColoring[indx]   = rgb[0];
         voxelColoring[indx+1] = rgb[1];
         voxelColoring[indx+2] = rgb[2];
         voxelColoring[indx+3] = rgb[3];
      }
   }
}

/**
 * Set all voxels' colors invalid if the global coloring invalid flag is set
 */
void
VolumeFile::checkForInvalidVoxelColors()
{
   if (voxelColoringValid == false) {
      if (voxelColoring != NULL) {
         voxelColoringValid = true;
         const int numVoxels = getTotalNumberOfVoxels();
         for (int i = 0; i < numVoxels; i++) {
            voxelColoring[i*4+3] = VOXEL_COLOR_STATUS_INVALID;
         }
      }
   }
}

/**
 * Get the color for a voxel.  Note the 4th byte denotes whether or not
 * the voxel should be displayed.  Returns true if a valid voxel index.
 */
bool
VolumeFile::getVoxelColor(const int ijk[3], unsigned char rgb[4])
{
   if (getVoxelIndexValid(ijk)) {
      if (voxelColoring != NULL) {
         const int indx = getVoxelColorIndex(ijk);
         checkForInvalidVoxelColors();
         rgb[0] = voxelColoring[indx];
         rgb[1] = voxelColoring[indx+1];
         rgb[2] = voxelColoring[indx+2];
         rgb[3] = voxelColoring[indx+3];
         return true;
      }
   }
   return false;
}

/**
 * compute a voxel color index.
 */
int 
VolumeFile::getVoxelColorIndex(const VoxelIJK& v) const
{
   return getVoxelColorIndex(v.getI(), v.getJ(), v.getK());
}

/**
 * compute a voxel color data index.
 */
int 
VolumeFile::getVoxelColorIndex(const int ijk[3]) const
{
   return getVoxelColorIndex(ijk[0], ijk[1], ijk[2]);
}

/**
 * compute a voxel color data index.
 */
int 
VolumeFile::getVoxelColorIndex(const int i, const int j, const int k) const
{
   const int indx = (i + j * dimensions[0] + k * dimensions[0] * dimensions[1]) * 4;
   return indx;
}
      
/**
 * Get the color for a voxel.  Note the 4th byte denotes whether or not
 * the voxel should be displayed.  Returns true if a valid voxel index.
 */
bool
VolumeFile::getVoxelColor(const int i, const int j, const int k, unsigned char rgb[4])
{
   const int ijk[3] = { i, j, k };

   return getVoxelColor(ijk, rgb);
}

/**
 * flip volume about an axis.
 * Cannot use vtkImageFlip because it crashes if more than one component per voxel (ie: rgb)
 */
void
VolumeFile::flip(const VOLUME_AXIS axis, const bool updateOrientation)
{
   int dim[3];
   getDimensions(dim);
   float spacing[3];
   getSpacing(spacing);
   float origin[3];
   getOrigin(origin);
   float* data1 = NULL;
   float* data2 = NULL;
   if (voxels != NULL) {
      data1 = new float[numberOfComponentsPerVoxel];
      data2 = new float[numberOfComponentsPerVoxel];
   }
   
   if (DebugControl::getDebugOn()) {
      std::cout << "VolumeFile flipping about axis: " << getAxisLabel(axis).toAscii().constData() << std::endl;
   }
   
   switch(axis) {
      case VOLUME_AXIS_X:
         if (voxels != NULL) {
            for (int iz = 0; iz < dim[2]; iz++) {
               for (int iy = 0; iy < dim[1]; iy++) {
                  const int halfDimX = dim[0] / 2;
                  for (int ix = 0; ix < halfDimX; ix++) {
                     const int ijk1[3] = { ix, iy, iz };
                     const int ijk2[3] = { dim[0] - ix - 1, iy, iz }; 
                     getVoxelAllComponents(ijk1, data1);
                     getVoxelAllComponents(ijk2, data2);
                     setVoxelAllComponents(ijk1, data2);
                     setVoxelAllComponents(ijk2, data1);
                  }
               }
            }
         }
         origin[0] = (dim[0] - 1)* spacing[0] + origin[0];
         spacing[0] = -spacing[0];
         break;
      case VOLUME_AXIS_Y:
         if (voxels != NULL) {
            for (int iz = 0; iz < dim[2]; iz++) {
               for (int ix = 0; ix < dim[0]; ix++) {
                  const int halfDimY = dim[1] / 2;
                  for (int iy = 0; iy < halfDimY; iy++) {
                     const int ijk1[3] = { ix, iy, iz };
                     const int ijk2[3] = { ix, dim[1] - iy - 1, iz }; 
                     getVoxelAllComponents(ijk1, data1);
                     getVoxelAllComponents(ijk2, data2);
                     setVoxelAllComponents(ijk1, data2);
                     setVoxelAllComponents(ijk2, data1);
                  }
               }
            }
         }
         origin[1] = (dim[1] - 1) * spacing[1] + origin[1];
         spacing[1] = -spacing[1];
         break;
      case VOLUME_AXIS_Z:
         if (voxels != NULL) {
            for (int ix = 0; ix < dim[0]; ix++) {
               for (int iy = 0; iy < dim[1]; iy++) {
                  const int halfDimZ = dim[2] / 2;
                  for (int iz = 0; iz < halfDimZ; iz++) {
                     const int ijk1[3] = { ix, iy, iz };
                     const int ijk2[3] = { ix, iy, dim[2] - iz - 1 }; 
                     getVoxelAllComponents(ijk1, data1);
                     getVoxelAllComponents(ijk2, data2);
                     setVoxelAllComponents(ijk1, data2);
                     setVoxelAllComponents(ijk2, data1);
                  }
               }
            }
         }
         origin[2] = (dim[2] - 1) * spacing[2] + origin[2];
         spacing[2] = -spacing[2];
         break;
      case VOLUME_AXIS_ALL:
         std::cout << "ALL axis not supported for flip() in VolumeFile." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE:
         std::cout << "OBLIQUE axis not supported for flip() in VolumeFile." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE_X:
         std::cout << "X OBLIQUE axis not supported for flip() in VolumeFile." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE_Y:
         std::cout << "Y OBLIQUE axis not supported for flip() in VolumeFile." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE_Z:
         std::cout << "Z OBLIQUE axis not supported for flip() in VolumeFile." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE_ALL:
         std::cout << "ALL OBLIQUE axis not supported for flip() in VolumeFile." << std::endl;
         return;
         break;
      case VOLUME_AXIS_UNKNOWN:
         std::cout << "UNKNOWN axis not supported for flip() in VolumeFile." << std::endl;
         return;
         break;
   }
   
   //
   // Only update spacing and origin if the orientation is valid
   //
   if (isValidOrientation(orientation) && updateOrientation) {
      switch (volumeSpace) {
         case VOLUME_SPACE_COORD_LPI:
            setSpacing(spacing);
            setOrigin(origin);
            break;
         case VOLUME_SPACE_VOXEL_NATIVE:
            break;
      }
   }
   
   if (data1 != NULL) {
      delete[] data1;
   }
   if (data2 != NULL) {
      delete[] data2;
   }
   
   if (updateOrientation) {
      orientation[axis] = getInverseOrientation(orientation[axis]);
   }
   
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
   setVoxelColoringInvalid();
}

/**
 * Get the volume extent of non-zero voxels.  Return voxel index range of non-zero voxels.
 */
void
VolumeFile::getNonZeroVoxelExtent(int extentVoxelIndices[6],
                                  float extentCoordinates[6]) const
{
   bool voxelsFound = false;
   
   if (voxels != NULL) {
      extentVoxelIndices[0] = dimensions[0] + 1;
      extentVoxelIndices[1] = -1;
      extentVoxelIndices[2] = dimensions[1] + 1;
      extentVoxelIndices[3] = -1;
      extentVoxelIndices[4] = dimensions[2] + 1;
      extentVoxelIndices[5] = -1;
   
      for (int i = 0; i < dimensions[0]; i++) {
         for (int j = 0; j < dimensions[1]; j++) {
            for (int k = 0; k < dimensions[2]; k++) {
               for (int m = 0; m < numberOfComponentsPerVoxel; m++) {
                  if (getVoxel(i, j, k, m) != 0.0) {
                     voxelsFound = true;
                     extentVoxelIndices[0] = std::min(extentVoxelIndices[0], i);
                     extentVoxelIndices[1] = std::max(extentVoxelIndices[1], i);
                     extentVoxelIndices[2] = std::min(extentVoxelIndices[2], j);
                     extentVoxelIndices[3] = std::max(extentVoxelIndices[3], j);
                     extentVoxelIndices[4] = std::min(extentVoxelIndices[4], k);
                     extentVoxelIndices[5] = std::max(extentVoxelIndices[5], k);
                  }
               }
            }
         }
      }
   }
   
   //
   // Nothing found
   //
   if (voxelsFound) {
      float xyz[3];
      getVoxelCoordinate(extentVoxelIndices[0],
                         extentVoxelIndices[2],
                         extentVoxelIndices[4],
                         xyz);
      extentCoordinates[0] = xyz[0]; 
      extentCoordinates[2] = xyz[1]; 
      extentCoordinates[4] = xyz[2]; 
      getVoxelCoordinate(extentVoxelIndices[1],
                         extentVoxelIndices[3],
                         extentVoxelIndices[5],
                         xyz);
      extentCoordinates[1] = xyz[0]; 
      extentCoordinates[3] = xyz[1]; 
      extentCoordinates[5] = xyz[2]; 
   }
   else {
      extentVoxelIndices[0] = -1;
      extentVoxelIndices[1] = -1;
      extentVoxelIndices[2] = -1;
      extentVoxelIndices[3] = -1;
      extentVoxelIndices[4] = -1;
      extentVoxelIndices[5] = -1;
   }
}

/**
 * Get orientation
 */
void 
VolumeFile::getOrientation(VolumeFile::ORIENTATION orient[3]) const
{
   orient[0] = orientation[0];
   orient[1] = orientation[1];
   orient[2] = orientation[2];
}

/**
 * set orientation.
 */
void 
VolumeFile::setOrientation(const VolumeFile::ORIENTATION orient[3])
{
   orientation[0] = orient[0];
   orientation[1] = orient[1];
   orientation[2] = orient[2];
   
   setModified();
}

/**
 * Returns true if the orientation is valid.  A valid orientation is one in which
 * one axis is left or right, one axis is posterior or anterior, and one axis is
 * inferior or superior.
 */
bool
VolumeFile::isValidOrientation(const VolumeFile::ORIENTATION orient[3])
{
   int lrCount = 0;
   int paCount = 0;
   int isCount = 0;
   
   for (int i = 0; i < 3; i++) {
      switch (orient[i]) {
         case ORIENTATION_RIGHT_TO_LEFT:
         case ORIENTATION_LEFT_TO_RIGHT:
            lrCount++;
            break;
         case ORIENTATION_POSTERIOR_TO_ANTERIOR:
         case ORIENTATION_ANTERIOR_TO_POSTERIOR:
            paCount++;
            break;
         case ORIENTATION_INFERIOR_TO_SUPERIOR:
         case ORIENTATION_SUPERIOR_TO_INFERIOR:
            isCount++;
            break;
         case ORIENTATION_UNKNOWN:
            break;
      }
   }
   
   if ((lrCount == 1) && (paCount == 1) && (isCount == 1)) {
      return true;
   }
   return false;
}

/**
 * Get the inverse orientation for an orientation (ie "left" is inverse of "right")
 */
VolumeFile::ORIENTATION  
VolumeFile::getInverseOrientation(const VolumeFile::ORIENTATION orient)
{
   switch (orient) {
      case ORIENTATION_UNKNOWN:
         return ORIENTATION_UNKNOWN;
         break;
      case ORIENTATION_RIGHT_TO_LEFT:
         return ORIENTATION_LEFT_TO_RIGHT;
         break;
      case ORIENTATION_LEFT_TO_RIGHT:
         return ORIENTATION_RIGHT_TO_LEFT;
         break;
      case ORIENTATION_POSTERIOR_TO_ANTERIOR:
         return ORIENTATION_ANTERIOR_TO_POSTERIOR;
         break;
      case ORIENTATION_ANTERIOR_TO_POSTERIOR:
         return ORIENTATION_POSTERIOR_TO_ANTERIOR;
         break;
      case ORIENTATION_INFERIOR_TO_SUPERIOR:
         return ORIENTATION_SUPERIOR_TO_INFERIOR;
         break;
      case ORIENTATION_SUPERIOR_TO_INFERIOR:
         return ORIENTATION_INFERIOR_TO_SUPERIOR;
         break;
   }
   return ORIENTATION_UNKNOWN;
}

/**
 * Get the ac position 
 */
void
VolumeFile::getSpmAcPosition(float ac[3]) const
{
   ac[0] = spmAcPosition[0];
   ac[1] = spmAcPosition[1];
   ac[2] = spmAcPosition[2];
}

/**
 * Set the ac position 
 */
void
VolumeFile::setSpmAcPosition(const float ac[3])
{
   spmAcPosition[0] = ac[0];
   spmAcPosition[1] = ac[1];
   spmAcPosition[2] = ac[2];
   
   setModified();
}

/**
 * get the dimensions of the volume (const method).
 */
void 
VolumeFile::getDimensions(int& dimX, int& dimY, int& dimZ) const
{
   dimX = dimensions[0];
   dimY = dimensions[1];
   dimZ = dimensions[2];
}
      
/**
 * Get the dimensions (const method)
 */
void
VolumeFile::getDimensions(int dim[3]) const
{
   dim[0] = dimensions[0];
   dim[1] = dimensions[1];
   dim[2] = dimensions[2];
}

/**
 * Get the dimensions 
 */
void
VolumeFile::getDimensions(int dim[3]) 
{
   dim[0] = dimensions[0];
   dim[1] = dimensions[1];
   dim[2] = dimensions[2];
}

/**
 * Set the dimensions 
 */
void
VolumeFile::setDimensions(const int dim[3])
{
   dimensions[0] = dim[0];
   dimensions[1] = dim[1];
   dimensions[2] = dim[2];
   setModified();
}

/**
 * Get the origin (at the center of the first voxel)
 */
void
VolumeFile::getOrigin(float originOut[3]) const
{
   originOut[0] = origin[0];
   originOut[1] = origin[1];
   originOut[2] = origin[2];
}

/**
 * Set the origin (at the center of the first voxel)
 */
void
VolumeFile::setOrigin(const float originIn[3])
{
   origin[0] = originIn[0];
   origin[1] = originIn[1];
   origin[2] = originIn[2];
   setModified();
}

/**
 * get the origin at the corner of the first voxel.
 */
void 
VolumeFile::getOriginAtCornerOfVoxel(float originCornerOfVoxelOut[3]) const
{
   originCornerOfVoxelOut[0] = origin[0] - (spacing[0] * 0.5);
   originCornerOfVoxelOut[1] = origin[1] - (spacing[1] * 0.5);
   originCornerOfVoxelOut[2] = origin[2] - (spacing[2] * 0.5);
}

/**
 * set the origin at the corner of the first voxel.
 */
void 
VolumeFile::setOriginAtCornerOfVoxel(const float originCornerOfVoxelIn[3],
                                     const float voxelSizesIn[3])
{
   origin[0] = originCornerOfVoxelIn[0] + (voxelSizesIn[0] * 0.5);
   origin[1] = originCornerOfVoxelIn[1] + (voxelSizesIn[1] * 0.5);
   origin[2] = originCornerOfVoxelIn[2] + (voxelSizesIn[2] * 0.5);
   setModified();
}
      
/**
 * Get the spacing 
 */
void
VolumeFile::getSpacing(float spacingOut[3]) const
{
   spacingOut[0] = spacing[0];
   spacingOut[1] = spacing[1];
   spacingOut[2] = spacing[2];
}

/**
 * Set the spacing 
 */
void
VolumeFile::setSpacing(const float spacingIn[3])
{
   spacing[0] = spacingIn[0];
   spacing[1] = spacingIn[1];
   spacing[2] = spacingIn[2];
   setModified();
}

/**
 * Resize the volume; if provided, update params with cropping.
 */
void
VolumeFile::resize(const int cropping[6],
                   ParamsFile* paramsFile)
{
   if (voxels != NULL) {
      //
      // Get the new dimensions
      //
      int dimNew[3] = { cropping[1] - cropping[0] + 1,
                        cropping[3] - cropping[2] + 1,
                        cropping[5] - cropping[4] + 1 };
      if ((dimNew[0] < 1) || (dimNew[1] < 1) || (dimNew[2] < 1)) {
         return;
      }
      
      //
      // if expanding an axis
      //
      for (int i = 0; i < 3; i++) {
         if (dimNew[i] > dimensions[i]) {
            //
            // Take one voxel off
            //
            dimNew[i]--;
         }
      }
      
      //
      // Keep the old volume data so that it can be copied
      //
      float* oldVoxels = voxels;
      float space[3];
      getSpacing(space);
      int dimOld[3];
      getDimensions(dimOld);
      float originOld[3];
      getOrigin(originOld);
      
      //
      // Create new volume data
      //                                      
      setDimensions(dimNew);
      setSpacing(spacing);
      voxels = new float[getTotalNumberOfVoxelElements()];
      
      //
      // Set the new origin
      //
      float originNew[3] = { 
                             cropping[0] * spacing[0] + originOld[0],
                             cropping[2] * spacing[1] + originOld[1],
                             cropping[4] * spacing[2] + originOld[2]
                           };
      setOrigin(originNew);
      
      //
      // Load the cropped volumes voxels
      //
      for (int k = 0; k < dimNew[2]; k++) {
         for (int j = 0; j < dimNew[1]; j++) {
            for (int i = 0; i < dimNew[0]; i++) {
               //
               // Index to voxel that is to be set
               //
               int ijk[3] = { i, j, k };
               const int pointID = getVoxelDataIndex(ijk);
               
               //
               // IJK index of voxel in "old" volume (prior to resizing)
               //
               int oldIJK[3] = { i + cropping[0], j + cropping[2], k + cropping[4] };
               
               //
               // Is location valid in volume before resizing
               //
               if ((oldIJK[0] >= 0) && (oldIJK[0] < dimOld[0]) &&
                   (oldIJK[1] >= 0) && (oldIJK[1] < dimOld[1]) &&
                   (oldIJK[2] >= 0) && (oldIJK[2] < dimOld[2])) {
                  const int oldPointID = (oldIJK[0] + oldIJK[1]* dimOld[0]
                                          + oldIJK[2] * dimOld[0] * dimOld[1])
                                         * numberOfComponentsPerVoxel;
                  for (int m = 0; m < numberOfComponentsPerVoxel; m++) {
                     voxels[pointID + m] = oldVoxels[oldPointID + m];
                  }
               }
               else {
                  for (int m = 0; m < numberOfComponentsPerVoxel; m++) {
                     voxels[pointID + m] = 0.0;
                  }
               }
            }
         }
      }
      
      //
      // Update the voxel coloring allocation
      //
      allocateVoxelColoring();
      
      //
      // delete old voxels
      //
      delete[] oldVoxels;
      
      //
      // Update parameters file
      //
      if (paramsFile != NULL) {
         paramsFile->setParameter(ParamsFile::keyCropped, ParamsFile::keyValueYes);
         paramsFile->setParameter(ParamsFile::keyCropMinX, cropping[0]);
         paramsFile->setParameter(ParamsFile::keyCropMaxX, cropping[1]);
         paramsFile->setParameter(ParamsFile::keyCropMinY, cropping[2]);
         paramsFile->setParameter(ParamsFile::keyCropMaxY, cropping[3]);
         paramsFile->setParameter(ParamsFile::keyCropMinZ, cropping[4]);
         paramsFile->setParameter(ParamsFile::keyCropMaxZ, cropping[5]);
      }
      
      setModified();
      minMaxVoxelValuesValid = false;
      minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
   }
}

/**
 * pad segmentation volume (second/second to last slice along axis is extended).
 * In output volume first and last slices are all zeros.
 */
void 
VolumeFile::padSegmentation(const int padding[6],
                            const bool erodePaddingFlag)
{
   //
   // used for padding erosion
   //
   const int dilateIterations = 0;
   const int erodeIterations = 1;
   const int sliceStep = 5;

   //
   // Enlarge "this" volume
   //
   const int dimMod[6] = {
      -padding[0],
      dimensions[0] + padding[1],
      -padding[2],
      dimensions[1] + padding[3],
      -padding[4],
      dimensions[2] + padding[5],
   };
   resize(dimMod);
   
   //
   // Copy "this" volume
   //
   VolumeFile origVolume(*this);
   
   //
   // Do left X padding
   //
   if (padding[0] > 0) {
      origVolume.fillSegmentationCavitiesInSingleSlice(VOLUME_AXIS_X, padding[0]);
      for (int i = 1; i <= padding[0]; i++) {
         copySlice(&origVolume,
                   padding[0] + 1,
                   VOLUME_AXIS_X,
                   i);
      }
      
      if (erodePaddingFlag) {
         for (int iSlice = padding[0] - 1; iSlice > 0; iSlice -= sliceStep) {
            const int erodeExtent[6] = {
               0,
               iSlice,
               0,
               dimensions[1] - 1,
               0,
               dimensions[2] - 1,
            };
            doVolMorphOpsWithinMask(erodeExtent, dilateIterations, erodeIterations);         
         } 
      }
   }   
   
   //
   // Do right X padding
   //
   if (padding[1] > 0) {
      const int iStart = dimensions[0] - padding[1];
      origVolume.fillSegmentationCavitiesInSingleSlice(VOLUME_AXIS_X, iStart - 1);
      for (int i = iStart - 1; i < (dimensions[0] - 1); i++) {
         copySlice(&origVolume,
                   iStart - 2,
                   VOLUME_AXIS_X,
                   i);
      }
      
      if (erodePaddingFlag) {
         for (int iSlice = iStart + 1; iSlice < dimensions[0]; iSlice += sliceStep) {
            const int erodeExtent[6] = {
               iSlice,
               dimensions[0] - 1,
               0,
               dimensions[1] - 1,
               0,
               dimensions[2] - 1,
            };
            doVolMorphOpsWithinMask(erodeExtent, dilateIterations, erodeIterations);         
         } 
      }
   }
   
   //
   // Do posterior Y padding
   //
   if (padding[2] > 0) {
      origVolume.fillSegmentationCavitiesInSingleSlice(VOLUME_AXIS_Y, padding[2]);
      for (int j = 1; j <= padding[2]; j++) {
         copySlice(&origVolume,
                   padding[2] + 1,
                   VOLUME_AXIS_Y,
                   j);
      }
      
      if (erodePaddingFlag) {
         for (int iSlice = padding[2] - 1; iSlice > 0; iSlice -= sliceStep) {
            const int erodeExtent[6] = {
               0,
               dimensions[0] - 1,
               0,
               iSlice,
               0,
               dimensions[2] - 1,
            };
            doVolMorphOpsWithinMask(erodeExtent, dilateIterations, erodeIterations);         
         } 
      }
   }   
   
   //
   // Do anterior Y padding
   //
   if (padding[3] > 0) {
      const int jStart = dimensions[1] - padding[3];
      origVolume.fillSegmentationCavitiesInSingleSlice(VOLUME_AXIS_Y, jStart - 1);
      for (int j = jStart - 1; j < (dimensions[1] - 1); j++) {
         copySlice(&origVolume,
                   jStart - 2,
                   VOLUME_AXIS_Y,
                   j);
      }
  
      if (erodePaddingFlag) {
         for (int iSlice = jStart + 1; iSlice < dimensions[1]; iSlice += sliceStep) {
            const int erodeExtent[6] = {
               0,
               dimensions[0] - 1,
               iSlice,
               dimensions[1] - 1,
               0,
               dimensions[2] - 1,
            };
            doVolMorphOpsWithinMask(erodeExtent, dilateIterations, erodeIterations);         
         } 
      }
   }
   
   //
   // Do inferior Z padding
   //
   if (padding[4] > 0) {
      origVolume.fillSegmentationCavitiesInSingleSlice(VOLUME_AXIS_Z, padding[4]);
      for (int k = 1; k <= padding[4]; k++) {
         copySlice(&origVolume,
                   padding[4] + 1,
                   VOLUME_AXIS_Z,
                   k);
      }
      
      if (erodePaddingFlag) {
         for (int iSlice = padding[4] - 1; iSlice > 0; iSlice -= sliceStep) {
            const int erodeExtent[6] = {
               0,
               dimensions[0] - 1,
               0,
               dimensions[1] - 1,
               0,
               iSlice,
            };
            doVolMorphOpsWithinMask(erodeExtent, dilateIterations, erodeIterations);         
         } 
      }
   }   
   
   //
   // Do superior Z padding
   //
   if (padding[5] > 0) {
      const int kStart = dimensions[2] - padding[5];
      origVolume.fillSegmentationCavitiesInSingleSlice(VOLUME_AXIS_Z, kStart - 1);
      for (int k = kStart - 1; k < (dimensions[2] - 1); k++) {
         copySlice(&origVolume,
                   kStart - 2,
                   VOLUME_AXIS_Z,
                   k);
      }
      
      if (erodePaddingFlag) {
         for (int iSlice = kStart + 1; iSlice < dimensions[2]; iSlice += sliceStep) {
            const int erodeExtent[6] = {
               0,
               dimensions[0] - 1,
               0,
               dimensions[1] - 1,
               iSlice,
               dimensions[2] - 1,
            };
            doVolMorphOpsWithinMask(erodeExtent, dilateIterations, erodeIterations);         
         } 
      }
   }   
}      

/**
 * copy a subvolume to "this" volume.
 */
void 
VolumeFile::copySubVolume(const VolumeFile* sourceVolume,
                          const int extentIn[6],
                          const unsigned char offRgbValue[4],
                          const unsigned char onRgbValue[4],
                          VolumeModification* modifiedVoxels) throw (FileException)
{
   if (sourceVolume == NULL) {
      throw FileException("Source Volume is invalid in VolumeFile::copySubVolume()");
   }
   
   int sourceDim[3];
   sourceVolume->getDimensions(sourceDim);
   if ((dimensions[0] != sourceDim[0]) ||
       (dimensions[1] != sourceDim[1]) ||
       (dimensions[2] != sourceDim[2])) {
      throw FileException("Source Volume has different dimensions in VolumeFile::copySubVolume()");
   }
   if (numberOfComponentsPerVoxel != sourceVolume->getNumberOfComponentsPerVoxel()) {
      throw FileException("Source Volume has different number of components in VolumeFile::copySubVolume()");
   }
   
   int extent[6] = {
      extent[0] = extentIn[0],
      extent[1] = extentIn[1],
      extent[2] = extentIn[2],
      extent[3] = extentIn[3],
      extent[4] = extentIn[4],
      extent[5] = extentIn[5],
   };
   
   clampVoxelIndex(VOLUME_AXIS_X, extent[0]);
   clampVoxelIndex(VOLUME_AXIS_X, extent[1]);
   clampVoxelIndex(VOLUME_AXIS_Y, extent[2]);
   clampVoxelIndex(VOLUME_AXIS_Y, extent[3]);
   clampVoxelIndex(VOLUME_AXIS_Z, extent[4]);
   clampVoxelIndex(VOLUME_AXIS_Z, extent[5]);

   for (int i = extent[0]; i <= extent[1]; i++) {
      for (int j = extent[2]; j <= extent[3]; j++) {
         for (int k = extent[4]; k <= extent[5]; k++) {
            for (int m = 0; m < numberOfComponentsPerVoxel; m++) {
               const float v =  sourceVolume->getVoxel(i, j, k, m);
               setVoxel(i, j, k, m, v);
               if (m == 0) {
                  if (modifiedVoxels != NULL) {
                     modifiedVoxels->addVoxel(this, i, j, k);
                  }
                  if (v != 0.0) {
                     setVoxelColor(i, j, k, onRgbValue);
                  }
                  else {
                     setVoxelColor(i, j, k, offRgbValue);
                  }
               }
            }
         }
      }
   }
}                         

/**
 * copy a slice to "this" volume.  Dimensions orthogonal to axis must match.
 */
void 
VolumeFile::copySlice(const VolumeFile* sourceVolume,
                      const int sourceSliceNumber,
                      const VOLUME_AXIS axis,
                      const int destinationSliceNumber)
{
   int sourceDim[3];
   sourceVolume->getDimensions(sourceDim);
   const int numComp = getNumberOfComponentsPerVoxel();
   if (sourceVolume->getNumberOfComponentsPerVoxel() != numComp) {
      return;
   }
   
   switch (axis) {
      case VOLUME_AXIS_X:
         if ((dimensions[1] != sourceDim[1]) ||
             (dimensions[2] != sourceDim[2])) {
            return;
         }
         if ((sourceSliceNumber < 0) ||
             (sourceSliceNumber >= sourceDim[0])) {
            return;
         }
         if ((destinationSliceNumber < 0) ||
             (destinationSliceNumber >= dimensions[0])) {
            return;
         }
         for (int j = 0; j < dimensions[1]; j++) {
            for (int k = 0; k < dimensions[2]; k++) {
               for (int m = 0; m < numComp; m++) {
                  setVoxel(destinationSliceNumber, j, k, m,
                           sourceVolume->getVoxel(sourceSliceNumber, j, k, m));
               }
            }
         }
         break;
      case VOLUME_AXIS_Y:
         if ((dimensions[0] != sourceDim[0]) ||
             (dimensions[2] != sourceDim[2])) {
            return;
         }
         if ((sourceSliceNumber < 0) ||
             (sourceSliceNumber >= sourceDim[1])) {
            return;
         }
         if ((destinationSliceNumber < 0) ||
             (destinationSliceNumber >= dimensions[1])) {
            return;
         }
         for (int i = 0; i < dimensions[0]; i++) {
            for (int k = 0; k < dimensions[2]; k++) {
               for (int m = 0; m < numComp; m++) {
                  setVoxel(i, destinationSliceNumber, k, m,
                           sourceVolume->getVoxel(i, sourceSliceNumber, k, m));
               }
            }
         }
         break;
      case VOLUME_AXIS_Z:
         if ((dimensions[0] != sourceDim[0]) ||
             (dimensions[1] != sourceDim[1])) {
            return;
         }
         if ((sourceSliceNumber < 0) ||
             (sourceSliceNumber >= sourceDim[2])) {
            return;
         }
         if ((destinationSliceNumber < 0) ||
             (destinationSliceNumber >= dimensions[2])) {
            return;
         }
         for (int i = 0; i < dimensions[0]; i++) {
            for (int j = 0; j < dimensions[1]; j++) {
               for (int m = 0; m < numComp; m++) {
                  setVoxel(i, j, destinationSliceNumber, m,
                           sourceVolume->getVoxel(i, j, sourceSliceNumber, m));
               }
            }
         }
         break;
      case VOLUME_AXIS_ALL:
         std::cout << "ERROR: ALL axis not supported for copySlice()." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE:
         std::cout << "ERROR: OBLIQUE axis not supported for copySlice()." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE_X:
         std::cout << "ERROR: X OBLIQUE axis not supported for copySlice()." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE_Y:
         std::cout << "ERROR: Y OBLIQUE axis not supported for copySlice()." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE_Z:
         std::cout << "ERROR: Z OBLIQUE axis not supported for copySlice()." << std::endl;
         return;
         break;
      case VOLUME_AXIS_OBLIQUE_ALL:
         std::cout << "ERROR: ALL OBLIQUE axis not supported for copySlice()." << std::endl;
         return;
         break;
      case VOLUME_AXIS_UNKNOWN:
         std::cout << "ERROR: UNKNOWN axis not supported for copySlice()." << std::endl;
         return;
         break;
   }
   
}
                     
/**
 * convert coordinates into a voxel index (returns true if inside volume).
 * NOTE: copied from VTK's 
 *  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
 */
bool 
VolumeFile::convertCoordinatesToVoxelIJK(const float* xyz,
                                            int ijk[3]) const
{
   float offset[3];
   return convertCoordinatesToVoxelIJK(xyz, ijk, offset);
}

/**
 * convert coordinates into a voxel index (returns true if inside volume).
 * NOTE: copied from VTK's 
 *  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
 */
bool 
VolumeFile::convertCoordinatesToVoxelIJK(const float* xyz,
                                         int ijk[3],
                                         float offset[3]) const
{
   bool insideVolume = true;
   
   //
   // NOTE: Origin is at center of voxel but we need to know the coordinate
   // at the corner of the voxel
   //
   float originCorner[3];
   getOriginAtCornerOfVoxel(originCorner);
   
   //
   //  Compute the ijk location
   //
   for (int i=0; i<3; i++) {
      const float d = xyz[i] - originCorner[i];
      const float floatLoc = d / spacing[i];
      // Floor for negtive indexes.
      ijk[i] = (int) (floor(floatLoc));
      if ( ijk[i] >= 0 && ijk[i] < dimensions[i] ) {
         offset[i] = floatLoc - (float)ijk[i];
      }
      else if ( ijk[i] < 0 || ijk[i] > dimensions[i] ) {
         offset[i] = floatLoc - (float)ijk[i];
         insideVolume = false;
      } 
      else { //if ( ijk[i] == dimensions[i] )
         if (dimensions[i] == 1) {
            offset[i] = 0.0;
         }
         else {
            ijk[i] -= 1;
            offset[i] = 1.0;
         }
      }
   }

   return insideVolume;
}
                                        
/**
 * Get an "interpolated" voxel at a coordinate.
 * The coordinate is the center of the interpolated voxel.
 * Returns true if the coordinate is in the volume, else false.
 */
bool
VolumeFile::getInterpolatedVoxel(const float xyzIn[3], float& voxelValue)
{
   //
   // Because of the weighting system used, we need to offset
   // by half a voxel.  This since voxels interpolated are always
   // to the larger indices.  Without this, if the xyz was near the
   // left edge of a voxel, the voxel on the to the left would not
   // get used.
   //
   const float xyz[3] = {
      xyzIn[0] - (spacing[0] * 0.5),
      xyzIn[1] - (spacing[1] * 0.5),
      xyzIn[2] - (spacing[2] * 0.5)
   };
   
   voxelValue = 0.0;

   //
   // Get the voxel coordinates
   //
   int ijk[3];
   float pcoords[3];
   const int insideVolume = convertCoordinatesToVoxelIJK((float*)xyz, ijk, pcoords);
   if (insideVolume == false) {
      return false;
   }   

   //
   // Is this voxel along the edge of the volume
   //
   if ((ijk[0] == 0) || (ijk[0] == (dimensions[0] - 1)) ||
       (ijk[1] == 0) || (ijk[1] == (dimensions[1] - 1)) ||
       (ijk[2] == 0) || (ijk[2] == (dimensions[2] - 1))) {
      voxelValue = getVoxel(ijk);
   }
   else {
      convertCoordinatesToVoxelIJK(xyz, ijk, pcoords);
      const float r = pcoords[0];
      const float s = pcoords[1];
      const float t = pcoords[2];
      
      
      //
      // Weighting from book Visualization Toolkit, 2nd Ed, page 316
      // 
      for (int j = 0; j < 8; j++) {
         int dijk[3] = { 0, 0, 0 };
         float weight = 0.0;
         switch(j) {
            case 0:
               weight = (1.0 - r) * (1.0 - s) * (1.0 - t);
               break;
            case 1:
               weight = r * (1.0 - s) * (1.0 - t);
               dijk[0] = 1;
               break;
            case 2:
               weight = (1.0 - r) * s * (1.0 - t);
               dijk[1] = 1;
               break;
            case 3:
               weight = r * s * (1.0 - t);
               dijk[0] = 1;
               dijk[1] = 1;
               break;
            case 4:
               weight = (1.0 - r) * (1.0 - s) * t;
               dijk[2] = 1;
               break;
            case 5:
               weight = r * (1.0 - s) * t;
               dijk[0] = 1;
               dijk[2] = 1;
               break;
            case 6:
               weight = (1.0 - r) * s * t;
               dijk[1] = 1;
               dijk[2] = 1;
               break;
            case 7:
               weight = r * s * t;
               dijk[0] = 1;
               dijk[1] = 1;
               dijk[2] = 1;
               break;
         }
         
         //
         // adjust the voxel indices
         //
         int vijk[3] = { ijk[0] + dijk[0], ijk[1] + dijk[1], ijk[2] + dijk[2] };
         
         //
         // Add into the voxel
         //
         const float vv = getVoxel(vijk, 0);
         voxelValue += vv * weight;
      }
   }
   
   return true;
}

/**
 * convert to VTK structured points.
 */
vtkStructuredPoints* 
VolumeFile::convertToVtkStructuredPoints(const bool makeUnsignedCharType) const
{
   vtkStructuredPoints* sp = vtkStructuredPoints::New();
   
   sp->SetDimensions((int*)dimensions);
#ifdef HAVE_VTK5
   {
      double ds[3] = { spacing[0], spacing[1], spacing[2] };
      double dorg[3] = { origin[0], origin[1], origin[2] };
      sp->SetSpacing(ds);
      sp->SetOrigin(dorg);
   }
#else // HAVE_VTK5
   sp->SetSpacing((float*)spacing);
   sp->SetOrigin((float*)origin);
#endif // HAVE_VTK5
   
   const int numVoxels = getTotalNumberOfVoxels();
   
   vtkDataArray* scalars = NULL;
#ifdef HAVE_VTK6
   if (makeUnsignedCharType) {
      scalars = vtkUnsignedCharArray::New();
      sp->AllocateScalars(VTK_UNSIGNED_CHAR, numberOfComponentsPerVoxel);
   }
   else {
      scalars = vtkFloatArray::New();
      sp->AllocateScalars(VTK_FLOAT, numberOfComponentsPerVoxel);
   }
#else
   if (makeUnsignedCharType) {
      scalars = vtkUnsignedCharArray::New();
      sp->SetScalarTypeToUnsignedChar();
   }
   else {
      scalars = vtkFloatArray::New();
      sp->SetScalarTypeToFloat();
   }
   
   scalars->SetNumberOfComponents(numberOfComponentsPerVoxel);
#endif
   scalars->SetNumberOfTuples(numVoxels);
   float* temp = new float[numberOfComponentsPerVoxel];
   for (int i = 0; i < numVoxels; i++) {
      for (int m = 0; m < numberOfComponentsPerVoxel; m++) {
         float f = voxels[i*numberOfComponentsPerVoxel+m];
         if (makeUnsignedCharType) {
            f = std::min(255.0f, f);
            f = std::max(0.0f, f);
         }
         temp[m] = f;
      }
      scalars->InsertTuple(i, temp);
   }
   delete[] temp;
   sp->GetPointData()->SetScalars(scalars);
   scalars->Delete();
   
   return sp;
}

/**
 * convert from VTK structured points.
 */
void 
VolumeFile::convertFromVtkStructuredPoints(vtkStructuredPoints* sp)
{
   if (voxels != NULL) {
      delete[] voxels;
      voxels = NULL;
   }
   
   sp->GetDimensions(dimensions);
#ifdef HAVE_VTK5
   {
      double ds[3];
      double dorg[3];
      sp->GetSpacing(ds);
      sp->GetOrigin(dorg);
      spacing[0] = ds[0];
      spacing[1] = ds[1];
      spacing[2] = ds[2];
      const float forg[3] = { dorg[0], dorg[1], dorg[2] };
      setOrigin(forg);
   }
#else // HAVE_VTK5
   sp->GetSpacing(spacing);
   sp->GetOrigin(origin);
#endif // HAVE_VTK5
   
   vtkDataArray* scalars = sp->GetPointData()->GetScalars();
   numberOfComponentsPerVoxel = scalars->GetNumberOfComponents();
   
   const int num = getTotalNumberOfVoxels();
   voxels = new float[num];
   for (int i = 0; i < num; i++) {
      for (int j = 0; j < numberOfComponentsPerVoxel; j++) {
         const int indx = i * numberOfComponentsPerVoxel + j;
         voxels[indx] = scalars->GetComponent(i, j);
      }
   }
   allocateVoxelColoring();
   
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}      

/**
 * convert from VTK image data.
 */
void 
VolumeFile::convertFromVtkImageData(vtkImageData* sp)
{
   if (voxels != NULL) {
      delete[] voxels;
      voxels = NULL;
   }
   
   sp->GetDimensions(dimensions);
#ifdef HAVE_VTK5
   {
      double ds[3];
      double dorg[3];
      sp->GetSpacing(ds);
      sp->GetOrigin(dorg);
      spacing[0] = ds[0];
      spacing[1] = ds[1];
      spacing[2] = ds[2];
      const float forg[3] = { dorg[0], dorg[1], dorg[2] };
      setOrigin(forg);
   }
#else // HAVE_VTK5
   sp->GetSpacing(spacing);
   sp->GetOrigin(origin);
#endif // HAVE_VTK5   
   vtkDataArray* scalars = sp->GetPointData()->GetScalars();
   numberOfComponentsPerVoxel = scalars->GetNumberOfComponents();
   
   const int num = getTotalNumberOfVoxels();
   voxels = new float[num];
   for (int i = 0; i < num; i++) {
      for (int j = 0; j < numberOfComponentsPerVoxel; j++) {
         const int indx = i * numberOfComponentsPerVoxel + j;
         voxels[indx] = scalars->GetComponent(i, j);
      }
   }
   allocateVoxelColoring();
   
   setModified();
}      

/**
 * convert to VTK image data.
 */
vtkImageData* 
VolumeFile::convertToVtkImageData(const bool makeUnsignedCharType) const
{
   vtkImageData* id = vtkImageData::New();
   
   id->SetDimensions((int*)dimensions);
#ifdef HAVE_VTK5
   {
      double ds[3] = { spacing[0], spacing[1], spacing[2] };
      float forg[3];
      getOriginAtCornerOfVoxel(forg);
      double dorg[3] = { forg[0], forg[1], forg[2] };
      id->SetSpacing(ds);
      id->SetOrigin(dorg);
   }
#else // HAVE_VTK5
   id->SetSpacing((float*)spacing);
   id->SetOrigin((float*)origin);
#endif // HAVE_VTK5
   
   const int numVoxels = getTotalNumberOfVoxels();
   
   vtkDataArray* scalars = NULL;
#ifdef HAVE_VTK6
   if (makeUnsignedCharType) {
      scalars = vtkUnsignedCharArray::New();
      id->AllocateScalars(VTK_UNSIGNED_CHAR, numberOfComponentsPerVoxel);
   }
   else {
      scalars = vtkFloatArray::New();
      id->AllocateScalars(VTK_FLOAT, numberOfComponentsPerVoxel);
   }
#else
   if (makeUnsignedCharType) {
      scalars = vtkUnsignedCharArray::New();
      id->SetScalarTypeToUnsignedChar();
   }
   else {
      scalars = vtkFloatArray::New();
      id->SetScalarTypeToFloat();
   }
   
   scalars->SetNumberOfComponents(numberOfComponentsPerVoxel);
#endif
   scalars->SetNumberOfTuples(numVoxels);
   float* temp = new float[numberOfComponentsPerVoxel];
   for (int i = 0; i < numVoxels; i++) {
      for (int m = 0; m < numberOfComponentsPerVoxel; m++) {
         float f = voxels[i*numberOfComponentsPerVoxel+m];
         if (makeUnsignedCharType) {
            f = std::min(255.0f, f);
            f = std::max(0.0f, f);
         }
         temp[m] = f;
      }
      scalars->InsertTuple(i, temp);
   }
   delete[] temp;
   id->GetPointData()->SetScalars(scalars);
   scalars->Delete();
   
   return id;
}

/**
 * Apply transformation matrix to a volume.
 */
void
VolumeFile::applyTransformationMatrix(const TransformationMatrix& tmIn)
{
   //
   // This makes the transfomations function correctly
   //
   TransformationMatrix tm(tmIn);
   float trans[3];
   tm.getTranslation(trans[0], trans[1], trans[2]);
   tm.translate(0.0, 0.0, 0.0);
   tm.transpose();
   tm.translate(trans[0], trans[1], trans[2]);

   vtkTransform* transform = vtkTransform::New();
   tm.getMatrix(transform);
   applyTransformationMatrix(transform);
   transform->Delete();
}


/**
 * Apply transformation matrix to a volume.
 */
void
VolumeFile::applyTransformationMatrix(vtkTransform* transform)
{
   vtkStructuredPoints* spInput = convertToVtkStructuredPoints();
   
   vtkImageReslice* reslice = vtkImageReslice::New();   
   reslice->SetNumberOfThreads(1);
#ifdef HAVE_VTK6
   reslice->SetInputData(spInput);
#else
   reslice->SetInput(spInput);
#endif
   reslice->SetInformationInput(spInput);
   reslice->SetResliceTransform(transform);
   reslice->SetAutoCropOutput(1);
 
   switch (volumeType) {
      case VOLUME_TYPE_ANATOMY:
         reslice->SetInterpolationModeToCubic();
         break;
      case VOLUME_TYPE_FUNCTIONAL:
         reslice->SetInterpolationModeToCubic();
         break;
      case VOLUME_TYPE_PAINT:
         reslice->SetInterpolationModeToNearestNeighbor();
         break;
      case VOLUME_TYPE_PROB_ATLAS:
         reslice->SetInterpolationModeToNearestNeighbor();
         break;
      case VOLUME_TYPE_RGB:
         reslice->SetInterpolationModeToNearestNeighbor();
         break;
      case VOLUME_TYPE_ROI:
         reslice->SetInterpolationModeToNearestNeighbor();
         break;
      case VOLUME_TYPE_SEGMENTATION:
         reslice->SetInterpolationModeToNearestNeighbor();
         break;
      case VOLUME_TYPE_VECTOR:
         reslice->SetInterpolationModeToNearestNeighbor();
         break;
      case VOLUME_TYPE_UNKNOWN:
         reslice->SetInterpolationModeToCubic();
         break;
   }
   reslice->Update();

   convertFromVtkImageData(reslice->GetOutput());

   reslice->Delete();
   spInput->Delete();
   
   allocateVoxelColoring();
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 * resample the image to the specified spacing.
 */
void 
VolumeFile::resampleToSpacing(const float newSpacing[3],
                              const INTERPOLATION_TYPE interpolationType)
{  
   vtkStructuredPoints* spInput = convertToVtkStructuredPoints();

   vtkImageResample* resample = vtkImageResample::New();   
   resample->SetNumberOfThreads(1);
#ifdef HAVE_VTK6
   resample->SetInputData(spInput);
#else
   resample->SetInput(spInput);
#endif
   resample->SetAxisOutputSpacing(0, newSpacing[0]);
   resample->SetAxisOutputSpacing(1, newSpacing[1]);
   resample->SetAxisOutputSpacing(2, newSpacing[2]);
   resample->SetDimensionality(3);
   resample->SetInterpolationModeToCubic();
   switch (interpolationType) {
      case INTERPOLATION_TYPE_CUBIC:
         resample->SetInterpolationModeToCubic();
         break;
      case INTERPOLATION_TYPE_LINEAR:
         resample->SetInterpolationModeToLinear();
         break;
      case INTERPOLATION_TYPE_NEAREST_NEIGHBOR:
         resample->SetInterpolationModeToNearestNeighbor();
         break;
   }
   resample->Update();
   
   convertFromVtkImageData(resample->GetOutput());
   
   resample->Delete();
   spInput->Delete();
   
   allocateVoxelColoring();
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}      

/**
 * make all voxels in a segmentation volume 0 or 255.
 */
void 
VolumeFile::makeSegmentationZeroTwoFiftyFive()
{
   const int num = getTotalNumberOfVoxelElements();
   for (int i = 0; i < num; i++) {
      if (voxels[i] != 0.0) {
         voxels[i] = 255.0;
      }
   }
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 * Import an analyze volume.
 */
void
VolumeFile::importAnalyzeVolume(const QString& fileNameIn) throw (FileException)
{
   readFile(fileNameIn);
}

/**
 * Import a minc volume.
 */
void
VolumeFile::importMincVolume(const QString& fileName) throw (FileException)
{
#ifdef HAVE_MINC
   filename = fileName;
   
   int output_signed, is_signed, mincid, imgid, icvid;
   double valid_range[2] = {0.0,0.0};
   nc_type datatype, output_datatype=NC_FLOAT;
   int ndims, dims[MAX_VAR_DIMS], idim, jdim, dataSizeInBytes, i, slice_offset;
   long start[MAX_VAR_DIMS], count[MAX_VAR_DIMS], end[MAX_VAR_DIMS];
   long size;
   double temp;

   /* Open the file */
   mincid = miopen((char*)fileName.toAscii().constData(), NC_NOWRITE);

   /* Inquire about the image variable */
   imgid = ncvarid(mincid, MIimage);
   ncvarinq(mincid, imgid, NULL, NULL, &ndims, dims, NULL);
   miget_datatype(mincid, imgid, &datatype, &is_signed);

   /* Output is NC_FLOAT type -> signed. */ 
   output_signed = TRUE;

   /* Get output range */
   if (output_datatype == datatype) {
     miget_valid_range(mincid, imgid, valid_range);
   }
   else {
     miget_default_range(output_datatype, output_signed, valid_range);
     if ((datatype == NC_CHAR) || (datatype == NC_BYTE)) {
        valid_range[0] = 0.0;
        valid_range[1] = 255.0;
     }
   }
   if (valid_range[0] > valid_range[1]) {
      temp = valid_range[0];
      valid_range[0] = valid_range[1];
      valid_range[1] = temp;
   }

   ORIENTATION orient[3] = {
      ORIENTATION_UNKNOWN,
      ORIENTATION_UNKNOWN,
      ORIENTATION_UNKNOWN
   };
   
	/* --------------- MNI's mincstats.c --------------- */
   /* Get start, step, and dircos attributes */
   const int WORLD_NDIMS = 3;
   double dircos[WORLD_NDIMS];
   double voxdim, origin; // called step and start in minc, but start means something else here
   const char *dimensions[] = {MIxspace, MIyspace, MIzspace};
   QString dimensionNames[3] = { "zspace", "yspace", "xspace" };
   
   float mincOrigin[3] = { 0.0, 0.0, 0.0 };
   float mincVoxDim[3] = { 1.0, 1.0, 1.0 };
   for (jdim=0; jdim < WORLD_NDIMS; jdim++) {

      /* Set default values */
      voxdim = 1.0;
      origin = 0.0;
      for (idim=0; idim < WORLD_NDIMS; idim++)
         dircos[idim] = 0.0;
      dircos[jdim] = 1.0;

      /* Get the attributes */
      get_minc_attribute(mincid, (char*)dimensions[jdim], MIstart, 1, &origin);
      get_minc_attribute(mincid, (char*)dimensions[jdim], MIstep,  1, &voxdim);
      get_minc_attribute(mincid, (char*)dimensions[jdim], MIdirection_cosines, 
                         WORLD_NDIMS, dircos);
      mincOrigin[jdim] = origin;
      mincVoxDim[jdim] = voxdim;
      
      char dimName[MAX_NC_NAME];
      ncdiminq(mincid, dims[jdim], dimName, NULL);
      dimensionNames[jdim] = StringUtilities::makeLowerCase(dimName);

/*      
      if (mivar_exists(mincid, MIalignment)) {
         const int MAX_LEN_STR = 256;
         char alignChars[MAX_LEN_STR];
         int varid = ncvarid(mincid, MIalignment);
         char* alignmentStr = miattgetstr(mincid,
                                          varid,
                                          MIalignment,
                                          MAX_LEN_STR,
                                          alignChars);
         if (alignmentStr != NULL) {
            std::cout << "MINC alignment: " << alignmentStr << std::endl;
         }
      }
*/
                                       
      if (DebugControl::getDebugOn()) {
         std::cout << "----dim=" << jdim << "----" << std::endl;
         std::cout << "origin=" << (float)origin << std::endl;
         std::cout << "voxdim=" << (float)voxdim << std::endl;
         std::cout << "dircos=" << (float)dircos[0] << "," 
                   << (float)dircos[1] << ", " << (float)dircos[2] << std::endl;
         std::cout << "name  =" << dimName << std::endl;
      }
      
      orient[jdim] = ORIENTATION_UNKNOWN;
      if (dircos[0] == 1.0) {
         orient[jdim] = ORIENTATION_LEFT_TO_RIGHT;
      }
      else if (dircos[0] == -1.0) {
         orient[jdim] = ORIENTATION_RIGHT_TO_LEFT;
      }
      else if (dircos[1] == 1.0) {
         orient[jdim] = ORIENTATION_POSTERIOR_TO_ANTERIOR;
      }
      else if (dircos[1] == -1.0) {
         orient[jdim] = ORIENTATION_ANTERIOR_TO_POSTERIOR;
      }
      else if (dircos[2] == 1.0) {
         orient[jdim] = ORIENTATION_INFERIOR_TO_SUPERIOR;
      }
      else if (dircos[2] == -1.0) {
         orient[jdim] = ORIENTATION_SUPERIOR_TO_INFERIOR;
      }      
   }

	/* ----------- back to MNI's minctoraw.c ------------ */
   /* Set up image conversion */
   icvid = miicv_create();
   miicv_setint(icvid, MI_ICV_TYPE, output_datatype);
   miicv_setstr(icvid, MI_ICV_SIGN, (char*)(output_signed ? MI_SIGNED : MI_UNSIGNED));
   miicv_setdbl(icvid, MI_ICV_VALID_MIN, valid_range[0]);
   miicv_setdbl(icvid, MI_ICV_VALID_MAX, valid_range[1]);
   miicv_setint(icvid, MI_ICV_DO_NORM, TRUE);
   //miicv_setint(icvid, MI_ICV_USER_NORM, TRUE);
   miicv_attach(icvid, mincid, imgid);

   /* Set input file start, count and end vectors for reading a slice
      at a time */
   for (idim=0; idim < ndims; idim++) {
      ncdiminq(mincid, dims[idim], NULL, &end[idim]);
   }
   /* DLH Note: end[2]=xdim; end[1]=ydim; end[0]=zdim */
   miset_coords(ndims, (long) 0, start);
   miset_coords(ndims, (long) 1, count);
   dataSizeInBytes = size = nctypelen(output_datatype);
   int pixels = 1;
   for (idim=ndims-2; idim < ndims; idim++) {
      count[idim] = end[idim];
      size *= count[idim];
      pixels *= count[idim];
   }
   int num_voxels = pixels*end[0];

   if (DebugControl::getDebugOn()) {
	   std::cout << "valid range: "  << valid_range[0] << " - " << valid_range[1] << std::endl;
	   std::cout << "dims: " <<  dims[0] << " " <<  dims[1] << " " <<  dims[2] << std::endl;
	   std::cout << "dataSizeInBytes: " << dataSizeInBytes << std::endl;
	   std::cout << "size: " << (int)size << " pixels " << pixels << " voxels " << num_voxels << std::endl;
	   std::cout << "start: " << (int)start[0] << " " <<   (int)start[1] << " " <<  (int)start[2] << std::endl;
	   std::cout << "count: " << (int)count[0] << " " << (int)count[1] << " " << (int)count[2] << std::endl;
	   std::cout << "end: " << (int)end[0] << " " << (int)end[1] << " " <<  (int)end[2] << std::endl;
   }

   /* Allocate space */
   float* slicedata = new float [pixels];
   float* voxdataflat = new float [num_voxels];

   /* Loop over input slices */
   while (start[0] < end[0]) {

      /* Read in the slice */
      miicv_get(icvid, start, count, (void*)slicedata);

      /* Copy slice to volume */
      slice_offset = start[0]*pixels;
      for ( i=0 ; i < pixels ; i++ ) voxdataflat[i+slice_offset] = slicedata[i];
 			 
      /* Increment start counter */
      idim = ndims-1;
      start[idim] += count[idim];
      while ( (idim>0) && (start[idim] >= end[idim])) {
         start[idim] = 0;
         idim--;
         start[idim] += count[idim];
      }
   }       /* End loop over slices */

//   float org[3] = { 0.0, 0.0, 0.0 };
//   float space[3] = { 1.0, 1.0, 1.0 };
   
   int dimen[3] = { end[2], end[1], end[0] };

   //
   // In most cases the data is stored z, y, x which is LPI.
   // The following code handles all storage orientations
   //
   ORIENTATION initOrient[3] = { orient[0], orient[1], orient[2] };
   float initOrigin[3] = { mincOrigin[0], mincOrigin[1], mincOrigin[2] };
   float initVoxDim[3] = { mincVoxDim[0], mincVoxDim[1], mincVoxDim[2] };   
   for (int i = 0; i < 3; i++) {
      const int ii = 2 - i;
      int indx = -1;
      if (dimensionNames[i] == "zspace") {
         indx = 2;
      }
      else if (dimensionNames[i] == "yspace") {
         indx = 1;
      }
      else if (dimensionNames[i] == "xspace") {
         indx = 0;
      }
      initOrient[ii] = orient[indx];
      initOrigin[ii] = mincOrigin[indx];
      initVoxDim[ii] = mincVoxDim[indx];
   }
   
   initOrigin[0] += (initVoxDim[0] * 0.5);
   initOrigin[1] += (initVoxDim[1] * 0.5);
   initOrigin[2] += (initVoxDim[2] * 0.5);

   initialize(VOXEL_DATA_TYPE_FLOAT,
              dimen,
              initOrient, //orient,
              initOrigin, //mincOrigin,
              initVoxDim, //mincVoxDim,
              true,
              true);
   
   filename = fileName;
   
   //
   // Set the voxels
   //
   for (int i = 0; i < num_voxels; i++) {
      voxels[i] = voxdataflat[i];
   }
   
   /* Clean up */
   miclose(mincid);
   miicv_free(icvid);
   delete [] slicedata;
   delete [] voxdataflat;
   
   //
   // Is the orientation valid?  If so, the volume may still need to be made LPI
   //
   if (isValidOrientation(initOrient)) {
      const VolumeFile::ORIENTATION lpiOrientation[3] = {
                                    VolumeFile::ORIENTATION_LEFT_TO_RIGHT,
                                    VolumeFile::ORIENTATION_POSTERIOR_TO_ANTERIOR,
                                    VolumeFile::ORIENTATION_INFERIOR_TO_SUPERIOR
                                                  };
      bool makeItLPI = false;
      for (int i = 0; i < 3; i++) {
         if (initOrient[i] != lpiOrientation[i]) {
            makeItLPI = true;
            break;
         }
      }
      
      if (makeItLPI) {
         //
         // Make the volume LPI
         //
         try {
            permuteToOrientation(lpiOrientation);
         }
         catch (FileException& e) {
            clearModified();
            QString msg("Volume was read but had problems placing it in an LPI orientation.\n");
            msg.append(e.whatQString());
            throw FileException(fileName, msg);
         }
      }
   }
   
   clearModified();
   
#else // HAVE_MINC
   throw FileException(fileName, "Unable to read.  MINC volume file support not compiled.");
#endif
}

/**
 * Get a double attribute from a minc file.
 */
void 
VolumeFile::get_minc_attribute(int mincid, char *varname, char *attname, 
                               int maxvals, double vals[])
{
#ifdef HAVE_MINC
   int varid;
   int old_ncopts;
   int att_length;

   if (!mivar_exists(mincid, varname)) return;
   varid = ncvarid(mincid, varname);
   old_ncopts = ncopts; ncopts = 0;
   (void) miattget(mincid, varid, attname, NC_DOUBLE, maxvals, vals, 
                   &att_length);
   ncopts = old_ncopts;
#endif
}

/**
 * Import a vtk structured points volume.
 */
void
VolumeFile::importVtkStructuredPointsVolume(const QString& fileName) throw (FileException)
{
   vtkStructuredPointsReader* reader = vtkStructuredPointsReader::New();
   reader->SetFileName((char*)fileName.toAscii().constData());
   reader->Update();
   vtkStructuredPoints* data = reader->GetOutput();
   int dim[3];
   data->GetDimensions(dim);
   if ((dim[0] < 1) || (dim[1] < 1) || (dim[2] < 1)) {
      reader->Delete();
      throw FileException(fileName, "Error reading volume");
   }
   
   float org[3];
   float space[3];
#ifdef HAVE_VTK5
   {
      double ds[3];
      double dorg[3];
      data->GetSpacing(ds);
      data->GetOrigin(dorg);
      space[0] = ds[0];
      space[1] = ds[1];
      space[2] = ds[2];
      org[0] = dorg[0] + (space[0] * 0.5);
      org[1] = dorg[1] + (space[1] * 0.5);
      org[2] = dorg[2] + (space[2] * 0.5);
   }
#else // HAVE_VTK5
   data->GetOrigin(org);
   data->GetSpacing(space);
#endif // HAVE_VTK5

   ORIENTATION orient[3] = { 
                          ORIENTATION_LEFT_TO_RIGHT,
                          ORIENTATION_POSTERIOR_TO_ANTERIOR,
                          ORIENTATION_INFERIOR_TO_SUPERIOR
                        };
   initialize(VOXEL_DATA_TYPE_FLOAT, dim, orient, org, space);
   
   filename = fileName;
   
   convertFromVtkStructuredPoints(data);

   reader->Delete();
   clearModified();
}

/**
 * Export a vtk structured points volume.
 */
void
VolumeFile::exportVtkStructuredPointsVolume(const QString& fileName) throw (FileException)
{
   if (voxels != NULL) {
      vtkStructuredPoints* sp = convertToVtkStructuredPoints();
      vtkStructuredPointsWriter* writer = vtkStructuredPointsWriter::New();
      writer->SetFileName((char*)fileName.toAscii().constData());
#ifdef HAVE_VTK6
      writer->SetInputData(sp);
#else
      writer->SetInput(sp);
#endif
      writer->Write();
      writer->Delete();
      sp->Delete();
   }
}

/**
 * Export an analyze volume.
 */
void
VolumeFile::exportAnalyzeVolume(const QString& fileName) throw (FileException)
{
   const QString savedFileName(getFileName());
   const unsigned long isModified = getModified();
   const FILE_READ_WRITE_TYPE ft = getFileReadType();
   setFileWriteType(FILE_READ_WRITE_TYPE_ANALYZE);
   writeFile(fileName);
   setFileWriteType(ft);
   setFileName(savedFileName);
   setModifiedCounter(isModified);   
}

/**
 * Export an minc volume.
 */
void
VolumeFile::exportMincVolume(const QString& fileName) throw (FileException)
{
#ifdef HAVE_MINC
   //
   // create an image conversion  variable
   //
   int icv = miicv_create();
   if (icv == MI_ERROR) {
      throw FileException(fileName, "ERROR: Unable to create mnc file image conversion variable.");
      return;
   }

   //
   // Min/max voxels
   //
   float minVoxel, maxVoxel;
   getMinMaxVoxelValues(minVoxel, maxVoxel);
   
   //
   // set source data to float
   //
   miicv_setint(icv, MI_ICV_TYPE, NC_FLOAT);
   miicv_setint(icv, MI_ICV_DO_NORM, 1);
   miicv_setdbl(icv, MI_ICV_VALID_MIN, 0);
   miicv_setdbl(icv, MI_ICV_VALID_MAX, maxVoxel); //255);

   int cdf = nccreate(fileName.toAscii().constData(), NC_CLOBBER);

   //
   // set output dimensions of file
   //
   int dim[3];
   dim[0] = ncdimdef(cdf, MIzspace, dimensions[2]);
   dim[1] = ncdimdef(cdf, MIyspace, dimensions[1]);
   dim[2] = ncdimdef(cdf, MIxspace, dimensions[0]);
   //int img = micreate_std_variable(cdf, MIimage, NC_BYTE, 3, dim);
   const int img = micreate_std_variable(cdf, MIimage, NC_FLOAT, 3, dim);

   //
   // set output type to unsigned
   //
   miattputstr(cdf, img, MIsigntype, MI_UNSIGNED);

   //
   // set output range
   //
   float range[2] = { 0.0, maxVoxel };  //255.0 };
   ncattput(cdf, img, MIvalid_range, NC_FLOAT, 2, range);
   miattputstr(cdf, img, MIsigntype, MI_SIGNED);

   //
   // create variables for image min/max
   //
   int max = micreate_std_variable(cdf, MIimagemax, NC_FLOAT, 0, NULL);
   int min = micreate_std_variable(cdf, MIimagemin, NC_FLOAT, 0, NULL);

      float originCorner[3];
      getOriginAtCornerOfVoxel(originCorner);
      
      //
      // spacing origin and direction cosines for z-axis
      //
      int varid = micreate_std_variable(cdf, MIzspace, 
                                    NC_INT, 0, NULL);
      (void) miattputdbl(cdf, varid, MIstep, spacing[2]);
      (void) miattputdbl(cdf, varid, MIstart, originCorner[2]);
      const double zdircos[3] = { 0.0, 0.0, 1.0 };
      (void) ncattput(cdf, varid, MIdirection_cosines, NC_DOUBLE,
                      3, zdircos);

      //
      // spacing origin and direction cosines for y-axis
      //
      varid = micreate_std_variable(cdf, MIyspace, 
                                    NC_INT, 0, NULL);
      (void) miattputdbl(cdf, varid, MIstep, spacing[1]);
      (void) miattputdbl(cdf, varid, MIstart, originCorner[1]);
      const double ydircos[3] = { 0.0, 1.0, 0.0 };
      (void) ncattput(cdf, varid, MIdirection_cosines, NC_DOUBLE,
                      3, ydircos);

      //
      // spacing origin and direction cosines for x-axis
      //
      varid = micreate_std_variable(cdf, MIxspace, 
                                    NC_INT, 0, NULL);
      (void) miattputdbl(cdf, varid, MIstep, spacing[0]);
      (void) miattputdbl(cdf, varid, MIstart, originCorner[0]);
      const double xdircos[3] = { 1.0, 0.0, 0.0 };
      (void) ncattput(cdf, varid, MIdirection_cosines, NC_DOUBLE,
                      3, xdircos);
                      
   //
   // end definition mode
   //
   ncendef(cdf);

   //
   // attach image variable
   //
   miicv_attach(icv, cdf, img);

   //
   // write the image max and min
   //
   double image_maximum = maxVoxel;
   double image_minimum = minVoxel;
   ncvarput1(cdf, max, NULL, &image_maximum);
   ncvarput1(cdf, min, NULL, &image_minimum);

   //
   // write the image
   //
   long start[3] = { 0, 0, 0 };
   long count[3] = { dimensions[2], dimensions[1], dimensions[0] };
   miicv_put(icv, start, count, voxels);

   //
   // close file and free icv
   //
   ncclose(cdf);
   miicv_free(icv);

#else // HAVE_MINC
   throw FileException(fileName, "Unable to read.  MINC volume file support not compiled.");
#endif
}

/**
 * Initialize the sub volumes.
 */
void
VolumeFile::initializeSubVolumes(const int num)
{
   numberOfSubVolumes = num;
   scaleSlope.resize(numberOfSubVolumes);
   scaleOffset.resize(numberOfSubVolumes);
   subVolumeNames.resize(numberOfSubVolumes);
   for (int i = 0; i < numberOfSubVolumes; i++) {
      scaleSlope[i]  = 1.0;
      scaleOffset[i] = 0.0;
      std::ostringstream str;
      if (filename.isEmpty() == false) {
         str << FileUtilities::basename(filename).toAscii().constData();  // << " ";
      }
      //str << "Sub Volume ";
      //str << i;
      subVolumeNames[i] = str.str().c_str();
   }
}

/// see if a file is a NIFTI volume file
bool 
VolumeFile::isFileNifti(const QString& name)
{
   bool valid = StringUtilities::endsWith(name, SpecFile::getNiftiVolumeFileExtension()) ||
                StringUtilities::endsWith(name, SpecFile::getNiftiGzipVolumeFileExtension());   
   return valid;
}
      
/**
 * read a raw volume file that has no header.
 */
void 
VolumeFile::readFileVolumeRaw(const QString& name,
                              const int subVolumeNumber,
                              const VOXEL_DATA_TYPE vdt, 
                              const int dim[3],
                              const ORIENTATION orient[3],
                              const float org[3], 
                              const float space[3],
                              const bool byteSwapIn) throw (FileException)
{
   initialize(vdt, dim, orient, org, space, true, false);
   filename = name;
   dataFileName = name;
   gzFile dataFile = gzopen(name.toAscii().constData(), "rb");
   if (dataFile == NULL) {
      throw FileException(name, "Unable to open.");
   }
   readVolumeFileDataSubVolume(byteSwapIn,
                               1.0,
                               0.0,
                               0,
                               subVolumeNumber,
                               dataFile);
   gzclose(dataFile);
}      

/**
 * Scale Anatomy and Segmentation Volumes to be 0 to 255 
 * BUT ONLY if the maximum value is 1.0 or less.
 */
void
VolumeFile::scaleAnatomyAndSegmentationVolumesTo255()
{
   switch (volumeType) {
      case VOLUME_TYPE_FUNCTIONAL:
      case VOLUME_TYPE_PAINT:
      case VOLUME_TYPE_PROB_ATLAS:
      case VOLUME_TYPE_RGB:
      case VOLUME_TYPE_ROI:
         break;
      case VOLUME_TYPE_SEGMENTATION:
      case VOLUME_TYPE_ANATOMY:
         {
            float minValues, maxValues;
            getMinMaxVoxelValues(minValues, maxValues);
            if (maxValues <= 1.0) {
               scaleVoxelValues(255.0, 0.0, 255.0);
            }
         }
         break;
      case VOLUME_TYPE_VECTOR:
         break;
      case VOLUME_TYPE_UNKNOWN:
         break;
   }
}

/**
 * Permute (reorient) the volume to the specified orientation.
 */
void
VolumeFile::permuteToOrientation(const ORIENTATION newOrientation[3])
                                                            throw (FileException)
{
   if (isValidOrientation(orientation) == false) {
      throw FileException("Input volume has invalid orientation.");
   }
   if (isValidOrientation(newOrientation) == false) {
      throw FileException("New orientation is invalid.");
   }

   VOLUME_AXIS axis = VOLUME_AXIS_X;
   while(axis != VOLUME_AXIS_ALL) {
      //
      // Is the orientation correct for this axis
      //
      if (orientation[axis] == newOrientation[axis]) {
         //
         // Move on to next axis
         //
         switch (axis) {
            case VOLUME_AXIS_X:
               axis = VOLUME_AXIS_Y;
               break;
            case VOLUME_AXIS_Y:
               axis = VOLUME_AXIS_Z;
               break;
            case VOLUME_AXIS_Z:
               axis = VOLUME_AXIS_ALL;
               break;
            case VOLUME_AXIS_ALL:
            case VOLUME_AXIS_OBLIQUE:
            case VOLUME_AXIS_OBLIQUE_X:
            case VOLUME_AXIS_OBLIQUE_Y:
            case VOLUME_AXIS_OBLIQUE_Z:
            case VOLUME_AXIS_OBLIQUE_ALL:
            case VOLUME_AXIS_UNKNOWN:
               throw FileException("PROGRAM ERROR: Invalid axis for incrementing.");
               break;
         }
      }
      //
      // Is the orientation opposite for this axis (ie: "left" and need "right")
      //
      else if (orientation[axis] == getInverseOrientation(newOrientation[axis])) {
         //
         // flip about the axis
         //
         flip(axis);
      }
      else {
         //
         // Find the axis that contains the desired orientation or its opposite
         //
         VOLUME_AXIS foundInAxis = VOLUME_AXIS_ALL;
         for (int i = (axis + 1); i < 3; i++) {
            if ((orientation[i] == newOrientation[axis]) ||
                (orientation[i] == getInverseOrientation(newOrientation[axis]))) {
               foundInAxis = static_cast<VOLUME_AXIS>(i);
               break;
            }
         }
         if (foundInAxis == VOLUME_AXIS_ALL) {
            throw FileException("PROGRAM ERROR: Unable to find axis for rotating.");
         }
         
         //
         // Determine which axis to rotate about
         //
         VOLUME_AXIS rotateAxis = VOLUME_AXIS_ALL;
         if (axis == VOLUME_AXIS_X) {
            if (foundInAxis == VOLUME_AXIS_Y) {
               rotateAxis = VOLUME_AXIS_Z;
            }
            else if (foundInAxis == VOLUME_AXIS_Z) {
               rotateAxis = VOLUME_AXIS_Y;
            }
         }
         else if (axis == VOLUME_AXIS_Y) {
            if (foundInAxis == VOLUME_AXIS_Z) {
               rotateAxis = VOLUME_AXIS_X;
            }
         }
         
         switch (rotateAxis) {
            case VOLUME_AXIS_X:
            case VOLUME_AXIS_Y:
            case VOLUME_AXIS_Z:
               rotate(rotateAxis);
               break;
            case VOLUME_AXIS_ALL:
            case VOLUME_AXIS_OBLIQUE:
            case VOLUME_AXIS_OBLIQUE_X:
            case VOLUME_AXIS_OBLIQUE_Y:
            case VOLUME_AXIS_OBLIQUE_Z:
            case VOLUME_AXIS_OBLIQUE_ALL:
            case VOLUME_AXIS_UNKNOWN:
               throw FileException("PROGRAM ERROR: Invalid axis for rotating.");
               break;
         }
      }
   }   
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 * Flood fill starting at a voxel or remove a connected set of voxels.
 */
void
VolumeFile::floodFillAndRemoveConnected(const SEGMENTATION_OPERATION operation,
                      const VOLUME_AXIS axis,
                      const int ijkStart[3],
                      const float newVoxelValue,
                      const unsigned char newRgbValue[4],
                      VolumeModification* modifiedVoxels)
{
   //
   // Dimensions of volume
   //
   int dim[3];
   getDimensions(dim);
   
   //
   // Flag for operation being performed
   //
   bool fillingFlag = false;
   bool threeDimensionalFlag = false;
   switch (operation) {
      case SEGMENTATION_OPERATION_DILATE:
         break;
      case SEGMENTATION_OPERATION_ERODE:
         break;
      case SEGMENTATION_OPERATION_FLOOD_FILL_2D:
         fillingFlag = true;
         break;
      case SEGMENTATION_OPERATION_FLOOD_FILL_3D:
         fillingFlag = true;
         threeDimensionalFlag = true;
         break;
      case SEGMENTATION_OPERATION_REMOVE_CONNECTED_2D:
         break;
      case SEGMENTATION_OPERATION_REMOVE_CONNECTED_3D:
         threeDimensionalFlag = true;
         break;
      case SEGMENTATION_OPERATION_TOGGLE_ON:
         break;
      case SEGMENTATION_OPERATION_TOGGLE_OFF:
         break;
   }
   
   //
   // Initialize to the staring voxel
   //
   std::stack<VoxelIJK> st;
   st.push(VoxelIJK(ijkStart));
    
   //
   // While there are voxels to process
   //
   while (st.empty() == false) {
      //
      // Get the next voxel to process
      //
      const VoxelIJK v = st.top();
      st.pop();
      int i, j, k;
      v.getIJK(i, j, k);

      //
      // If the voxel has valid indices
      //
      if ((i >= 0) && (i < dim[0]) &&
          (j >= 0) && (j < dim[1]) &&
          (k >= 0) && (k < dim[2])) {
         const int ijk[3] = { i, j, k };
         float currentValue = getVoxel(ijk);
         
         //
         // See if voxel has proper value for operation
         //
         bool matchingVoxel = false;
         if (fillingFlag) {
            matchingVoxel = (currentValue == 0.0);
         }
         else {
            matchingVoxel = (currentValue != 0.0);
         }
         
         //
         // If the voxel should be modified
         //
         if (matchingVoxel) {
            //
            // Update the voxels value
            //
            if (modifiedVoxels != NULL) {
               modifiedVoxels->addVoxel(this, ijk);
            }
            setVoxel(ijk, 0, newVoxelValue);
            setVoxelColor(ijk, newRgbValue);
            
            //
            // Determine neighboring voxels
            //
            int iDelta = 0;
            int jDelta = 0;
            int kDelta = 0;
            switch (axis) {
               case VOLUME_AXIS_X:
                  if (threeDimensionalFlag) {
                     iDelta = 1;
                  }
                  else {
                     iDelta = 0;
                  }
                  jDelta = 1;
                  kDelta = 1;
                  break;
               case VOLUME_AXIS_Y:
                  iDelta = 1;
                  if (threeDimensionalFlag) {
                     jDelta = 1;
                  }
                  else {
                     jDelta = 0;
                  }
                  kDelta = 1;
                  break;
               case VOLUME_AXIS_Z:
                  iDelta = 1;
                  jDelta = 1;
                  if (threeDimensionalFlag) {
                     kDelta = 1;
                  }
                  else {
                     kDelta = 0;
                  }
                  break;
               case VOLUME_AXIS_ALL:
               case VOLUME_AXIS_OBLIQUE:
               case VOLUME_AXIS_OBLIQUE_X:
               case VOLUME_AXIS_OBLIQUE_Y:
               case VOLUME_AXIS_OBLIQUE_Z:
               case VOLUME_AXIS_OBLIQUE_ALL:
               case VOLUME_AXIS_UNKNOWN:
                  break;
            }
            
            //
            // Add neighboring voxels for search
            //
            if (iDelta != 0) {
               st.push(VoxelIJK(i - iDelta, j, k));
               st.push(VoxelIJK(i + iDelta, j, k));
            }
            if (jDelta != 0) {
               st.push(VoxelIJK(i, j - jDelta, k));
               st.push(VoxelIJK(i, j + jDelta, k));
            }
            if (kDelta != 0) {
               st.push(VoxelIJK(i, j, k - kDelta));
               st.push(VoxelIJK(i, j, k + kDelta));
            }
         }
      }
   }
}

/**
 * perform operation on segmentation volume.
 */
void 
VolumeFile::performSegmentationOperation(const SEGMENTATION_OPERATION operation,
                                         const VOLUME_AXIS axis,
                                         const bool threeDimensionalFlag,
                                         const int ijkMinIn[3],
                                         const int ijkMaxIn[3],
                                         const float voxelValue,
                                         const unsigned char rgbValue[4],
                                         VolumeModification* modifiedVoxels)
{
   //
   // Limit to valid indices
   //
   int ijkMin[3] = {
      ijkMinIn[0], ijkMinIn[1], ijkMinIn[2]
   };
   clampVoxelIndex(VOLUME_AXIS_X, ijkMin[0]);
   clampVoxelIndex(VOLUME_AXIS_Y, ijkMin[1]);
   clampVoxelIndex(VOLUME_AXIS_Z, ijkMin[2]);
   int ijkMax[3] = {
      ijkMaxIn[0], ijkMaxIn[1], ijkMaxIn[2]
   };
   clampVoxelIndex(VOLUME_AXIS_X, ijkMax[0]);
   clampVoxelIndex(VOLUME_AXIS_Y, ijkMax[1]);
   clampVoxelIndex(VOLUME_AXIS_Z, ijkMax[2]);
   
   bool allocatedModifiedVoxelsInHere = false;
   const int STRUCTURE_ELEMENT_SIZE = 1;  // 1 =>  3 x 3
   
   switch (operation) {
      case SEGMENTATION_OPERATION_DILATE:
      case SEGMENTATION_OPERATION_ERODE:
         {
            //
            // Need to use modified voxels since we need to check voxels before setting them
            //
            if (modifiedVoxels == NULL) {
               modifiedVoxels = new VolumeModification;
               allocatedModifiedVoxelsInHere = true;
            }
            
            //
            // Check each voxel in the desired region
            //
            for (int i = ijkMin[0]; i <= ijkMax[0]; i++) {
               for (int j = ijkMin[1]; j <= ijkMax[1]; j++) {
                  for (int k = ijkMin[2]; k <= ijkMax[2]; k++) {
                  
                     //
                     // Make sure the voxel is in the volume since we
                     // may be near the edge of the volume
                     //
                     const int ijk[3] = { i, j, k };
                     if (getVoxelIndexValid(ijk)) {
                        
                        //
                        // Get the value of the voxel
                        //
                        float value = getVoxel(ijk);
                        
                        bool voxelMatches = false;
                        
                        //
                        // If eroding, look for "ON" voxels
                        //
                        if (operation == SEGMENTATION_OPERATION_ERODE) {
                           if (value != 0.0) {
                              voxelMatches = true;
                           }
                        }
                        
                        //
                        // If dilating, look for "OFF" boxels
                        //
                        if (operation == SEGMENTATION_OPERATION_DILATE) {
                           if (value == 0.0) {
                              voxelMatches = true;
                           }
                        }
                        
                        //
                        // Should we continue processing this voxel
                        //
                        if (voxelMatches) {
                           //
                           // Create Structuring Element based upon the axis
                           //
                           int iMin = ijk[0];
                           int iMax = ijk[0];
                           int jMin = ijk[1];
                           int jMax = ijk[1];
                           int kMin = ijk[2];
                           int kMax = ijk[2];
                           switch (axis) {
                              case VOLUME_AXIS_X:
                                 if (threeDimensionalFlag) {
                                    iMin -= STRUCTURE_ELEMENT_SIZE;
                                    iMax += STRUCTURE_ELEMENT_SIZE;
                                 }
                                 jMin -= STRUCTURE_ELEMENT_SIZE;
                                 jMax += STRUCTURE_ELEMENT_SIZE;
                                 kMin -= STRUCTURE_ELEMENT_SIZE;
                                 kMax += STRUCTURE_ELEMENT_SIZE;
                                 break;
                              case VOLUME_AXIS_Y:                              
                                 iMin -= STRUCTURE_ELEMENT_SIZE;
                                 iMax += STRUCTURE_ELEMENT_SIZE;
                                 if (threeDimensionalFlag) {
                                    jMin -= STRUCTURE_ELEMENT_SIZE;
                                    jMax += STRUCTURE_ELEMENT_SIZE;
                                 }
                                 kMin -= STRUCTURE_ELEMENT_SIZE;
                                 kMax += STRUCTURE_ELEMENT_SIZE;
                                 break;
                              case VOLUME_AXIS_Z:
                                 iMin -= STRUCTURE_ELEMENT_SIZE;
                                 iMax += STRUCTURE_ELEMENT_SIZE;
                                 jMin -= STRUCTURE_ELEMENT_SIZE;
                                 jMax += STRUCTURE_ELEMENT_SIZE;
                                 if (threeDimensionalFlag) {
                                    kMin -= STRUCTURE_ELEMENT_SIZE;
                                    kMax += STRUCTURE_ELEMENT_SIZE;
                                 }
                                 break;
                              case VOLUME_AXIS_ALL:
                              case VOLUME_AXIS_OBLIQUE:
                              case VOLUME_AXIS_OBLIQUE_X:
                              case VOLUME_AXIS_OBLIQUE_Y:
                              case VOLUME_AXIS_OBLIQUE_Z:
                              case VOLUME_AXIS_OBLIQUE_ALL:
                              case VOLUME_AXIS_UNKNOWN:                           
                                 break;
                           }
                           
                           //
                           // Check all voxels "under" the structuring element
                           //
                           bool setVoxelFlag = false;
                           for (int ii = iMin; ii <= iMax; ii++) {
                              for (int jj = jMin; jj <= jMax; jj++) {
                                 for (int kk = kMin; kk <= kMax; kk++) {
                                    //
                                    // Ignore the voxel under the center of the 
                                    // structuring element
                                    //
                                    if ((ii != i) || (jj != j) || (kk != k)) {
                                       //
                                       // Make sure voxel is valid since structuring element
                                       // may exceed bounds of the volume
                                       //
                                       const int iijjkk[3] = { ii, jj, kk };
                                       if (getVoxelIndexValid(iijjkk)) {
                                          float value = getVoxel(iijjkk);
                                          
                                          //
                                          // If dilating, look for voxels that are "ON"
                                          // under the structuring element
                                          //
                                          if (operation == SEGMENTATION_OPERATION_DILATE) {
                                             if (value != 0.0) {
                                                setVoxelFlag = true;
                                                break;
                                             }
                                          }
                                          
                                          //
                                          // If eroding look for voxels that are "OFF"
                                          // under the structuring element
                                          //
                                          if (operation == SEGMENTATION_OPERATION_ERODE) {
                                             if (value == 0.0) {
                                                setVoxelFlag = true;
                                                break;
                                             }
                                          }
                                       }
                                    }
                                 }
                                 if (setVoxelFlag) {
                                    break;
                                 }
                              }
                              if (setVoxelFlag) {
                                 break;
                              }
                           }
                           
                           if (setVoxelFlag) {
                              //
                              // For now, just note which voxels need to be set since 
                              // we do not want to modify the volume until after all voxels
                              // under structuring element have been checked.
                              //
                              modifiedVoxels->addVoxel(this, ijk);
                           }
                        }
                     }
                  }
               }
            }
            
            //
            // Now that all of the voxels have been examined
            // set the voxels that need to be changed.
            //
            const int num = modifiedVoxels->getNumberOfVoxels();
            for (int m = 0; m < num; m++) {
               const VoxelModified* vm = modifiedVoxels->getModifiedVoxel(m);
               const int* ijk = vm->getIJK();
               setVoxel(ijk, 0, voxelValue);
               setVoxelColor(ijk, rgbValue);
            }
         }
         break;
      case SEGMENTATION_OPERATION_FLOOD_FILL_2D:
      case SEGMENTATION_OPERATION_FLOOD_FILL_3D:
      case SEGMENTATION_OPERATION_REMOVE_CONNECTED_2D:
      case SEGMENTATION_OPERATION_REMOVE_CONNECTED_3D:
         floodFillAndRemoveConnected(operation,
                                     axis,
                                     ijkMin,
                                     voxelValue,
                                     rgbValue,
                                     modifiedVoxels);
         break;
      case SEGMENTATION_OPERATION_TOGGLE_ON:
      case SEGMENTATION_OPERATION_TOGGLE_OFF:
         for (int i = ijkMin[0]; i <= ijkMax[0]; i++) {
            for (int j = ijkMin[1]; j <= ijkMax[1]; j++) {
               for (int k = ijkMin[2]; k <= ijkMax[2]; k++) {
                  const int ijk[3] = { i, j, k };
                  if (getVoxelIndexValid(ijk)) {
                     if (modifiedVoxels != NULL) {
                        modifiedVoxels->addVoxel(this, ijk);
                     }
                     setVoxel(ijk, 0, voxelValue);
                     setVoxelColor(ijk, rgbValue);
                  }
               }
            }
         }
         break;
   }
   
   if (allocatedModifiedVoxelsInHere) {
      delete modifiedVoxels;
   }
}

/**
 * Undo a volume modification
 */
void
VolumeFile::undoModification(const VolumeModification* modifiedVoxels)
{
   const int num = modifiedVoxels->getNumberOfVoxels();
   for (int i = 0; i < num; i++) {
      const VoxelModified* vm = modifiedVoxels->getModifiedVoxel(i);
      const int* ijk = vm->getIJK();
      if (getVoxelIndexValid(ijk)) {
         setVoxel(ijk, 0, vm->getVoxelValue());
         setVoxelColor(ijk, vm->getVoxelColor());
      }
   }
}

/**
 * change a range of values to zero.
 */
void 
VolumeFile::setRangeOfValuesToZero(const VolumeFile* inputVolume,
                                   VolumeFile* outputVolume,
                                   const float minValue,
                                   const float maxValue,
                                   const bool inclusiveRangeFlag)
                                                    throw (FileException)
{
   int dimA[3], dimOut[3];
   inputVolume->getDimensions(dimA);
   outputVolume->getDimensions(dimOut);
   for (int d = 0; d < 3; d++) {
      if (dimA[d] != dimOut[d]) {
         throw FileException("Input and Output Volumes have different dimensions.");
      }
   }
   
   const int num = inputVolume->getTotalNumberOfVoxels();
   for (int i = 0; i < num; i++) {
      float value = inputVolume->voxels[i];
      
      if (inclusiveRangeFlag) {
         if ((value >= minValue) && (value <= maxValue)) {
            value = 0.0;
         }
      }
      else {
         if ((value > minValue) && (value < maxValue)) {
            value = 0.0;
         }
      }
      outputVolume->voxels[i] = value;
   }
}
                                  
/**
 * create a segmentation volume mask.  Output volume must exist.
 */
void 
VolumeFile::createSegmentationMask(const QString& outputFileName,
                                   const std::vector<QString>& inputFileNames,
                                   const int numberOfDilationIterations) throw (FileException)
{
   //
   // Verify inputs
   //
   if (outputFileName.isEmpty()) {
      throw FileException("Output file name is empty.");
   }
   const int numInputVolumes = static_cast<int>(inputFileNames.size());
   if (numInputVolumes <= 0) {
      throw FileException("There are no input volume file names.");
   }
   if (numberOfDilationIterations < 0) {
      throw FileException("Number of dilation iterations is less than zero.");
   }
   
   //
   // Comment for mask volume
   //
   QString comment("Mask volume with " 
                   + QString::number(numberOfDilationIterations)
                   + " dilation iterations created from: \n");
                   
   //
   // The mask volume
   //
   VolumeFile maskVolume;
   try {
      maskVolume.readFile(outputFileName);
   }
   catch (FileException& e) {
      const QString msg("ERROR: unable to open output volume file which should already exist,\n "
                        + e.whatQString());
      throw FileException(msg);
   }
   maskVolume.setAllVoxels(0.0);
   
   //
   // Read the volumes
   //
   for (int inputIndex = 0; inputIndex < numInputVolumes; inputIndex++) {
      QString errorMessage;
      
      std::vector<VolumeFile*> volumesRead;
      try {
         //
         // Read all sub volumes in the volume file
         //
         readFile(inputFileNames[inputIndex],
                  VolumeFile::VOLUME_READ_SELECTION_ALL,
                  volumesRead,
                  false);
          
         //
         // Add file name to mask file comment
         //
         comment += (FileUtilities::basename(inputFileNames[inputIndex]) + "\n");
         
         //
         // Process all subvolumes that were read
         //
         const int numVolumesRead = static_cast<int>(volumesRead.size());
         for (int subVolIndex = 0; subVolIndex < numVolumesRead; subVolIndex++) {
            //
            // Test each voxel in the mask volume to see if it is inside an input volume voxel
            //
            const VolumeFile* inputVolume = volumesRead[subVolIndex];
            int maskDim[3];
            maskVolume.getDimensions(maskDim);
            for (int i = 0; i < maskDim[0]; i++) {
               for (int j = 0; j < maskDim[1]; j++) {
                  for (int k = 0; k < maskDim[2]; k++) {
                     //
                     // Get coordinate of mask volume voxel
                     //
                     float xyz[3];
                     maskVolume.getVoxelCoordinate(i, j, k, xyz);
                     
                     //
                     // Is coordinate in an input volume voxel
                     //
                     int ijk[3];
                     if (inputVolume->convertCoordinatesToVoxelIJK(xyz, ijk)) {
                        //
                        // If voxel in input volume is on, turn on corresponding mask voxel
                        //
                        const float voxelValue = inputVolume->getVoxel(ijk, 0);
                        if (voxelValue != 0) {
                           maskVolume.setVoxel(i, j, k, 0, voxelValue);
                        }
                     }
                  }
               }
            }
         }
      }
      catch (FileException& e) {
         //
         // Save the exception message so that volumes read can be deleted before throwing
         //
         errorMessage = e.whatQString();
      }
      
      //
      // Delete volumes that were read
      //
      for (unsigned int i = 0; i < volumesRead.size(); i++) {
         if (volumesRead[i] != NULL) {
            delete volumesRead[i];
            volumesRead[i] = NULL;
         }
         volumesRead.clear();
      }
      
      //
      // If there was an error, abort
      //
      if (errorMessage.isEmpty() == false) {
         throw FileException(errorMessage);
      }
   }
   
   //
   // Make all non-zero voxels 255
   //
   maskVolume.makeSegmentationZeroTwoFiftyFive();
   
   //
   // Dilate the mask volume
   //
   if (numberOfDilationIterations > 0) {
      maskVolume.doVolMorphOps(numberOfDilationIterations, 0);
   }
   
   
   //
   // Write the mask volume
   //
   maskVolume.setFileComment(comment);
   maskVolume.setDescriptiveLabel("Mask");
   maskVolume.writeFile(outputFileName);
}
                                         
/**
 * perform unary operations on a volume.
 */
void 
VolumeFile::performUnaryOperation(const UNARY_OPERATION operation,
                                  const VolumeFile* inputVolume,
                                  VolumeFile* outputVolume,
                                  const float scalar)
                                              throw (FileException)
{
   int dimA[3], dimOut[3];
   inputVolume->getDimensions(dimA);
   outputVolume->getDimensions(dimOut);
   for (int d = 0; d < 3; d++) {
      if (dimA[d] != dimOut[d]) {
         throw FileException("Input and Output Volumes have different dimensions.");
      }
   }
   
   for (int i = 0; i < dimA[0]; i++) {
      for (int j = 0; j < dimA[1]; j++) {
         for (int k = 0; k < dimA[2]; k++) {
            float value = 0.0;
            if (inputVolume->getVoxelAllComponents(i, j, k, &value)) {
               switch (operation) {
                  case UNARY_OPERATION_ADD:
                     value += scalar;
                     break;
                  case UNARY_OPERATION_ABS_VALUE:
                     if (value < 0.0) {
                        value = -value;
                     }
                     break;
                  case UNARY_OPERATION_CEILING:
                     value = std::min(value, scalar);
                     break;
                  case UNARY_OPERATION_FLOOR:
                     value = std::max(value, scalar);
                     break;
                  case UNARY_OPERATION_MULTIPLY:
                     value *= scalar;
                     break;
                  case UNARY_OPERATION_FIX_NOT_A_NUMBER:
                     if (MathUtilities::isNaN(value)) {
                        value = 0.0;
                     }
                     break;
                  case UNARY_OPERATION_SQUARE_ROOT:
                     if (value > 0.0) {
                        value = std::sqrt(value);
                     }
                     break;
                  case UNARY_OPERATION_SUBTRACT_FROM_ONE:
                     value = 1.0 - value;
                     break;
                  case UNARY_OPERATION_LOG2:  // use scalar as base
                     value = MathUtilities::log(scalar, value);
                     break;
               }
               
               outputVolume->setVoxel(i, j, k, 0, value);
            }
         }
      }
   }
}

/**
 * perform binary operations on a volume.
 */
void 
VolumeFile::performMathematicalOperation(const VOLUME_MATH_OPERATION operation,
                                   const VolumeFile* inputVolumeA,
                                   const VolumeFile* inputVolumeB,
                                   const VolumeFile* inputVolumeC,
                                   VolumeFile* outputVolume)
                                              throw (FileException)
{
   int dimA[3], dimB[3], dimOut[3];
   inputVolumeA->getDimensions(dimA);
   inputVolumeB->getDimensions(dimB);
   outputVolume->getDimensions(dimOut);
   for (int d = 0; d < 3; d++) {
      if ((dimA[d] != dimB[d]) ||
          (dimA[d] != dimOut[d]) ||
          (dimB[d] != dimOut[d])) {
         throw FileException("Input and Output Volumes have different dimensions.");
      }
   }
   
   //
   // For paint volume, handle paint names
   //
   const int paintVolumeBQuestionIndex = inputVolumeB->getRegionIndexFromName("???");
   std::vector<int> paintNamesVolumeA;
   std::vector<int> paintNamesVolumeB;
   if (operation == VOLUME_MATH_OPERATION_COMBINE_PAINT) {
      const int numA = inputVolumeA->getNumberOfRegionNames();
      if (numA <= 0) {
         throw FileException("There are no paint regions in the first volume.");
      }
      paintNamesVolumeA.resize(numA, -1);
      
      const int numB = inputVolumeB->getNumberOfRegionNames();
      if (numB <= 0) {
         throw FileException("There are no paint regions in the second volume.");
      }
      paintNamesVolumeB.resize(numB, -1);
      
      //outputVolume->regionNames.clear();
   }

   //
   // Special volume for paint name of output volume since the output volume may be
   // the same as one of the input volumes.
   //
   VolumeFile paintNameVolume;
   
   for (int i = 0; i < dimA[0]; i++) {
      for (int j = 0; j < dimA[1]; j++) {
         for (int k = 0; k < dimA[2]; k++) {
            float valueA = 0.0, valueB = 0.0, valueC = 0.0;
            if (inputVolumeA->getVoxelAllComponents(i, j, k, &valueA)) {
               if (inputVolumeB->getVoxelAllComponents(i, j, k, &valueB)) {
                  if (inputVolumeC != NULL) {
                     inputVolumeC->getVoxelAllComponents(i, j, k, &valueC);
                  }
                  float result = 0.0;
                  
                  switch (operation) {
                     case VOLUME_MATH_OPERATION_ADD:
                        result = valueA + valueB;
                        break;
                     case VOLUME_MATH_OPERATION_AND:
                        if ((valueA > 0.0) && (valueB > 0.0)) {
                           result = 255.0;
                        }
                        break;
                     case VOLUME_MATH_OPERATION_NAND:
                        if (! ((valueA > 0.0) && (valueB > 0.0))) {
                           result = 255.0;
                        }
                        break;
                     case VOLUME_MATH_OPERATION_SUBTRACT:
                        result = valueA - valueB;
                        break;
                     case VOLUME_MATH_OPERATION_MULTIPLY:
                        result = valueA * valueB;
                        break;
                     case VOLUME_MATH_OPERATION_DIVIDE:
                        if (valueB != 0.0) {
                           result = valueA / valueB;
                        }
                        else {
                           result = valueA;
                        }
                        break;
                     case VOLUME_MATH_OPERATION_OR:
                        if ((valueA > 0.0) || (valueB > 0.0)) {
                           result = 255.0;
                        }
                        break;
                     case VOLUME_MATH_OPERATION_NOR:
                        if ((valueA == 0.0) && (valueB == 0.0)) {
                           result = 255.0;
                        }
                        break;
                     case VOLUME_MATH_OPERATION_SUBTRACT_POSITIVE:
                        result = valueA - valueB;
                        result = std::max(result, 0.0f);
                        break;
                     case VOLUME_MATH_OPERATION_MAX:
                        result = std::max(valueA, valueB);
                        break;
                     case VOLUME_MATH_OPERATION_DIFFRATIO:
                        {
                           const float denom = valueA + valueB;
                           if (valueC == 255.0) {
                              result = 1.0;
                           }
                           else if (denom == 0.0) {
                              result = -1.0;
                           }
                           else {
                              result = (valueA - valueB) / denom;
                           }
                        }
                        break;
                     case VOLUME_MATH_OPERATION_SQRT:
                        result = valueA * valueB;
                        if (result > 0.0) {
                           result = sqrt(result);
                        }
                        break;
                     case VOLUME_MATH_OPERATION_COMBINE_PAINT:
                        {
                           const int valueBInt = static_cast<int>(valueB);
                           if (valueBInt != paintVolumeBQuestionIndex) {
                              if (paintNamesVolumeB[valueBInt] < 0) {
                                 paintNamesVolumeB[valueBInt] = 
                                    paintNameVolume.addRegionName(inputVolumeB->getRegionNameFromIndex(valueBInt));
                              }
                              result = paintNamesVolumeB[valueBInt];
                           }
                           else {
                              const int valueAInt = static_cast<int>(valueA);
                              if (paintNamesVolumeA[valueAInt] < 0) {
                                 paintNamesVolumeA[valueAInt] = 
                                    paintNameVolume.addRegionName(inputVolumeA->getRegionNameFromIndex(valueAInt));
                              }
                              result = paintNamesVolumeA[valueAInt];
                           }
                        }
                        break;
                     case VOLUME_MATH_OPERATION_AVERAGE:
                        result = (valueA + valueB) * 0.5;
                        break;
                     case VOLUME_MATH_EXCLUSIVE_OR:
                        if ((valueA != 0.0) && (valueB == 0.0)) {
                           result = valueA;
                        }
                        else if ((valueA == 0.0) && (valueB != 0.0)) {
                           result = valueB;
                        }
                        else {
                           result = 0.0;
                        }
                        break;
                  }
                  
                  outputVolume->setVoxel(i, j, k, 0, result);
               }
            }
         }
      }
   }
   
   //
   // Set the paint names for the output volume
   //
   if (operation == VOLUME_MATH_OPERATION_COMBINE_PAINT) {
      outputVolume->regionNames = paintNameVolume.regionNames;
   }
}                                              

/**
 * Read data of the specified type.
 */
void 
VolumeFile::readRgbDataVoxelInterleaved(gzFile dataFile) throw (FileException)
{
   const int numVoxels = getTotalNumberOfVoxels();
   const int length = numVoxels * sizeof(unsigned char) * 3;
   unsigned char* data = new unsigned char[length];
   
   const int numRead = gzread(dataFile, (char*)data, (unsigned)length);
   if (numRead != length) {
      std::ostringstream str;
      str << "Premature EOF reading zipped file.  Tried to read\n"
          << length << "bytes.  Actually read " << numRead << ".\n";
      throw FileException(getDataFileNameForReadError(),
                            str.str().c_str());
   }
   
   for (int i = 0; i < numVoxels; i++) {
      const int indx = i * 3;
      voxels[indx]   = data[indx];
      voxels[indx+1] = data[indx+1];
      voxels[indx+2] = data[indx+2];
   }
   delete[] data;
}

/**
 * Read data of the specified type.
 */
void 
VolumeFile::readRgbDataSliceInterleaved(gzFile dataFile) throw (FileException)
{
   const int numVoxels = getTotalNumberOfVoxels();
   const int length = numVoxels * sizeof(unsigned char) * 3;
   unsigned char* data = new unsigned char[length];
   
   const int numRead = gzread(dataFile, (char*)data, (unsigned)length);
   if (numRead != length) {
      std::ostringstream str;
      str << "Premature EOF reading zipped file.  Tried to read\n"
          << length << "bytes.  Actually read " << numRead << ".\n";
      throw FileException(getDataFileNameForReadError(),
                            str.str().c_str());
   }
   
   int dim[3];
   getDimensions(dim);
   const int sliceOffset = dim[0] * dim[1];
   const int componentOffset = sliceOffset * 3;
   for (int k = 0; k < dim[2]; k++) {
      for (int j = 0; j < dim[1]; j++) {
         for (int i = 0; i < dim[0]; i++) {
            int ijk[3] = { i, j, k };
            const int pointID = getVoxelDataIndex(ijk);
            const int dataID = componentOffset * k + (j * dim[0]) + i;
            voxels[pointID]   = data[dataID];
            voxels[pointID+1] = data[dataID + sliceOffset];
            voxels[pointID+2] = data[dataID + (sliceOffset * 2)];
         }
      }
   }
   
   delete[] data;
}

/**
 * Read data of the specified type.
 */
void 
VolumeFile::readCharData(gzFile dataFile) throw (FileException)
{
   const int numVoxels = getTotalNumberOfVoxels();
   const int length = numVoxels * sizeof(char);
   char* data = new char[length];
   
   const int numRead = gzread(dataFile, (char*)data, (unsigned)length);
   if (numRead != length) {
      std::ostringstream str;
      str << "Premature EOF reading zipped file.  Tried to read\n"
          << length << "bytes.  Actually read " << numRead << ".\n";
      throw FileException(getDataFileNameForReadError(),
                            str.str().c_str());
   }
   
   for (int i = 0; i < numVoxels; i++) {
      voxels[i] = data[i];
   }
   delete[] data;
}

/**
 * Read data of the specified type.
 */
void 
VolumeFile::readUnsignedCharData(gzFile dataFile) throw (FileException)
{
   const int numVoxels = getTotalNumberOfVoxels();
   const int length = numVoxels * sizeof(unsigned char);
   unsigned char* data = new unsigned char[length];
   
   const int numRead = gzread(dataFile, (char*)data, (unsigned)length);
   if (numRead != length) {
      std::ostringstream str;
      str << "Premature EOF reading zipped file.  Tried to read\n"
          << length << "bytes.  Actually read " << numRead << ".\n";
      throw FileException(getDataFileNameForReadError(),
                            str.str().c_str());
   }
   
   for (int i = 0; i < numVoxels; i++) {
      voxels[i] = data[i];
   }
   delete[] data;
}

/**
 * Read data of the specified type.
 */
void 
VolumeFile::readShortData(gzFile dataFile, const bool byteSwapData) throw (FileException)
{
   const int numVoxels = getTotalNumberOfVoxels();
   const int length = numVoxels * sizeof(short);
   short* data = new short[length];
   
   const int numRead = gzread(dataFile, (char*)data, (unsigned)length);
   if (numRead != length) {
      std::ostringstream str;
      str << "Premature EOF reading zipped file.  Tried to read\n"
          << length << "bytes.  Actually read " << numRead << ".\n";
      throw FileException(getDataFileNameForReadError(),
                            str.str().c_str());
   }
   
   if (byteSwapData) {
      ByteSwapping::swapBytes(data, numVoxels);
   }
   
   for (int i = 0; i < numVoxels; i++) {
      voxels[i] = data[i];
   }
   delete[] data;
}

/**
 * Read data of the specified type.
 */
void 
VolumeFile::readUnsignedShortData(gzFile dataFile, const bool byteSwapData) throw (FileException)
{
   const int numVoxels = getTotalNumberOfVoxels();
   const int length = numVoxels * sizeof(unsigned short);
   unsigned short* data = new unsigned short[length];
   
   const int numRead = gzread(dataFile, (char*)data, (unsigned)length);
   if (numRead != length) {
      std::ostringstream str;
      str << "Premature EOF reading zipped file.  Tried to read\n"
          << length << "bytes.  Actually read " << numRead << ".\n";
      throw FileException(getDataFileNameForReadError(),
                            str.str().c_str());
   }
   
   if (byteSwapData) {
      ByteSwapping::swapBytes(data, numVoxels);
   }
   
   for (int i = 0; i < numVoxels; i++) {
      voxels[i] = data[i];
   }
   delete[] data;
}

/**
 * Read data of the specified type.
 */
void 
VolumeFile::readIntData(gzFile dataFile, const bool byteSwapData) throw (FileException)
{
   const int numVoxels = getTotalNumberOfVoxels();
   const int length = numVoxels * sizeof(int);
   int* data = new int[length];
   
   const int numRead = gzread(dataFile, (char*)data, (unsigned)length);
   if (numRead != length) {
      std::ostringstream str;
      str << "Premature EOF reading zipped file.  Tried to read\n"
          << length << "bytes.  Actually read " << numRead << ".\n";
      throw FileException(getDataFileNameForReadError(),
                            str.str().c_str());
   }
   
   if (byteSwapData) {
      ByteSwapping::swapBytes(data, numVoxels);
   }
   
   for (int i = 0; i < numVoxels; i++) {
      voxels[i] = data[i];
   }
   delete[] data;
}

/**
 * Read data of the specified type.
 */
void 
VolumeFile::readUnsignedIntData(gzFile dataFile, const bool byteSwapData) throw (FileException)
{
   const int numVoxels = getTotalNumberOfVoxels();
   const int length = numVoxels * sizeof(unsigned int);
   unsigned int* data = new unsigned int[length];
   
   const int numRead = gzread(dataFile, (char*)data, (unsigned)length);
   if (numRead != length) {
      std::ostringstream str;
      str << "Premature EOF reading zipped file.  Tried to read\n"
          << length << "bytes.  Actually read " << numRead << ".\n";
      throw FileException(getDataFileNameForReadError(),
                            str.str().c_str());
   }
   
   if (byteSwapData) {
      ByteSwapping::swapBytes(data, numVoxels);
   }
   
   for (int i = 0; i < numVoxels; i++) {
      voxels[i] = data[i];
   }
   delete[] data;
}

/**
 * Read data of the specified type.
 */
void 
VolumeFile::readLongLongData(gzFile dataFile, const bool byteSwapData) throw (FileException)
{
   const int numVoxels = getTotalNumberOfVoxels();
   const int length = numVoxels * sizeof(long long);
   long long* data = new long long[length];
   
   const int numRead = gzread(dataFile, (char*)data, (unsigned)length);
   if (numRead != length) {
      std::ostringstream str;
      str << "Premature EOF reading zipped file.  Tried to read\n"
          << length << "bytes.  Actually read " << numRead << ".\n";
      throw FileException(getDataFileNameForReadError(),
                            str.str().c_str());
   }
   
   if (byteSwapData) {
      ByteSwapping::swapBytes(data, numVoxels);
   }
   
   for (int i = 0; i < numVoxels; i++) {
      voxels[i] = data[i];
   }
   delete[] data;
}

/**
 * Read data of the specified type.
 */
void 
VolumeFile::readUnsignedLongLongData(gzFile dataFile, const bool byteSwapData) throw (FileException)
{
   const int numVoxels = getTotalNumberOfVoxels();
   const int length = numVoxels * sizeof(unsigned long long);
   unsigned long long* data = new unsigned long long[length];
   
   const int numRead = gzread(dataFile, (char*)data, (unsigned)length);
   if (numRead != length) {
      std::ostringstream str;
      str << "Premature EOF reading zipped file.  Tried to read\n"
          << length << "bytes.  Actually read " << numRead << ".\n";
      throw FileException(getDataFileNameForReadError(),
                            str.str().c_str());
   }
   
   if (byteSwapData) {
      ByteSwapping::swapBytes(data, numVoxels);
   }
   
   for (int i = 0; i < numVoxels; i++) {
      voxels[i] = data[i];
   }
   delete[] data;
}

/**
 * Read data of the specified type.
 */
void 
VolumeFile::readFloatData(gzFile dataFile, const bool byteSwapData) throw (FileException)
{
   const int numVoxels = getTotalNumberOfVoxels();
   const int length = numVoxels * sizeof(float);
   float* data = new float[length];
   
   const int numRead = gzread(dataFile, (char*)data, (unsigned)length);
   if (numRead != length) {
      std::ostringstream str;
      str << "Premature EOF reading zipped file.  Tried to read\n"
          << length << "bytes.  Actually read " << numRead << ".\n";
      throw FileException(getDataFileNameForReadError(),
                            str.str().c_str());
   }
   
   if (byteSwapData) {
      ByteSwapping::swapBytes(data, numVoxels);
   }
   
   for (int i = 0; i < numVoxels; i++) {
      voxels[i] = data[i];
   }
   delete[] data;
}

/**
 * Read data of the specified type.
 */
void 
VolumeFile::readDoubleData(gzFile dataFile, const bool byteSwapData) throw (FileException)
{
   const int numVoxels = getTotalNumberOfVoxels();
   const int length = numVoxels * sizeof(double);
   double* data = new double[length];
   
   const int numRead = gzread(dataFile, (char*)data, (unsigned)length);
   if (numRead != length) {
      std::ostringstream str;
      str << "Premature EOF reading zipped file.  Tried to read\n"
          << length << "bytes.  Actually read " << numRead << ".\n";
      throw FileException(getDataFileNameForReadError(),
                            str.str().c_str());
   }

   if (byteSwapData) {
      ByteSwapping::swapBytes(data, numVoxels);
   }
   
   for (int i = 0; i < numVoxels; i++) {
      voxels[i] = data[i];
   }
   delete[] data;
}

/**
 * Read both header and volume.
 */
void
VolumeFile::readFile(const QString& fileNameIn) throw (FileException)
{
   readFile(fileNameIn, 0, false);
}

/**
 * Write the volume to a file (header and data).
 */
void
VolumeFile::writeFile(const QString& filenameIn) throw (FileException)
{
   std::vector<VolumeFile*> volumes;
   volumes.push_back(this);
   writeFile(filenameIn,
             getVolumeType(),
             getVoxelDataType(),
             volumes,
             false);
}

/**
 * Read the spec file data (should never be called).
 */
void 
VolumeFile::readFileData(QFile& /*file*/, QTextStream& /*stream*/, QDataStream&,
                                  QDomElement& /* rootElement */) throw (FileException)
{
   throw FileException(filename, "VolumeFile::readFileData was called and should never be called."
                         "This is a programming error.");
}


/** write a volume in the specified type and add the correct file extension 
 * (assumes no file extension).
 */
void 
VolumeFile::writeVolumeFile(VolumeFile* vf,
                            const FILE_READ_WRITE_TYPE fileType,
                            const QString filenameWithoutExtensionIn,
                            const bool compressVolumeFileFlagIn,
                            QString& fileNameOut,
                            QString& dataFileNameOut) throw (FileException)
{
   bool compressVolumeFileFlag = compressVolumeFileFlagIn;
   
   if (vf == NULL) {
      throw FileException("VolumeFile::writeVolumeFile was passed a NULL volume pointer.");
   }
   if (filenameWithoutExtensionIn.isEmpty()) {
      throw FileException("Filename is empty");
   }
   
   QString filenameWithoutExtension = filenameWithoutExtensionIn;
   if (filenameWithoutExtension.endsWith("+orig") == false) {
      filenameWithoutExtension += "+orig";
   }
   fileNameOut = "";
   dataFileNameOut = "";
   QString ext;
   bool zipAfniFlag = false;
   switch(fileType) {
      case FILE_READ_WRITE_TYPE_RAW:
         throw FileException("RAW volume type not supported for writing.");
         break;
      case FILE_READ_WRITE_TYPE_AFNI:
         fileNameOut = filenameWithoutExtension + SpecFile::getAfniVolumeFileExtension();
         dataFileNameOut = filenameWithoutExtension + ".BRIK";
         if (compressVolumeFileFlag) {
            dataFileNameOut += ".gz";
            zipAfniFlag = true;
         }
         break;
      case FILE_READ_WRITE_TYPE_ANALYZE:
         fileNameOut = filenameWithoutExtension + SpecFile::getAnalyzeVolumeFileExtension();
         dataFileNameOut = filenameWithoutExtension + ".img";
         break;
      case FILE_READ_WRITE_TYPE_NIFTI:
         fileNameOut = filenameWithoutExtension + SpecFile::getNiftiVolumeFileExtension();
         if (compressVolumeFileFlag) {
            fileNameOut += ".gz";
         }
         break;
      case FILE_READ_WRITE_TYPE_NIFTI_GZIP:
         fileNameOut = filenameWithoutExtension + SpecFile::getNiftiGzipVolumeFileExtension();
         compressVolumeFileFlag = true;
         break;
      case FILE_READ_WRITE_TYPE_SPM_OR_MEDX:
         fileNameOut = filenameWithoutExtension + SpecFile::getAnalyzeVolumeFileExtension();
         dataFileNameOut = filenameWithoutExtension + ".img";
         break;
      case FILE_READ_WRITE_TYPE_WUNIL:
         fileNameOut = filenameWithoutExtension + SpecFile::getWustlVolumeFileExtension();
         dataFileNameOut = filenameWithoutExtension + ".img";
         break;
      case FILE_READ_WRITE_TYPE_UNKNOWN:
         throw FileException("Undefined file type not supported for writing volume.");
         break;
   }
   
   std::vector<VolumeFile*> theFiles;
   theFiles.push_back(vf);
   vf->writeFile(fileNameOut,
                 vf->getVolumeType(),
                 vf->getVoxelDataType(),
                 theFiles,
                 zipAfniFlag);
}

/**
 * Write the spec file data (should never be called).
 */
void 
VolumeFile::writeFileData(QTextStream& /*stream*/, QDataStream&,
                                 QDomDocument& /* xmlDoc */,
                                  QDomElement& /* rootElement */) throw (FileException)
{
   throw FileException(filename, "VolumeFile::writeFileData was called and should never be called."
                         "This is a programming error.");
}

/**
 * Dual hreshold volume (voxels between thresholds become 255, all other voxels 0).
 */
void    
VolumeFile::dualThresholdVolume(const float thresholdLow, const float thresholdHigh)
{
   int cnt = 0;
   const int num = getTotalNumberOfVoxelElements();
   for (int i = 0; i < num; i++){
      if ((voxels[i] > thresholdLow) && (voxels[i] < thresholdHigh)) {
         voxels[i] = 255;
         cnt++;
      }
      else {
         voxels[i] = 0;
      }
   }
   if (DebugControl::getDebugOn()) {
      std::cout << "\tDual Threshold " << thresholdLow << ", " << thresholdHigh << std::endl;
      std::cout << "\tThresholded " << cnt 
                << " voxels " << (cnt/(float)(num)*100.0)
                << "%"
                << std::endl;
   }
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 * threshold volume (voxels above become 255, voxels below 0).
 */
void    
VolumeFile::thresholdVolume(const float thresholdValue)
{
   int cnt = 0;
   const int num = getTotalNumberOfVoxelElements();
   for (int i = 0; i < num; i++){
      if (voxels[i] > thresholdValue) {
         voxels[i] = 255;
         cnt++;
      }
      else {
         voxels[i] = 0;
      }
   }
   if (DebugControl::getDebugOn()) {
      std::cout << "\tThreshold " << thresholdValue << std::endl;
      std::cout << "\tThresholded " << cnt 
                << " voxels " << (cnt/(float)(num)*100.0)
                << "%"
                << std::endl;
   }
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 * threshold volume (voxels below become 255, voxels above 0).
 */
void    
VolumeFile::inverseThresholdVolume(const float thresholdValue)
{
   int cnt = 0;
   const int num = getTotalNumberOfVoxelElements();
   for (int i = 0; i < num; i++){
      if (voxels[i] < thresholdValue){
         voxels[i] = 255;
         cnt++;
      }
      else {
         voxels[i] = 0;
      }
   }
   
   if (DebugControl::getDebugOn()) {
      std::cout << "\tInverse Threshold " << thresholdValue << std::endl;
      std::cout << "\tInverse Thresholded " << cnt 
                << " voxels " << (cnt/(float)(num)*100.0)
                << std::endl;
   }
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 * clamp a voxel dimensions to within valid values (0 to dim).
 */
void 
VolumeFile::clampVoxelDimension(int voxelIJK[3]) const
{
   clampVoxelDimension(VOLUME_AXIS_X, voxelIJK[0]);
   clampVoxelDimension(VOLUME_AXIS_Y, voxelIJK[1]);
   clampVoxelDimension(VOLUME_AXIS_Z, voxelIJK[2]);
}

/**
 * clamp a voxel dimension to within valid values (0 to dim).
 */
void 
VolumeFile::clampVoxelDimension(const VOLUME_AXIS axis,
                               int& voxelIndex) const
{
   int maxIndex = 0;
   switch (axis) {
      case VOLUME_AXIS_X:
         maxIndex = dimensions[0];
         break;
      case VOLUME_AXIS_Y:
         maxIndex = dimensions[1];
         break;
      case VOLUME_AXIS_Z:
         maxIndex = dimensions[2];
         break;
      case VOLUME_AXIS_ALL:
      case VOLUME_AXIS_OBLIQUE:
      case VOLUME_AXIS_OBLIQUE_X:
      case VOLUME_AXIS_OBLIQUE_Y:
      case VOLUME_AXIS_OBLIQUE_Z:
      case VOLUME_AXIS_OBLIQUE_ALL:
      case VOLUME_AXIS_UNKNOWN:
         break;
   }
   
   voxelIndex = std::max(voxelIndex, 0);
   voxelIndex = std::min(voxelIndex, maxIndex);
}                          


/**
 * clamp a voxel index to within valid values (0 to dim-1).
 */
void 
VolumeFile::clampVoxelIndex(int voxelIJK[3]) const
{
   clampVoxelIndex(VOLUME_AXIS_X, voxelIJK[0]);
   clampVoxelIndex(VOLUME_AXIS_Y, voxelIJK[1]);
   clampVoxelIndex(VOLUME_AXIS_Z, voxelIJK[2]);
}
      
/**
 * clamp a voxel index to within valid values (0 to dim-1).
 */
void 
VolumeFile::clampVoxelIndex(const VOLUME_AXIS axis,
                               int& voxelIndex) const
{
   int maxIndex = 0;
   switch (axis) {
      case VOLUME_AXIS_X:
         maxIndex = dimensions[0];
         break;
      case VOLUME_AXIS_Y:
         maxIndex = dimensions[1];
         break;
      case VOLUME_AXIS_Z:
         maxIndex = dimensions[2];
         break;
      case VOLUME_AXIS_ALL:
      case VOLUME_AXIS_OBLIQUE:
      case VOLUME_AXIS_OBLIQUE_X:
      case VOLUME_AXIS_OBLIQUE_Y:
      case VOLUME_AXIS_OBLIQUE_Z:
      case VOLUME_AXIS_OBLIQUE_ALL:
      case VOLUME_AXIS_UNKNOWN:
         break;
   }
   
   voxelIndex = std::max(voxelIndex, 0);
   voxelIndex = std::min(voxelIndex, maxIndex - 1);
}                          

/**
 * get 6 connected neighbors for a voxel.
 */
void 
VolumeFile::getNeighbors(const VoxelIJK& voxel, 
                            std::vector<VoxelIJK>& neighbors) const
{
   int ijk[3];
   voxel.getIJK(ijk);
   
   int n[3];
   n[0] = ijk[0] - 1;
   n[1] = ijk[1];
   n[2] = ijk[2];
   if (getVoxelIndexValid(n)) {
      neighbors.push_back(VoxelIJK(n));
   }
   
   n[0] = ijk[0] + 1;
   n[1] = ijk[1];
   n[2] = ijk[2];
   if (getVoxelIndexValid(n)) {
      neighbors.push_back(VoxelIJK(n));
   }
   
   n[0] = ijk[0];
   n[1] = ijk[1] - 1;
   n[2] = ijk[2];
   if (getVoxelIndexValid(n)) {
      neighbors.push_back(VoxelIJK(n));
   }
   
   n[0] = ijk[0];
   n[1] = ijk[1] + 1;
   n[2] = ijk[2];
   if (getVoxelIndexValid(n)) {
      neighbors.push_back(VoxelIJK(n));
   }
   
   n[0] = ijk[0];
   n[1] = ijk[1];
   n[2] = ijk[2] - 1;
   if (getVoxelIndexValid(n)) {
      neighbors.push_back(VoxelIJK(n));
   }
   
   n[0] = ijk[0];
   n[1] = ijk[1];
   n[2] = ijk[2] + 1;
   if (getVoxelIndexValid(n)) {
      neighbors.push_back(VoxelIJK(n));
   }
}

/**
 * Fill the largest connect set of voxels within the specified bounds.
 */
bool 
VolumeFile::fillBiggestObjectWithinMask(const int imin, 
                                        const int imax, 
                                        const int jmin, 
                                        const int jmax, 
                                        const int kmin, 
                                        const int kmax,
                                        const float minValue,
                                        const float maxValue)
{
   const int extent[6] = {
      imin,
      imax,
      jmin,
      jmax,
      kmin,
      kmax
   };
   
   return fillBiggestObjectWithinMask(extent, minValue, maxValue);
}
                                 
/** 
 * Fill the largest connect set of voxels within the specified bounds.
 */
bool 
VolumeFile::fillBiggestObjectWithinMask(const int extent[6],
                                        const float minValue,
                                        const float maxValue)
{
   VoxelIJK bigSeed(-1, -1, -1);
   const int numObjects = findBiggestObjectWithinMask(extent, minValue, maxValue, bigSeed);
   if ((numObjects > 0) && (bigSeed.getI() >= 0)) {
      floodFillWithVTK(bigSeed, 255, 255, 0);
      return true;
   }
   return false;
}                                       
      
/**
 * Find the largest connect set of voxels in the volume.
 * Returns the number of objects found.
 */
int 
VolumeFile::findBiggestObject(const float minValue,
                              const float maxValue, 
                              VoxelIJK& bigSeed) const
{
   const int extent[6] = {
      0,
      dimensions[0],
      0,
      dimensions[1],
      0,
      dimensions[2]
   };
   
   return findBiggestObjectWithinMask(extent, minValue, maxValue, bigSeed);
}

/**
 * Find the largest connect set of voxels within the specified bounds.
 */
int 
VolumeFile::findBiggestObjectWithinMask(const int extent[6],
                                        const float minValue,
                                        const float maxValue, 
                                        int bigSeed[3]) const
{
   VoxelIJK ijkSeed(bigSeed);
   const int val = findBiggestObjectWithinMask(extent, minValue, maxValue, ijkSeed);
   bigSeed[0] = ijkSeed.getI();
   bigSeed[1] = ijkSeed.getJ();
   bigSeed[2] = ijkSeed.getK();
   return val;
}                                       
                                       
/**
 * Find the largest connect set of voxels within the specified bounds.
 * Returns the number of objects found.
 */
int	
VolumeFile::findBiggestObjectWithinMask(const int extent[6], 
                                           const float minValue,
                                           const float maxValue,
                                           VoxelIJK& bigSeed) const
{
   return findBiggestObjectWithinMask(extent[0],
                                      extent[1],
                                      extent[2],
                                      extent[3],
                                      extent[4],
                                      extent[5],
                                      minValue,
                                      maxValue,
                                      bigSeed);
}

/**
 * Find the largest connect set of voxels within the specified bounds.
 * Returns the number of objects found.
 */
int	
VolumeFile::findBiggestObjectWithinMask(const int iminIn, 
                                           const int imaxIn, 
                                           const int jminIn, 
                                           const int jmaxIn, 
                                           const int kminIn, 
                                           const int kmaxIn, 
                                           const float minValue,
                                           const float maxValue,
                                           VoxelIJK& bigSeed) const
{
   bigSeed.setIJK(-1, -1, -1);

   int imin = iminIn;
   int imax = imaxIn;
   int jmin = jminIn;
   int jmax = jmaxIn;
   int kmin = kminIn;
   int kmax = kmaxIn;
   
   //
   // largest object count
   //
   int largestObjectCount = 0;
      
   //
   // Number of objects found
   //
   int numberOfObjectsFound = 0;
   
   //
   // Make sure search region is within the volume
   //
   clampVoxelDimension(VOLUME_AXIS_X, imin);
   clampVoxelDimension(VOLUME_AXIS_X, imax);
   clampVoxelDimension(VOLUME_AXIS_Y, jmin);
   clampVoxelDimension(VOLUME_AXIS_Y, jmax);
   clampVoxelDimension(VOLUME_AXIS_Z, kmin);
   clampVoxelDimension(VOLUME_AXIS_Z, kmax);

   if (DebugControl::getDebugOn()) {
   	std::cout << "FindBiggestObjectWithinMask "
		          << imin << " "
                << imax << " "
                << jmin << " "
                << jmax << " "
                << kmin << " "
                << kmax << " "
                << std::endl;
   }

   const int numVoxels = getTotalNumberOfVoxels();
   
   //
   // single slice ?
   //
	if ((imin == imax) || (jmin == jmax) || (kmin == kmax)) {
      if (DebugControl::getDebugOn()) {
         std::cout << "For x, y or z, min = max" << std::endl;
      }
   }
	else {
      //
      // Create a flag for noting which voxels have been searched.
      // Mark those within region and within min and max values NOT SEARCHED
      // and all others SEARCHED.
      //
		VOXEL_SEARCH_STATUS* voxelSearched = new VOXEL_SEARCH_STATUS[numVoxels];
		for (int k = 0; k < dimensions[2]; k++) {
			for (int j = 0; j < dimensions[1]; j++) {
				for (int i = 0; i < dimensions[0]; i++) {
               const int idx = getVoxelDataIndex(i, j, k);
               voxelSearched[idx] = VOXEL_SEARCHED;
               if ((i >= imin) && (i < imax) &&
                   (j >= jmin) && (j < jmax) &&
                   (k >= kmin) && (k < kmax)) {
                  if ((voxels[idx] >= minValue) &&
                      (voxels[idx] <= maxValue)) {
                     voxelSearched[idx] = VOXEL_NOT_SEARCHED;
                  }
               }
				}
			}
		}
	
      //
      // Find a voxel within value range that has not been searched
      //
      VoxelIJK seedVoxel;
      bool voxelFound = findUnsearchedVoxel(minValue, maxValue, voxelSearched, seedVoxel);
      
      if (voxelFound == false) {
         if (DebugControl::getDebugOn()) {
            std::cout << "FindBiggestObjectWithinMask no initial voxel found with values: "
                      << minValue << " " << maxValue << std::endl;
         }
      }
      
      //
      // Loop through all objects
      //
      while (voxelFound) {
         //
         // Create a stack 
         //
         std::stack<VoxelIJK> stack;
         stack.push(seedVoxel);
         
         //
         // Number of voxels in this connected piece
         //
         int numConnectedVoxels = 0;
         
         //
         // While there are voxels to search
         //
         while (stack.empty() == false) {
            //
            // Get the next voxel to search
            //
            const VoxelIJK v = stack.top();
            stack.pop();
          
            const int idx = getVoxelDataIndex(v.getIJK());
            if (voxelSearched[idx] == VOXEL_NOT_SEARCHED) {
               voxelSearched[idx] = VOXEL_SEARCHED;
               
               //
               // Increase connected count
               //
               numConnectedVoxels++;
            
               //
               // Get the neighboring voxels
               //
               std::vector<VoxelIJK> neighbors;
               getNeighbors(v, neighbors);
               
               //
               // Add neighbors to stack
               //
               for (unsigned int i = 0; i < neighbors.size(); i++) {
                  VoxelIJK& v = neighbors[i];
                  const int idx = getVoxelDataIndex(v.getIJK());
                  if (voxelSearched[idx] == VOXEL_NOT_SEARCHED) {
                     stack.push(v);
                  }
               }
            }
         }  // while (stack.isEmpty() == false)
         
         //
         // Was an object found
         //
         if (numConnectedVoxels > 0) {
            numberOfObjectsFound++;
            if (DebugControl::getDebugOn()) {
               int i, j, k;
               seedVoxel.getIJK(i, j, k);
               std::cout << "\t"
                         << "seed : "
                         << i << ", "
                         << j << ", "
                         << k << ": size "
                         << numConnectedVoxels 
                         << std::endl;
            }
         }
         
         //
         // Is this bigger than previous object
         //
         if (numConnectedVoxels > largestObjectCount) {
            largestObjectCount = numConnectedVoxels;
            bigSeed = seedVoxel;
         }
         
         //
         // Find another voxel to search
         //
         voxelFound = findUnsearchedVoxel(minValue, maxValue, voxelSearched, seedVoxel);
      } // while (voxelFound)
      
      
		if (largestObjectCount > 0) {
         if (DebugControl::getDebugOn()) {
               int i, j, k;
               bigSeed.getIJK(i, j, k);
               std::cout << "\t"
                         << "MAX seed : "
                         << i << ", "
                         << j << ", "
                         << k << ": size "
                         << largestObjectCount 
                         << std::endl;
         }
      }
		delete[] voxelSearched;
	}	
   
	if (largestObjectCount == 0) {
      if (DebugControl::getDebugOn()) {
         std::cout << "\tNo voxels found, clearing volume." << std::endl;
      }
		for (int i = 0; i < numVoxels; i++) {
         voxels[i] = 0.0;
      }
	}
   
   return numberOfObjectsFound;
}

/**
 * Find voxel within value range that has not been searched.
 * Returns true if found else false.
 */
bool	
VolumeFile::findUnsearchedVoxel(const float minValue,
                                   const float maxValue, 
                                   VOXEL_SEARCH_STATUS searchStatus[],
                                   VoxelIJK& seedOut) const
{
   //
   // Loop through region
   //
   for (int k = 0; k < dimensions[2]; k++) {
      for (int j = 0; j < dimensions[1]; j++) {
         for (int i = 0; i < dimensions[0]; i++) {
            const int idx = getVoxelDataIndex(i, j, k);
            if (searchStatus[idx] == VOXEL_NOT_SEARCHED) {
               if ((voxels[idx] >= minValue) && (voxels[idx] <= maxValue)) {
                  seedOut.setIJK(i, j, k);
                  return true;
               }
            }
         }
      }
   }
   return false;
}

/**
 * remove islands (all but the largest connected piece of surface).
 * returns true if islands were removed
 */
bool 
VolumeFile::removeIslandsFromSegmentation()
{
   //
   // Find the biggest piece of surface
   //
   VoxelIJK biggestPiece;
   if (findBiggestObject(255.0, 255.0, biggestPiece) > 1) {
      //
      // Keep only the biggest piece of surface
      //
      floodFillWithVTK(biggestPiece, 255, 255, 0);
      return true;
   }
   
   return false;
}

/**
 * flood fill slice with VTK.
 */
void 
VolumeFile::floodFillSliceWithVTK(const VolumeFile::VOLUME_AXIS axis,
                             const int seed[3],
                             const int connectedValueIn,
                             const int connectedValueOut,
                             const int unconnectedValueOut,
                             VolumeModification* modifiedVoxels)
{
   int sliceNum = 0;
   switch (axis) {
      case VOLUME_AXIS_X:
         sliceNum = seed[0];
         break;
      case VOLUME_AXIS_Y:
         sliceNum = seed[1];
         break;
      case VOLUME_AXIS_Z:
         sliceNum = seed[2];
         break;
      case VOLUME_AXIS_ALL:
      case VOLUME_AXIS_OBLIQUE:
      case VOLUME_AXIS_OBLIQUE_X:
      case VOLUME_AXIS_OBLIQUE_Y:
      case VOLUME_AXIS_OBLIQUE_Z:
      case VOLUME_AXIS_OBLIQUE_ALL:
      case VOLUME_AXIS_UNKNOWN:
         return;
         break;
   }
   VolumeFile vf(*this);
   vf.setAllVoxels(0.0);
   vf.copySlice(this,
                sliceNum,
                axis,
                sliceNum);
   vf.floodFillWithVTK(seed,
                        connectedValueIn,
                        connectedValueOut,
                        unconnectedValueOut,
                        modifiedVoxels);
   copySlice(&vf,
             sliceNum,
             axis,
             sliceNum);

}
      
/**
 * flood fill with VTK.
 */
void 
VolumeFile::floodFillWithVTK(const int seed[3],
                             const int connectedValueIn,
                             const int connectedValueOut,
                             const int unconnectedValueOut,
                             VolumeModification* modifiedVoxels)
{
   VoxelIJK seedIJK(seed);
   floodFillWithVTK(seedIJK,
                    connectedValueIn,
                    connectedValueOut,
                    unconnectedValueOut,
                    modifiedVoxels);
}
                            
/**
 * flood fill with VTK.
 */
void 
VolumeFile::floodFillWithVTK(const VoxelIJK& seedVoxel,
                             const int connectedValueIn,
                             const int connectedValueOut,
                             const int unconnectedValueOut,
                             VolumeModification* modifiedVoxels)
{
   if ((seedVoxel.getI() < 0) ||
       (seedVoxel.getJ() < 0) ||
       (seedVoxel.getK() < 0)) {
      std::cout << "ERROR: VolumeFile::floodFillWithVTK() called with invalid seed." << std::endl;
      std::cout << "ERROR: Seed = (" << seedVoxel.getI() << ", "
                << seedVoxel.getJ() << ", "
                << seedVoxel.getK() << ")" << std::endl;
      return;
   }
   
   VolumeFile* copyOfVolume = NULL;
   if (modifiedVoxels != NULL) {
      copyOfVolume = new VolumeFile(*this);
   }
   vtkStructuredPoints* sp = convertToVtkStructuredPoints(true);
   
   vtkImageSeedConnectivity* connect = vtkImageSeedConnectivity::New();
#ifdef HAVE_VTK6
   connect->SetInputData(sp);
#else
   connect->SetInput(sp);
#endif
   connect->SetInputConnectValue(connectedValueIn);
   connect->SetOutputConnectedValue(connectedValueOut);
   connect->SetOutputUnconnectedValue(unconnectedValueOut);
   int i, j, k;
   seedVoxel.getIJK(i, j, k);
   connect->AddSeed(i, j, k);
   connect->Update();
   
   convertFromVtkImageData(connect->GetOutput());
   
   connect->Delete();
   sp->Delete();
   
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
   
   if (modifiedVoxels != NULL) {
      for (int i = 0; i < dimensions[0]; i++) {
         for (int j = 0; j < dimensions[1]; j++) {
            for (int k = 0; k < dimensions[2]; k++) {
               if (getVoxel(i, j, k) != copyOfVolume->getVoxel(i, j, k)) {
                  modifiedVoxels->addVoxel(this, i, j, k, copyOfVolume);
               }
            }
         }
      }
   }
   if (copyOfVolume != NULL) {
      delete copyOfVolume;
      copyOfVolume = NULL;
   }
}

//
// Find non-zero voxel extent and write limits file if filename is not isEmpty
//
void 
VolumeFile::findLimits(const QString& limitFileName, int extent[6])
{
   //
   // Find non-zero voxels
   //
   float coordExtent[6];
   getNonZeroVoxelExtent(extent, coordExtent);
   
   if (DebugControl::getDebugOn()) {
      std::cout << "\textent: X "
                << extent[0] << " " << extent[1]
                << "; Y "
                << extent[2] << " " << extent[3]
                << "; Z "
                << extent[4] << " " << extent[5]
                << std::endl;
   }

   //
   // Should limits file be written
   //
   if (limitFileName.isEmpty() == false) {
      QFile file(limitFileName);
      if (file.open(QIODevice::WriteOnly)) {
         QTextStream stream(&file);
         stream.setRealNumberNotation(QTextStream::FixedNotation);
         stream.setRealNumberPrecision(6);

         stream << "LimitXmin=" << extent[0] << "\n";
         stream << "LimitXmax=" << extent[1] << "\n";
         stream << "LimitYmin=" << extent[2] << "\n";
         stream << "LimitYmax=" << extent[3] << "\n";
         stream << "LimitZmin=" << extent[4] << "\n";
         stream << "LimitZmax=" << extent[5] << "\n";
         
         file.close();
      }
      else {
        std::cout << "Unable to open limits file: " << limitFileName.toAscii().constData() << std::endl;
      }
   }
}

/**
 * Shift the volume "offset" voxels in the specified axis
 */
void	
VolumeFile::shiftAxis(const VOLUME_AXIS axis, const int offset)
{
	int	i, j, k, idx1, idx2;
	float	*voltemp;

   if (DebugControl::getDebugOn()) {
      std::cout << "ShiftAxis axis=" << axis 
                << " offset=" << offset
                << std::endl;
   }
   const int numVoxels = getTotalNumberOfVoxels();
   
	voltemp = new float [numVoxels];
	for (i = 0; i < numVoxels; i++) {
      voltemp [i] = 0;
   }
   
   const int nslices = dimensions[2];
   const int nrow    = dimensions[1];
   const int ncol    = dimensions[0];
   
	switch (axis){
		case VOLUME_AXIS_X:
			for (k = 0; k < nslices; k++){
				for (j = 0; j < nrow; j++){
					for (i = 0; i < ncol; i++){
						if (((i+offset) > 0) && ((i+offset) < ncol)){
							idx1 = getVoxelDataIndex(i, j, k);
							idx2 = getVoxelDataIndex((i+offset), j, k);
							voltemp[idx2] = voxels[idx1];
						}
					}
				}
			}
			break;
		case VOLUME_AXIS_Y:
			for (k = 0; k < nslices; k++){
				for (j = 0; j < nrow; j++){
					for (i = 0; i < ncol; i++){
						if (((j+offset) > 0) && ((j+offset) < nrow)){
							idx1 = getVoxelDataIndex(i, j, k);
							idx2 = getVoxelDataIndex(i, (j+offset), k);
							voltemp[idx2] = voxels[idx1];
						}
					}
				}
			}
			break;
		case VOLUME_AXIS_Z:
			for (k = 0; k < nslices; k++){
				for (j = 0; j < nrow; j++){
					for (i = 0; i < ncol; i++){
						if (((k+offset) > 0) && ((k+offset) < nslices)){
							idx1 = getVoxelDataIndex(i, j, k);
							idx2 = getVoxelDataIndex(i, j, (k+offset));
							voltemp[idx2] = voxels[idx1];
						}
					}
				}
			}
			break;
      case VOLUME_AXIS_OBLIQUE:
      case VOLUME_AXIS_OBLIQUE_X:
      case VOLUME_AXIS_OBLIQUE_Y:
      case VOLUME_AXIS_OBLIQUE_Z:
      case VOLUME_AXIS_OBLIQUE_ALL:
      case VOLUME_AXIS_ALL:
      case VOLUME_AXIS_UNKNOWN:
         break;
	}
	for (i = 0; i < numVoxels; i++) {
      voxels[i] = voltemp[i];
   }
   
	delete[] voltemp;

   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 * Smear voxel data along a specified axis.  During smearing, a voxel is set
 * to the maximum of its value and the corresponding voxel in an adjacent
 * slice.
 *
 * "mag" Number of iterations of smearing
 *
 * "sign"  offset of slice to smear with the current slice
 *     "-1" uses slice above
 *      "1" uses slice below
 *
 *  "core"  
 *     "0"  If a voxel's value decreased as a result of smearing, set the
 *          voxel to zero
 *     "1"  Output smearing regardless of voxel value changes.
 */
void	
VolumeFile::smearAxis(const VOLUME_AXIS axis, 
                         const int mag, 
                         const int sign, 
                         const int core) throw (FileException)
{
	const int fliphem = 1; // for 1582

   const int numVoxels = getTotalNumberOfVoxels();
   
	float* vol1 = new float[numVoxels];
	float* vol2 = new float[numVoxels];
   for (int i = 0; i < numVoxels; i++){
      vol1[i] = 0;
      vol2[i] = 0;
   }

	for (int i = 0; i < numVoxels; i++) {
	   vol1[i] = voxels[i];
   }
   
   const int nslices = dimensions[2];
   const int nrow    = dimensions[1];
   const int ncol    = dimensions[0];
   
	for (int count = 0; count < mag; count++){
	   switch (axis){
	      case VOLUME_AXIS_X:
            if (DebugControl::getDebugOn()) {
   	         std::cout << "\tSmearXaxis " << count
                         << " of " << mag << " smears" << std::endl;
            }
            for (int k = 0; k < nslices; k++){
               for (int j = 0; j < nrow; j++){
                  for (int i = 0; i < ncol; i++){
                     if (fliphem == 0){
                        if (((i-sign) > 0) && ((i-sign) < ncol)){
                           int idx1 = getVoxelDataIndex(i, j, k);
                           int idx2 = getVoxelDataIndex((i-sign), j, k);
		                     vol2[idx1] = std::max(vol1[idx2], vol1[idx1]);
                        }
                     }else{ //special case for 1582 HAD 5.28.99
		                  if (((i+sign) > 0) && ((i+sign) < ncol)){
                           int idx1 = getVoxelDataIndex(i, j, k);
                           int idx2 = getVoxelDataIndex((i+sign), j, k);
                           vol2[idx1] = std::max(vol1[idx2], vol1[idx1]);
                        }
                     }
                  }
               }
            }
            break;
         case VOLUME_AXIS_Y:
            if (DebugControl::getDebugOn()) {
   	         std::cout << "\tSmearYaxis " << count
                         << " of " << mag << " smears" << std::endl;
            }
            for (int k = 0; k < nslices; k++){
               for (int j = 0; j < nrow; j++){
                  for (int i = 0; i < ncol; i++){
                     if (((j-sign) > 0) && ((j-sign) < nrow)){
                        int idx1 = getVoxelDataIndex(i, j, k);
                        int idx2 = getVoxelDataIndex(i, (j-sign), k);
                        vol2[idx1] = std::max(vol1[idx2], vol1[idx1]);
                     }
                  }
               }
            }
		      break;
	      case VOLUME_AXIS_Z:	
            if (DebugControl::getDebugOn()) {
   	         std::cout << "\tSmearZaxis " << count
                         << " of " << mag << " smears" << std::endl;
            }
            for (int k = 0; k < nslices; k++){
               for (int j = 0; j < nrow; j++){
                  for (int i = 0; i < ncol; i++){
                     if (((k-sign) > 0) && ((k-sign) < nslices)){
                        int idx1 = getVoxelDataIndex(i, j, k);
                        int idx2 = getVoxelDataIndex(i, j, (k-sign));
                        vol2[idx1] = std::max(vol1[idx2], vol1[idx1]);
                     }
                  }
               }
            }
	         break;
         case VOLUME_AXIS_ALL:
         case VOLUME_AXIS_OBLIQUE:
         case VOLUME_AXIS_OBLIQUE_X:
         case VOLUME_AXIS_OBLIQUE_Y:
         case VOLUME_AXIS_OBLIQUE_Z:
         case VOLUME_AXIS_OBLIQUE_ALL:
         case VOLUME_AXIS_UNKNOWN:
            throw FileException("VOLUME SMEAR: AXIS must be X, Y, or Z");
            break;
      }
	   for (int i = 0; i < ncol*nrow*nslices; i++) {
	      vol1[i] = vol2[i];
      }
   }
	
   if (core == 0) {
      for (int i = 0; i < numVoxels; i++){
         voxels[i] = vol2[i] - voxels[i];
         if (voxels[i] < 0) {
            voxels[i] = 0;
         }
      }
   }
   else  {
      for (int i = 0; i < numVoxels; i++){
         voxels[i] = vol2[i]; 
      }
   }
   
	delete[] vol1;
	delete[] vol2;

   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 * Stretch the voxel values and exlucde top and bottom percentage (0 to 100)
 */
void 
VolumeFile::stretchVoxelValuesExcludePercentage(const float bottomPercentToExclude,
                                                const float topPercentToExclude)
{
   const StatisticHistogram* histogram = getHistogram(256, 
                                                      bottomPercentToExclude,
                                                      topPercentToExclude);
   float minValue, maxValue, range, mean, sampleDeviation;
   histogram->getDataStatistics(minValue, maxValue, range, mean, sampleDeviation);
   rescaleVoxelValues(minValue, maxValue, 0.0, 255.0);
   delete histogram;
   histogram = NULL;
}
                                               
/**
 * Stretch the voxels in the volume to range 0 to 255.
 */
void 
VolumeFile::stretchVoxelValues()
{
   const int numVoxels = getTotalNumberOfVoxels();
	float	minVoxel, maxVoxel;
   getMinMaxVoxelValues(minVoxel, maxVoxel);

	if (maxVoxel > minVoxel) {
		const float scale = 255.0/(maxVoxel-minVoxel);
      if (DebugControl::getDebugOn()) {
         std::cout << "stretchVoxelValues scale " << scale << std::endl;
      }
		for (int i = 0; i < numVoxels; i++){
         float value = std::min(scale * (voxels[i] - minVoxel), 255.0f);
			voxels[i] = static_cast<int>(value);
		}
	/* .000001 instead of 0 because of weird case where max=0
		evaluated to 0.000000000000000000000000000036 ??? */
	} else if ( maxVoxel > 0.000001 ) { //all voxels same positive value
		for (int i = 0; i < numVoxels; i++){
         voxels[i]=255;
      }
      if (DebugControl::getDebugOn()) {
   		std::cout << "0<min=max=" << maxVoxel << "; set all voxels to 255" << std::endl;
      }
	}
	else { //all voxels must be same nonpositive value
		for (int i = 0; i < numVoxels; i++) {
         voxels[i] = 0.0;
      }
      if (DebugControl::getDebugOn()) {
   		std::cout << "all voxels 0" << std::endl;
      }
	}
   
   if (DebugControl::getDebugOn()) {
   	std::cout << "min " <<  minVoxel << " max " << maxVoxel << std::endl;
   }

   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 * apply a volume mask to "this" volume using STEREOTAXIC coordinates.
 * Voxel in "my" volume remain on only if the corresponding voxel in 
 * the mask volume is non-zero.
 */
void 
VolumeFile::maskWithVolume(const VolumeFile* maskVolume) throw (FileException)
{
   if (maskVolume == NULL) {
      throw FileException("Input mask volume is NULL");
   }
   
   //
   // loop through my voxels
   //
   int dim[3];
   getDimensions(dim);
   for (int i = 0; i < dim[0]; i++) {
      for (int j = 0; j < dim[1]; j++) {
         for (int k = 0; k < dim[2]; k++) {
            //
            // Get coordinate of my voxel
            //
            float xyz[3];
            getVoxelCoordinate(i, j, k, xyz);
            
            //
            // Find voxel in mask containing the "my" voxel's coordinate
            //
            int ijk[3];
            bool voxelInMask = false;;
            if (maskVolume->convertCoordinatesToVoxelIJK(xyz, ijk)) {
               voxelInMask = (maskVolume->getVoxel(ijk, 0) != 0.0);
            }
            
            //
            // If is not in mask
            //
            if (voxelInMask == false) {
               //
               // Turn off 
               //
               setVoxel(i, j, k, 0, 0.0);
            }
         }
      }
   }
}
      
/** 
 * voxels in "this" volume are set to "newVoxelValue" if the corresponding voxel
 * value in "maskVolume" is within the min and max values
 */
void 
VolumeFile::maskWithVolumeThreshold(const VolumeFile* maskVolume,
                                    const float maskMinimumVoxelValue,
                                    const float maskMaximumVoxelValue,
                                    const float newVoxelValue) throw (FileException)
{
   if (maskVolume == NULL) {
      throw FileException("VolumeFile::maskWithVolumeThreshold: Mask volume is invalid.");
   }
   
   //
   // Verify dimensions match
   //
   int myDim[3], maskDim[3];
   getDimensions(myDim);
   maskVolume->getDimensions(maskDim);
   for (int i = 0; i < 3; i++) {
      if (myDim[i] != maskDim[i]) {
         throw FileException("VolumeFile::maskWithVolumeThreshold: Mask volume dimensions to not match.");
      }
   }
   
   //
   // apply the mask
   //
   const int numComponents = getNumberOfComponentsPerVoxel();
   for (int i = 0; i < myDim[0]; i++) {
      for (int j = 0; j < myDim[1]; j++) {
         for (int k = 0; k < myDim[2]; k++) {
            const float maskVoxelValue = maskVolume->getVoxel(i, j, k);
            if ((maskVoxelValue >= maskMinimumVoxelValue) &&
                (maskVoxelValue <= maskMaximumVoxelValue)) {
               for (int m = 0; m < numComponents; m++) {
                  setVoxel(i, j, k, m, newVoxelValue);
               }
            }
         }
      }
   }
}
                                
/**
 * Clear all of the voxels outside the specified extent.
 */
void 
VolumeFile::maskVolume(const int limitsIn[6])
{
   if (DebugControl::getDebugOn()) {
      std::cout << "Extent (maskVolume): " 
                << limitsIn[0] << " to " << limitsIn[1] << ", "
                << limitsIn[2] << " to " << limitsIn[3] << ", "
                << limitsIn[4] << " to " << limitsIn[5] << std::endl;
   }
   
   const int numVoxels = getTotalNumberOfVoxels();
   float* out = new float[numVoxels];
	for (int i = 0; i < numVoxels; i++){
      out[i] = 0;
   }

   int limits[6];
   for (int i = 0; i < 6; i++) {
      limits[i] = limitsIn[i];
   }
   clampVoxelDimension(VOLUME_AXIS_X, limits[0]);
   clampVoxelDimension(VOLUME_AXIS_X, limits[1]);
   clampVoxelDimension(VOLUME_AXIS_Y, limits[2]);
   clampVoxelDimension(VOLUME_AXIS_Y, limits[3]);
   clampVoxelDimension(VOLUME_AXIS_Z, limits[4]);
   clampVoxelDimension(VOLUME_AXIS_Z, limits[5]);

	int cnt = 0;
	for (int k = limits [4]; k < limits [5]; k++){
	   for (int j = limits [2]; j < limits [3]; j++){
	      for (int i = limits [0]; i < limits [1]; i++){
            const int idx = getVoxelDataIndex(i, j, k); 
            cnt++;
	         out[idx] = voxels[idx];
	      }
	   }
	}
   
	for (int i = 0; i < numVoxels; i++){
      voxels[i] = out[i];
   }
   
   if (DebugControl::getDebugOn()) {
      std::cout << "COPIED " << cnt << " of " << numVoxels
                << " voxels: " << 100.0*((float)cnt/(float)(numVoxels))
                << " percent." << std::endl;
   }
   
   delete[] out;

   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 *
 */
void	
VolumeFile::sculptVolume(const int option, const VolumeFile* vol2, 
                            const int numsteps, int seed[3], int limits[6])
{
   clampVoxelDimension(VOLUME_AXIS_X, limits[0]);
   clampVoxelDimension(VOLUME_AXIS_X, limits[1]);
   clampVoxelDimension(VOLUME_AXIS_Y, limits[2]);
   clampVoxelDimension(VOLUME_AXIS_Y, limits[3]);
   clampVoxelDimension(VOLUME_AXIS_Z, limits[4]);
   clampVoxelDimension(VOLUME_AXIS_Z, limits[5]);

   clampVoxelIndex(VOLUME_AXIS_X, seed[0]);
   clampVoxelIndex(VOLUME_AXIS_Y, seed[1]);
   clampVoxelIndex(VOLUME_AXIS_Z, seed[2]);
   
   QString msg;
	if (option == 0)
	   msg = "Sculpt And option";
	else if (option == 1)
	   msg = "Sculpt SeedAnd option";
	else if (option == 2)
	   msg = "Sculpt AndNot option";
	else if (option == 3)
	   msg = "Sculpt SeedAndNot option";
	else	
	   exit (-1);
   if (DebugControl::getDebugOn()) {
      std::cout << msg.toAscii().constData() << std::endl;
      std::cout << "Sculpt Seed: " << seed[0] << ", " << seed[1] << ", " << seed[2] << std::endl;
   }
	
   const int numVoxels = getTotalNumberOfVoxels();
   VolumeFile sculpttemp(*this);
   VolumeFile voltemp(*this);
   VolumeFile voltemp2(*this);
   VolumeFile shelltemp(*this);
	for (int i = 0; i < numVoxels; i++) {
		sculpttemp.voxels[i] = 0.0; 
      voltemp.voxels[i]    = 0.0;
      voltemp2.voxels[i]   = 0.0;
      shelltemp.voxels[i]  = 0.0;
   }

	if ((option == 1) || (option == 3)){
	    const int idx = getVoxelDataIndex(seed[0], seed[1], seed[2]);
	    voltemp.voxels[idx] = voxels[idx];
	}
	else{
	   for (int i = 0; i < numVoxels; i++)
	      voltemp.voxels[i] = voxels[i];
	}
	for (int step = 0; step < numsteps; step++) {
      if (DebugControl::getDebugOn()) {
	      std::cout << "\tStep " << step << " of " << numsteps << std::endl;
      }

	   for (int i = 0; i < numVoxels; i++) {
	      voltemp2.voxels[i] = voltemp.voxels[i];
      }
      
	   //11/8voltemp2.makeShellVolume(&shelltemp, 1, 0);
      shelltemp = voltemp2;
      shelltemp.makeShellVolume(1, 0);
      
	   if ((option == 0) || (option == 1)) { //And
	      for (int i = 0; i < numVoxels; i++) {
            sculpttemp.voxels[i] = shelltemp.voxels[i] * vol2->voxels[i]; 
         }
	      for (int i = 0; i < numVoxels; i++) {
            if ((voltemp.voxels[i] > 0) || (sculpttemp.voxels[i] > 0)) {
               voltemp.voxels[i] = 255;
            }
            else {
               voltemp.voxels[i] = 0;
            }
	      }
	   }
      else { //AndNot
	      for (int i = 0; i < numVoxels; i++) {
            sculpttemp.voxels[i] = shelltemp.voxels[i] * (255-vol2->voxels[i]); 
         }
	      for (int i = 0; i < numVoxels; i++) {
            if ((voltemp.voxels[i] > 0) || (sculpttemp.voxels[i] > 0)) {
               voltemp.voxels[i] = 255;
            }
            else {
               voltemp.voxels[i] = 0;
            }
	      }
      }
	   voltemp.imposeLimits(limits);
	}

	for (int i = 0; i < numVoxels; i++) {
	   voxels[i] = voltemp.voxels[i];
   }

   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 *
 */
void	
VolumeFile::imposeLimits(const int limits[6])
{
   const int numVoxels = getTotalNumberOfVoxels();
   float *out = new float[numVoxels];
	for (int i = 0; i < numVoxels; i++)	{
      out[i] = 0;
   }
   
   for (int k = limits[4]; k < limits[5]; k++){
      for (int j = limits[2]; j < limits[3]; j++){
         for (int i = limits[0]; i < limits[1]; i++){
            const int idx = getVoxelDataIndex(i, j, k);
            out[idx] = voxels[idx];
         }
      }
   }
	for (int i = 0; i < numVoxels; i++) {
	   voxels[i] = out[i];
   }
	delete[] out;

   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 *
 */
void	
VolumeFile::makeShellVolume(const int Ndilation, 
                               const int Nerosion)
{
   const int numVoxels = getTotalNumberOfVoxels();
   VolumeFile vol2(*this);
   VolumeFile vol3(*this);
	for (int i = 0; i < numVoxels; i++) {
	   vol2.voxels[i] = 0;
   }
   
   int localNeighsOffset[26];
	for (int i = 0; i < 26; i++){
	   const int ii = localNeighbors[i][0];
	   const int jj = localNeighbors[i][1];
	   const int kk = localNeighbors[i][2];
	   localNeighsOffset[i] = ii + (jj * dimensions[0]) + (kk * dimensions[0] * dimensions[1]); 
	}

   for (int i = 0; i < numVoxels; i++) {
     vol3.voxels[i] = voxels[i];
   }

   if (Nerosion > 0) {
      for (int j = 0; j < Nerosion; j++) {
         if ((j % 2) == 0) {
            const int cnt = stripBorderVoxels(localNeighsOffset, 6, &vol2);
            if (DebugControl::getDebugOn()) {
               std::cout << "\tErode 6 neighs " << j << "; " << cnt << " voxels..."
                         << std::endl;
            }
         }
         else {
            const int cnt = stripBorderVoxels(localNeighsOffset, 26, &vol2);
            if (DebugControl::getDebugOn()) {
               std::cout << "\tErode 26 neighs " << j << "; " << cnt << " voxels..."
                         << std::endl;
            }
         }
      }
   }

   for (int i = 0; i < numVoxels; i++) {
      voxels[i] = vol3.voxels[i];
   }

	int cnt = 0;
	if (Ndilation > 0){
	   for (int i = 0; i < numVoxels; i++)
	      voxels[i] = 255 - voxels[i];	
           for (int i = 0; i < Ndilation; i++){
              if ((i % 2) == 0){
                 cnt = stripBorderVoxels(localNeighsOffset, 6, &vol2);
                 if (DebugControl::getDebugOn()) {
                    std::cout << "\tDilate 6 neighs " << i << "; " << cnt << " voxels..."
                              << std::endl;
                 }
              }
              else {
                 cnt = stripBorderVoxels(localNeighsOffset, 26, &vol2);
                 if (DebugControl::getDebugOn()) {
                    std::cout << "\tDilate 26 neighs " << i << "; " << cnt << " voxels..."
                              << std::endl;
                 }
              }
           }
	   for (int i = 0; i < numVoxels; i++) {
	      voxels[i] = 255 - voxels[i];	
      }
	}

	for (int i = 0; i < numVoxels; i++) {
	   voxels[i] = vol2.voxels[i];	
   }

   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 *
 */
int
VolumeFile::stripBorderVoxels(const int neighborOffsets[],
		                           int numNeighs, 
                                 VolumeFile *vol2)
{
   const int numVoxels = getTotalNumberOfVoxels();
   int cnt = 0;
   const int slices = dimensions[2] - 1;
   const int rows   = dimensions[1] - 1;
   const int columns = dimensions[0] - 1;
   for (int k = 1; k < slices; k++) {
	   if ((k % 50) == 0) {
         if (DebugControl::getDebugOn()) {
            std::cout << "\tslice " << k << std::endl;
         }
      }
      for (int j = 1; j < rows; j++) {
         for (int i = 1; i < columns; i++) {
            int idx = getVoxelDataIndex(i, j, k);
            if (voxels[idx] == 255) {
               int neighs[26];
               computeNeighbors(idx, neighborOffsets, numNeighs, neighs);
               int n = 0;
               bool done = false;
               while ((n < numNeighs) && (done == false)) {
                  if (voxels[neighs[n]] == 0) {
                     done = true; // Found border voxel
                     voxels[idx] = 127;
                     cnt++; 
                  }
                  n++;
               }
	         }
	      }
	   }
	}

 	for (int idx = 0; idx < numVoxels; idx++){
	   if (voxels[idx] == 127){
	      vol2->voxels[idx] = 255;	
	      voxels[idx] = 0;	
	   }
	}
   
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;

	return(cnt);
}

/**
 * Get the neighbors for a node given one-dimensional offsets.
 */
int	
VolumeFile::computeNeighbors(const int idx, 
                                 const int neighborOffsets[], 
                                 const int numNeighs,
                                 int neighsOut[]) const
{
   const int numVoxels = getTotalNumberOfVoxels();
   int cnt = 0;
	for (int nn = 0; nn < numNeighs; nn++) {
      const int idx2 = idx + neighborOffsets[nn];
	   if ((idx2 >= 0) && (idx2 < numVoxels)) {
	      neighsOut[cnt] = idx2; 
      }
	   else {
	      neighsOut[cnt] = 0; 
      }
	   cnt++;
	}
   
	return (cnt);
}

/**
 * ???
 */
void
VolumeFile::makePlane(const float xslope, 
                         const float xoffset, 
                         const float yslope, 
                         const float yoffset, 
                         const float zslope, 
                         const float zoffset, 
                         const float offset, 
                         const float thickness)
{
	if (DebugControl::getDebugOn()) {
      std::cout << "MakePlane " << xslope << "x + " << yslope << "y + " 
                << zslope << "z - " << offset << " < " << thickness << std::endl;
   }
   
	for (int k = 0; k < dimensions[2]; k++){
	   for (int j = 0; j < dimensions[1]; j++){
	      for (int i = 0; i < dimensions[0]; i++){
	         float result = 
               (xslope * (i - xoffset)) + (yslope * (j - yoffset)) + (zslope * (k - zoffset));
	         result = result - offset;
            if (result < 0) {
               result = -result;
            }
	         if (result < thickness) {
	            const int idx = getVoxelDataIndex(i, j, k); 
               voxels[idx] = 255.0;
	         }
	      }
	   }
	}
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 * dilation and erosion.
 */
void 
VolumeFile::doVolMorphOpsWithinMask(const int extent[6], const int nDilation, const int nErosion) 
{
   VolumeFile vf(*this);
   //vf.maskVolume(extent);
   vf.doVolMorphOps(nDilation, nErosion);
   unsigned char rgb[4];
   copySubVolume(&vf, extent, rgb, rgb);
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 * dilation and erosion.
 */
void 
VolumeFile::doVolMorphOps(const int nDilation, const int nErosion) 
{
   if (DebugControl::getDebugOn()) {
   	std::cout << nDilation << " dilation iters, "
                << nErosion << " erosion iters" << std::endl;
   }
   
   int localNeighsOffset[26];
	for (int i = 0; i < 26; i++) {
	   const int ii = localNeighbors[i][0];
	   const int jj = localNeighbors[i][1];
	   const int kk = localNeighbors[i][2];
	   localNeighsOffset[i] = ii + (jj * dimensions[0]) + (kk * dimensions[0] * dimensions[1]); 
	}

   const int numVoxels = getTotalNumberOfVoxels();
	int cnt = 0;
	if (nDilation > 0){
	   for (int i = 0; i < numVoxels; i++) {
	      voxels[i] = 255.0 - voxels[i];	
      }
      for (int i = 0; i < nDilation; i++){
         if ((i % 2) == 0){
            cnt = stripBorderVoxels(localNeighsOffset, 6);
            if (DebugControl::getDebugOn()) {
               std::cout << "\tDilate 6 neighs " << i << "; " << cnt << " voxels...\n" << std::endl;
            }
         }
         else{
            cnt = stripBorderVoxels(localNeighsOffset, 26);
            if (DebugControl::getDebugOn()) {
               std::cout << "\tDilate 26 neighs " << i << "; " << cnt << " voxels...\n" << std::endl;
            }
         }
      }
	   for (int i = 0; i < numVoxels; i++)
	      voxels[i] = 255.0 - voxels[i];	
	}

	cnt = 0;
   if (nErosion > 0){
      for (int j = 0; j < nErosion; j++) {
         if ((j % 2) == 0) {
            cnt = stripBorderVoxels(localNeighsOffset, 6);
            if (DebugControl::getDebugOn()) {
               std::cout << "\tErode 6 neighs " << j << "; " << cnt << " voxels...\n" << std::endl;
            }
         }
         else {
            cnt = stripBorderVoxels(localNeighsOffset, 26);
            if (DebugControl::getDebugOn()) {
               std::cout << "\tErode 26 neighs " << j << "; " << cnt << " voxels...\n" << std::endl;
            }
         }
      }
   }
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 * ??
 */
int
VolumeFile::stripBorderVoxels(const int neighborOffsets[], 
                                 const int numNeighs)
{
	/* Mark border voxels */
   if (DebugControl::getDebugOn()) {
   	std::cout << "StripBorderVoxels ..." << std::endl;
   }
   
   const int nslices = dimensions[2];
   const int nrow    = dimensions[1];
   const int ncol    = dimensions[0];
   
   int cnt = 0;
   for (int k = 1; k < nslices-1; k++){
      if ((k % 20) == 0) {
         if (DebugControl::getDebugOn()) {
            std::cout << "\tslice " << k << std::endl;
         }
      }
      for (int j = 1; j < nrow-1; j++) {
         for (int i = 1; i < ncol-1; i++) {
            const int idx = getVoxelDataIndex(i, j, k);
	         if (voxels[idx] == 255.0) {
               int neighs[26];
	            computeNeighbors(idx, neighborOffsets,
                                 numNeighs, neighs);
	            int n = 0;
	            bool done = false;
	            while ((n < numNeighs) && (done == false)) {
	               if (voxels[neighs[n]] == 0){
                     done = true; // Found border voxel
                     voxels[idx] = 127.0;
                     cnt++; 
	               }
                  n++;
	            }
	         }
	      }
	   }
	}

   const int numVoxels = getTotalNumberOfVoxels();
 	for (int idx = 0; idx < numVoxels; idx++) {
	   if (voxels[idx] == 127.0) {
	      voxels[idx] = 0.0;	
      }
   }
   
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
	return (cnt);
}

/**
 * Classify intensities
 */
void	
VolumeFile::classifyIntensities(const float mean, 
                                   const float low, 
                                   const float high, 
                                   const float signum)
{
	//%printf ("ClassifyIntensities: mean %f, low %f, high %f, signum %f\n",
	//%	mean, low, high, signum);
   if (DebugControl::getDebugOn()) {
      std::cout << "ClassifyIntensities: mean " << mean 
                << ", low " << low 
                << ", high " << high
                << ", signum " << signum
                << std::endl;
   }
            
   const int numVoxels = getTotalNumberOfVoxels();
	for (int i = 0; i < numVoxels; i++){
      float sigma = 0.0;
      if (voxels[i] <= mean) {
         sigma = (mean-low) / signum;
      }
      else {
         sigma = (high-mean) / signum;
      }
      
      const float t1 = -((voxels[i] - mean) * (voxels[i] - mean));
      voxels[i] = exp(t1 / (2.0 * sigma * sigma));
	}
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
   
   if (DebugControl::getDebugOn()) {
      for (float f = 0.0; f <= 255.0; f += 1.0) {
         float sigma = 0.0;
         if (f <= mean) {
            sigma = (mean-low) / signum;
         }
         else {
            sigma = (high-mean) / signum;
         }
         
         const float t1 = -((f - mean) * (f - mean));
         const float f2 = exp(t1 / (2.0 * sigma * sigma));     
         std::cout << "   " << f << " => " << f2 << std::endl;    
      }
   }
}

/**
 * get the number of non-zero voxels.
 */
int 
VolumeFile::getNumberOfNonZeroVoxels() const
{
   int cnt = 0;
   
   const int num = getTotalNumberOfVoxels();
   const int ncomp = getNumberOfComponentsPerVoxel();
   for (int i = 0; i < num; i++) {
      for (int j = 0; j < ncomp; j++) {
         const int offset = i * ncomp + j;
         if (voxels[offset] != 0.0) {
            cnt++;
            break;
         }
      }
   }
   
   return cnt;
}

/**
 *
 */
void 
VolumeFile::blur()
{
	float	lpf_filter [5];

  lpf_filter [0] = 1.0/16.0;
  lpf_filter [1] = 1.0/4.0;
  lpf_filter [2] = 3.0/8.0;
  lpf_filter [3] = 1.0/4.0;
  lpf_filter [4] = 1.0/16.0;

   seperableConvolve(dimensions[0], dimensions[1], dimensions[2], voxels, lpf_filter);
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}

/**
 *
 */
void	
VolumeFile::seperableConvolve(int ncol, int nrow, int nslices, 
	                            	float *volume, float *filter)
{
	int	isize = ncol*nrow;

	//printf ("\tSeperableConvolve filtersize=%d\n", FSIZE); 
	float* Result = new float[ncol*nrow*nslices];
	float* TempSpace = new float[ncol*nrow*nslices];

	float* voxel = volume; 
	float* tempResult = Result;
	oneDimConvolve (voxel, tempResult, filter, 0, 1, ncol, nrow, nslices);	
	//printf ("%d %f %f\n", idx, voxel[idx], tempResult[idx]);

	voxel = Result; 
	tempResult = TempSpace;
	oneDimConvolve (voxel, tempResult, filter, 1, ncol, ncol, nrow, nslices);	
	//printf ("%d %f %f\n", idx, voxel[idx], tempResult[idx]);

	voxel = TempSpace; 
	tempResult = volume;
	oneDimConvolve (voxel, tempResult, filter, 2, isize, ncol, nrow, nslices);
	//printf ("%d %f %f\n", idx, voxel[idx], tempResult[idx]);

	delete[] Result; 
	delete[] TempSpace; 
	return;
}

/**
 *
 */
void	
VolumeFile::oneDimConvolve (float *voxel, float *tempResult, float *filter, 
                const int dim, const int inc,
		          const int ncol, const int nrow, const int nslices)
{
   const int FILTSIZE = 5;
	float	p[FILTSIZE];

	int cnt = 0;
	for (int k = 0; k < nslices; k++){
	   for (int j = 0; j < nrow; j++){
	      for (int i = 0; i < ncol; i++){
             float* ip = voxel;	 
             if (dim == 0){
                if (i == 0){
                   p[0] = *ip;
                   p[1] = *ip;
                   p[2] = *ip;
                   p[3] = *(ip+inc);
                   p[4] = *(ip+inc+inc);
                        }
                else if (i == 1) {
                   p[0] = *(ip-inc);
                   p[1] = *(ip-inc);
                   p[2] = *ip;
                   p[3] = *(ip+inc);
                   p[4] = *(ip+inc+inc);
                }
                else if (i == ncol-2) {
                   p[0] = *(ip-inc-inc);
                   p[1] = *(ip-inc);
                   p[2] = *ip;
                   p[3] = *(ip+inc);
                   p[4] = *(ip+inc);
                }
                else if (i == ncol-1) {
                   p[0] = *(ip-inc-inc);
                   p[1] = *(ip-inc);
                   p[2] = *ip;
                   p[3] = *ip;
                   p[4] = *ip;
                }
                else {
                   p[0] = *(ip-inc-inc);
                   p[1] = *(ip-inc);
                   p[2] = *ip;
                   p[3] = *(ip+inc);
                   p[4] = *(ip+inc+inc);
                }
             }
             else if (dim == 1) {
                if (j == 0){
                   p[0] = *ip;
                   p[1] = *ip;
                   p[2] = *ip;
                   p[3] = *(ip+inc);
                   p[4] = *(ip+inc+inc);
                        }
                else if (j == 1) {
                   p[0] = *(ip-inc);
                   p[1] = *(ip-inc);
                   p[2] = *ip;
                   p[3] = *(ip+inc);
                   p[4] = *(ip+inc+inc);
                }
                else if (j == nrow-2) {
                   p[0] = *(ip-inc-inc);
                   p[1] = *(ip-inc);
                   p[2] = *ip;
                   p[3] = *(ip+inc);
                   p[4] = *(ip+inc);
                }
                else if (j == nrow-1) {
                   p[0] = *(ip-inc-inc);
                   p[1] = *(ip-inc);
                   p[2] = *ip;
                   p[3] = *ip;
                   p[4] = *ip;
                }
	             else {
                   p[0] = *(ip-inc-inc);
                   p[1] = *(ip-inc);
                   p[2] = *ip;
                   p[3] = *(ip+inc);
                   p[4] = *(ip+inc+inc);
                }
             }
             else if (dim == 2) {
                if (k == 0){
                   p[0] = *ip;
                   p[1] = *ip;
                   p[2] = *ip;
                   p[3] = *(ip+inc);
                   p[4] = *(ip+inc+inc);
                        }
                else if (k == 1) {
                   p[0] = *(ip-inc);
                   p[1] = *(ip-inc);
                   p[2] = *ip;
                   p[3] = *(ip+inc);
                   p[4] = *(ip+inc+inc);
                }
                else if (k == nslices-2) {
                   p[0] = *(ip-inc-inc);
                   p[1] = *(ip-inc);
                   p[2] = *ip;
                   p[3] = *(ip+inc);
                   p[4] = *(ip+inc);
                }
                else if (k == nslices-1) {
                   p[0] = *(ip-inc-inc);
                   p[1] = *(ip-inc);
                   p[2] = *ip;
                   p[3] = *ip;
                   p[4] = *ip;
                }
                else {
                   p[0] = *(ip-inc-inc);
                   p[1] = *(ip-inc);
                   p[2] = *ip;
                   p[3] = *(ip+inc);
                   p[4] = *(ip+inc+inc);
                }
             }
             *tempResult = 0;
             for (int n = 0; n < FILTSIZE; n++){
               *tempResult += filter[n]*p[n];
             }
             tempResult++;
             voxel++;
             cnt++;
	      }
	   }
	}
}

void	
VolumeFile::breadthFirstFloodFill(const VoxelIJK& seed, 
                                     const float valueToFind, 
                                     VolumeFile *markVolume,
                                     const float markValue,
                                     const bool onlyDoUnmarkedVoxels) const
{
   //
   // See if this voxel has already been searched
   //
   if (onlyDoUnmarkedVoxels) {
      if (markVolume->getVoxel(seed) == markValue) {
         return;
      }
   }

   //
   // Initialize to the staring voxel
   //
   std::stack<VoxelIJK> st;
   st.push(VoxelIJK(seed));
    
   //
   // Keep track of voxels that have been searched
   //
   const int numVoxels = getTotalNumberOfVoxels();
   if (numVoxels <= 0) {
      return;
   }
   int* voxelSearched = new int[numVoxels];
   for (int i = 0; i < numVoxels; i++) {
      if (getVoxelWithFlatIndex(i) == valueToFind) {
         voxelSearched[i] = false;
      }
      else {
         voxelSearched[i] = true;
      }
      if (onlyDoUnmarkedVoxels) {
         if (markVolume->getVoxelWithFlatIndex(i) == markValue) {
            voxelSearched[i] = true;
         }
      }
   }
   
   //
   // While there are voxels to process
   //
   while (st.empty() == false) {
      //
      // Get the next voxel to process
      //
      const VoxelIJK v = st.top();
      st.pop();
      int i, j, k;
      v.getIJK(i, j, k);

      //
      // If the voxel has valid indices
      //
      if ((i >= 0) && (i < dimensions[0]) &&
          (j >= 0) && (j < dimensions[1]) &&
          (k >= 0) && (k < dimensions[2])) {
         const int ijk[3] = { i, j, k };
         const float currentValue = getVoxel(ijk);
         
         //
         // See if voxel has been searched
         //
         const int flatIndex = getVoxelDataIndex(ijk);
         if (voxelSearched[flatIndex] == false) {
            voxelSearched[flatIndex] = true;
         
            //
            // See if voxel has proper value for operation
            //
            if (currentValue == valueToFind) {
               //
               // Update the voxels value
               //
               markVolume->setVoxel(ijk, 0, markValue);
               
               //
               // Add neighboring voxels for search
               //
               std::vector<VoxelIJK> newVoxels;
               newVoxels.reserve(6);
               newVoxels.push_back(VoxelIJK(i - 1, j, k));
               newVoxels.push_back(VoxelIJK(i + 1, j, k));
               newVoxels.push_back(VoxelIJK(i, j - 1, k));
               newVoxels.push_back(VoxelIJK(i, j + 1, k));
               newVoxels.push_back(VoxelIJK(i, j, k - 1));
               newVoxels.push_back(VoxelIJK(i, j, k + 1));
               
               for (unsigned int i = 0; i < 6; i++) {
                  const VoxelIJK& v= newVoxels[i];
                  if (getVoxelIndexValid(v)) {
                     const int flatIndex = getVoxelDataIndex(v);
                     if (voxelSearched[flatIndex] == false) {
                        st.push(v);
                     }
                  }
               }
            }
         }
      }
   }
   
   delete[] voxelSearched;
}

/**
 * Fill internal cavities in a segmentation volume.
 */
void 
VolumeFile::fillSegmentationCavities(const VolumeFile* markVolumeIn)
{
   //
   // Volume to keep track of unset voxels
   //
   VolumeFile* markVolume = NULL;
   if (markVolumeIn != NULL) {
      markVolume = new VolumeFile(*markVolumeIn);
   }
   else {
      markVolume = new VolumeFile(*this);
      markVolume->setAllVoxels(0.0);
   }
   
   //
   // Find voxels along edges that are not set
   //
   const int imax = dimensions[0] - 1;
   const int jmax = dimensions[1] - 1;
   const int kmax = dimensions[2] - 1;
   for (int k = 0; k <= kmax; k++) {
      for (int j = 0; j <= jmax; j++) {
         for (int i = 0; i <= imax; i++) {
            //
            // is this an edge voxel
            //
            if ((i == 0) || (i == imax) ||
                (j == 0) || (j == jmax) ||
                (k == 0) || (k == kmax)) {
               if (markVolume->getVoxel(i, j, k) == 0.0) {
                  if (getVoxel(i, j, k) == 0.0) {
                     VoxelIJK seed(i, j, k);
                     breadthFirstFloodFill(seed, 
                                           0.0,
                                           markVolume,
                                           1.0,
                                           true);
                  }
               }
            }
         }
      }
   }
   
   //
   // If there are any other voxels not marked and not 255 set them because
   // they must be cavities in the segmentation
   //
   const int numVoxels = getTotalNumberOfVoxels();
   for (int i = 0; i < numVoxels; i++) {
      if ((voxels[i] == 0.0) &&
          (markVolume->voxels[i] == 0.0)) {
         voxels[i] = 255.0;
      }
   }
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
   
   setVoxelColoringInvalid();
   
   delete markVolume;
}
      
/**
 * Fill internal cavities in a segmentation volume but only one slice.
 */
void 
VolumeFile::fillSegmentationCavitiesInSingleSlice(const VOLUME_AXIS axis,
                                                  const int sliceNumber)
{
   //
   // Create a mask volume with all voxels masked
   //
   VolumeFile maskVolume(*this);
   maskVolume.setAllVoxels(1.0);
   
   //
   // Determine voxels that should be unmasked
   //
   int iMin = 0;
   int iMax = dimensions[0] - 1;
   int jMin = 0;
   int jMax = dimensions[1] - 1;
   int kMin = 0;
   int kMax = dimensions[2] - 1;
   switch (axis) {
      case VOLUME_AXIS_X:         
         iMin = sliceNumber;
         iMax = sliceNumber;
         break;
      case VOLUME_AXIS_Y:
         jMin = sliceNumber;
         jMax = sliceNumber;
         break;
      case VOLUME_AXIS_Z:
         kMin = sliceNumber;
         kMax = sliceNumber;
         break;
      case VOLUME_AXIS_ALL:
      case VOLUME_AXIS_OBLIQUE:
      case VOLUME_AXIS_OBLIQUE_X:
      case VOLUME_AXIS_OBLIQUE_Y:
      case VOLUME_AXIS_OBLIQUE_Z:
      case VOLUME_AXIS_OBLIQUE_ALL:
      case VOLUME_AXIS_UNKNOWN:
         break;
   }
   for (int i = iMin; i <= iMax; i++) {
      for (int j = jMin; j <= jMax; j++) {
         for (int k = kMin; k <= kMax; k++) {
            maskVolume.setVoxel(i, j, k, 0, 0);
         }
      }
   }
   
   //
   // cavites in the slice that is not masked
   //
   fillSegmentationCavities(&maskVolume);
}

/**
 * invert the voxels in a segmentation volume.
 */
void 
VolumeFile::invertSegmentationVoxels()
{
   //
   // Voxels below 0.001 become 255, above 0.001 become 0
   //
   inverseThresholdVolume(0.001);
}
      
/**
 * get the number of objects in a segmentation volume.
 */
int 
VolumeFile::getNumberOfSegmentationObjects() const
{
   VoxelIJK seed;
   return findBiggestObject(255.0, 255.0, seed);
}

/**
 * get the number of objects in a segmentation volume.
 */
int 
VolumeFile::getNumberOfSegmentationObjectsSubVolume(const int extent[6]) const
{
   VolumeFile volumeCopy(*this);
   volumeCopy.maskVolume(extent);
   return volumeCopy.getNumberOfSegmentationObjects();
}

/**
 * get the number of cavities in a segmentation volume.
 */
int 
VolumeFile::getNumberOfSegmentationCavitiesSubVolume(const int extent[6]) const
{
   VolumeFile volumeCopy(*this);
   volumeCopy.maskVolume(extent);
   return volumeCopy.getNumberOfSegmentationCavities();
}

/**
 * get the number of cavities in a segmentation volume.
 */
int 
VolumeFile::getNumberOfSegmentationCavities() const
{
   //
   // Make a copy of "this" volume
   //
   VolumeFile volumeCopy(*this);
   
   //
   // Flood fill around using all exterior voxels
   //
   const int maxDimX = dimensions[0] - 1;
   const int maxDimY = dimensions[1] - 1;
   const int maxDimZ = dimensions[2] - 1;
   const unsigned char rgb4[4] = { 255, 0, 0, 0 };
   for (int i = 0; i <= maxDimX; i++) {
      for (int j = 0; j <= maxDimY; j++) {
         for (int k = 0; k <= maxDimZ; k++) {
            if ((i == 0) || (i == maxDimX) ||
                (j == 0) || (j == maxDimY) ||
                (k == 0) || (k == maxDimZ)) {
               if (getVoxel(i, j, k, 0) == 0.0) {
                  
                  const int ijk[3] = { i, j, k };
                  volumeCopy.floodFillAndRemoveConnected(SEGMENTATION_OPERATION_FLOOD_FILL_3D,
                                              VOLUME_AXIS_Z,
                                              ijk,
                                              255.0,
                                              rgb4);
               }
            }
         }
      }
   }
   
   //
   // Invert the voxels
   //
   volumeCopy.invertSegmentationVoxels();
   
   //
   // Count the objects
   //
   return volumeCopy.getNumberOfSegmentationObjects();
}

/**
 * Generate a Cerebral Hull volume from "this" segmentation volume.
 */
void 
VolumeFile::createCerebralHullVolume(VolumeFile& cerebralHullOut) const
{
   //
   // Copy "this" volume to the cerebral hull volume
   //
   cerebralHullOut = *this;
   
   //
   // Change file's name and description
   //
   cerebralHullOut.setFileWriteType(getFileWriteType());
   cerebralHullOut.makeDefaultFileName("CerebralHull");
   cerebralHullOut.setDescriptiveLabel("CerebralHull");
   
   //
   // dialate and erode to fill in sulci
   //
   //cerebralHullOut.doVolMorphOps(6, 6);

   //
   // dilate six iterations
   //
   cerebralHullOut.doVolMorphOps(6, 0);

   //
   // fill cavities that may occur in deep sulci such as around insula
   //
   cerebralHullOut.fillSegmentationCavities();
   
   //
   // erode six iterations
   //
   cerebralHullOut.doVolMorphOps(0, 6);

   //
   // OR cerebral hull together with "this" segmentation
   //
   performMathematicalOperation(VOLUME_MATH_OPERATION_OR,
                                this,
                                &cerebralHullOut,
                                NULL,
                                &cerebralHullOut);
}

/**
 * get the euler data for this volume but limit to a subvolume.
 */
void 
VolumeFile::getEulerCountsForSegmentationSubVolume(int& numberOfObjects,
                                         int& numberOfCavities,
                                         int& numberOfHoles,
                                         int& eulerCount,
                                         const int extent[6]) const
{
   numberOfObjects  = getNumberOfSegmentationObjectsSubVolume(extent);
   numberOfCavities = getNumberOfSegmentationCavitiesSubVolume(extent);
   eulerCount       = getEulerNumberForSegmentationSubVolume(extent);
   numberOfHoles    = numberOfObjects + numberOfCavities - eulerCount;
}                         

/**
 * get the euler data for this volume.
 */
void 
VolumeFile::getEulerCountsForSegmentationVolume(int& numberOfObjects,
                                                int& numberOfCavities,
                                                int& numberOfHoles,
                                                int& eulerCount) const
{
   numberOfObjects  = getNumberOfSegmentationObjects();
   numberOfCavities = getNumberOfSegmentationCavities();
   eulerCount       = getEulerNumberForSegmentationVolume();
   numberOfHoles    = numberOfObjects + numberOfCavities - eulerCount;
}                         

/**
 * determine the euler number of a subvolume of segmentation volume.
 */
int 
VolumeFile::getEulerNumberForSegmentationSubVolume(const int extentIn[6]) const
{
/*
   int extent[6] = {
      extentIn[0],
      extentIn[1],
      extentIn[2],
      extentIn[3],
      extentIn[4],
      extentIn[5]
   };
   
   //
   // Limit extent
   //
   clampVoxelIndex(VOLUME_AXIS_X, extent[0]);
   clampVoxelIndex(VOLUME_AXIS_X, extent[1]);
   clampVoxelIndex(VOLUME_AXIS_Y, extent[2]);
   clampVoxelIndex(VOLUME_AXIS_Y, extent[3]);
   clampVoxelIndex(VOLUME_AXIS_Z, extent[4]);
   clampVoxelIndex(VOLUME_AXIS_Z, extent[5]);
   
   //
   // add padding around the volume
   //
   extent[0]--;
   extent[1]++;
   extent[2]--;
   extent[3]++;
   extent[4]--;
   extent[5]++;
   
   //
   // Copy the volume
   //
   VolumeFile volumeCopy(*this);
   
   //
   // Resize the volume
   //
   volumeCopy.resize(extent);
   
   //
   // Zero out the voxels along the edges
   //
   int dim[3];
   volumeCopy.getDimensions(dim);
   volumeCopy.setAllVoxelsInSlice(VOLUME_AXIS_X, 0, 0.0);
   volumeCopy.setAllVoxelsInSlice(VOLUME_AXIS_X, dim[0] - 1, 0.0);
   volumeCopy.setAllVoxelsInSlice(VOLUME_AXIS_Y, 0, 0.0);
   volumeCopy.setAllVoxelsInSlice(VOLUME_AXIS_Y, dim[1] - 1, 0.0);
   volumeCopy.setAllVoxelsInSlice(VOLUME_AXIS_Z, 0, 0.0);
   volumeCopy.setAllVoxelsInSlice(VOLUME_AXIS_Z, dim[2] - 1, 0.0);
*/   
   //
   // Get the euler number for the volume
   //
   VolumeFile volumeCopy(*this);
   volumeCopy.maskVolume(extentIn);
   return volumeCopy.getEulerNumberForSegmentationVolume();
}
      
/**
 * determine the euler number of a segmentation volume.
 */
int 
VolumeFile::getEulerNumberForSegmentationVolume() const
{
   if (eulerTableValid == false) {
      eulerTableValid = true;
      createEulerTable();
   }
   float eulerSum = 0;
   
   const int d[3] = { 1, 1, 1 };
   for (int k = 0; k < dimensions[2]; k++) {
      for (int j = 0; j < dimensions[1]; j++) {
         for (int i = 0; i < dimensions[0]; i++) {
            const int val = computeEulerOctant(i, j, k, d);
            eulerSum += eulerTable[val];
         }
      }
   }
   
   return static_cast<int>(eulerSum);
}  

/**
 * create the euler table.
 */
void 
VolumeFile::createEulerTable()
{
   for (int n = 0; n < 256; n++) {
      int corners[8];
      for (int i = 0; i < 8; i++) {
         const int t1 = n >> (i + 1 - 1);
         const int t2 = t1 & 1;
         corners[7 - i] = t2;
      }
      
      float vertices = 0.0;
      for (int k = 0; k < 8; k++) {
         if (corners[k] == 1) {
            vertices += 1.0;
         }
      }
      
      float edges = 0.0;
      if ((corners[0] == 1) && (corners[1] == 1)) {
         edges += 1.0;
      }
      if ((corners[0] == 1) && (corners[2] == 1)) {
         edges += 1.0;
      }
      if ((corners[0] == 1) && (corners[4] == 1)) {
         edges += 1.0;
      }
      if ((corners[2] == 1) && (corners[3] == 1)) {
         edges += 1.0;
      }
      if ((corners[3] == 1) && (corners[7] == 1)) {
         edges += 1.0;
      }
      if ((corners[6] == 1) && (corners[7] == 1)) {
         edges += 1.0;
      }
      if ((corners[2] == 1) && (corners[6] == 1)) {
         edges += 1.0;
      }
      if ((corners[1] == 1) && (corners[5] == 1)) {
         edges += 1.0;
      }
      if ((corners[5] == 1) && (corners[7] == 1)) {
         edges += 1.0;
      }
      if ((corners[4] == 1) && (corners[5] == 1)) {
         edges += 1.0;
      }
      if ((corners[4] == 1) && (corners[6] == 1)) {
         edges += 1.0;
      }
      if ((corners[1] == 1) && (corners[3] == 1)) {
         edges += 1.0;
      }
      
      float faces = 0.0;
      if ((corners[0] == 1) && (corners[2] == 1) &&
          (corners[4] == 1) && (corners[6] == 1)) {
         faces += 1.0;
      }
      if ((corners[0] == 1) && (corners[1] == 1) &&
          (corners[2] == 1) && (corners[3] == 1)) {
         faces += 1.0;
      }
      if ((corners[1] == 1) && (corners[5] == 1) &&
          (corners[3] == 1) && (corners[7] == 1)) {
         faces += 1.0;
      }
      if ((corners[4] == 1) && (corners[5] == 1) &&
          (corners[6] == 1) && (corners[7] == 1)) {
         faces += 1.0;
      }
      if ((corners[2] == 1) && (corners[6] == 1) &&
          (corners[7] == 1) && (corners[3] == 1)) {
         faces += 1.0;
      }
      if ((corners[0] == 1) && (corners[1] == 1) &&
          (corners[4] == 1) && (corners[5] == 1)) {
         faces += 1.0;
      }
      
      float oct = 0.0;
      if (vertices == 8.0) {
         oct = 1.0;
      }
      eulerTable[n] = (vertices / 8.0)
                    - (edges / 4.0)
                    + (faces / 2.0)
                    - oct;
   }
}
      
/**
 * Compute the euler value for a voxel.
 */
int	
VolumeFile::computeEulerOctant(const int i, const int j, const int k, const int D[3]) const
{
	if ((i+D[0] >= 0) && (i+D[0] < dimensions[0]) &&
	    (j+D[1] >= 0) && (j+D[1] < dimensions[1]) &&
	    (k+D[2] >= 0) && (k+D[2] < dimensions[2])) {
     int vals[8];
	  vals[7] = getVoxelDataIndex(i, j, k); //ComputeIndex (i, 		j, 		k, ncol, nrow);

	  const int offsetI = D[0];
	  const int offsetJ = D[1] * dimensions[0];
	  const int offsetK = D[2] * dimensions[0] * dimensions[1];
     int newvals[8];
	  newvals[7] = vals[7];
	  newvals[6] = vals[7] + offsetI; 
	  newvals[5] = vals[7] + offsetJ;
	  newvals[4] = vals[7] + offsetI + offsetJ;
	  newvals[3] = vals[7] + offsetK;
	  newvals[2] = vals[7] + offsetI + offsetK; 
	  newvals[1] = vals[7] + offsetJ + offsetK;
	  newvals[0] = vals[7] + offsetI + offsetJ + offsetK;

     int octant[8];
	  octant[7] = static_cast<int>(voxels[newvals[7]]);
	  octant[6] = static_cast<int>(voxels[newvals[6]]);
	  octant[5] = static_cast<int>(voxels[newvals[5]]);
	  octant[4] = static_cast<int>(voxels[newvals[4]]);
	  octant[3] = static_cast<int>(voxels[newvals[3]]);
	  octant[2] = static_cast<int>(voxels[newvals[2]]);
	  octant[1] = static_cast<int>(voxels[newvals[1]]);
	  octant[0] = static_cast<int>(voxels[newvals[0]]);

     int ans2 = 0;
     for (int i = 0; i < 8; i++){
        if (octant[i] != 0.0){
            if (i == 0) {
               ans2 += 1;
            }
            else {
               ans2 += 2 << (i-1);
            }
        } 
     }

	  return ans2;
	}
   
	return 0;
}

/**
 * get topology information by generating a surface.
 * Cavities are filled prior to euler count.
 */
void 
VolumeFile::getSegmentationTopologyInformation(int& numberOfObjects,
                                               int& numberOfCavities,
                                               int& numberOfHoles,
                                               int& eulerCount) const throw (FileException)
{
   //
   // Copy this volume
   //
   VolumeFile volumeCopy(*this);
   
   //
   // Remove islands and fill cavities
   //
   volumeCopy.fillSegmentationCavities();
   
   //
   // Convert to structured points
   //
   vtkStructuredPoints* sp = volumeCopy.convertToVtkStructuredPoints();

   //
   // Shrinker - does this actually do anything ?
   //
   vtkImageShrink3D* shrinker = vtkImageShrink3D::New();
#ifdef HAVE_VTK6
   shrinker->SetInputData(sp);
#else
   shrinker->SetInput(sp);
#endif
   shrinker->SetShrinkFactors(1, 1, 1);
   shrinker->AveragingOn();

   //
   // Gaussian smooth the volume
   //  
   vtkImageGaussianSmooth* gaussian = vtkImageGaussianSmooth::New();
   gaussian->SetDimensionality(3);
   gaussian->SetStandardDeviation(0);
#ifdef HAVE_VTK6
   gaussian->SetInputConnection(shrinker->GetOutputPort());
#else
   gaussian->SetInput(shrinker->GetOutput());
#endif

   //
   // Marching cubes converts volume to a surface
   //
   vtkMarchingCubes* mc = vtkMarchingCubes::New();
#ifdef HAVE_VTK6
   mc->SetInputConnection(gaussian->GetOutputPort());
#else
   mc->SetInput(gaussian->GetOutput());
#endif
   mc->SetValue(0, 127.5);
   mc->ComputeScalarsOff();
   mc->ComputeGradientsOff();
   mc->ComputeNormalsOff();

   //
   // Clean up surface created by marching cubes
   //
   vtkCleanPolyData* clean = vtkCleanPolyData::New();
#ifdef HAVE_VTK6
   clean->SetInputConnection(mc->GetOutputPort());
#else
   clean->SetInput(mc->GetOutput());
#endif
         
   //    
   // Make sure mesh is only triangles
   //
   vtkTriangleFilter *triangleFilter = vtkTriangleFilter::New();
#ifdef HAVE_VTK6
   triangleFilter->SetInputConnection(clean->GetOutputPort());
#else
   triangleFilter->SetInput(clean->GetOutput());
#endif
   
   //
   // Compute normals on the surface to fix polygon orientations
   //
   vtkPolyDataNormals* rawNormals = vtkPolyDataNormals::New();
#ifdef HAVE_VTK6
   rawNormals->SetInputConnection(triangleFilter->GetOutputPort());
#else
   rawNormals->SetInput(triangleFilter->GetOutput());
#endif
   rawNormals->SplittingOff();
   rawNormals->ConsistencyOn();
   rawNormals->ComputePointNormalsOn();
   rawNormals->NonManifoldTraversalOn();
   rawNormals->Update();
   
   //
   // Get topology
   //
   TopologyFile topologyFile;
   topologyFile.importFromVtkFile(rawNormals->GetOutput());
   
   //
   // Remove any islands
   //
   //topologyFile.disconnectIslands();
   
   int faces, edges, vertices;
   topologyFile.getEulerCount(false, 
                              faces, 
                              vertices, 
                              edges, 
                              eulerCount, 
                              numberOfHoles, 
                              numberOfObjects);
   numberOfCavities = getNumberOfSegmentationCavities();
   
   rawNormals->Delete();
   triangleFilter->Delete();
   clean->Delete();
   mc->Delete();
   gaussian->Delete();
   shrinker->Delete();
}
                                              
/**
 * make a sphere within the volume.
 */
void    
VolumeFile::makeSphere(const int center[3], const float radius)
{
   if (DebugControl::getDebugOn()) {
      std::cout << "MakeSphere of " << radius << " radius at "
                << center[0] << ", " << center[1] << ", " << center[2] << std::endl;
   }
   
   const float radiusSquared = radius * radius;
   
   int xDim, yDim, zDim;
   getDimensions(xDim, yDim, zDim);
   
   for (int k = 0; k < zDim; k++) {
      for (int j = 0; j < yDim; j++) {
         for (int i = 0; i < xDim; i++) {
            const float dx = i - center[0];
            const float dy = j - center[1];
            const float dz = k - center[2];
            const float result = dx*dx + dy*dy + dz*dz;
            float value = 0;
            if (result <= radiusSquared) {
               value = 255;
            }
            setVoxel(i, j, k, 0, value);
         }
      }
   }
}

/**
 * assign voxels within border for paint volumes.
 */
void 
VolumeFile::assignVoxelsWithinBorder(const VOLUME_AXIS axis,
                                     const QString& paintName,
                                     const Border* border,
                                     const int slicesAboveAndBelowPlane)
{
   const float voxelValue = addRegionName(paintName);
   const unsigned char voxelColor[4] = { 0, 0, 0, VOXEL_COLOR_STATUS_INVALID };
   const float scaleFactor = 1000.0;

   const int numLinks = border->getNumberOfLinks();

   float sliceZ = 0;
   
   std::vector<float> polygon;
   int numToSkip = 1;
   int numInPolygon = 0;
   for (int i = 0; i < (numLinks - 1); i++) {
      const float* pos = border->getLinkXYZ(i);
      switch (axis) {
         case VOLUME_AXIS_X:
            polygon.push_back(pos[1] * scaleFactor);
            polygon.push_back(pos[2] * scaleFactor);
            if (i == 0) {
               sliceZ = pos[0];
            }
            break;
         case VOLUME_AXIS_Y:
            polygon.push_back(pos[0] * scaleFactor);
            polygon.push_back(pos[2] * scaleFactor);
            if (i == 0) {
               sliceZ = pos[1];
            }
            break;
         case VOLUME_AXIS_Z:
            polygon.push_back(pos[0] * scaleFactor);
            polygon.push_back(pos[1] * scaleFactor);
            if (i == 0) {
               sliceZ = pos[2];
            }
            break;
         case VOLUME_AXIS_ALL:
         case VOLUME_AXIS_OBLIQUE:
         case VOLUME_AXIS_OBLIQUE_X:
         case VOLUME_AXIS_OBLIQUE_Y:
         case VOLUME_AXIS_OBLIQUE_Z:
         case VOLUME_AXIS_OBLIQUE_ALL:
         case VOLUME_AXIS_UNKNOWN:
            return;
            break;
      }
      polygon.push_back(0.0);
      numInPolygon++;
      i += numToSkip; 
   }
   if (numInPolygon < 3) {
      return;
   }
      
   int sliceNumber = 0;
   float sliceDistance = std::numeric_limits<float>::max();
   switch(axis) {
      case VolumeFile::VOLUME_AXIS_X:  // PARASAGITTAL
         for (int i = 0; i < dimensions[0]; i++) {
            const float val = i * spacing[0] + origin[0];
            const float dist = fabs(sliceZ - val);
            if (dist < sliceDistance) {
               sliceNumber = i;
               sliceDistance = dist;
            }
         }
         break;
      case VolumeFile::VOLUME_AXIS_Y:  // CORONAL
         for (int i = 0; i < dimensions[1]; i++) {
            const float val = i * spacing[1] + origin[1];
            const float dist = fabs(sliceZ - val);
            if (dist < sliceDistance) {
               sliceNumber = i;
               sliceDistance = dist;
            }
         }
         break;
      case VolumeFile::VOLUME_AXIS_Z:  // HORIZONTAL
         for (int i = 0; i < dimensions[2]; i++) {
            const float val = i * spacing[2] + origin[2];
            const float dist = fabs(sliceZ - val);
            if (dist < sliceDistance) {
               sliceNumber = i;
               sliceDistance = dist;
            }
         }
         break;
      case VOLUME_AXIS_ALL:
      case VOLUME_AXIS_OBLIQUE:
      case VOLUME_AXIS_OBLIQUE_X:
      case VOLUME_AXIS_OBLIQUE_Y:
      case VOLUME_AXIS_OBLIQUE_Z:
      case VOLUME_AXIS_OBLIQUE_ALL:
      case VOLUME_AXIS_UNKNOWN:
         return;
         break;
   }
   
   //
   // Get the bounds but adjust it based upon the axis
   //
   float bounds[6], boundsTemp[6];
   border->getBounds(boundsTemp);
   switch (axis) {
      case VOLUME_AXIS_X:
         bounds[0] = boundsTemp[2] * scaleFactor;
         bounds[1] = boundsTemp[3] * scaleFactor;
         bounds[2] = boundsTemp[4] * scaleFactor;
         bounds[3] = boundsTemp[5] * scaleFactor;
         break;
      case VOLUME_AXIS_Y:
         bounds[0] = boundsTemp[0] * scaleFactor;
         bounds[1] = boundsTemp[1] * scaleFactor;
         bounds[2] = boundsTemp[4] * scaleFactor;
         bounds[3] = boundsTemp[5] * scaleFactor;
         break;
      case VOLUME_AXIS_Z:
         bounds[0] = boundsTemp[0] * scaleFactor;
         bounds[1] = boundsTemp[1] * scaleFactor;
         bounds[2] = boundsTemp[2] * scaleFactor;
         bounds[3] = boundsTemp[3] * scaleFactor;
         break;
      case VOLUME_AXIS_ALL:
      case VOLUME_AXIS_OBLIQUE:
      case VOLUME_AXIS_OBLIQUE_X:
      case VOLUME_AXIS_OBLIQUE_Y:
      case VOLUME_AXIS_OBLIQUE_Z:
      case VOLUME_AXIS_OBLIQUE_ALL:
      case VOLUME_AXIS_UNKNOWN:
         return;
         break;
   }
   bounds[4] = -1.0;
   bounds[5] =  1.0;
   
   float normal[3] = { 0.0, 0.0, 1.0 };
   
   int maxI = 0;
   int maxJ = 0;
   switch (axis) {
      case VOLUME_AXIS_X:
         maxI = dimensions[1];
         maxJ = dimensions[2];
         break;
      case VOLUME_AXIS_Y:
         maxI = dimensions[0];
         maxJ = dimensions[2];
         break;
      case VOLUME_AXIS_Z:
         maxI = dimensions[0];
         maxJ = dimensions[1];
         break;
      case VOLUME_AXIS_ALL:
      case VOLUME_AXIS_OBLIQUE:
      case VOLUME_AXIS_OBLIQUE_X:
      case VOLUME_AXIS_OBLIQUE_Y:
      case VOLUME_AXIS_OBLIQUE_Z:
      case VOLUME_AXIS_OBLIQUE_ALL:
      case VOLUME_AXIS_UNKNOWN:
         return;
         break;
   }
   
   for (int i = 0; i < maxI; i++) {
      for (int j = 0; j < maxJ; j++) {
         //
         // Get voxel indices
         //
         int ijk[3];         
         switch (axis) {
            case VOLUME_AXIS_X:
               ijk[0] = sliceNumber;
               ijk[1] = i;
               ijk[2] = j;
               break;
            case VOLUME_AXIS_Y:
               ijk[0] = i;
               ijk[1] = sliceNumber;
               ijk[2] = j;
               break;
            case VOLUME_AXIS_Z:
               ijk[0] = i;
               ijk[1] = j;
               ijk[2] = sliceNumber;
               break;
            case VOLUME_AXIS_ALL:
            case VOLUME_AXIS_OBLIQUE:
            case VOLUME_AXIS_OBLIQUE_X:
            case VOLUME_AXIS_OBLIQUE_Y:
            case VOLUME_AXIS_OBLIQUE_Z:
            case VOLUME_AXIS_OBLIQUE_ALL:
            case VOLUME_AXIS_UNKNOWN:
               return;
               break;
         }
         
         if (getVoxelIndexValid(ijk)) {
            float coord[3];
            getVoxelCoordinate(ijk, coord);
            
            coord[0] *= scaleFactor;
            coord[1] *= scaleFactor;
            coord[2] *= scaleFactor;
            
            float xyz[3] = { 0.0, 0.0, 0.0 };
            switch (axis) {
               case VOLUME_AXIS_X:
                  xyz[0] = coord[1];
                  xyz[1] = coord[2];
                  break;
               case VOLUME_AXIS_Y:
                  xyz[0] = coord[0];
                  xyz[1] = coord[2];
                  break;
               case VOLUME_AXIS_Z:
                  xyz[0] = coord[0];
                  xyz[1] = coord[1];
                  break;
               case VOLUME_AXIS_ALL:
               case VOLUME_AXIS_OBLIQUE:
               case VOLUME_AXIS_OBLIQUE_X:
               case VOLUME_AXIS_OBLIQUE_Y:
               case VOLUME_AXIS_OBLIQUE_Z:
               case VOLUME_AXIS_OBLIQUE_ALL:
               case VOLUME_AXIS_UNKNOWN:
                  return;
                  break;
            }
            
            if (MathUtilities::pointInPolygon(xyz,
                                           numInPolygon,
                                           (float*)&polygon[0],
                                           bounds,
                                           normal)) {
               setVoxel(ijk, 0, voxelValue);
               setVoxelColor(ijk, voxelColor);
               
               //
               // Is more than the current slice being filled
               //
               if (slicesAboveAndBelowPlane > 0) {
                  int iStart = ijk[0];
                  int iEnd   = ijk[0];
                  int jStart = ijk[1];
                  int jEnd   = ijk[1];
                  int kStart = ijk[2];
                  int kEnd   = ijk[2];
                  switch (axis) {
                     case VOLUME_AXIS_X:
                        iStart -= slicesAboveAndBelowPlane;
                        iEnd   += slicesAboveAndBelowPlane;
                        break;
                     case VOLUME_AXIS_Y:
                        jStart -= slicesAboveAndBelowPlane;
                        jEnd   += slicesAboveAndBelowPlane;
                        break;
                     case VOLUME_AXIS_Z:
                        kStart -= slicesAboveAndBelowPlane;
                        kEnd   += slicesAboveAndBelowPlane;
                        break;
                     case VOLUME_AXIS_ALL:
                     case VOLUME_AXIS_OBLIQUE:
                     case VOLUME_AXIS_OBLIQUE_X:
                     case VOLUME_AXIS_OBLIQUE_Y:
                     case VOLUME_AXIS_OBLIQUE_Z:
                     case VOLUME_AXIS_OBLIQUE_ALL:
                     case VOLUME_AXIS_UNKNOWN:
                        return;
                        break;
                  }
                  
                  for (int ii = iStart; ii <= iEnd; ii++) {
                     for (int jj = jStart; jj <= jEnd; jj++) {
                        for (int kk = kStart; kk <= kEnd; kk++) {
                           if (getVoxelIndexValid(ii, jj, kk)) {
                              setVoxel(ii, jj, kk, 0, voxelValue);
                              setVoxelColor(ii, jj, kk, voxelColor);
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}
                                    
/**
 * assing paint volume voxels.
 */
void 
VolumeFile::assignPaintVolumeVoxels(const QString& paintName,
                             const int ijkMin[3],
                             const int ijkMax[3],
                             VolumeModification* modifiedVoxels)
{
   const float voxelValue = addRegionName(paintName);
   const unsigned char voxelColor[4] = { 0, 0, 0, VOXEL_COLOR_STATUS_INVALID };
   
   for (int i = ijkMin[0]; i <= ijkMax[0]; i++) {
      for (int j = ijkMin[1]; j <= ijkMax[1]; j++) {
         for (int k = ijkMin[2]; k <= ijkMax[2]; k++) {
            if (getVoxelIndexValid(i, j, k)) {
               if (modifiedVoxels != NULL) {
                  modifiedVoxels->addVoxel(this, i, j, k);
               }

               setVoxel(i, j, k, 0, voxelValue);
               setVoxelColor(i, j, k, voxelColor);
            }
         }
      }
   }
   
   setModified();
   minMaxVoxelValuesValid = false;
   minMaxTwoToNinetyEightPercentVoxelValuesValid = false;
}
                                   
/**
 * AC-PC align a volume.
 */
void 
VolumeFile::acPcAlign(const int superiorAcVoxel[3],
                      const int inferiorPcVoxel[3],
                      const int superiorLateralFissureVoxel[3]) throw (FileException)
{
   //
   // force origin's negative, spacing negative, and make orientation LPI
   //
   origin[0] = -fabs(origin[0]);
   origin[1] = -fabs(origin[1]);
   origin[2] = -fabs(origin[2]);
   spacing[0] = fabs(spacing[0]);
   spacing[1] = fabs(spacing[1]);
   spacing[2] = fabs(spacing[2]);
   orientation[0] = ORIENTATION_LEFT_TO_RIGHT;
   orientation[1] = ORIENTATION_POSTERIOR_TO_ANTERIOR;
   orientation[2] = ORIENTATION_INFERIOR_TO_SUPERIOR;
   
   //
   // old AC voxel
   //
   const float zeros[3] = { 0.0, 0.0, 0.0 };
   int oldAcVoxel[3];
   convertCoordinatesToVoxelIJK(zeros, oldAcVoxel);
   
   //
   // Translate (set origin) so AC becomes the origin
   //
   const float newOrigin[3] = {
      -(superiorAcVoxel[0] * spacing[0]),
      -(superiorAcVoxel[1] * spacing[1]),
      -(superiorAcVoxel[2] * spacing[2])
   };
   setOrigin(newOrigin);
   
   //
   // Get stereotaxic coordinates of PC and CC
   //
   float superiorAcXYZ[3] = { 0.0, 0.0, 0.0 };
   //convertCoordinatesToVoxelIJK(superiorAcXYZ, superiorAcVoxel);
   getVoxelCoordinate(superiorAcVoxel, superiorAcXYZ);
   float inferiorPcXYZ[3];
   getVoxelCoordinate(inferiorPcVoxel, inferiorPcXYZ);
   float superiorLfXYZ[3];
   getVoxelCoordinate(superiorLateralFissureVoxel, superiorLfXYZ);
      
   //
   // Vector from PC to AC
   //
   float acToPc[3] = {
      inferiorPcXYZ[0] - superiorAcXYZ[0],
      inferiorPcXYZ[1] - superiorAcXYZ[1],
      inferiorPcXYZ[2] - superiorAcXYZ[2]
    };
   
   //
   // Normalize the vector
   //
   vtkMath::Normalize(acToPc);
   
   // assume a vector goes from the AC in the
   // negative Y direction so it would be something like (0, -1, 0).
   // Use the dot product to determine the angle between a vector running 
   // down the negative Y axis and the AC to PC unit vector.
   const float negYVector[3] = { 0.0, -1.0, 0.0 };
   const float negYDot = vtkMath::Dot(negYVector, acToPc);
   const float angle = std::acos(negYDot) * MathUtilities::radiansToDegrees();

   //
   // Normal vector formed by PC, AC, and point on negative Y Axis
   // 
   const double floatAC[3] = { superiorAcXYZ[0], superiorAcXYZ[1], superiorAcXYZ[2] };
   const double floatPC[3] = { inferiorPcXYZ[0], inferiorPcXYZ[1], inferiorPcXYZ[2] };
   const double negY[3] = { 0.0, -25.0, 0.0 };
   double pcAcNegYNormal[3];
   vtkTriangle::ComputeNormal((double*)floatPC, (double*)floatAC, (double*)negY, pcAcNegYNormal);

   //
   // Create a transformation matrix for the rotation and apply it to the volume
   //
   TransformationMatrix tm;
   tm.rotate(angle, pcAcNegYNormal);
   tm.transpose();
   applyTransformationMatrix(tm);
   
   
   // *************************************************
   // At this point in time the AC is at the origin and
   // the PC is on the negative Y axis.
   // *************************************************


   //
   // Distance of lateral fissure voxel from XZ plane
   //
   const float lfDist = sqrt(superiorLfXYZ[0] * superiorLfXYZ[0] +
                             superiorLfXYZ[2] * superiorLfXYZ[2]);
                             
   //
   // Vector from Y axis to LF point in the XZ plane
   //
   float lfVect[3] = {
      superiorLfXYZ[0], 
      0.0, 
      superiorLfXYZ[2]
   };
   vtkMath::Normalize(lfVect);
   
   //
   // Vector pointing up in Z
   //
   float ccZ[3] = { 0.0, 0.0, lfDist };
   vtkMath::Normalize(ccZ);
   
   //
   // Angle that volume shold be rotated about Y axis so that 
   // the lateral fissure point is directly above the Y-axis.
   //
   const float yDot = vtkMath::Dot(lfVect, ccZ);
   const float yAngle = std::acos(yDot) * MathUtilities::radiansToDegrees();
   const float yAngle2 = std::atan2(superiorLfXYZ[0], superiorLfXYZ[2])
                       * MathUtilities::radiansToDegrees();
   if (DebugControl::getDebugOn()) {
      std::cout << "AC-PC align along Y Axis:" << std::endl;
      std::cout << "   " << yAngle << "   " << yAngle2 << std::endl;
   }
   
   //
   // Create transformation matrix and apply it to the volume
   //
   const double posYVec[3] = { 0.0, 1.0, 0.0 };
   TransformationMatrix tm2;
   tm2.rotate(yAngle2, posYVec);
   //tm2.rotateY(yAngle);
   tm2.transpose();
   applyTransformationMatrix(tm2);
}                     

/**
 * convert from ITK Image.
 */
void 
VolumeFile::convertFromITKImage(VolumeITKImage& itkImageIn) throw (FileException)
{
#ifndef HAVE_ITK
      throw FileException("ITK support not compiled into this version of Caret");
#endif // HAVE_ITK

#ifdef HAVE_ITK
   //
   //
   //
   VolumeITKImage::ImageTypeFloat3::RegionType region = itkImageIn.image->GetLargestPossibleRegion();

   //
   //
   //
   VolumeITKImage::ImageTypeFloat3::SizeType size = region.GetSize();
   //std::cout << "Size: " << size[0] << ", " << size[1] << ", " << size[2] << std::endl;
   const int dim[3] = { size[0], size[1], size[2] };
   
   //
   // Get origin
   //
   VolumeITKImage::ImageTypeFloat3::PointType org = itkImageIn.image->GetOrigin();
   //std::cout << "Origin: " << org[0] << ", " << org[1] << ", " << org[2] << std::endl;
   float org2[3] = { org[0], org[1], org[2] };
   
   //
   // Get Spacing
   //
   VolumeITKImage::ImageTypeFloat3::SpacingType space = itkImageIn.image->GetSpacing();
   //std::cout << "Spacing: " << space[0] << ", " << space[1] << ", " << space[2] << std::endl;
   float space2[3] = { space[0], space[1], space[2] };
   
   org[0] += (space2[0] * 0.5);
   org[1] += (space2[1] * 0.5);
   org[2] += (space2[2] * 0.5);
   
   //
   // Do voxels need to be reallocated ?
   //
   if ((dimensions[0] != dim[0]) || 
       (dimensions[1] != dim[1]) ||
       (dimensions[2] != dim[2])) {
      initialize(VOXEL_DATA_TYPE_FLOAT,
                 dim,
                 orientation,
                 org2,
                 space2,
                 false,
                 true);
   }
   setSpacing(space2);
   setOrigin(org2);
   
   //
   // Load the pixels
   //
   VolumeITKImage::ImageTypeFloat3::IndexType voxelIndex;
   for (int i = 0; i < dimensions[0]; i++) {
      for (int j = 0; j < dimensions[1]; j++) {
         for (int k = 0; k < dimensions[2]; k++) {
            voxelIndex[0] = i;
            voxelIndex[1] = j;
            voxelIndex[2] = k;
            VolumeITKImage::ImageTypeFloat3::PixelType pixelValue = itkImageIn.image->GetPixel(voxelIndex);
            setVoxel(i, j, k, 0, pixelValue);
         }
      }
   }
   
#endif
}

/**
 * convert to ITK Image.
 */
void 
VolumeFile::convertToITKImage(VolumeITKImage& itkImageOut) throw (FileException)
{
#ifndef HAVE_ITK
      throw FileException("ITK support not compiled into this version of Caret");
#endif // HAVE_ITK

#ifdef HAVE_ITK
   //
   // Size (dimensions) of image
   //
   VolumeITKImage::ImageTypeFloat3::SizeType size;
   size[0] = dimensions[0];
   size[1] = dimensions[1];
   size[2] = dimensions[2];
   
   //
   // First index in each direction
   //
   VolumeITKImage::ImageTypeFloat3::IndexType start;
   start[0] = 0;
   start[1] = 0;
   start[2] = 0;
   
   //
   // Set voxel start and dimensions
   //
   VolumeITKImage::ImageTypeFloat3::RegionType region;
   region.SetIndex(start);
   region.SetSize(size);
   itkImageOut.image->SetRegions(region);
   
   //
   // Set origin
   //
   VolumeITKImage::ImageTypeFloat3::PointType org;
   float forg[3];
   getOriginAtCornerOfVoxel(forg);
   org[0] = forg[0];
   org[1] = forg[1];
   org[2] = forg[2];
   itkImageOut.image->SetOrigin(org);
   
   //
   // Set Spacing
   //
   VolumeITKImage::ImageTypeFloat3::SpacingType space;
   space[0] = spacing[0];
   space[1] = spacing[1];
   space[2] = spacing[2];
   itkImageOut.image->SetSpacing(space);
   
   //
   // Allocate the image data
   //
   itkImageOut.image->Allocate();
   
   //
   // Load the pixels
   //
   VolumeITKImage::ImageTypeFloat3::IndexType voxelIndex;
   for (int i = 0; i < dimensions[0]; i++) {
      for (int j = 0; j < dimensions[1]; j++) {
         for (int k = 0; k < dimensions[2]; k++) {
            voxelIndex[0] = i;
            voxelIndex[1] = j;
            voxelIndex[2] = k;
            itkImageOut.image->SetPixel(voxelIndex, getVoxel(i, j, k));
         }
      }
   }
#endif
}
      
/**
 * get the types of volumes and names available for writing.
 */
void 
VolumeFile::getVolumeFileTypesAndNames(std::vector<FILE_READ_WRITE_TYPE>& fileTypes,
                                       std::vector<QString>& fileTypeNames)
{
   fileTypes.clear();
   fileTypeNames.clear();
   
   fileTypes.push_back(FILE_READ_WRITE_TYPE_AFNI);   fileTypeNames.push_back("AFNI");
   fileTypes.push_back(FILE_READ_WRITE_TYPE_ANALYZE);   fileTypeNames.push_back("Analyze");
   fileTypes.push_back(FILE_READ_WRITE_TYPE_NIFTI);   fileTypeNames.push_back("NIFTI");
   fileTypes.push_back(FILE_READ_WRITE_TYPE_NIFTI_GZIP);   fileTypeNames.push_back("NIFTI_GZIP");
   fileTypes.push_back(FILE_READ_WRITE_TYPE_SPM_OR_MEDX);   fileTypeNames.push_back("SPM/MEDx");
   fileTypes.push_back(FILE_READ_WRITE_TYPE_WUNIL);   fileTypeNames.push_back("WU-NIL");
   
   //
   // Cause compilation warning if types updated
   //
   FILE_READ_WRITE_TYPE frwt = FILE_READ_WRITE_TYPE_RAW;
   switch (frwt) {
      case FILE_READ_WRITE_TYPE_RAW:
         break;
      case FILE_READ_WRITE_TYPE_AFNI:
         break;
      case FILE_READ_WRITE_TYPE_ANALYZE:
         break;
      case FILE_READ_WRITE_TYPE_NIFTI:
         break;
      case FILE_READ_WRITE_TYPE_NIFTI_GZIP:
         break;
      case FILE_READ_WRITE_TYPE_SPM_OR_MEDX:
         break;
      case FILE_READ_WRITE_TYPE_WUNIL:
         break;
      case FILE_READ_WRITE_TYPE_UNKNOWN:
         break;
   }
}

/**
 * read the file header and the specified subvolume data.
 * subVolumeNumber may also be on of VOLUME_READING_SELECTION
 */
void 
VolumeFile::readFile(const QString& fileNameIn, 
                     const int subVolumeNumber,
                     const bool spmRightIsOnLeft) throw (FileException)
{
   QTime timer;
   timer.start();

   //
   // Read the sub-volume
   //
   std::vector<VolumeFile*> volumes;
   readFile(fileNameIn,
            subVolumeNumber,
            volumes,
            spmRightIsOnLeft);
   
   //
   // Was anything read?
   //
   if (volumes.empty() == false) {
      //
      // Copy the volume
      //
      *this = *(volumes[0]);
      this->filename = volumes[0]->filename;
      this->dataFileName = volumes[0]->dataFileName;
   }
   
   //
   // Free the volumes
   //
   for (unsigned int i = 0; i < volumes.size(); i++) {
      delete volumes[i];
   }
   
   timeToReadFileInSeconds = (static_cast<float>(timer.elapsed()) / 1000.0);
   if (DebugControl::getDebugOn()) {
      std::cout << "Time to read " << FileUtilities::basename(filename).toAscii().constData()
                << " was "
                << timeToReadFileInSeconds
                << " seconds." << std::endl;
   }
}                          

/**
 * read the names of the volume's sub-volumes.
 */
void 
VolumeFile::readSubVolumeNames(const QString& fileNameIn,
                               std::vector<QString>& subVolumeNamesOut) throw (FileException)
{   
   //
   // Read the sub-volume
   //
   std::vector<VolumeFile*> volumes;
   readFile(fileNameIn,
            VOLUME_READ_HEADER_ONLY,
            volumes);
            
   //
   // Was anything read?
   //
   if (volumes.empty() == false) {
      subVolumeNamesOut = volumes[0]->subVolumeNames;
   }

   //
   // Free the volumes
   //
   for (unsigned int i = 0; i < volumes.size(); i++) {
      delete volumes[i];
   }
}
                                     
/**
 * read the specified sub-volumes in a volume file.
 * subVolumeNumber may also be on of VOLUME_READING_SELECTION
 */
void 
VolumeFile::readFile(const QString& fileNameIn, 
                     const int readSelection,
                     std::vector<VolumeFile*>& volumesReadOut,
                     const bool spmRightIsOnLeft) throw (FileException)
{
   //
   // clear the output
   //
   volumesReadOut.clear();
   
   //
   // hdr/image pair may be NIFTI
   //
   bool niftiFlag = false;
   if (StringUtilities::endsWith(fileNameIn, SpecFile::getAnalyzeVolumeFileExtension())) {
      niftiFlag = NiftiFileHeader::hdrIsNiftiFile(fileNameIn);
      if (DebugControl::getDebugOn()) {
         std::cout << "HDR file is a NIFTI header file." << std::endl;
      }
   }
   
   FILE_READ_WRITE_TYPE fileTypeToWrite = FILE_READ_WRITE_TYPE_NIFTI;
   
   //
   // Note that SPM and Analyze are the same file type
   //
   if (StringUtilities::endsWith(fileNameIn, SpecFile::getAnalyzeVolumeFileExtension())
       && (niftiFlag == false)) {
      readFileSpm(fileNameIn, readSelection, volumesReadOut, spmRightIsOnLeft);
      fileTypeToWrite = FILE_READ_WRITE_TYPE_SPM_OR_MEDX;
   }
   else if (StringUtilities::endsWith(fileNameIn, SpecFile::getAfniVolumeFileExtension())) {
      readFileAfni(fileNameIn, readSelection, volumesReadOut);
      fileTypeToWrite = FILE_READ_WRITE_TYPE_AFNI;
   }
   else if (StringUtilities::endsWith(fileNameIn, SpecFile::getMincVolumeFileExtension())) {
      VolumeFile* vf = new VolumeFile;
      try {
         vf->importMincVolume(fileNameIn);
         volumesReadOut.push_back(vf);
      }
      catch (FileException& e) {
         delete vf;
         vf = NULL;
         throw e;
      }
      fileTypeToWrite = FILE_READ_WRITE_TYPE_NIFTI;
   }
   else if (StringUtilities::endsWith(fileNameIn, SpecFile::getNiftiVolumeFileExtension()) ||
            StringUtilities::endsWith(fileNameIn, SpecFile::getNiftiGzipVolumeFileExtension()) ||
            niftiFlag) {
      readFileNifti(fileNameIn, readSelection, volumesReadOut);
      fileTypeToWrite = FILE_READ_WRITE_TYPE_NIFTI;
   }
   else if (StringUtilities::endsWith(fileNameIn, SpecFile::getWustlVolumeFileExtension())) {
      readFileWuNil(fileNameIn, readSelection, volumesReadOut);
      fileTypeToWrite = FILE_READ_WRITE_TYPE_WUNIL;
   }
   else if (StringUtilities::endsWith(fileNameIn, ".vtk")) {
      VolumeFile* vf = new VolumeFile;
      try {
         vf->importVtkStructuredPointsVolume(fileNameIn);
         volumesReadOut.push_back(vf);
      }
      catch (FileException& e) {
         delete vf;
         vf = NULL;
         throw e;
      }
      fileTypeToWrite = FILE_READ_WRITE_TYPE_NIFTI;
   }
   else {
      throw FileException(fileNameIn, "File extension not recognized as a volume\n"
                                      "file type supported by Caret.");
   }
   
   for (unsigned int i = 0; i < volumesReadOut.size(); i++) {
      switch (volumeSpace) {
         case VOLUME_SPACE_COORD_LPI:
            break;
         case VOLUME_SPACE_VOXEL_NATIVE:
            break;
      }
      volumesReadOut[i]->clearModified();
      volumesReadOut[i]->setFileWriteType(fileTypeToWrite);
   }
}                          

/**
 * write the specified sub-volumes.
 */
void 
VolumeFile::writeFile(const QString& fileNameIn,
                      const VOLUME_TYPE volumeType,
                      const VOXEL_DATA_TYPE writeVoxelDataType,
                      std::vector<VolumeFile*>& volumesToWrite,
                      const bool zipAfniBrikFile,
                      const ColorFile* labelColorsForCaret6or7,
                      const int writeForCaretVersion) throw (FileException)
{
   QFile file(fileNameIn);
   if (AbstractFile::getOverwriteExistingFilesAllowed() == false) {
      if (file.exists()) {
         throw FileException("file exists and overwrite is prohibited.");
      }
   }

   if (volumesToWrite.size() == 0) {
      throw FileException(fileNameIn, "No volume data to write.");
   }
   
   switch (volumeType) {
      case VOLUME_TYPE_ANATOMY:
         break;
      case VOLUME_TYPE_FUNCTIONAL:
         break;
      case VOLUME_TYPE_PAINT:
         if (volumesToWrite.size() > 1) {
            synchronizeRegionNames(volumesToWrite);
         }
         break;
      case VOLUME_TYPE_PROB_ATLAS:
         break;
      case VOLUME_TYPE_RGB:
         break;
      case VOLUME_TYPE_SEGMENTATION:
         break;
      case VOLUME_TYPE_VECTOR:
         break;
      case VOLUME_TYPE_ROI:
         break;
      case VOLUME_TYPE_UNKNOWN:
         break;
   }
   
   //
   // Verify that the volumes have the same dimensions
   //
   int dim1[3];
   volumesToWrite[0]->getDimensions(dim1);
   for (unsigned int i = 1; i < volumesToWrite.size(); i++) {
      int dim2[3];
      volumesToWrite[i]->getDimensions(dim2);
      
      if ((dim1[0] != dim2[0]) ||
          (dim1[1] != dim2[1]) ||
          (dim1[2] != dim2[2])) {
         QString msg("All volumes written to a file must have the same dimensions.\n"
                     "These volumes have different dimensions: ");
         msg += FileUtilities::basename(volumesToWrite[0]->getFileName());
         msg += " and ";
         msg += FileUtilities::basename(volumesToWrite[i]->getFileName());
         throw FileException(msg);
      }
   }
   
   //
   // Note that SPM and Analyze are the same file type
   //
   if (StringUtilities::endsWith(fileNameIn, SpecFile::getAnalyzeVolumeFileExtension())) {
      writeFileSPM(fileNameIn, writeVoxelDataType, volumesToWrite);
   }
   else if (StringUtilities::endsWith(fileNameIn, SpecFile::getAfniVolumeFileExtension())) {
      writeFileAfni(fileNameIn, writeVoxelDataType, volumesToWrite, zipAfniBrikFile);
   }
   else if (StringUtilities::endsWith(fileNameIn, SpecFile::getMincVolumeFileExtension())) {
      if (volumesToWrite.size() != 1) {
         throw FileException(fileNameIn, "Multi-volume MINC files not supported.");
      }
      VolumeFile* vf = volumesToWrite[0];
      try {
         vf->exportMincVolume(fileNameIn);
      }
      catch (FileException& e) {
         delete vf;
         vf = NULL;
         throw e;
      }
   }
   else if (StringUtilities::endsWith(fileNameIn, SpecFile::getNiftiVolumeFileExtension()) ||
            StringUtilities::endsWith(fileNameIn, SpecFile::getNiftiGzipVolumeFileExtension())) {
      writeFileNifti(fileNameIn, writeVoxelDataType, volumesToWrite, labelColorsForCaret6or7, writeForCaretVersion);
   }
   else if (StringUtilities::endsWith(fileNameIn, SpecFile::getWustlVolumeFileExtension())) {
      writeFileWuNil(fileNameIn, writeVoxelDataType, volumesToWrite);
   }
   else if (StringUtilities::endsWith(fileNameIn, ".vtk")) {
      if (volumesToWrite.size() != 1) {
         throw FileException(fileNameIn, "Multi-volume VTK files not supported.");
      }
      VolumeFile* vf = volumesToWrite[0];
      try {
         vf->exportVtkStructuredPointsVolume(fileNameIn);
      }
      catch (FileException& e) {
         delete vf;
         vf = NULL;
         throw e;
      }
   }
   else {
      throw FileException(fileNameIn, "File extension not recognized as a volume\n"
                                      "file type supported by Caret.");
   }
   
   for (unsigned int i = 0; i < volumesToWrite.size(); i++) {
      volumesToWrite[i]->clearModified();
   }
}                          
                      
/**
 * read the specified sub-volumes in a volume file.
 */
void 
VolumeFile::readFileAfni(const QString& fileNameIn, 
                         const int readSelection,
                         std::vector<VolumeFile*>& volumesReadOut) throw (FileException)
{
   QFile file(fileNameIn);
   if (file.open(QIODevice::ReadOnly) == false) {
      throw FileException(fileNameIn, file.errorString());
   }
   QTextStream stream(&file);
   
   VolumeFile volumeRead;
   volumeRead.filename = fileNameIn; 
   volumeRead.voxelDataType = VOXEL_DATA_TYPE_SHORT; 
  
   AfniHeader* afniHeader = &(volumeRead.afniHeader);
   
   afniHeader->clear();
   afniHeader->readHeader(volumeRead.filename, file, stream);
   
   //
   // Close the file
   //
   file.close();
   
   QString errorMessage;
   
   //
   // Get Dataset rank first since it tells the number of subvolumes in 2nd value
   //
   const AfniAttribute* rankAttr = afniHeader->getAttribute(AfniAttribute::NAME_DATASET_RANK);
   if (rankAttr != NULL) {
      std::vector<int> values;
      rankAttr->getValue(values);
      if (values.size() < 2) {
         errorMessage.append("Required attribute DATASET_RANK has fewer than 2 elements\n");
      }
      else {
         volumeRead.initializeSubVolumes(values[1]);
      }
   }
   else {
      errorMessage.append("Required attribute DATASET_RANK not found\n");
   }

   //
   // Get the dimensions of the volume
   //
   const AfniAttribute* dimAttr = afniHeader->getAttribute(AfniAttribute::NAME_DATASET_DIMENSIONS);
   if (dimAttr != NULL) {
      std::vector<int> values;
      dimAttr->getValue(values);
      if (values.size() < 3) {
         errorMessage.append("Required attribute DATASET_DIMENSIONS has fewer than 3 elements\n");
      }
      else {
         volumeRead.setDimensions(&values[0]);
      }
   }
   else {
      errorMessage.append("Required attribute DATASET_DIMENSIONS not found\n");
   }

   //
   // Get the spacing of the volume
   //
   const AfniAttribute* spaceAttr = afniHeader->getAttribute(AfniAttribute::NAME_DELTA);
   if (spaceAttr != NULL) {
      std::vector<float> values;
      spaceAttr->getValue(values);
      if (values.size() < 3) {
         errorMessage.append("Required attribute DELTA has fewer than 3 elements\n");
      }
      else {
         volumeRead.setSpacing(&values[0]);
      }
   }
   else {
      errorMessage.append("Required attribute DELTA not found\n");
   }

   //
   // Get the orientation of the volume
   //
   const AfniAttribute* orientAttr = afniHeader->getAttribute(AfniAttribute::NAME_ORIENT_SPECIFIC);
   if (orientAttr != NULL) {
      std::vector<int> values;
      orientAttr->getValue(values);
      if (values.size() < 3) {
         errorMessage.append("Required attribute ORIENT_SPECIFIC has fewer than 3 elements\n");
      }
      else {
         ORIENTATION orientation[3] = {
            ORIENTATION_UNKNOWN,
            ORIENTATION_UNKNOWN,
            ORIENTATION_UNKNOWN
         };
         
         for (int j = 0; j < 3; j++) {
            switch(values[j]) {
               case 0:
                  orientation[j] = ORIENTATION_RIGHT_TO_LEFT;
                  break;
               case 1:
                  orientation[j] = ORIENTATION_LEFT_TO_RIGHT;
                  break;
               case 2:
                  orientation[j] = ORIENTATION_POSTERIOR_TO_ANTERIOR;
                  break;
               case 3:
                  orientation[j] = ORIENTATION_ANTERIOR_TO_POSTERIOR;
                  break;
               case 4:
                  orientation[j] = ORIENTATION_INFERIOR_TO_SUPERIOR;
                  break;
               case 5:
                  orientation[j] = ORIENTATION_SUPERIOR_TO_INFERIOR;
                  break;
               default:
                  {
                     std::ostringstream str;
                     str << "Unrecognized orientation value: "
                           << values[j];
                     errorMessage.append(str.str().c_str());
                     errorMessage.append("\n");
                  }
            }
         }
         volumeRead.setOrientation(orientation);
      }
   }
   else {
      errorMessage.append("Required attribute ORIENT_SPECIFIC not found\n");
   }

   //
   // Get the origin of the volume
   //
   const AfniAttribute* originAttr = afniHeader->getAttribute(AfniAttribute::NAME_ORIGIN);
   if (originAttr != NULL) {
      std::vector<float> values;
      originAttr->getValue(values);
      if (values.size() < 3) {
         errorMessage.append("Required attribute ORIGIN has fewer than 3 elements\n");
      }
      else {
         float origin[3] = { 0.0, 0.0, 0.0 };
         int dimensions[3];
         volumeRead.getDimensions(dimensions);
         float spacing[3];
         volumeRead.getSpacing(spacing);

         //
         //   NOTE: AFNI uses DICOM style where right, anterior, and inferior are negative.
         //         In addition, AFNI's specifies the origin using the center of the voxel.
         //
         //   ORIENTATION   ORIGIN  DELTA
         //     R->L (0)       -      +
         //     L->R (1)       +      -
         //     P->A (2)       +      -
         //     A->P (3)       -      +
         //     I->S (4)       -      +
         //     S->I (5)       +      -
         // 
         for (int i = 0; i < 3; i++) {
            switch(volumeRead.orientation[i]) {
               case ORIENTATION_UNKNOWN:
                  break;
               case ORIENTATION_RIGHT_TO_LEFT:
                  spacing[i] = -fabs(spacing[i]);
                  origin[i] = fabs(values[i]); // + (fabs(spacing[i]) * 0.5);
                  break;
               case ORIENTATION_LEFT_TO_RIGHT:
                  spacing[i] = fabs(spacing[i]);
                  origin[i]  = -fabs(values[i]); // - (fabs(spacing[i]) * 0.5);
                  break;
               case ORIENTATION_POSTERIOR_TO_ANTERIOR:
                  spacing[i] = fabs(spacing[i]);
                  origin[i]  = -fabs(values[i]); // - (fabs(spacing[i]) * 0.5);
                  break;
               case ORIENTATION_ANTERIOR_TO_POSTERIOR:
                  spacing[i] = -fabs(spacing[i]);
                  origin[i] = fabs(values[i]); // + (fabs(spacing[i]) * 0.5);
                  break;
               case ORIENTATION_INFERIOR_TO_SUPERIOR:
                  spacing[i] = fabs(spacing[i]);
                  origin[i]  = -fabs(values[i]); // - (fabs(spacing[i]) * 0.5);
                  break;
               case ORIENTATION_SUPERIOR_TO_INFERIOR:
                  spacing[i] = -fabs(spacing[i]);
                  origin[i] =  fabs(values[i]); // + (fabs(spacing[i]) * 0.5);
                  break;
            }
         }
         volumeRead.setOrigin(origin);
         volumeRead.setSpacing(spacing);
      }
   }
   else {
      errorMessage.append("Required attribute ORIGIN not found\n");
   }

   //
   // AFNI data type is optional and the default is short
   //
   volumeRead.voxelDataType = VOXEL_DATA_TYPE_SHORT;
   
   //
   // Get the data type of the volume's voxels
   //
   const AfniAttribute* typesAttr = afniHeader->getAttribute(AfniAttribute::NAME_BRICK_TYPES);
   if (typesAttr != NULL) {
      std::vector<int> values;
      typesAttr->getValue(values);
      if (values.size() < 1) {
         errorMessage.append("Required attribute BRICK_TYPES has fewer than 1 elements\n");
      }
      else {
         switch(values[0]) {
            case 0:
               volumeRead.voxelDataType = VOXEL_DATA_TYPE_CHAR_UNSIGNED;
               break;
            case 1:
               volumeRead.voxelDataType = VOXEL_DATA_TYPE_SHORT;
               break;
            case 2:
               volumeRead.voxelDataType = VOXEL_DATA_TYPE_INT;
               break;
            case 3:
               volumeRead.voxelDataType = VOXEL_DATA_TYPE_FLOAT;
               break;
            case 4:
               volumeRead.voxelDataType = VOXEL_DATA_TYPE_DOUBLE;
               break;
            case 5:
               errorMessage.append("Complex AFNI data type not supported\n");
               break;
            case 6:
               volumeRead.voxelDataType = VOXEL_DATA_TYPE_RGB_VOXEL_INTERLEAVED;
               break;
            case 7:
               errorMessage.append("RGBA AFNI data type not supported\n");
               break;
            default:
               {
                  std::ostringstream str;
                  str << "Unrecognized AFNI data type: "
                      << values[0];
                  errorMessage.append(str.str().c_str());
                  errorMessage.append("\n");
               }
               break;
         }
      }
   }
   else {
      errorMessage.append("Required attribute BRICK_TYPES not found\n");
   }
   
   //
   // If error message is not isEmpty something required is missing from the volume
   //
   if (errorMessage.isEmpty() == false) {
      throw FileException(FileUtilities::basename(volumeRead.filename), errorMessage);
   }
   
   //
   // Get the sccale factors of the volume
   //
   const AfniAttribute* scaleAttr = afniHeader->getAttribute(AfniAttribute::NAME_BRICK_FLOAT_FACS);
   if (scaleAttr != NULL) {
      std::vector<float> values;
      scaleAttr->getValue(values);
      for (int i = 0; i < volumeRead.numberOfSubVolumes; i++) {
         if (i < static_cast<int>(values.size())) {
            volumeRead.scaleSlope[i] = values[i];
            if (volumeRead.scaleSlope[i] == 0.0) {
               volumeRead.scaleSlope[i] = 1.0;
            }
         }
         else {
            volumeRead.scaleSlope[i] = 1.0;
         }
      }
   }
   
   //
   // QT will tell us if the "endian" for this computer
   //
   //int wordSize;
   bool bigEndian = (QSysInfo::ByteOrder == QSysInfo::BigEndian);
   //qSysInfo(&wordSize, &bigEndian);
   
   //
   // Get the byte order
   //
   const AfniAttribute* byteAttr = afniHeader->getAttribute(AfniAttribute::NAME_BYTEORDER_STRING);
   bool byteSwapFlag = false;
   if (byteAttr != NULL) {
      if (byteAttr->getValue() == "MSB_FIRST") {
         if (bigEndian == false) {
            byteSwapFlag = true;
         }
      }
      else {
         if (bigEndian) {
            byteSwapFlag = true;
         }
      }
   } 
   
   //
   // Get the subvolume names
   //
   const AfniAttribute* nameAttr = afniHeader->getAttribute(AfniAttribute::NAME_BRICK_LABS);
   if (nameAttr != NULL) {
      std::vector<QString> names;
      StringUtilities::token(nameAttr->getValue(), "~", names);
      const unsigned int minNames = std::min(names.size(), volumeRead.subVolumeNames.size());
      for (unsigned int i = 0; i < minNames; i++) {
         volumeRead.subVolumeNames[i] = names[i];
      }
   }
   
   //
   // Get the HISTORY_NOTE and use it as a comment
   //
   const AfniAttribute* commentAttr = afniHeader->getAttribute(AfniAttribute::NAME_HISTORY_NOTE);
   if (commentAttr != NULL) {
      volumeRead.setHeaderTag(headerTagComment, commentAttr->getValue());
   }
   
   //
   // Get the region names (for paint volume)
   //
   const AfniAttribute* regionAttr = afniHeader->getAttribute(AfniAttribute::NAME_LUT_NAMES);
   if (regionAttr != NULL) {
      volumeRead.regionNames.clear();
      StringUtilities::token(regionAttr->getValue(), "~", volumeRead.regionNames);
   }
   
   //
   // Get the study meta data
   //
   std::vector<StudyMetaDataLinkSet> studyMetaDataLinkSets;
   const AfniAttribute* studyMetaDataAttr = afniHeader->getAttribute(AfniAttribute::NAME_CARET_METADATA_LINK);
   if (studyMetaDataAttr != NULL) {
      std::vector<QString> md;
      StringUtilities::token(studyMetaDataAttr->getValue(), "~", md);
      for (unsigned int i = 0; i < md.size(); i++) {
         StudyMetaDataLinkSet smdls;
         smdls.setLinkSetFromCodedText(md[i]);
         studyMetaDataLinkSets.push_back(smdls);
      }
   }

   //
   // Set the PubMed ID
   //
   const AfniAttribute* pubMedIDAttr = afniHeader->getAttribute(AfniAttribute::NAME_CARET_PUBMED_ID);
   if (pubMedIDAttr != NULL) {
      volumeRead.setFilePubMedID(pubMedIDAttr->getValue());
   }
   
   //
   // Default data file name
   //
   volumeRead.dataFileName = FileUtilities::filenameWithoutExtension(volumeRead.filename);
   volumeRead.dataFileName.append(".BRIK");

   //
   // When an Analyze volume is converted to an AFNI volume with "to3d", just
   // the AFNI header is created with the attribute VOLUME_FILENAMES containing
   // the name of the Analyze volume data.
   //
   const AfniAttribute* volumeFileName = afniHeader->getAttribute(AfniAttribute::NAME_VOLUME_FILENAMES);
   if (volumeFileName != NULL) {
      volumeRead.dataFileName = FileUtilities::dirname(volumeRead.filename);
      volumeRead.dataFileName.append("/");
      volumeRead.dataFileName.append(volumeFileName->getValue());
   }
   
   //
   // Data file might be gzipped
   //
   if (QFile::exists(volumeRead.dataFileName) == false) {
      QString zipName(volumeRead.dataFileName);
      zipName.append(".gz");
      if (QFile::exists(zipName)) {
         volumeRead.dataFileName = zipName;
      }
   }

   //
   // Keep track of data files zipped status
   //   
   volumeRead.dataFileWasZippedFlag = (volumeRead.dataFileName.right(3) == ".gz");

   //
   // If only reading header
   //
   if (readSelection == -2) {
      VolumeFile* vf = new VolumeFile;
      vf->copyVolumeData(volumeRead, false);
      vf->filename = volumeRead.filename;
      vf->dataFileName = volumeRead.dataFileName;
      if (studyMetaDataLinkSets.empty() == false) {
         vf->setStudyMetaDataLinkSet(studyMetaDataLinkSets[0]);
      }
      volumesReadOut.push_back(vf);
      return;
   }
   
   //
   // Open the data file
   //
   gzFile dataFile = gzopen(volumeRead.dataFileName.toAscii().constData(), "rb");
   if (dataFile == NULL) {
      QString msg("Unable to open data file: ");
      msg.append(volumeRead.dataFileName);
      throw FileException(fileNameIn, msg);
   }

   try {         
      //
      // loop through and read the volume files
      //
      const int numSubs = volumeRead.subVolumeNames.size();
      for (int i = 0; i < numSubs; i++) {
         //
         // Determine if this sub volume should be read
         //
         bool readingSingleSubVolume = false;
         bool readIt = false;
         if (readSelection == VOLUME_READ_SELECTION_ALL) {
            readIt = true;
         }
         else if (readSelection == i) {
            readIt = true;
            readingSingleSubVolume = true;
         }
         
         if (readIt) {
            //
            // copy everything but voxel data from the first volume
            //
            VolumeFile* vf = NULL;
            vf = new VolumeFile;
            vf->copyVolumeData(volumeRead, false);
            vf->filename = volumeRead.filename;
            vf->dataFileName = volumeRead.dataFileName;
            
            //
            // Set the descriptive label
            //
            vf->descriptiveLabel = volumeRead.subVolumeNames[i];
            
            //
            // If only reading a single sub-volume
            //
            if (readingSingleSubVolume) {
               //
               // Read just the sub-volume
               //
               vf->readVolumeFileDataSubVolume(byteSwapFlag,
                                               vf->scaleSlope[i],
                                               vf->scaleOffset[i],
                                               0,
                                               i,
                                               dataFile);
            }
            else {
               //
               // Read just the sub-volume
               //
               vf->readVolumeFileData(byteSwapFlag,
                                      vf->scaleSlope[i],
                                      vf->scaleOffset[i],
                                      dataFile);
            }
            
            if (i < static_cast<int>(studyMetaDataLinkSets.size())) {
               vf->setStudyMetaDataLinkSet(studyMetaDataLinkSets[i]);
            }
            
            //
            // return the volume to the user
            //
            volumesReadOut.push_back(vf);
            
            //
            // If only reading a single sub-volume
            //
            if (readingSingleSubVolume) {
               break;
            }
         }
      }
   }
   catch (FileException& e) {
      gzclose(dataFile);
      throw e;
   }
   
   //
   // Close the file
   //
   gzclose(dataFile);
}                          

/**
 * read the specified sub-volumes in a volume file.
 */
void 
VolumeFile::readFileAnalyze(const QString& fileNameIn, 
                            const int readSelection,
                            std::vector<VolumeFile*>& volumesReadOut,
                            const bool spmFlag) throw (FileException)
{
   //
   // Open the data file
   //   
   QFile file(fileNameIn);
   if (file.open(QIODevice::ReadOnly) == false) {
      throw FileException(fileNameIn, file.errorString());
   }
   
   //
   // Read the header file
   //
   struct dsr hdr;
   const unsigned long headerSize = sizeof(hdr);
   const unsigned long numBytesRead = file.read((char*)&hdr, headerSize);
   if (numBytesRead != headerSize) {
      std::ostringstream str;
      str << "Tried to read " 
          << headerSize
          << " bytes from header.\n"
          << "Only read "
          << numBytesRead
          << ".";
      throw FileException(fileNameIn, str.str().c_str());
   }
   
   //
   // Close the file
   //
   file.close();
   
   short spmAcShort[5] = { 0, 0, 0, 0, 0 };
   if (spmFlag) {
      //
      // Need to use memcpy since "hdr.hist.originator" is on odd byte boundary
      //
      memcpy(spmAcShort, hdr.hist.originator, 10);
   }
   
   //
   // Volume file for reading header
   //
   VolumeFile volumeRead;
   volumeRead.filename = fileNameIn;
   
   //
   // See if data needs to be byteswapped
   //
   bool byteSwapFlag= false;
   if ((hdr.dime.dim[0] < 0) || (hdr.dime.dim[0] > 15)) {
      byteSwapFlag = true;

      ByteSwapping::swapBytes(&hdr.hk.sizeof_hdr, 1);
      ByteSwapping::swapBytes(&hdr.hk.extents, 1);
      ByteSwapping::swapBytes(&hdr.hk.session_error, 1);
      
      ByteSwapping::swapBytes(hdr.dime.dim, 8);
      ByteSwapping::swapBytes(&hdr.dime.unused8, 1);
      ByteSwapping::swapBytes(&hdr.dime.unused9, 1);
      ByteSwapping::swapBytes(&hdr.dime.unused10, 1);
      ByteSwapping::swapBytes(&hdr.dime.unused11, 1);
      ByteSwapping::swapBytes(&hdr.dime.unused12, 1);
      ByteSwapping::swapBytes(&hdr.dime.unused13, 1);
      ByteSwapping::swapBytes(&hdr.dime.unused14, 1);
      ByteSwapping::swapBytes(&hdr.dime.datatype, 1);
      ByteSwapping::swapBytes(&hdr.dime.bitpix, 1);
      ByteSwapping::swapBytes(&hdr.dime.dim_un0, 1);
      ByteSwapping::swapBytes(hdr.dime.pixdim, 8);
      ByteSwapping::swapBytes(&hdr.dime.vox_offset, 1);
      ByteSwapping::swapBytes(&hdr.dime.funused1, 1);
      ByteSwapping::swapBytes(&hdr.dime.funused2, 1);
      ByteSwapping::swapBytes(&hdr.dime.funused3, 1);
      ByteSwapping::swapBytes(&hdr.dime.cal_max, 1);
      ByteSwapping::swapBytes(&hdr.dime.cal_min, 1);
      ByteSwapping::swapBytes(&hdr.dime.compressed, 1);
      ByteSwapping::swapBytes(&hdr.dime.verified, 1);
      ByteSwapping::swapBytes(&hdr.dime.glmax, 1);
      ByteSwapping::swapBytes(&hdr.dime.glmin, 1);
      
      ByteSwapping::swapBytes(&hdr.hist.views, 1);
      ByteSwapping::swapBytes(&hdr.hist.vols_added, 1);
      ByteSwapping::swapBytes(&hdr.hist.start_field, 1);
      ByteSwapping::swapBytes(&hdr.hist.field_skip, 1);
      ByteSwapping::swapBytes(&hdr.hist.omax, 1);
      ByteSwapping::swapBytes(&hdr.hist.omin, 1);
      ByteSwapping::swapBytes(&hdr.hist.smax, 1);
      ByteSwapping::swapBytes(&hdr.hist.smin, 1);
      ByteSwapping::swapBytes(spmAcShort, 5);
   }
   
   //
   // Create the volume data file name
   //
   volumeRead.dataFileName = FileUtilities::filenameWithoutExtension(volumeRead.filename);
   volumeRead.dataFileName.append(".img");
   
   if ((hdr.dime.dim[0] > 0) &&
       (hdr.dime.dim[1] > 0) &&
       (hdr.dime.dim[2] > 0) &&
       (hdr.dime.dim[3] > 0)) {
      int dimensions[3];
      dimensions[0] = hdr.dime.dim[1];
      dimensions[1] = hdr.dime.dim[2];
      dimensions[2] = hdr.dime.dim[3];
      volumeRead.setDimensions(dimensions);
   }
         
   int numSubVols = 1;
   if (hdr.dime.dim[0] >= 4) {
      numSubVols = hdr.dime.dim[4];
   }
   volumeRead.initializeSubVolumes(numSubVols);
   
   switch(hdr.dime.datatype) {
      case ANDT_UNKNOWN:
         throw FileException(FileUtilities::basename(volumeRead.filename),
                               "Analyze unknown data type not supported.");
         break;
      case ANDT_BINARY:
         throw FileException(FileUtilities::basename(volumeRead.filename),
                               "Analyze binary data type not supported.");
         break;
      case ANDT_UNSIGNED_CHAR:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_CHAR_UNSIGNED;
         break;
      case ANDT_SIGNED_SHORT:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_SHORT;
         break;
      case ANDT_SIGNED_INT:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_INT;
         break;
      case ANDT_FLOAT:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_FLOAT;
         break;
      case ANDT_COMPLEX:
         throw FileException(FileUtilities::basename(volumeRead.filename),
                               "Analyze complex data type not supported.");
         break;
      case ANDT_DOUBLE:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_DOUBLE;
         break;
      case ANDT_RGB:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED;
         break;
      default:
         throw FileException(FileUtilities::basename(volumeRead.filename),
                               "Analyze data type is unknown value.");
         break;
   }
   
   //
   // Comment info
   //
   char comm[80];
   for (int m = 0; m < 79; m++) {
      comm[m] = hdr.hist.descrip[m];
   }
   comm[79] = '\0';
   volumeRead.setHeaderTag(headerTagComment, comm);
   
   float spacing[3];
   spacing[0] = hdr.dime.pixdim[1];
   spacing[1] = hdr.dime.pixdim[2];
   spacing[2] = hdr.dime.pixdim[3];
   volumeRead.setSpacing(spacing);
    
   if (spmFlag) {
      volumeRead.spmAcPosition[0] = spmAcShort[0] - 1;
      volumeRead.spmAcPosition[1] = spmAcShort[1] - 1;
      volumeRead.spmAcPosition[2] = spmAcShort[2] - 1;
      if (hdr.dime.funused1 > 0.0) {
         for (int i = 0; i < volumeRead.numberOfSubVolumes; i++) {
            if (i < static_cast<int>(volumeRead.scaleSlope.size())) {
               volumeRead.scaleSlope[i] = hdr.dime.funused1;
               if (volumeRead.scaleSlope[i] == 0.0) {
                  volumeRead.scaleSlope[i] = 1.0;
               }
            }
         }
      }
      
      float origin[3];
      origin[0] = -volumeRead.spmAcPosition[0] * spacing[0];
      origin[1] = -volumeRead.spmAcPosition[1] * spacing[1];
      origin[2] = -volumeRead.spmAcPosition[2] * spacing[2];
      volumeRead.setOrigin(origin);
      
      //
      // SPM or MEDx always LPI orientation
      //
      volumeRead.orientation[0] = ORIENTATION_LEFT_TO_RIGHT;
      volumeRead.orientation[1] = ORIENTATION_POSTERIOR_TO_ANTERIOR;
      volumeRead.orientation[2] = ORIENTATION_INFERIOR_TO_SUPERIOR;
  
      if (DebugControl::getDebugOn()) {
         cout << "Spm Voxel Size: " 
               << volumeRead.spacing[0] << " "
               << volumeRead.spacing[1] << " "
               << volumeRead.spacing[2] << endl;
         cout << "Spm AC Position: " 
               << volumeRead.spmAcPosition[0] << " "
               << volumeRead.spmAcPosition[1] << " "
               << volumeRead.spmAcPosition[2] << endl;
         cout << "Spm Scaling: " << volumeRead.scaleSlope[0] << endl;
      }
   }
   
   //
   // Data file might be gzipped
   //
   if (QFile::exists(volumeRead.dataFileName) == false) {
      QString zipName(volumeRead.dataFileName);
      zipName.append(".gz");
      if (QFile::exists(zipName)) {
         volumeRead.dataFileName = zipName;
      }
   }
   
   //
   // If only reading header
   //
   if (readSelection == -2) {
      VolumeFile* vf = new VolumeFile;
      vf->copyVolumeData(volumeRead, false);
      vf->filename = volumeRead.filename;
      vf->dataFileName = volumeRead.dataFileName;
      volumesReadOut.push_back(vf);
      return;
   }
   
   //
   // Open the data file
   //
   gzFile dataFile = gzopen(volumeRead.dataFileName.toAscii().constData(), "rb");
   if (dataFile == NULL) {
      QString msg("Unable to open data file: ");
      msg.append(volumeRead.dataFileName);
      throw FileException(fileNameIn, msg);
   }

   try {         
      //
      // loop through and read the volume files
      //
      const int numSubs = volumeRead.subVolumeNames.size();
      for (int i = 0; i < numSubs; i++) {
         //
         // Determine if this sub volume should be read
         //
         bool readingSingleSubVolume = false;
         bool readIt = false;
         if (readSelection == VOLUME_READ_SELECTION_ALL) {
            readIt = true;
         }
         else if (readSelection == i) {
            readIt = true;
            readingSingleSubVolume = true;
         }
         
         if (readIt) {
            //
            // copy everything but voxel data from the first volume
            //
            VolumeFile* vf = NULL;
            vf = new VolumeFile;
            vf->copyVolumeData(volumeRead, false);
            vf->filename = volumeRead.filename;
            vf->dataFileName = volumeRead.dataFileName;
            
            //
            // Set the descriptive label
            //
            vf->descriptiveLabel = volumeRead.subVolumeNames[i];
            
            //
            // If only reading a single sub-volume
            //
            if (readingSingleSubVolume) {
               //
               // Read just the sub-volume
               //
               vf->readVolumeFileDataSubVolume(byteSwapFlag,
                                               vf->scaleSlope[i],
                                               vf->scaleOffset[i],
                                               0,
                                               i,
                                               dataFile);
            }
            else {
               //
               // Read just the sub-volume
               //
               vf->readVolumeFileData(byteSwapFlag,
                                      vf->scaleSlope[i],
                                      vf->scaleOffset[i],
                                      dataFile);
            }
            
            //
            // return the volume to the user
            //
            volumesReadOut.push_back(vf);
            
            //
            // If only reading a single sub-volume
            //
            if (readingSingleSubVolume) {
               break;
            }
         }
      }
   }
   catch (FileException& e) {
      gzclose(dataFile);
      throw e;
   }
   
   //
   // Close the file
   //
   gzclose(dataFile);
}                          

/**
 * read the specified sub-volumes in a volume file.
 */
void 
VolumeFile::readFileNifti(const QString& fileNameIn, 
                          const int readSelection,
                          std::vector<VolumeFile*>& volumesReadOut) throw (FileException)
{

   //
   // Open the file
   //
   VolumeFile volumeRead;
   volumeRead.filename = fileNameIn;
   gzFile dataFile = gzopen(fileNameIn.toAscii().constData(), "rb");
   if (dataFile == NULL) {
      throw FileException(fileNameIn, "Unable to open with ZLIB for reading.");
   }
   
   //
   // The NIFTI header
   //
   NiftiFileHeader niftiFileHeader;
   niftiFileHeader.readHeader(dataFile, volumeRead.filename);
   nifti_1_header hdr = niftiFileHeader.getNiftiHeaderStruct();
   volumeRead.niftiSFormTransformationMatrix = niftiFileHeader.getSFormTransformationMatrix();
   volumeRead.niftiQFormTransformationMatrix = niftiFileHeader.getQFormTransformationMatrix();

   
   //
   // Do bytes need to be swapped ?
   //
   bool byteSwapFlag = niftiFileHeader.doesDataNeedByteSwapping();
   
   //
   // Offset of data
   //
   const long niftiReadDataOffset = static_cast<unsigned long>(hdr.vox_offset);
   
   //
   // Get volume dimensions
   //
   if (hdr.dim[0] < 3) {
      gzclose(dataFile);
      throw FileException(volumeRead.filename, "has less than 3 dimensions.");
   }
   
   volumeRead.dimensions[0] = hdr.dim[1];
   volumeRead.dimensions[1] = hdr.dim[2];
   volumeRead.dimensions[2] = hdr.dim[3];
   
   //
   // Multiple volumes in file ?
   //
   int numSubVols = 1;
   if (hdr.dim[0] > 3) {
      //
      // 4 dim volume is time series
      //
      if (hdr.dim[0] == 4) {
         numSubVols = hdr.dim[4];
      }
      else if (hdr.dim[0] == 5) {
         //
         // IF 5 dimensions and dim[4] = 1, then multiple values (subvolumes) per voxel
         //
         if (hdr.dim[4] == 1) {
            //
            // We'll consider RGB's to be a single volume
            //
            if ((hdr.dim[5] == 3) &&
                (hdr.datatype == NIFTI_TYPE_RGB24)) {
               hdr.dim[5] = 1;
            }
            if ((hdr.dim[5] == 3) &&
                (hdr.intent_code == NIFTI_INTENT_VECTOR)) {
               volumeRead.volumeType = VOLUME_TYPE_VECTOR;
            }
            numSubVols = hdr.dim[5];
         }
         else if (hdr.dim[4] > 1) {
            if (hdr.dim[5] > 1) {
               throw FileException(volumeRead.filename, "Multiple values per timepoint not supported.");
            }
            numSubVols = hdr.dim[4];
         }
      }
      else {
         throw FileException(volumeRead.filename, "Dimensions greater than 5 not supported.");
      }
   }
   volumeRead.initializeSubVolumes(numSubVols);
   
   //
   // Set the data type
   //
   switch (hdr.datatype) {
      case NIFTI_TYPE_UINT8:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_CHAR_UNSIGNED;
         break;
      case NIFTI_TYPE_INT8:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_CHAR;
         break;
      case NIFTI_TYPE_INT16:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_SHORT;
         break;
      case NIFTI_TYPE_UINT16:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_SHORT_UNSIGNED;
         break;
      case NIFTI_TYPE_INT32:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_INT;
         break;
      case NIFTI_TYPE_UINT32:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_INT_UNSIGNED;
         break;
      case NIFTI_TYPE_INT64:
         if (sizeof(long long) == 8) {
            volumeRead.voxelDataType = VOXEL_DATA_TYPE_LONG;
         }
         else {
            throw FileException("Volume contains 64-bit long long data type which "
                                "this computer does not support.");
         }
         break;
      case NIFTI_TYPE_UINT64:
         if (sizeof(unsigned long long) == 8) {
            volumeRead.voxelDataType = VOXEL_DATA_TYPE_LONG_UNSIGNED;
         }
         else {
            throw FileException("Volume contains 64-bit unsigned long long data type which "
                                "this computer does not support.");
         }
         break;
      case NIFTI_TYPE_FLOAT32:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_FLOAT;
         break;
      case NIFTI_TYPE_FLOAT64:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_DOUBLE;
         break;
      case NIFTI_TYPE_FLOAT128:
         gzclose(dataFile);
         throw FileException(volumeRead.filename, "FLOAT 128 type not supported.");
         break;
      case NIFTI_TYPE_RGB24:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED;
         break;
      case NIFTI_TYPE_COMPLEX64:
         gzclose(dataFile);
         throw FileException(volumeRead.filename, "COMPLEX 64 type not supported.");
         break;
      case NIFTI_TYPE_COMPLEX128:
         gzclose(dataFile);
         throw FileException(volumeRead.filename, "COMPLEX 128 type not supported.");
         break;
      case NIFTI_TYPE_COMPLEX256:
         gzclose(dataFile);
         throw FileException(volumeRead.filename, "COMPLEX 256 type not supported.");
         break;
      default:
         {
            gzclose(dataFile);
            std::ostringstream str;
            str << "datatype number "
                << hdr.datatype
                << " is unknown.";
            throw FileException(volumeRead.filename, str.str().c_str());
         }
         break;
   }
   
   //
   // Comment info
   //
   char comm[80];
   for (int m = 0; m < 79; m++) {
      comm[m] = hdr.descrip[m];
   }
   comm[79] = '\0';
   volumeRead.setHeaderTag(headerTagComment, comm);
   
   //
   // Spacing
   //
   volumeRead.spacing[0] = hdr.pixdim[1];
   volumeRead.spacing[1] = hdr.pixdim[2];
   volumeRead.spacing[2] = hdr.pixdim[3];

   
   //
   // default origin
   //
   volumeRead.origin[0] = 0.0;
   volumeRead.origin[1] = 0.0;
   volumeRead.origin[2] = 0.0;
   
   //
   // Default orientation
   //
   volumeRead.orientation[0] = ORIENTATION_UNKNOWN;
   volumeRead.orientation[1] = ORIENTATION_UNKNOWN;
   volumeRead.orientation[2] = ORIENTATION_UNKNOWN;
   
   for (int i = 0; i < numSubVols; i++) {
      volumeRead.scaleOffset[i] = hdr.scl_inter;
      volumeRead.scaleSlope[i]  = hdr.scl_slope;
   }

/*
   TransformationMatrix qformTM;
   bool qformTMValid = false;
   ORIENTATION qformOrientation[3] = {
      ORIENTATION_UNKNOWN,
      ORIENTATION_UNKNOWN,
      ORIENTATION_UNKNOWN
   };
*/

   //
   // Convert units to MilliMeters
   //
   float spacingScale = 1.0;
   switch (hdr.xyzt_units) {
      case NIFTI_UNITS_METER:
         spacingScale = 1000.0;
         break;
      case NIFTI_UNITS_MM:
         spacingScale = 1.0;
         break;
      case NIFTI_UNITS_MICRON:
         spacingScale = 0.001;
         break;
   }
   hdr.qoffset_x *= spacingScale;
   hdr.qoffset_y *= spacingScale;
   hdr.qoffset_z *= spacingScale;
   hdr.pixdim[1] *= spacingScale;
   hdr.pixdim[2] *= spacingScale;
   hdr.pixdim[3] *= spacingScale;

   //
   // sform_code take priority over qform_code
   //
   if (hdr.sform_code > 0) {
      //
      // Get coordinates of voxels
      //
      int ijk0[3] = { 0, 0, 0 };
      int ijk1[3] = { 1, 1, 1 };
      float xyz0[3], xyz1[3];
      niftiFileHeader.getVoxelCoordinate(ijk0,
                                         NiftiFileHeader::STEREOTAXIC_TYPE_SFORM,
                                         xyz0);
      niftiFileHeader.getVoxelCoordinate(ijk1,
                                         NiftiFileHeader::STEREOTAXIC_TYPE_SFORM,
                                         xyz1);

      volumeRead.origin[0] = xyz0[0] * spacingScale;
      volumeRead.origin[1] = xyz0[1] * spacingScale;
      volumeRead.origin[2] = xyz0[2] * spacingScale;
      volumeRead.spacing[0] = (xyz1[0] - xyz0[0]) * spacingScale;
      volumeRead.spacing[1] = (xyz1[1] - xyz0[1]) * spacingScale;
      volumeRead.spacing[2] = (xyz1[2] - xyz0[2]) * spacingScale;

      niftiFileHeader.getSFormOrientation(volumeRead.orientation);
   }
   else if (hdr.qform_code > 0) {
      //
      // Get coordinates of voxels
      //
      int ijk0[3] = { 0, 0, 0 };
      int ijk1[3] = { 1, 1, 1 };
      float xyz0[3], xyz1[3];
      niftiFileHeader.getVoxelCoordinate(ijk0,
                                         NiftiFileHeader::STEREOTAXIC_TYPE_QFORM,
                                         xyz0);
      niftiFileHeader.getVoxelCoordinate(ijk1,
                                         NiftiFileHeader::STEREOTAXIC_TYPE_QFORM,
                                         xyz1);

      volumeRead.origin[0] = xyz0[0] * spacingScale;
      volumeRead.origin[1] = xyz0[1] * spacingScale;
      volumeRead.origin[2] = xyz0[2] * spacingScale;
      volumeRead.spacing[0] = (xyz1[0] - xyz0[0]) * spacingScale;
      volumeRead.spacing[1] = (xyz1[1] - xyz0[1]) * spacingScale;
      volumeRead.spacing[2] = (xyz1[2] - xyz0[2]) * spacingScale;

      niftiFileHeader.getQFormOrientation(volumeRead.orientation);
   }
   else {
      //
      // Get coordinates of voxels
      //
      int ijk0[3] = { 0, 0, 0 };
      int ijk1[3] = { 1, 1, 1 };
      float xyz0[3], xyz1[3];
      niftiFileHeader.getVoxelCoordinate(ijk0,
                                         NiftiFileHeader::STEREOTAXIC_TYPE_NONE,
                                         xyz0);
      niftiFileHeader.getVoxelCoordinate(ijk1,
                                         NiftiFileHeader::STEREOTAXIC_TYPE_NONE,
                                         xyz1);

      volumeRead.origin[0] = xyz0[0] * spacingScale;
      volumeRead.origin[1] = xyz0[1] * spacingScale;
      volumeRead.origin[2] = xyz0[2] * spacingScale;
      volumeRead.spacing[0] = (xyz1[0] - xyz0[0]) * spacingScale;
      volumeRead.spacing[1] = (xyz1[1] - xyz0[1]) * spacingScale;
      volumeRead.spacing[2] = (xyz1[2] - xyz0[2]) * spacingScale;

      niftiFileHeader.getQFormOrientation(volumeRead.orientation);
   }
   
/*
   //
   // The origin and spacing are always ordered L/R, P/A, I/S in NIFTI even if
   // the data is in a different order such as AIL.  So, swap some values to
   // that they are correct for Caret.
   //
   float originTemp[3] = { volumeRead.origin[0], volumeRead.origin[1], volumeRead.origin[2] };
   float spacingTemp[3] = { volumeRead.spacing[0], volumeRead.spacing[1], volumeRead.spacing[2] };
   for (int i = 0; i < 3; i++) {
      int orientIndex = i;
      switch (volumeRead.orientation[i]) {
         case ORIENTATION_UNKNOWN:
            break;
         case ORIENTATION_RIGHT_TO_LEFT:
         case ORIENTATION_LEFT_TO_RIGHT:
            orientIndex = 0;
            break;
         case ORIENTATION_POSTERIOR_TO_ANTERIOR:
         case ORIENTATION_ANTERIOR_TO_POSTERIOR:
            orientIndex = 1;
            break;
         case ORIENTATION_INFERIOR_TO_SUPERIOR:
         case ORIENTATION_SUPERIOR_TO_INFERIOR:
            orientIndex = 2;
            break;
      }
      
      volumeRead.origin[i] = originTemp[orientIndex];
      volumeRead.spacing[i] = spacingTemp[orientIndex];
   }
*/
   //
   // Check the intent code
   //
   volumeRead.volumeType = VOLUME_TYPE_UNKNOWN;
   switch (hdr.intent_code) {
      case NIFTI_INTENT_LABEL:
         volumeRead.volumeType = VOLUME_TYPE_PAINT;
         break;
      case NIFTI_INTENT_VECTOR:
         if (hdr.dim[5] == 3) {
            volumeRead.volumeType = VOLUME_TYPE_VECTOR;
         }
         break;
   }

   //
   // Storage for study meta data links
   //
   std::vector<StudyMetaDataLinkSet> studyMetaDataLinkSets;
   
   //
   // Intention string and TR
   //
   niftiFileHeader.getNiftiIntentionInformation(volumeRead.niftiIntentCodeAndParamString,
                                                volumeRead.niftiIntentName);
   volumeRead.niftiIntentCode = hdr.intent_code;
   volumeRead.niftiIntentParameter1 = hdr.intent_p1;
   volumeRead.niftiIntentParameter2 = hdr.intent_p2;
   volumeRead.niftiIntentParameter3 = hdr.intent_p3;
   volumeRead.niftiTR = hdr.slice_duration;

   QString caretExtensionString = "";

   //
   // Read the extender
   //
   if (hdr.vox_offset >= 352) {
      nifti1_extender extender;
      const unsigned long extLength = sizeof(extender);
      const unsigned long numBytesRead = gzread(dataFile, (voidp)&extender, extLength);
      if (extLength == numBytesRead) {
         if (DebugControl::getDebugOn()) {
            std::cout << "NIFTI extension[0] " << static_cast<int>(extender.extension[0]) << std::endl;
         }
      }

      int extCount = 1;      
      z_off_t pos = gztell(dataFile);
      while (pos < hdr.vox_offset) {
         //
         // Read in the extension size and code
         //
         int extensionSize, extensionCode;
         if ((gzread(dataFile, (voidp)&extensionSize, sizeof(extensionSize)) != 4) ||
             (gzread(dataFile, (voidp)&extensionCode, sizeof(extensionCode)) != 4)) {
            std::cout << "WARNING: " << volumeRead.filename.toAscii().constData() << std::endl 
                      << "Problem reading extension " << extCount
                      << ".  This and all other extensions ignored." << std::endl;
            break;
         }
         else {
            if (byteSwapFlag) {
               ByteSwapping::swapBytes(&extensionSize, 1);
               ByteSwapping::swapBytes(&extensionCode, 1);
            }
            
            if (DebugControl::getDebugOn()) {
               std::cout << "Extension Size: " << extensionSize << std::endl;
               std::cout << "Extension Code: " << extensionCode << std::endl;
            }
                  
            //
            // Check the code
            //
            const int evenNum = (extensionCode / 2) * 2;
            if ((evenNum < 0) || (evenNum > 100)) {
               std::cout << "WARNING: " << volumeRead.filename.toAscii().constData() << std::endl 
                         << "Invalid extension code " << extensionCode << " for extension " 
                         << extCount
                         << ".  This and all other extensions ignored." << std::endl;
               break;
            }
            
            //
            // Check extension size
            //
            if ((extensionSize % 16) != 0) {
               std::cout << "WARNING: NIFTI extension (code " << extensionCode
                         << ") has size that is not a multiple of 16." << std::endl;
            }
            
            //
            // The 8-byte extension size/code is included in extensionSize
            //
            const int dataSize = extensionSize - 8; 
            
            if (dataSize > 0) {
               char* data = new char[dataSize + 1];
               if (gzread(dataFile, (voidp)data, dataSize) != dataSize) {
                  std::cout << "WARNING: " << volumeRead.filename.toAscii().constData() << std::endl 
                            << "Problem reading extension " << extCount
                            << " data.  This and all other extensions ignored." << std::endl;
                  break;
               }
               data[dataSize] = '\0';
               
               //
               // Is this the AFNI extension
               //
               if (extensionCode == 4) { // NIFTI_ECODE_AFNI) {
                  if (DebugControl::getDebugOn()) {
                     std::cout << "AFNI extension: " << data << std::endl;
                  }

                  //
                  // Process the NIFTI extension
                  //       
                  try {
                     volumeRead.afniHeader.readFromNiftiExtension(QString(data));
                  }
                  catch (FileException& e) {
                     throw FileException(volumeRead.filename, e.whatQString());
                  }

                  //
                  // Get the subvolume names
                  //
                  const AfniAttribute* nameAttr = volumeRead.afniHeader.getAttribute(AfniAttribute::NAME_BRICK_LABS);
                  if (nameAttr != NULL) {
                     std::vector<QString> names;
                     StringUtilities::token(nameAttr->getValue(), "~", names);
                     const unsigned int minNames = std::min(names.size(), volumeRead.subVolumeNames.size());
                     for (unsigned int i = 0; i < minNames; i++) {
                        volumeRead.subVolumeNames[i] = names[i];
                     }
                  }
                  
                  //
                  // Get the HISTORY_NOTE and use it as a comment
                  //
                  const AfniAttribute* commentAttr = volumeRead.afniHeader.getAttribute(AfniAttribute::NAME_HISTORY_NOTE);
                  if (commentAttr != NULL) {
                     volumeRead.setHeaderTag(headerTagComment, commentAttr->getValue());
                     volumeRead.setFileComment(commentAttr->getValue());
                  }
                  
                  //
                  // Get the region names (for paint volume)
                  //
                  const AfniAttribute* regionAttr = volumeRead.afniHeader.getAttribute(AfniAttribute::NAME_LUT_NAMES);
                  if (regionAttr != NULL) {
                     volumeRead.regionNames.clear();
                     StringUtilities::token(regionAttr->getValue(), "~", volumeRead.regionNames);
                  }
   
                  //
                  // Get the study metadata links
                  //
                  const AfniAttribute* mdAttr = volumeRead.afniHeader.getAttribute(AfniAttribute::NAME_CARET_METADATA_LINK);
                  if (mdAttr != NULL) {
                     StudyMetaDataLinkSet smdls;
                     std::vector<QString> md;
                     StringUtilities::token(mdAttr->getValue(), "~", md);
                     for (unsigned int m = 0; m < md.size(); m++) {
                        smdls.setLinkSetFromCodedText(md[m]);
                        studyMetaDataLinkSets.push_back(smdls);
                     }
                  }

                  //
                  // Get the PubMed ID
                  //
                  const AfniAttribute* pmidAttr = volumeRead.afniHeader.getAttribute(AfniAttribute::NAME_CARET_PUBMED_ID);
                  if (pmidAttr != NULL) {
                     volumeRead.setFilePubMedID(pmidAttr->getValue());
                  }
               }
               else if ((extensionCode == 30) ||
                        (extensionCode == 64)) {
                  caretExtensionString = QString(data);
               }
               
               delete[] data;
            }
         }
         
         extCount++;
         pos = gztell(dataFile);         
      }
   }
   
   //
   // Keep track of data files zipped status
   //   
   volumeRead.dataFileWasZippedFlag = (volumeRead.filename.right(3) == ".gz");

   //
   // If only reading header
   //
   if (readSelection == -2) {
      VolumeFile* vf = new VolumeFile;
      vf->copyVolumeData(volumeRead, false);
      vf->filename = volumeRead.filename;
      vf->dataFileName = volumeRead.dataFileName;
      volumesReadOut.push_back(vf);
      return;
   }
  
   //
   // Is this a NIFTI hdr/img volume file pair
   //
   if ((hdr.magic[0] == 'n') &&
       (hdr.magic[1] == 'i') &&
       (hdr.magic[2] == '1')) {
      gzclose(dataFile);
      //
      // Create the volume data file name
      //
      volumeRead.dataFileName = FileUtilities::filenameWithoutExtension(volumeRead.filename);
      volumeRead.dataFileName.append(".img");

      //
      // Data file might be gzipped
      //
      if (QFile::exists(volumeRead.dataFileName) == false) {
         QString zipName(volumeRead.dataFileName);
         zipName.append(".gz");
         if (QFile::exists(zipName)) {
            volumeRead.dataFileName = zipName;
         }
      }

      dataFile = gzopen(volumeRead.dataFileName.toAscii().constData(), "rb");
      if (dataFile == NULL) {
         throw FileException(fileNameIn, "Unable to open with ZLIB for reading.");
      }
   }
   
   try {         
      //
      // loop through and read the volume files
      //
      const int numSubs = volumeRead.subVolumeNames.size();
      for (int i = 0; i < numSubs; i++) {
         //
         // Determine if this sub volume should be read
         //
         bool readingSingleSubVolume = false;
         bool readIt = false;
         if (readSelection == VOLUME_READ_SELECTION_ALL) {
            readIt = true;
         }
         else if (readSelection == i) {
            readIt = true;
            readingSingleSubVolume = true;
         }
         
         if (readIt) {
            //
            // copy everything but voxel data from the first volume
            //
            VolumeFile* vf = NULL;
            vf = new VolumeFile;
            vf->copyVolumeData(volumeRead, false);
            vf->filename = volumeRead.filename;
            vf->dataFileName = volumeRead.dataFileName;
            
            //
            // Set the descriptive label
            //
            vf->descriptiveLabel = volumeRead.subVolumeNames[i];
            
            //
            // If only reading a single sub-volume
            //
            if (readingSingleSubVolume) {
               //
               // Read just the sub-volume
               //
               vf->readVolumeFileDataSubVolume(byteSwapFlag,
                                               vf->scaleSlope[i],
                                               vf->scaleOffset[i],
                                               niftiReadDataOffset,
                                               i,
                                               dataFile);
            }
            else {
               //
               // Read just the sub-volume
               //
               vf->readVolumeFileData(byteSwapFlag,
                                      vf->scaleSlope[i],
                                      vf->scaleOffset[i],
                                      dataFile);
            }
            
            if (i < static_cast<int>(studyMetaDataLinkSets.size())) {
               vf->setStudyMetaDataLinkSet(studyMetaDataLinkSets[i]);
            }
            
            //
            // Apply qform transformation
            //
/*
            if (qformTMValid) {
               vf->applyTransformationMatrix(qformTM);
            }
*/
            //
            // return the volume to the user
            //
            volumesReadOut.push_back(vf);
            
            //
            // If only reading a single sub-volume
            //
            if (readingSingleSubVolume) {
               break;
            }
         }
      }

      if (caretExtensionString.isEmpty() == false) {
         NiftiCaretExtension caretExtension(volumesReadOut, NULL, 5);
         QString filesComment = "";
         caretExtension.readAndApplyExtensionToVolumes(caretExtensionString,
                                                       filesComment);
      }
   }
   catch (FileException& e) {
      gzclose(dataFile);
      throw e;
   }

   //
   // Close the file
   //
   gzclose(dataFile);
}                          

/*=== 27 July 2009
 **
 * read the specified sub-volumes in a volume file.
 *
void
VolumeFile::readFileNifti(const QString& fileNameIn,
                          const int readSelection,
                          std::vector<VolumeFile*>& volumesReadOut) throw (FileException)
{

   //
   // Open the file
   //
   VolumeFile volumeRead;
   volumeRead.filename = fileNameIn;
   gzFile dataFile = gzopen(fileNameIn.toAscii().constData(), "rb");
   if (dataFile == NULL) {
      throw FileException(fileNameIn, "Unable to open with ZLIB for reading.");
   }

   //
   // The NIFTI header
   //
   nifti_1_header hdr;

   //
   // Read the NIFTI header and close file after reading header
   //
   const unsigned long headerSize = sizeof(hdr);
   const unsigned long numBytesRead = gzread(dataFile, (voidp)&hdr, headerSize);
   if (numBytesRead != headerSize) {
      gzclose(dataFile);
      std::ostringstream str;
      str << "Tried to read "
          << headerSize
          << " bytes from header.\n"
          << "Only read "
          << numBytesRead
          << ".";
      throw FileException(volumeRead.filename, str.str().c_str());
   }

   //
   // Make sure it is a NIFTI file
   //
   const int version = NIFTI_VERSION(hdr);
   switch (version) {
      case 0:
         gzclose(dataFile);
         throw FileException(volumeRead.filename, "Is not a NIFTI volume file.");
         break;
      case 1:
         break;
      default:
         {
            gzclose(dataFile);
            std::ostringstream str;
            str << "Is an invalid NIFTI version: "
                << version
                << ".";
            throw FileException(volumeRead.filename, str.str().c_str());
         }
         break;
   }

   //
   // Do bytes need to be swapped ?
   //
   bool byteSwapFlag = false;
   if (NIFTI_NEEDS_SWAP(hdr)) {
      byteSwapFlag = true;

      ByteSwapping::swapBytes(&hdr.sizeof_hdr, 1);
      ByteSwapping::swapBytes(&hdr.extents, 1);
      ByteSwapping::swapBytes(&hdr.session_error, 1);
      ByteSwapping::swapBytes(hdr.dim, 8);
      ByteSwapping::swapBytes(&hdr.intent_p1, 1);
      ByteSwapping::swapBytes(&hdr.intent_p2, 1);
      ByteSwapping::swapBytes(&hdr.intent_p3, 1);
      ByteSwapping::swapBytes(&hdr.intent_code, 1);
      ByteSwapping::swapBytes(&hdr.datatype, 1);
      ByteSwapping::swapBytes(&hdr.bitpix, 1);
      ByteSwapping::swapBytes(&hdr.slice_start, 1);
      ByteSwapping::swapBytes(hdr.pixdim, 8);
      ByteSwapping::swapBytes(&hdr.vox_offset, 1);
      ByteSwapping::swapBytes(&hdr.scl_slope, 1);
      ByteSwapping::swapBytes(&hdr.scl_inter, 1);
      ByteSwapping::swapBytes(&hdr.slice_end, 1);
      ByteSwapping::swapBytes(&hdr.cal_max, 1);
      ByteSwapping::swapBytes(&hdr.cal_min, 1);
      ByteSwapping::swapBytes(&hdr.slice_duration, 1);
      ByteSwapping::swapBytes(&hdr.toffset, 1);
      ByteSwapping::swapBytes(&hdr.glmax, 1);
      ByteSwapping::swapBytes(&hdr.glmin, 1);
      ByteSwapping::swapBytes(&hdr.qform_code, 1);
      ByteSwapping::swapBytes(&hdr.sform_code, 1);
      ByteSwapping::swapBytes(&hdr.quatern_b, 1);
      ByteSwapping::swapBytes(&hdr.quatern_c, 1);
      ByteSwapping::swapBytes(&hdr.quatern_d, 1);
      ByteSwapping::swapBytes(&hdr.qoffset_x, 1);
      ByteSwapping::swapBytes(&hdr.qoffset_y, 1);
      ByteSwapping::swapBytes(&hdr.qoffset_z, 1);
      ByteSwapping::swapBytes(hdr.srow_x, 4);
      ByteSwapping::swapBytes(hdr.srow_y, 4);
      ByteSwapping::swapBytes(hdr.srow_z, 4);
   }

   //
   // Offset of data
   //
   const long niftiReadDataOffset = static_cast<unsigned long>(hdr.vox_offset);

   //
   // Get volume dimensions
   //
   if (hdr.dim[0] < 3) {
      gzclose(dataFile);
      throw FileException(volumeRead.filename, "has less than 3 dimensions.");
   }

   volumeRead.dimensions[0] = hdr.dim[1];
   volumeRead.dimensions[1] = hdr.dim[2];
   volumeRead.dimensions[2] = hdr.dim[3];

   //
   // Multiple volumes in file ?
   //
   int numSubVols = 1;
   if (hdr.dim[0] > 3) {
      //
      // 4 dim volume is time series
      //
      if (hdr.dim[0] == 4) {
         numSubVols = hdr.dim[4];
      }
      else if (hdr.dim[0] == 5) {
         //
         // IF 5 dimensions and dim[4] = 1, then multiple values (subvolumes) per voxel
         //
         if (hdr.dim[4] == 1) {
            //
            // We'll consider RGB's to be a single volume
            //
            if ((hdr.dim[5] == 3) &&
                (hdr.datatype == NIFTI_TYPE_RGB24)) {
               hdr.dim[5] = 1;
            }
            if ((hdr.dim[5] == 3) &&
                (hdr.intent_code == NIFTI_INTENT_VECTOR)) {
               volumeRead.volumeType = VOLUME_TYPE_VECTOR;
            }
            numSubVols = hdr.dim[5];
         }
         else if (hdr.dim[4] > 1) {
            if (hdr.dim[5] > 1) {
               throw FileException(volumeRead.filename, "Multiple values per timepoint not supported.");
            }
            numSubVols = hdr.dim[4];
         }
      }
      else {
         throw FileException(volumeRead.filename, "Dimensions greater than 5 not supported.");
      }
   }
   volumeRead.initializeSubVolumes(numSubVols);

   //
   // Set the data type
   //
   switch (hdr.datatype) {
      case NIFTI_TYPE_UINT8:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_CHAR_UNSIGNED;
         break;
      case NIFTI_TYPE_INT8:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_CHAR;
         break;
      case NIFTI_TYPE_INT16:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_SHORT;
         break;
      case NIFTI_TYPE_UINT16:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_SHORT_UNSIGNED;
         break;
      case NIFTI_TYPE_INT32:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_INT;
         break;
      case NIFTI_TYPE_UINT32:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_INT_UNSIGNED;
         break;
      case NIFTI_TYPE_INT64:
         if (sizeof(long long) == 8) {
            volumeRead.voxelDataType = VOXEL_DATA_TYPE_LONG;
         }
         else {
            throw FileException("Volume contains 64-bit long long data type which "
                                "this computer does not support.");
         }
         break;
      case NIFTI_TYPE_UINT64:
         if (sizeof(unsigned long long) == 8) {
            volumeRead.voxelDataType = VOXEL_DATA_TYPE_LONG_UNSIGNED;
         }
         else {
            throw FileException("Volume contains 64-bit unsigned long long data type which "
                                "this computer does not support.");
         }
         break;
      case NIFTI_TYPE_FLOAT32:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_FLOAT;
         break;
      case NIFTI_TYPE_FLOAT64:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_DOUBLE;
         break;
      case NIFTI_TYPE_FLOAT128:
         gzclose(dataFile);
         throw FileException(volumeRead.filename, "FLOAT 128 type not supported.");
         break;
      case NIFTI_TYPE_RGB24:
         volumeRead.voxelDataType = VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED;
         break;
      case NIFTI_TYPE_COMPLEX64:
         gzclose(dataFile);
         throw FileException(volumeRead.filename, "COMPLEX 64 type not supported.");
         break;
      case NIFTI_TYPE_COMPLEX128:
         gzclose(dataFile);
         throw FileException(volumeRead.filename, "COMPLEX 128 type not supported.");
         break;
      case NIFTI_TYPE_COMPLEX256:
         gzclose(dataFile);
         throw FileException(volumeRead.filename, "COMPLEX 256 type not supported.");
         break;
      default:
         {
            gzclose(dataFile);
            std::ostringstream str;
            str << "datatype number "
                << hdr.datatype
                << " is unknown.";
            throw FileException(volumeRead.filename, str.str().c_str());
         }
         break;
   }

   //
   // Comment info
   //
   char comm[80];
   for (int m = 0; m < 79; m++) {
      comm[m] = hdr.descrip[m];
   }
   comm[79] = '\0';
   volumeRead.setHeaderTag(headerTagComment, comm);

   //
   // Convert units to MilliMeters
   //
   float spacingScale = 1.0;
   switch (hdr.xyzt_units) {
      case NIFTI_UNITS_METER:
         spacingScale = 1000.0;
         break;
      case NIFTI_UNITS_MM:
         spacingScale = 1.0;
         break;
      case NIFTI_UNITS_MICRON:
         spacingScale = 0.001;
         break;
   }
   hdr.qoffset_x *= spacingScale;
   hdr.qoffset_y *= spacingScale;
   hdr.qoffset_z *= spacingScale;
   hdr.pixdim[1] *= spacingScale;
   hdr.pixdim[2] *= spacingScale;
   hdr.pixdim[3] *= spacingScale;

   //
   // Spacing
   //
   volumeRead.spacing[0] = hdr.pixdim[1];
   volumeRead.spacing[1] = hdr.pixdim[2];
   volumeRead.spacing[2] = hdr.pixdim[3];


   //
   // default origin
   //
   volumeRead.origin[0] = 0.0;
   volumeRead.origin[1] = 0.0;
   volumeRead.origin[2] = 0.0;

   //
   // Default orientation
   //
   volumeRead.orientation[0] = ORIENTATION_UNKNOWN;
   volumeRead.orientation[1] = ORIENTATION_UNKNOWN;
   volumeRead.orientation[2] = ORIENTATION_UNKNOWN;

   for (int i = 0; i < numSubVols; i++) {
      volumeRead.scaleOffset[i] = hdr.scl_inter;
      volumeRead.scaleSlope[i]  = hdr.scl_slope;
   }

   TransformationMatrix qformTM;
   bool qformTMValid = false;
   ORIENTATION qformOrientation[3] = {
      ORIENTATION_UNKNOWN,
      ORIENTATION_UNKNOWN,
      ORIENTATION_UNKNOWN
   };

   //
   // sform_code take priority over qform_code
   //
   if (hdr.sform_code > 0) {
      qformTMValid = false;

      float x[2], y[2], z[2];
      for (int i = 0; i < 2; i++) {
         const int j = i;
         const int k = i;
         x[i] = hdr.srow_x[0] * i + hdr.srow_x[1] * j + hdr.srow_x[2] * k + hdr.srow_x[3];
         y[i] = hdr.srow_y[0] * i + hdr.srow_y[1] * j + hdr.srow_y[2] * k + hdr.srow_y[3];
         z[i] = hdr.srow_z[0] * i + hdr.srow_z[1] * j + hdr.srow_z[2] * k + hdr.srow_z[3];
      }
      //
      // Convert units to MilliMeters
      //
      float spacingScale = 1.0;
      switch (hdr.xyzt_units) {
         case NIFTI_UNITS_METER:
            spacingScale = 1000.0;
            break;
         case NIFTI_UNITS_MM:
            spacingScale = 1.0;
            break;
         case NIFTI_UNITS_MICRON:
            spacingScale = 0.001;
            break;
      }
      volumeRead.origin[0] = x[0] * spacingScale;
      volumeRead.origin[1] = y[0] * spacingScale;
      volumeRead.origin[2] = z[0] * spacingScale;
      volumeRead.spacing[0] = (x[1] - x[0]) * spacingScale;
      volumeRead.spacing[1] = (y[1] - y[0]) * spacingScale;
      volumeRead.spacing[2] = (z[1] - z[0]) * spacingScale;


      //
      // NIFTI origin is in the center of the voxel
      // so move the origin to the corner of voxel
      //
      //volumeRead.origin[0] -= volumeRead.spacing[0] * 0.5;
      //volumeRead.origin[1] -= volumeRead.spacing[1] * 0.5;
      //volumeRead.origin[2] -= volumeRead.spacing[2] * 0.5;

      NiftiHelper::mat44 m;
      m.m[0][0] = hdr.srow_x[0];
      m.m[0][1] = hdr.srow_x[1];
      m.m[0][2] = hdr.srow_x[2];
      m.m[0][3] = hdr.srow_x[3];
      m.m[1][0] = hdr.srow_y[0];
      m.m[1][1] = hdr.srow_y[1];
      m.m[1][2] = hdr.srow_y[2];
      m.m[1][3] = hdr.srow_y[3];
      m.m[2][0] = hdr.srow_z[0];
      m.m[2][1] = hdr.srow_z[1];
      m.m[2][2] = hdr.srow_z[2];
      m.m[2][3] = hdr.srow_z[3];
      m.m[3][0] = 0.0;
      m.m[3][1] = 0.0;
      m.m[3][2] = 0.0;
      m.m[3][3] = 1.0;
      NiftiHelper::mat44ToCaretOrientation(m,
                                           volumeRead.orientation[0],
                                           volumeRead.orientation[1],
                                           volumeRead.orientation[2]);
   }
   else if (hdr.qform_code > 0) {
      volumeRead.origin[0] = hdr.qoffset_x;
      volumeRead.origin[1] = hdr.qoffset_y;
      volumeRead.origin[2] = hdr.qoffset_z;

      float qfac = (hdr.pixdim[0] < 0.0) ? -1.0 : 1.0 ;  // left-handedness?
      NiftiHelper::mat44 m =
         NiftiHelper::nifti_quatern_to_mat44(hdr.quatern_b, hdr.quatern_c, hdr.quatern_c,
                                             0.0, 0.0, 0.0,
                                             1.0, 1.0, 1.0,
                                             qfac);

      qformTMValid = true;
      for (int i = 0; i < 4; i++) {
         for (int j = 0; j < 4; j++) {
            qformTM.setMatrixElement(i, j, m.m[i][j]);
         }
      }
      for (int j = 0; j < 3; j++) {
         float f = qformTM.getMatrixElement(j, 3);
         f = -f;
         qformTM.setMatrixElement(j, 3, f);
      }
      qformTM.transpose();


      //
      // Force correct sign for spacing
      //
      for (int i = 0; i < 3; i++) {
         if (volumeRead.origin[i] < 0.0) {
            volumeRead.spacing[i] = std::fabs(volumeRead.spacing[i]);
         }
         else {
            volumeRead.spacing[i] = -std::fabs(volumeRead.spacing[i]);
         }
      }

      //
      // Set orientation
      //
      volumeRead.orientation[0] = ORIENTATION_LEFT_TO_RIGHT;
      volumeRead.orientation[1] = ORIENTATION_POSTERIOR_TO_ANTERIOR;
      volumeRead.orientation[2] = ORIENTATION_INFERIOR_TO_SUPERIOR;

      NiftiHelper::mat44ToCaretOrientation(m,
                                           qformOrientation[0],
                                           qformOrientation[1],
                                           qformOrientation[2]);
   }


   //
   // The origin and spacing are always ordered L/R, P/A, I/S in NIFTI even if
   // the data is in a different order such as AIL.  So, swap some values to
   // that they are correct for Caret.
   //
   float originTemp[3] = { volumeRead.origin[0], volumeRead.origin[1], volumeRead.origin[2] };
   float spacingTemp[3] = { volumeRead.spacing[0], volumeRead.spacing[1], volumeRead.spacing[2] };
   for (int i = 0; i < 3; i++) {
      int orientIndex = i;
      switch (volumeRead.orientation[i]) {
         case ORIENTATION_UNKNOWN:
            break;
         case ORIENTATION_RIGHT_TO_LEFT:
         case ORIENTATION_LEFT_TO_RIGHT:
            orientIndex = 0;
            break;
         case ORIENTATION_POSTERIOR_TO_ANTERIOR:
         case ORIENTATION_ANTERIOR_TO_POSTERIOR:
            orientIndex = 1;
            break;
         case ORIENTATION_INFERIOR_TO_SUPERIOR:
         case ORIENTATION_SUPERIOR_TO_INFERIOR:
            orientIndex = 2;
            break;
      }

      volumeRead.origin[i] = originTemp[orientIndex];
      volumeRead.spacing[i] = spacingTemp[orientIndex];
   }

   //
   // Check the intent code
   //
   volumeRead.volumeType = VOLUME_TYPE_UNKNOWN;
   switch (hdr.intent_code) {
      case NIFTI_INTENT_LABEL:
         volumeRead.volumeType = VOLUME_TYPE_PAINT;
         break;
      case NIFTI_INTENT_VECTOR:
         if (hdr.dim[5] == 3) {
            volumeRead.volumeType = VOLUME_TYPE_VECTOR;
         }
         break;
   }

   //
   // Storage for study meta data links
   //
   std::vector<StudyMetaDataLinkSet> studyMetaDataLinkSets;

   //
   // Intention string and TR
   //
   NiftiHelper::getNiftiIntentionInformation(hdr,
                                             volumeRead.niftiIntentCodeAndParamString,
                                             volumeRead.niftiIntentName);
   volumeRead.niftiIntentCode = hdr.intent_code;
   volumeRead.niftiIntentParameter1 = hdr.intent_p1;
   volumeRead.niftiIntentParameter2 = hdr.intent_p2;
   volumeRead.niftiIntentParameter3 = hdr.intent_p3;
   volumeRead.niftiTR = hdr.slice_duration;

   //
   // Read the extender
   //
   if (hdr.vox_offset >= 352) {
      nifti1_extender extender;
      const unsigned long extLength = sizeof(extender);
      const unsigned long numBytesRead = gzread(dataFile, (voidp)&extender, extLength);
      if (extLength == numBytesRead) {
         if (DebugControl::getDebugOn()) {
            std::cout << "NIFTI extension[0] " << static_cast<int>(extender.extension[0]) << std::endl;
         }
      }

      int extCount = 1;
      z_off_t pos = gztell(dataFile);
      while (pos < hdr.vox_offset) {
         //
         // Read in the extension size and code
         //
         int extensionSize, extensionCode;
         if ((gzread(dataFile, (voidp)&extensionSize, sizeof(extensionSize)) != 4) ||
             (gzread(dataFile, (voidp)&extensionCode, sizeof(extensionCode)) != 4)) {
            std::cout << "WARNING: " << volumeRead.filename.toAscii().constData() << std::endl
                      << "Problem reading extension " << extCount
                      << ".  This and all other extensions ignored." << std::endl;
            break;
         }
         else {
            if (byteSwapFlag) {
               ByteSwapping::swapBytes(&extensionSize, 1);
               ByteSwapping::swapBytes(&extensionCode, 1);
            }

            if (DebugControl::getDebugOn()) {
               std::cout << "Extension Size: " << extensionSize << std::endl;
               std::cout << "Extension Code: " << extensionCode << std::endl;
            }

            //
            // Check the code
            //
            const int evenNum = (extensionCode / 2) * 2;
            if ((evenNum < 0) || (evenNum > 100)) {
               std::cout << "WARNING: " << volumeRead.filename.toAscii().constData() << std::endl
                         << "Invalid extension code " << extensionCode << " for extension "
                         << extCount
                         << ".  This and all other extensions ignored." << std::endl;
               break;
            }

            //
            // Check extension size
            //
            if ((extensionSize % 16) != 0) {
               std::cout << "WARNING: NIFTI extension (code " << extensionCode
                         << ") has size that is not a multiple of 16." << std::endl;
            }

            //
            // The 8-byte extension size/code is included in extensionSize
            //
            const int dataSize = extensionSize - 8;

            if (dataSize > 0) {
               char* data = new char[dataSize + 1];
               if (gzread(dataFile, (voidp)data, dataSize) != dataSize) {
                  std::cout << "WARNING: " << volumeRead.filename.toAscii().constData() << std::endl
                            << "Problem reading extension " << extCount
                            << " data.  This and all other extensions ignored." << std::endl;
                  break;
               }
               data[dataSize] = '\0';

               //
               // Is this the AFNI extension
               //
               if (extensionCode == 4) { // NIFTI_ECODE_AFNI) {
                  if (DebugControl::getDebugOn()) {
                     std::cout << "AFNI extension: " << data << std::endl;
                  }

                  //
                  // Process the NIFTI extension
                  //
                  try {
                     volumeRead.afniHeader.readFromNiftiExtension(QString(data));
                  }
                  catch (FileException& e) {
                     throw FileException(volumeRead.filename, e.whatQString());
                  }

                  //
                  // Get the subvolume names
                  //
                  const AfniAttribute* nameAttr = volumeRead.afniHeader.getAttribute(AfniAttribute::NAME_BRICK_LABS);
                  if (nameAttr != NULL) {
                     std::vector<QString> names;
                     StringUtilities::token(nameAttr->getValue(), "~", names);
                     const unsigned int minNames = std::min(names.size(), volumeRead.subVolumeNames.size());
                     for (unsigned int i = 0; i < minNames; i++) {
                        volumeRead.subVolumeNames[i] = names[i];
                     }
                  }

                  //
                  // Get the HISTORY_NOTE and use it as a comment
                  //
                  const AfniAttribute* commentAttr = volumeRead.afniHeader.getAttribute(AfniAttribute::NAME_HISTORY_NOTE);
                  if (commentAttr != NULL) {
                     volumeRead.setHeaderTag(headerTagComment, commentAttr->getValue());
                     volumeRead.setFileComment(commentAttr->getValue());
                  }

                  //
                  // Get the region names (for paint volume)
                  //
                  const AfniAttribute* regionAttr = volumeRead.afniHeader.getAttribute(AfniAttribute::NAME_LUT_NAMES);
                  if (regionAttr != NULL) {
                     volumeRead.regionNames.clear();
                     StringUtilities::token(regionAttr->getValue(), "~", volumeRead.regionNames);
                  }

                  //
                  // Get the study metadata links
                  //
                  const AfniAttribute* mdAttr = volumeRead.afniHeader.getAttribute(AfniAttribute::NAME_CARET_METADATA_LINK);
                  if (mdAttr != NULL) {
                     StudyMetaDataLinkSet smdls;
                     std::vector<QString> md;
                     StringUtilities::token(mdAttr->getValue(), "~", md);
                     for (unsigned int m = 0; m < md.size(); m++) {
                        smdls.setLinkSetFromCodedText(md[m]);
                        studyMetaDataLinkSets.push_back(smdls);
                     }
                  }

                  //
                  // Get the PubMed ID
                  //
                  const AfniAttribute* pmidAttr = volumeRead.afniHeader.getAttribute(AfniAttribute::NAME_CARET_PUBMED_ID);
                  if (pmidAttr != NULL) {
                     volumeRead.setFilePubMedID(pmidAttr->getValue());
                  }
               }

               delete[] data;
            }
         }

         extCount++;
         pos = gztell(dataFile);
      }
   }

   //
   // Keep track of data files zipped status
   //
   volumeRead.dataFileWasZippedFlag = (volumeRead.filename.right(3) == ".gz");

   //
   // If only reading header
   //
   if (readSelection == -2) {
      VolumeFile* vf = new VolumeFile;
      vf->copyVolumeData(volumeRead, false);
      vf->filename = volumeRead.filename;
      vf->dataFileName = volumeRead.dataFileName;
      volumesReadOut.push_back(vf);
      return;
   }

   //
   // Is this a NIFTI hdr/img volume file pair
   //
   if ((hdr.magic[0] == 'n') &&
       (hdr.magic[1] == 'i') &&
       (hdr.magic[2] == '1')) {
      gzclose(dataFile);
      //
      // Create the volume data file name
      //
      volumeRead.dataFileName = FileUtilities::filenameWithoutExtension(volumeRead.filename);
      volumeRead.dataFileName.append(".img");

      //
      // Data file might be gzipped
      //
      if (QFile::exists(volumeRead.dataFileName) == false) {
         QString zipName(volumeRead.dataFileName);
         zipName.append(".gz");
         if (QFile::exists(zipName)) {
            volumeRead.dataFileName = zipName;
         }
      }

      dataFile = gzopen(volumeRead.dataFileName.toAscii().constData(), "rb");
      if (dataFile == NULL) {
         throw FileException(fileNameIn, "Unable to open with ZLIB for reading.");
      }
   }

   try {
      //
      // loop through and read the volume files
      //
      const int numSubs = volumeRead.subVolumeNames.size();
      for (int i = 0; i < numSubs; i++) {
         //
         // Determine if this sub volume should be read
         //
         bool readingSingleSubVolume = false;
         bool readIt = false;
         if (readSelection == VOLUME_READ_SELECTION_ALL) {
            readIt = true;
         }
         else if (readSelection == i) {
            readIt = true;
            readingSingleSubVolume = true;
         }

         if (readIt) {
            //
            // copy everything but voxel data from the first volume
            //
            VolumeFile* vf = NULL;
            vf = new VolumeFile;
            vf->copyVolumeData(volumeRead, false);
            vf->filename = volumeRead.filename;
            vf->dataFileName = volumeRead.dataFileName;

            //
            // Set the descriptive label
            //
            vf->descriptiveLabel = volumeRead.subVolumeNames[i];

            //
            // If only reading a single sub-volume
            //
            if (readingSingleSubVolume) {
               //
               // Read just the sub-volume
               //
               vf->readVolumeFileDataSubVolume(byteSwapFlag,
                                               vf->scaleSlope[i],
                                               vf->scaleOffset[i],
                                               niftiReadDataOffset,
                                               i,
                                               dataFile);
            }
            else {
               //
               // Read just the sub-volume
               //
               vf->readVolumeFileData(byteSwapFlag,
                                      vf->scaleSlope[i],
                                      vf->scaleOffset[i],
                                      dataFile);
            }

            if (i < static_cast<int>(studyMetaDataLinkSets.size())) {
               vf->setStudyMetaDataLinkSet(studyMetaDataLinkSets[i]);
            }

            //
            // Apply qform transformation
            //
            if (qformTMValid) {
               vf->applyTransformationMatrix(qformTM);
            }

            //
            // return the volume to the user
            //
            volumesReadOut.push_back(vf);

            //
            // If only reading a single sub-volume
            //
            if (readingSingleSubVolume) {
               break;
            }
         }
      }
   }
   catch (FileException& e) {
      gzclose(dataFile);
      throw e;
   }

   //
   // Close the file
   //
   gzclose(dataFile);
}
===*/

/**
 * read the specified sub-volumes in an SPM volume file.
 */
void 
VolumeFile::readFileSpm(const QString& fileNameIn, 
                        const int readSelection,
                        std::vector<VolumeFile*>& volumesReadOut,
                        const bool rightIsOnLeft) throw (FileException)
{
   readFileAnalyze(fileNameIn,
                   readSelection,
                   volumesReadOut,
                   true);
                   
   if (rightIsOnLeft) {
      for (unsigned int i = 0; i < volumesReadOut.size(); i++) {
         volumesReadOut[i]->flip(VOLUME_AXIS_X);
      }
   }
}                          

/**
 * read the specified sub-volumes in a Wash U. volume file.
 */
void 
VolumeFile::readFileWuNil(const QString& fileNameIn, 
                          const int readSelection,
                          std::vector<VolumeFile*>& volumesReadOut) throw (FileException)
{
   VolumeFile volumeRead;
   volumeRead.filename = fileNameIn;
   
   QFile file(fileNameIn);
   if (file.open(QIODevice::ReadOnly) == false) {
      throw FileException(fileNameIn, file.errorString());
   }
   QTextStream stream(&file);
   
   volumeRead.wunilHeader.clear();
   volumeRead.wunilHeader.readHeader(file, stream);
   
   QString errorMessage;
   
   //
   // Only support float in RAI orientation
   //
   volumeRead.voxelDataType = VOXEL_DATA_TYPE_FLOAT;
   volumeRead.orientation[0] = ORIENTATION_RIGHT_TO_LEFT;
   volumeRead.orientation[1] = ORIENTATION_ANTERIOR_TO_POSTERIOR;
   volumeRead.orientation[2] = ORIENTATION_INFERIOR_TO_SUPERIOR;
   
   //
   // Data type of voxels
   //
   WuNilAttribute* dataType = volumeRead.wunilHeader.getAttribute(WuNilAttribute::NAME_NUMBER_FORMAT);
   if (dataType != NULL) {
      if (dataType->getValue() != "float") {
         errorMessage.append("Only \"number format\" of float supported.\n");
      }
   }
   else {
      errorMessage.append("required attribute \"number format\" not found.\n");
   }
   
   //
   // orientation
   //
   int ifhOrientation = 2;
   WuNilAttribute* orientAttr = volumeRead.wunilHeader.getAttribute(WuNilAttribute::NAME_ORIENTATION);
   if (orientAttr != NULL) {
      std::vector<int> orient;
      orientAttr->getValue(orient);
      if (orient.size() < 1) {
         errorMessage.append("Required attribute \"orientation\" does not have a value.\n");
      }
      else {
         if ((orient[0] != 2) && (orient[0] != 3) && (orient[0] != 4) && (orient[0] != 100)) {
            std::ostringstream str;
            str << "\"orientation\" of " << orient[0] << " not supported";
            errorMessage.append(str.str().c_str());
            errorMessage.append("\n");
         }
         ifhOrientation = orient[0];
      }
   }
   else {
      errorMessage.append("Required attribute \"orientation\" not found.\n");
   }
   
   //
   // X dimension
   //
   int dimensions[3] = { 0, 0, 0 };
   WuNilAttribute* xattr= volumeRead.wunilHeader.getAttribute(WuNilAttribute::NAME_MATRIX_SIZE_1);
   if (xattr != NULL) {
      std::vector<int> dim;
      xattr->getValue(dim);
      if (dim.size() == 0) {
         errorMessage.append("Required attribute \"matrix size [1]\" does not have a value.\n");
      }
      else {
         dimensions[0] = dim[0];
      }
   }
   else {
      errorMessage.append("Required attribute \"matrix size [1]\" not found.\n");
   }
   
   //
   // Y dimension
   //
   WuNilAttribute* yattr= volumeRead.wunilHeader.getAttribute(WuNilAttribute::NAME_MATRIX_SIZE_2);
   if (yattr != NULL) {
      std::vector<int> dim;
      yattr->getValue(dim);
      if (dim.size() == 0) {
         errorMessage.append("Required attribute \"matrix size [2]\" does not have a value.\n");
      }
      else {
         dimensions[1] = dim[0];
      }
   }
   else {
      errorMessage.append("Required attribute \"matrix size [2]\" not found.\n");
   }
   
   //
   // Z dimension
   //
   WuNilAttribute* zattr= volumeRead.wunilHeader.getAttribute(WuNilAttribute::NAME_MATRIX_SIZE_3);
   if (zattr != NULL) {
      std::vector<int> dim;
      zattr->getValue(dim);
      if (dim.size() == 0) {
         errorMessage.append("Required attribute \"matrix size [2]\" does not have a value.\n");
      }
      else {
         dimensions[2] = dim[0];
      }
   }
   else {
      errorMessage.append("Required attribute \"matrix size [2]\" not found.\n");
   }
   volumeRead.setDimensions(dimensions);
   
   //
   //  Number of subvolumes
   //
   WuNilAttribute* numsubattr= volumeRead.wunilHeader.getAttribute(WuNilAttribute::NAME_MATRIX_SIZE_4);
   if (numsubattr != NULL) {
      std::vector<int> dim;
      numsubattr->getValue(dim);
      if (dim.size() == 0) {
         errorMessage.append("Required attribute \"matrix size [4]\" does not have a value.\n");
      }
      else {
         volumeRead.initializeSubVolumes(dim[0]);
      }
   }
   else {
      errorMessage.append("Required attribute \"matrix size [4]\" not found.\n");
   }

   float mmppix[3];
   WuNilAttribute* mmppixAttr = volumeRead.wunilHeader.getAttribute(WuNilAttribute::NAME_MMPPIX);
   if (mmppixAttr != NULL) {
      std::vector<float> mmppixVector;
      mmppixAttr->getValue(mmppixVector);
      if (mmppixVector.size() < 3) {
         errorMessage.append("Required attribute \"mmppix\" does not have 3 elements\n");
      }
      else {
         mmppix[0] = mmppixVector[0];
         mmppix[1] = mmppixVector[1];
         mmppix[2] = mmppixVector[2];
      }
   }
   else {
      float scale0 = 1.0;
      WuNilAttribute* scale0Att = volumeRead.wunilHeader.getAttribute(WuNilAttribute::NAME_SCALING_FACTOR_1);
      if (scale0Att != NULL) {
         std::vector<float> scaleVector;
         scale0Att->getValue(scaleVector);
         if (scaleVector.empty() == false) {
            scale0 = scaleVector[0];
         }
      }
      float scale1 = 1.0;
      WuNilAttribute* scale1Att = volumeRead.wunilHeader.getAttribute(WuNilAttribute::NAME_SCALING_FACTOR_2);
      if (scale1Att != NULL) {
         std::vector<float> scaleVector;
         scale1Att->getValue(scaleVector);
         if (scaleVector.empty() == false) {
            scale1 = scaleVector[0];
         }
      }
      float scale2 = 1.0;
      WuNilAttribute* scale2Att = volumeRead.wunilHeader.getAttribute(WuNilAttribute::NAME_SCALING_FACTOR_3);
      if (scale2Att != NULL) {
         std::vector<float> scaleVector;
         scale2Att->getValue(scaleVector);
         if (scaleVector.empty() == false) {
            scale2 = scaleVector[0];
         }
      }
      if ((scale0Att != NULL) &&
          (scale1Att != NULL) &&
          (scale2Att != NULL)) {
         //
         // From Avi Snyder
         //
         mmppix[0] =  scale0;
         mmppix[1] = -scale1;
         mmppix[2] = -scale2;
      }
      else {
         errorMessage.append("Required attribute \"mmppix\" or \"scaling factor\" not found.\n");
      }
   }
   
   //
   // Get center
   //
   float center[3] = { 0.0, 0.0, 0.0 };
   WuNilAttribute* centerAttr = volumeRead.wunilHeader.getAttribute(WuNilAttribute::NAME_CENTER);
   if (centerAttr != NULL) {
      std::vector<float> centerVector;
      centerAttr->getValue(centerVector);
      if (centerVector.size() < 3) {
         errorMessage.append("Required attribute \"center\" does not have 3 elements\n");
      }
      else {
         center[0] = centerVector[0];
         center[1] = centerVector[1];
         center[2] = centerVector[2];
      }
   }
   else {
      //
      // From Avi Snyder
      //
		center[0] = mmppix[0] * static_cast<float>(dimensions[0] - dimensions[0]/2);
		center[1] = mmppix[1] * static_cast<float>(1 + dimensions[1]/2);
		center[2] = mmppix[2] * static_cast<float>(1 + dimensions[2]/2);
   
      //errorMessage.append("Required attribute \"center\" not found.\n");
   }
   
   const int firstVoxelIndex[3] = { 0, 0, 0 };
   
   //
   // Determine stereotaxic coordinates of first voxel
   //
   float org[3];
   WuNilHeader::voxelIndicesToStereotaxicCoordinates(volumeRead.dimensions,
                                                     center,
                                                     mmppix,
                                                     firstVoxelIndex,
                                                     org);
   
   //
   // default to transverse orientation
   //
   volumeRead.orientation[0] = ORIENTATION_RIGHT_TO_LEFT;
   volumeRead.orientation[1] = ORIENTATION_ANTERIOR_TO_POSTERIOR;
   volumeRead.orientation[2] = ORIENTATION_INFERIOR_TO_SUPERIOR;
   
   //
   // Set spacing based upon volume's orientation code
   //
   float space[3] = { 0.0, 0.0, 0.0 };
   switch (ifhOrientation) {
      case 2: // transverse
         space[0] = -mmppix[0];
         space[1] =  mmppix[1];
         space[2] = -mmppix[2];
         //volumeRead.setOriginAtCornerOfVoxel(org, space);
         volumeRead.setOrigin(org);//Tim C: corner of voxel convention discarded by 4dfp, due to interpolation method
         volumeRead.orientation[0] = ORIENTATION_RIGHT_TO_LEFT;
         volumeRead.orientation[1] = ORIENTATION_ANTERIOR_TO_POSTERIOR;
         volumeRead.orientation[2] = ORIENTATION_INFERIOR_TO_SUPERIOR;
         break;
      case 3: // coronal
         space[0] = -mmppix[0];
         space[1] =  mmppix[1];
         space[2] =  mmppix[2];
         volumeRead.orientation[0] = ORIENTATION_RIGHT_TO_LEFT;
         volumeRead.orientation[1] = ORIENTATION_SUPERIOR_TO_INFERIOR;
         volumeRead.orientation[2] = ORIENTATION_ANTERIOR_TO_POSTERIOR;
         org[0] = fabs(org[0]);
         org[1] = fabs(org[1]);
         org[2] = fabs(org[2]);
         volumeRead.setOriginAtCornerOfVoxel(org, space);
         break;
      case 4: // sagittal
         space[0] = -mmppix[0];
         space[1] =  mmppix[1];//Tim C: ground truth from S2T says x and y flip, that is, i and k voxel indices
         space[2] = -mmppix[2];
         volumeRead.orientation[0] = ORIENTATION_ANTERIOR_TO_POSTERIOR;
         volumeRead.orientation[1] = ORIENTATION_SUPERIOR_TO_INFERIOR;
         volumeRead.orientation[2] = ORIENTATION_LEFT_TO_RIGHT;
         org[0] = fabs(org[0]);
         org[1] = fabs(org[1]);
         org[2] = -fabs(org[2]);
         volumeRead.setOriginAtCornerOfVoxel(org, space);
         break;
      case 100:
         space[0] = fabs(mmppix[0]);
         space[1] = fabs(mmppix[1]);
         space[2] = fabs(mmppix[2]);
         volumeRead.orientation[0] = ORIENTATION_UNKNOWN;
         volumeRead.orientation[1] = ORIENTATION_UNKNOWN;
         volumeRead.orientation[2] = ORIENTATION_UNKNOWN;
         float org[3] = { 0, 0, 0 };
         volumeRead.setOriginAtCornerOfVoxel(org, space);
         break;
   }
   volumeRead.setSpacing(space);
     
   //
   // Get the study metadata link
   //
   WuNilAttribute* metaAttr = volumeRead.wunilHeader.getAttribute(WuNilAttribute::NAME_CARET_METADATA);
   if (metaAttr != NULL) {
      StudyMetaDataLinkSet smdls;
      smdls.setLinkSetFromCodedText(metaAttr->getValue().trimmed());
      volumeRead.setStudyMetaDataLinkSet(smdls);
   }
   
   //
   // get the region names
   //
   std::vector<QString> wuRegionNames;
   volumeRead.wunilHeader.getRegionNames(wuRegionNames);
   const int numWuRegionNames = static_cast<int>(wuRegionNames.size());
   for (int i = 0; i < numWuRegionNames; i++) {
      volumeRead.addRegionName(wuRegionNames[i]);
   }

   //
   // If error message is not isEmpty something required is missing from the volume
   //
   if (errorMessage.isEmpty() == false) {
      throw FileException(FileUtilities::basename(volumeRead.filename), errorMessage);
   }
   
   //
   // WU NIL files are big endian by default
   //
   bool bigEndianDataFlag = true;
   WuNilAttribute* endianAttr = volumeRead.wunilHeader.getAttribute(WuNilAttribute::NAME_IMAGEDATA_BYTE_ORDER);
   if (endianAttr != NULL) {
      if (endianAttr->getValue().trimmed() == "littleendian") {
         bigEndianDataFlag = false;
      }
   }
   
   //
   // WU NIL files normally created on sun workstations which are big endian
   //
   bool bigEndianComputerFlag = (QSysInfo::ByteOrder == QSysInfo::BigEndian);
   //const bool byteSwapFlag = (bigEndianComputerFlag == false);
   const bool byteSwapFlag = (bigEndianComputerFlag != bigEndianDataFlag);

   //
   // Create the volume data file name
   //
   volumeRead.dataFileName = FileUtilities::filenameWithoutExtension(volumeRead.filename);
   volumeRead.dataFileName.append(".img");
   
   //
   // Data file might be gzipped
   //
   if (QFile::exists(volumeRead.dataFileName) == false) {
      QString zipName(volumeRead.dataFileName);
      zipName.append(".gz");
      if (QFile::exists(zipName)) {
         volumeRead.dataFileName = zipName;
      }
   }
   
   //
   // If only reading header
   //
   if (readSelection == -2) {
      VolumeFile* vf = new VolumeFile;
      vf->copyVolumeData(volumeRead, false);
      vf->filename = volumeRead.filename;
      vf->dataFileName = volumeRead.dataFileName;
      volumesReadOut.push_back(vf);
      return;
   }
   
   //
   // Open the data file
   //
   gzFile dataFile = gzopen(volumeRead.dataFileName.toAscii().constData(), "rb");
   if (dataFile == NULL) {
      QString msg("Unable to open data file: ");
      msg.append(volumeRead.dataFileName);
      throw FileException(fileNameIn, msg);
   }

   try {         
      //
      // loop through and read the volume files
      //
      const int numSubs = volumeRead.subVolumeNames.size();
      for (int i = 0; i < numSubs; i++) {
         //
         // Determine if this sub volume should be read
         //
         bool readingSingleSubVolume = false;
         bool readIt = false;
         if (readSelection == VOLUME_READ_SELECTION_ALL) {
            readIt = true;
         }
         else if (readSelection == i) {
            readIt = true;
            readingSingleSubVolume = true;
         }
         
         if (readIt) {
            //
            // copy everything but voxel data from the first volume
            //
            VolumeFile* vf = NULL;
            vf = new VolumeFile;
            vf->copyVolumeData(volumeRead, false);
            vf->filename = volumeRead.filename;
            vf->dataFileName = volumeRead.dataFileName;
            
            //
            // Set the descriptive label
            //
            vf->descriptiveLabel = volumeRead.subVolumeNames[i];
            
            //
            // If only reading a single sub-volume
            //
            if (readingSingleSubVolume) {
               //
               // Read just the sub-volume
               //
               vf->readVolumeFileDataSubVolume(byteSwapFlag,
                                               vf->scaleSlope[i],
                                               vf->scaleOffset[i],
                                               0,
                                               i,
                                               dataFile);
            }
            else {
               //
               // Read just the sub-volume
               //
               vf->readVolumeFileData(byteSwapFlag,
                                      vf->scaleSlope[i],
                                      vf->scaleOffset[i],
                                      dataFile);
            }
            
            //
            // return the volume to the user
            //
            volumesReadOut.push_back(vf);
            
            //
            // If only reading a single sub-volume
            //
            if (readingSingleSubVolume) {
               break;
            }
         }
      }
   }
   catch (FileException& e) {
      gzclose(dataFile);
      throw e;
   }
   
   //
   // Close the file
   //
   gzclose(dataFile);
}                      

/**
 * write the specified AFNI sub-volumes.
 */
void 
VolumeFile::writeFileAfni(const QString& fileNameIn,
                          const VOXEL_DATA_TYPE writeVoxelDataTypeIn,
                          std::vector<VolumeFile*>& volumesToWrite,
                          const bool zipAfniBrikFile) throw (FileException)
{
   if (volumesToWrite.empty()) {
      throw FileException(fileNameIn, "No volume data to write.");
   }
   
   VolumeFile* firstVolume = volumesToWrite[0];
   firstVolume->filename = fileNameIn;
   firstVolume->voxelDataType = writeVoxelDataTypeIn;
   const int numSubVolumes = static_cast<int>(volumesToWrite.size());

   switch(firstVolume->volumeType) {
      case VOLUME_TYPE_ANATOMY:
         break;
      case VOLUME_TYPE_FUNCTIONAL:
         break;
      case VOLUME_TYPE_PAINT:
         break; 
      case VOLUME_TYPE_PROB_ATLAS:
         break; 
      case VOLUME_TYPE_RGB:
         firstVolume->voxelDataType = VOXEL_DATA_TYPE_RGB_VOXEL_INTERLEAVED;
         break;     
      case VOLUME_TYPE_ROI:
         break;
      case VOLUME_TYPE_SEGMENTATION:
         break;
      case VOLUME_TYPE_VECTOR:
         break;
      case VOLUME_TYPE_UNKNOWN:
         //throw FileException(fileNameIn, "Unknown type of volume.");
         break;
   }
   
   //
   // Check for invalid voxel data types in AFNI files
   //
   switch(firstVolume->voxelDataType) {
      case VOXEL_DATA_TYPE_UNKNOWN:
         throw FileException(firstVolume->filename, "Unknown data type");
         break;
      case VOXEL_DATA_TYPE_CHAR:
         throw FileException(firstVolume->filename, "AFNI does not support byte-signed data type.");
         break;
      case VOXEL_DATA_TYPE_CHAR_UNSIGNED:
         break;
      case VOXEL_DATA_TYPE_SHORT:
         break;
      case VOXEL_DATA_TYPE_SHORT_UNSIGNED:
         throw FileException(firstVolume->filename, "AFNI does not support short-unsigned data type.");
         break;
      case VOXEL_DATA_TYPE_INT:
         break;
      case VOXEL_DATA_TYPE_INT_UNSIGNED:
         throw FileException(firstVolume->filename, "AFNI does not support int-unsigned data type.");
         break;
      case VOXEL_DATA_TYPE_LONG:
         throw FileException(firstVolume->filename, "AFNI does not support long-signed data type.");
         break;
      case VOXEL_DATA_TYPE_LONG_UNSIGNED:
         throw FileException(firstVolume->filename, "AFNI does not support long-unsigned data type.");
         break;
      case VOXEL_DATA_TYPE_FLOAT:
         break;
      case VOXEL_DATA_TYPE_DOUBLE:
         break;
      case VOXEL_DATA_TYPE_RGB_VOXEL_INTERLEAVED:
         break;
      case VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED:
         break;
      case VOXEL_DATA_TYPE_VECTOR:
         break;
   }
   
   //
   // Setup the AFNI header
   //   
   firstVolume->afniHeader.setupFromVolumeFiles(volumesToWrite, NULL);
   
   //
   // Open file and create a text stream
   //
   QFile file(firstVolume->filename);
   if (file.open(QIODevice::WriteOnly) == false) {
      throw FileException(firstVolume->filename, file.errorString());
   }
   QTextStream stream(&file);
   
   //
   // Write the header file
   //
   firstVolume->afniHeader.writeHeader(stream);
   
   //
   // Close the header file
   //
   file.close();
   
   //
   // Update file permissions ?
   //
   if (getFileWritePermissions() != 0) {
      QFile::setPermissions(firstVolume->filename, getFileWritePermissions());
   }

   //
   // Create the name of the data file
   //
   firstVolume->dataFileName = FileUtilities::filenameWithoutExtension(firstVolume->filename);
   firstVolume->dataFileName.append(".BRIK");
   if (zipAfniBrikFile) {
      firstVolume->dataFileName.append(".gz");
   }
   firstVolume->dataFileWasZippedFlag = zipAfniBrikFile;
   
   //
   // Open the data file
   //
   gzFile zipFile = NULL;
   std::ofstream* cppFile = NULL;
   if (zipAfniBrikFile) {
      zipFile = gzopen(firstVolume->dataFileName.toAscii().constData(), "wb");
      if (zipFile == NULL) {
         throw FileException(firstVolume->dataFileName, "Unable to open for writing");
      }
   }
   else {
      cppFile = new std::ofstream(firstVolume->dataFileName.toAscii().constData(),
                                  std::ios::out | std::ios::binary);
      if (cppFile == NULL) {
         throw FileException(firstVolume->dataFileName, "Unable to open for writing");
      }
   }
   
   //
   // Write the volume data
   //
   QString writeErrorMessage;
   for (int i = 0; i < numSubVolumes; i++) {
      try {
         volumesToWrite[i]->writeVolumeFileData(firstVolume->voxelDataType,
                                                false,
                                                zipAfniBrikFile,
                                                zipFile,
                                                cppFile,
                                                1.0);
      }
      catch (FileException& e) {
         writeErrorMessage = e.whatQString();
      }
   }
   
   //
   // Close the data file
   //
   if (zipAfniBrikFile) {
      gzclose(zipFile);
   }
   else {
      cppFile->close();
      delete cppFile;
   }

   if (writeErrorMessage.isEmpty() == false) {
      throw FileException(firstVolume->dataFileName, writeErrorMessage);
   }
   
   //
   // Update file permissions ?
   //
   if (getFileWritePermissions() != 0) {
      QFile::setPermissions(firstVolume->dataFileName, getFileWritePermissions());
   }
}                          
                      
/**
 * write the specified Analyze sub-volumes.
 */
void 
VolumeFile::writeFileAnalyze(const QString& fileNameIn,
                             const VOXEL_DATA_TYPE writeVoxelDataType,
                             std::vector<VolumeFile*>& volumesToWrite) throw (FileException)
{
   writeFileSPM(fileNameIn,
                writeVoxelDataType,
                volumesToWrite,
                true);
}                          
                      
/**
 * write the specified NIFTI sub-volumes.
 */
void 
VolumeFile::writeFileNifti(const QString& fileNameIn,
                           const VOXEL_DATA_TYPE writeVoxelDataTypeIn,
                           std::vector<VolumeFile*>& volumesToWrite,
                           const ColorFile* labelColorsForCaret6or7,
                           const int writeForCaretVersion) throw (FileException)
{
   const int numSubVolumes = static_cast<int>(volumesToWrite.size());
   if (numSubVolumes <= 0) {
      throw FileException(fileNameIn, "No volume data to write.");
   }

   //
   // Get the maximum absolute value of all voxels
   //
   float maxAbsVoxelValue = 0.0;
   for (int i = 0; i < numSubVolumes; i++) {
      float minValue, maxValue;
      volumesToWrite[i]->getMinMaxVoxelValues(minValue, maxValue);
      maxAbsVoxelValue = std::max(std::fabs(minValue), maxAbsVoxelValue);
      maxAbsVoxelValue = std::max(std::fabs(maxValue), maxAbsVoxelValue);
   }

   //
   // Get the first volume
   //
   VolumeFile* firstVolume = volumesToWrite[0];
   firstVolume->filename = fileNameIn;
   firstVolume->voxelDataType = writeVoxelDataTypeIn;

   switch(firstVolume->volumeType) {
      case VOLUME_TYPE_ANATOMY:
         break;
      case VOLUME_TYPE_FUNCTIONAL:
         break;
      case VOLUME_TYPE_PAINT:
         break; 
      case VOLUME_TYPE_PROB_ATLAS:
         break; 
      case VOLUME_TYPE_RGB:
         firstVolume->voxelDataType = VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED;
         break;     
      case VOLUME_TYPE_ROI:
         break;
      case VOLUME_TYPE_SEGMENTATION:
         break;
      case VOLUME_TYPE_VECTOR:
         break;
      case VOLUME_TYPE_UNKNOWN:
         //throw FileException(firstVolume->filename, "Unknown type of volume.");
         break;
   }

   //
   // Create a NIFTI header and zero it out
   //
   nifti_1_header hdr;
   memset(&hdr, 0, sizeof(hdr));
   
   //
   // Set the size of the header
   //
   hdr.sizeof_hdr = sizeof(struct dsr);
   hdr.regular = 'r';
   
   //
   // Set the dimensions
   //
   hdr.dim[0] = 3; //set at 3 to start
   hdr.dim[1] = firstVolume->dimensions[0];
   hdr.dim[2] = firstVolume->dimensions[1];
   hdr.dim[3] = firstVolume->dimensions[2];
   hdr.dim[4] = 1;
   hdr.dim[5] = 1;
   if (numSubVolumes > 1) {
      hdr.dim[0]++;
      hdr.dim[hdr.dim[0]] = numSubVolumes;
   }
   //
   // Set the datatype
   //
   float maxDataTypeValue = std::numeric_limits<float>::max();
   switch (firstVolume->voxelDataType) {
      case VOXEL_DATA_TYPE_UNKNOWN:
         throw FileException(firstVolume->filename, "Unknown data type.");;
         break;
      case VOXEL_DATA_TYPE_CHAR:
         hdr.datatype = NIFTI_TYPE_INT8;
         maxDataTypeValue = std::numeric_limits<char>::max();
         break;
      case VOXEL_DATA_TYPE_CHAR_UNSIGNED:
         hdr.datatype = NIFTI_TYPE_UINT8;
         maxDataTypeValue = std::numeric_limits<unsigned char>::max();
         break;
      case VOXEL_DATA_TYPE_SHORT:
         hdr.datatype = NIFTI_TYPE_INT16;
         maxDataTypeValue = std::numeric_limits<short>::max();
         break;
      case VOXEL_DATA_TYPE_SHORT_UNSIGNED:
         hdr.datatype = NIFTI_TYPE_UINT16;
         maxDataTypeValue = std::numeric_limits<unsigned short>::max();
         break;
      case VOXEL_DATA_TYPE_INT:
         hdr.datatype = NIFTI_TYPE_INT32;
         maxDataTypeValue = std::numeric_limits<int>::max();
         break;
      case VOXEL_DATA_TYPE_INT_UNSIGNED:
         hdr.datatype = NIFTI_TYPE_UINT32;
         maxDataTypeValue = std::numeric_limits<unsigned int>::max();
         break;
      case VOXEL_DATA_TYPE_LONG:
         hdr.datatype = NIFTI_TYPE_INT64;
         maxDataTypeValue = std::numeric_limits<long>::max();
         break;
      case VOXEL_DATA_TYPE_LONG_UNSIGNED:
         hdr.datatype = NIFTI_TYPE_UINT64;
         maxDataTypeValue = std::numeric_limits<unsigned long>::max();
         break;
      case VOXEL_DATA_TYPE_FLOAT:
         hdr.datatype = NIFTI_TYPE_FLOAT32;
         maxDataTypeValue = std::numeric_limits<float>::max();
         break;
      case VOXEL_DATA_TYPE_DOUBLE:
         hdr.datatype = NIFTI_TYPE_FLOAT64;
         maxDataTypeValue = std::numeric_limits<float>::max();// FLOAT since date stored in FLOAT which is smaller than double
         break;
      case VOXEL_DATA_TYPE_RGB_VOXEL_INTERLEAVED:
         hdr.datatype = NIFTI_TYPE_RGB24;
         hdr.dim[0]++;  
         hdr.dim[hdr.dim[0]] = 3;  // 3 values per voxel
         break;
      case VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED:
         hdr.datatype = NIFTI_TYPE_RGB24;
         hdr.dim[0]++;  
         hdr.dim[hdr.dim[0]] = 3;  // 3 values per voxel
         break;
      case VOXEL_DATA_TYPE_VECTOR:
         hdr.datatype = NIFTI_TYPE_FLOAT32;
         hdr.dim[0]++;  
         hdr.dim[hdr.dim[0]] = 3;  // 3 values per voxel
         break;
   }
   
   hdr.intent_code = NIFTI_INTENT_NONE;
   switch(firstVolume->volumeType) {
      case VOLUME_TYPE_ANATOMY:
         break;
      case VOLUME_TYPE_FUNCTIONAL:
         break;
      case VOLUME_TYPE_PAINT:
         hdr.intent_code = NIFTI_INTENT_LABEL;
         break; 
      case VOLUME_TYPE_PROB_ATLAS:
         hdr.intent_code = NIFTI_INTENT_LABEL;
         break; 
      case VOLUME_TYPE_RGB:
         break;     
      case VOLUME_TYPE_ROI:
         break;
      case VOLUME_TYPE_SEGMENTATION:
         break;
      case VOLUME_TYPE_VECTOR:
         hdr.intent_code = NIFTI_INTENT_VECTOR;
         break;
      case VOLUME_TYPE_UNKNOWN:
         break;
   }

   //
   //  Data offset and slope
   //
   hdr.scl_slope = 1.0;
   hdr.scl_inter = 0.0;

   //
   // Set scaling only if needed.  Data will be divided by this value
   // as it is written.
   // Scaling is applied after data is read.
   // When data is written, it will be multiplied by this value to restore
   // the original values.
   //
   if (maxAbsVoxelValue > maxDataTypeValue) {
      if (maxDataTypeValue > 0.0) {
         hdr.scl_slope = maxAbsVoxelValue / maxDataTypeValue;
         if (hdr.scl_slope <= 0.0) {
            hdr.scl_slope = 1.0;
         }
      }
   }

   //
   // voxel sizes
   //
   hdr.pixdim[0] = 1; // is actually "qfac" in NIFTI not 3 as in analyze;
   hdr.pixdim[1] = firstVolume->spacing[0];
   hdr.pixdim[2] = firstVolume->spacing[1];
   hdr.pixdim[3] = firstVolume->spacing[2];
      
   //
   // Comment
   //
   const QString comm(firstVolume->getHeaderTag(headerTagComment));
   if (comm.isEmpty() == false) {
      int len = comm.length();
      if (len > 79) {
         len = 79;
      }
      for (int i = 0; i < len; i++) {
         hdr.descrip[i] = comm[i].toAscii();
      }
      hdr.descrip[len] = '\0';
   }
  
/*
   //
   // origin in NIFTI file is in center of voxel, caret origin is at corner of voxel
   //
   const float halfVoxelOffset[3] = {
      firstVolume->spacing[0] * 0.5,
      firstVolume->spacing[1] * 0.5,
      firstVolume->spacing[2] * 0.5
   };
*/   
   //
   // Set origin info
   //
   // QFORM is always UNKNOWN
   // This should be fixed so that QFORM produces the same coordinates as SFORM
   //
   hdr.qform_code = NIFTI_XFORM_UNKNOWN;
   if ((firstVolume->orientation[0] == ORIENTATION_LEFT_TO_RIGHT) && 
       (firstVolume->orientation[1] == ORIENTATION_POSTERIOR_TO_ANTERIOR) &&
       (firstVolume->orientation[2] == ORIENTATION_INFERIOR_TO_SUPERIOR)) {
      hdr.qform_code = NIFTI_XFORM_TALAIRACH;
      hdr.sform_code = NIFTI_XFORM_TALAIRACH;
   }
   else {
      hdr.qform_code = NIFTI_XFORM_SCANNER_ANAT;
      hdr.sform_code = NIFTI_XFORM_SCANNER_ANAT;
   }
   hdr.quatern_b = 0.0;
   hdr.quatern_c = 0.0;
   hdr.quatern_d = 0.0;
   hdr.qoffset_x = firstVolume->origin[0]; // + halfVoxelOffset[0];
   hdr.qoffset_y = firstVolume->origin[1]; // + halfVoxelOffset[1];
   hdr.qoffset_z = firstVolume->origin[2]; // + halfVoxelOffset[2];
   hdr.srow_x[0] = firstVolume->spacing[0];
   hdr.srow_x[1] = 0.0;
   hdr.srow_x[2] = 0.0;
   hdr.srow_x[3] = firstVolume->origin[0]; // + halfVoxelOffset[0];
   hdr.srow_y[0] = 0.0;
   hdr.srow_y[1] = firstVolume->spacing[1];
   hdr.srow_y[2] = 0.0;
   hdr.srow_y[3] = firstVolume->origin[1]; // + halfVoxelOffset[1];
   hdr.srow_z[0] = 0.0;
   hdr.srow_z[1] = 0.0;
   hdr.srow_z[2] = firstVolume->spacing[2];
   hdr.srow_z[3] = firstVolume->origin[2]; // + halfVoxelOffset[2];
   
   //
   // set the magic number info
   //
   hdr.magic[0] = 'n';
   hdr.magic[1] = '+';
   hdr.magic[2] = '1';
   hdr.magic[3] = '\0';
   
   //
   // Setup the AFNI header placed into a NIFTI extension and then
   // create the extension
   //   
   firstVolume->afniHeader.setupFromVolumeFiles(volumesToWrite,
                                                labelColorsForCaret6or7);
   QString afniExtensionString;
   firstVolume->afniHeader.writeToNiftiExtension(afniExtensionString,
                                                 &hdr);
   
      
   // Make sure niftiExtension length is a multiple of 16 bytes !!!!!
   // and that includes the "esize" and "ecode".
   // fill with blanks
   const int len = afniExtensionString.length() + 8;
   const int fillNeeded = 16 - (len % 16);
   if (fillNeeded > 0) {
      afniExtensionString += QString(fillNeeded, QChar(' '));
   }
   const int afniExtensionSize = afniExtensionString.length();
      
   //
   // Create the NIFTI extender and immediately follows the header
   //
   nifti1_extender extender;
   nifti1_extension afniExtension;
   afniExtension.esize = 8 + afniExtensionSize;  // include size of esize/ecode
   afniExtension.ecode = 4; //NIFTI_ECODE_AFNI;
   afniExtension.edata = NULL;
   extender.extension[0] = 0;
   extender.extension[1] = 0;
   extender.extension[2] = 0;
   extender.extension[3] = 0;
   if (afniExtension.esize > 0) {
      extender.extension[0] = 1;
   }
   
   if (DebugControl::getDebugOn()) {
      std::cout << "AFNI extension len/added/after: " 
                << len << ", " << fillNeeded << ", " << afniExtensionSize << std::endl;
   }

   //
   // Create the Caret extension
   //
   NiftiCaretExtension caretExtensionCreator(volumesToWrite, 
                                             labelColorsForCaret6or7,
                                             writeForCaretVersion);
   QString caretExtensionString = caretExtensionCreator.getExtensionAsString();
   // Make sure niftiExtension length is a multiple of 16 bytes !!!!!
   // and that includes the "esize" and "ecode".
   // fill with blanks
   const int caretExtlen = caretExtensionString.length() + 8;
   const int caretExtFillNeeded = 16 - (caretExtlen % 16);
   if (caretExtFillNeeded > 0) {
      caretExtensionString += QString(caretExtFillNeeded, QChar(' '));
   }
   const int caretExtensionSize = caretExtensionString.length();

   nifti1_extension caretExtension;
   caretExtension.esize = 8 + caretExtensionSize;
   caretExtension.ecode = 30;
   caretExtension.edata = NULL;

   //
   // Data offset is size of header
   //
   hdr.vox_offset = hdr.sizeof_hdr + sizeof(nifti1_extender);
   if (afniExtension.esize > 0) {
      hdr.vox_offset += sizeof(afniExtension.esize)
                        + sizeof(afniExtension.ecode)
                        + afniExtensionSize;
   }
   if (caretExtension.esize > 0) {
      hdr.vox_offset += sizeof(caretExtension.esize)
                        + sizeof(caretExtension.ecode)
                        + caretExtensionSize;
   }
   
   //
   // Is file being written compressed
   //
   const bool zipDataFileFlag = (firstVolume->filename.right(3) == ".gz");
   
   //
   // NIFTI stored in one file
   //
   firstVolume->dataFileName = firstVolume->filename;
   
   //
   // Open the data file
   //
   gzFile zipFile = NULL;
   std::ofstream* cppFile = NULL;
   if (zipDataFileFlag) {
      zipFile = gzopen(firstVolume->filename.toAscii().constData(), "wb");
      if (zipFile == NULL) {
         throw FileException(firstVolume->filename, "Unable to open for writing");
      }
   }
   else {
      cppFile = new std::ofstream(firstVolume->filename.toAscii().constData(),
                                  std::ios::out | std::ios::binary);
      if (cppFile == NULL) {
         throw FileException(firstVolume->filename, "Unable to open for writing");
      }
   }
   firstVolume->dataFileWasZippedFlag = zipDataFileFlag;

   if (DebugControl::getDebugOn()) {
      std::cout << "AFNI Extension Length: "
                << afniExtensionString.length()
                << ", Size: "
                << afniExtensionSize;
      std::cout << "Caret Extension Length: "
                << caretExtensionString.length()
                << ", Size: "
                << caretExtensionSize;
   }
   
   //
   // Write the header
   //
   const unsigned long headerSize = sizeof(hdr);
   if (zipDataFileFlag) {
      //
      // Write the header
      //
      gzwrite(zipFile, (voidp)&hdr, headerSize);
      
      //
      // Write the extender that tells whether or not there are extensions
      //
      gzwrite(zipFile, (voidp)&extender, sizeof(extender));

      //
      // write the afni extension
      //
      if (afniExtension.esize > 0) {
         gzwrite(zipFile, (voidp)&afniExtension.esize, sizeof(afniExtension.esize));
         gzwrite(zipFile, (voidp)&afniExtension.ecode, sizeof(afniExtension.ecode));
         gzwrite(zipFile, (voidp)afniExtensionString.toAscii().constData(),
                          afniExtensionSize);
      }
      if (caretExtension.esize > 0) {
         gzwrite(zipFile, (voidp)&caretExtension.esize, sizeof(caretExtension.esize));
         gzwrite(zipFile, (voidp)&caretExtension.ecode, sizeof(caretExtension.ecode));
         gzwrite(zipFile, (voidp)caretExtensionString.toAscii().constData(),
                          caretExtensionSize);
      }
   }
   else {
      //
      // Write the header
      //
      cppFile->write((const char*)&hdr, headerSize);
      
      //
      // Write the extender that tells whether or not there are extensions
      //
      cppFile->write((const char*)&extender, sizeof(extender));     
      
      //
      // write the afni extension
      //
      if (afniExtension.esize > 0) {
         cppFile->write((const char*)&afniExtension.esize, sizeof(afniExtension.esize));
         cppFile->write((const char*)&afniExtension.ecode, sizeof(afniExtension.ecode));
         cppFile->write((const char*)afniExtensionString.toAscii().constData(),
                        afniExtensionSize);
      }
      if (caretExtension.esize > 0) {
         cppFile->write((const char*)&caretExtension.esize, sizeof(caretExtension.esize));
         cppFile->write((const char*)&caretExtension.ecode, sizeof(caretExtension.ecode));
         cppFile->write((const char*)caretExtensionString.toAscii().constData(),
                          caretExtensionSize);
      }
   }
   

   //
   // Write the volume data
   //
   QString writeErrorMessage;
   for (int i = 0; i < numSubVolumes; i++) {
      try {
         volumesToWrite[i]->writeVolumeFileData(firstVolume->voxelDataType,
                                                false,
                                                zipDataFileFlag,
                                                zipFile,
                                                cppFile,
                                                hdr.scl_slope);
      }
      catch (FileException& e) {
         writeErrorMessage = e.whatQString();
         break;
      }
   }
   
   //
   // Close the data file
   //
   if (zipDataFileFlag) {
      gzclose(zipFile);
   }
   else {
      cppFile->close();
      delete cppFile;
   }

   if (writeErrorMessage.isEmpty() == false) {
      throw FileException(firstVolume->filename, writeErrorMessage);
   }
}                          
                      
/**
 * write the specified SPM sub-volumes.
 */
void 
VolumeFile::writeFileSPM(const QString& fileNameIn,
                         const VOXEL_DATA_TYPE writeVoxelDataTypeIn,
                         std::vector<VolumeFile*>& volumesToWrite,
                         const bool analyzeFlag) throw (FileException)
{
   const int numSubVolumes = static_cast<int>(volumesToWrite.size());
   if (numSubVolumes <= 0) {
      throw FileException(fileNameIn, "No volume data to write.");
   }
   
   //
   // Get the first volume
   //
   VolumeFile* firstVolume = volumesToWrite[0];
   firstVolume->filename = fileNameIn;
   firstVolume->voxelDataType = writeVoxelDataTypeIn;

   //
   // check volume type for compaibility
   //
   switch(firstVolume->volumeType) {
      case VOLUME_TYPE_ANATOMY:
         break;
      case VOLUME_TYPE_FUNCTIONAL:
         break;
      case VOLUME_TYPE_PAINT:
         throw FileException(firstVolume->filename, 
                               "Paint Volume cannot be written to an SPM/MEDx file.");
         break; 
      case VOLUME_TYPE_PROB_ATLAS:
         throw FileException(firstVolume->filename, 
                               "Prob Atlas Volume cannot be written to an Analyze file.");
         break; 
      case VOLUME_TYPE_RGB:
         firstVolume->voxelDataType = VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED;
         break;     
      case VOLUME_TYPE_ROI:
         break;
      case VOLUME_TYPE_SEGMENTATION:
         break;
      case VOLUME_TYPE_VECTOR:
         break;
      case VOLUME_TYPE_UNKNOWN:
         //throw FileException(firstVolume->filename, "Unknown type of volume.");
         break;
   }
   
   //
   // Analyze header structure
   //
   struct dsr hdr;
   memset((void*)&hdr, 0, sizeof(struct dsr));
   
   //
   // Set the size of the header
   //
   hdr.hk.sizeof_hdr = sizeof(struct dsr);
   hdr.hk.regular = 'r';
   
   //
   // dimensions
   //
   hdr.dime.dim[0] = 3;
   hdr.dime.dim[1] = firstVolume->dimensions[0];
   hdr.dime.dim[2] = firstVolume->dimensions[1];
   hdr.dime.dim[3] = firstVolume->dimensions[2];
   hdr.dime.dim[4] = 1;
   if (numSubVolumes > 1) {
      hdr.dime.dim[0]++;
      hdr.dime.dim[hdr.dime.dim[0]] = numSubVolumes;
   }
   
   //
   // Minimum and Maximum voxel
   //
   float mins, maxs;
   firstVolume->getMinMaxVoxelValues(mins, maxs);
   hdr.dime.glmax = static_cast<int>(mins);
   hdr.dime.glmin = static_cast<int>(maxs);
   
   //
   // Verify data type is supported
   //
   switch (firstVolume->voxelDataType) {
      case VOXEL_DATA_TYPE_CHAR_UNSIGNED:
         hdr.dime.datatype = 2;
         hdr.dime.bitpix   = 8;
         break;
      case VOXEL_DATA_TYPE_SHORT:
         hdr.dime.datatype = 4;
         hdr.dime.bitpix   = 16;
         break;
      case VOXEL_DATA_TYPE_INT:
         hdr.dime.datatype = 8;
         hdr.dime.bitpix   = 32;
         break;
      case VOXEL_DATA_TYPE_FLOAT:
         hdr.dime.datatype = 16;
         hdr.dime.bitpix   = 32;
         break;
      case VOXEL_DATA_TYPE_DOUBLE:
         hdr.dime.datatype = 64;
         hdr.dime.bitpix   = 64;
         break;
      case VOXEL_DATA_TYPE_RGB_VOXEL_INTERLEAVED:
         hdr.dime.datatype = 128;
         hdr.dime.bitpix   = 24;
         hdr.dime.glmax = 255;
         hdr.dime.glmin = 0;
         break;
      case VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED:
         hdr.dime.datatype = 128;
         hdr.dime.bitpix   = 24;
         hdr.dime.glmax = 255;
         hdr.dime.glmin = 0;
         break;
      case VOXEL_DATA_TYPE_UNKNOWN:
         throw FileException(firstVolume->filename, "Data Type is Unknown");
         break;
      case VOXEL_DATA_TYPE_CHAR:
         throw FileException(firstVolume->filename, "Data Type CHAR is not supported by Analyze");
         break;
      case VOXEL_DATA_TYPE_SHORT_UNSIGNED:
         throw FileException(firstVolume->filename, "Data Type SHORT UNSIGNED is not supported by Analyze");
         break;
      case VOXEL_DATA_TYPE_INT_UNSIGNED:
         throw FileException(firstVolume->filename, "Data Type INT UNSIGNED is not supported by Analyze");
         break;
      case VOXEL_DATA_TYPE_LONG:
         throw FileException(firstVolume->filename, "Data Type LONG is not supported by Analyze");
         break;
      case VOXEL_DATA_TYPE_LONG_UNSIGNED:
         throw FileException(firstVolume->filename, "Data Type LONG UNSIGNED is not supported by Analyze");
         break;
      case VOXEL_DATA_TYPE_VECTOR:
         hdr.dime.datatype = 16;
         hdr.dime.bitpix   = 32;
         break;
   }
   
   //
   // voxel sizes
   //
   hdr.dime.pixdim[0] = 3;
   hdr.dime.pixdim[1] = firstVolume->spacing[0];
   hdr.dime.pixdim[2] = firstVolume->spacing[1];
   hdr.dime.pixdim[3] = firstVolume->spacing[2];
      
   if (analyzeFlag == false) {
      //
      // Is the volume being made LPI with true coordinates
      //
      if ((firstVolume->spacing[0] != 0.0) && 
          (firstVolume->spacing[1] != 0.0) && 
          (firstVolume->spacing[2] != 0.0)) {
         short spmAcShort[5] = { 0, 0, 0, 0, 0 };
         spmAcShort[0] = static_cast<short>(-(firstVolume->origin[0] / firstVolume->spacing[0])) + 1;
         spmAcShort[1] = static_cast<short>(-(firstVolume->origin[1] / firstVolume->spacing[1])) + 1;
         spmAcShort[2] = static_cast<short>(-(firstVolume->origin[2] / firstVolume->spacing[2])) + 1;

         //
         // Need to use memcpy since "hdr.hist.originator" is on odd byte boundary
         //
         memcpy(hdr.hist.originator, spmAcShort, 10);
      }
      //}
      
      //
      // voxel scaling
      //
      hdr.dime.funused1 = 1.0;  //scaleSlope[0];
   }
   
   //
   // Offset of image data
   //
   hdr.dime.vox_offset = 0.0;
   
   //
   // Planar orientation
   //
   hdr.hist.orient = 0;
   
   //
   // Voxel units
   //
   //strcpy(hdr.dime.vox_units, "mm");
   
   //
   // Comment
   //
   const QString comm(firstVolume->getHeaderTag(headerTagComment));
   if (comm.isEmpty() == false) {
      int len = comm.length();
      if (len > 79) {
         len = 79;
      }
      for (int i = 0; i < len; i++) {
         hdr.hist.descrip[i] = comm[i].toAscii();
      }
      hdr.hist.descrip[len] = '\0';
   }
 
   //
   // Write the header
   //
   std::ofstream headerFile(firstVolume->filename.toAscii().constData(),
                            std::ios::out | std::ios::binary);
   if (!headerFile) {
      throw FileException(firstVolume->filename, "Unable to open for writing.");
   }
   const unsigned long headerSize = sizeof(hdr);
   headerFile.write((const char*)&hdr, headerSize);
   headerFile.close();
   
   //
   // Update file permissions ?
   //
   if (getFileWritePermissions() != 0) {
      QFile::setPermissions(firstVolume->filename, getFileWritePermissions());
   }

   //
   // Create the name of the data file
   //
   firstVolume->dataFileName = FileUtilities::filenameWithoutExtension(firstVolume->filename);
   firstVolume->dataFileName.append(".img");
   
   //
   // Open the data file
   //
   std::ofstream* cppFile = new std::ofstream(firstVolume->dataFileName.toAscii().constData(),
                                              std::ios::out | std::ios::binary);
   if (cppFile == NULL) {
      throw FileException(firstVolume->dataFileName, "Unable to open for writing");
   }
   
   //
   // Write the volume data
   //
   QString writeErrorMessage;
   for (int i = 0; i < numSubVolumes; i++) {
      try {
         volumesToWrite[i]->writeVolumeFileData(firstVolume->voxelDataType,
                                                false,
                                                false,
                                                NULL,
                                                cppFile,
                                                1.0);
      }
      catch (FileException& e) {
         writeErrorMessage = e.whatQString();
         break;
      }
   }
   
   //
   // Close the data file
   //
   cppFile->close();
   delete cppFile;

   if (writeErrorMessage.isEmpty() == false) {
      throw FileException(firstVolume->dataFileName, writeErrorMessage);
   }
   
   //
   // Update file permissions ?
   //
   if (getFileWritePermissions() != 0) {
      QFile::setPermissions(firstVolume->dataFileName, getFileWritePermissions());
   }
}                          
                      
/**
 * write the specified WU NIL sub-volumes.
 */
void 
VolumeFile::writeFileWuNil(const QString& fileNameIn,
                           const VOXEL_DATA_TYPE writeVoxelDataTypeIn,
                           std::vector<VolumeFile*>& volumesToWrite) throw (FileException)
{
   const int numSubVolumes = static_cast<int>(volumesToWrite.size());
   if (numSubVolumes <= 0) {
      throw FileException(fileNameIn, "No volume data to write.");
   }
   if (numSubVolumes > 1) {
      throw FileException(fileNameIn, "Multiple subvolumes not supported for WU NIL volume files.");
   }
   
   //
   // Get the first volume
   //
   VolumeFile* firstVolume = volumesToWrite[0];
   firstVolume->filename = fileNameIn;
   firstVolume->voxelDataType = writeVoxelDataTypeIn;

   switch(firstVolume->volumeType) {
      case VOLUME_TYPE_ANATOMY:
         break;
      case VOLUME_TYPE_FUNCTIONAL:
         break;
      case VOLUME_TYPE_PAINT:
         break; 
      case VOLUME_TYPE_PROB_ATLAS:
         break; 
      case VOLUME_TYPE_RGB:
         throw FileException(firstVolume->filename, 
                               "RGB Volume cannot be written to a WU-NIL file.");
         break;     
      case VOLUME_TYPE_ROI:
         break;
      case VOLUME_TYPE_SEGMENTATION:
         break;
      case VOLUME_TYPE_VECTOR:
         break;
      case VOLUME_TYPE_UNKNOWN:
         //throw FileException(firstVolume->filename, "Unknown type of volume.");
         break;
   }
   
   //
   // WU-NIL files are always float
   //
   WuNilHeader wunilHeader;
   firstVolume->voxelDataType = VOXEL_DATA_TYPE_FLOAT;
   WuNilAttribute format(WuNilAttribute::NAME_NUMBER_FORMAT, "float");
   wunilHeader.addAttribute(format);
   
   WuNilAttribute bpp(WuNilAttribute::NAME_NUMBER_OF_BYTES_PER_PIXEL, 4);
   wunilHeader.addAttribute(bpp);
   
   WuNilAttribute orient(WuNilAttribute::NAME_ORIENTATION, 2);
   wunilHeader.addAttribute(orient);

   WuNilAttribute numdim(WuNilAttribute::NAME_NUMBER_OF_DIMENSIONS, 4);
   wunilHeader.addAttribute(numdim);

   WuNilAttribute s1(WuNilAttribute::NAME_SCALING_FACTOR_1, fabs(firstVolume->spacing[0]));
   wunilHeader.addAttribute(s1);

   WuNilAttribute s2(WuNilAttribute::NAME_SCALING_FACTOR_2, fabs(firstVolume->spacing[1]));
   wunilHeader.addAttribute(s2);

   WuNilAttribute s3(WuNilAttribute::NAME_SCALING_FACTOR_3, fabs(firstVolume->spacing[2]));
   wunilHeader.addAttribute(s3);

   WuNilAttribute m1(WuNilAttribute::NAME_MATRIX_SIZE_1, firstVolume->dimensions[0]);
   wunilHeader.addAttribute(m1);

   WuNilAttribute m2(WuNilAttribute::NAME_MATRIX_SIZE_2, firstVolume->dimensions[1]);
   wunilHeader.addAttribute(m2);   

   WuNilAttribute m3(WuNilAttribute::NAME_MATRIX_SIZE_3, firstVolume->dimensions[2]);
   wunilHeader.addAttribute(m3);   

   WuNilAttribute m4(WuNilAttribute::NAME_MATRIX_SIZE_4, 1);
   wunilHeader.addAttribute(m4);
   
   const bool bigEndianFlag = (QSysInfo::ByteOrder == QSysInfo::BigEndian);
   if (bigEndianFlag) {
      WuNilAttribute byteOrder(WuNilAttribute::NAME_IMAGEDATA_BYTE_ORDER, "bigendian");
      wunilHeader.addAttribute(byteOrder);
   }
   else {
      WuNilAttribute byteOrder(WuNilAttribute::NAME_IMAGEDATA_BYTE_ORDER, "littleendian");
      wunilHeader.addAttribute(byteOrder);
   }
   
   float lpiorigin[3];
   //firstVolume->getOriginAtCornerOfVoxel(originCorner);
   firstVolume->getOrigin(lpiorigin);//Tim C: corner of voxel convention discarded due to interpolation method, see read wunil
   float org[3], rai[3];
   //find rai origin
   rai[0] = firstVolume->spacing[0] * (firstVolume->dimensions[0] - 1) + lpiorigin[0];
   rai[1] = firstVolume->spacing[1] * (firstVolume->dimensions[1] - 1) + lpiorigin[1];
   rai[2] = lpiorigin[2];
   org[0] = -rai[0] + firstVolume->spacing[0] * firstVolume->dimensions[0];//this spacing gets flipped due to RAI origin
   org[1] = -firstVolume->spacing[1] - rai[1];//ditto
   org[2] = -rai[2] - firstVolume->spacing[2] * firstVolume->dimensions[2];
   WuNilAttribute nc(WuNilAttribute::NAME_CENTER, org, 3);
   wunilHeader.addAttribute(nc);
   
   const float mmpix[3] = { fabs(firstVolume->spacing[0]), -fabs(firstVolume->spacing[1]), -fabs(firstVolume->spacing[2]) };
   WuNilAttribute nm(WuNilAttribute::NAME_MMPPIX, mmpix, 3);
   wunilHeader.addAttribute(nm);
   
   //
   // Set the study metadata
   //
   WuNilAttribute md(WuNilAttribute::NAME_CARET_METADATA,
                     firstVolume->getStudyMetaDataLinkSet().getLinkSetAsCodedText());
   wunilHeader.addAttribute(md);
   
   //
   // set the region names (note: names zero and one are skipped since
   // a voxel value of one is not allowed in a WU volume).
   //
   std::vector<QString> wuRegionNames;
   const int numRegionNames = firstVolume->getNumberOfRegionNames();
   for (int i = 2; i < numRegionNames; i++) {
      wuRegionNames.push_back(firstVolume->regionNames[i]);
   }
   wunilHeader.setRegionNames(wuRegionNames);

   //
   // write the header
   //
   QFile file(firstVolume->filename);
   if (file.open(QIODevice::WriteOnly) == false) {
      throw FileException(firstVolume->filename, file.errorString());
   }
   QTextStream stream(&file);
   wunilHeader.writeHeader(stream);
   file.close();
   
   //
   // Update file permissions ?
   //
   if (getFileWritePermissions() != 0) {
      QFile::setPermissions(firstVolume->filename, getFileWritePermissions());
   }

   //
   // Create the name of the data file
   //
   firstVolume->dataFileName = FileUtilities::filenameWithoutExtension(firstVolume->filename);
   firstVolume->dataFileName.append(".img");
   
   //
   // Open the data file
   //
   std::ofstream* cppFile = new std::ofstream(firstVolume->dataFileName.toAscii().constData(),
                                              std::ios::out | std::ios::binary);
   if (cppFile == NULL) {
      throw FileException(firstVolume->dataFileName, "Unable to open for writing");
   }
   
   //
   // WU NIL files normally created on sun workstations which are big endian
   //
   //bool bigEndian = (QSysInfo::ByteOrder == QSysInfo::BigEndian);
   //const bool byteSwapFlag = (bigEndian == false);
   //
   // WU NIL volumes now support both big and little endian
   //
   const bool byteSwapFlag = false;
   
   //
   // Write the volume data
   //
   QString writeErrorMessage;
   for (int i = 0; i < numSubVolumes; i++) {
      try {
         //
         // Need to flip to RAI as WU volumes are always RAI
         //
         VolumeFile copyOfVolume(*volumesToWrite[i]);
         copyOfVolume.flip(VOLUME_AXIS_X);
         copyOfVolume.flip(VOLUME_AXIS_Y);
         copyOfVolume.writeVolumeFileData(firstVolume->voxelDataType,
                                                byteSwapFlag,
                                                false,
                                                NULL,
                                                cppFile,
                                                1.0);
      }
      catch (FileException& e) {
         writeErrorMessage = e.whatQString();
         break;
      }
   }
   
   //
   // Close the data file
   //
   cppFile->close();
   delete cppFile;

   if (writeErrorMessage.isEmpty() == false) {
      throw FileException(firstVolume->dataFileName, writeErrorMessage);
   }

   //
   // Update file permissions ?
   //
   if (getFileWritePermissions() != 0) {
      QFile::setPermissions(firstVolume->dataFileName, getFileWritePermissions());
   }
}                          
                      
/**
 * read the volume data.
 */
void 
VolumeFile::readVolumeFileData(const bool byteSwapNeeded,
                               const float scaleFact,
                               const float offsetFact,
                               gzFile dataFile) throw (FileException)
{
   QString errorMessage;
   
   int dataSizeInBytes = 0;
   numberOfComponentsPerVoxel = 1;
   
   switch(voxelDataType) {
      case VOXEL_DATA_TYPE_CHAR:
      case VOXEL_DATA_TYPE_CHAR_UNSIGNED:
         dataSizeInBytes = 1;
         break;
      case VOXEL_DATA_TYPE_SHORT:
      case VOXEL_DATA_TYPE_SHORT_UNSIGNED:
         dataSizeInBytes = 2;
         break;
      case VOXEL_DATA_TYPE_INT:
      case VOXEL_DATA_TYPE_INT_UNSIGNED:
         dataSizeInBytes = 4;
         break;
      case VOXEL_DATA_TYPE_LONG:
      case VOXEL_DATA_TYPE_LONG_UNSIGNED:
         dataSizeInBytes = 8;
         break;
      case VOXEL_DATA_TYPE_FLOAT:
         dataSizeInBytes = 4;
         break;
      case VOXEL_DATA_TYPE_DOUBLE:
         dataSizeInBytes = 8;
         break;
      case VOXEL_DATA_TYPE_RGB_VOXEL_INTERLEAVED:
         dataSizeInBytes = 1;
         numberOfComponentsPerVoxel = 3;
         break;
      case VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED:
         dataSizeInBytes = 1;
         numberOfComponentsPerVoxel = 3;
         break;
      case VOXEL_DATA_TYPE_VECTOR:
         dataSizeInBytes = 4;
         numberOfComponentsPerVoxel = 4;
         break;
      case VOXEL_DATA_TYPE_UNKNOWN:
         dataSizeInBytes = 0;
         break;
   }
   if (dataSizeInBytes <= 0) {
      errorMessage.append("Invalid data type to data type not set.");
   }
   
   int dimensions[3];
   getDimensions(dimensions);
   if ((dimensions[0] <= 0) || (dimensions[1] <= 0) || (dimensions[2] <= 0)) {
      errorMessage.append("Dimensions must be greater than zero.");
   }
   
   if (errorMessage.isEmpty() == false) {
      throw FileException(FileUtilities::basename(dataFileName), errorMessage);
   }
   

   if (voxels != NULL) {
      delete[] voxels;
      voxels = NULL;
   }
   voxels = new float[getTotalNumberOfVoxelElements()];
   
   allocateVoxelColoring();

   try {
      switch(voxelDataType) {
         case VOXEL_DATA_TYPE_CHAR:
            readCharData(dataFile);
            break;
         case VOXEL_DATA_TYPE_CHAR_UNSIGNED:
            readUnsignedCharData(dataFile);
            break;
         case VOXEL_DATA_TYPE_SHORT:
            readShortData(dataFile, byteSwapNeeded);
            break;
         case VOXEL_DATA_TYPE_SHORT_UNSIGNED:
            readUnsignedShortData(dataFile, byteSwapNeeded);
            break;
         case VOXEL_DATA_TYPE_INT:
            readIntData(dataFile, byteSwapNeeded);
            break;
         case VOXEL_DATA_TYPE_INT_UNSIGNED:
            readUnsignedIntData(dataFile, byteSwapNeeded);
            break;
         case VOXEL_DATA_TYPE_LONG:
            readLongLongData(dataFile, byteSwapNeeded);
            break;
         case VOXEL_DATA_TYPE_LONG_UNSIGNED:
            readUnsignedLongLongData(dataFile, byteSwapNeeded);
            break;
         case VOXEL_DATA_TYPE_FLOAT:
            readFloatData(dataFile, byteSwapNeeded);
            break;
         case VOXEL_DATA_TYPE_DOUBLE:
            readDoubleData(dataFile, byteSwapNeeded);
            break;
         case VOXEL_DATA_TYPE_RGB_VOXEL_INTERLEAVED:
            readRgbDataVoxelInterleaved(dataFile);
            break;
         case VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED:
            readRgbDataSliceInterleaved(dataFile);
            break;
         case VOXEL_DATA_TYPE_VECTOR:
            readFloatData(dataFile, byteSwapNeeded);
            break;
         case VOXEL_DATA_TYPE_UNKNOWN:
            throw FileException(FileUtilities::basename(dataFileName), "Unknown data type.");
            break;
      }
   }
   catch (FileException& e) {
      throw e;
   }
        
   if ((voxelDataType != VOXEL_DATA_TYPE_RGB_VOXEL_INTERLEAVED) &&
       (voxelDataType != VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED)) {
      //
      // Note: Raw volumes do not have scale factors
      //
      float scaledMin = voxels[0] * scaleFact + offsetFact;
      float scaledMax = voxels[0] * scaleFact + offsetFact;
      float unscaledMin = voxels[0];
      float unscaledMax = voxels[0];
      const int totalNum = getTotalNumberOfVoxelElements();
      for (int i = 0; i < totalNum; i++) {
         float s = voxels[i];
         if (s < unscaledMin) unscaledMin = s;
         if (s > unscaledMax) unscaledMax = s;
         if (scaleFact != 0.0) {
            s *= scaleFact;
         }
         s += offsetFact;
         if (s < scaledMin) scaledMin = s;
         if (s > scaledMax) scaledMax = s;
         voxels[i] = s;
      }
      if (DebugControl::getDebugOn()) {
         std::cout << "Unscaled range: " << unscaledMin << " " << unscaledMax << std::endl;
         std::cout << "Scaled range: " << scaledMin << " " << scaledMax << std::endl;
      }
   }

   //
   // WuNil volume data for paint file starts at 2
   //
   if ((fileReadType == FILE_READ_WRITE_TYPE_WUNIL) && 
         ((volumeType == VOLUME_TYPE_PAINT) ||
          (volumeType == VOLUME_TYPE_PROB_ATLAS))) {
      const int totalNum = getTotalNumberOfVoxelElements();
      for (int i = 0; i < totalNum; i++) {
         float val = voxels[i];
         if (val > 0.0) {
            val += 1.0;
            voxels[i] = val;
         }
      }
   }

   //
   // Should the volume be made LPI
   //
   switch (volumeSpace) {
      case VOLUME_SPACE_COORD_LPI:
         if (isValidOrientation(orientation)) {
            ORIENTATION newOrient[3] = {
               ORIENTATION_LEFT_TO_RIGHT,
               ORIENTATION_POSTERIOR_TO_ANTERIOR,
               ORIENTATION_INFERIOR_TO_SUPERIOR
            };
            try {
               permuteToOrientation(newOrient);
            }
            catch (FileException& e) {
            }
            
            //
            // Adjustment made for WU's IFH (28 April 2005)
            //
            //if (fileReadType == FILE_READ_WRITE_TYPE_WUNIL) {
            //   origin[0] += 1.0;
            //   origin[1] += 1.0;
            //   origin[2] += 1.0;
            //}
         }
         break;
      case VOLUME_SPACE_VOXEL_NATIVE:
         //{
         //   const float zeros[3] = { 0.0, 0.0, 0.0 };
         //   setOrigin(zeros);
         //   if (fileReadType != FILE_READ_WRITE_TYPE_SPM_OR_MEDX) {
         //      const float ones[3] = { 1.0, 1.0, 1.0 };
         //      setSpacing(ones);
         //   }
         //}
         break;
   }

   clearModified();
}                          

/**
 * read the volume data.
 */
void 
VolumeFile::readVolumeFileDataSubVolume(const bool byteSwapNeeded,
                                        const float scaleFact,
                                        const float offsetFact,
                                        const long dataOffset,
                                        const int subVolumeNumber,
                                        gzFile dataFile) throw (FileException)
{
   int dataSizeInBytes = 0;
   numberOfComponentsPerVoxel = 1;
   
   switch(voxelDataType) {
      case VOXEL_DATA_TYPE_CHAR:
      case VOXEL_DATA_TYPE_CHAR_UNSIGNED:
         dataSizeInBytes = 1;
         break;
      case VOXEL_DATA_TYPE_SHORT:
      case VOXEL_DATA_TYPE_SHORT_UNSIGNED:
         dataSizeInBytes = 2;
         break;
      case VOXEL_DATA_TYPE_INT:
      case VOXEL_DATA_TYPE_INT_UNSIGNED:
         dataSizeInBytes = 4;
         break;
      case VOXEL_DATA_TYPE_LONG:
      case VOXEL_DATA_TYPE_LONG_UNSIGNED:
         dataSizeInBytes = 8;
         break;
      case VOXEL_DATA_TYPE_FLOAT:
         dataSizeInBytes = 4;
         break;
      case VOXEL_DATA_TYPE_DOUBLE:
         dataSizeInBytes = 8;
         break;
      case VOXEL_DATA_TYPE_RGB_VOXEL_INTERLEAVED:
         dataSizeInBytes = 1;
         numberOfComponentsPerVoxel = 3;
         break;
      case VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED:
         dataSizeInBytes = 1;
         numberOfComponentsPerVoxel = 3;
         break;
      case VOXEL_DATA_TYPE_VECTOR:
         dataSizeInBytes = 4;
         numberOfComponentsPerVoxel = 4;
         break;
      case VOXEL_DATA_TYPE_UNKNOWN:
         dataSizeInBytes = 0;
         break;
   }
   
   //
   // Determine the offset
   //
   const z_off_t offset = (subVolumeNumber * dataSizeInBytes * numberOfComponentsPerVoxel *
                       dimensions[0] * dimensions[1] * dimensions[2])
                    + dataOffset;
   if (DebugControl::getDebugOn()) {
      std::cout << "Data offset: " << offset << std::endl;
   }
   
   //
   // read the data
   //
   gzseek(dataFile, 
          offset, 
          SEEK_SET);
   readVolumeFileData(byteSwapNeeded, 
                      scaleFact,
                      offsetFact,
                      dataFile);
}
      
/**
 * write the volume data.
 */
void 
VolumeFile::writeVolumeFileData(const VOXEL_DATA_TYPE voxelDataTypeForWriting,
                                const bool byteSwapNeeded,
                                const bool compressDataWithZlib,
                                gzFile zipStream,
                                std::ofstream* cppStream,
                                const float divideByThisValue) throw (FileException)
{
   if (voxelDataType == VOXEL_DATA_TYPE_UNKNOWN) {
      throw FileException("Unknown data type for writing.");
   }
   
   //
   // Write the data
   //
   const int numVoxels = getTotalNumberOfVoxels();   
   switch(voxelDataTypeForWriting) {
      case VOXEL_DATA_TYPE_CHAR:
         {
            char* data = new char[numVoxels];
            for (int i = 0; i < numVoxels; i++) {
               data[i] = static_cast<char>(voxels[i] / divideByThisValue);
            }
            if (compressDataWithZlib) {
               gzwrite(zipStream, (void*)data, numVoxels * sizeof(char));
            }
            else {
               cppStream->write((const char*)data, numVoxels * sizeof(char));
            }
            delete[] data;
         }
         break;
      case VOXEL_DATA_TYPE_CHAR_UNSIGNED:
         {
            unsigned char* data = new unsigned char[numVoxels];
            for (int i = 0; i < numVoxels; i++) {
               data[i] = static_cast<unsigned char>(voxels[i] / divideByThisValue);
            }
            if (compressDataWithZlib) {
               gzwrite(zipStream, (void*)data, numVoxels * sizeof(unsigned char));
            }
            else {
               cppStream->write((const char*)data, numVoxels * sizeof(unsigned char));
            }
            delete[] data;
         }
         break;
      case VOXEL_DATA_TYPE_SHORT:
         {
            short* data = new short[numVoxels];
            for (int i = 0; i < numVoxels; i++) {
               data[i] = static_cast<short>(voxels[i] / divideByThisValue);
            }
            if (byteSwapNeeded) {
               ByteSwapping::swapBytes(data, numVoxels);
            }
            if (compressDataWithZlib) {
               gzwrite(zipStream, (void*)data, numVoxels * sizeof(short));
            }
            else {
               cppStream->write((const char*)data, numVoxels * sizeof(short));
            }
            delete[] data;
         }
         break;
      case VOXEL_DATA_TYPE_SHORT_UNSIGNED:
         {
            unsigned short* data = new unsigned short[numVoxels];
            for (int i = 0; i < numVoxels; i++) {
               data[i] = static_cast<unsigned short>(voxels[i] / divideByThisValue);
            }
            if (byteSwapNeeded) {
               ByteSwapping::swapBytes(data, numVoxels);
            }
            if (compressDataWithZlib) {
               gzwrite(zipStream, (void*)data, numVoxels * sizeof(unsigned short));
            }
            else {
               cppStream->write((const char*)data, numVoxels * sizeof(unsigned short));
            }
            delete[] data;
         }
         break;
      case VOXEL_DATA_TYPE_INT:
         {
            int* data = new int[numVoxels];
            for (int i = 0; i < numVoxels; i++) {
               data[i] = static_cast<int>(voxels[i] / divideByThisValue);
            }
            if (byteSwapNeeded) {
               ByteSwapping::swapBytes(data, numVoxels);
            }
            if (compressDataWithZlib) {
               gzwrite(zipStream, (void*)data, numVoxels * sizeof(int));
            }
            else {
               cppStream->write((const char*)data, numVoxels * sizeof(int));
            }
            delete[] data;
         }
         break;
      case VOXEL_DATA_TYPE_INT_UNSIGNED:
         {
            unsigned int* data = new unsigned int[numVoxels];
            for (int i = 0; i < numVoxels; i++) {
               data[i] = static_cast<unsigned int>(voxels[i] / divideByThisValue);
            }
            if (byteSwapNeeded) {
               ByteSwapping::swapBytes(data, numVoxels);
            }
            if (compressDataWithZlib) {
               gzwrite(zipStream, (void*)data, numVoxels * sizeof(unsigned int));
            }
            else {
               cppStream->write((const char*)data, numVoxels * sizeof(unsigned int));
            }
            delete[] data;
         }
         break;
      case VOXEL_DATA_TYPE_LONG:
         {
            long long* data = new long long[numVoxels];
            for (int i = 0; i < numVoxels; i++) {
               data[i] = static_cast<long long>(voxels[i] / divideByThisValue);
            }
            if (byteSwapNeeded) {
               ByteSwapping::swapBytes(data, numVoxels);
            }
            if (compressDataWithZlib) {
               gzwrite(zipStream, (void*)data, numVoxels * sizeof(long long));
            }
            else {
               cppStream->write((const char*)data, numVoxels * sizeof(long long));
            }
            delete[] data;
         }
         break;
      case VOXEL_DATA_TYPE_LONG_UNSIGNED:
         {
            unsigned long long* data = new unsigned long long[numVoxels];
            for (int i = 0; i < numVoxels; i++) {
               data[i] = static_cast<unsigned long long>(voxels[i] / divideByThisValue);
            }
            if (byteSwapNeeded) {
               ByteSwapping::swapBytes(data, numVoxels);
            }
            if (compressDataWithZlib) {
               gzwrite(zipStream, (void*)data, numVoxels * sizeof(unsigned long long));
            }
            else {
               cppStream->write((const char*)data, numVoxels * sizeof(unsigned long long));
            }
            delete[] data;
         }
         break;
      case VOXEL_DATA_TYPE_FLOAT:
         {
            float* data = new float[numVoxels];
            for (int i = 0; i < numVoxels; i++) {
               data[i] = static_cast<float>(voxels[i] / divideByThisValue);
            }
            if (byteSwapNeeded) {
               ByteSwapping::swapBytes(data, numVoxels);
            }
            if (compressDataWithZlib) {
               gzwrite(zipStream, (void*)data, numVoxels * sizeof(float));
            }
            else {
               cppStream->write((const char*)data, numVoxels * sizeof(float));
            }
            delete[] data;
         }
         break;
      case VOXEL_DATA_TYPE_DOUBLE:
         {
            double* data = new double[numVoxels];
            for (int i = 0; i < numVoxels; i++) {
               data[i] = static_cast<double>(voxels[i] / divideByThisValue);
            }
            if (byteSwapNeeded) {
               ByteSwapping::swapBytes(data, numVoxels);
            }
            if (compressDataWithZlib) {
               gzwrite(zipStream, (void*)data, numVoxels * sizeof(double));
            }
            else {
               cppStream->write((const char*)data, numVoxels * sizeof(double));
            }
            delete[] data;
         }
         break;
      case VOXEL_DATA_TYPE_RGB_VOXEL_INTERLEAVED:
         {
            unsigned char* data = new unsigned char[numVoxels * 3];
            for (int i = 0; i < numVoxels; i++) {
               data[i*3]   = static_cast<unsigned char>(voxels[i*3]);
               data[i*3+1] = static_cast<unsigned char>(voxels[i*3+1]);
               data[i*3+2] = static_cast<unsigned char>(voxels[i*3+2]);
            }
            if (compressDataWithZlib) {
               gzwrite(zipStream, (void*)data, numVoxels * 3 * sizeof(unsigned char));
            }
            else {
               cppStream->write((const char*)data, numVoxels * 3 * sizeof(unsigned char));
            }
            delete[] data;
         }
         break;
      case VOXEL_DATA_TYPE_RGB_SLICE_INTERLEAVED:
         {
            unsigned char* data = new unsigned char[numVoxels * 3];
            int ctr = 0;
            for (int k = 0; k < dimensions[2]; k++) {
               for (int n = 0; n < 3; n++) {
                  for (int j = 0; j < dimensions[1]; j++) {
                     for (int i = 0; i < dimensions[0]; i++) {
                        int ijk[3] = { i, j, k };
                        const int id = getVoxelDataIndex(ijk, n);
                        data[ctr] = static_cast<unsigned char>(voxels[id]);
                        ctr++;
                     }
                  }
               }
            }
            if (compressDataWithZlib) {
               gzwrite(zipStream, (void*)data, numVoxels * 3 * sizeof(unsigned char));
            }
            else {
               cppStream->write((const char*)data, numVoxels * 3 * sizeof(unsigned char));
            }
            delete[] data;
         }
         break;
      case VOXEL_DATA_TYPE_VECTOR:
         {
            float* data = new float[numVoxels];
            for (int i = 0; i < numVoxels; i++) {
               data[i] = static_cast<float>(voxels[i] / divideByThisValue);
            }
            if (byteSwapNeeded) {
               ByteSwapping::swapBytes(data, numVoxels);
            }
            if (compressDataWithZlib) {
               gzwrite(zipStream, (void*)data, numVoxels * sizeof(float));
            }
            else {
               cppStream->write((const char*)data, numVoxels * sizeof(float));
            }
            delete[] data;
         }
         break;
      case VOXEL_DATA_TYPE_UNKNOWN:
         throw FileException("Unknown data type for writing.");
         break;
   }
}                          

/**
 * perform uniformity correction with AFNI 3duniformize.
 */
void 
VolumeFile::afniUniformityCorrection(const int grayMin,
                                     const int whiteMax,
                                     const int iterations) throw (FileException)
{
   throw FileException("PROGRAMMER: use biasCorrectionWithAFNI() instead of afniUniformityCorrection");

   //
   // Make sure program exists
   //
   const QString programName("3duniformize");
   if (SystemUtilities::externalProgramExists(programName) == false) {
      throw FileException("AFNI program \"" + programName + "\" not found in PATH.");
   }
   
   //
   // Copy "this" volume
   //
   VolumeFile volumeCopy(*this);
   
   //
   // Create temporary names for input and output
   //
   QString inputName("caret-3d-uniform-input+orig.HEAD");
   QString inputNameData("caret-3d-uniform-input+orig.BRIK");
   QString outputPrefix("caret-3d-uniform-output");
   QString outputName(outputPrefix + "+orig.HEAD");
   QString outputNameData(outputPrefix + "+orig.BRIK");
   
   //
   // Make sure output files do not exist
   //
   QFile::remove(outputName);
   QFile::remove(outputNameData);
   
   //
   // Write out volume with temporary name and short data type
   //
   std::vector<VolumeFile*> volumes;
   volumes.push_back(&volumeCopy);
   VolumeFile::writeFile(inputName,
                         getVolumeType(),
                         VolumeFile::VOXEL_DATA_TYPE_SHORT,
                         volumes);
   
   //
   // Arguments to run program
   //
   QStringList args;
   args << "-clip_low"
        << QString::number(grayMin)
        << "-clip_high"
        << QString::number(whiteMax)
        << "-niter"
        << QString::number(iterations)
        << "-prefix"
        << outputPrefix
        << "-anat"
        << inputName;

   //
   // Create and run the QProcess to execute the program
   //
   QProcess uniform;
   uniform.start(programName, args);
   uniform.waitForStarted();
   uniform.waitForFinished();
   
   //
   // See if program was successful
   //
   if ((uniform.exitStatus() == QProcess::NormalExit) &&
       (uniform.error() == QProcess::UnknownError)) {
      if (DebugControl::getDebugOn()) {
         std::cout << "3duniformize output:" << std::endl;
         std::cout << "   stdout: " << QString(uniform.readAllStandardOutput()).toAscii().constData() << std::endl;
         std::cout << "   stderr: " << QString(uniform.readAllStandardError()).toAscii().constData() << std::endl;
      }
      
      //
      // Read the output of the program
      //
      volumeCopy.readFile(outputName);
      
      //
      // Copy the voxels
      //
      const int num = volumeCopy.getTotalNumberOfVoxels();
      for (int i = 0; i < num; i++) {
         setVoxelWithFlatIndex(i, 0, volumeCopy.getVoxelWithFlatIndex(i));
      }
      stretchVoxelValues();
      setVoxelColoringInvalid();
      
      //
      // Clean up
      //
      QFile::remove(inputName);
      QFile::remove(inputNameData);
      QFile::remove(outputName);
      QFile::remove(outputNameData);
   }
   else {
      QString msg(uniform.readAllStandardError() + "\n\n" + uniform.readAllStandardOutput());
      throw FileException(msg);
   }
}                                    

/**
 * set study meta data link.
 */
/*
void 
VolumeFile::setStudyMetaDataLinkSet(const StudyMetaDataLinkSet& smdls) 
{ 
   studyMetaDataLinkSet = smdls; 
   setModified();
}
*/      

/**
 * get NIFTI intention and tr.
 */
void 
VolumeFile::getNiftiInfo(QString& intentCodeAndParamStringOut,
                        QString& intentNameOut,
                        int& intentCodeOut,
                        float& intentParameter1Out,
                        float& intentParameter2Out,
                        float& intentParameter3Out,
                        float &trOut) const
{
   intentCodeAndParamStringOut = niftiIntentCodeAndParamString;
   intentNameOut = niftiIntentName;
   intentCodeOut = niftiIntentCode;
   intentParameter1Out = niftiIntentParameter1;
   intentParameter2Out = niftiIntentParameter2;
   intentParameter3Out = niftiIntentParameter3;
   trOut = niftiTR;
}

/**
 * compare a file for unit testing (returns true if "within tolerance").
 */
bool 
VolumeFile::compareFileForUnitTesting(const AbstractFile* af,
                                      const float tolerance,
                                      QString& messageOut) const
{
   messageOut = "";
   
   const VolumeFile* vf = dynamic_cast<const VolumeFile*>(af);
   if (vf == NULL) {
      messageOut += "ERROR: File for comparison is not a Volume File.\n";
      return false;
   }

   bool compareVoxelsFlag = true;
   
   if ((dimensions[0] != vf->dimensions[0]) ||
       (dimensions[1] != vf->dimensions[1]) ||
       (dimensions[2] != vf->dimensions[2])) {
      messageOut += "ERROR: The volumes have a different number dimensions.\n";
      compareVoxelsFlag = false;
   }
   
   if ((orientation[0] != vf->orientation[0]) ||
       (orientation[1] != vf->orientation[1]) ||
       (orientation[2] != vf->orientation[2])) {
      messageOut += "ERROR: The volumes have different orientations.\n";
   }
   
   if ((origin[0] != vf->origin[0]) ||
       (origin[1] != vf->origin[1]) ||
       (origin[2] != vf->origin[2])) {
      messageOut += "ERROR: The volumes have different origins.\n";
   }
   
   if ((spacing[0] != vf->spacing[0]) ||
       (spacing[1] != vf->spacing[1]) ||
       (spacing[2] != vf->spacing[2])) {
      messageOut += "ERROR: The volumes have different voxel sizes.\n";
   }
   
   if (compareVoxelsFlag) {
      const int numVoxels = getTotalNumberOfVoxelElements();
      if (numVoxels == vf->getTotalNumberOfVoxelElements()) {
         int diffCount = 0;
         for (int i = 0; i < numVoxels; i++) {
            float diff = (getVoxelWithFlatIndex(i) - vf->getVoxelWithFlatIndex(i));
            if (diff < 0) diff = -diff;
            if (diff > tolerance) {
               diffCount++;
            }
         }
         if (diffCount > 0) {
            messageOut += "ERROR: There are " 
                          + QString::number(diffCount)
                          + " voxels with a difference that are greater than "
                          + QString::number(tolerance, 'f', 3)
                          + "\n";
         }
      }
      else {
         messageOut += "ERROR: The volumes have a different number of voxels.\n";
      }
   }
   
   return messageOut.isEmpty();
}                                     

/**
 * Find each of the disconnect objects (islands) within the volume.
 * A voxel is considered part of the segmentation if its value is greater than or equal to one.
 */
void	
VolumeFile::findObjectsWithinSegmentationVolume(std::vector<VoxelGroup>& objectsOut) const
{
   objectsOut.clear();
   
   VoxelIJK bigSeed(-1, -1, -1);

   int imin = 0;
   int imax = 0;
   int jmin = 0;
   int jmax = 0;
   int kmin = 0;
   int kmax = 0;
   getDimensions(imax, jmax, kmax);
   
   //
   // Make sure search region is within the volume
   //
   clampVoxelDimension(VOLUME_AXIS_X, imin);
   clampVoxelDimension(VOLUME_AXIS_X, imax);
   clampVoxelDimension(VOLUME_AXIS_Y, jmin);
   clampVoxelDimension(VOLUME_AXIS_Y, jmax);
   clampVoxelDimension(VOLUME_AXIS_Z, kmin);
   clampVoxelDimension(VOLUME_AXIS_Z, kmax);

   const int numVoxels = getTotalNumberOfVoxels();
   
   const float minValue = 1.0;
   const float maxValue = std::numeric_limits<float>::max();
   
   //
   // single slice ?
   //
	if ((imin == imax) || (jmin == jmax) || (kmin == kmax)) {
      if (DebugControl::getDebugOn()) {
         std::cout << "For x, y or z, min = max" << std::endl;
      }
   }
	else {
      //
      // Create a flag for noting which voxels have been searched.
      // Mark those within region and within min and max values NOT SEARCHED
      // and all others SEARCHED.
      //
		VOXEL_SEARCH_STATUS* voxelSearched = new VOXEL_SEARCH_STATUS[numVoxels];
		for (int k = 0; k < dimensions[2]; k++) {
			for (int j = 0; j < dimensions[1]; j++) {
				for (int i = 0; i < dimensions[0]; i++) {
               const int idx = getVoxelDataIndex(i, j, k);
               voxelSearched[idx] = VOXEL_SEARCHED;
               if ((i >= imin) && (i < imax) &&
                   (j >= jmin) && (j < jmax) &&
                   (k >= kmin) && (k < kmax)) {
                  if ((voxels[idx] >= minValue) &&
                      (voxels[idx] <= maxValue)) {
                     voxelSearched[idx] = VOXEL_NOT_SEARCHED;
                  }
               }
				}
			}
		}
	
      //
      // Find a voxel within value range that has not been searched
      //
      VoxelIJK seedVoxel;
      bool voxelFound = findUnsearchedVoxel(minValue, maxValue, voxelSearched, seedVoxel);
      
      if (voxelFound == false) {
         if (DebugControl::getDebugOn()) {
            std::cout << "FindBiggestObjectWithinMask no initial voxel found with values: "
                      << minValue << " " << maxValue << std::endl;
         }
      }
      
      //
      // Loop through all objects
      //
      while (voxelFound) {
         //
         // Create a stack 
         //
         std::stack<VoxelIJK> stack;
         stack.push(seedVoxel);
         
         //
         // Stores voxels in the object
         //
         VoxelGroup voxelGroupObject;
         
         //
         // While there are voxels to search
         //
         while (stack.empty() == false) {
            //
            // Get the next voxel to search
            //
            const VoxelIJK v = stack.top();
            stack.pop();
          
            const int idx = getVoxelDataIndex(v.getIJK());
            if (voxelSearched[idx] == VOXEL_NOT_SEARCHED) {
               voxelSearched[idx] = VOXEL_SEARCHED;
               
               //
               // Add voxel to group
               //
               voxelGroupObject.addVoxel(v);
               
               //
               // Get the neighboring voxels
               //
               std::vector<VoxelIJK> neighbors;
               getNeighbors(v, neighbors);
               
               //
               // Add neighbors to stack
               //
               for (unsigned int i = 0; i < neighbors.size(); i++) {
                  VoxelIJK& v = neighbors[i];
                  const int idx = getVoxelDataIndex(v.getIJK());
                  if (voxelSearched[idx] == VOXEL_NOT_SEARCHED) {
                     stack.push(v);
                  }
               }
            }
         }  // while (stack.isEmpty() == false)
         
         //
         // Was an object found
         //
         if (voxelGroupObject.getNumberOfVoxels()) {
            objectsOut.push_back(voxelGroupObject);
         }
         
         //
         // Find another voxel to search
         //
         voxelFound = findUnsearchedVoxel(minValue, maxValue, voxelSearched, seedVoxel);
      } // while (voxelFound)

		delete[] voxelSearched;
	}	
}

/**
 * bias correction using AFNI's 3dUniformize code.
 */
void 
VolumeFile::biasCorrectionWithAFNI(const int grayMinimumIn,
                                   const int whiteMaximumIn,
                                   const int numberOfIterations) throw (FileException)
{  
   int grayMinimum = grayMinimumIn;
   int whiteMaximum = whiteMaximumIn;
   
   //
   // Get range of input voxels
   //
   float minVoxel, maxVoxel;
   getMinMaxVoxelValues(minVoxel, maxVoxel);
   
   //
   // Get value of gray and white relative to minimum value
   //
   const float diff = maxVoxel - minVoxel;
   const float grayRelative = (static_cast<float>(grayMinimum) - minVoxel) / diff;
   const float whiteRelative = (static_cast<float>(whiteMaximum) - minVoxel) / diff;
   
   //
   // stretch out the voxels
   //
   const float maxStretchValue = 255.0;
   rescaleVoxelValues(minVoxel, maxVoxel, 0.0, maxStretchValue);
   //stretchVoxelValues();
   
   //
   // Remap gray and white values
   //
   grayMinimum = static_cast<int>(grayRelative * maxStretchValue);
   whiteMaximum = static_cast<int>(whiteRelative * maxStretchValue);
   
   //
   // Use AFNI's bias correction algorithm
   //
   biasCorrectVolume(this, grayMinimum, whiteMaximum, numberOfIterations);
   
   //
   // Stretch the voxels values excluding 1% at each extreme
   //
   stretchVoxelValuesExcludePercentage(1.0, 1.0);
   
   //
   // Invalidate coloring since voxel values changed
   //
   setVoxelColoringInvalid();
}
                                  
/**
 * get data file name for read error (if data file name empty, return file name).
 */
QString 
VolumeFile::getDataFileNameForReadError() const
{
   QString name = dataFileName;
   if (name.isEmpty()) {
      name = getFileName();
   }

   name = FileUtilities::basename(name);

   return name;
}
      
/**
 * Write the file's memory in caret6 format to the specified name.
 */
QString
VolumeFile::writeFileInCaret6Format(const QString& filenameIn, Structure structure,const ColorFile* colorFileIn, const bool useCaret6ExtensionFlag) throw (FileException)
{
   //
   // The generic method to read a volume file only reads the first volume
   // so read again to get all volumes
   //
   QString nameToRead = this->getFileName();
   std::vector<VolumeFile*> volumes;
   VolumeFile::readFile(nameToRead, VOLUME_READ_SELECTION_ALL, volumes, false);
   if (volumes.size() > 0) {
      QString name = filenameIn;
      if (useCaret6ExtensionFlag) {
         if (name.endsWith(SpecFile::getNiftiGzipVolumeFileExtension()) == false) {
            if (name.endsWith(SpecFile::getAfniVolumeFileExtension())) {
               name = FileUtilities::replaceExtension(filenameIn,
                                   SpecFile::getAfniVolumeFileExtension(),
                                   SpecFile::getNiftiGzipVolumeFileExtension());
            }
            else if (name.endsWith(SpecFile::getAnalyzeVolumeFileExtension())) {
               name = FileUtilities::replaceExtension(filenameIn,
                                   SpecFile::getAnalyzeVolumeFileExtension(),
                                   SpecFile::getNiftiGzipVolumeFileExtension());
            }
            else if (name.endsWith(SpecFile::getNiftiVolumeFileExtension())) {
               name = FileUtilities::replaceExtension(filenameIn,
                                   SpecFile::getNiftiVolumeFileExtension(),
                                   SpecFile::getNiftiGzipVolumeFileExtension());
            }
            else if (name.endsWith(SpecFile::getWustlVolumeFileExtension())) {
               name = FileUtilities::replaceExtension(filenameIn,
                                   SpecFile::getWustlVolumeFileExtension(),
                                   SpecFile::getNiftiGzipVolumeFileExtension());
            }
            else {
               name = FileUtilities::replaceExtension(filenameIn,
                                   "XXXXXXXXXXXXXXXXXXXXXXXXXXXX",
                                   SpecFile::getNiftiGzipVolumeFileExtension());
            }
         }
      }

      VOLUME_TYPE volumeType = volumes[0]->getVolumeType();
      VOXEL_DATA_TYPE voxelDataType = volumes[0]->getVoxelDataType();
      VolumeFile::writeFile(name,
                            volumeType,
                            voxelDataType,
                            volumes,
                            true,
                            colorFileIn,
                            6);

      return name;
   }

   return "";
}

/**
 * Write the file's memory in caret7 format to the specified name.
 */
QString
VolumeFile::writeFileInCaret7Format(const QString& filenameIn, 
                                    Structure structure,
                                    const ColorFile* colorFileIn, 
                                    const bool useCaret7ExtensionFlag) throw (FileException)
{
    //
    // The generic method to read a volume file only reads the first volume
    // so read again to get all volumes
    //
    QString nameToRead = this->getFileName();
    std::vector<VolumeFile*> volumes;
    VolumeFile::readFile(nameToRead, VOLUME_READ_SELECTION_ALL, volumes, false);
    if (volumes.size() > 0) {
        QString name = filenameIn;
        if (useCaret7ExtensionFlag) {
            if (name.endsWith(SpecFile::getNiftiGzipVolumeFileExtension()) == false) {
                if (name.endsWith(SpecFile::getAfniVolumeFileExtension())) {
                    name = FileUtilities::replaceExtension(filenameIn,
                                                           SpecFile::getAfniVolumeFileExtension(),
                                                           SpecFile::getNiftiGzipVolumeFileExtension());
                }
                else if (name.endsWith(SpecFile::getAnalyzeVolumeFileExtension())) {
                    name = FileUtilities::replaceExtension(filenameIn,
                                                           SpecFile::getAnalyzeVolumeFileExtension(),
                                                           SpecFile::getNiftiGzipVolumeFileExtension());
                }
                else if (name.endsWith(SpecFile::getNiftiVolumeFileExtension())) {
                    name = FileUtilities::replaceExtension(filenameIn,
                                                           SpecFile::getNiftiVolumeFileExtension(),
                                                           SpecFile::getNiftiGzipVolumeFileExtension());
                }
                else if (name.endsWith(SpecFile::getWustlVolumeFileExtension())) {
                    name = FileUtilities::replaceExtension(filenameIn,
                                                           SpecFile::getWustlVolumeFileExtension(),
                                                           SpecFile::getNiftiGzipVolumeFileExtension());
                }
                else {
                    name = FileUtilities::replaceExtension(filenameIn,
                                                           "XXXXXXXXXXXXXXXXXXXXXXXXXXXX",
                                                           SpecFile::getNiftiGzipVolumeFileExtension());
                }
            }
        }
        
        VOLUME_TYPE volumeType = volumes[0]->getVolumeType();
        VOXEL_DATA_TYPE voxelDataType = volumes[0]->getVoxelDataType();
        VolumeFile::writeFile(name,
                              volumeType,
                              voxelDataType,
                              volumes,
                              true,
                              colorFileIn,
                              7);
        
        return name;
    }
    
    return "";
}

