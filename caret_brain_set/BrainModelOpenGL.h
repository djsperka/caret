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

#ifndef __BRAIN_MODEL_OPENGL_H__
#define __BRAIN_MODEL_OPENGL_H__

#include <vector>

#include <QGLWidget>  // includes OpenGL includes ;)
#include <QMutex>

#ifdef CARET_OS_WINDOWS
#include <Windows.h>
#endif
#ifdef CARET_OS_MACOSX
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "BorderFile.h"
#include "BrainModel.h"
#include "BrainModelOpenGLSelectedItem.h"
#include "ColorFile.h"
#include "VolumeFile.h"

class BrainModelContours;
class BrainModelSurface;
class BrainModelSurfaceAndVolume;
class BrainModelSurfaceNodeColoring;
class BrainModelVolume;
class BrainSet;
class CellFile;
class CellProjectionFile;
class ContourFile;
class CoordinateFile;
class DisplaySettingsCells;
class QImage;
class TopologyFile;
class VtkModelFile;

/// This class is used to draw BrainModels using OpenGL
class BrainModelOpenGL {
   public:
      /// bitmasks for selecting objects
      enum /* SELECTION_MASK */ {
          SELECTION_MASK_OFF                        =    0,
          SELECTION_MASK_NODE                       =    1,   
          SELECTION_MASK_BORDER                     =    2,
          SELECTION_MASK_CELL_PROJECTION            =    4,
          SELECTION_MASK_FOCUS_PROJECTION           =    8,
          SELECTION_MASK_PALETTE_METRIC            =   16,
          SELECTION_MASK_PALETTE_SHAPE             =   32,
          SELECTION_MASK_CUT                        =   64,
          SELECTION_MASK_CONTOUR                    =   128,
          SELECTION_MASK_CONTOUR_CELL               =  256,   // 2^8
          SELECTION_MASK_VOXEL_UNDERLAY             =  512,
          SELECTION_MASK_VOXEL_OVERLAY_SECONDARY    =  1024,
          SELECTION_MASK_VOXEL_OVERLAY_PRIMARY      = 2048,
          SELECTION_MASK_VOXEL_FUNCTIONAL_CLOUD     = 4096,
          SELECTION_MASK_TILE                       = 8192,
          SELECTION_MASK_LINK                       = 16384,
          SELECTION_MASK_VOLUME_BORDER              = 32768,
          SELECTION_MASK_VOLUME_CELL                = 65536,  // 2^16
          SELECTION_MASK_VOLUME_FOCI                = 131072,
          SELECTION_MASK_TRANSFORMATION_MATRIX_AXES = 262144,
          SELECTION_MASK_VTK_MODEL                  = 524288,
          SELECTION_MASK_TRANSFORM_CELL             = 1048576,
          SELECTION_MASK_TRANSFORM_FOCI             = 2097152,
          SELECTION_MASK_TRANSFORM_CONTOUR_CELL     = 4194304,
          SELECTION_MASK_TRANSFORM_CONTOUR          = 8388608, // 2^23
          SELECTION_MASK_ALL                        = 0xffffffff  // ALL bitmasks ON
      };
      
      /// Constructor
      BrainModelOpenGL();
      
      /// Destructor
      ~BrainModelOpenGL();
      
      /// Draw a brain model for WebCaret
      void drawBrainModelWebCaret(BrainSet* bs,
                                  BrainModel* bm,
                                  const int viewingWindowNumberIn,
                                  const int viewportIn[4]);
      
      /// Draw a brain model
      void drawBrainModel(BrainSet* bs,
                          BrainModel* bm,
                          const int viewingWindowNumberIn,
                          const int viewportIn[4],
                          QGLWidget* glWidgetIn);
      
      /// select something in the brain model for WebCaret
      void selectBrainModelItemWebCaret(BrainSet* bs,
                                        BrainModel* bm,
                                        const int viewportIn[4],
                                        const unsigned long selectionMaskIn,
                                        const int selectionXIn, 
                                        const int selectionYIn);
      
      /// Identify something in the brain model for WebCaret
      QString identifyBrainModelItemWebCaret(BrainSet* bs,
                                        BrainModel* bm,
                                        const int viewingWindowNumberIn,
                                        const int viewportIn[4],
                                        const unsigned long selectionMaskIn,
                                        const int selectionXIn, 
                                        const int selectionYIn,
                                        const bool enableHtml,
                                        const bool enableVocabularyLinks);
      
      /// select something in the brain model           
      void selectBrainModelItem(BrainSet* bs,
                                BrainModel* bm,
                                const int viewingWindowNumberIn,
                                const int viewportIn[4],
                                QGLWidget* glWidgetIn,
                                const unsigned long selectionMaskIn,
                                const int selectionXIn, 
                                const int selectionYIn,
                                const bool viewModeFlag);

      /// identify items in the brain model (returns text for ID display)           
      QString identifyBrainModelItem(BrainSet* bs,
                                     BrainModel* bm,
                                     BrainModel* allWindowBrainModelsForIdentificationIn[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS],
                                     const int viewingWindowNumberIn,
                                     const int viewportIn[4],
                                     QGLWidget* glWidgetIn,
                                     const unsigned long selectionMaskIn,
                                     const int selectionXIn, 
                                     const int selectionYIn,
                                     const bool viewModeFlag,
                                     const bool enableHtml,
                                     const bool enableVocabularyLinks);

      
      /// all brain models displayed in windows for identification
      /// ONLY VALID WHEN ::identifyBrainModel... called
      BrainModel* getBrainModelInWindowNumberForIdentification(const int windowNumber);
     
       /// Draw all fiducial surface brain models
      void drawAllFiducialSurfaceBrainModels(std::vector<BrainSet*> brainSets,
                                             const int viewingWindowNumberIn,
                                             const int viewportIn[4],
                                             QGLWidget* glWidgetIn);
      
      /// Get a 3D point on a surface at a screen X/Y.
      bool getSurfacePointAtDisplayXY(BrainSet* bs,
                                    BrainModelSurface* bms,
                                    const int viewingWindowNumberIn,
                                    const int viewportIn[4],
                                    const int displayX, 
                                    const int displayY,
                                    float pointOut[3]);
      
      /// initialize OpenGL (should only call one time)
      void initializeOpenGL(const bool offScreenRenderingFlagIn);
      
      // set the image subregion box/display
      void setImageSubRegion(const int box[4], const bool showFlag);
      
      /// get the editing segmentation volume flag
      bool getEditingSegmentationVolumeFlag() const { return editingSegmentationVolumeFlag; }
      
      /// set the editing segmentation volume flag
      void setEditingSegmentationVolumeFlag(const bool b) { editingSegmentationVolumeFlag = b; }

      /// get the editing paint volume flag
      bool getEditingPaintVolumeFlag() const { return editingPaintVolumeFlag; }
      
      /// set the editing paint volume flag
      void setEditingPaintVolumeFlag(const bool b) { editingPaintVolumeFlag = b; }

      /// get the orthographic size
      void getOrtho(float& orthoRight, float& orthoTop) const;
      
      /// Update the orthographic window size.
      void updateOrthoSize(const int windowNumber, const int width, const int height);
   
      /// get caption displayed in main window
      QString getMainWindowCaption() const { return mainWindowCaption; }

      /// set caption displayed in main window
      void setMainWindowCaption(const QString& s) { mainWindowCaption = s; }

      /// selected node
      BrainModelOpenGLSelectedItem getSelectedNode() const { return selectedNode; }
       
      /// selected border (first)
      BrainModelOpenGLSelectedItem getSelectedBorder1() const { return selectedBorder1; }
       
      /// selected border (second)
      BrainModelOpenGLSelectedItem getSelectedBorder2() const { return selectedBorder2; }
       
      /// selected border (volume)
      BrainModelOpenGLSelectedItem getSelectedVolumeBorder() const { return selectedVolumeBorder; }
       
      /// selected cell projection
      BrainModelOpenGLSelectedItem getSelectedCellProjection() const { return selectedCellProjection; }
       
      /// selected cell (volume)
      BrainModelOpenGLSelectedItem getSelectedVolumeCell() const { return selectedVolumeCell; }
       
      /// selected cut
      BrainModelOpenGLSelectedItem getSelectedCut() const { return selectedCut; }
       
      /// selected foci projection
      BrainModelOpenGLSelectedItem getSelectedFocusProjection() const { return selectedFocusProjection; }
       
      /// selected foci (volume)
      BrainModelOpenGLSelectedItem getSelectedVolumeFoci() const { return selectedVolumeFoci; }
       
      /// selected palette for metric
      BrainModelOpenGLSelectedItem getSelectedPaletteMetric() const { return selectedPaletteMetric; }
       
      /// selected palette for shape
      BrainModelOpenGLSelectedItem getSelectedPaletteShape() const { return selectedPaletteShape; }
       
      /// selected contour
      BrainModelOpenGLSelectedItem getSelectedContour() const { return selectedContour; }
       
      /// selected contour cell
      BrainModelOpenGLSelectedItem getSelectedContourCell() const { return selectedContourCell; }
       
      /// selected voxel underlay
      BrainModelOpenGLSelectedItem getSelectedVoxelUnderlay() const { return selectedVoxelUnderlay; }
       
      /// selected voxel overlay secondary
      BrainModelOpenGLSelectedItem getSelectedVoxelOverlaySecondary() const { return selectedVoxelOverlaySecondary; }
       
      /// selected voxel overlay primary
      BrainModelOpenGLSelectedItem getSelectedVoxelOverlayPrimary() const { return selectedVoxelOverlayPrimary; }
       
      /// selected functional voxel (surface and volume cloud)
      BrainModelOpenGLSelectedItem getSelectedVoxelFunctionalCloud() const { return selectedVoxelFunctionalCloud; }
       
      /// selected surface tile
      BrainModelOpenGLSelectedItem getSelectedSurfaceTile() const { return selectedSurfaceTile; }
       
      /// selected link (index 1 & 2 are nodes)
      BrainModelOpenGLSelectedItem getSelectedLink() const { return selectedLink; }
      
      /// selected transformation axes
      BrainModelOpenGLSelectedItem getSelectedTransformationMatrixAxes() const { return selectedTransformationMatrixAxes; }
      
      /// selected transformation cell
      BrainModelOpenGLSelectedItem getSelectedTransformationCell() const { return selectedTransformCell; }
      
      /// selected transformation foci
      BrainModelOpenGLSelectedItem getSelectedTransformationFoci() const { return selectedTransformFoci; }
      
      /// selected transformation contour
      BrainModelOpenGLSelectedItem getSelectedTransformationContour() const { return selectedTransformContour; }
      
      /// selected transformation contour cell
      BrainModelOpenGLSelectedItem getSelectedTransformationContourCell() const { return selectedTransformContourCell; }
      
      /// selected vtk model
      BrainModelOpenGLSelectedItem getSelectedVtkModel() const { return selectedVtkModel; }
      
      /// get the selection modelview matrix
      GLdouble* getSelectionModelviewMatrix(const int viewWindowNumber)
                        { return selectionModelviewMatrix[viewWindowNumber]; }
      
      /// get the selection projection matrix
      GLdouble* getSelectionProjectionMatrix(const int viewWindowNumber)
                        { return selectionProjectionMatrix[viewWindowNumber]; }
      
      /// get the selection viewport
      GLint* getSelectionViewport(const int viewWindowNumber) 
                        { return selectionViewport[viewWindowNumber]; }
         
      /// Get the dimensions of the surface orthographic projection
      void getOrthographicBox(const int modelViewNumber,
                              double& orthoLeft,   double& orthoRight,
                              double& orthoBottom, double& orthoTop,
                              double& orthoNear,   double& orthoFar) const;
   
      /// set the linear object for drawing borders, cuts, etc.
      void setLinearObject(const Border& lo) { linearObjectBeingDrawn = lo; }
      
      /// set the linear object valid
      void setDrawLinearObjectOnly(const bool loo) { drawLinearObjectOnly = loo; }
      
      /// set node special highlighting
      void setNodeSpecialHighlighting(const std::vector<int>& nsh)
                                      { nodeSpecialHighlighting = nsh; }
      
      /// set OpenGL text enabled
      static void setOpenGLTextEnabled(const bool b) { openGLTextEnabledFlag = b; }
      
      /// get the default ortho right and top for command line scene generation
      static void getDefaultOrthoRightAndTop(const int windowWidthIn,
                                             const int windowHeightIn,
                                             double& orthoRightOut,
                                             double& orthoTopOut);

      /// get minimum/maximum point size
      static void getMinMaxPointSize(float& minSizeOut, float& maxSizeOut);

      /// get minimum/maximum line width
      static void getMinMaxLineWidth(float& minWidthOut, float& maxWidthOut);
                                             
   protected:
      /// Draw a brain model
      void drawBrainModelPrivate(BrainModel* bm,
                          const int viewingWindowNumberIn,
                          const int viewportIn[4],
                          QGLWidget* glWidgetIn);
      
      /// draw contours
      void drawBrainModelContours(BrainModelContours* bmc);
      
      /// draw surface
      void drawBrainModelSurface(BrainModelSurface* bms,
                                 BrainModelSurfaceAndVolume* bmsv,
                                 const bool drawTheSurface,
                                 const bool surfaceInVolumeAllViewFlag);
      
      /// draw surface and volume
      void drawBrainModelSurfaceAndVolume(BrainModelSurfaceAndVolume* bmsv);
      
      /// draw volume
      void drawBrainModelVolume(BrainModelVolume* bmv);
      
      /// draw volume in montage
      void drawBrainModelVolumeMontage(BrainModelVolume* bmv);
      
      /// draw volume with all axis
      void drawBrainModelVolumeAllAxis(BrainModelVolume* bmv);
      
      /// draw volume with oblique axis montage
      void drawBrainModelVolumeObliqueAxisMontage(BrainModelVolume* bmv,
                                                  const bool selectFlag);
      
      /// draw volume with oblique axis
      void drawBrainModelVolumeObliqueAxis(BrainModelVolume* bmv);
      
      /// draw volume with oblique ALL axis
      void drawBrainModelVolumeObliqueAllAxis(BrainModelVolume* bmv);
      
      /// draw volume with oblique axis slice
      void drawBrainModelVolumeObliqueAxisSlice(BrainModelVolume* bmv,
                                                VolumeFile::VOLUME_AXIS axis,
                                                const int montageOffset,
                                                const bool selectFlag);
                                                  
      /// Draw volume crosshairs
      void drawVolumeCrosshairs(BrainModelVolume* bmv,
                                const VolumeFile* vf,
                                const VolumeFile::VOLUME_AXIS volumeSliceAxis);
      
      /// Draw volume crosshair coordinates
      void drawVolumeCrosshairCoordinates(BrainModelVolume* bmv,
                                          const VolumeFile* vf,
                                          const int viewportHeight);
      
      /// Draw the volume cropping lines
      void drawVolumeCroppingLines(BrainModelVolume* bmv,
                                             VolumeFile* vf,
                                             VolumeFile::VOLUME_AXIS volumeSliceAxis);
      
      /// stuff for web caret
      void webCaretCommon(BrainSet* bs,
                          const int viewport[4]);
                          
      /// Create a sphere quadric and put it in a display list.
      void createSphereQuadricAndDisplayList();
      
      /// Create a disk quadric and put it in a display list.
      void createDiskQuadricAndDisplayList();
      
      /// Create a cone quadic and put it in a display list
      void createConeQuadricAndDisplayList();
      
      /// Create a cylinder quadric and put it in a display list
      void createCylinderQuadricAndDisplayList();
      
      /// Create a square and put it in a display list
      void createSquareDisplayList();
      
      /// Create a box and put it in a display list
      void createBoxDisplayList();
      
      /// Create a ring and put it in a display list
      void createRingQuadricAndDisplayList();
      
      /// Create a diamond and put it in a display list
      void createDiamondQuadricAndDisplayList();
      
      /// Execute OpenGL commands to draw a sphere.
      void drawingCommandsSphere();
      
      /// Execute OpenGL commands to draw a disk.
      void drawingCommandsDisk();
      
      /// Execute OpenGL commands to draw a cone
      void drawingCommandsCone();
      
      /// Execute OpenGL commands to draw a cylinder
      void drawingCommandsCylinder();
      
      /// Execute OpenGL commands to draw a box
      void drawingCommandsBox();
      
      /// Execute OpenGL commands to draw a ring
      void drawingCommandsRing();
      
      /// Execute OpenGL commands to draw a diamond
      void drawingCommandsDiamond();
      
      /// Execute OpenGL commands to draw a square
      void drawingCommandsSquare();
      
      /// Draw the surface as points.
      void drawSurfaceNodes(const BrainModelSurfaceNodeColoring* bs,
                            const int modelNumber,
                            const CoordinateFile* cf, 
                            const int numCoords,
                            const bool drawInSurfaceEditColor);
      
      /// Draw the surface as links.
      void drawSurfaceLinks(const BrainModelSurfaceNodeColoring* bs,
                            const int modelNumber,
                            const CoordinateFile* cf,
                            const TopologyFile* tf, const int numTiles,
                            const bool drawEdgesOnly,
                            const bool drawInSurfaceEditColor);
                                   
      /// Draw the surface as links with hidden line removal.
      void drawSurfaceLinksNoBackside(const BrainModelSurfaceNodeColoring* bs,
                                      const int modelNumber,
                                      const CoordinateFile* cf,
                                      const TopologyFile* tf, const int numTiles);
                                      
      /// Draw the surface as tiles, possibly with lighting.
      void drawSurfaceTiles(const BrainModelSurfaceNodeColoring* bs,
                                   const BrainModelSurface* s,
                                   const CoordinateFile* cf,
                                   TopologyFile* tf, const int numTiles,
                                   const int numCoords);
                                   
      /// Draw the surface normals
      void drawSurfaceNormals(const BrainModelSurface* bms,
                              const CoordinateFile* cf,
                              const int numCoords);
                              
      /// Draw the surface forces
      void drawSurfaceForces(const CoordinateFile* cf,
                             const int numCoords);
                                    
      /// draw a sphere
      void drawSphere(const float diameter);
      
      /// draw a 2D disk (filled circle)
      void drawDisk(const float diameter);
      
      /// draw a cone
      void drawCone();
      
      /// draw cylinder
      void drawCylinder();
      
      /// draw a box
      void drawBox();
      
      /// draw a ring
      void drawRing();
      
      /// draw a square
      void drawSquare();
      
      /// draw a diamond
      void drawDiamond();
      
      /// draw a symbol
      void drawSymbol(const ColorFile::ColorStorage::SYMBOL symbol,
                      const float x,
                      const float y,
                      const float z,
                      const float size,
                      const BrainModel* bm);
                      
      /// Draw node highlighting.
      void drawNodeHighlighting(const BrainModelSurface* bms, const int numCoords);
      
      /// Draw Surface ROI members.
      void drawSurfaceROIMembers(const BrainModelSurface* bms, const int numCoords);
      
      /// Draw a transformation cell or foci file
      void drawTransformationCellOrFociFile(BrainModel* bm,
                              CellFile* cellFile, 
                              ColorFile* colorFile,
                              const DisplaySettingsCells* dsc,
                              const int transformFileNumber,
                              const int selectionMask);
      
      // Draw a cell or foci projection file
      void drawCellOrFociProjectionFile(BrainModelSurface* bms,
                                        CellProjectionFile* cellProjectionFile, 
                                        ColorFile* colorFile,
                                        const DisplaySettingsCells* dsc,
                                        const bool fociFileFlag);

      /// Draw cell projection and foci projections
      void drawCellAndFociProjections(BrainModelSurface* s);
      
      /// draw the geodesic path.
      void drawGeodesicPath(const CoordinateFile* cf);
      
      /// Draw a surface outline and transform axes over the volume.
      void drawVolumeSurfaceOutlineAndTransformationMatrixAxes(
                                    const BrainModelVolume* bmv,
                                    const VolumeFile::VOLUME_AXIS axis,
                                    const float axisCoord);
                                    
      /// Draw a cuts file.
      void drawCuts();
      
      /// Draw electrode trajectory
      void drawElectrodeTrajectory(BrainModelSurface* s);

      /// Draw electrode trajectory and nodes on volume
		void drawVolumeElectrodeTrajectory(BrainModelVolume* bmv, const VolumeFile::VOLUME_AXIS volumeSliceAxis, const float axisCoord);

      /// Draw the borders.
      void drawBorders(BrainModelSurface* s);
      
      /// draw main window caption
      void drawMainWindowCaption();
      
      /// Draw the Surface Shape palette colorbar.
      void drawShapePalette(const int modelNumber);

      /// Draw the metric's palette colorbar.
      void drawMetricPalette(const int modelNumber,
                             const bool surfaceFlag);
                             
      /// Display a QImage in the OpenGL window.
      void displayAnImage(QImage* image);

      /// Display background and splash images
      void displayImages();
      
      /// Draw the linear object (typically used while drawing borders)
      void drawLinearObject();
      
      /// Draw transformation data files
      void drawTransformationDataFiles(const TransformationMatrix* tm);
      
      /// Draw all the VTK models.
      void drawAllVtkModels();

      /// Draw a VTK model file
      void drawVtkModelFile(VtkModelFile* vmf, const int modelNum);
      
      /// Draw the transformation axes
      void drawTransformationMatrixAxes(const BrainModel* bm);
      
      /// Draw the surface axes
      void drawSurfaceAxes(const BrainModelSurface* bms);
      
      /// Draw the deformation field vectors.
      void drawDeformationFieldVectors(BrainModelSurface* bms);

      /// Draw the vectors in 3d
      void drawVectorFile3D(BrainModelSurface* bms);

      /// check a vectors orientation (true if orientation is valid for display)
      bool checkVectorOrientation(const float vector[3]);

      /// Draw an arrow symbol
      void drawArrowSymbol(const float xyz[3],
                           const float tipXYZ[3],
                           const float radius);

      /// Draw a cylinder symbol
      void drawCylinderSymbol(const float xyz[3],
                              const float tipXYZ[3],
                              const float radius);

      /// Draw the volume foci file.
      void drawVectorsOnVolume(const VolumeFile::VOLUME_AXIS axis,
                               const float axisCoord,
                               const float voxelSize);

      /// Convert from volume item XYZ to screen XYZ
      void convertVolumeItemXYZToScreenXY(const VolumeFile::VOLUME_AXIS axis,
                                             float xyz[3]);
      
      /// Draw the volume border file
      void drawVolumeBorderFile(const VolumeFile::VOLUME_AXIS axis,
                                const float axisCoord,
                                const float voxelSize);
      
      /// Draw the volume contour file
      void drawVolumeContourFile(const VolumeFile::VOLUME_AXIS axis,
                                 const float axisCoord,
                                 const float voxelSize);
                                        
      /// Draw the volume cell file
      void drawVolumeCellFile(const VolumeFile::VOLUME_AXIS axis,
                              const float axisCoord,
                              const float voxelSize);
      
      /// Draw the volume foci file
      void drawVolumeFociFile(const VolumeFile::VOLUME_AXIS axis,
                              const float axisCoord,
                              const float voxelSize);
      
      /// Draw the oblique volume cell file
      void drawObliqueVolumeCellFile(const VolumeFile::VOLUME_AXIS axis,
                                     const CellFile* cf,
                                     const DisplaySettingsCells* dsc,
                                     const ColorFile* colorFile,
                                     const float voxelSize,
                                     const TransformationMatrix* tm,
                                     const float sliceCornerCoords[4][3],
                                     const int transformDataFileIndex = -1);
                                           
      /// Draw the oblique volume foci file
      void drawObliqueVolumeFociFile(const VolumeFile::VOLUME_AXIS axis,
                                     const float voxelSize,
                                     const TransformationMatrix* tm,
                                     const float sliceCornerCoords[4][3]);
                                           
      /// Draw the oblique volume contour file.
      void drawObliqueContourFile(const VolumeFile::VOLUME_AXIS axis,
                                  const ContourFile* cf,
                                  const float voxelSize,
                                  const TransformationMatrix* tm,
                                  const float sliceCornerCoords[4][3]);
                                  
      /// Draw the volume identify symbols
      void drawVolumeIdentifySymbols(const VolumeFile::VOLUME_AXIS axis,
                                     const float axisCoord);
                                            
      /// Draw a volume file slice
      void drawVolumeFileSlice(VolumeFile* vf, const VolumeFile::VOLUME_AXIS axis,
                          const int currentSlice,
                          const unsigned long maskForThisUnderlayOrOverlay,
                          const bool overlayFlag);
                                 
      /// Draw the contours that are being aligned.
      void drawModelContoursAlignment(BrainModelContours* bmc,
                                      const int alignmentSectionNumber);
      
      /// process hits made while selecting objects with mouse
      void processSelectedItems(const int numItems);
       
      /// Draw a volume slice
      void drawVolumeSliceOverlayAndUnderlay(BrainModelVolume* bmv,
                           const VolumeFile::VOLUME_AXIS volumeSliceAxis, 
                           const int currentSlice,
                           VolumeFile* &firstVolume);

      /// check for an OpenGL Error
      void checkForOpenGLError(const BrainModel* bm, const QString& msg = "");
      
      /// get valid line width
      GLfloat getValidLineWidth(const GLfloat widthIn) const;
      
      /// get valid point size
      GLfloat getValidPointSize(const GLfloat pointSizeIn) const;
      
      // enable the surface clipping planes
      void enableSurfaceClippingPlanes(BrainModelSurface* bms);

      // disable the surface clipping planes
      void disableSurfaceClippingPlanes();

      /// linear (border, cut, contour) being drawn
      Border linearObjectBeingDrawn;
       
      /// resampling density of linear object being drawn
      float linearObjectBeingDrawnSampling;
       
      /// linear object augment border count
      int linearObjectAugmentBorderCount;
       
      /// draw linear object flag
      bool drawLinearObjectOnly;
       
      /// draw image sub region box
      bool drawImageSubRegionBoxFlag;
      
      /// image sub region box
      int imageSubRegionBox[4];
      
      /// quadric containing a sphere
      GLUquadricObj* sphereQuadric;
      
      /// quadric containing a 2D disk (filled circle)
      GLUquadricObj* diskQuadric;
      
      /// quadric containing a cone
      GLUquadricObj* coneQuadric;
      
      /// quadric containing a cylinder
      GLUquadricObj* cylinderQuadric;
      
      /// quadric containing a diamond
      GLUquadricObj* diamondQuadric;
      
      /// quadric containing a ring
      GLUquadricObj* ringQuadric;
      
      /// display list containing a sphere
      GLuint sphereDisplayList;

      /// display list containing a 2D disk (filled circle)
      GLuint diskDisplayList;

      /// display list containing a cone
      GLuint coneDisplayList;
      
      /// display list for a cylinder
      GLuint cylinderDisplayList;
      
      /// display list for a square
      GLuint squareDisplayList;
      
      /// display list for a box
      GLuint boxDisplayList;
      
      /// display list for a diamond
      GLuint diamondDisplayList;
      
      /// display list for a ring
      GLuint ringDisplayList;
      
      /// version number of OpenGL
      static float versionOfOpenGL;
      
      /// the brain set
      BrainSet* brainSet;
      
      /// the selection mask
      unsigned long selectionMask;
      
      /// node special highlighting (used when adding tiles with mouse)
      std::vector<int> nodeSpecialHighlighting;
      
      /// the main window flag
      bool mainWindowFlag;
      
      /// the viewport
      int viewport[4];
      
      /// QT OpenGL widget being drawn into (may be NULL)
      QGLWidget* glWidget;
      
      /// selection x coordinate
      int selectionX;
      
      /// selection y coordinate
      int selectionY;

      /// viewing window number (0 = main window)
      int viewingWindowNumber;

      /// default orthographic window size
      static float defaultOrthoWindowSize;
       
      /// orthographic projection box for model
      double orthographicLeft[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
      double orthographicRight[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
      double orthographicBottom[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
      double orthographicTop[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
      double orthographicFar[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
      double orthographicNear[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
       
      /// Modelview matrix created when display last drawn.
      /// Used for projecting items selected with the mouse.
      GLdouble selectionModelviewMatrix[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS][16];
      
      /// Projection matrix created when last resize event received.
      /// Used for projecting items selected with the mouse.
      GLdouble selectionProjectionMatrix[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS][16];
      
      /// Viewport set when last resize event received.
      /// Used for projecting items selected with the mouse.
      GLint selectionViewport[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS][4];
      
      /// Mutex to allow only one model to be drawn at a time
      QMutex paintMutex;
       
      /// volume slice axis
      //VolumeFile::VOLUME_AXIS volumeSliceAxis;

      /// volume slice coordinate
      float volumeSliceCoordinate;
       
      /// editing segmentation volume flag
      bool editingSegmentationVolumeFlag;
       
      /// editing paint volume flag
      bool editingPaintVolumeFlag;
      
      /// selection buffer size
      enum { SELECTION_BUFFER_SIZE = 5000};
       
      /// selection buffer
      GLuint selectionBuffer[SELECTION_BUFFER_SIZE];
       
      /// selected node
      BrainModelOpenGLSelectedItem selectedNode;
       
      /// selected border (first)
      BrainModelOpenGLSelectedItem selectedBorder1;
       
      /// selected border (second)
      BrainModelOpenGLSelectedItem selectedBorder2;
       
      /// selected border (volume)
      BrainModelOpenGLSelectedItem selectedVolumeBorder;
       
      /// selected cell projection
      BrainModelOpenGLSelectedItem selectedCellProjection;
       
      /// selected cell (volume)
      BrainModelOpenGLSelectedItem selectedVolumeCell;
       
      /// selected cut
      BrainModelOpenGLSelectedItem selectedCut;
       
      /// selected foci projection
      BrainModelOpenGLSelectedItem selectedFocusProjection;
       
      /// selected foci (volume)
      BrainModelOpenGLSelectedItem selectedVolumeFoci;
       
      /// selected palette for metric
      BrainModelOpenGLSelectedItem selectedPaletteMetric;
       
      /// selected palette for shape
      BrainModelOpenGLSelectedItem selectedPaletteShape;
       
      /// selected contour
      BrainModelOpenGLSelectedItem selectedContour;
       
      /// selected contour cell
      BrainModelOpenGLSelectedItem selectedContourCell;
       
      /// selected voxel underlay
      BrainModelOpenGLSelectedItem selectedVoxelUnderlay;
       
      /// selected voxel overlay secondary
      BrainModelOpenGLSelectedItem selectedVoxelOverlaySecondary;
       
      /// selected voxel overlay primary
      BrainModelOpenGLSelectedItem selectedVoxelOverlayPrimary;
       
      /// selected functional voxel (surface and volume cloud)
      BrainModelOpenGLSelectedItem selectedVoxelFunctionalCloud;
       
      /// selected surface tile
      BrainModelOpenGLSelectedItem selectedSurfaceTile;
       
      /// selected link (index 1 & 2 are nodes)
      BrainModelOpenGLSelectedItem selectedLink;
      
      /// selected transformation axes
      BrainModelOpenGLSelectedItem selectedTransformationMatrixAxes;
      
      /// selected vtk model
      BrainModelOpenGLSelectedItem selectedVtkModel;
      
      /// selected transform cell
      BrainModelOpenGLSelectedItem selectedTransformCell;
      
      /// selected transform foci
      BrainModelOpenGLSelectedItem selectedTransformFoci;
      
      /// selected transform contour
      BrainModelOpenGLSelectedItem selectedTransformContour;
      
      /// selected transform contour cell
      BrainModelOpenGLSelectedItem selectedTransformContourCell;
      
      /// the brain model being drawn
      BrainModel* brainModel;
      
      /// all brain models displayed in windows for identification
      /// ONLY VALID WHEN ::identifyBrainModel... called
      BrainModel* allWindowBrainModelsForIdentification[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];

      /// disable clearing flag (used my draw all fiducial mode)
      bool disableClearingFlag;
       
      /// initialization completed flag
      bool initializationCompletedFlag;
      
      /// offScreen rendering (WebCaret, caret_command)
      bool offScreenRenderingFlag;
      
      /// use display lists for shapes (spheres, boxes, rings, etc)
      bool useDisplayListsForShapes;
      
      /// surface edit drawing color
      static unsigned char surfaceEditDrawColor[3];
      
      /// stippling for polygon
      static GLubyte polygonStipple[128];
      
      /// OpenGL text enabled
      static bool openGLTextEnabledFlag;
      
      /// maximum point size
      float maximumPointSize;
      
      /// minimum point size
      float minimumPointSize;
      
      /// maximum line width
      float maximumLineWidth;
      
      /// minimum line width
      float minimumLineWidth;
      
      /// caption displayed in main window
      QString mainWindowCaption;
};

#ifdef __BRAIN_MODEL_OPENGL_MAIN__
float BrainModelOpenGL::versionOfOpenGL = 0.0;
float BrainModelOpenGL::defaultOrthoWindowSize = 125.0;
unsigned char BrainModelOpenGL::surfaceEditDrawColor[3] = { 0, 0, 255 };
GLubyte BrainModelOpenGL::polygonStipple[128];
bool BrainModelOpenGL::openGLTextEnabledFlag = true;

#endif // __BRAIN_MODEL_OPENGL_MAIN__

#endif // __BRAIN_MODEL_OPENGL_H__

