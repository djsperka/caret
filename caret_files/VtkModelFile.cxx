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

#include <QFileInfo>
#include <QTime>

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkLookupTable.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"
#include "vtkTriangleFilter.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"

#include "BorderColorFile.h"
#include "BorderFile.h"
#include "CellColorFile.h"
#include "CellFile.h"
#include "DebugControl.h"
#include "FileUtilities.h"
#include "FociColorFile.h"
#include "FociFile.h"
#include "SpecFile.h"
#include "VtkModelFile.h"

/**
 * Constructor.
 */
VtkModelFile::VtkModelFile()
   : AbstractFile("VTK Model File",    
                  SpecFile::getVtkModelFileExtension(),
                  false, 
                  AbstractFile::FILE_FORMAT_ASCII,
                  FILE_IO_READ_AND_WRITE,
                  FILE_IO_NONE,
                  FILE_IO_READ_AND_WRITE,
                  FILE_IO_NONE)  // be sure to update all constructors
{
   clear();
}

/**
 * Constructor. (construct from polydata)
 */
VtkModelFile::VtkModelFile(vtkPolyData* polyData)
   : AbstractFile("VTK Model File", 
                  SpecFile::getVtkModelFileExtension(),
                  false, 
                  AbstractFile::FILE_FORMAT_ASCII,
                  FILE_IO_READ_AND_WRITE,
                  FILE_IO_NONE,
                  FILE_IO_READ_AND_WRITE,
                  FILE_IO_NONE)
{
   clear();
   readPolyData(polyData);
}

/**
 * Constructor - converts borders to VTK lines.
 */
VtkModelFile::VtkModelFile(const BorderFile* bf, const BorderColorFile* colors)
   : AbstractFile("VTK Model File", 
                  SpecFile::getVtkModelFileExtension(),
                  false, 
                  AbstractFile::FILE_FORMAT_ASCII,
                  FILE_IO_READ_AND_WRITE,
                  FILE_IO_NONE,
                  FILE_IO_READ_AND_WRITE,
                  FILE_IO_NONE)
{
   clear();
   if (bf == NULL) {
      return;
   }
   const int numBorders = bf->getNumberOfBorders();
   for (int i = 0; i < numBorders; i++) {
      const Border* border = bf->getBorder(i);
      const int numLinks = border->getNumberOfLinks();
      if (numLinks > 0) {
         std::vector<int> pts;
         for (int j = 0; j < numLinks; j++) {
           unsigned char rgbaColor[4] = { 170, 170, 170, 255 };
           const int colorIndex = border->getBorderColorIndex();
           if ((colorIndex >= 0) && (colorIndex < colors->getNumberOfColors())) {
              colors->getColorByIndex(colorIndex, rgbaColor[0], rgbaColor[1], 
                                                  rgbaColor[2], rgbaColor[3]);
           }

           const float normal[3] = { 0.0, 0.0, 1.0 };
           const float* xyz = border->getLinkXYZ(j);
           pts.push_back(coordinates.getNumberOfCoordinates());
           addCoordinate(xyz, rgbaColor, normal);
/*           
           pointColors.push_back(rgbaColor[0]);
           pointColors.push_back(rgbaColor[1]);
           pointColors.push_back(rgbaColor[2]);
           pointColors.push_back(rgbaColor[3]);
           
           pointNormals.push_back(0.0);
           pointNormals.push_back(0.0);
           pointNormals.push_back(1.0);
*/
         }
         
         lines.push_back(VtkModelObject(&pts[0], pts.size()));
      }
   }
}

/**
 * Constructor - converts cells to VTK vertices.
 */
VtkModelFile::VtkModelFile(const CellFile* cells, const CellColorFile* colors)
   : AbstractFile("VTK Model File", 
                  SpecFile::getVtkModelFileExtension(),
                  false, 
                  AbstractFile::FILE_FORMAT_ASCII,
                  FILE_IO_READ_AND_WRITE,
                  FILE_IO_NONE,
                  FILE_IO_READ_AND_WRITE,
                  FILE_IO_NONE)
{
   clear();
   if (cells == NULL) {
      return;
   }
   
   const int numCells = cells->getNumberOfCells();
   for (int i = 0; i < numCells; i++) {
      const CellData* cd = cells->getCell(i);
      
      unsigned char rgbaColor[4] = { 170, 170, 170, 255 };
      const int colorIndex = cd->getColorIndex();
      if ((colorIndex >= 0) && (colorIndex < colors->getNumberOfColors())) {
         colors->getColorByIndex(colorIndex, rgbaColor[0], rgbaColor[1], 
                                             rgbaColor[2], rgbaColor[3]);
      }

      const float normal[3] = { 0.0, 0.0, 1.0 };
      const float* xyz = cd->getXYZ();
      vertices.push_back(coordinates.getNumberOfCoordinates());
      addCoordinate(xyz, rgbaColor, normal);

/*     
      pointColors.push_back(rgbaColor[0]);
      pointColors.push_back(rgbaColor[1]);
      pointColors.push_back(rgbaColor[2]);
      pointColors.push_back(rgbaColor[3]);
     
      pointNormals.push_back(0.0);
      pointNormals.push_back(0.0);
      pointNormals.push_back(1.0);
*/
   }
}

/**
 * Constructor - converts foci to VTK vertices.
 */
VtkModelFile::VtkModelFile(const FociFile* foci, const FociColorFile* colors)
   : AbstractFile("VTK Model File", 
                  SpecFile::getVtkModelFileExtension(),
                  false, 
                  AbstractFile::FILE_FORMAT_ASCII,
                  FILE_IO_READ_AND_WRITE,
                  FILE_IO_NONE,
                  FILE_IO_READ_AND_WRITE,
                  FILE_IO_NONE)
{
   clear();
   if (foci == NULL) {
      return;
   }

   const int numFoci = foci->getNumberOfCells();
   for (int i = 0; i < numFoci; i++) {
      const CellData* cd = foci->getCell(i);
      
      unsigned char rgbaColor[4] = { 170, 170, 170, 255 };
      const int colorIndex = cd->getColorIndex();
      if ((colorIndex >= 0) && (colorIndex < colors->getNumberOfColors())) {
         colors->getColorByIndex(colorIndex, rgbaColor[0], rgbaColor[1], 
                                             rgbaColor[2], rgbaColor[3]);
      }

      const float normal[3] = { 0.0, 0.0, 0.0 };
      const float* xyz = cd->getXYZ();
      vertices.push_back(coordinates.getNumberOfCoordinates());
      addCoordinate(xyz, rgbaColor, normal);

/*     
      pointColors.push_back(rgbaColor[0]);
      pointColors.push_back(rgbaColor[1]);
      pointColors.push_back(rgbaColor[2]);
      pointColors.push_back(rgbaColor[3]);
     
      pointNormals.push_back(0.0);
      pointNormals.push_back(0.0);
      pointNormals.push_back(1.0);
*/
   }
}
      
/**
 * Destructor.
 */
VtkModelFile::~VtkModelFile()
{
   clear();
}

/**
 * add a coordinate.
 */
void 
VtkModelFile::addCoordinate(const float xyz[3],
                            const unsigned char* rgbaColorIn,
                            const float* normalIn)
{
   vertices.push_back(coordinates.getNumberOfCoordinates());
   coordinates.addCoordinate(xyz);
  
   unsigned char rgbaColor[4] = { 170, 170, 170, 255 };
   if (rgbaColorIn != NULL) {
      rgbaColor[0] = rgbaColorIn[0];
      rgbaColor[1] = rgbaColorIn[1];
      rgbaColor[2] = rgbaColorIn[2];
      rgbaColor[3] = rgbaColorIn[3];
   }
   pointColors.push_back(rgbaColor[0]);
   pointColors.push_back(rgbaColor[1]);
   pointColors.push_back(rgbaColor[2]);
   pointColors.push_back(rgbaColor[3]);
  
   float normal[3] = { 0.0, 0.0, 1.0 };
   if (normalIn != NULL) {
      normal[0] = normalIn[0];
      normal[1] = normalIn[1];
      normal[2] = normalIn[2];
   }
   pointNormals.push_back(normal[0]);
   pointNormals.push_back(normal[1]);
   pointNormals.push_back(normal[2]);
}

/**
 * Clears current file data in memory.
 */
void 
VtkModelFile::clear()
{
   clearAbstractFile();
   coordinates.clear();
   triangles.clear();
   vertices.clear();
   lines.clear();
   polygons.clear();
   pointNormals.clear();
   pointColors.clear();
   displayFlag = true;
}

/**
 * returns true if the file is isEmpty.
 */

bool 
VtkModelFile::empty() const
{
   return coordinates.empty();
}

/**
 * apply a transformation matrix to the vtk model coordinates.
 */
void 
VtkModelFile::applyTransformationMatrix(const TransformationMatrix& matrix)
{
   coordinates.applyTransformationMatrix(matrix);
   setModified();
}

/**
 * read the file (overridden since file has no header).
 */
void 
VtkModelFile::readFile(const QString& fileNameIn) throw (FileException)
{
   clear();
   
   if (fileNameIn.isEmpty()) {
      throw FileException(fileNameIn, "Filename for reading is isEmpty");   
   }

   filename = fileNameIn;
  
   QTime timer;
   timer.start();

   vtkPolyData* polyData = NULL;
      
   //
   // See which type of VTK file to read
   //
   vtkPolyDataReader* vtkReader = NULL;
   vtkXMLPolyDataReader* xmlReader = NULL;
   if (FileUtilities::filenameExtension(filename) == "vtk") {
      //
      // Read normal VTK file
      //
      vtkReader = vtkPolyDataReader::New();
      vtkReader->SetFileName((char*)filename.toAscii().constData());
      vtkReader->Update();        
      polyData = vtkReader->GetOutput();
   }
   else if (FileUtilities::filenameExtension(filename) == "vtp") {
      //
      // Read XML VTK file
      //
      xmlReader = vtkXMLPolyDataReader::New();
      xmlReader->SetFileName((char*)filename.toAscii().constData());
      xmlReader->Update();
      polyData = xmlReader->GetOutput();
   }
   else {
      QString msg("Unrecognized extension neither of \"vtk\" nor \"vtp\".");
      throw FileException(filename, msg);
   }
   
   if (polyData != NULL) {
      readPolyData(polyData);
      //polyData->Delete();
   }
   
   timeToReadFileInSeconds = static_cast<float>(timer.elapsed()) / 1000.0;
   QFileInfo fi(filename);
   const float fileSize = fi.size() / 1048576.0;
   if (DebugControl::getDebugOn() ||
       DebugControl::getFileReadTimingFlag()) {
      std::cout << "Time to read " << FileUtilities::basename(getFileName()).toAscii().constData()
                << " ("
                << fileSize
                << " MB) was "
                << timeToReadFileInSeconds
                << " seconds." << std::endl;
   }

   //
   // Free up memory
   //
   if (vtkReader != NULL) {
      vtkReader->Delete();
   }
   if (xmlReader != NULL) {
      xmlReader->Delete();
   }
}   

/**
 * read in the model from vtk poly data.
 */
void 
VtkModelFile::readPolyData(vtkPolyData* polyData)
{      
   const int numPoints = polyData->GetNumberOfPoints();
   if (numPoints <= 0) {
      throw FileException(filename, "contains no points.");
   }
   
   //
   // Convert any triangle strips to triangles
   //
   vtkTriangleFilter* triangleFilter = NULL;
   if (polyData->GetNumberOfStrips() > 0) {
      triangleFilter = vtkTriangleFilter::New();
#ifdef HAVE_VTK6
      triangleFilter->SetInputData(polyData);
#else
      triangleFilter->SetInput(polyData);
#endif
      triangleFilter->Update();
      polyData = triangleFilter->GetOutput();
   }

   //
   // Compute normals
   //
   vtkPolyDataNormals* polyNormals = vtkPolyDataNormals::New();
#ifdef HAVE_VTK6
   polyNormals->SetInputData(polyData);
#else
   polyNormals->SetInput(polyData);
#endif
   polyNormals->SplittingOff();
   polyNormals->ConsistencyOn();
   polyNormals->ComputePointNormalsOn();
   polyNormals->NonManifoldTraversalOn();
   polyNormals->Update();
   polyData = polyNormals->GetOutput();
   
   //
   // Read in the points
   //
   coordinates.setNumberOfCoordinates(numPoints);
   vtkPoints* points = polyData->GetPoints();
   for (int i = 0; i < numPoints; i++) {
#ifdef HAVE_VTK5
      double xyz[3];
#else // HAVE_VTK5
      float xyz[3];
#endif // HAVE_VTK5
      points->GetPoint(i, xyz);
      coordinates.setCoordinate(i, xyz);
   }
   coordinates.clearModified();
   
   //
   // Get the point normals
   //
   pointNormals.resize(numPoints * 3);
   vtkPointData* pointData = polyData->GetPointData();
   vtkDataArray* normals = pointData->GetNormals();
   if ((normals->GetNumberOfTuples() == numPoints) &&
       (normals->GetNumberOfComponents() == 3)) {
      for (int i = 0; i < numPoints; i++) {
#ifdef HAVE_VTK5
         const double* xyz = normals->GetTuple3(i);
#else // HAVE_VTK5
         const float* xyz = normals->GetTuple3(i);
#endif // HAVE_VTK5
         pointNormals[i * 3]     = xyz[0];
         pointNormals[i * 3 + 1] = xyz[1];
         pointNormals[i * 3 + 2] = xyz[2];
      }
   }
   else {
      std::cout << "Normals failed for VTK model: " << filename.toAscii().constData() << std::endl;
      for (int i = 0; i < numPoints; i++) {
         pointNormals[i * 3]     = 0.0;
         pointNormals[i * 3 + 1] = 0.0;
         pointNormals[i * 3 + 2] = 1.0;
      }
   }
   
   //
   // Read in the vertices
   //
   const int numVertices = polyData->GetNumberOfVerts();
   if (numVertices > 0) {
      vtkCellArray* verts = polyData->GetVerts();
      vtkIdType npts;
      vtkIdType* pts;
      for (verts->InitTraversal(); verts->GetNextCell(npts, pts); ) {
         for (int i = 0; i < npts; i++) {
            vertices.push_back(pts[i]);
         }
      }
   }
   
   //
   // Read in the lines
   //
   const int numLines = polyData->GetNumberOfLines();
   if (numLines > 0) {
      vtkCellArray* cellLines = polyData->GetLines();
      vtkIdType npts;
      vtkIdType* pts;
      for (cellLines->InitTraversal(); cellLines->GetNextCell(npts, pts); ) {
         if (npts > 0) {
            int npts2 = npts;
            int* pts2 = new int[npts2];
            for (int k = 0; k < npts2; k++) {
               pts2[k] = pts[k];
            }
            delete[] pts2;
            lines.push_back(VtkModelObject(pts2, npts2));
         }
      }
   }
   
   //
   // Read in the polygons (convert triangle strips to triangles if needed)
   //
   vtkCellArray* polys = polyData->GetPolys();
   const int numPolys = polyData->GetNumberOfPolys();
   if (numPolys > 0) {
      vtkIdType npts;
      vtkIdType* pts;
      for (polys->InitTraversal(); polys->GetNextCell(npts,pts); ) {
         if (npts == 3) {
            triangles.push_back(pts[0]);
            triangles.push_back(pts[1]);
            triangles.push_back(pts[2]);
         }
         else if (npts > 3) {
            int* pp = new int[npts*3];
            for (int k = 0; k < npts*3; k++) {
               pp[k] = pts[k];
            }
            polygons.push_back(VtkModelObject(pp, static_cast<int>(npts)));
            delete[] pp;
         }
      }
   }  
   
   //
   // Allocate and Read in the colors
   //
   pointColors.resize(numPoints * 4, 170);
   for (int i = 0; i < numPoints; i++) {
      pointColors[i*4] = 255;
   }
   vtkDataArray* scalars = pointData->GetScalars();
   if (scalars != NULL) {
      if ( (scalars->GetDataType() == VTK_UNSIGNED_CHAR) &&
            (scalars->GetNumberOfComponents() == 3) ) {
         vtkUnsignedCharArray* colors = (vtkUnsignedCharArray*)scalars;
         for (int i = 0; i < numPoints; i++) {
#ifdef HAVE_VTK5
            double rgb[3];
#else // HAVE_VTK5
            float rgb[3];
#endif // HAVE_VTK5
            colors->GetTuple(i, rgb);
            const unsigned char rgbaChar[4] = {
               static_cast<unsigned char>(rgb[0]),
               static_cast<unsigned char>(rgb[1]),
               static_cast<unsigned char>(rgb[2]),
               255
            };
            setPointColor(i, rgbaChar);
         }
      }
      if ( (scalars->GetDataType() == VTK_UNSIGNED_CHAR) &&
            (scalars->GetNumberOfComponents() == 4) ) {
         vtkUnsignedCharArray* colors = (vtkUnsignedCharArray*)scalars;
         for (int i = 0; i < numPoints; i++) {
#ifdef HAVE_VTK5
            double rgba[4] = { 170, 170, 170, 255 };
#else // HAVE_VTK5
            float rgba[4] = { 170, 170, 170, 255 };
#endif // HAVE_VTK5
            colors->GetTuple(i, rgba);
            const unsigned char rgbaChar[4] = {
               static_cast<unsigned char>(rgba[0]),
               static_cast<unsigned char>(rgba[1]),
               static_cast<unsigned char>(rgba[2]),
               static_cast<unsigned char>(rgba[3])
            };
            setPointColor(i, rgbaChar);
         }
      }
      else if ( (scalars->GetDataType() == VTK_FLOAT) &&
                  (scalars->GetNumberOfComponents() == 1) ) {
         float maxValue = -100000.0;
         for (int j = 0; j < numPoints; j++) {
            if (scalars->GetComponent(j, 0) > maxValue) {
               maxValue = scalars->GetComponent(j, 0);
            }
         }

         vtkLookupTable* lookupTable = scalars->GetLookupTable();
         for (int i = 0; i < numPoints; i++) {
            const float value = scalars->GetComponent(i, 0);
            if (lookupTable == NULL) {
               const unsigned char rgbaChar[4] = {
                  static_cast<unsigned char>(value),
                  static_cast<unsigned char>(value),
                  static_cast<unsigned char>(value),
                  255
               };
               setPointColor(i, rgbaChar);
            }
            else {
#ifdef HAVE_VTK5
               double rgb[3] = { 0.0, 0.0, 0.0 };
               lookupTable->GetColor((double)value, rgb);
#else // HAVE_VTK5
               float rgb[3] = { 0.0, 0.0, 0.0 };
               lookupTable->GetColor((float)value, rgb);
#endif // HAVE_VTK5
               const unsigned char rgbaChar[4] = {
                  static_cast<unsigned char>(rgb[0]),
                  static_cast<unsigned char>(rgb[1]),
                  static_cast<unsigned char>(rgb[2]),
                  255
               };
               setPointColor(i, rgbaChar);
            }
         }
      }
   }
   
   polyNormals->Delete();
   clearModified();
   if (triangleFilter != NULL) {
      triangleFilter->Delete();
   }
}

/**
 * get the RGB colors for a point.
 */
const unsigned char* 
VtkModelFile::getPointColor(const int indx) const
{
   if ((indx >= 0) && (indx < getNumberOfPoints())) {
      return &pointColors[indx * 4];
   }

   static unsigned char dummy[4] = { 170, 170, 170, 255 };
   return dummy;
}    
  
/**
 * get the normal vector for a point.
 */
const float*
VtkModelFile::getPointNormal(const int indx) const
{
   if ((indx >= 0) && (indx < getNumberOfPoints())) {
      return &pointNormals[indx * 3];
   }

   static float dummy[3] = { 0.0, 0.0, 1.0 };
   return dummy;
}

/**
 * set the RGB colors for a point.
 */
void
VtkModelFile::setPointColor(const int indx, const unsigned char rgba[4])
{
   if ((indx >= 0) && (indx < getNumberOfPoints())) {
      pointColors[indx * 4]     = rgba[0];
      pointColors[indx * 4 + 1] = rgba[1];
      pointColors[indx * 4 + 2] = rgba[2];
      pointColors[indx * 4 + 3] = rgba[3];
      setModified();
   }
}    
  
/**
 * set the entire model to a specific color.
 */
void 
VtkModelFile::setToSolidColor(const unsigned char rgba[4])
{
   const int num = getNumberOfPoints();
   for (int i = 0; i < num; i++) {
      setPointColor(i, rgba);
   }
}
      
/**
 * get a vertex (pointer to its "point" indices).
 */
const int* 
VtkModelFile::getVertex(const int indx) const
{
   return &vertices[indx];
}

/**
 * get a vertex (pointer to its 3 "point" indices).
 */
const int* 
VtkModelFile::getTriangle(const int indx) const
{
   return &triangles[indx*3];
}

/**
 * get a triangles 3D position (average of its coordinates).
 */
void 
VtkModelFile::getTriangleCoordinate(const int indx, 
                                    float coord[3]) const
{
   if ((indx >= 0) && (indx < getNumberOfTriangles())) {
      const int* tri = getTriangle(indx);
      const CoordinateFile* cf = getCoordinateFile();
      const float* v1 = cf->getCoordinate(tri[0]);
      const float* v2 = cf->getCoordinate(tri[1]);
      const float* v3 = cf->getCoordinate(tri[2]);
      coord[0] = (v1[0] + v2[0] + v3[0]) / 3.0;
      coord[1] = (v1[1] + v2[1] + v3[1]) / 3.0;
      coord[2] = (v1[2] + v2[2] + v3[2]) / 3.0;
   }
   else {
      coord[0] = 0.0;
      coord[1] = 0.0;
      coord[2] = 0.0;
   }
}      

/**
 * get a line.
 */
const VtkModelFile::VtkModelObject* 
VtkModelFile::getLine(const int indx) const
{
   return &lines[indx];
}

/**
 * get a polygon.
 */
const VtkModelFile::VtkModelObject* 
VtkModelFile::getPolygon(const int indx) const
{
   return &polygons[indx];
}

/**
 * write the file.
 */
void 
VtkModelFile::writeFile(const QString& fileNameIn) throw (FileException)
{
   if (fileNameIn.isEmpty()) {
      throw FileException(fileNameIn, "Filename for reading is isEmpty");   
   }
   filename = fileNameIn;

   //
   // Write points
   //
   vtkPoints* pointData = vtkPoints::New();
   const int numCoords = coordinates.getNumberOfCoordinates();
   for (int i = 0; i < numCoords; i++) {
      pointData->InsertPoint(i, coordinates.getCoordinate(i));
   }

   //
   // Write normals
   //
   vtkFloatArray* normalsVTK = vtkFloatArray::New();
   normalsVTK->SetNumberOfTuples(numCoords);
   normalsVTK->SetNumberOfComponents(3);
   for (int i = 0; i < numCoords; i++) {
      normalsVTK->InsertTuple(i, getPointNormal(i));
   }
   
   //
   // Write triangles & polygons
   //
   vtkCellArray* polysVTK = NULL;
   const int numTriangles = getNumberOfTriangles();
   if (numTriangles > 0) {
      if (polysVTK == NULL) {
         polysVTK = vtkCellArray::New();
      }
      //int size = trianglesVTK->EstimateSize(numTriangles, 3);
      //polysVTK->Allocate(size, 25);
      for (int j = 0; j < numTriangles; j++) {
         const int* v = getTriangle(j);
         vtkIdType v2[3] = { v[0], v[1], v[2] };
         polysVTK->InsertNextCell(static_cast<vtkIdType>(3), v2);
      }
   }
   const int numPolys = getNumberOfPolygons();
   if (numPolys > 0) {
      if (polysVTK == NULL) {
         polysVTK = vtkCellArray::New();
      }
      for (int j = 0; j < numPolys; j++) {
         const VtkModelObject* vmo = getPolygon(j);
         vtkIdType num = vmo->getNumberOfItems();
         if (num > 0) {
            vtkIdType* pts = new vtkIdType[num*3];
            for (int k = 0; k < num; k++) {
               const int* pp = vmo->getPointIndex(0);
               for (int m = 0; m < 3; m++) {
                  pts[k*3+m] = pp[m];
               }
            }
            polysVTK->InsertNextCell(num,
                                     pts);
            delete[] pts;
         }
      }
   }
   
   //
   // Write lines
   //
   const int numLines = getNumberOfLines();
   vtkCellArray* linesVTK = NULL;
   if (numLines > 0) {
      linesVTK = vtkCellArray::New();
      //int size = linesVTK->EstimateSize(numLines, 2);
      for (int j = 0; j < numLines; j++) {
         const VtkModelObject* vmo = getLine(j);
         vtkIdType num = vmo->getNumberOfItems();
         if (num > 0) {
            vtkIdType* pts = new vtkIdType[num*3];
            for (int k = 0; k < num; k++) {
               const int* pp = vmo->getPointIndex(0);
               for (int m = 0; m < 3; m++) {
                  pts[k*3+m] = pp[m];
               }
            }
            linesVTK->InsertNextCell(num, pts);
         }
      }
   }
   
   //
   // Write the vertices
   //
   const int numVerts = getNumberOfVertices();
   vtkCellArray* vertsVTK = NULL;
   if (numVerts > 0) {
      vertsVTK = vtkCellArray::New();
      for (int j = 0; j < numVerts; j++) {
         const int* v = getVertex(j);
         vtkIdType v2[3] = { v[0], v[1], v[2] };
         vertsVTK->InsertNextCell(static_cast<vtkIdType>(1), v2);
      }
   }
   
   //
   // Write colors
   //
   vtkUnsignedCharArray* colorsVTK = vtkUnsignedCharArray::New();
   colorsVTK->SetNumberOfComponents(4);
   colorsVTK->SetNumberOfTuples(numCoords);
   for (int i = 0; i < numCoords; i++) {
      const unsigned char* rgba = getPointColor(i);
      float floatRGBA[4] = { rgba[0], rgba[1], rgba[2], rgba[3] };
      colorsVTK->InsertTuple(i, floatRGBA);
   }
   
   //
   // Create the polydata
   //
   vtkPolyData* polyData = vtkPolyData::New();
   polyData->SetPoints(pointData);
   pointData->Delete();
   polyData->GetPointData()->SetScalars(colorsVTK);
   colorsVTK->Delete();
   polyData->GetPointData()->SetNormals(normalsVTK);
   normalsVTK->Delete();
   if (vertsVTK != NULL) {
      polyData->SetVerts(vertsVTK);
      vertsVTK->Delete();
   }
   if (linesVTK != NULL) {
      polyData->SetLines(linesVTK);
      linesVTK->Delete();
   }
   if (polysVTK != NULL) {
      polyData->SetPolys(polysVTK);
      polysVTK->Delete();
   }

   if (FileUtilities::filenameExtension(filename) == "vtp") {
      vtkXMLPolyDataWriter *writer = vtkXMLPolyDataWriter::New();
#ifdef HAVE_VTK6
      writer->SetInputData(polyData);
#else
      writer->SetInput(polyData);
#endif
      //writer->SetHeader("Written by Caret");
      writer->SetFileName((char*)filename.toAscii().constData());
      writer->Write();

      writer->Delete();
   }
   else {
      vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
#ifdef HAVE_VTK6
      writer->SetInputData(polyData);
#else
      writer->SetInput(polyData);
#endif
      writer->SetHeader("Written by Caret");
      writer->SetFileName((char*)filename.toAscii().constData());
      writer->Write();

      writer->Delete();
   }
   polyData->Delete();
   
   clearModified();
}

/**
 * Read the vtk model file data (should never be called).
 */
void 
VtkModelFile::readFileData(QFile& /*file*/, QTextStream& /*stream*/, QDataStream&,
                                  QDomElement& /* rootElement */) throw (FileException)
{
   throw FileException(filename, "Program Error: VtkModelFile::readFileData should never be called.");
}

/**
 * Write the vtk model file data (should never be called).
 */
void 
VtkModelFile::writeFileData(QTextStream& /*stream*/, QDataStream&,
                                 QDomDocument& /* xmlDoc */,
                                  QDomElement& /* rootElement */) throw (FileException)
{
   throw FileException(filename, "Program Error: VtkModelFile::writeFileData should never be called.");
}
      
