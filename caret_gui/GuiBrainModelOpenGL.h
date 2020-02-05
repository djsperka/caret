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

#ifndef __VE_GUI_BRAIN_SURFACE_OPENGL_H__
#define __VE_GUI_BRAIN_SURFACE_OPENGL_H__

#include <QImage>
#include <QGLWidget>

#include "BrainModelOpenGL.h"
#include "BrainModelVolume.h"

class BrainModel;
class BrainModelContours;
class BrainModelSurface;
class BrainModelSurfaceAndVolume;
class BrainModelSurfaceNodeColoring;
class BrainModelVolume;
class CellFile;
class ColorFile;
class CoordinateFile;
class DisplaySettingsCells;
class GuiBrainModelOpenGLPopupMenu;
class GuiBrainSetAndModelSelectionControl;
class TopologyFile;
class BrainModelSelectedItem;			// djs
class GuiBrainModelOpenGLKeyEvent;
class GuiBrainModelOpenGLMouseEvent;

/// OpenGL widget for surface display
 class GuiBrainModelOpenGL : public QGLWidget {
    
    Q_OBJECT
    
    public:
       
       /// Rotation axis enums.
       enum BRAIN_MODEL_ROTATION_AXIS {
          BRAIN_MODEL_ROTATION_AXIS_X,
          BRAIN_MODEL_ROTATION_AXIS_Y,
          BRAIN_MODEL_ROTATION_AXIS_Z,
          BRAIN_MODEL_ROTATION_AXIS_XY,
          BRAIN_MODEL_ROTATION_AXIS_OFF
       };
       
      /// caret mouse modes
      enum MOUSE_MODES {
         MOUSE_MODE_NONE,
         MOUSE_MODE_VIEW,
         MOUSE_MODE_BORDER_DRAW,
         MOUSE_MODE_BORDER_DRAW_NEW,
         MOUSE_MODE_BORDER_DELETE,
         MOUSE_MODE_BORDER_DELETE_POINT,
         MOUSE_MODE_BORDER_INTERPOLATE,
         MOUSE_MODE_BORDER_INTERPOLATE_NEW,
         MOUSE_MODE_BORDER_MOVE_POINT,
         MOUSE_MODE_BORDER_REVERSE,
         MOUSE_MODE_BORDER_RENAME,
         MOUSE_MODE_BORDER_UPDATE,
         MOUSE_MODE_BORDER_UPDATE_NEW,
         MOUSE_MODE_CUT_DRAW,
         MOUSE_MODE_CUT_DELETE,
         MOUSE_MODE_FOCI_DELETE,
         MOUSE_MODE_CELL_ADD,
         MOUSE_MODE_CELL_DELETE,
         MOUSE_MODE_SURFACE_ROI_BORDER_SELECT,
         MOUSE_MODE_SURFACE_ROI_PAINT_INDEX_SELECT,
         MOUSE_MODE_SURFACE_ROI_METRIC_NODE_SELECT,
         MOUSE_MODE_SURFACE_ROI_GEODESIC_NODE_SELECT,
         MOUSE_MODE_ALIGN_STANDARD_ORIENTATION,
         MOUSE_MODE_ALIGN_STANDARD_ORIENTATION_FULL_HEM_FLATTEN,
         MOUSE_MODE_CONTOUR_SET_SCALE,
         MOUSE_MODE_CONTOUR_DRAW,
         MOUSE_MODE_CONTOUR_ALIGN,
         MOUSE_MODE_CONTOUR_ALIGN_REGION,
         MOUSE_MODE_CONTOUR_POINT_MOVE,
         MOUSE_MODE_CONTOUR_POINT_DELETE,
         MOUSE_MODE_CONTOUR_DELETE,
         MOUSE_MODE_CONTOUR_REVERSE,
         MOUSE_MODE_CONTOUR_MERGE,
         MOUSE_MODE_CONTOUR_CELL_ADD,
         MOUSE_MODE_CONTOUR_CELL_DELETE,
         MOUSE_MODE_CONTOUR_CELL_MOVE,
         MOUSE_MODE_VOLUME_SEGMENTATION_EDIT,
         MOUSE_MODE_EDIT_ADD_NODE,
         MOUSE_MODE_EDIT_ADD_TILE,
         MOUSE_MODE_EDIT_DELETE_TILE_BY_LINK,
         MOUSE_MODE_EDIT_DISCONNECT_NODE,
         MOUSE_MODE_EDIT_MOVE_NODE,
         MOUSE_MODE_SURFACE_ROI_SHAPE_NODE_SELECT,
         MOUSE_MODE_TRANSFORMATION_MATRIX_AXES,
         MOUSE_MODE_TRANSFORMATION_MATRIX_SET_TRANSLATE,
         MOUSE_MODE_VOLUME_PAINT_EDIT,
         MOUSE_MODE_IMAGE_SUBREGION,
         MOUSE_MODE_SURFACE_ROI_SULCAL_BORDER_NODE_START,
         MOUSE_MODE_SURFACE_ROI_SULCAL_BORDER_NODE_END
      };
      
       /// Constructor
       GuiBrainModelOpenGL(QWidget* parent, 
                           GuiBrainModelOpenGL* sharedBrainModelOpenGL,
                           const char* name,
                           const BrainModel::BRAIN_MODEL_VIEW_NUMBER svn);
       
       /// Destructor
       ~GuiBrainModelOpenGL();
    
       /// is this the main windows OpenGL widget
       bool isMainWindowOpenGL() const 
             { return (viewingWindowIndex == BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW); }
       
       /// Get the surface/volume viewing index
       BrainModel::BRAIN_MODEL_VIEW_NUMBER getModelViewNumber() const { return viewingWindowIndex; }
       
       /// reset the linear object being drawn (border, contour, cut)
       void resetLinearObjectBeingDrawn();
      
       /// get the mouse mode
       MOUSE_MODES getMouseMode() const { return mouseMode; }
      
       /// set the mouse mode
       void setMouseMode(const MOUSE_MODES mm);
      
       /// save the graphics window into the image file.
       /// returns true if an error occurs.
       bool saveImage(const QString& name, const QString& format,
                      const bool addToSpecFile);
       
       /// Capture an image of the current graphics.
       void captureImage(QImage& image);
       
       /// Capture a portion of the image of the current graphics.
       void captureImageSubRegion(QImage& image);
       
       /// get image sub region valid
       bool getCaptureImageSubRegionValid() const;
       
       /// get the yoke status for this window
       bool getYokeView() const { return yokeView; }
       
       /// set the yoke status for this window
       void setYokeView(const bool yoke) { yokeView = yoke; }
       
       /// get the rotation axis
       BRAIN_MODEL_ROTATION_AXIS getRotationAxis() const { return rotationAxis; }
       
       /// set the rotation axis
       void setRotationAxis(const BRAIN_MODEL_ROTATION_AXIS axis) { rotationAxis = axis; }
       
       /// Update all graphics windows.  When modelToUpdate is NULL all OpenGL
       /// windows are updated.  If it is not NULL, the specified window and all
       /// windows "yoked" to it are updated.
       static void updateAllGL(GuiBrainModelOpenGL *modelToUpdate = NULL);
       
       /// get the minimum and maximum point size
       static void getPointSizeRange(float& minSize, float& maxSize);
       
       /// get the minimum and maximum line width
       static void getLineWidthRange(float& minSize, float& maxSize);
       
       /// get caption displayed in main window
       static QString getMainWindowCaption();

       /// set caption displayed in main window
       static void setMainWindowCaption(const QString& s);

       /// Get the displayed brain model index
       int getDisplayedBrainModelIndex() const;
       
       /// Get the orthographic projection box extents
       void getOrthographicBox(double& orthoLeft,   double& orthoRight,
                                      double& orthoBottom, double& orthoTop,
                                      double& orthoNear,   double& orthoFar) const;

       /// Get the brain set for this OpenGL Widget
       BrainSet* getBrainSet();
       
       /// Get the displayed brain model
       BrainModel* getDisplayedBrainModel();
       
       /// Get the displayed brain model contours (returns NULL if a surface is not displayed)
       BrainModelContours* getDisplayedBrainModelContours();
       
       /// Get the displayed brain model surface (returns NULL if a surface is not displayed)
       BrainModelSurface* getDisplayedBrainModelSurface();
       
       /// Get the displayed brain model surface and volume (returns NULL if a surface is not displayed)
       BrainModelSurfaceAndVolume* getDisplayedBrainModelSurfaceAndVolume();
       
       /// Get the displayed brain model volume (returns NULL if a volume is not displayed)
       BrainModelVolume* getDisplayedBrainModelVolume();
       
       /// display the brain model volume (if there is one)
       void displayBrainModelVolume();
       
       /// Get information about surface model in caret main window.
       static bool getCaretMainWindowModelInfo(
                                   BrainModelSurface*& mainCaretWindowModelSurface,
                                   GuiBrainModelOpenGL*& mainCaretWindowBrainModelOpenGL,
                                   int& modelViewIndex);
                                   
       /// Set the displayed brain model index
       void setDisplayedBrainModelIndex(const int newIndex);
      
       /// set the displayed brain model
       void setDisplayedBrainModel(const BrainModel* bm);
       
       /// Get the currently selected node DJS. 
       BrainModelOpenGLSelectedItem getSelectedNode() const { return openGL->getSelectedNode(); }
       
       
       /// Reset the displayed brain models in all windows
       static void setAllDisplayedBrainModelIndices(const int newIndex);
       
       /// Enable/Disable painting of the surface
       static void setPaintingEnabled(const bool flag) { paintingEnabled = flag; }
      
       /// get the BrainModelOpenGL object for a window (may be null if window closed)
       static GuiBrainModelOpenGL* getBrainModelOpenGLForWindow(const BrainModel::BRAIN_MODEL_VIEW_NUMBER n)
                                     { return allBrainSurfaceOpenGL[n]; }
       
       /// get main window center model coordinate (returns true if valid)
       static bool getMainWindowCenterModelCoordinate(float posOut[3]);
       
       /// get the opengl drawing object
       static BrainModelOpenGL* getOpenGLDrawing() { return openGL; }

       /// are viewing an oblique slice in a window
       static bool viewingObliqueSlice();
       
       /// set the model selection control used for this BrainModelOpenGL model selection
       void setModelSelectionControl(class GuiBrainSetAndModelSelectionControl* msc)
          { modelSelectionControl = msc; }
          
    public slots:
       /// redraw window
       void slotRedrawWindow();
       
    protected:
       /// initialize the OpenGL
       void initializeGL();
       
       /// Draws the graphics (called when graphics need to be updated).
       void paintGL();
       
       /// Sets the size of the OpenGL area.
       void resizeGL(int w, int h);
       
       /// Gets called by QT when mouse is moved with button down over OpenGL widget
       void mouseMoveEvent(QMouseEvent *me);
       
       /// Gets called by QT when mouse button is pressed over OpenGL widget
       void mousePressEvent(QMouseEvent *me);
       
       /// Get called by QT when mouse button is released over OpenGL widget
       void mouseReleaseEvent(QMouseEvent* me);
       
       /// Gets called by QT when a key is pressed
       void keyPressEvent(QKeyEvent* ke);
       
       /// Gets called by QT when a key is released
       void keyReleaseEvent(QKeyEvent* ke);
       
    private slots:
    
    private:
       /// create a display list containing a sphere
       void createSphereDisplayList();
       
       /// convert window coords to model corods
       bool convertWindowToModelCoords(const int windowX, const int windowY,
                                       const bool useZBufferForWindowZ,
                                       float& modelX, float& modelY, float& modelZ);
                                       
       /// select (Identify) something in the brain model           
       void selectBrainModelItem(const int x, const int y, 
                                 const unsigned long selectionMaskIn,
                                 const bool identifyFlag = false);

       /// update the node display flags
       static void updateNodeDisplayFlags(const int numNodes);
       
       /// key processing for contours
       void keyContourView(GuiBrainModelOpenGLKeyEvent& ke);
       
       /// key processing for surface
       void keySurfaceView(GuiBrainModelOpenGLKeyEvent& ke);
       
       /// key processing for volume
       void keyVolumeView(GuiBrainModelOpenGLKeyEvent& ke);
       
       /// key processing for surface and volume
       void keySurfaceAndVolumeView(GuiBrainModelOpenGLKeyEvent& ke);
       
       /// key processing for translation axes
       void keyTranslationAxes(GuiBrainModelOpenGLKeyEvent& ke);
       
       /// route the mouse event to the proper handler
       void routeMouseEvent(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for surface view mode
       void mouseSurfaceView(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for surface and volume view mode
       void mouseSurfaceAndVolumeView(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for border draw mode
       void mouseBorderDraw(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for border draw mode NEW
       void mouseBorderDrawNew(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for border delete mode
       void mouseBorderDelete(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for border delete point mode
       void mouseBorderDeletePoint(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for border move point
       void mouseBorderMovePoint(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for border interpolate
       void mouseBorderInterpolate(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for border interpolate NEW
       void mouseBorderInterpolateNew(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for border reverse
       void mouseBorderReverse(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for border rename
       void mouseBorderRename(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for border update
       void mouseBorderUpdate(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for border update NEW
       void mouseBorderUpdateNew(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for draw cut mode
       void mouseCutDraw(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for delete cut mode
       void mouseCutDelete(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for delete foci
       void mouseFociDelete(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for delete cell
       void mouseCellDelete(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for roi border selection
       void mouseSurfaceRoiBorderSelect(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for roi paint selection
       void mouseSurfaceRoiPaintSelect(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for roi metric selection
       void mouseSurfaceRoiMetricSelect(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for roi shape selection
       void mouseSurfaceRoiShapeSelect(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for roi geodesic node selection
       void mouseSurfaceRoiGeodesicNodeSelect(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for align surface to standard orientation
       void mouseAlignStandardOrientation(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse processing for align surface to standard orientation on flatten full hem
       void mouseAlignStandardOrientationFullHemFlatten(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for cell add mode
       void mouseCellAdd(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for volume transforms in view mode
       void mouseVolumeView(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for image subregion selection
       void mouseImageSubRegion(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for roi sulcal border start node
       void mouseSurfaceROISulcalBorderNodeStart(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for roi sulcal border end node
       void mouseSurfaceROISulcalBorderNodeEnd(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for contour transforms in view mode
       void mouseContourView(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for contour set scale
       void mouseContourSetScale(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for contour draw
       void mouseContourDraw(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for contour align
       void mouseContourAlign(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for contour region align
       void mouseContourAlignRegion(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for contour move point
       void mouseContourPointMove(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for contour delete point
       void mouseContourPointDelete(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for contour delete
       void mouseContourDelete(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for contour point reversal
       void mouseContourReverse(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse process for contour merge
       void mouseContourMerge(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse mode for contour cell add
       void mouseContourCellAdd(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse mode for contour cell delete
       void mouseContourCellDelete(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse mode for contour cell delete
       void mouseContourCellMove(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse mode for volume segmentation editing
       void mouseVolumeSegmentationEdit(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse mode for volume paint editing
       void mouseVolumePaintEdit(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse mode for adding a node
       void mouseAddNodes(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse mode for adding a tile
       void mouseAddTile(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse mode for deleting tile by clicking link
       void mouseDeleteTileByLink(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse mode for disconnect node
       void mouseDisconnectNode(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse mode for move node
       void mouseMoveNode(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse mode for setting translation axes translate
       void mouseTranslationAxesSetTranslate(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// mouse mode for translation axes
       void mouseTranslationAxes(const GuiBrainModelOpenGLMouseEvent& me);
       
       /// Draw the linear object (typically used while drawing borders)
       void drawLinearObject();

       /// Convert from border XYZ to screen XYZ
       void convertVolumeBorderLinkToScreenXY(const VolumeFile::VOLUME_AXIS axis,
                                              float xyz[3]);
                                  
       /// see if in view mode
       bool getInViewMode() const { return (mouseMode == MOUSE_MODE_VIEW); }
       
       /// width of OpenGL window
       int windowWidth[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
       
       /// height of OpenGL window
       int windowHeight[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
       
       ///  rotation axis
       BRAIN_MODEL_ROTATION_AXIS rotationAxis;
       
       /// yoke view status
       bool yokeView;
       
       /// all graphics windows
       static GuiBrainModelOpenGL* allBrainSurfaceOpenGL[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
       
       /// index for this viewer in the BrainModel.h
       BrainModel::BRAIN_MODEL_VIEW_NUMBER viewingWindowIndex;
       
       /// the mouse mode 
       MOUSE_MODES mouseMode;
      
       /// last X position of mouse
       int lastMouseX;
       
       /// last Y position of mouse
       int lastMouseY;
       
       /// X position of mouse when mouse button pressed
       int mousePressX;
       
       /// Y position of mouse when mouse button pressed
       int mousePressY;
       
       /// track minimum and maximum mouse movement (minX, minY, maxX, maxY)
       int mouseMovedBounds[4];
       
       /// orthographic projection box for model
       double orthographicLeft[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
       double orthographicRight[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
       double orthographicBottom[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
       double orthographicTop[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
       double orthographicFar[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
       double orthographicNear[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
       
       /// selected node
       BrainModelOpenGLSelectedItem selectedNode;

       /// selected border (first)
       BrainModelOpenGLSelectedItem selectedBorder1;

       /// selected border (second)
       BrainModelOpenGLSelectedItem selectedBorder2;

       /// selected border (volume)
       BrainModelOpenGLSelectedItem selectedVolumeBorder;

       /// selected cell
       BrainModelOpenGLSelectedItem selectedCellProjection;

       /// selected volume cell
       BrainModelOpenGLSelectedItem selectedVolumeCell;
       
       /// selected transform cell
       BrainModelOpenGLSelectedItem selectedTransformCell;

       /// selected cut
       BrainModelOpenGLSelectedItem selectedCut;
 
       /// selected foci
       BrainModelOpenGLSelectedItem selectedFocusProjection;

       /// selected volume foci
       BrainModelOpenGLSelectedItem selectedVolumeFoci;
       
       /// selected transform foci
       BrainModelOpenGLSelectedItem selectedTransformFocus;
       
       /// selected palette metric
       BrainModelOpenGLSelectedItem selectedPaletteMetric;

       /// selected palette shape
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
       BrainModelOpenGLSelectedItem selectedTransformationAxes;
       
       /// selected vtk model
       BrainModelOpenGLSelectedItem selectedVtkModel;
       
       /// enable OpenGL drawing
       static bool paintingEnabled;
       
       /// linear (border, cut, contour) being drawn
       Border linearObjectBeingDrawn;
       
       /// resampling density of linear object being drawn
       float linearObjectBeingDrawnSampling;
       
       /// linear object augment border 1
       BrainModelOpenGLSelectedItem linearObjectAugmentBorder1;
       
       /// linear object augment border 2
       BrainModelOpenGLSelectedItem linearObjectAugmentBorder2;
       
       /// linear object augment border count
       int linearObjectAugmentBorderCount;
       
       /// draw linear object flag
       bool drawLinearObjectOnly;
       
       /// the popup menu shown with right mouse press
       GuiBrainModelOpenGLPopupMenu* popupMenu;
       
       /// border file number, border number, and border point number being moved
       int borderPointBeingMoved[3];
       
       /// node numbers of tile being entered
       int newTileNodeNumbers[3];
       
       /// counter for new tile nodes
       int newTileNodeCounter;
       
       /// model selection control
       GuiBrainSetAndModelSelectionControl* modelSelectionControl;
       
       /// volume slice axis
       //VolumeFile::VOLUME_AXIS volumeSliceAxis;
       
       /// volume slice coordinate
       //float volumeSliceCoordinate;
       
       /// a key was up last time
       bool keyUpLastTime;
       
       /// box for image subregion
       int imageSubRegionBox[4];
       
       /// image subregion box valid
       bool imageSubRegionBoxValid;
       
       /// version number of OpenGL
       static float versionOfOpenGL;
       
       /// left mouse button pressonly
       static int leftMouseButtonPressMask;
       
       /// left mouse button + shift press
       static int leftShiftMouseButtonPressMask;
       
       /// left mouse button + control press
       static int leftControlMouseButtonPressMask;
       
       /// left mouse button only move
       static int leftMouseButtonMoveMask;
       
       /// left mouse button + shift move
       static int leftShiftMouseButtonMoveMask;
       
       /// left mouse button + control move
       static int leftControlMouseButtonMoveMask;
       
       /// left mouse button + alt move
       static int leftAltMouseButtonMoveMask;
       
       /// default orthographic window size
       static float defaultOrthoWindowSize;
       
       /// amount mouse must move to NOT be an ID node operation
       static int mouseMoveTolerance;
       
       /// OpenGL drawing
       static BrainModelOpenGL* openGL;
       
       /// min point size
       static float minPointSize;
       
       /// max point size
       static float maxPointSize;
       
       /// min line size
       static float minLineSize;
       
       /// max line size
       static float maxLineSize;
};

#ifdef __GUI_BRAIN_SURFACE_OPENGL_INIT_STATIC__
                
BrainModelOpenGL* GuiBrainModelOpenGL::openGL = NULL;

GuiBrainModelOpenGL* 
   GuiBrainModelOpenGL::allBrainSurfaceOpenGL[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS] = 
   { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

bool GuiBrainModelOpenGL::paintingEnabled = true;

float GuiBrainModelOpenGL::versionOfOpenGL = 0.0;

int GuiBrainModelOpenGL::leftMouseButtonPressMask = 0;
int GuiBrainModelOpenGL::leftShiftMouseButtonPressMask = 0;
int GuiBrainModelOpenGL::leftControlMouseButtonPressMask = 0;
int GuiBrainModelOpenGL::leftMouseButtonMoveMask = 0;
int GuiBrainModelOpenGL::leftShiftMouseButtonMoveMask = 0;
int GuiBrainModelOpenGL::leftControlMouseButtonMoveMask = 0;
int GuiBrainModelOpenGL::leftAltMouseButtonMoveMask = 0;

float GuiBrainModelOpenGL::minPointSize = 0.0;
float GuiBrainModelOpenGL::maxPointSize = 0.0;
float GuiBrainModelOpenGL::minLineSize = 0.0;
float GuiBrainModelOpenGL::maxLineSize = 0.0;

float GuiBrainModelOpenGL::defaultOrthoWindowSize = 125.0;

int GuiBrainModelOpenGL::mouseMoveTolerance = 2;

#endif // __GUI_BRAIN_SURFACE_OPENGL_INIT_STATIC__

#endif

