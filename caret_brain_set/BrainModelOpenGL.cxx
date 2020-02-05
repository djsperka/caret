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
#include <cmath>
#include <iostream>
#include <limits>

#include <QGlobalStatic>
#ifdef Q_OS_WIN32
#define NOMINMAX
#endif

#include "AreaColorFile.h"
#include "BorderColorFile.h"
#include "BrainModel.h"
#include "BrainModelContours.h"
#include "BrainModelIdentification.h"
#define __BRAIN_MODEL_OPENGL_MAIN__
#include "BrainModelOpenGL.h"
#undef __BRAIN_MODEL_OPENGL_MAIN__
#include "BrainModelSurface.h"
#include "BrainModelSurfaceAndVolume.h"
#include "BrainModelSurfaceNodeColoring.h"
#include "BrainModelSurfacePointProjector.h"
#include "BrainModelSurfaceROINodeSelection.h"
#include "BrainModelVolume.h"
#include "BrainModelVolumeRegionOfInterest.h"
#include "BrainModelVolumeVoxelColoring.h"
#include "BrainSet.h"
#include "BrainSetNodeAttribute.h"
#include "CellColorFile.h"
#include "CellFile.h"
#include "ColorFile.h"
#include "ContourCellColorFile.h"
#include "ContourCellFile.h"
#include "CutsFile.h"
#include "DebugControl.h"
#include "DeformationFieldFile.h"
#include "DisplaySettingsDeformationField.h"
#include "DisplaySettingsBorders.h"
#include "DisplaySettingsCells.h"
#include "DisplaySettingsCoCoMac.h"
#include "DisplaySettingsContours.h"
#include "DisplaySettingsCuts.h"
#include "DisplaySettingsFoci.h"
#include "DisplaySettingsGeodesicDistance.h"
#include "DisplaySettingsImages.h"
#include "DisplaySettingsMetric.h"
#include "DisplaySettingsSurface.h"
#include "DisplaySettingsSurfaceShape.h"
#include "DisplaySettingsVectors.h"
#include "DisplaySettingsVolume.h"
#include "DisplaySettingsModels.h"
#include "FileUtilities.h"
#include "FociColorFile.h"
#include "FociFile.h"
#include "FociProjectionFile.h"
#include "GeodesicDistanceFile.h"
#include "MathUtilities.h"
#include "PaletteFile.h"
#include "SectionFile.h"
#include "SurfaceShapeFile.h"
#include "VectorFile.h"
#include "TopologyHelper.h"
#include "TrajectoryFile.h"
#include "TransformationMatrixFile.h"
#include "VtkModelFile.h"

#include "vtkMath.h"
#include "vtkTransform.h"
#include "vtkTriangle.h"

// These are utility functions to help generate cylinder points
void generateCircle(double center[3], double r, int N, double *pts, double *norms, bool bOutward);
void intersectLinePlane(double p0[3], double n[3], double p1[3], double p2[3], double p[3]);
void dumpvec2(std::string s, double f[3]);


/* Number of slices used in Cylinder,Disk,Cone quadrics */
static const int QUADRIC_NUMBER_OF_SLICES = 4;

/**
 * Constructor.
 */
BrainModelOpenGL::BrainModelOpenGL()
{
   initializationCompletedFlag = false;
   offScreenRenderingFlag = false;
   useDisplayListsForShapes = true;
   
   disableClearingFlag = false;
   brainSet = NULL;
   
   sphereQuadric = NULL;
   diskQuadric = NULL;
   coneQuadric = NULL;
   cylinderQuadric = NULL;
   diamondQuadric = NULL;
   ringQuadric = NULL;
    
   boxDisplayList = 0;
   coneDisplayList = 0;
   diamondDisplayList = 0;
   cylinderDisplayList = 0;
   diskDisplayList = 0;
   ringDisplayList = 0;
   sphereDisplayList = 0;
   squareDisplayList = 0;
   selectionMask = SELECTION_MASK_OFF;
   nodeSpecialHighlighting.clear();
   drawLinearObjectOnly = false;
   //qt4if (paintMutex.locked()) {
    //  paintMutex.unlock();
   //qt4}
   drawImageSubRegionBoxFlag = false;
   
   static bool firstTime = true;
   if (firstTime) {
      for (int i = 0; i < 16; i++) {
         const int indx = i * 8;
         polygonStipple[indx+0] = 0xAA;
         polygonStipple[indx+1] = 0xAA;
         polygonStipple[indx+2] = 0xAA;
         polygonStipple[indx+3] = 0xAA;
         polygonStipple[indx+4] = 0x55;
         polygonStipple[indx+5] = 0x55;
         polygonStipple[indx+6] = 0x55;
         polygonStipple[indx+7] = 0x55;
      }
   }
   firstTime = false;
}

/**
 * Destructor.
 */
BrainModelOpenGL::~BrainModelOpenGL()
{
   if (sphereQuadric != NULL) {
      gluDeleteQuadric(sphereQuadric);
      sphereQuadric = NULL;
   }
   if (diskQuadric != NULL) {
      gluDeleteQuadric(diskQuadric);
      diskQuadric = NULL;
   }
   if (coneQuadric != NULL) {
      gluDeleteQuadric(coneQuadric);
      coneQuadric = NULL;
   }
   if (cylinderQuadric != NULL) {
      gluDeleteQuadric(cylinderQuadric);
      cylinderQuadric = NULL;
   }
   if (diamondQuadric != NULL) {
      gluDeleteQuadric(diamondQuadric);
      diamondQuadric = NULL;
   }
   if (ringQuadric != NULL) {
      gluDeleteQuadric(ringQuadric);
      ringQuadric = NULL;
   }
/*
   if (sphereDisplayList > 0) {
      glDeleteLists(sphereDisplayList, 1);
      sphereDisplayList = 0;
   }
   if (diskDisplayList > 0) {
      glDeleteLists(diskDisplayList, 1);
      diskDisplayList = 0;
   }
*/
}

/**
 * stuff for web caret.
 */
void 
BrainModelOpenGL::webCaretCommon(BrainSet* bs,
                                 const int viewport[4])
{
   offScreenRenderingFlag = true;
   
   PreferencesFile* pf = bs->getPreferencesFile();
   pf->setDisplayListsEnabled(false);
   
   useDisplayListsForShapes = false;
   initializeOpenGL(true);
   
   updateOrthoSize(0, viewport[2], viewport[3]);

   bs->setDisplaySplashImage(false);
   
}

/**
 * Draw a brain model for WebCaret.
 */
void 
BrainModelOpenGL::drawBrainModelWebCaret(BrainSet* bs,
                                         BrainModel* bm,
                                         const int viewingWindowNumberIn,
                                         const int viewportIn[4])
{
   webCaretCommon(bs, viewportIn);
   
   drawBrainModel(bs,
                  bm, 
                  viewingWindowNumberIn, 
                  viewportIn, 
                  NULL);
}

/**
 * select (Identify) something in the brain model for WebCaret.
 */
void 
BrainModelOpenGL::selectBrainModelItemWebCaret(BrainSet* bs,
                                               BrainModel* bm,
                                               const int viewportIn[4],
                                               const unsigned long selectionMaskIn,
                                               const int selectionXIn, 
                                               const int selectionYIn)
{
   webCaretCommon(bs, viewportIn);
   
   selectBrainModelItem(bs,
                        bm,
                        0,
                        viewportIn,
                        NULL,
                        selectionMaskIn,
                        selectionXIn,
                        selectionYIn,
                        true);
}

/**
 * Identify something in the brain model for WebCaret.
 */
QString 
BrainModelOpenGL::identifyBrainModelItemWebCaret(BrainSet* bs,
                                                 BrainModel* bm,
                                                 const int viewingWindowNumberIn,
                                                 const int viewportIn[4],
                                                 const unsigned long selectionMaskIn,
                                                 const int selectionXIn, 
                                                 const int selectionYIn,
                                                 const bool enableHtml,
                                                 const bool enableVocabularyLinks)
{
   webCaretCommon(bs, viewportIn);
   
   BrainModel* allWindowBrainModelsForIdentification[BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS];
   for (int i = 0; i < BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; i++) {
      allWindowBrainModelsForIdentification[i] = NULL;
   }
   allWindowBrainModelsForIdentification[BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW] = bm;
   
   return identifyBrainModelItem(bs,
                                 bm,
                                 allWindowBrainModelsForIdentification,
                                 viewingWindowNumberIn,
                                 viewportIn,
                                 NULL,
                                 selectionMaskIn,
                                 selectionXIn,
                                 selectionYIn,
                                 true,
                                 enableHtml,
                                 enableVocabularyLinks);
}
      
/**
 * all brain models displayed in windows for identification
 * ONLY VALID WHEN ::identifyBrainModel... called
 */
BrainModel* 
BrainModelOpenGL::getBrainModelInWindowNumberForIdentification(const int windowNumber) 
{ 
   return allWindowBrainModelsForIdentification[windowNumber]; 
}
     
/**
 * Draw a brain model.
 */
void 
BrainModelOpenGL::drawBrainModel(BrainSet* bs,
                                 BrainModel* bm,
                                 const int viewingWindowNumberIn,
                                 const int viewportIn[4],
                                 QGLWidget* glWidgetIn)
{
   brainSet = bs;
   selectionMask = SELECTION_MASK_OFF;
   drawBrainModelPrivate(bm,
                         viewingWindowNumberIn,
                         viewportIn,
                         glWidgetIn);
   brainSet = NULL;
}

/**
 * Draw all fiducial surface brain models.
 */
void 
BrainModelOpenGL::drawAllFiducialSurfaceBrainModels(std::vector<BrainSet*> brainSetsIn,
                                                    const int viewingWindowNumberIn,
                                                    const int viewportIn[4],
                                                    QGLWidget* glWidgetIn)
{
   selectionMask = SELECTION_MASK_OFF;
   QString firstFiducialTransforms;
   bool firstFiducialFlag = true;
   
   //
   // loop through brain sets looking for fiducial surfaces
   //
   for (unsigned int i = 0; i < brainSetsIn.size(); i++) {
      BrainModelSurface* bms = brainSetsIn[i]->getActiveFiducialSurface();
      if (bms != NULL) {
         brainSet = brainSetsIn[i];
         
         //
         // If not the first fiducial surface, replace its transforms with 
         // those from the first fiducial surface
         //
         const QString savedTransformations = bms->getTransformationsAsString(viewingWindowNumberIn);
         if (firstFiducialFlag == false) {
            bms->setTransformationsAsString(viewingWindowNumberIn, firstFiducialTransforms);
         }
         
         //
         // Draw the fiducial surface
         //
         drawBrainModelPrivate(bms,
                               viewingWindowNumberIn,
                               viewportIn,
                               glWidgetIn);
                          
         //
         // Was this the first fiducial surface ?
         //
         if (firstFiducialFlag) {
            //
            // Save its transformations and disable clearing 
            //
            firstFiducialTransforms = bms->getTransformationsAsString(viewingWindowNumberIn);
            firstFiducialFlag = false;
            disableClearingFlag = true;
         }
         else {
            //
            // Restore transformations
            //
            bms->setTransformationsAsString(viewingWindowNumberIn, savedTransformations);
         }
      }
   }
   
   brainSet = NULL;
   disableClearingFlag = false;
}

/**
 * Draw a brain model.
 */
void 
BrainModelOpenGL::drawBrainModelPrivate(BrainModel* bm,
                                 const int viewingWindowNumberIn,
                                 const int viewportIn[4],
                                 QGLWidget* glWidgetIn)
{
   if (DebugControl::getOpenGLDebugOn()) {
      checkForOpenGLError(bm, "At beginning of drawBrainModelPrivate()");
   }
   
   brainModel = bm;
   mainWindowFlag = (viewingWindowNumberIn == 0);
   viewport[0]    = viewportIn[0];
   viewport[1]    = viewportIn[1];
   viewport[2]    = viewportIn[2];
   viewport[3]    = viewportIn[3];
   viewingWindowNumber = viewingWindowNumberIn;
   glWidget       = glWidgetIn;
   
   //
   // No text will be drawn if (glWidget == NULL)
   //
   if (openGLTextEnabledFlag == false) {
      glWidget = NULL; 
   }
   
   if (DebugControl::getOpenGLDebugOn()) {
      checkForOpenGLError(bm, "In drawBrainModelPrivate() before viewport set");
   }
   
   glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
   selectionViewport[viewingWindowNumber][0] = viewport[0];
   selectionViewport[viewingWindowNumber][1] = viewport[1];
   selectionViewport[viewingWindowNumber][2] = viewport[2];
   selectionViewport[viewingWindowNumber][3] = viewport[3];
   
   if (DebugControl::getOpenGLDebugOn()) {
      checkForOpenGLError(bm, "In drawBrainModelPrivate() after viewport set");
   }
   
   if (mainWindowFlag) {
      brainSet->updateNodeDisplayFlags();
   }
   
   paintMutex.lock();
   
   glMatrixMode(GL_MODELVIEW);
   
   PreferencesFile* pf = brainSet->getPreferencesFile();
   if (brainSet->getDisplaySplashImage()) {
      glClearColor(1.0, 1.0, 1.0, 0.0);
   }
   else {
      unsigned char rb, gb, bb;
      pf->getSurfaceBackgroundColor(rb, gb, bb);
      const float bg[3] = { static_cast<float>(rb) / 255.0,
                           static_cast<float>(gb) / 255.0,
                           static_cast<float>(bb) / 255.0 };
      glClearColor(bg[0], bg[1], bg[2], 0.0);
   }
   
   if (DebugControl::getOpenGLDebugOn()) {
      checkForOpenGLError(bm, "In drawBrainModelPrivate() after glClear()");
   }
   
   glPushMatrix();
      glLoadIdentity();
      float lightPosition[4];
      pf->getLightPosition(lightPosition[0], lightPosition[1], lightPosition[2]);
      lightPosition[3] = 0.0;
      glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
      glEnable(GL_LIGHT0);

      //
      // Light 1 position is opposite of light 0
      //
      lightPosition[0] = -lightPosition[0];
      lightPosition[1] = -lightPosition[1];
      lightPosition[2] = -lightPosition[2];
      glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);      
      glEnable(GL_LIGHT1);
   glPopMatrix();
   
   if (DebugControl::getOpenGLDebugOn()) {
      checkForOpenGLError(bm, "In drawBrainModelPrivate() after lighting");
   }
   
   //
   // if this flag is set, we are only drawing the linear object and leaving
   // the previously drawn display as it was.
   //
   if (disableClearingFlag == false) {
      if (drawLinearObjectOnly == false) {
         if (selectionMask != SELECTION_MASK_OFF) {
            //
            // Not clearing color buffer prevents flashing when ID'ing a volume slice voxel
            //
            glClear(GL_DEPTH_BUFFER_BIT);
         }
         else {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         }
      }
   }
   
   glLoadIdentity();
   
   if (mainWindowFlag) {
      displayImages();
   }
   
   if (DebugControl::getOpenGLDebugOn()) {
      checkForOpenGLError(bm, "In drawBrainModelPrivate() after image display");
   }
   
   if (bm != NULL) {
      switch (bm->getModelType()) {
         case BrainModel::BRAIN_MODEL_CONTOURS:
            drawBrainModelContours(dynamic_cast<BrainModelContours*>(bm));
            break;
         case BrainModel::BRAIN_MODEL_SURFACE:
            drawBrainModelSurface(dynamic_cast<BrainModelSurface*>(bm), NULL, true, false);
            break;
         case BrainModel::BRAIN_MODEL_VOLUME:
            drawBrainModelVolume(dynamic_cast<BrainModelVolume*>(bm));
            break;
         case BrainModel::BRAIN_MODEL_SURFACE_AND_VOLUME:
            drawBrainModelSurfaceAndVolume(dynamic_cast<BrainModelSurfaceAndVolume*>(bm));
            break;
      }
   }
   
   if (mainWindowFlag) {
      drawMainWindowCaption();
   }
   
   if (drawImageSubRegionBoxFlag) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0.0, static_cast<double>(viewport[2]), 
              0.0, static_cast<double>(viewport[3]), 
              -1.0, 1.0);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glColor3f(1.0, 0.0, 0.0);
      
      const int x1 = std::min(imageSubRegionBox[0], imageSubRegionBox[2]);
      const int x2 = std::max(imageSubRegionBox[0], imageSubRegionBox[2]);
      const int y1 = std::min(imageSubRegionBox[1], imageSubRegionBox[3]);
      const int y2 = std::max(imageSubRegionBox[1], imageSubRegionBox[3]);
      
      GLboolean depthBufferOn = glIsEnabled(GL_DEPTH_TEST);
      glDisable(GL_DEPTH_TEST);
      
      glLineWidth(getValidLineWidth(1.0));
      glBegin(GL_LINE_LOOP);
         glVertex2i(x1, y1);
         glVertex2i(x2, y1);
         glVertex2i(x2, y2);
         glVertex2i(x1, y2);
      glEnd();
      
      if (depthBufferOn) {
         glEnable(GL_DEPTH_TEST);
      }
   }
      
/**
   GLenum errorCode = glGetError();
   if (errorCode != GL_NO_ERROR) {
      std::cout << std::endl;
      std::cout << "OpenGL Error: " << (char*)gluErrorString(errorCode) << std::endl;
      if (bm != NULL) {
         std::cout << "While drawing brain model " << bm->getDescriptiveName().toAscii().constData() << std::endl;
      }
      std::cout << "In window number " << viewingWindowNumber << std::endl;
      GLint nameStackDepth, modelStackDepth, projStackDepth;
      glGetIntegerv(GL_PROJECTION_STACK_DEPTH,
                    &projStackDepth);
      glGetIntegerv(GL_MODELVIEW_STACK_DEPTH,
                    &modelStackDepth);
      glGetIntegerv(GL_NAME_STACK_DEPTH,
                    &nameStackDepth);
      std::cout << "Projection Matrix Stack Depth " << projStackDepth << std::endl;
      std::cout << "Model Matrix Stack Depth " << modelStackDepth << std::endl;
      std::cout << "Name Matrix Stack Depth " << nameStackDepth << std::endl;
      std::cout << std::endl;
   }
*/
   
   glFlush();
   
   paintMutex.unlock();   

   checkForOpenGLError(bm, "At end of drawBrainModelPrivate().");
}

/**
 * check for an OpenGL Error.
 */
void 
BrainModelOpenGL::checkForOpenGLError(const BrainModel* bm,
                                      const QString& msg)
{
   GLenum errorCode = glGetError();
   if (errorCode != GL_NO_ERROR) {
      std::cout << std::endl;
      std::cout << "OpenGL Error: " << (char*)gluErrorString(errorCode) << std::endl;
      std::cout << "OpenGL Version: " << (char*)(glGetString(GL_VERSION)) << std::endl;
      std::cout << "OpenGL Vendor:  " << (char*)(glGetString(GL_VENDOR)) << std::endl;
      if (msg.isEmpty() == false) {
         std::cout << msg.toAscii().constData() << std::endl;
      }
      if (bm != NULL) {
         std::cout << "While drawing brain model " << bm->getDescriptiveName().toAscii().constData() << std::endl;
      }
      std::cout << "In window number " << viewingWindowNumber << std::endl;
      GLint nameStackDepth, modelStackDepth, projStackDepth;
      glGetIntegerv(GL_PROJECTION_STACK_DEPTH,
                    &projStackDepth);
      glGetIntegerv(GL_MODELVIEW_STACK_DEPTH,
                    &modelStackDepth);
      glGetIntegerv(GL_NAME_STACK_DEPTH,
                    &nameStackDepth);
      std::cout << "Projection Matrix Stack Depth " << projStackDepth << std::endl;
      std::cout << "Model Matrix Stack Depth " << modelStackDepth << std::endl;
      std::cout << "Name Matrix Stack Depth " << nameStackDepth << std::endl;
      std::cout << std::endl;
   }
}
      
/**
 * initialize OpenGL (should only call one time).
 */
void
BrainModelOpenGL::initializeOpenGL(const bool offScreenRenderingFlagIn)
{
   offScreenRenderingFlag = offScreenRenderingFlagIn;
   
   if (versionOfOpenGL == 0.0) {
      //
      // Note: The version string might be something like 1.2.4.  std::atof()
      // will get just the 1.2 which is okay.
      //
      const char* versionStr = (char*)(glGetString(GL_VERSION));
      versionOfOpenGL = std::atof(versionStr);
   }
   
   drawImageSubRegionBoxFlag = false;
   
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   glClearDepth(1.0);
   glFrontFace(GL_CCW);
#ifndef GL_VERSION_1_3 
   glEnable(GL_NORMALIZE);
#else
   if (versionOfOpenGL >= 1.3) {
      glEnable(GL_RESCALE_NORMAL);
   }
   else {
      glEnable(GL_NORMALIZE);
   }
#endif
   
  // glEnable(GL_CULL_FACE);
  // glCullFace(GL_BACK);
   
   glShadeModel(GL_SMOOTH);
   
   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
   glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
   const GLfloat lightColor[4] = { 0.9, 0.9, 0.9, 1.0 };
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
   glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor);
   glEnable(GL_LIGHT0);
   glDisable(GL_LIGHT1);
   
   const GLfloat materialColor[4] = { 0.8, 0.8, 0.8, 1.0 };
   glMaterialfv(GL_FRONT, GL_DIFFUSE, materialColor);
   glColorMaterial(GL_FRONT, GL_DIFFUSE);
   
   const GLfloat ambient[4] = { 0.8, 0.8, 0.8, 1.0 };
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
   
   createBoxDisplayList();
   createConeQuadricAndDisplayList();
   createCylinderQuadricAndDisplayList();
   createDiamondQuadricAndDisplayList();
   createDiskQuadricAndDisplayList();
   createRingQuadricAndDisplayList();
   createSphereQuadricAndDisplayList();
   createSquareDisplayList();
   
   float sizes[2];
   glGetFloatv(GL_POINT_SIZE_RANGE, sizes);
   minimumPointSize = sizes[0];
   maximumPointSize = sizes[1];

   glGetFloatv(GL_LINE_WIDTH_RANGE, sizes);
   minimumLineWidth = sizes[0];
   maximumLineWidth = sizes[1];

   initializationCompletedFlag = true;
}

/**
 * get minimum/maximum point size.
 */
void 
BrainModelOpenGL::getMinMaxPointSize(float& minSizeOut, float& maxSizeOut)
{
   float sizes[2];
   glGetFloatv(GL_POINT_SIZE_RANGE, sizes);
   minSizeOut = sizes[0];
   maxSizeOut = sizes[1];
}

/**
 * get minimum/maximum line width.
 */
void 
BrainModelOpenGL::getMinMaxLineWidth(float& minWidthOut, float& maxWidthOut)
{
   float sizes[2];
   glGetFloatv(GL_LINE_WIDTH_RANGE, sizes);
   minWidthOut = sizes[0];
   maxWidthOut = sizes[1];
}
      
/**
 * draw contours.
 */
void 
BrainModelOpenGL::drawBrainModelContours(BrainModelContours* bmc)
{
   const DisplaySettingsContours* dsc = brainSet->getDisplaySettingsContours();

   //
   // See if contours are being aligned
   //
   bool aligningContours = dsc->getAlignmentContourValid();
   int alignmentSectionNumber = dsc->getAlignmentContourNumber();
   
   //
   // Don't set projection matrix when selecting
   //
   if (selectionMask == SELECTION_MASK_OFF) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      
      glOrtho(orthographicLeft[viewingWindowNumber], orthographicRight[viewingWindowNumber], 
              orthographicBottom[viewingWindowNumber], orthographicTop[viewingWindowNumber], 
              orthographicNear[viewingWindowNumber], orthographicFar[viewingWindowNumber]);
      
      glGetDoublev(GL_PROJECTION_MATRIX, selectionProjectionMatrix[viewingWindowNumber]);
   }

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   float translate[3];
   bmc->getTranslation(viewingWindowNumber, translate);
   glTranslatef(translate[0], translate[1], translate[2]);
   
   //
   // Origin cross (do before rotation and scaling)
   //
   if (dsc->getDisplayCrossAtOrigin()) {
      unsigned char r, g, b;
      PreferencesFile* pref = brainSet->getPreferencesFile();
      pref->getSurfaceForegroundColor(r, g, b);
      glColor3ub(r, g, b);
      const float length = orthographicRight[viewingWindowNumber] * 0.20;
      glBegin(GL_LINES);
         const float z = 10.0 - orthographicFar[viewingWindowNumber];
         glVertex3f(-length, 0.0, z);
         glVertex3f( length, 0.0, z);
         glVertex3f(0.0, -length, z);
         glVertex3f(0.0,  length, z);
      glEnd();
   }
   
   float matrix[16];
   bmc->getRotationMatrix(viewingWindowNumber, matrix);
   glMultMatrixf(matrix);
   
   float scale[3];
   bmc->getScaling(viewingWindowNumber, scale);
   glScalef(scale[0], scale[1], scale[2]);
   
   //
   // Save modeling matrix
   //
   if (selectionMask == SELECTION_MASK_OFF) {
      glGetDoublev(GL_MODELVIEW_MATRIX, selectionModelviewMatrix[viewingWindowNumber]);
   }
   
   //
   // If this flag is set we only want to draw the linear object.  When this
   // flag is set, glClear() is not called so the previous stuff is not erased.
   //
   if (drawLinearObjectOnly) {
      glDisable(GL_DEPTH_TEST);
      const int num = linearObjectBeingDrawn.getNumberOfLinks();
      glPointSize(getValidPointSize(2.0));
      glColor3f(1.0, 0.0, 0.0);
      glBegin(GL_POINTS);
         for (int i = 0; i < num; i++) {
            glVertex3fv(linearObjectBeingDrawn.getLinkXYZ(i));
         }
      glEnd();
      glEnable(GL_DEPTH_TEST);
      //return;
   }
   
   const ContourFile* cf = bmc->getContourFile();
   const int numContours = cf->getNumberOfContours();
   const float spacing = cf->getSectionSpacing();
   const int minimumSection = cf->getMinimumSelectedSection();
   const int maximumSection = cf->getMaximumSelectedSection();
   
   const ContourCellFile* cellFile = brainSet->getContourCellFile();
   const int numCells = cellFile->getNumberOfCells();
   const ContourCellColorFile* colorFile = brainSet->getContourCellColorFile();
   const int numColors = colorFile->getNumberOfColors();
   
   //
   // Are contours hidden ?
   //
   const bool hideContours = (cf->getSectionType() == ContourFile::SECTION_TYPE_HIDE);
   
   //
   // Set color for cells with missing colors
   //
   unsigned char noColorRed, noColorGreen, noColorBlue;
   PreferencesFile* pref = brainSet->getPreferencesFile();
   pref->getSurfaceForegroundColor(noColorRed, noColorGreen, noColorBlue);
   
   const float cellSize = dsc->getContourCellSize();
   
   glPointSize(getValidPointSize(dsc->getPointSize()));
   glLineWidth(getValidLineWidth(dsc->getLineThickness()));
   
   if (hideContours == false) {
      if (selectionMask & SELECTION_MASK_CONTOUR) {
         glPushName(SELECTION_MASK_CONTOUR);
         for (int i = 0; i < numContours; i++) {
            const CaretContour* contour = cf->getContour(i);
            const int sectionNumber = contour->getSectionNumber();
            
            if ((sectionNumber >= minimumSection) && (sectionNumber <= maximumSection)) {
               const int numPoints = contour->getNumberOfPoints();
               const float z = sectionNumber * spacing;
               
               glPushName(i);
               for (int j = 0; j < numPoints; j++) {
                  float x, y;
                  contour->getPointXY(j, x, y);
                  glPushName(j);
                  glBegin(GL_POINTS);
                     glVertex3f(x, y, z);
                  glEnd();
                  glPopName();
               }
               glPopName();
            }
         }
         glPopName();
      }
      if (selectionMask & SELECTION_MASK_CONTOUR_CELL) {
         if (dsc->getDisplayContourCells()) {
            glPushName(SELECTION_MASK_CONTOUR_CELL);
            for (int i = 0; i < numCells; i++) {
               const CellData* cd = cellFile->getCell(i);
               if (cd->getDisplayFlag() == false) {
                  continue;
               }
               
               const int sectionNumber = cd->getSectionNumber();
               if ((sectionNumber >= minimumSection) && (sectionNumber <= maximumSection)) {
                  const int colorIndex = cd->getColorIndex();
                  float pointSize = 1;
                  float lineSize  = 1;
                  if ((colorIndex >= 0) && (colorIndex < numColors)) {
                     colorFile->getPointLineSizeByIndex(colorIndex, pointSize, lineSize);
                  }
                  
                  if (pointSize < 1) {
                     pointSize = 1;
                  }
                  
                  const float size = pointSize * cellSize;
                  float xyz[3];
                  cd->getXYZ(xyz);
                  xyz[2] = sectionNumber * spacing;
                  
                  glPushName(i);
                  glPointSize(getValidPointSize(size));
                  glBegin(GL_POINTS);
                     glVertex3fv(xyz);
                  glEnd();
                  glPopName();
               }
            }
            glPopName();
         }
      }
   
      if (selectionMask == SELECTION_MASK_OFF) {
         bool drawPoints = false;
         bool drawLines  = false;
         switch (dsc->getDrawMode()) {
            case DisplaySettingsContours::DRAW_MODE_POINTS:
               drawPoints = true;
               break;
            case DisplaySettingsContours::DRAW_MODE_LINES:
               drawLines = true;
               break;
            case DisplaySettingsContours::DRAW_MODE_POINTS_AND_LINES:
               drawPoints = true;
               drawLines  = true;
               break;
         }
         
         if (drawLines) {
            for (int i = 0; i < numContours; i++) {
               const CaretContour* contour = cf->getContour(i);
               const int sectionNumber = contour->getSectionNumber();
               if ((sectionNumber >= minimumSection) && (sectionNumber <= maximumSection)) {
                  //
                  // Don't draw alignment section number
                  //
                  if (aligningContours && (sectionNumber == alignmentSectionNumber)) {
                     continue;
                  }
                  
                  const float z = sectionNumber * spacing;
                  
                  const int numPoints = contour->getNumberOfPoints();
                  
                  if (numPoints > 1) {
                     glColor3ub(0, 125, 0);
                     glBegin(GL_LINE_LOOP);
                        for (int j = 0; j < numPoints; j++) {
                           float x, y;
                           contour->getPointXY(j, x, y);
                           glVertex3f(x, y, z);
                        }
                     glEnd();
                  }
               }
            }
         }
         
         if (drawPoints) {
            for (int i = 0; i < numContours; i++) {
               const CaretContour* contour = cf->getContour(i);
               const int sectionNumber = contour->getSectionNumber();
               if ((sectionNumber >= minimumSection) && (sectionNumber <= maximumSection)) {
                  //
                  // Don't draw alignment section number
                  //
                  if (aligningContours && (sectionNumber == alignmentSectionNumber)) {
                     continue;
                  }
                  
                  const float z = sectionNumber * spacing;
                  
                  const int numPoints = contour->getNumberOfPoints();
                  if (numPoints > 0) {
                     glPointSize(getValidPointSize(dsc->getPointSize()));
                     glColor3ub(0, 175, 0);
                     glBegin(GL_POINTS);
                        for (int j = 0; j < numPoints; j++) {
                           if ((j == 0) && (dsc->getShowEndPoints())) {
                              continue; // skip as drawn later
                           }
                           else {
                              float x, y;
                              contour->getPointXY(j, x, y);
                              glVertex3f(x, y, z);
                           }
                        }
                     glEnd();
                     
                     //
                     // Draw highlighted contours
                     //
                     glPointSize(getValidPointSize(dsc->getPointSize() * 2.0));
                     glColor3ub(0, 255, 0);
                     glBegin(GL_POINTS);
                        for (int j = 0; j < numPoints; j++) {
                           if ((j == 0) && (dsc->getShowEndPoints())) {
                              continue; // skip as drawn later
                           }
                           else if (contour->getHighlightFlag(j)) {
                              float x, y;
                              contour->getPointXY(j, x, y);
                              glVertex3f(x, y, z);
                           }
                        }
                     glEnd();
                     
                     //
                     // Draw end points in red
                     //
                     if (dsc->getShowEndPoints()) {
                        glPointSize(getValidPointSize(dsc->getPointSize() * 2.0));
                        glColor3ub(255, 0, 0);
                        float x, y;
                        contour->getPointXY(0, x, y);
                        glBegin(GL_POINTS);
                           glVertex3f(x, y, z);
                        glEnd();
                     }
                  }
               }
            }
         }
         
         if (dsc->getDisplayContourCells()) {
            for (int i = 0; i < numCells; i++) {
               const CellData* cd = cellFile->getCell(i);
               if (cd->getDisplayFlag() == false) {
                  continue;
               }
               
               const int sectionNumber = cd->getSectionNumber();

               //
               // Don't draw alignment section number cells
               //
               if (aligningContours && (sectionNumber == alignmentSectionNumber)) {
                  continue;
               }

               if ((sectionNumber >= minimumSection) && (sectionNumber <= maximumSection)) {                     
                  const int colorIndex = cd->getColorIndex();
                  float pointSize = 1;
                  float lineSize  = 1;
                  unsigned char r = 0, g = 0, b = 0;
                  if ((colorIndex >= 0) && (colorIndex < numColors)) {
                     colorFile->getColorByIndex(colorIndex, r, g, b);
                     colorFile->getPointLineSizeByIndex(colorIndex, pointSize, lineSize);
                  }
                  else {
                     r = noColorRed;
                     g = noColorGreen;
                     b = noColorBlue;
                  }
                  
                  if (pointSize < 1) {
                     pointSize = 1;
                  }
                  
                  const float size = pointSize * cellSize;
                  float xyz[3];
                  cd->getXYZ(xyz);
                  xyz[2] = sectionNumber * spacing;
                  
                  glColor3ub(r, g, b);
                  glPointSize(getValidPointSize(size));
                  glBegin(GL_POINTS);
                     glVertex3fv(xyz);
                  glEnd();
               }
            }
         }
      }
   } // if (hideContours == false) ...
   
   //
   // Draw alignment section
   //
   if (aligningContours) {
      drawModelContoursAlignment(bmc, alignmentSectionNumber);
   }
}

/**
 * Draw the contours that are being aligned.
 */
void
BrainModelOpenGL::drawModelContoursAlignment(BrainModelContours* bmc,
                                                const int alignmentSectionNumber)
{
   //
   // Display alignment region box
   //
   if (bmc->getAligningRegionFlag()) {
      if (bmc->getAlignmentRegionBoxValid()) {
         float bounds[4];
         bmc->getAlignmentRegionBox(bounds);
         const float minX = std::min(bounds[0], bounds[2]);
         const float maxX = std::max(bounds[0], bounds[2]);
         const float minY = std::min(bounds[1], bounds[3]);
         const float maxY = std::max(bounds[1], bounds[3]);
         glColor3ub(125, 0, 0);
         glBegin(GL_LINE_LOOP);
            glVertex3f(minX, minY, -1.0);
            glVertex3f(maxX, minY, -1.0);
            glVertex3f(maxX, maxY, -1.0);
            glVertex3f(minX, maxY, -1.0);
         glEnd();
      }
   }

   const ContourFile* cf = bmc->getContourFile();
   
   //
   // Create the alignment transformation matrix
   //
   glPushMatrix();
   glLoadIdentity();
   
   // Get the center of gravity of the contour section and translate to it
   // so that contour rotates around its center of gravity and not
   // the origin
   //
   float cogX, cogY;
   cf->getSectionCOG(alignmentSectionNumber, cogX, cogY);
   glTranslatef(cogX, cogY, 0.0);
   
   float translate[3];
   bmc->getAlignmentTranslation(translate);
   glTranslatef(translate[0], translate[1], translate[2]);
   
   float matrix[16];
   bmc->getAlignmentRotationMatrix(matrix);
   glMultMatrixf(matrix);
   
   const float scale = bmc->getAlignmentScaling();
   glScalef(scale, scale, 1.0);
   
   //
   // Undo the Center of Gravity Translation
   //
   glTranslatef(-cogX, -cogY, 0.0);
   
   //
   // Get the alignment transformation matrix
   //
   double alignmentMatrix[16]; //[4][4];
   glGetDoublev(GL_MODELVIEW_MATRIX, (double*)alignmentMatrix);
   
   glPopMatrix();
   
   //
   // Used when contours are aligned
   //
   TransformationMatrix am;
   am.setMatrix(alignmentMatrix);
   bmc->setAlignmentMatrix(am);
   
/*  !!!!!!!!!!!! NEED TO FINISH !!!!!!!!!!!!!!!!!!!!!!!!!!
   //
   // Copy the matrix to the alignment dialog
   //
   GuiContourAlignmentDialog* cad = theMainWindow->getContourAlignmentDialog(false);
   if (cad != NULL) {
      cad->setAlignmentTransformationMatrix(alignmentMatrix);
   }
*/
   
   const int numContours = cf->getNumberOfContours();
   const float spacing = cf->getSectionSpacing();
   
   const DisplaySettingsContours* dsc = brainSet->getDisplaySettingsContours();
   
   glPointSize(getValidPointSize(dsc->getPointSize()));
   glLineWidth(getValidLineWidth(dsc->getLineThickness()));
   
   //
   // First iteration draw region being aligned
   // Second iteration draw what is not in the region (or everything if no region)
   //
   const bool regionAlignment = bmc->getAligningRegionFlag();
   for (int iter = 0; iter < 2; iter++) {
      //
      // Use the alignment matrix
      //
      glPushMatrix();
      if (iter == 0) {
         if (regionAlignment == false) {
            glPopMatrix();
            continue;
         }
         glMultMatrixd((double*)alignmentMatrix);
      }
      else {
         if (regionAlignment == false) {
            glMultMatrixd((double*)alignmentMatrix);
         }
      }
      
      for (int i = 0; i < numContours; i++) {
         const CaretContour* contour = cf->getContour(i);
         const int sectionNumber = contour->getSectionNumber();
         if (sectionNumber == alignmentSectionNumber) {
            const float z = sectionNumber * spacing;
            
            const int numPoints = contour->getNumberOfPoints();
            
            glBegin(GL_POINTS);
               for (int j = 0; j < numPoints; j++) {
                  glColor3ub(0, 0, 255);
                  bool drawIt = false;
                  if (iter == 0) {
                     if (contour->getSpecialFlag(j)) {
                        drawIt = true;
                     }
                  }
                  else {
                     if (regionAlignment) {
                        if (contour->getSpecialFlag(j) == false) {
                           drawIt = true;
                           glColor3ub(0, 0, 150);
                        }
                     }
                     else {
                        drawIt = true;
                     }
                  }
                  
                  if (drawIt) {
                     float x, y;
                     contour->getPointXY(j, x, y);
                     glVertex3f(x, y, z);
                  }
               }
            glEnd();
         }
      }
   
      if (dsc->getDisplayContourCells()) {
         CellFile* cellFile = brainSet->getContourCellFile();
         const int numCells = cellFile->getNumberOfCells();
         for (int i = 0; i < numCells; i++) {
            const CellData* cd = cellFile->getCell(i);
            if (cd->getDisplayFlag() == false) {
               continue;
            }
            
            const int sectionNumber = cd->getSectionNumber();
            if (sectionNumber == alignmentSectionNumber) {                     
               float pointSize = 6;
               
               const float size = pointSize;
               float xyz[3];
               cd->getXYZ(xyz);
               xyz[2] = sectionNumber * spacing;
               
               glColor3ub(0, 0, 255);
               bool drawIt = false;
               if (iter == 0) {
                  if (cd->getSpecialFlag()) {
                     drawIt = true;
                  }
               }
               else {
                  if (regionAlignment) {
                     if (cd->getSpecialFlag() == false) {
                        drawIt = true;
                        glColor3ub(0, 0, 150);
                     }
                  }
                  else {
                     drawIt = true;
                  }
               }
               
               if (drawIt) {
                  glPointSize(getValidPointSize(size));
                  glBegin(GL_POINTS);
                     glVertex3fv(xyz);
                  glEnd();
               }
            }
         }
      }
      
      glPopMatrix();   
   }
}

/**
 * get the default ortho right and top for command line scene generation.
 */
void 
BrainModelOpenGL::getDefaultOrthoRightAndTop(const int windowWidthIn,
                                             const int windowHeightIn,
                                             double& orthoRightOut,
                                             double& orthoTopOut)
{
   const double aspectRatio = (static_cast<double>(windowWidthIn)) /
                              (static_cast<double>(windowHeightIn));
   orthoRightOut = defaultOrthoWindowSize * aspectRatio;
   orthoTopOut   = defaultOrthoWindowSize;
}
                                             
/**
 * Update the orthographic window size.
 */
void
BrainModelOpenGL::updateOrthoSize(const int windowNumber, const int width, const int height)
{
   const double aspectRatio = (static_cast<double>(width)) /
                              (static_cast<double>(height));     
   orthographicRight[windowNumber]  =    defaultOrthoWindowSize * aspectRatio;
   orthographicLeft[windowNumber]   =   -defaultOrthoWindowSize * aspectRatio;
   orthographicTop[windowNumber]    =    defaultOrthoWindowSize;
   orthographicBottom[windowNumber] =   -defaultOrthoWindowSize;
   orthographicNear[windowNumber]   = -5000.0; //-500.0; //-10000.0;
   orthographicFar[windowNumber]    =  5000.0; //500.0; // 10000.0;   
}

/**
 * draw surface.
 */
void 
BrainModelOpenGL::drawBrainModelSurface(BrainModelSurface* bms,
                                        BrainModelSurfaceAndVolume* bmsv,
                                        const bool drawTheSurface,
                                        const bool surfaceInVolumeAllViewFlag)
{
   if (DebugControl::getOpenGLDebugOn()) {
      checkForOpenGLError(bms, "At beginning of drawBrainModelSurface()");
   }
   
   const DisplaySettingsSurface* dss = brainSet->getDisplaySettingsSurface();

   //
   // Don't set projection matrix when selecting
   //
   if (selectionMask == SELECTION_MASK_OFF) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      const double aspectRatio = (static_cast<double>(viewport[2])) /
                                 (static_cast<double>(viewport[3]));     
                                 
      switch (dss->getViewingProjection()) {
         case DisplaySettingsSurface::VIEWING_PROJECTION_ORTHOGRAPHIC:
            glOrtho(orthographicLeft[viewingWindowNumber], orthographicRight[viewingWindowNumber], 
                    orthographicBottom[viewingWindowNumber], orthographicTop[viewingWindowNumber], 
                    orthographicNear[viewingWindowNumber], orthographicFar[viewingWindowNumber]);
            break;
         case DisplaySettingsSurface::VIEWING_PROJECTION_PERSPECTIVE:
            gluPerspective(bms->getPerspectiveFieldOfView(),
                           aspectRatio,
                           1.0, 1000.0);
            break;
      }
      glGetDoublev(GL_PROJECTION_MATRIX, selectionProjectionMatrix[viewingWindowNumber]);
   }

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   
   switch (dss->getViewingProjection()) {
         case DisplaySettingsSurface::VIEWING_PROJECTION_ORTHOGRAPHIC:
            break;
         case DisplaySettingsSurface::VIEWING_PROJECTION_PERSPECTIVE:
            gluLookAt(0.0, 0.0, bms->getPerspectiveZooming(viewingWindowNumber), 
                      0.0, 0.0,0.0, 0.0,1.0, 0.0);
            break;
   }
   
   float translate[3];
   bms->getTranslation(viewingWindowNumber, translate);
   glTranslatef(translate[0], translate[1], translate[2]);
   
   float matrix[16];
   bms->getRotationMatrix(viewingWindowNumber, matrix);
   glMultMatrixf(matrix);
   
   if (DebugControl::getDebugOn()) {
      vtkTransform* tm = bms->getRotationTransformMatrix(viewingWindowNumber);
      float angles[3];
      tm->GetOrientation(angles);
     // std::cout << "angles: " 
     //          << angles[0] << " "
     //          << angles[1] << " "
     //          << angles[2] << std::endl;
   }

   float scale[3];
   bms->getScaling(viewingWindowNumber, scale);
   glScalef(scale[0], scale[1], scale[2]);
   
   //
   // Save modeling matrix
   //
   bool idMode = true;
   if (selectionMask == SELECTION_MASK_OFF) {
      idMode = false;
      glGetDoublev(GL_MODELVIEW_MATRIX, selectionModelviewMatrix[viewingWindowNumber]);
   }
   
   //
   // If this flag is set we only want to draw the linear object.  When this
   // flag is set, glClear() is not called so the previous stuff is not erased.
   //
   if (drawLinearObjectOnly && (bmsv == NULL)) {
      drawLinearObject();
      return;
   }

   //
   // Allow clipping planes
   //
   enableSurfaceClippingPlanes(bms);

   const CoordinateFile* cf = bms->getCoordinateFile();
   const int modelNumber = bms->getBrainModelIndex();
   const int numCoords = cf->getNumberOfCoordinates();
   
   switch (bms->getSurfaceType()) {
      case BrainModelSurface::SURFACE_TYPE_RAW:
      case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
         //
         // Draw the VTK models
         //
         drawAllVtkModels();
         break;
      case BrainModelSurface::SURFACE_TYPE_INFLATED:
      case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
      case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
      case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
      case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
      case BrainModelSurface::SURFACE_TYPE_FLAT:
      case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
      case BrainModelSurface::SURFACE_TYPE_HULL:
      case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
      case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
         break;
   }

   //
   // Draw cells and foci here so that they show through a transparent surface
   //
   drawCellAndFociProjections(bms);

   //
   // Get display list number for this surface
   //  
   PreferencesFile* pf = brainSet->getPreferencesFile();
   const bool displayListsEnabled = pf->getDisplayListsEnabled();
   unsigned long displayListNumber = bms->getDisplayListNumber();
   
   //
   // If need to redraw surface
   //
   if ((displayListNumber == 0) ||
       (displayListsEnabled == false) ||
       idMode) {
      //
      // If not ID mode then need to create a new display list for this surface
      //
      bool creatingDisplayList = false;
      if (displayListsEnabled && (idMode == false)) {
         displayListNumber = glGenLists(1);
         bms->setDisplayListNumber(displayListNumber);
         glNewList(displayListNumber, GL_COMPILE);
         creatingDisplayList = true;
      }
      
      //
      // Draw vectors here so surface opacity functions successfully.
      // Otherwise Z-Buffer prevents vectors from being drawn inside the surface.
      //
      if (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL) {
         this->disableSurfaceClippingPlanes();
         drawVectorFile3D(bms);
         this->enableSurfaceClippingPlanes(bms);
      }

      int numTiles = -1;
      TopologyFile* tf = bms->getTopologyFile();
      if (tf != NULL) {
         numTiles = tf->getNumberOfTiles();
      }
      
      const DisplaySettingsSurface::DRAW_MODE surfaceDrawingMode = dss->getDrawMode();
      
      if ((numCoords > 0) && drawTheSurface) {
         glColor3ub(170, 170, 170);
         BrainModelSurfaceNodeColoring* bsnc = brainSet->getNodeColoring();
         if ((selectionMask & SELECTION_MASK_NODE) ||
             (selectionMask & SELECTION_MASK_TILE) ||
             (selectionMask & SELECTION_MASK_LINK)) {
            if (surfaceDrawingMode != DisplaySettingsSurface::DRAW_MODE_NONE) {
               if (selectionMask & SELECTION_MASK_NODE) {
                  drawSurfaceNodes(bsnc, modelNumber, cf, numCoords, false);
               }
               if (selectionMask & SELECTION_MASK_TILE) {
                  if (numTiles > 0) {
                     if (surfaceDrawingMode != DisplaySettingsSurface::DRAW_MODE_NONE) {
                        drawSurfaceTiles(bsnc, bms, cf, tf, numTiles, numCoords);
                     }
                  }
               }
               if (selectionMask & SELECTION_MASK_LINK) {
                  if ((surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_LINKS) || 
                      (surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_NODES_AND_LINKS) ||
                      (surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_LINK_HIDDEN_LINE_REMOVAL) ||
                      (surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_TILES_LINKS_NODES)) {
                     drawSurfaceLinks(bsnc, modelNumber, cf, tf, numTiles, false, false);
                  }
                  
                  if (surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_LINKS_EDGES_ONLY) {
                     brainSet->classifyNodes(NULL, true);
                     drawSurfaceLinks(bsnc, modelNumber, cf, tf, numTiles, true, false);
                  }
               }
            }
         }
         else if (selectionMask == SELECTION_MASK_OFF) {
            if (numTiles > 0) {
               if ((surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_TILES) || 
                   (surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_TILES_WITH_LIGHT)) {
                  drawSurfaceTiles(bsnc, bms, cf, tf, numTiles, numCoords);
               }
               if ((surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_TILES_WITH_LIGHT_NO_BACK) ||
                   (surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_LINK_HIDDEN_LINE_REMOVAL)) {
                  glEnable(GL_CULL_FACE);
                  glCullFace(GL_BACK);
                  if (surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_LINK_HIDDEN_LINE_REMOVAL) {
                     glLineWidth(getValidLineWidth(dss->getLinkSize()));
                     glPolygonMode(GL_FRONT, GL_LINE);
                  }
                  drawSurfaceTiles(bsnc, bms, cf, tf, numTiles, numCoords);
                  glPolygonMode(GL_FRONT, GL_FILL);
                  glDisable(GL_CULL_FACE);
               }
               
               if ((surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_LINKS) || 
                   (surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_NODES_AND_LINKS)) {
                  drawSurfaceLinks(bsnc, modelNumber, cf, tf, numTiles, false, false);
               }
               
               if (surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_LINKS_EDGES_ONLY) {
                  brainSet->classifyNodes(NULL, true);
                  drawSurfaceLinks(bsnc, modelNumber, cf, tf, numTiles, true, false);
               }
               
               //if (surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_LINK_HIDDEN_LINE_REMOVAL) {
               //   drawSurfaceLinksNoBackside(bsnc, modelNumber, cf, tf, numTiles);
               //}
            }
            
            if (surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_TILES_LINKS_NODES) {
               if (numTiles > 0) {
                  glPolygonOffset(1.0, 1.0);
                  glEnable(GL_POLYGON_OFFSET_FILL);
                  drawSurfaceTiles(bsnc, bms, cf, tf, numTiles, numCoords);
                  glDisable(GL_POLYGON_OFFSET_FILL);
                  drawSurfaceLinks(bsnc, modelNumber, cf, tf, numTiles, false, true);
               }
               drawSurfaceNodes(bsnc, modelNumber, cf, numCoords, true);
            }
            
            if ((surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_NODES) || 
                (surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_NODES_AND_LINKS)) {
               drawSurfaceNodes(bsnc, modelNumber, cf, numCoords, false);
            }
            
            //
            // Push node ID symbols above a flat surface
            //
            glPushMatrix();
               if ((bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FLAT) ||
                  (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR)) {
                  glTranslatef(0.0, 0.0, 0.5);
               }
               drawSurfaceROIMembers(bms, numCoords);
               drawNodeHighlighting(bms, numCoords);
            glPopMatrix();
            
            if (dss->getShowNormals()) {
               drawSurfaceNormals(bms, cf, numCoords);
            }
            drawSurfaceForces(cf, numCoords);
         }
      }
      
      //
      // End display list if creating a display list
      //
      if (creatingDisplayList) {
         glEndList();
      }
   }   
   
   //
   // If display lists enabled, execute it
   //
   if (displayListsEnabled && (idMode == false)) {
      if (glIsList(displayListNumber)) {
         glCallList(displayListNumber);
      }
      else {
         std::cout << "PROGRAM WARNING: display list " << displayListNumber 
                   << " does not exist for surface " << bms->getDescriptiveName().toAscii().constData() 
                   << " in window " << viewingWindowNumber << std::endl;
      }
   }

   drawBorders(bms);

   // djs draw electrode trajectory...
   drawElectrodeTrajectory(bms);

   //drawCellAndFociProjections(bms);

   drawCuts();
   
   drawGeodesicPath(cf);
   
   drawDeformationFieldVectors(bms);

   drawSurfaceAxes(bms);
   
   drawTransformationMatrixAxes(bms);
      
/*
   switch (bms->getSurfaceType()) {
      case BrainModelSurface::SURFACE_TYPE_RAW:
      case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
         //
         // Draw the VTK models
         //
         drawAllVtkModels();
         break;
      case BrainModelSurface::SURFACE_TYPE_INFLATED:
      case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
      case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
      case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
      case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
      case BrainModelSurface::SURFACE_TYPE_FLAT:
      case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
      case BrainModelSurface::SURFACE_TYPE_HULL:
      case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
      case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
         break;
   }
*/

   if ((bmsv == NULL) && (surfaceInVolumeAllViewFlag == false)) {
      drawMetricPalette(modelNumber, true);
      drawShapePalette(modelNumber);
      
      const int num = linearObjectBeingDrawn.getNumberOfLinks();
      if (num > 0) {
         glDisable(GL_DEPTH_TEST);
         glPointSize(getValidPointSize(2.0));
         glColor3f(1.0, 0.0, 0.0);
         glBegin(GL_POINTS);
            for (int i = 0; i < num; i++) {
               glVertex3fv(linearObjectBeingDrawn.getLinkXYZ(i));
            }
         glEnd();
         glEnable(GL_DEPTH_TEST);
      }
   }

   disableSurfaceClippingPlanes();

   if (DebugControl::getOpenGLDebugOn()) {
      checkForOpenGLError(bms, "At end of drawBrainModelSurface()");
   }
}

/**
 * Enable the surface clipping planes.
 */
void
BrainModelOpenGL::enableSurfaceClippingPlanes(BrainModelSurface* bms)
{
   //
   // Setup clipping planes
   //
   DisplaySettingsSurface* dss = brainSet->getDisplaySettingsSurface();
   bool applyClippingPlanesFlag = false;
   switch (dss->getClippingPlaneApplication()) {
      case DisplaySettingsSurface::CLIPPING_PLANE_APPLICATION_MAIN_WINDOW_ONLY:
         if (viewingWindowNumber == BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW) {
            applyClippingPlanesFlag = true;
         }
         break;
      case DisplaySettingsSurface::CLIPPING_PLANE_APPLICATION_FIDUCIAL_SURFACES_ONLY:
         if (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL) {
            applyClippingPlanesFlag = true;
         }
         break;
      case DisplaySettingsSurface::CLIPPING_PLANE_APPLICATION_ALL_SURFACES:
         applyClippingPlanesFlag = true;
         break;
   }
   if (applyClippingPlanesFlag) {
      //
      // Negative X
      //
      if (dss->getClippingPlaneEnabled(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_X_NEGATIVE)) {
         GLdouble plane[4] = {
            1.0,
            0.0,
            0.0,
            -dss->getClippingPlaneCoordinate(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_X_NEGATIVE)
         };
         glClipPlane(GL_CLIP_PLANE0, plane);
         glEnable(GL_CLIP_PLANE0);
      }

      //
      // Positive X
      //
      if (dss->getClippingPlaneEnabled(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_X_POSITIVE)) {
         GLdouble plane[4] = {
            -1.0,
            0.0,
            0.0,
            dss->getClippingPlaneCoordinate(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_X_POSITIVE)
         };
         glClipPlane(GL_CLIP_PLANE1, plane);
         glEnable(GL_CLIP_PLANE1);
      }

      //
      // Negative Y
      //
      if (dss->getClippingPlaneEnabled(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Y_NEGATIVE)) {
         GLdouble plane[4] = {
            0.0,
            1.0,
            0.0,
            -dss->getClippingPlaneCoordinate(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Y_NEGATIVE)
         };
         glClipPlane(GL_CLIP_PLANE2, plane);
         glEnable(GL_CLIP_PLANE2);
      }

      //
      // Positive Y
      //
      if (dss->getClippingPlaneEnabled(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Y_POSITIVE)) {
         GLdouble plane[4] = {
            0.0,
            -1.0,
            0.0,
            dss->getClippingPlaneCoordinate(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Y_POSITIVE)
         };
         glClipPlane(GL_CLIP_PLANE3, plane);
         glEnable(GL_CLIP_PLANE3);
      }

      //
      // Negative Z
      //
      if (dss->getClippingPlaneEnabled(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Z_NEGATIVE)) {
         GLdouble plane[4] = {
            0.0,
            0.0,
            1.0,
            -dss->getClippingPlaneCoordinate(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Z_NEGATIVE)
         };
         glClipPlane(GL_CLIP_PLANE4, plane);
         glEnable(GL_CLIP_PLANE4);
      }

      //
      // Positive Z
      //
      if (dss->getClippingPlaneEnabled(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Z_POSITIVE)) {
         GLdouble plane[4] = {
            0.0,
            0.0,
            -1.0,
            dss->getClippingPlaneCoordinate(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Z_POSITIVE)
         };
         glClipPlane(GL_CLIP_PLANE5, plane);
         glEnable(GL_CLIP_PLANE5);
      }
   }
}

/**
 * Disable the surface clipping planes.
 */
void
BrainModelOpenGL::disableSurfaceClippingPlanes()
{
   glDisable(GL_CLIP_PLANE0);
   glDisable(GL_CLIP_PLANE1);
   glDisable(GL_CLIP_PLANE2);
   glDisable(GL_CLIP_PLANE3);
   glDisable(GL_CLIP_PLANE4);
   glDisable(GL_CLIP_PLANE5);
}

/**
 * Draw the surface axes.
 */
void 
BrainModelOpenGL::drawSurfaceAxes(const BrainModelSurface* bms)
{
   if (selectionMask != SELECTION_MASK_OFF) {
      return;
   }

   DisplaySettingsSurface* dss = brainSet->getDisplaySettingsSurface();
   bool showAxes, showLetters, showHashMarks;
   float axesLength;
   float axesOffset[3];
   dss->getSurfaceAxesInfo(showAxes, showLetters, showHashMarks, axesLength, axesOffset);

   if (showAxes) {
      //
      // Get the foreground color
      //
      PreferencesFile* pref = brainSet->getPreferencesFile();
      unsigned char r, g, b;
      pref->getSurfaceForegroundColor(r, g, b);
      glColor3ub(r, g, b);
      glLineWidth(getValidLineWidth(2.0));
      
      //
      // See if flat surface
      //
      bool flatSurfaceFlag = false;
      switch (bms->getSurfaceType()) {
         case BrainModelSurface::SURFACE_TYPE_RAW:
         case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
         case BrainModelSurface::SURFACE_TYPE_INFLATED:
         case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
         case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
         case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
         case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
            break;
         case BrainModelSurface::SURFACE_TYPE_FLAT:
         case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
            flatSurfaceFlag = true;
            break;
         case BrainModelSurface::SURFACE_TYPE_HULL:
         case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
         case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
            break;
      }
      
      glPushMatrix();
      {
         glTranslatef(axesOffset[0], axesOffset[1], axesOffset[2]);
         
         //
         // Draw the axes
         //
         glBegin(GL_LINES);
            glVertex3f(-axesLength, 0.0, 0.0);
            glVertex3f( axesLength, 0.0, 0.0);
            glVertex3f(0.0, -axesLength, 0.0);
            glVertex3f(0.0,  axesLength, 0.0);
            if (flatSurfaceFlag == false) {
               glVertex3f(0.0, 0.0, -axesLength);
               glVertex3f(0.0, 0.0,  axesLength);
            }
         glEnd();
         
         if (showHashMarks) {
            const float hashStep = 10.0;
            const float hashSize = 10.0;
            
            if (flatSurfaceFlag == false) {
               //
               // Along Z-axis with hashmarks in +/- X/Y
               //
               glBegin(GL_LINES);
                  for (float z = -axesLength; z <= axesLength; z+= hashStep) {
                     glVertex3f(-hashSize, 0.0, z);
                     glVertex3f( hashSize, 0.0, z);
                     
                     glVertex3f(0.0, -hashSize, z);
                     glVertex3f(0.0,  hashSize, z);
                  }
                  glVertex3f(-hashSize, 0.0, axesLength);
                  glVertex3f( hashSize, 0.0, axesLength);
                  
                  glVertex3f(0.0, -hashSize, axesLength);
                  glVertex3f(0.0,  hashSize, axesLength);
               glEnd();
            }
            
            //
            // Along Y-axis with hashmarks in +/- X
            //
            glBegin(GL_LINES);
               for (float y = -axesLength; y <= axesLength; y+= hashStep) {
                  glVertex3f(-hashSize, y, 0.0);
                  glVertex3f( hashSize, y, 0.0);

                  glVertex3f(0.0, y, -hashSize);
                  glVertex3f(0.0, y,  hashSize);
               }
               glVertex3f(-hashSize, axesLength, 0.0);
               glVertex3f( hashSize, axesLength, 0.0);

               glVertex3f(0.0, axesLength, -hashSize);
               glVertex3f(0.0, axesLength,  hashSize);
            glEnd();
            
            //
            // Along X-axis with hashmarks in +/- Y
            //
            glBegin(GL_LINES);
               for (float x = -axesLength; x <= axesLength; x+= hashStep) {
                  glVertex3f(x, -hashSize, 0.0);
                  glVertex3f(x,  hashSize, 0.0);

                  glVertex3f(x, 0.0, -hashSize);
                  glVertex3f(x, 0.0,  hashSize);
               }
               glVertex3f(axesLength, -hashSize, 0.0);
               glVertex3f(axesLength,  hashSize, 0.0);

               glVertex3f(axesLength, 0.0, -hashSize);
               glVertex3f(axesLength, 0.0,  hashSize);
            glEnd();
         }
         
         if (showLetters && (glWidget != NULL) && (flatSurfaceFlag == false)) {
            const float letterOffset = 10.0;
            glWidget->renderText(0.0, 0.0, -axesLength - letterOffset, "I");
            glWidget->renderText(0.0, 0.0,  axesLength + letterOffset, "S");
            glWidget->renderText(0.0, -axesLength - letterOffset, 0.0, "P");
            glWidget->renderText(0.0,  axesLength + letterOffset, 0.0, "A");
            if (bms->getStructure() == Structure::STRUCTURE_TYPE_CORTEX_RIGHT) {
               glWidget->renderText(-axesLength - letterOffset, 0.0, 0.0, "M");
               glWidget->renderText(axesLength + letterOffset, 0.0, 0.0, "L");
            }
            else {
               glWidget->renderText(-axesLength - letterOffset, 0.0, 0.0, "L");
               glWidget->renderText(axesLength + letterOffset, 0.0, 0.0, "M");
            }
         }
      }
      glPopMatrix();
      
   }
}

/**
 * Draw the transformation axes.
 */
void 
BrainModelOpenGL::drawTransformationMatrixAxes(const BrainModel* bm)
{
   //
   // Get the transformation matrix file and the number of matrices in it
   //
   TransformationMatrixFile* tmf = brainSet->getTransformationMatrixFile();
   const int numMatrices = tmf->getNumberOfMatrices();
   if (numMatrices <= 0) {
      return;
   }

   //
   // See if Translation axes should be drawn
   //
   bool drawAxes = false;
   bool volumeFlag = false;
   const BrainModelSurface* bms = dynamic_cast<const BrainModelSurface*>(bm);
   if (bms != NULL) {
      switch (bms->getSurfaceType()) {
         case BrainModelSurface::SURFACE_TYPE_RAW:
         case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
            drawAxes = true;
            break;
         case BrainModelSurface::SURFACE_TYPE_INFLATED:
         case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
         case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
         case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
         case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         case BrainModelSurface::SURFACE_TYPE_FLAT:
         case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
         case BrainModelSurface::SURFACE_TYPE_HULL:
         case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
         case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
            break;
      }
   }
   if (dynamic_cast<const BrainModelVolume*>(bm) != NULL) {
      drawAxes = true;
      volumeFlag = true;
   }
   if (dynamic_cast<const BrainModelSurfaceAndVolume*>(bm) != NULL) {
      drawAxes = true;
   }
   
   //
   // Check for selection being made
   //
   bool doingSelection = false;
   if (drawAxes) {
      if (selectionMask != SELECTION_MASK_OFF) {
         if (selectionMask & SELECTION_MASK_TRANSFORMATION_MATRIX_AXES) {
            doingSelection = true;
         }
         else {
            drawAxes = false;
         }
      }
   }
   
   //
   // Draw the translation axes
   //
   if (drawAxes) {
      for (int i = 0; i < numMatrices; i++) {
         const TransformationMatrix* tm = tmf->getTransformationMatrix(i);
         if (tm->getShowAxes()) {
            if (doingSelection) {
               glPushName(SELECTION_MASK_TRANSFORMATION_MATRIX_AXES);
               glPushName(i);
            }
            
            glPushMatrix();
            {
               //
               // Do the transformation
               //
               double matrix[16];
               tm->getMatrix(matrix);
               glMultMatrixd(matrix);
               
               const unsigned char xColor[3] = { 255,   0,   0 };
               const unsigned char yColor[3] = {   0, 255,   0 };
               const unsigned char zColor[3] = {   0,   0, 255 };

               float thickness = 2.0;
               if (tmf->getSelectedTransformationAxesIndex() == i) {
                  thickness = 4.0;
               }
               
               if (volumeFlag) {
                  //
                  // Draw the axes as OpenGL lines
                  //
                  const float axisEnd = tm->getAxesLength();
                  const float axisStart = -axisEnd * 0.10;
                  const float axesLength = tm->getAxesLength();
                  glLineWidth(getValidLineWidth(thickness));
                  glBegin(GL_LINES);
                     glColor3ubv(xColor);
                     glVertex3f(axisStart, 0.0, 0.0);
                     glVertex3f(axisEnd,   0.0, 0.0);
                     glColor3ubv(yColor);
                     glVertex3f(0.0, axisStart, 0.0);
                     glVertex3f(0.0, axisEnd,   0.0);
                     glColor3ubv(zColor);
                     glVertex3f(0.0, 0.0, axisStart);
                     glVertex3f(0.0, 0.0, axisEnd);
                  glEnd();
                  
                  if (doingSelection == false) {
                     const float letterPos = axesLength + 5;
                     glColor3ubv(xColor);
                     glWidget->renderText(letterPos, 0.0, 0.0, "X");
                     glColor3ubv(yColor);
                     glWidget->renderText(0.0, letterPos, 0.0, "Y");
                     glColor3ubv(zColor);
                     glWidget->renderText(0.0, 0.0, letterPos, "Z");
                  }
               }
               else {
                  //
                  // Draw the axes as cylinders
                  //
                  const float axesDiameter = thickness;
                  const float axesLength = tm->getAxesLength();
                  const float axisBehindOriginLength = axesLength * 0.10;
                  const float axisTotalLength = axesLength + axisBehindOriginLength;
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  glPushMatrix();
                     glColor3ubv(xColor);
                     glRotatef(90.0, 0.0, 1.0, 0.0);
                     glTranslatef(0.0, 0.0, -axisBehindOriginLength);
                     glPushMatrix();
                        glScalef(1.0, 1.0, -1.0);
                        drawDisk(axesDiameter);  // caps off end of cylinder
                     glPopMatrix();
                     glScalef(axesDiameter, axesDiameter, axisTotalLength);
                     drawCylinder();
                  glPopMatrix();
                  glPushMatrix();
                     glColor3ubv(yColor);
                     glRotatef(-90.0, 1.0, 0.0, 0.0);
                     glTranslatef(0.0, 0.0, -axisBehindOriginLength);
                     glPushMatrix();
                        glScalef(1.0, 1.0, -1.0);
                        drawDisk(axesDiameter);  // caps off end of cylinder
                     glPopMatrix();
                     glScalef(axesDiameter, axesDiameter, axisTotalLength);
                     drawCylinder();
                  glPopMatrix();
                  glPushMatrix();
                     glColor3ubv(zColor);
                     glTranslatef(0.0, 0.0, -axisBehindOriginLength);
                     glPushMatrix();
                        glScalef(1.0, 1.0, -1.0);
                        drawDisk(axesDiameter);  // caps off end of cylinder
                     glPopMatrix();
                     glScalef(axesDiameter, axesDiameter, axisTotalLength);
                     drawCylinder();
                  glPopMatrix();

                  //
                  // Draw the arrow heads
                  //
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  const float coneSize = axesDiameter + 5.0;
                  glPushMatrix();
                     glColor3ubv(xColor);
                     glTranslatef(axesLength, 0.0, 0.0);
                     glRotatef(90.0, 0.0, 1.0, 0.0);
                     glScalef(coneSize, coneSize, coneSize);
                     drawCone();
                  glPopMatrix();
                  glPushMatrix();
                     glColor3ubv(yColor);
                     glTranslatef(0.0, axesLength, 0.0);
                     glRotatef(-90.0, 1.0, 0.0, 0.0);
                     glScalef(coneSize, coneSize, coneSize);
                     drawCone();
                  glPopMatrix();
                  glPushMatrix();
                     glColor3ubv(zColor);
                     glTranslatef(0.0, 0.0, axesLength);
                     glScalef(coneSize, coneSize, coneSize);
                     drawCone();
                  glPopMatrix();
                  glDisable(GL_COLOR_MATERIAL);
                  glDisable(GL_LIGHTING);
                  
                  if (doingSelection == false) {
                     const float letterPos = axesLength + coneSize + 5;
                     glColor3ubv(xColor);
                     glWidget->renderText(letterPos, 0.0, 0.0, "X");
                     glColor3ubv(yColor);
                     glWidget->renderText(0.0, letterPos, 0.0, "Y");
                     glColor3ubv(zColor);
                     glWidget->renderText(0.0, 0.0, letterPos, "Z");
                  }
               }
            }
            glPopMatrix();
            
            if (doingSelection) {
               glPopName();
               glPopName();
            }
         } // if show axes
      } // for (i ==
   }  // if (drawAxes)

   for (int i = 0; i < numMatrices; i++) {
      const TransformationMatrix* tm = tmf->getTransformationMatrix(i);
      glPushMatrix();
      {
         //
         // Do the transformation
         //
         double matrix[16];
         tm->getMatrix(matrix);
         glMultMatrixd(matrix);
      
         //
         // Draw the data files associated with the axes
         //
         if (volumeFlag == false) {
            drawTransformationDataFiles(tm);
         }
      }
      glPopMatrix();
   }
}
      
/**
 * draw surface and volume.
 */
void 
BrainModelOpenGL::drawBrainModelSurfaceAndVolume(BrainModelSurfaceAndVolume* bmsv)
{
   //
   // Draw the surface
   //
   drawBrainModelSurface(bmsv, bmsv, bmsv->getDisplaySurface(), false);

   //
   // Make sure there is an anatomical volume
   //
   VolumeFile* anatomyVolume = bmsv->getAnatomyVolumeFile();
   if (anatomyVolume == NULL) {
      return;
   }

   //
   // Check for valid dimensions
   //
   int dim[3];
   anatomyVolume->getDimensions(dim);
   if ((dim[0] <= 0) || (dim[1] <= 0) || (dim[2] <= 0)) {
      return;
   }
   
   //
   // Make sure/update anatomy volume coloring
   //
   BrainModelVolumeVoxelColoring* voxelColoring = brainSet->getVoxelColoring();
   
   //
   // Get the selected slices
   //
   int slices[3];
   bmsv->getSelectedSlices(slices);
   
   //
   // Get the volume spacing
   //
   float spacing[3];
   anatomyVolume->getSpacing(spacing);
                                
   //
   // Get the origin
   //
   float originCenter[3], originCorner[3];
   anatomyVolume->getOrigin(originCenter);
   anatomyVolume->getOriginAtCornerOfVoxel(originCorner);

   //
   // Might be drawing secondary overlay on the slices
   //
   bool drawSecondaryDataOnSlices = false;
   VolumeFile* secondaryVolumeFile = bmsv->getOverlaySecondaryVolumeFile();
   if (bmsv->getDisplaySecondaryOverlayVolumeOnSlices()) {
      if (secondaryVolumeFile != NULL) {
         drawSecondaryDataOnSlices = true;
      }
   }

   //
   // Might be drawing primary overlay on the slices
   //
   bool drawPrimaryDataOnSlices = false;
   VolumeFile* primaryVolumeFile = bmsv->getOverlayPrimaryVolumeFile();
   if (bmsv->getDisplayPrimaryOverlayVolumeOnSlices()) {
      if (primaryVolumeFile != NULL) {
         drawPrimaryDataOnSlices = true;
      }
   }

   //
   // Draw the horizontal slice
   //
   if (bmsv->getDisplayHorizontalSlice()) {
      const float sliceZ = slices[2] * spacing[2] + originCenter[2];
      for (int i = 0; i < dim[0]; i++) {
         for (int j = 0; j < dim[1]; j++) {
            int ijk[3] = { i, j, slices[2] };
            unsigned char rgb[4];
            voxelColoring->getVoxelColoring(anatomyVolume, ijk[0], ijk[1], ijk[2], rgb);
            glColor3ubv(rgb);
            const float xCenter = i * spacing[0] + originCenter[0];
            const float yCenter = j * spacing[1] + originCenter[1];
            const float xCorner = i * spacing[0] + originCorner[0];
            const float yCorner = j * spacing[1] + originCorner[1];

            bool drawIt = false;
            if (rgb[3] == VolumeFile::VOXEL_COLOR_STATUS_VALID) {
               //
               // Avoid drawing black voxels
               //
               if ((rgb[0] != 0) || (rgb[1] != 0) || (rgb[2] != 0)) {
                  drawIt = true;
               }
               if (bmsv->getDrawAnatomyBlackVoxels()) {
                  drawIt = true;
               }
            }
            
            unsigned long maskForThisSlice = SELECTION_MASK_OFF;
            if (selectionMask & SELECTION_MASK_VOXEL_UNDERLAY) {
               maskForThisSlice = SELECTION_MASK_VOXEL_UNDERLAY;
            }
            
            if (drawSecondaryDataOnSlices) {
               float xyz[3] = { xCenter, yCenter, sliceZ };
               float p[3];
               if (secondaryVolumeFile->convertCoordinatesToVoxelIJK(xyz, ijk, p)!=0) {
                  voxelColoring->getVoxelColoring(secondaryVolumeFile, ijk[0], ijk[1], ijk[2], rgb);
                  if (rgb[3] == VolumeFile::VOXEL_COLOR_STATUS_VALID) {
                     glColor3ubv(rgb);
                     drawIt = true;
                     if (selectionMask & SELECTION_MASK_VOXEL_OVERLAY_SECONDARY) {
                        maskForThisSlice = SELECTION_MASK_VOXEL_OVERLAY_SECONDARY;
                     }
                  }
               }
            }
               
            if (drawPrimaryDataOnSlices) {
               float xyz[3] = { xCenter, yCenter, sliceZ };
               float p[3];
               if (primaryVolumeFile->convertCoordinatesToVoxelIJK(xyz, ijk, p)!=0) {
                  voxelColoring->getVoxelColoring(primaryVolumeFile, ijk[0], ijk[1], ijk[2], rgb);
                  if (rgb[3] == VolumeFile::VOXEL_COLOR_STATUS_VALID) {
                     glColor3ubv(rgb);
                     drawIt = true;
                     if (selectionMask & SELECTION_MASK_VOXEL_OVERLAY_PRIMARY) {
                        maskForThisSlice = SELECTION_MASK_VOXEL_OVERLAY_PRIMARY;
                     }
                  }
               }
            }
            
            if (drawIt) {
               if (maskForThisSlice != SELECTION_MASK_OFF) {
                  glPushName(maskForThisSlice);
                  glPushName(ijk[0]);
                  glPushName(ijk[1]);
                  glPushName(ijk[2]);
                  glPushName(VolumeFile::VOLUME_AXIS_Z);
               }
               
               glBegin(GL_QUADS);
                  glVertex3f(xCorner, yCorner, sliceZ);
                  glVertex3f(xCorner + spacing[0], yCorner, sliceZ);
                  glVertex3f(xCorner + spacing[0], yCorner + spacing[1], sliceZ);
                  glVertex3f(xCorner, yCorner + spacing[1], sliceZ);
               glEnd();
               
               if (maskForThisSlice != SELECTION_MASK_OFF) {
                  glPopName();
                  glPopName();
                  glPopName();
                  glPopName();
                  glPopName();
               }
            }
         }
      }
   }

   //
   // Draw the coronal slice
   //
   if (bmsv->getDisplayCoronalSlice()) {
      const float sliceY = slices[1] * spacing[1] + originCenter[1];
      for (int i = 0; i < dim[0]; i++) {
         for (int k = 0; k < dim[2]; k++) {
            int ijk[3] = { i, slices[1], k };
            unsigned char rgb[4];
            voxelColoring->getVoxelColoring(anatomyVolume, ijk[0], ijk[1], ijk[2], rgb);
            glColor3ubv(rgb);
            const float xCenter = i * spacing[0] + originCenter[0];
            const float zCenter = k * spacing[2] + originCenter[2];
            const float xCorner = i * spacing[0] + originCorner[0];
            const float zCorner = k * spacing[2] + originCorner[2];
            
            bool drawIt = false;
            if (rgb[3] == VolumeFile::VOXEL_COLOR_STATUS_VALID) {
               //
               // Avoid drawing black voxels
               //
               if ((rgb[0] != 0) || (rgb[1] != 0) || (rgb[2] != 0)) {
                  drawIt = true;
               }
               if (bmsv->getDrawAnatomyBlackVoxels()) {
                  drawIt = true;
               }
            }
            
            unsigned long maskForThisSlice = SELECTION_MASK_OFF;
            if (selectionMask & SELECTION_MASK_VOXEL_UNDERLAY) {
               maskForThisSlice = SELECTION_MASK_VOXEL_UNDERLAY;
            }
            
            if (drawSecondaryDataOnSlices) {
               float xyz[3] = { xCenter, sliceY, zCenter };
               float p[3];
               if (secondaryVolumeFile->convertCoordinatesToVoxelIJK(xyz, ijk, p)!=0) {
                  voxelColoring->getVoxelColoring(secondaryVolumeFile, ijk[0], ijk[1], ijk[2], rgb);
                  if (rgb[3] == VolumeFile::VOXEL_COLOR_STATUS_VALID) {
                     glColor3ubv(rgb);
                     drawIt = true;
                     if (selectionMask & SELECTION_MASK_VOXEL_OVERLAY_SECONDARY) {
                        maskForThisSlice = SELECTION_MASK_VOXEL_OVERLAY_SECONDARY;
                     }
                  }
               }
            }
            
            if (drawPrimaryDataOnSlices) {
               float xyz[3] = { xCenter, sliceY, zCenter };
               float p[3];
               if (primaryVolumeFile->convertCoordinatesToVoxelIJK(xyz, ijk, p)!=0) {
                  voxelColoring->getVoxelColoring(primaryVolumeFile, ijk[0], ijk[1], ijk[2], rgb);
                  if (rgb[3] == VolumeFile::VOXEL_COLOR_STATUS_VALID) {
                     glColor3ubv(rgb);
                     drawIt = true;
                     if (selectionMask & SELECTION_MASK_VOXEL_OVERLAY_PRIMARY) {
                        maskForThisSlice = SELECTION_MASK_VOXEL_OVERLAY_PRIMARY;
                     }
                  }
               }
            }
            
            if (drawIt) {
               if (maskForThisSlice != SELECTION_MASK_OFF) {
                  glPushName(maskForThisSlice);
                  glPushName(ijk[0]);
                  glPushName(ijk[1]);
                  glPushName(ijk[2]);
                  glPushName(VolumeFile::VOLUME_AXIS_Y);
               }                  
               glBegin(GL_QUADS);
                  glVertex3f(xCorner, sliceY, zCorner);
                  glVertex3f(xCorner + spacing[0], sliceY, zCorner);
                  glVertex3f(xCorner + spacing[0], sliceY, zCorner + spacing[2]);
                  glVertex3f(xCorner, sliceY, zCorner + spacing[2]);
               glEnd();
               
               if (maskForThisSlice != SELECTION_MASK_OFF) {
                  glPopName();
                  glPopName();
                  glPopName();
                  glPopName();
                  glPopName();
               }
            }
         }
      }
   }
   
   //
   // Draw the parasaggital slice
   //
   if (bmsv->getDisplayParasagittalSlice()) {
      const float sliceX = slices[0] * spacing[0] + originCenter[0];
      for (int j = 0; j < dim[1]; j++) {
         for (int k = 0; k < dim[2]; k++) {
            int ijk[3] = { slices[0], j, k };
            unsigned char rgb[4];
            voxelColoring->getVoxelColoring(anatomyVolume, ijk[0], ijk[1], ijk[2], rgb);
            glColor3ubv(rgb);
            const float yCenter = j * spacing[1] + originCenter[1];
            const float zCenter = k * spacing[2] + originCenter[2];
            const float yCorner = j * spacing[1] + originCorner[1];
            const float zCorner = k * spacing[2] + originCorner[2];
            
            bool drawIt = false;
            if (rgb[3] == VolumeFile::VOXEL_COLOR_STATUS_VALID) {
               //
               // Avoid drawing black voxels
               //
               if ((rgb[0] != 0) || (rgb[1] != 0) || (rgb[2] != 0)) {
                  drawIt = true;
               }
               if (bmsv->getDrawAnatomyBlackVoxels()) {
                  drawIt = true;
               }
            }
            unsigned long maskForThisSlice = SELECTION_MASK_OFF;
            if (selectionMask & SELECTION_MASK_VOXEL_UNDERLAY) {
               maskForThisSlice = SELECTION_MASK_VOXEL_UNDERLAY;
            }
            
            if (drawSecondaryDataOnSlices) {
               float xyz[3] = { sliceX, yCenter, zCenter };
               float p[3];
               if (secondaryVolumeFile->convertCoordinatesToVoxelIJK(xyz, ijk, p)!=0) {
                  voxelColoring->getVoxelColoring(secondaryVolumeFile, ijk[0], ijk[1], ijk[2], rgb);
                  if (rgb[3] == VolumeFile::VOXEL_COLOR_STATUS_VALID) {
                     glColor3ubv(rgb);
                     drawIt = true;
                     if (selectionMask & SELECTION_MASK_VOXEL_OVERLAY_SECONDARY) {
                        maskForThisSlice = SELECTION_MASK_VOXEL_OVERLAY_SECONDARY;
                     }
                  }
               }
            }
               
            if (drawPrimaryDataOnSlices) {
               float xyz[3] = { sliceX, yCenter, zCenter };
               float p[3];
               if (primaryVolumeFile->convertCoordinatesToVoxelIJK(xyz, ijk, p)!=0) {
                  voxelColoring->getVoxelColoring(primaryVolumeFile, ijk[0], ijk[1], ijk[2], rgb);
                  if (rgb[3] == VolumeFile::VOXEL_COLOR_STATUS_VALID) {
                     glColor3ubv(rgb);
                     drawIt = true;
                     if (selectionMask & SELECTION_MASK_VOXEL_OVERLAY_PRIMARY) {
                        maskForThisSlice = SELECTION_MASK_VOXEL_OVERLAY_PRIMARY;
                     }
                  }
               }
            }
            
            if (drawIt) {
               if (maskForThisSlice != SELECTION_MASK_OFF) {
                  glPushName(maskForThisSlice);
                  glPushName(ijk[0]);
                  glPushName(ijk[1]);
                  glPushName(ijk[2]);
                  glPushName(VolumeFile::VOLUME_AXIS_X);
               }
               
               glBegin(GL_QUADS);
                  glVertex3f(sliceX, yCorner, zCorner);
                  glVertex3f(sliceX, yCorner + spacing[1], zCorner);
                  glVertex3f(sliceX, yCorner + spacing[1], zCorner + spacing[2]);
                  glVertex3f(sliceX, yCorner, zCorner + spacing[2]);
               glEnd();
               
               if (maskForThisSlice != SELECTION_MASK_OFF) {
                  glPopName();
                  glPopName();
                  glPopName();
                  glPopName();
                  glPopName();
               }
            }
         }
      }
   }
   
   //
   // Draw functional and segmentation volume clouds and view dependend anatomy slice
   //
   for (int mm = 0; mm < 4; mm++) { 
      //
      // Get display list number for this surface
      // 
      PreferencesFile* pf = brainSet->getPreferencesFile();
      bool displayListsEnabled = pf->getDisplayListsEnabled();
      displayListsEnabled = false;   /// DISPLAY LISTS DO NOT WORK FOR VOLUME CLOUD
                                     /// MAC HANGS WHEN LIST EXECUTED
      unsigned long displayListNumber = 0;
      
      bool showIt = false;
      VolumeFile* vf = NULL;  
      
      unsigned char alpha = 255;
      bool opacityEnabled = false;
      int cloudSelectionMask = 0;
      bool useLighting = true;
      bool doingAnatomyViewDepSlice = false;
      
      switch (mm) {
         case 0:  // functional volume cloud
            showIt = bmsv->getDisplayFunctionalVolumeCloud();
            vf = bmsv->getFunctionalVolumeFile();
            displayListNumber = bmsv->getVoxelCloudDisplayListNumber();
            opacityEnabled = bmsv->getFunctionalVolumeCloudOpacityEnabled();
            alpha = static_cast<unsigned char>(
                              std::min(bmsv->getFunctionalVolumeCloudOpacity() * 255.0, 255.0));
            cloudSelectionMask = SELECTION_MASK_VOXEL_FUNCTIONAL_CLOUD;
            break;
         case 1:  // segmentation volume cloud
            showIt = bmsv->getDisplaySegmentationVolumeCloud();
            vf = bmsv->getSegmentationVolumeFile();
            displayListNumber = 0;
            break;
         case 2:  // view dependent anatomy slice
            showIt = bmsv->getDisplayViewDependentSlice();
            vf = bmsv->getAnatomyVolumeFile();
            displayListNumber = 0;
            useLighting = false;
            doingAnatomyViewDepSlice = true;
            break;
         case 3:
            showIt = bmsv->getDisplayVectorVolumeCloud();
            vf = bmsv->getVectorVolumeFile();
            displayListNumber = 0;
            useLighting = false;
            break;
      }
      //
      // Display volume "cloud"
      //
      if (showIt) {
         //
         // Make sure volume is valid
         //
         if (vf != NULL) {
            //
            // If need to redraw surface
            //
            const bool idMode = (selectionMask != 0);
            if ((displayListNumber == 0) ||
                (displayListsEnabled == false) ||
                idMode) {
                
               //
               // If not ID mode then need to create a new display list for this surface
               //
               bool creatingDisplayList = false;
               if (displayListsEnabled && (idMode == false)) {
                  displayListNumber = glGenLists(1);
                  bmsv->setVoxelCloudDisplayListNumber(displayListNumber);
                  glNewList(displayListNumber, GL_COMPILE);
                  creatingDisplayList = true;
               }
               
               //
               // turn on lighting
               //
               if (useLighting) {
                  glEnable(GL_LIGHTING);
                  glEnable(GL_LIGHT1);
                  glEnable(GL_COLOR_MATERIAL);
               }
               
               //
               // Get the volume spacing
               //
               float spacing[3];
               vf->getSpacing(spacing);
               
               //
               // Get the origin
               //
               float originCorner[3];
               vf->getOriginAtCornerOfVoxel(originCorner);
               float originCenter[3];
               vf->getOrigin(originCenter);

               //
               // Check for valid dimensions
               //
               int dim[3];
               vf->getDimensions(dim);
               
               if (opacityEnabled) {
                  glEnable(GL_BLEND);
                  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
               }
               
               //
               // Check/Update voxel to surface distances
               //
               const float distanceThreshold = bmsv->getFunctionalVolumeDistanceThreshold();
               float* voxelToSurfaceDistances = NULL;
               const bool voxelToSurfaceDistancesValid = false;
               
               //
               // If this is the anatomical view dependent slice
               //
               if (doingAnatomyViewDepSlice) {
                  glPushMatrix();
                     glLoadIdentity();
                  
                     //
                     // Create and enable the clipping planes to only draw
                     // when (-thickness <= Z <= thickness)
                     // 
                     const float thickness = 1.0;
                     GLdouble clipPlanePositive[4] = {  // should clip (Z < -thickness)
                                                        0.0,
                                                        0.0,
                                                       -1.0,  //-thickness,
                                                        thickness
                                                     };
                     GLdouble clipPlaneNegative[4] = {   // should clip (Z > thickness)
                                                        0.0,
                                                        0.0,
                                                        1.0, // thickness,
                                                        thickness
                                                     };
                     glClipPlane(GL_CLIP_PLANE0, clipPlanePositive);
                     glClipPlane(GL_CLIP_PLANE1, clipPlaneNegative);
                     glEnable(GL_CLIP_PLANE0);
                     glEnable(GL_CLIP_PLANE1);
                  glPopMatrix();
               }
               
               const float sx = spacing[0];
               const float sy = spacing[1];
               const float sz = spacing[2];

               const bool drawVectorsFlag = (vf->getVolumeType() == VolumeFile::VOLUME_TYPE_VECTOR);
               int increment = 1;
               if (drawVectorsFlag) {
                  DisplaySettingsVolume* dsv = brainSet->getDisplaySettingsVolume();
                  increment = dsv->getVectorVolumeSparsity();
               }
               //
               // Draw the voxels
               //
               for (int i = 0; i < dim[0]; i += increment) {
                  for (int j = 0; j < dim[1]; j += increment) {
                     for (int k = 0; k < dim[2]; k += increment) {
                        int ijk[3] = { i, j, k };
                        const int pointID = vf->getVoxelNumber(ijk);
                        
                        if (voxelToSurfaceDistancesValid) {
                           if (voxelToSurfaceDistances[pointID] > distanceThreshold) {
                              continue;
                           }
                        }
                        
                        unsigned char rgb[4];
                        voxelColoring->getVoxelColoring(vf, i, j, k, rgb);
                        if (rgb[3] == VolumeFile::VOXEL_COLOR_STATUS_VALID) {
                           glColor4ub(rgb[0], rgb[1], rgb[2], alpha);
                           
                           if (selectionMask & cloudSelectionMask) {
                              glPushName(cloudSelectionMask);
                              glPushName(i);
                              glPushName(j);
                              glPushName(k);
                           }
                           
                           //
                           // Is this a vector volume
                           //
                           if (drawVectorsFlag) {
                              const float x = i * sx + originCenter[0];
                              const float y = j * sy + originCenter[1];
                              const float z = k * sz + originCenter[2];
                              const float xp = x + sx;
                              const float yp = y + sy;
                              const float zp = z + sz;
                              const float mag = vf->getVoxel(i, j, k, 3);
                              glBegin(GL_LINES);
                                 glColor4ub(255, 0, 0, 255);
                                 glVertex3f(xp, yp, zp);
                                 glColor4ub(255, 255, 0, 255);
                                 glVertex3f(xp + vf->getVoxel(i, j, k, 0) * mag,
                                            yp + vf->getVoxel(i, j, k, 1) * mag,
                                            zp + vf->getVoxel(i, j, k, 2) * mag);
                              glEnd();
                           }
                           else {
                              const float x = i * sx + originCorner[0];
                              const float y = j * sy + originCorner[1];
                              const float z = k * sz + originCorner[2];
                              glBegin(GL_QUADS);
                                 //
                                 // Near face
                                 //
                                 glNormal3f(0.0, 0.0, 1.0);
                                 glVertex3f(     x,      y, z);
                                 glVertex3f(x + sx,      y, z);
                                 glVertex3f(x + sx, y + sy, z);
                                 glVertex3f(     x, y + sy, z);
         
                                 //
                                 // Far face
                                 //
                                 glNormal3f(0.0, 0.0, -1.0);
                                 glVertex3f(x + sx,      y, z + sz);
                                 glVertex3f(     x,      y, z + sz);
                                 glVertex3f(     x, y + sy, z + sz);
                                 glVertex3f(x + sx, y + sy, z + sz);
                                 
                                 //
                                 // Right face
                                 //
                                 glNormal3f(1.0, 0.0, 0.0);
                                 glVertex3f(x + sx,      y, z);
                                 glVertex3f(x + sx,      y, z + sz);
                                 glVertex3f(x + sx, y + sy, z + sz);
                                 glVertex3f(x + sx, y + sy, z);
                                 
                                 //
                                 // Left face
                                 //
                                 glNormal3f(-1.0, 0.0, 0.0);
                                 glVertex3f(x,      y, z);
                                 glVertex3f(x, y + sy, z);
                                 glVertex3f(x, y + sy, z + sz);
                                 glVertex3f(x,      y, z + sz);
                                 
                                 //
                                 // Bottom face
                                 //
                                 glNormal3f(0.0, -1.0, 0.0);
                                 glVertex3f(x,      y,      z);
                                 glVertex3f(x,      y, z + sz);
                                 glVertex3f(x + sx, y, z + sz);
                                 glVertex3f(x + sx, y,      z);
                                 
                                 //
                                 // Top face
                                 //
                                 glNormal3f(0.0, 1.0, 0.0);
                                 glVertex3f(x,      y + sy,      z);
                                 glVertex3f(x + sx, y + sy,      z);
                                 glVertex3f(x + sx, y + sy, z + sz);
                                 glVertex3f(x,      y + sy, z + sz);
                              glEnd();
                           }
                           
                           if (selectionMask & cloudSelectionMask) {
                              glPopName();
                              glPopName();
                              glPopName();
                              glPopName();
                           }
                        }
                     }
                  }
               }
               
               if (opacityEnabled) {
                  glDisable(GL_BLEND);
               }
               //glDisable(GL_COLOR_MATERIAL);
               
               glDisable(GL_LIGHTING);
               glDisable(GL_LIGHT1);
               glDisable(GL_COLOR_MATERIAL);
               
               //
               // If this is the anatomical view dependent slice
               //
               if (doingAnatomyViewDepSlice) {
                  glDisable(GL_CLIP_PLANE0);
                  glDisable(GL_CLIP_PLANE1);
               }
               
               //
               // End display list if creating a display list
               //
               if (creatingDisplayList) {
                  glEndList();
               }
               
            }
            
            //
            // If display lists enabled, execute it
            //
            if (displayListsEnabled && (idMode == false)) {
               if (glIsList(displayListNumber)) {
                  glCallList(displayListNumber);
               }
               else {
                  std::cout << "PROGRAM WARNING: display list " << displayListNumber
                            << " does not exist for surface and volume " << bmsv->getDescriptiveName().toAscii().constData() 
                            << " in window " << viewingWindowNumber << std::endl;
               }
            }
         }
      }
   }
   
   //
   // turn off lighting
   //
   glDisable(GL_LIGHTING);
   glDisable(GL_LIGHT1);

   //
   // Draw the palette
   //
   drawMetricPalette(bmsv->getBrainModelIndex(), true);
   
   //
   // Draw shape bar
   //
   drawShapePalette(bmsv->getBrainModelIndex());
}

/**
 * Draw a volume slice
 */
void
BrainModelOpenGL::drawVolumeSliceOverlayAndUnderlay(BrainModelVolume* bmv,
                                  const VolumeFile::VOLUME_AXIS volumeSliceAxis, 
                                  const int currentSlice,
                                  VolumeFile* &firstVolume)
{
   bool firstVolumeFlag = true;
   firstVolume = NULL;
   float firstVolumeVoxelSize = 1.0;
   float volumeSliceCoordinate = 0.0;
   
   //
   // Draw the underlay and the overlays
   //
   for (int i = 0; i < 4; i++) {
      //
      // Should only the underlay be shown in a viewing window
      //
      if (i > 0) {
         if (viewingWindowNumber > 0) {
            if (bmv->getUnderlayVolumeFile()) {
               if (bmv->getShowUnderlayOnlyInWindow(viewingWindowNumber)) {
                  continue;
               }
            }
         }
      }
      
      bool overlayFlag = false;
      unsigned long theSelectionMask = SELECTION_MASK_OFF;
      VolumeFile* vf = NULL;
      switch(i) {
         case 0:
            theSelectionMask = SELECTION_MASK_VOXEL_UNDERLAY;
            vf = bmv->getUnderlayVolumeFile();
            break;
         case 1:
            theSelectionMask = SELECTION_MASK_VOXEL_OVERLAY_SECONDARY;
            vf = bmv->getOverlaySecondaryVolumeFile();
            overlayFlag = true;
            break;
         case 2:
            theSelectionMask = SELECTION_MASK_VOXEL_OVERLAY_PRIMARY;
            vf = bmv->getOverlayPrimaryVolumeFile();
            overlayFlag = true;
            break;
         case 3:
            {
               //
               // Draw the selected voxels for Region of Interest
               //
               BrainModelVolumeRegionOfInterest* roi = brainSet->getVolumeRegionOfInterestController();
               if (roi->getDisplayVolumeROI()) {
                  vf = roi->getROIVolume();
               }
            }
            break;
      }

      if (vf != NULL) {
         //
         // Get origin and spacing
         //
         float originCenter[3], originCorner[3], spacing[3];
         int dim[3];
         vf->getOrigin(originCenter);
         vf->getOriginAtCornerOfVoxel(originCorner);
         vf->getSpacing(spacing);
         vf->getDimensions(dim);
         
         int sliceToDraw = currentSlice;
         if (firstVolumeFlag) {
            firstVolumeFlag = false;
            firstVolume = vf;
            switch(volumeSliceAxis) {
               case VolumeFile::VOLUME_AXIS_X:  // PARASAGITTAL
                  volumeSliceCoordinate = currentSlice * spacing[0] + originCenter[0];
                  firstVolumeVoxelSize = spacing[0];
                  break;
               case VolumeFile::VOLUME_AXIS_Y:  // CORONAL
                  volumeSliceCoordinate = currentSlice * spacing[1] + originCenter[1];
                  firstVolumeVoxelSize = spacing[1];
                  break;
               case VolumeFile::VOLUME_AXIS_Z:  // HORIZONTAL
                  volumeSliceCoordinate = currentSlice * spacing[2] + originCenter[2];
                  firstVolumeVoxelSize = spacing[2];
                  break;
               default:
                  std::cout << "PROGRAM ERROR: Invalid volume axis at " << __LINE__ << " in " 
                           << __FILE__ << std::endl;
                  return;
            }
         }
         else {
            sliceToDraw = -1;
            float sliceDistance = std::numeric_limits<float>::max();
            
            switch(volumeSliceAxis) {
               case VolumeFile::VOLUME_AXIS_X:  // PARASAGITTAL
                  for (int i = 0; i < dim[0]; i++) {
                     const float val = i * spacing[0] + originCenter[0];
                     const float dist = fabs(volumeSliceCoordinate - val);
                     if (dist < sliceDistance) {
                        sliceToDraw = i;
                        sliceDistance = dist;
                     }
                  }
                  break;
               case VolumeFile::VOLUME_AXIS_Y:  // CORONAL
                  for (int i = 0; i < dim[1]; i++) {
                     const float val = i * spacing[1] + originCenter[1];
                     const float dist = fabs(volumeSliceCoordinate - val);
                     if (dist < sliceDistance) {
                        sliceToDraw = i;
                        sliceDistance = dist;
                     }
                  }
                  break;
               case VolumeFile::VOLUME_AXIS_Z:  // HORIZONTAL
                  for (int i = 0; i < dim[2]; i++) {
                     const float val = i * spacing[2] + originCenter[2];
                     const float dist = fabs(volumeSliceCoordinate - val);
                     if (dist < sliceDistance) {
                        sliceToDraw = i;
                        sliceDistance = dist;
                     }
                  }
                  break;
               default:
                  std::cout << "PROGRAM ERROR: Invalid volume axis at " << __LINE__ << " in " 
                           << __FILE__ << std::endl;
                  return;
            }
         }
         
         drawVolumeFileSlice(vf, volumeSliceAxis, sliceToDraw, theSelectionMask, overlayFlag);
      }
   }

   //
   // Draw volume borders
   //   
   DisplaySettingsBorders* dsb = brainSet->getDisplaySettingsBorders();
   if (dsb->getDisplayBorders()) {
      drawVolumeBorderFile(volumeSliceAxis, volumeSliceCoordinate, firstVolumeVoxelSize);
   }
   
   //
   // Draw the volume cells
   //
   DisplaySettingsCells* dsc = brainSet->getDisplaySettingsCells();
   if (dsc->getDisplayVolumeCells()) {
      drawVolumeCellFile(volumeSliceAxis,
                         volumeSliceCoordinate,
                         firstVolumeVoxelSize);
   }
   
   //
   // Draw the volume foci
   //
   DisplaySettingsFoci* dsf = brainSet->getDisplaySettingsFoci();
   if (dsf->getDisplayVolumeCells()) {
      drawVolumeFociFile(volumeSliceAxis,
                         volumeSliceCoordinate,
                         firstVolumeVoxelSize);
   }

   //
   // Draw vectors on the volume
   //
   drawVectorsOnVolume(volumeSliceAxis,
                       volumeSliceCoordinate,
                       firstVolumeVoxelSize);

   //
   // Draw contours over volume slices
   //
   drawVolumeContourFile(volumeSliceAxis,
                         volumeSliceCoordinate,
                         firstVolumeVoxelSize);
                         
   //
   // Draw outline of surface and transformation axes
   //
   drawVolumeSurfaceOutlineAndTransformationMatrixAxes(bmv, volumeSliceAxis, volumeSliceCoordinate);
   
   //
   // Draw the identify symbols
   //
   drawVolumeIdentifySymbols(volumeSliceAxis, volumeSliceCoordinate);

   /// djs draw nodes found within electrode trajectory cylinder
   ///
   drawVolumeElectrodeTrajectory(bmv, volumeSliceAxis, volumeSliceCoordinate);
}

/**
 * draw volume with oblique ALL axis.
 */
void 
BrainModelOpenGL::drawBrainModelVolumeObliqueAllAxis(BrainModelVolume* bmv)
{
   glDisable(GL_DEPTH_TEST);
   
   //
   // See if selecting
   //
   bool selectFlag = false;
   if (selectionMask & SELECTION_MASK_VOXEL_UNDERLAY) {
      selectFlag = true;
   }
   else if (selectionMask != 0) {
      return;
   }
   
   //
   // Get the current slices
   //
   //int slices[3];
   //bmv->getSelectedSlices(viewingWindowNumber, slices);

   //
   // Determine viewport height and width
   //
   const int vpHeight = viewport[3] / 2;
   const int vpWidth  = viewport[2] / 2;
   for (int j = 0; j < 2; j++) {
      for (int i = 0; i < 2; i++) {
         //
         // viewport X & Y
         //
         const int vpX = i * vpWidth;
         const int vpY = j * vpHeight;
         
         //
         // if selecting, only draw if mouse is in this viewport
         //
         if (selectionMask != SELECTION_MASK_OFF) {
            if ((selectionX < vpX) ||
                (selectionY < vpY) ||
                (selectionX > (vpX + vpWidth)) ||
                (selectionY > (vpY + vpHeight))) {
               continue;
            }
         }
   
         //
         // Set the viewport
         //
         glViewport(vpX, vpY, vpWidth, vpHeight);
         if (selectionMask == SELECTION_MASK_OFF) {
            glGetIntegerv(GL_VIEWPORT, selectionViewport[viewingWindowNumber]);
         }
         
         //
         // Set orthographic transform
         //
         if (selectionMask == SELECTION_MASK_OFF) {
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            glOrtho(orthographicLeft[viewingWindowNumber], orthographicRight[viewingWindowNumber], 
                 orthographicBottom[viewingWindowNumber], orthographicTop[viewingWindowNumber], 
                 orthographicNear[viewingWindowNumber], orthographicFar[viewingWindowNumber]);
         
            glGetDoublev(GL_PROJECTION_MATRIX, selectionProjectionMatrix[viewingWindowNumber]);
         }
         
         glMatrixMode(GL_MODELVIEW);
         glLoadIdentity();

         //
         // Save modeling matrix
         //
         if (selectionMask == SELECTION_MASK_OFF) {
            glGetDoublev(GL_MODELVIEW_MATRIX, selectionModelviewMatrix[viewingWindowNumber]);
         }
         
         if (i == 0) {
            if (j == 0) {
               VolumeFile* vf = bmv->getUnderlayVolumeFile();
               if (vf == NULL) {
                  vf = bmv->getOverlaySecondaryVolumeFile();
               }
               if (vf == NULL) {
                  vf = bmv->getOverlayPrimaryVolumeFile();
               }
               //
               // Draw the crosshair coordinates
               //
               if (vf != NULL) {
                  //drawVolumeCrosshairCoordinates(bmv, vf, vpHeight);
               }
               
               BrainModelSurface* bms = brainSet->getActiveFiducialSurface();
               if (bms != NULL) {
                  glEnable(GL_DEPTH_TEST);
                  drawBrainModelSurface(bms, NULL, true, true);
               
                  //
                  // Get the current oblique slices
                  //
                  int slices[3];
                  bmv->getSelectedObliqueSlices(slices);
                  
                  //
                  // Draw the volume slices
                  //
                  if (vf != NULL) {
                     int dim[3];
                     vf->getDimensions(dim);
                     float originCenter[3], originCorner[3], spacing[3];
                     vf->getOrigin(originCenter);
                     vf->getOriginAtCornerOfVoxel(originCorner);
                     vf->getSpacing(spacing);
                     const float minX = originCorner[0];
                     const float maxX = originCorner[0] + spacing[0] * dim[0];
                     const float minY = originCorner[1];
                     const float maxY = originCorner[1] + spacing[1] * dim[1];
                     const float minZ = originCorner[2];
                     const float maxZ = originCorner[2] + spacing[2] * dim[2];
                     float sliceX = originCenter[0] + spacing[0] * slices[0];
                     float sliceY = originCenter[1] + spacing[1] * slices[1];
                     float sliceZ = originCenter[2] + spacing[2] * slices[2];
                     int sliceOffsets[3];
                     bmv->getSelectedObliqueSliceOffsets(viewingWindowNumber, sliceOffsets);
                     sliceX += sliceOffsets[0];
                     sliceY += sliceOffsets[1];
                     sliceZ += sliceOffsets[2];
                     
                     glEnable(GL_BLEND);
                     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                     glColor4ub(190, 190, 190, 130);

                     glPushMatrix();
                        GLfloat mat[16];
                        DisplaySettingsVolume* dsv = brainSet->getDisplaySettingsVolume();
                        const TransformationMatrix* obtm = dsv->getObliqueSlicesTransformationMatrix();
                        if (obtm != NULL) {
                           obtm->getMatrix(mat);
                        }
                        else {
                           bmv->getObliqueRotationMatrix(mat);  // USE MAIN WINDOW MATRIX
                        }
                        glMultMatrixf(mat);
                        glBegin(GL_TRIANGLES);
                           glVertex3f(sliceX, minY, minZ);
                           glVertex3f(sliceX, maxY, minZ);
                           glVertex3f(sliceX, maxY, maxZ);
                           glVertex3f(sliceX, minY, minZ);
                           glVertex3f(sliceX, maxY, maxZ);
                           glVertex3f(sliceX, minY, maxZ);

                           glVertex3f(minX, sliceY, minZ);
                           glVertex3f(maxX, sliceY, minZ);
                           glVertex3f(maxX, sliceY, maxZ);
                           glVertex3f(minX, sliceY, minZ);
                           glVertex3f(maxX, sliceY, maxZ);
                           glVertex3f(minX, sliceY, maxZ);
                           
                           glVertex3f(minX, minY, sliceZ);
                           glVertex3f(maxX, minY, sliceZ);
                           glVertex3f(maxX, maxY, sliceZ);
                           glVertex3f(minX, minY, sliceZ);
                           glVertex3f(maxX, maxY, sliceZ);
                           glVertex3f(minX, maxY, sliceZ);
                        glEnd();
                     glPopMatrix();
                     
                     glDisable(GL_BLEND);
                  }
               
                  glDisable(GL_DEPTH_TEST);
               }
            }
            else {
               //
               // Draw parasaggital
               //
               drawBrainModelVolumeObliqueAxisSlice(bmv, 
                                        VolumeFile::VOLUME_AXIS_OBLIQUE_X,
                                        0,
                                        selectFlag);
            }
         }
         else {
            if (j == 0) {
               //
               // Draw horizontal
               //
               drawBrainModelVolumeObliqueAxisSlice(bmv, 
                                        VolumeFile::VOLUME_AXIS_OBLIQUE_Z,
                                        0,
                                        selectFlag);
            }
            else {
               //
               // Draw coronal
               //
               drawBrainModelVolumeObliqueAxisSlice(bmv, 
                                        VolumeFile::VOLUME_AXIS_OBLIQUE_Y,
                                        0,
                                        selectFlag);
            }
         }
      }
   }
         
   glEnable(GL_DEPTH_TEST);
}

/**
 * draw volume with oblique axis montage.
 */
void 
BrainModelOpenGL::drawBrainModelVolumeObliqueAxisMontage(BrainModelVolume* bmv,
                                                         const bool selectFlag)
{
   const DisplaySettingsVolume* dsv = brainSet->getDisplaySettingsVolume();
   
   //
   // Don't set projection matrix when selecting
   //
   if (selectionMask == SELECTION_MASK_OFF) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      glOrtho(orthographicLeft[viewingWindowNumber], orthographicRight[viewingWindowNumber], 
              orthographicBottom[viewingWindowNumber], orthographicTop[viewingWindowNumber], 
              orthographicNear[viewingWindowNumber], orthographicFar[viewingWindowNumber]);
      
      glGetDoublev(GL_PROJECTION_MATRIX, selectionProjectionMatrix[viewingWindowNumber]);
   }

   //
   // Get montage info 
   //
   int rows, columns, sliceIncrement;
   dsv->getMontageViewSettings(rows, columns, sliceIncrement);

   //
   // Montage slice offset
   //
   int montageSliceOffset = 0;
   
   //
   // Determine viewport height and width
   //
   const int vpHeight = viewport[3] / rows;
   const int vpWidth  = viewport[2] / columns;
   for (int i = (rows - 1); i >= 0; i--) {
      for (int j = 0; j < columns; j++) {
      //for (int i = 0; i < rows; i++) {
         const int vpX = j * vpWidth;
         const int vpY = i * vpHeight;
         //
         // if selecting, only draw if mouse is in this viewport
         //
         if (selectFlag) {
            if ((selectionX < vpX) ||
                (selectionY < vpY) ||
                (selectionX > (vpX + vpWidth)) ||
                (selectionY > (vpY + vpHeight))) {
               continue;
            }
         }
         
         //
         // Set the viewport
         //
         glViewport(vpX, vpY, vpWidth, vpHeight);
         
         //
         // Don't set projection matrix when selecting
         //
         if (selectFlag == false) {
            //
            // Set orthographic transform
            //
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            glOrtho(orthographicLeft[viewingWindowNumber], orthographicRight[viewingWindowNumber], 
                    orthographicBottom[viewingWindowNumber], orthographicTop[viewingWindowNumber], 
                    orthographicNear[viewingWindowNumber], orthographicFar[viewingWindowNumber]);
            
            glGetDoublev(GL_PROJECTION_MATRIX, selectionProjectionMatrix[viewingWindowNumber]);
         }
         
         glMatrixMode(GL_MODELVIEW);
         glLoadIdentity();

         drawBrainModelVolumeObliqueAxisSlice(bmv, 
                                              bmv->getSelectedAxis(viewingWindowNumber),
                                              montageSliceOffset,
                                              selectFlag);
         
         //
         // Move to next slice
         //
         montageSliceOffset += sliceIncrement;
      }
   }
}
      
/**
 * draw volume with oblique axis.
 */
void 
BrainModelOpenGL::drawBrainModelVolumeObliqueAxis(BrainModelVolume* bmv)
{
   //
   // Get the master volume file
   //
   const VolumeFile* masterVolume = bmv->getMasterVolumeFile();
   if (masterVolume == NULL) {
      return;
   }
   DisplaySettingsVolume* dsv = brainSet->getDisplaySettingsVolume();
   
   //
   // See if selecting
   //
   bool selectFlag = false;
   if (selectionMask & SELECTION_MASK_VOXEL_UNDERLAY) {
      selectFlag = true;
   }
   else if (selectionMask != 0) {
      return;
   }
   
   switch(bmv->getSelectedAxis(viewingWindowNumber)) {
      case VolumeFile::VOLUME_AXIS_X:
      case VolumeFile::VOLUME_AXIS_Y:
      case VolumeFile::VOLUME_AXIS_Z:
      case VolumeFile::VOLUME_AXIS_ALL:
      case VolumeFile::VOLUME_AXIS_OBLIQUE:
         break;
      case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
         if (dsv->getMontageViewSelected()) {
            drawBrainModelVolumeObliqueAxisMontage(bmv, selectFlag);
            return;
         }
         break;
      case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
      case VolumeFile::VOLUME_AXIS_UNKNOWN:
         break;
   }
   
   //
   // Don't set projection matrix when selecting
   //
   if (selectFlag == false) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      
      glOrtho(orthographicLeft[viewingWindowNumber], orthographicRight[viewingWindowNumber], 
              orthographicBottom[viewingWindowNumber], orthographicTop[viewingWindowNumber], 
              orthographicNear[viewingWindowNumber], orthographicFar[viewingWindowNumber]);
              
      glGetDoublev(GL_PROJECTION_MATRIX, selectionProjectionMatrix[viewingWindowNumber]);
   }

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   
   drawBrainModelVolumeObliqueAxisSlice(bmv, 
                                        bmv->getSelectedAxis(viewingWindowNumber),
                                        0,
                                        selectFlag);
}
   
/**
 * draw volume with oblique axis.
 */
void 
BrainModelOpenGL::drawBrainModelVolumeObliqueAxisSlice(BrainModelVolume* bmv,
                                                  VolumeFile::VOLUME_AXIS axis,
                                                  const int montageOffset,
                                                  const bool selectFlag)
{
   //
   // Get the master volume file
   //
   const VolumeFile* masterVolume = bmv->getMasterVolumeFile();
   if (masterVolume == NULL) {
      return;
   }
   
   DisplaySettingsVolume* dsv = brainSet->getDisplaySettingsVolume();
   
   //
   // Save modeling matrix
   //
   if (selectFlag == false) {
      glGetDoublev(GL_MODELVIEW_MATRIX, selectionModelviewMatrix[viewingWindowNumber]);
   }

   //
   // Create a transformation matrix
   //
   TransformationMatrix tm;
   
   int sliceOffsets[3] = { 0, 0, 0 };
   
   switch (axis) {
      case VolumeFile::VOLUME_AXIS_X:
         return;
         break;
      case VolumeFile::VOLUME_AXIS_Y:
         return;
         break;
      case VolumeFile::VOLUME_AXIS_Z:
         return;
         break;
      case VolumeFile::VOLUME_AXIS_ALL:
         return;
         break;
      case VolumeFile::VOLUME_AXIS_OBLIQUE:
         {
            //
            // Get the current oblique slices
            //
            int slices[3];
            bmv->getSelectedObliqueSlices(slices);
            
            //
            // Get coordinate of slices
            //
            float xyz[3];
            masterVolume->getVoxelCoordinate(slices, 
                                             xyz);
   
            //
            // Translate by the slices
            //
            tm.translate(xyz);
            
            //
            // Rotate by the oblique rotation matrix
            //
            vtkTransform* rotationMatrix = bmv->getObliqueRotationMatrix();
            TransformationMatrix rotMatrix;
            rotMatrix.setMatrix(rotationMatrix);
            tm.preMultiply(rotMatrix);
         }
         break;
      case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
         {
            int montageSliceOffsets[3] = { 0, 0, 0 };
            if (axis == VolumeFile::VOLUME_AXIS_OBLIQUE_X) {
               montageSliceOffsets[0] = montageOffset;
            }
            else if (axis == VolumeFile::VOLUME_AXIS_OBLIQUE_Y) {
               montageSliceOffsets[1] = montageOffset;
            }
            else if (axis == VolumeFile::VOLUME_AXIS_OBLIQUE_Z) {
               montageSliceOffsets[2] = montageOffset;
            }
            
            //
            // NEED TO ADD SLICE OFFSETS
            //
            const TransformationMatrix* obtm = dsv->getObliqueSlicesTransformationMatrix();
            if (obtm != NULL) {
               tm = *obtm;
               TransformationMatrix rotMatrix = tm;
               rotMatrix.setTranslation(0.0, 0.0, 0.0);


               //
               // Add in slice offsets
               //
               bmv->getSelectedObliqueSliceOffsets(viewingWindowNumber, sliceOffsets);
               
               //
               // Montage offsets
               //
               sliceOffsets[0] += montageSliceOffsets[0];
               sliceOffsets[1] += montageSliceOffsets[1];
               sliceOffsets[2] += montageSliceOffsets[2];
               
               float offsetXYZ[3] = {
                                 static_cast<float>(sliceOffsets[0]),
                                 static_cast<float>(sliceOffsets[1]),
                                 static_cast<float>(sliceOffsets[2])
                              };
               rotMatrix.multiplyPoint(offsetXYZ);
               TransformationMatrix tm2;
               tm2.translate(offsetXYZ[0], offsetXYZ[1], offsetXYZ[2]);
               tm.preMultiply(tm2);
            }
            else {
               //
               // Get the rotation matrix
               //
               vtkTransform* rotationMatrix = bmv->getObliqueRotationMatrix();
               tm.setMatrix(rotationMatrix);
               const TransformationMatrix rotMatrix = tm;
               
               //
               // Get the current oblique slices 
               //
               int slices[3];
               bmv->getSelectedObliqueSlices(slices);

               //
               // Get coordinate of slices
               //
               float xyz[3];
               masterVolume->getVoxelCoordinate(slices, 
                                                xyz);

               //
               // Set the translation
               //
               tm.setTranslation(xyz[0], xyz[1], xyz[2]);
               

               //
               // Add in slice offsets
               //
               bmv->getSelectedObliqueSliceOffsets(viewingWindowNumber, sliceOffsets);
               
               //
               // Montage offsets
               //
               sliceOffsets[0] += montageSliceOffsets[0];
               sliceOffsets[1] += montageSliceOffsets[1];
               sliceOffsets[2] += montageSliceOffsets[2];
               
               float offsetXYZ[3] = {
                                 static_cast<float>(sliceOffsets[0]),
                                 static_cast<float>(sliceOffsets[1]),
                                 static_cast<float>(sliceOffsets[2])
                              };
                              
               rotMatrix.multiplyPoint(offsetXYZ);
               TransformationMatrix tm2;
               tm2.translate(offsetXYZ[0], offsetXYZ[1], offsetXYZ[2]);
               tm.preMultiply(tm2);
            }
         }
         break;
      case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
         return;
         break;
      case VolumeFile::VOLUME_AXIS_UNKNOWN:
         std::cout << "PROGRAM ERROR: UNKNOWN axis for drawing." << std::endl;
         return;
         break;
   }

   //
   // Translate but not with matrix
   //   
   float trans[3];
   bmv->getTranslation(viewingWindowNumber, trans);
   glTranslatef(trans[0], trans[1], trans[2]);
   
   //
   // Apply scaling to the matrix
   //
   float scale[3];
   bmv->getScaling(viewingWindowNumber, scale);
   glScalef(scale[0], scale[1], scale[2]);
   
   //
   // Get voxel coloring 
   //
   BrainModelVolumeVoxelColoring* voxelColoring = brainSet->getVoxelColoring();
   
   //
   // Determine coords to draw
   //
   const float minCoordX = orthographicLeft[viewingWindowNumber];   //-100.0;
   const float maxCoordX = orthographicRight[viewingWindowNumber];  // 100.0;
   const float minCoordY = orthographicBottom[viewingWindowNumber]; // -100.0;
   const float maxCoordY = orthographicTop[viewingWindowNumber];    // 100.0;
   const float voxSize = dsv->getObliqueSlicesSamplingSize(); //1.0;

   //
   // Loop through "oblique slice"
   //
   for (float x = minCoordX; x < maxCoordX; x += voxSize) {
      for (float y = minCoordY; y < maxCoordY; y += voxSize) {
         //
         // Apply the transformation matrix to the point
         //
         float pt[3] = { x, y, 0.0 };
         switch (axis) {
            case VolumeFile::VOLUME_AXIS_X:
               return;
               break;
            case VolumeFile::VOLUME_AXIS_Y:
               return;
               break;
            case VolumeFile::VOLUME_AXIS_Z:
               return;
               break;
            case VolumeFile::VOLUME_AXIS_ALL:
               return;
               break;
            case VolumeFile::VOLUME_AXIS_OBLIQUE:
               pt[0] = x;
               pt[1] = y;
               pt[2] = 0;
               break;
            case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
               pt[0] = 0.0;
               pt[1] = x;
               pt[2] = y;
               break;
            case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
               pt[0] = x;
               pt[1] = 0.0;
               pt[2] = y;
               break;
            case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
               pt[0] = x;
               pt[1] = y;
               pt[2] = 0;
               break;
            case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
               break;
            case VolumeFile::VOLUME_AXIS_UNKNOWN:
               break;
         }
         
         //
         // Pick location slice offset
         //
         const int pickLocation[3] = {
            static_cast<int>(pt[0]) + sliceOffsets[0],
            static_cast<int>(pt[1]) + sliceOffsets[1],
            static_cast<int>(pt[2]) + sliceOffsets[2]
         };
         
         //
         // Transforms screen into plane
         //
         tm.multiplyPoint(pt);
         
         //
         // Get voxel to draw from "top-most" volume
         //
         bool ijkValid = false;
         int ijk[3];
         VolumeFile* vf = NULL;
         unsigned char rgb[4];
         GLuint selMask = SELECTION_MASK_OFF;
         for (int i = 0; i < 3; i++) {
            vf = NULL;
            selMask = SELECTION_MASK_OFF;
            switch (i) {
               case 0:
                  if (selectFlag) {
                     selMask = SELECTION_MASK_VOXEL_OVERLAY_PRIMARY;
                  }
                  vf = bmv->getOverlayPrimaryVolumeFile();
                  break;
               case 1:
                  if (selectFlag) {
                     selMask = SELECTION_MASK_VOXEL_OVERLAY_SECONDARY;
                  }
                  vf = bmv->getOverlaySecondaryVolumeFile();
                  break;
               case 2:
                  if (selectFlag) {
                     selMask = SELECTION_MASK_VOXEL_UNDERLAY;
                  }
                  vf = bmv->getUnderlayVolumeFile();
                  break;
            }
            
            if (vf != NULL) {
               //
               // See if the 3D point is inside the volume
               //
               ijkValid = vf->convertCoordinatesToVoxelIJK(pt, ijk);
               if (ijkValid) {
                  //
                  // See if voxel color is valid
                  //
                  voxelColoring->getVoxelColoring(vf, ijk[0], ijk[1], ijk[2], rgb);
                  ijkValid = (rgb[3] == VolumeFile::VOXEL_COLOR_STATUS_VALID);
                  if (ijkValid) {
                     break;
                  }
               }
            }  // if (vf != NULL...
         } // for (i...
         
         //
         // Voxel is in volume
         //
         if ((vf != NULL) && ijkValid) {
            if (selMask != SELECTION_MASK_OFF) {
               glPushName(selMask);
               glPushName(ijk[0]);
               glPushName(ijk[1]);
               glPushName(ijk[2]);
               glPushName(pickLocation[0]);
               glPushName(pickLocation[1]);
               glPushName(pickLocation[2]);
            }
            
            //
            // Draw if coloring is valid
            //
            glBegin(GL_QUADS);
            {
               glColor3ubv(rgb);
               glVertex2f(x, y);
               glVertex2f(x + voxSize, y);
               glVertex2f(x + voxSize, y + voxSize);
               glVertex2f(x, y + voxSize);
            }
            glEnd();

            if (selMask != SELECTION_MASK_OFF) {
               glPopName();
               glPopName();
               glPopName();
               glPopName();
               glPopName();
               glPopName();
               glPopName();
            }
         }
      }
   }
   
   //
   // Points on screen counterclockwise starting at bottom left corner
   //   
   const float screenCornerCoords[4][2] = {
      { minCoordX, minCoordY },
      { maxCoordX, minCoordY },
      { maxCoordX, maxCoordY },
      { minCoordX, maxCoordY }
   };
   
   //
   // Screen coordinates converted to slice stereotaxic coordinates
   //
   float sliceCornerCoords[4][3];
   
   //
   // Loop through "oblique slice"
   //
   for (int i = 0; i < 4; i++) {
      //
      // Apply the transformation matrix to the point
      //
      const float x = screenCornerCoords[i][0];
      const float y = screenCornerCoords[i][1];
      
      float pt[3] = { x, y, 0.0 };
      switch (axis) {
         case VolumeFile::VOLUME_AXIS_X:
            return;
            break;
         case VolumeFile::VOLUME_AXIS_Y:
            return;
            break;
         case VolumeFile::VOLUME_AXIS_Z:
            return;
            break;
         case VolumeFile::VOLUME_AXIS_ALL:
            return;
            break;
         case VolumeFile::VOLUME_AXIS_OBLIQUE:
            pt[0] = x;
            pt[1] = y;
            pt[2] = 0;
            break;
         case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
            pt[0] = 0.0;
            pt[1] = x;
            pt[2] = y;
            break;
         case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
            pt[0] = x;
            pt[1] = 0.0;
            pt[2] = y;
            break;
         case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
            pt[0] = x;
            pt[1] = y;
            pt[2] = 0;
            break;
         case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
            return;
            break;
         case VolumeFile::VOLUME_AXIS_UNKNOWN:
            break;
      }
      tm.multiplyPoint(pt);
      sliceCornerCoords[i][0] = pt[0];
      sliceCornerCoords[i][1] = pt[1];
      sliceCornerCoords[i][2] = pt[2];
   }
   
   if (dsv->getDisplayCrosshairs()) {
      const float bigNumber = 10000.0;
      switch (axis) {
         case VolumeFile::VOLUME_AXIS_X:
            return;
            break;
         case VolumeFile::VOLUME_AXIS_Y:
            return;
            break;
         case VolumeFile::VOLUME_AXIS_Z:
            return;
            break;
         case VolumeFile::VOLUME_AXIS_ALL:
            return;
            break;
         case VolumeFile::VOLUME_AXIS_OBLIQUE:
            glBegin(GL_LINES);
               glColor3f(1.0, 0.0, 0.0);
               glVertex2f(-bigNumber, 0.0);
               glVertex2f( bigNumber, 0.0);
               glColor3f(0.0, 1.0, 0.0);
               glVertex2f(0.0, -bigNumber);
               glVertex2f(0.0,  bigNumber);
            glEnd();
            break;
         case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
            glBegin(GL_LINES);
               glColor3f(0.0, 1.0, 0.0);
               glVertex2f(-bigNumber, 0.0);
               glVertex2f( bigNumber, 0.0);
               glColor3f(0.0, 0.0, 1.0);
               glVertex2f(0.0, -bigNumber);
               glVertex2f(0.0,  bigNumber);
            glEnd();
            break;
         case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
            glBegin(GL_LINES);
               glColor3f(1.0, 0.0, 0.0);
               glVertex2f(-bigNumber, 0.0);
               glVertex2f( bigNumber, 0.0);
               glColor3f(0.0, 0.0, 1.0);
               glVertex2f(0.0, -bigNumber);
               glVertex2f(0.0,  bigNumber);
            glEnd();
            break;
         case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
            glBegin(GL_LINES);
               glColor3f(1.0, 0.0, 0.0);
               glVertex2f(-bigNumber, 0.0);
               glVertex2f( bigNumber, 0.0);
               glColor3f(0.0, 1.0, 0.0);
               glVertex2f(0.0, -bigNumber);
               glVertex2f(0.0,  bigNumber);
            glEnd();
            break;
         case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
            return;
            break;
         case VolumeFile::VOLUME_AXIS_UNKNOWN:
            break;
      }
   }

   drawObliqueVolumeCellFile(axis,
                             brainSet->getVolumeCellFile(),
                             brainSet->getDisplaySettingsCells(),
                             brainSet->getCellColorFile(),
                             voxSize,
                             &tm,
                             sliceCornerCoords,
                             -1);

   drawObliqueVolumeFociFile(axis,
                             voxSize,
                             &tm,
                             sliceCornerCoords);
                       
   //
   // Draw transformation cell files
   //
   const int num = brainSet->getNumberOfTransformationDataFiles();
   for (int i = 0; i < num; i++) {
      AbstractFile* af = brainSet->getTransformationDataFile(i);
      
      FociFile* ff = dynamic_cast<FociFile*>(af);
      if (ff != NULL) {
/*
         const TransformationMatrix* fociXForm = ff->getAssociatedTransformationMatrix();
         if (fociXForm != NULL) {
            FociFile ffCopy = *ff;
            ffCopy.applyTransformationMatrix(-std::numeric_limits<int>::min(),
                                                std::numeric_limits<int>::max(),
                                                *fociXForm, 
                                                false);
            drawObliqueVolumeCellOrFociFile(axis,
                                            &ffCopy,
                                            brainSet->getDisplaySettingsFoci(),
                                            brainSet->getFociColorFile(),
                                            voxSize,
                                            &tm,
                                            sliceCornerCoords,
                                            i);
         }
*/
      }
      ContourCellFile* ccf = dynamic_cast<ContourCellFile*>(af);
      if ((ff == NULL) && (ccf != NULL)) {
         const TransformationMatrix* cellXForm1 = ccf->getAssociatedTransformationMatrix();
         if (cellXForm1 != NULL) {
            CellFile cfCopy = *ccf;
            TransformationMatrix cellXForm = *cellXForm1;
            cfCopy.applyTransformationMatrix(-std::numeric_limits<int>::min(),
                                                std::numeric_limits<int>::max(),
                                                cellXForm, 
                                                false);
            drawObliqueVolumeCellFile(axis,
                                      &cfCopy,
                                      brainSet->getDisplaySettingsCells(),
                                      brainSet->getContourCellColorFile(),
                                      voxSize,
                                      &tm,
                                      sliceCornerCoords,
                                      i);
         }
      }
      CellFile* cf = dynamic_cast<CellFile*>(af);
      if ((ff == NULL) && (ccf == NULL) && (cf != NULL)) {
         const TransformationMatrix* cellXForm1 = cf->getAssociatedTransformationMatrix();
         if (cellXForm1 != NULL) {
            CellFile cfCopy = *cf;
            TransformationMatrix cellXForm = *cellXForm1;
            cfCopy.applyTransformationMatrix(-std::numeric_limits<int>::min(),
                                                std::numeric_limits<int>::max(),
                                                cellXForm, 
                                                false);
            drawObliqueVolumeCellFile(axis,
                                      &cfCopy,
                                      brainSet->getDisplaySettingsCells(),
                                      brainSet->getCellColorFile(),
                                      voxSize,
                                      &tm,
                                      sliceCornerCoords,
                                      i);
         }
      }
      
      ContourFile* contourFile = dynamic_cast<ContourFile*>(af);
      if (contourFile != NULL) {
         const TransformationMatrix* contourXForm1 = contourFile->getAssociatedTransformationMatrix();
         if (contourXForm1 != NULL) {
            ContourFile cfCopy = *contourFile;
            TransformationMatrix contourXForm = *contourXForm1;
            cfCopy.applyTransformationMatrix(std::numeric_limits<int>::min(),
                                             std::numeric_limits<int>::max(),
                                             contourXForm,
                                             false);
            drawObliqueContourFile(axis,
                                   &cfCopy,
                                   voxSize,
                                   &tm,
                                   sliceCornerCoords);
         }
      }
   }
}
      
/**
 * Draw volume with all axis.
 */
void 
BrainModelOpenGL::drawBrainModelVolumeAllAxis(BrainModelVolume* bmv)
{
   glDisable(GL_DEPTH_TEST);
   
   //
   // Don't set projection matrix when selecting
   //
/*
   if (selectionMask == SELECTION_MASK_OFF) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      glOrtho(orthographicLeft[viewingWindowNumber], orthographicRight[viewingWindowNumber], 
              orthographicBottom[viewingWindowNumber], orthographicTop[viewingWindowNumber], 
              orthographicNear[viewingWindowNumber], orthographicFar[viewingWindowNumber]);
      
      glGetDoublev(GL_PROJECTION_MATRIX, selectionProjectionMatrix[viewingWindowNumber]);
   }
*/
   //
   // Get slice for drawing
   //
   int slices[3];
   bmv->getSelectedOrthogonalSlices(viewingWindowNumber, slices);

   //
   // Determine viewport height and width
   //
   const int vpHeight = viewport[3] / 2;
   const int vpWidth  = viewport[2] / 2;
   for (int j = 0; j < 2; j++) {
      for (int i = 0; i < 2; i++) {
         //
         // viewport X & Y
         //
         const int vpX = i * vpWidth;
         const int vpY = j * vpHeight;
         
         //
         // if selecting, only draw if mouse is in this viewport
         //
         if (selectionMask != SELECTION_MASK_OFF) {
            if ((selectionX < vpX) ||
                (selectionY < vpY) ||
                (selectionX > (vpX + vpWidth)) ||
                (selectionY > (vpY + vpHeight))) {
               continue;
            }
         }
   
         //
         // Set the viewport
         //
         glViewport(vpX, vpY, vpWidth, vpHeight);
         if (selectionMask == SELECTION_MASK_OFF) {
            glGetIntegerv(GL_VIEWPORT, selectionViewport[viewingWindowNumber]);
         }
         
         //
         // Set orthographic transform
         //
         if (selectionMask == SELECTION_MASK_OFF) {
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            glOrtho(orthographicLeft[viewingWindowNumber], orthographicRight[viewingWindowNumber], 
                 orthographicBottom[viewingWindowNumber], orthographicTop[viewingWindowNumber], 
                 orthographicNear[viewingWindowNumber], orthographicFar[viewingWindowNumber]);
         
            glGetDoublev(GL_PROJECTION_MATRIX, selectionProjectionMatrix[viewingWindowNumber]);
         }
         
         glMatrixMode(GL_MODELVIEW);
         glLoadIdentity();

         float translate[3];
         bmv->getTranslation(viewingWindowNumber, translate);
         glTranslatef(translate[0], translate[1], translate[2]);
         
         glRotatef(bmv->getDisplayRotation(viewingWindowNumber), 0.0, 0.0, 1.0);
         float scale[3];
         bmv->getScaling(viewingWindowNumber, scale);
         glScalef(scale[0], scale[1], scale[2]);
         
         //
         // Save modeling matrix
         //
         if (selectionMask == SELECTION_MASK_OFF) {
            glGetDoublev(GL_MODELVIEW_MATRIX, selectionModelviewMatrix[viewingWindowNumber]);
         }
         
         VolumeFile::VOLUME_AXIS volumeSliceAxis = VolumeFile::VOLUME_AXIS_ALL;
         VolumeFile* firstVolume = NULL;
         if (i == 0) {
            if (j == 0) {
               VolumeFile* vf = bmv->getUnderlayVolumeFile();
               if (vf == NULL) {
                  vf = bmv->getOverlaySecondaryVolumeFile();
               }
               if (vf == NULL) {
                  vf = bmv->getOverlayPrimaryVolumeFile();
               }
               //
               // Draw the crosshair coordinates
               //
               if (vf != NULL) {
                  drawVolumeCrosshairCoordinates(bmv, vf, vpHeight);
               }
               
               BrainModelSurface* bms = brainSet->getActiveFiducialSurface();
               if (bms != NULL) {
                  glEnable(GL_DEPTH_TEST);
                  drawBrainModelSurface(bms, NULL, true, true);
               
                  //
                  // Draw the volume slices
                  //
                  if (vf != NULL) {
                     int dim[3];
                     vf->getDimensions(dim);
                     float originCenter[3], originCorner[3], spacing[3];
                     vf->getOrigin(originCenter);
                     vf->getOriginAtCornerOfVoxel(originCorner);
                     vf->getSpacing(spacing);
                     const float minX = originCorner[0];
                     const float maxX = originCorner[0] + spacing[0] * dim[0];
                     const float minY = originCorner[1];
                     const float maxY = originCorner[1] + spacing[1] * dim[1];
                     const float minZ = originCorner[2];
                     const float maxZ = originCorner[2] + spacing[2] * dim[2];
                     const float sliceX = originCenter[0] + spacing[0] * slices[0];
                     const float sliceY = originCenter[1] + spacing[1] * slices[1];
                     const float sliceZ = originCenter[2] + spacing[2] * slices[2];

                     glEnable(GL_BLEND);
                     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                     glColor4ub(190, 190, 190, 130);

                     glBegin(GL_TRIANGLES);
                        glVertex3f(sliceX, minY, minZ);
                        glVertex3f(sliceX, maxY, minZ);
                        glVertex3f(sliceX, maxY, maxZ);
                        glVertex3f(sliceX, minY, minZ);
                        glVertex3f(sliceX, maxY, maxZ);
                        glVertex3f(sliceX, minY, maxZ);

                        glVertex3f(minX, sliceY, minZ);
                        glVertex3f(maxX, sliceY, minZ);
                        glVertex3f(maxX, sliceY, maxZ);
                        glVertex3f(minX, sliceY, minZ);
                        glVertex3f(maxX, sliceY, maxZ);
                        glVertex3f(minX, sliceY, maxZ);
                        
                        glVertex3f(minX, minY, sliceZ);
                        glVertex3f(maxX, minY, sliceZ);
                        glVertex3f(maxX, maxY, sliceZ);
                        glVertex3f(minX, minY, sliceZ);
                        glVertex3f(maxX, maxY, sliceZ);
                        glVertex3f(minX, maxY, sliceZ);
                     glEnd();
                     
                     glDisable(GL_BLEND);
                  }
               
                  glDisable(GL_DEPTH_TEST);
               }
               
            }
            else {
               //
               // Draw parasaggital
               //
               volumeSliceAxis = VolumeFile::VOLUME_AXIS_X;
               drawVolumeSliceOverlayAndUnderlay(bmv, VolumeFile::VOLUME_AXIS_X, 
                                                 slices[0], firstVolume);
            }
         }
         else {
            if (j == 0) {
               //
               // Draw horizontal
               //
               volumeSliceAxis = VolumeFile::VOLUME_AXIS_Z;
               drawVolumeSliceOverlayAndUnderlay(bmv, VolumeFile::VOLUME_AXIS_Z, 
                                                 slices[2], firstVolume);
            }
            else {
               //
               // Draw coronal
               //
               volumeSliceAxis = VolumeFile::VOLUME_AXIS_Y;
               drawVolumeSliceOverlayAndUnderlay(bmv, VolumeFile::VOLUME_AXIS_Y, 
                                                 slices[1], firstVolume);
            }
         }
         
         if (firstVolume != NULL) {
            //
            // Draw the crosshairs
            //
            drawVolumeCrosshairs(bmv, firstVolume, volumeSliceAxis);
            
            //
            // Draw the cropping lines
            //
            drawVolumeCroppingLines(bmv, firstVolume, volumeSliceAxis);            
         }
      }
   }
         
   glEnable(GL_DEPTH_TEST);
}      

/**
 * Draw volume crosshairs
 */
void
BrainModelOpenGL::drawVolumeCrosshairs(BrainModelVolume* bmv,
                                       const VolumeFile* vf,
                                       const VolumeFile::VOLUME_AXIS volumeSliceAxis)
{
   DisplaySettingsVolume* dsv = brainSet->getDisplaySettingsVolume();
   
   //
   // Draw the crosshairs
   //
   if ((vf != NULL) && 
       (selectionMask == SELECTION_MASK_OFF) &&
       (dsv->getCroppingSlicesValid() == false)) {
      //
      // Draw the crosshairs
      //
      if (dsv->getDisplayCrosshairs()) {
         unsigned char red[3]   = { 255, 0, 0 };
         unsigned char green[3] = { 0, 255, 0 };
         unsigned char blue[3]  = { 0, 0, 255 };

         //
         // Get slice for drawing
         //
         int slices[3];
         bmv->getSelectedOrthogonalSlices(viewingWindowNumber, slices);
         float crosshairX = 0;
         float crosshairY = 0;
         
         //
         // Coordinate
         //
         float xyz[3];
         vf->getVoxelCoordinate(slices,
                                xyz);
         unsigned char* xColor = red;
         unsigned char* yColor = red;
         switch(volumeSliceAxis) {
            case VolumeFile::VOLUME_AXIS_X:  // PARASAGITTAL
               crosshairX = xyz[1]; //slices[1] * spacing[1] + origin[1];
               xColor = green;
               crosshairY = xyz[2]; //slices[2] * spacing[2] + origin[2];
               yColor = blue;
               break;
            case VolumeFile::VOLUME_AXIS_Y:  // CORONAL
               crosshairX = xyz[0]; //slices[0] * spacing[0] + origin[0];
               xColor = red;
               crosshairY = xyz[2]; //slices[2] * spacing[2] + origin[2];
               yColor = blue;
               break;
            case VolumeFile::VOLUME_AXIS_Z:  // HORIZONTAL
               crosshairX = xyz[0]; //slices[0] * spacing[0] + origin[0];
               xColor = red;
               crosshairY = xyz[1]; //slices[1] * spacing[1] + origin[1];
               yColor = green;
               break;
            case VolumeFile::VOLUME_AXIS_ALL:
            case VolumeFile::VOLUME_AXIS_OBLIQUE:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
            case VolumeFile::VOLUME_AXIS_UNKNOWN:
               break;
         }
         const float bigNumber = 10000;
         glLineWidth(getValidLineWidth(1.0));
         glColor3ubv(yColor);
         glBegin(GL_LINES);
            glVertex3f(crosshairX, -bigNumber, 0.0);
            glVertex3f(crosshairX,  bigNumber, 0.0);
         glEnd();
         glColor3ubv(xColor);
         glBegin(GL_LINES);
            glVertex3f(-bigNumber, crosshairY, 0.0);
            glVertex3f( bigNumber, crosshairY, 0.0);
         glEnd();
      }
   }
}

/**
 * Draw the volume cropping lines
 */
void
BrainModelOpenGL::drawVolumeCroppingLines(BrainModelVolume* bmv,
                                             VolumeFile* vf,
                                             VolumeFile::VOLUME_AXIS volumeSliceAxis)
{
   DisplaySettingsVolume* dsv = brainSet->getDisplaySettingsVolume();
   
   //
   // Draw the cropping volume is cropping dialog valid and this is the underlay volume
   //
   if (selectionMask == SELECTION_MASK_OFF) {
      VolumeFile* underlayVolume = bmv->getUnderlayVolumeFile();
      if (underlayVolume != NULL) {
         if (dsv->getCroppingSlicesValid()) {
            int croppingLines[6];
            dsv->getCroppingSlices(croppingLines);
            float origin[3];
            float spacing[3];
            vf->getOrigin(origin);
            vf->getSpacing(spacing);
            float crosshairX1 = 0;
            float crosshairX2 = 0;
            float crosshairY1 = 0;
            float crosshairY2 = 0;
            switch(volumeSliceAxis) {
               case VolumeFile::VOLUME_AXIS_X:  // PARASAGITTAL
                  crosshairX1 = croppingLines[2] * spacing[1] + origin[1];
                  crosshairX2 = croppingLines[3] * spacing[1] + origin[1];
                  crosshairY1 = croppingLines[4] * spacing[2] + origin[2];
                  crosshairY2 = croppingLines[5] * spacing[2] + origin[2];
                  break;
               case VolumeFile::VOLUME_AXIS_Y:  // CORONAL
                  crosshairX1 = croppingLines[0] * spacing[0] + origin[0];
                  crosshairX2 = croppingLines[1] * spacing[0] + origin[0];
                  crosshairY1 = croppingLines[4] * spacing[2] + origin[2];
                  crosshairY2 = croppingLines[5] * spacing[2] + origin[2];
                  break;
               case VolumeFile::VOLUME_AXIS_Z:  // HORIZONTAL
                  crosshairX1 = croppingLines[0] * spacing[0] + origin[0];
                  crosshairX2 = croppingLines[1] * spacing[0] + origin[0];
                  crosshairY1 = croppingLines[2] * spacing[1] + origin[1];
                  crosshairY2 = croppingLines[3] * spacing[1] + origin[1];
                  break;
               case VolumeFile::VOLUME_AXIS_ALL:
               case VolumeFile::VOLUME_AXIS_OBLIQUE:
               case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
               case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
               case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
               case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
               case VolumeFile::VOLUME_AXIS_UNKNOWN:
                  break;
            }
            const float bigNumber = 10000;
            glLineWidth(getValidLineWidth(1.0));
            glBegin(GL_LINES);
               glColor3ub(0, 150, 150);
               glVertex3f(crosshairX1, -bigNumber, 0.0);
               glVertex3f(crosshairX1,  bigNumber, 0.0);
               glColor3ub(0, 255, 255);
               glVertex3f(crosshairX2, -bigNumber, 0.0);
               glVertex3f(crosshairX2,  bigNumber, 0.0);
            glEnd();
            glBegin(GL_LINES);
               glColor3ub(150, 150, 0);
               glVertex3f(-bigNumber, crosshairY1, 0.0);
               glVertex3f( bigNumber, crosshairY1, 0.0);
               glColor3ub(255, 255, 0);
               glVertex3f(-bigNumber, crosshairY2, 0.0);
               glVertex3f( bigNumber, crosshairY2, 0.0);
            glEnd();
         }
      }
   }   
}

/**
 * Draw volume crosshair coordinates
 */
void
BrainModelOpenGL::drawVolumeCrosshairCoordinates(BrainModelVolume* bmv,
                                                    const VolumeFile* vf,
                                                    const int viewportHeight)
{
   DisplaySettingsVolume* dsv = brainSet->getDisplaySettingsVolume();
   
   //
   // Draw the coordinate's of the crosshairs
   //
   if (dsv->getDisplayCrosshairCoordinates() && (glWidget != NULL)) {
      //
      // Draw in the foreground color
      //
      PreferencesFile* pf = brainSet->getPreferencesFile();
      unsigned char rf, gf, bf;
      pf->getSurfaceForegroundColor(rf, gf, bf);
      glColor3ub(rf, gf, bf);

      //
      // Font for drawing numbers
      //
      const int fontHeight = 12;
      QFont font("times", fontHeight);
         
      //
      // Get slice for drawing
      //
      int slices[3];
      bmv->getSelectedOrthogonalSlices(viewingWindowNumber, slices);

      //
      // Determine the crosshair coordinates
      //
      float xyz[3];
      vf->getVoxelCoordinate(slices, 
                             xyz);
      
      //
      // Draw the text (Note: this renderText() method is done in window coordinates
      // with the origin in the top left corner of the window).
      //
      QString s;
      s.sprintf("(%0.2f, %0.2f, %0.2f)", xyz[0], xyz[1], xyz[2]); 
      if (glWidget != NULL) {
         if (DebugControl::getOpenGLDebugOn()) {
            checkForOpenGLError(bmv, "Before renderText() in drawVolumeCrosshairCoordinates");
         }
         glWidget->renderText(10, viewportHeight - 15, s, font);
         if (DebugControl::getOpenGLDebugOn()) {
            checkForOpenGLError(bmv, "After renderText() in drawVolumeCrosshairCoordinates");
         }
      }
   }
}

/**
 * draw volume in montage.
 */
void 
BrainModelOpenGL::drawBrainModelVolumeMontage(BrainModelVolume* bmv)
{
   const DisplaySettingsVolume* dsv = brainSet->getDisplaySettingsVolume();
   glDisable(GL_DEPTH_TEST);
   
   //
   // Don't set projection matrix when selecting
   //
   if (selectionMask == SELECTION_MASK_OFF) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      glOrtho(orthographicLeft[viewingWindowNumber], orthographicRight[viewingWindowNumber], 
              orthographicBottom[viewingWindowNumber], orthographicTop[viewingWindowNumber], 
              orthographicNear[viewingWindowNumber], orthographicFar[viewingWindowNumber]);
      
      glGetDoublev(GL_PROJECTION_MATRIX, selectionProjectionMatrix[viewingWindowNumber]);
   }

   //
   // Axis to be drawn
   //
   VolumeFile::VOLUME_AXIS volumeSliceAxis = bmv->getSelectedAxis(viewingWindowNumber);
         
   //
   // Get slice for drawing
   //
   int slices[3];
   bmv->getSelectedOrthogonalSlices(viewingWindowNumber, slices);
   int currentSlice;
   switch(volumeSliceAxis) {
      case VolumeFile::VOLUME_AXIS_X:  // PARASAGITTAL
         currentSlice = slices[0];
         break;
      case VolumeFile::VOLUME_AXIS_Y:  // CORONAL
         currentSlice = slices[1];
         break;
      case VolumeFile::VOLUME_AXIS_Z:  // HORIZONTAL
         currentSlice = slices[2];
         break;
      default:
         std::cout << "PROGRAM ERROR: Invalid volume axis at " << __LINE__ << " in " 
                   << __FILE__ << std::endl;
         return;
   }
   
   //
   // Get montage info 
   //
   int rows, columns, sliceIncrement;
   dsv->getMontageViewSettings(rows, columns, sliceIncrement);

   //
   // Determine viewport height and width
   //
   const int vpHeight = viewport[3] / rows;
   const int vpWidth  = viewport[2] / columns;
   for (int i = (rows - 1); i >= 0; i--) {
      for (int j = 0; j < columns; j++) {
      //for (int i = 0; i < rows; i++) {
         const int vpX = j * vpWidth;
         const int vpY = i * vpHeight;
         
         //
         // if selecting, only draw if mouse is in this viewport
         //
         bool drawSlice = true;
         if (selectionMask != SELECTION_MASK_OFF) {
            if ((selectionX < vpX) ||
                (selectionY < vpY) ||
                (selectionX > (vpX + vpWidth)) ||
                (selectionY > (vpY + vpHeight))) {
               drawSlice = false;
            }
         }
         
         if (drawSlice) {
            //
            // Set the viewport
            //
            glViewport(vpX, vpY, vpWidth, vpHeight);
            
            //
            // Don't set projection matrix when selecting
            //
            if (selectionMask == SELECTION_MASK_OFF) {
               //
               // Set orthographic transform
               //
               glMatrixMode(GL_PROJECTION);
               glLoadIdentity();

               glOrtho(orthographicLeft[viewingWindowNumber], orthographicRight[viewingWindowNumber], 
                       orthographicBottom[viewingWindowNumber], orthographicTop[viewingWindowNumber], 
                       orthographicNear[viewingWindowNumber], orthographicFar[viewingWindowNumber]);
               
               glGetDoublev(GL_PROJECTION_MATRIX, selectionProjectionMatrix[viewingWindowNumber]);
            }
            
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            float translate[3];
            bmv->getTranslation(viewingWindowNumber, translate);
            glTranslatef(translate[0], translate[1], translate[2]);
            
            glRotatef(bmv->getDisplayRotation(viewingWindowNumber), 0.0, 0.0, 1.0);
            float scale[3];
            bmv->getScaling(viewingWindowNumber, scale);
            glScalef(scale[0], scale[1], scale[2]);
            
            //
            // Save modeling matrix
            //
            if (selectionMask == SELECTION_MASK_OFF) {
               glGetDoublev(GL_MODELVIEW_MATRIX, selectionModelviewMatrix[viewingWindowNumber]);
            }
            
            VolumeFile* firstVolume = NULL;
            drawVolumeSliceOverlayAndUnderlay(bmv, volumeSliceAxis, currentSlice, firstVolume);
         }
         
         //
         // Move to next slice
         //
         currentSlice += sliceIncrement;
      }
   }
      
   glEnable(GL_DEPTH_TEST);
}      

/**
 * draw volume.
 */
void 
BrainModelOpenGL::drawBrainModelVolume(BrainModelVolume* bmv)
{
   if (DebugControl::getOpenGLDebugOn()) {
      checkForOpenGLError(bmv, "Beginning of drawBrainModelVolume()");
   }

   const DisplaySettingsVolume* dsv = brainSet->getDisplaySettingsVolume();
   switch (bmv->getSelectedAxis(viewingWindowNumber)) {
      case VolumeFile::VOLUME_AXIS_X:
         break;
      case VolumeFile::VOLUME_AXIS_Y:
         break;
      case VolumeFile::VOLUME_AXIS_Z:
         break;
      case VolumeFile::VOLUME_AXIS_ALL:
         drawBrainModelVolumeAllAxis(bmv);
         return;
         break;
      case VolumeFile::VOLUME_AXIS_OBLIQUE:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
         drawBrainModelVolumeObliqueAxis(bmv);
         drawMetricPalette(0, false);
         return;
         break;
      case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
         drawBrainModelVolumeObliqueAllAxis(bmv);
         return;
         break;
      case VolumeFile::VOLUME_AXIS_UNKNOWN:
         std::cout << "PROGRAM ERROR: UNKNOWN axis for drawing." << std::endl;
         return;
         break;
   }
   if (dsv->getMontageViewSelected()) {
      drawBrainModelVolumeMontage(bmv);
      return;
   }
   
   glDisable(GL_DEPTH_TEST);
   
   //
   // Don't set projection matrix when selecting
   //
   if (selectionMask == SELECTION_MASK_OFF) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      glOrtho(orthographicLeft[viewingWindowNumber], orthographicRight[viewingWindowNumber], 
              orthographicBottom[viewingWindowNumber], orthographicTop[viewingWindowNumber], 
              orthographicNear[viewingWindowNumber], orthographicFar[viewingWindowNumber]);
      
      glGetDoublev(GL_PROJECTION_MATRIX, selectionProjectionMatrix[viewingWindowNumber]);
   }

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   float translate[3];
   bmv->getTranslation(viewingWindowNumber, translate);
   glTranslatef(translate[0], translate[1], translate[2]);
   
   glRotatef(bmv->getDisplayRotation(viewingWindowNumber), 0.0, 0.0, 1.0);
   float scale[3];
   bmv->getScaling(viewingWindowNumber, scale);
   glScalef(scale[0], scale[1], scale[2]);
   
   //
   // Save modeling matrix
   //
   if (selectionMask == SELECTION_MASK_OFF) {
      glGetDoublev(GL_MODELVIEW_MATRIX, selectionModelviewMatrix[viewingWindowNumber]);
   }
   
   //
   // Axis to be drawn
   //
   VolumeFile::VOLUME_AXIS volumeSliceAxis = bmv->getSelectedAxis(viewingWindowNumber);
   
   //
   // Get slice for drawing
   //
   int slices[3];
   bmv->getSelectedOrthogonalSlices(viewingWindowNumber, slices);
   int currentSlice;
   switch(volumeSliceAxis) {
      case VolumeFile::VOLUME_AXIS_X:  // PARASAGITTAL
         currentSlice = slices[0];
         break;
      case VolumeFile::VOLUME_AXIS_Y:  // CORONAL
         currentSlice = slices[1];
         break;
      case VolumeFile::VOLUME_AXIS_Z:  // HORIZONTAL
         currentSlice = slices[2];
         break;
      default:
         std::cout << "PROGRAM ERROR: Invalid volume axis at " << __LINE__ << " in " 
                   << __FILE__ << std::endl;
         return;
   }
   
   if (DebugControl::getOpenGLDebugOn()) {
      checkForOpenGLError(bmv, "Before drawVolumeSliceOverlayAndUnderlay");
   }
   VolumeFile* firstVolume = NULL;
   drawVolumeSliceOverlayAndUnderlay(bmv, volumeSliceAxis, currentSlice, firstVolume);
   if (DebugControl::getOpenGLDebugOn()) {
      checkForOpenGLError(bmv, "After drawVolumeSliceOverlayAndUnderlay");
   }
   
   //
   // Draw the palette
   //
   drawMetricPalette(0, false);
   if (DebugControl::getOpenGLDebugOn()) {
      checkForOpenGLError(bmv, "After drawing palette");
   }
   
   //
   // Draw the cropping volume is cropping dialog valid and this is the underlay volume
   //
   bool croppingLinesDisplayed = false;
   if (selectionMask == SELECTION_MASK_OFF) {
      VolumeFile* underlayVolume = bmv->getUnderlayVolumeFile();
      if (underlayVolume != NULL) {
         drawVolumeCroppingLines(bmv, underlayVolume, volumeSliceAxis);
      }
   }
   
   //
   // Draw the crosshairs
   //
   if ((firstVolume != NULL) && 
       (selectionMask == SELECTION_MASK_OFF) &&
       (croppingLinesDisplayed == false)) {
      //
      // Draw the crosshairs and coordinates
      //
      drawVolumeCrosshairs(bmv, firstVolume, volumeSliceAxis);
      if (DebugControl::getOpenGLDebugOn()) {
         checkForOpenGLError(bmv, "After drawVolumeCrosshairs");
      }
      drawVolumeCrosshairCoordinates(bmv, firstVolume, viewport[3]);
      if (DebugControl::getOpenGLDebugOn()) {
         checkForOpenGLError(bmv, "After drawVolumeCrosshairCoordinates");
      }
   }
     
   if ((selectionMask == SELECTION_MASK_OFF) && (glWidget != NULL)) {
      if (dsv->getDisplayOrientationLabels()) {
         //
         // Orientation labels
         //
         QString orientLeftSideLabel;
         QString orientRightSideLabel;
         QString orientBottomSideLabel;
         QString orientTopSideLabel;
         switch(volumeSliceAxis) {
            case VolumeFile::VOLUME_AXIS_X:  // PARASAGITTAL
               orientLeftSideLabel   = "P";
               orientRightSideLabel  = "A";
               orientBottomSideLabel = "V";
               orientTopSideLabel    = "D";
               break;
            case VolumeFile::VOLUME_AXIS_Y:  // CORONAL
               orientLeftSideLabel   = "L";
               orientRightSideLabel  = "R";
               orientBottomSideLabel = "V";
               orientTopSideLabel    = "D";
               break;
            case VolumeFile::VOLUME_AXIS_Z:  // HORIZONTAL
               orientLeftSideLabel   = "L";
               orientRightSideLabel  = "R";
               orientBottomSideLabel = "P";
               orientTopSideLabel    = "A";
               break;
            case VolumeFile::VOLUME_AXIS_ALL:
            case VolumeFile::VOLUME_AXIS_OBLIQUE:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
            case VolumeFile::VOLUME_AXIS_UNKNOWN:
               break;
         }
      
         //
         // Font for drawing orientation labels
         //
         QFont font("times", 16);
         font.setBold(true);
         QFontMetrics fm(font);
         const int fontWidth = fm.width(orientBottomSideLabel) / 2;
         const int fontHeight = fm.height();
               
         const int halfX = viewport[2] / 2;
         const int halfY = (viewport[3] / 2) + (fontHeight / 2);

         //
         // Draw orientation labels
         //
         glColor3ub(0, 255, 255);
         if (glWidget != NULL) {
            glWidget->renderText((fontWidth / 2), halfY, 
                       orientLeftSideLabel, font);
            glWidget->renderText(viewport[2] - (fontWidth * 3), halfY, 
                       orientRightSideLabel, font);
            glWidget->renderText(halfX - (fontWidth / 2), static_cast<int>(viewport[3] - fontHeight * 1.5), 
                       orientBottomSideLabel, font);
            glWidget->renderText(halfX - (fontWidth / 2), static_cast<int>(fontHeight * 1.5),
                       orientTopSideLabel, font);
            if (DebugControl::getOpenGLDebugOn()) {
               checkForOpenGLError(bmv, "After drawing orientation labels");
            }
         }
      }
   }
   
   //
   // If this flag is set we only want to draw the linear object.  When this
   // flag is set, glClear() is not called so the previous stuff is not erased.
   //
   if (drawLinearObjectOnly) {
      drawLinearObject();
   }
   
   glEnable(GL_DEPTH_TEST);
}

/** 
 * Draw a surface outline over the volume.
 */
void
BrainModelOpenGL::drawVolumeSurfaceOutlineAndTransformationMatrixAxes(
                                           const BrainModelVolume* bmv,
                                           const VolumeFile::VOLUME_AXIS axis,
                                           const float axisCoord)
{
   DisplaySettingsVolume* dsv = brainSet->getDisplaySettingsVolume();
   
   glPushMatrix();

   for (int surfNum = 0; surfNum < DisplaySettingsVolume::MAXIMUM_OVERLAY_SURFACES; surfNum++) {
      float thickness = dsv->getOverlaySurfaceOutlineThickness(surfNum);
      BrainModelSurface* bms = dsv->getOverlaySurface(surfNum);
      bool drawSurfaceOutline = dsv->getDisplayOverlaySurfaceOutline(surfNum);
      DisplaySettingsVolume::SURFACE_OUTLINE_COLOR surfaceOutlineColor = 
                             dsv->getOverlaySurfaceOutlineColor(surfNum);
      //
      // No selections for outline surface
      //
      if (selectionMask != SELECTION_MASK_OFF) {
         drawSurfaceOutline = false;
      }
      
      //
      // Find the fiducial surface
      //
      if (bms == NULL) {
         drawSurfaceOutline = false;
      }
         
      //
      // Create and enable the clipping planes to only draw
      // when (-thickness <= Z <= thickness)
      //
      GLdouble clipPlanePositive[4] = {  // should clip (Z < -thickness)
                                         0.0,
                                         0.0,
                                        -1.0,  //-thickness,
                                         thickness
                                      };
      GLdouble clipPlaneNegative[4] = {   // should clip (Z > thickness)
                                         0.0,
                                         0.0,
                                         1.0, // thickness,
                                         thickness
                                      };
      glClipPlane(GL_CLIP_PLANE0, clipPlanePositive);
      glClipPlane(GL_CLIP_PLANE1, clipPlaneNegative);
      glEnable(GL_CLIP_PLANE0);
      glEnable(GL_CLIP_PLANE1);

      //
      // Since transformations are used for drawing transform data files
      // only modify transforms on last overlay surface drawn
      //
      if (surfNum < (DisplaySettingsVolume::MAXIMUM_OVERLAY_SURFACES - 1)) {
         glPushMatrix();
      }
      
      //
      // Transform the surface as needed
      //
      switch(axis) {
         case VolumeFile::VOLUME_AXIS_X:  // PARASAGITTAL
            glRotatef(-90.0, 0.0, 1.0, 0.0);
            glRotatef(-90.0, 1.0, 0.0, 0.0);
            glTranslatef(-axisCoord, 0.0, 0.0);
            break;
         case VolumeFile::VOLUME_AXIS_Y:  // CORONAL
            glRotatef(-90.0, 1.0, 0.0, 0.0);
            glTranslatef(0.0, -axisCoord, 0.0);
            break;
         case VolumeFile::VOLUME_AXIS_Z:  // HORIZONTAL
            glTranslatef(0.0, 0.0, -axisCoord);
            break;
         case VolumeFile::VOLUME_AXIS_ALL:
            break;
         case VolumeFile::VOLUME_AXIS_OBLIQUE:
         case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
         case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
         case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
         case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
         case VolumeFile::VOLUME_AXIS_UNKNOWN:
            break;
      }

      //
      // Should surface outline be shown ?
      //   
      if (drawSurfaceOutline) {
         //
         // color for surface
         //
         switch (surfaceOutlineColor) {
            case DisplaySettingsVolume::SURFACE_OUTLINE_COLOR_BLACK:
               glColor3f(0.0, 0.0, 0.0);
               break;
            case DisplaySettingsVolume::SURFACE_OUTLINE_COLOR_BLUE:
               glColor3f(0.0, 0.0, 1.0);
               break;
            case DisplaySettingsVolume::SURFACE_OUTLINE_COLOR_GREEN:
               glColor3f(0.0, 1.0, 0.0);
               break;
            case DisplaySettingsVolume::SURFACE_OUTLINE_COLOR_RED:
               glColor3f(1.0, 0.0, 0.0);
               break;
            case DisplaySettingsVolume::SURFACE_OUTLINE_COLOR_WHITE:
               glColor3f(1.0, 1.0, 1.0);
               break;
         }

         CoordinateFile* cf = bms->getCoordinateFile();
         TopologyFile*   tf = bms->getTopologyFile();
         const int numTiles = tf->getNumberOfTiles();
         
   //#define DRAW_AS_LINKS
   #ifdef DRAW_AS_LINKS
         glLineWidth(getValidLineWidth(1.0));
         glBegin(GL_LINES);
            for (int i = 0; i < numTiles; i++) {
               int v1, v2, v3;
               tf->getTile(i, v1, v2, v3);
               glVertex3fv(cf->getCoordinate(v1));
               glVertex3fv(cf->getCoordinate(v2));
               glVertex3fv(cf->getCoordinate(v2));
               glVertex3fv(cf->getCoordinate(v3));
               glVertex3fv(cf->getCoordinate(v1));
               glVertex3fv(cf->getCoordinate(v3));
            }
         glEnd();
   #else  // DRAW_AS_LINKS
   #ifdef GL_VERSION_1_1
         glEnableClientState(GL_VERTEX_ARRAY);
         glVertexPointer(3, GL_FLOAT, 0, cf->getCoordinate(0));
         glDrawElements(GL_TRIANGLES, (3 * numTiles), GL_UNSIGNED_INT, 
                           static_cast<const GLvoid*>(tf->getTile(0)));
         glDisableClientState(GL_VERTEX_ARRAY);
   #else  // GL_VERSION_1_1
         glBegin(GL_TRIANGLES);
            for (int i = 0; i < numTiles; i++) {
               unsigned int v1, v2, v3;
               tf->getTile(i, v1, v2, v3);
               glVertex3fv(cf->getCoordinate(v1));
               glVertex3fv(cf->getCoordinate(v2));
               glVertex3fv(cf->getCoordinate(v3));
            }
         glEnd();
   #endif  // GL_VERSION_1_1
   #endif // DRAW_AS_LINKS
      } // draw surface outline
      
      //
      // Since transformations are used for drawing transform data files
      // only modify transforms on last overlay surface drawn
      //
      if (surfNum < (DisplaySettingsVolume::MAXIMUM_OVERLAY_SURFACES - 1)) {
         glPopMatrix();
      }

      glDisable(GL_CLIP_PLANE0);
      glDisable(GL_CLIP_PLANE1);
   }
   
   //
   // Draw the translation axes
   //
   drawTransformationMatrixAxes(bmv);
   
   glPopMatrix();
}

/**
 * Convert from border XYZ to screen XYZ
 */
void
BrainModelOpenGL::convertVolumeItemXYZToScreenXY(const VolumeFile::VOLUME_AXIS axis,
                                  float xyz[3])
{
   const float zPos = 1.0;
   float xyzOut[3] = { 0.0, 0.0, 0.0 };
   switch (axis) {
      case VolumeFile::VOLUME_AXIS_X:
         xyzOut[0] = xyz[1];
         xyzOut[1] = xyz[2];
         xyzOut[2] = xyz[0];
         break;
      case VolumeFile::VOLUME_AXIS_Y:
         xyzOut[0] = xyz[0];
         xyzOut[1] = xyz[2];
         xyzOut[2] = xyz[1];
         break;
      case VolumeFile::VOLUME_AXIS_Z:
         xyzOut[0] = xyz[0];
         xyzOut[1] = xyz[1];
         xyzOut[2] = xyz[2];
         break;
      case VolumeFile::VOLUME_AXIS_OBLIQUE:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
      case VolumeFile::VOLUME_AXIS_ALL:
      case VolumeFile::VOLUME_AXIS_UNKNOWN:
         break;
   }
   xyz[0] = xyzOut[0];
   xyz[1] = xyzOut[1];
   xyz[2] = zPos;
}

/**
 * Draw the oblique volume cell file.
 * If "transformDataFileIndex" is greater than or equal to zero, the a 
 * transform data (cell or foci) file is being drawn.
 */
void 
BrainModelOpenGL::drawObliqueVolumeCellFile(const VolumeFile::VOLUME_AXIS axis,
                                            const CellFile* cf,
                                            const DisplaySettingsCells* dsc,
                                            const ColorFile* colorFile,
                                            const float voxelSize,
                                            const TransformationMatrix* tm,
                                            const float sliceCornerCoords[4][3],
                                            const int transformDataFileIndex)
{
   if (transformDataFileIndex < 0) {
      if (dsc->getDisplayVolumeCells() == false) {
         return;
      }
   }
   
   
   //
   // Set color for cells/foci with missing colors
   //
   unsigned char noColorRed, noColorGreen, noColorBlue;
   PreferencesFile* pref = brainSet->getPreferencesFile();
   pref->getSurfaceForegroundColor(noColorRed, noColorGreen, noColorBlue);

   const float halfVoxelSize = voxelSize * 0.6;
   const bool contourCellFlag = (dynamic_cast<const ContourCellFile*>(cf) != NULL);
   
   //
   // Check for selection mode
   //
   bool selectFlag = false;
   if (contourCellFlag) {
      if (transformDataFileIndex >= 0) {
         if (selectionMask & SELECTION_MASK_TRANSFORM_CONTOUR_CELL) {
            glPushName(SELECTION_MASK_TRANSFORM_CONTOUR_CELL);
            glPushName(transformDataFileIndex);
            selectFlag = true;
         }
      }
   }
   else {
      if (transformDataFileIndex >= 0) {
         if (selectionMask & SELECTION_MASK_TRANSFORM_CELL) {
            glPushName(SELECTION_MASK_TRANSFORM_CELL);
            glPushName(transformDataFileIndex);
            selectFlag = true;
         }
      }
      else {
         if (selectionMask & SELECTION_MASK_VOLUME_CELL) {
            glPushName(SELECTION_MASK_VOLUME_CELL);
            selectFlag = true;
         }
      }
   }
   if ((selectFlag == false) && (selectionMask != SELECTION_MASK_OFF)) {
      return;
   }
   
   //
   // Normal of plane
   //
   float planeNormal[3];
   MathUtilities::computeNormal((float*)sliceCornerCoords[0],
                              (float*)sliceCornerCoords[1],
                              (float*)sliceCornerCoords[2],
                              planeNormal);
                              
   //
   // Draw cells larger when selecting on a flat surface
   //
   float cellSize = dsc->getDrawSize();

   const int numCells = cf->getNumberOfCells();

   if (numCells > 0) {
      const int numColors = colorFile->getNumberOfColors();
      for (int i = 0; i < numCells; i++) {
         const CellData* cd = cf->getCell(i);
         
         if (transformDataFileIndex < 0) {
            if (cd->getDisplayFlag() == false) {
               continue;
            }
         }
                  
         ColorFile::ColorStorage::SYMBOL symbol = ColorFile::ColorStorage::SYMBOL_OPENGL_POINT;
         const int colorIndex = cd->getColorIndex();
         float pointSize = 1;
         float lineSize  = 1;
         unsigned char r = 0, g = 0, b = 0, alpha = 255;
         if ((colorIndex >= 0) && (colorIndex < numColors)) {
            const ColorFile::ColorStorage* cs = colorFile->getColor(colorIndex);
            cs->getRgba(r, g, b, alpha);
            lineSize = cs->getLineSize();
            pointSize = cs->getPointSize();
            symbol = cs->getSymbol();
         }
         else {
            r = noColorRed;
            g = noColorGreen;
            b = noColorBlue;
         }
         
         if (dsc->getSymbolOverride() != ColorFile::ColorStorage::SYMBOL_NONE) {
            symbol = dsc->getSymbolOverride();
         }
         if (pointSize < 1) {
            pointSize = 1;
         }
         
         float size = pointSize * cellSize;
         
         float xyz[3];
         cd->getXYZ(xyz);
         
         //
         // Get distance and point on plane
         //
         float intersection[3];
         float signedDistanceFromPlaneOut;
         bool valid = MathUtilities::rayIntersectPlane(sliceCornerCoords[0],
                                          sliceCornerCoords[1],
                                          sliceCornerCoords[2],
                                          xyz,
                                          planeNormal,
                                          intersection,
                                          &signedDistanceFromPlaneOut);
         if (signedDistanceFromPlaneOut < 0.0) {
            signedDistanceFromPlaneOut = -signedDistanceFromPlaneOut;
         }
         if (valid) {
            if (signedDistanceFromPlaneOut > halfVoxelSize) {
               valid = false;
            }
         }
         if (valid) {
            //
            // Undo screen to slice transformation
            //
            xyz[0] = intersection[0];
            xyz[1] = intersection[1];
            xyz[2] = intersection[2];
            tm->inverseMultiplyPoint(xyz);
            
            //
            // Convert to screen X/Y
            //
            float pt[3] = { xyz[0], xyz[1], xyz[2] };
            switch (axis) {
               case VolumeFile::VOLUME_AXIS_X:
                  return;
                  break;
               case VolumeFile::VOLUME_AXIS_Y:
                  return;
                  break;
               case VolumeFile::VOLUME_AXIS_Z:
                  return;
                  break;
               case VolumeFile::VOLUME_AXIS_ALL:
                  return;
                  break;
               case VolumeFile::VOLUME_AXIS_OBLIQUE:
                  xyz[0] = pt[0];
                  xyz[1] = pt[1];
                  xyz[2] = 0.0;
                  break;
               case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
                  xyz[0] = pt[1];
                  xyz[1] = pt[2];
                  xyz[2] = 0.0;
                  break;
               case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
                  xyz[0] = pt[0];
                  xyz[1] = pt[2];
                  xyz[2] = 0.0;
                  break;
               case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
                  xyz[0] = pt[0];
                  xyz[1] = pt[1];
                  xyz[2] = 0.0;
                  break;
               case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
                  break;
               case VolumeFile::VOLUME_AXIS_UNKNOWN:
                  break;
            }
            glColor3ub(r, g, b);
            if (selectFlag) {
               glPushName(i);
               size *= 2;
            }

            if (alpha < 255) {
               glEnable(GL_BLEND);
               glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            switch (symbol) {
               case ColorFile::ColorStorage::SYMBOL_OPENGL_POINT:
                  //
                  // Points must be at least 1.0 for OpenGL to draw something
                  //
                  size = std::max(size, 1.0f);
                  glPointSize(getValidPointSize(size));
                  glBegin(GL_POINTS);
                     glVertex3f(xyz[0], xyz[1], xyz[2]);
                  glEnd();
                  break;
               case ColorFile::ColorStorage::SYMBOL_SPHERE:
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  glPushMatrix();
                      glTranslatef(xyz[0], xyz[1], xyz[2]);
                      drawSphere(size);
                  glPopMatrix();
                  break;
               case ColorFile::ColorStorage::SYMBOL_BOX:
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  glPushMatrix();
                      glTranslatef(xyz[0], xyz[1], xyz[2]);
                      glScalef(size, size, size);
                      drawBox();
                  glPopMatrix();
                  break;
               case ColorFile::ColorStorage::SYMBOL_DIAMOND:
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  glPushMatrix();
                      glTranslatef(xyz[0], xyz[1], xyz[2]);
                      glScalef(size, size, size);
                      drawDiamond();
                  glPopMatrix();
                  break;
               case ColorFile::ColorStorage::SYMBOL_DISK:
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  glPushMatrix();
                      glTranslatef(xyz[0], xyz[1], xyz[2]);
                      drawDisk(size);
                  glPopMatrix();
                  break;
               case ColorFile::ColorStorage::SYMBOL_RING:
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  glPushMatrix();
                      glTranslatef(xyz[0], xyz[1], xyz[2]);
                      glScalef(size, size, size);
                      drawRing();
                  glPopMatrix();
                  break;
               case ColorFile::ColorStorage::SYMBOL_NONE:
                  break;
               case ColorFile::ColorStorage::SYMBOL_SQUARE:
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  glPushMatrix();
                      glTranslatef(xyz[0], xyz[1], xyz[2]);
                      glScalef(size, size, size);
                      drawSquare();
                  glPopMatrix();
                  break;
            }
            if (selectFlag) {
               glPopName();
            }
            glDisable(GL_BLEND);
            glDisable(GL_LIGHTING);
            glDisable(GL_COLOR_MATERIAL);   
         }
      }
   }
   if (selectFlag) {
      glPopName();
      if (transformDataFileIndex >= 0) {
         glPopName();
      }
   }
}
                                           
/**
 * Draw the oblique volume foci file.
 * If "transformDataFileIndex" is greater than or equal to zero, the a 
 * transform data (cell or foci) file is being drawn.
 */
void 
BrainModelOpenGL::drawObliqueVolumeFociFile(const VolumeFile::VOLUME_AXIS axis,
                                            const float voxelSize,
                                            const TransformationMatrix* tm,
                                            const float sliceCornerCoords[4][3])
{
   const FociProjectionFile* fpf = brainSet->getFociProjectionFile();
   const DisplaySettingsCells* dsf = brainSet->getDisplaySettingsFoci();
   const FociColorFile* colorFile = brainSet->getFociColorFile();
   
   //
   // Set color for cells/foci with missing colors
   //
   unsigned char noColorRed, noColorGreen, noColorBlue;
   PreferencesFile* pref = brainSet->getPreferencesFile();
   pref->getSurfaceForegroundColor(noColorRed, noColorGreen, noColorBlue);

   const float halfVoxelSize = voxelSize * 0.6;
   
   //
   // Check for selection mode
   //
   bool selectFlag = false;
   if (selectionMask & SELECTION_MASK_VOLUME_FOCI) {
      glPushName(SELECTION_MASK_VOLUME_FOCI);
      selectFlag = true;
   }
   if ((selectFlag == false) && (selectionMask != SELECTION_MASK_OFF)) {
      return;
   }
   
   //
   // Normal of plane
   //
   float planeNormal[3];
   MathUtilities::computeNormal((float*)sliceCornerCoords[0],
                              (float*)sliceCornerCoords[1],
                              (float*)sliceCornerCoords[2],
                              planeNormal);
                              
   //
   // Draw cells larger when selecting on a flat surface
   //
   float cellSize = dsf->getDrawSize();

   const int numCells = fpf->getNumberOfCellProjections();

   if (numCells > 0) {
      const int numColors = colorFile->getNumberOfColors();
      for (int i = 0; i < numCells; i++) {
         const CellProjection* cd = fpf->getCellProjection(i);
         
         ColorFile::ColorStorage::SYMBOL symbol = ColorFile::ColorStorage::SYMBOL_OPENGL_POINT;
         const int colorIndex = cd->getColorIndex();
         float pointSize = 1;
         float lineSize  = 1;
         unsigned char r = 0, g = 0, b = 0, alpha = 255;
         if ((colorIndex >= 0) && (colorIndex < numColors)) {
            const ColorFile::ColorStorage* cs = colorFile->getColor(colorIndex);
            cs->getRgba(r, g, b, alpha);
            lineSize = cs->getLineSize();
            pointSize = cs->getPointSize();
            symbol = cs->getSymbol();
         }
         else {
            r = noColorRed;
            g = noColorGreen;
            b = noColorBlue;
         }
         
         if (dsf->getSymbolOverride() != ColorFile::ColorStorage::SYMBOL_NONE) {
            symbol = dsf->getSymbolOverride();
         }
         if (pointSize < 1) {
            pointSize = 1;
         }
         
         float size = pointSize * cellSize;
         //
         // Double size for highlighting
         //
         if (cd->getHighlightFlag()) {
            size *= 2.0;
         }
         
         float xyz[3];
         cd->getVolumeXYZ(xyz);
         if ((xyz[0] == 0.0) && (xyz[1] == 0.0) && (xyz[2] == 0.0)) {
            continue;
         }
         
         //
         // Get distance and point on plane
         //
         float intersection[3];
         float signedDistanceFromPlaneOut;
         bool valid = MathUtilities::rayIntersectPlane(sliceCornerCoords[0],
                                          sliceCornerCoords[1],
                                          sliceCornerCoords[2],
                                          xyz,
                                          planeNormal,
                                          intersection,
                                          &signedDistanceFromPlaneOut);
         if (signedDistanceFromPlaneOut < 0.0) {
            signedDistanceFromPlaneOut = -signedDistanceFromPlaneOut;
         }
         if (valid) {
            if (signedDistanceFromPlaneOut > halfVoxelSize) {
               valid = false;
            }
         }
         if (valid) {
            //
            // Undo screen to slice transformation
            //
            xyz[0] = intersection[0];
            xyz[1] = intersection[1];
            xyz[2] = intersection[2];
            tm->inverseMultiplyPoint(xyz);
            
            //
            // Convert to screen X/Y
            //
            float pt[3] = { xyz[0], xyz[1], xyz[2] };
            switch (axis) {
               case VolumeFile::VOLUME_AXIS_X:
                  return;
                  break;
               case VolumeFile::VOLUME_AXIS_Y:
                  return;
                  break;
               case VolumeFile::VOLUME_AXIS_Z:
                  return;
                  break;
               case VolumeFile::VOLUME_AXIS_ALL:
                  return;
                  break;
               case VolumeFile::VOLUME_AXIS_OBLIQUE:
                  xyz[0] = pt[0];
                  xyz[1] = pt[1];
                  xyz[2] = 0.0;
                  break;
               case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
                  xyz[0] = pt[1];
                  xyz[1] = pt[2];
                  xyz[2] = 0.0;
                  break;
               case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
                  xyz[0] = pt[0];
                  xyz[1] = pt[2];
                  xyz[2] = 0.0;
                  break;
               case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
                  xyz[0] = pt[0];
                  xyz[1] = pt[1];
                  xyz[2] = 0.0;
                  break;
               case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
                  break;
               case VolumeFile::VOLUME_AXIS_UNKNOWN:
                  break;
            }
            glColor3ub(r, g, b);
            if (selectFlag) {
               glPushName(i);
               size *= 2;
            }

            if (alpha < 255) {
               glEnable(GL_BLEND);
               glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            switch (symbol) {
               case ColorFile::ColorStorage::SYMBOL_OPENGL_POINT:
                  //
                  // Points must be at least 1.0 for OpenGL to draw something
                  //
                  size = std::max(size, 1.0f);
                  glPointSize(getValidPointSize(size));
                  glBegin(GL_POINTS);
                     glVertex3f(xyz[0], xyz[1], xyz[2]);
                  glEnd();
                  break;
               case ColorFile::ColorStorage::SYMBOL_SPHERE:
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  glPushMatrix();
                      glTranslatef(xyz[0], xyz[1], xyz[2]);
                      drawSphere(size);
                  glPopMatrix();
                  break;
               case ColorFile::ColorStorage::SYMBOL_BOX:
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  glPushMatrix();
                      glTranslatef(xyz[0], xyz[1], xyz[2]);
                      glScalef(size, size, size);
                      drawBox();
                  glPopMatrix();
                  break;
               case ColorFile::ColorStorage::SYMBOL_DIAMOND:
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  glPushMatrix();
                      glTranslatef(xyz[0], xyz[1], xyz[2]);
                      glScalef(size, size, size);
                      drawDiamond();
                  glPopMatrix();
                  break;
               case ColorFile::ColorStorage::SYMBOL_DISK:
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  glPushMatrix();
                      glTranslatef(xyz[0], xyz[1], xyz[2]);
                      drawDisk(size);
                  glPopMatrix();
                  break;
               case ColorFile::ColorStorage::SYMBOL_RING:
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  glPushMatrix();
                      glTranslatef(xyz[0], xyz[1], xyz[2]);
                      glScalef(size, size, size);
                      drawRing();
                  glPopMatrix();
                  break;
               case ColorFile::ColorStorage::SYMBOL_NONE:
                  break;
               case ColorFile::ColorStorage::SYMBOL_SQUARE:
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  glPushMatrix();
                      glTranslatef(xyz[0], xyz[1], xyz[2]);
                      glScalef(size, size, size);
                      drawSquare();
                  glPopMatrix();
                  break;
            }
            if (selectFlag) {
               glPopName();
            }
            glDisable(GL_BLEND);
            glDisable(GL_LIGHTING);
            glDisable(GL_COLOR_MATERIAL);   
         }
      }
   }
   if (selectFlag) {
      glPopName();
   }
}

/**
 * Draw the oblique volume contour file.
 */
void 
BrainModelOpenGL::drawObliqueContourFile(const VolumeFile::VOLUME_AXIS axis,
                                                  const ContourFile* cf,
                                                  const float voxelSize,
                                                  const TransformationMatrix* tm,
                                                  const float sliceCornerCoords[4][3])
{
   const DisplaySettingsContours* dsc = brainSet->getDisplaySettingsContours();
   const float halfVoxelSize = voxelSize * 0.6;

   //
   // Normal of plane
   //
   float planeNormal[3];
   MathUtilities::computeNormal((float*)sliceCornerCoords[0],
                              (float*)sliceCornerCoords[1],
                              (float*)sliceCornerCoords[2],
                              planeNormal);
                              
   const int numContours = cf->getNumberOfContours();

   if (numContours > 0) {
      for (int i = 0; i < numContours; i++) {
         const CaretContour* contour = cf->getContour(i);
         const int numPoints = contour->getNumberOfPoints();
         
         for (int j = 0; j < numPoints; j++) {
            if ((j == 0) && (dsc->getShowEndPoints())) {
               glColor3ub(255, 0, 0);
            }
            else {
               glColor3ub(0, 255, 0);
            }
            float x, y, z;
            contour->getPointXYZ(j, x, y, z);
            
            float xyz[3] = { x, y, z };
            
            //
            // Get distance and point on plane
            //
            float intersection[3];
            float signedDistanceFromPlaneOut;
            bool valid = MathUtilities::rayIntersectPlane(sliceCornerCoords[0],
                                             sliceCornerCoords[1],
                                             sliceCornerCoords[2],
                                             xyz,
                                             planeNormal,
                                             intersection,
                                             &signedDistanceFromPlaneOut);
            if (signedDistanceFromPlaneOut < 0.0) {
               signedDistanceFromPlaneOut = -signedDistanceFromPlaneOut;
            }
            if (valid) {
               if (signedDistanceFromPlaneOut > halfVoxelSize) {
                  valid = false;
               }
            }
            if (valid) {
               //
               // Undo screen to slice transformation
               //
               xyz[0] = intersection[0];
               xyz[1] = intersection[1];
               xyz[2] = intersection[2];
               tm->inverseMultiplyPoint(xyz);
               
               //
               // Convert to screen X/Y
               //
               float pt[3] = { xyz[0], xyz[1], xyz[2] };
               switch (axis) {
                  case VolumeFile::VOLUME_AXIS_X:
                     return;
                     break;
                  case VolumeFile::VOLUME_AXIS_Y:
                     return;
                     break;
                  case VolumeFile::VOLUME_AXIS_Z:
                     return;
                     break;
                  case VolumeFile::VOLUME_AXIS_ALL:
                     return;
                     break;
                  case VolumeFile::VOLUME_AXIS_OBLIQUE:
                     xyz[0] = pt[0];
                     xyz[1] = pt[1];
                     xyz[2] = 0.0;
                     break;
                  case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
                     xyz[0] = pt[1];
                     xyz[1] = pt[2];
                     xyz[2] = 0.0;
                     break;
                  case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
                     xyz[0] = pt[0];
                     xyz[1] = pt[2];
                     xyz[2] = 0.0;
                     break;
                  case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
                     xyz[0] = pt[0];
                     xyz[1] = pt[1];
                     xyz[2] = 0.0;
                     break;
                  case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
                     break;
                  case VolumeFile::VOLUME_AXIS_UNKNOWN:
                     break;
               }

               //if (alpha < 255) {
               //   glEnable(GL_BLEND);
               //   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
               //}
               
               glBegin(GL_POINTS);
                  glVertex3fv(xyz);
               glEnd();
               
               //glDisable(GL_BLEND);
            }
         }
      }
   }
}
   
/**
 * Draw the volume contour file.
 */
void 
BrainModelOpenGL::drawVolumeContourFile(const VolumeFile::VOLUME_AXIS axis,
                                        const float axisCoord,
                                        const float voxelSize)
{
   const DisplaySettingsContours* dsc = brainSet->getDisplaySettingsContours();
   const float halfVoxelSize = voxelSize * 0.6;
   
      BrainModelContours* bmc = brainSet->getBrainModelContours();
      if (bmc != NULL) {
         const ContourFile* cf = bmc->getContourFile();
         const int numContours = cf->getNumberOfContours();
         glPointSize(getValidPointSize(dsc->getPointSize()));
         glLineWidth(getValidLineWidth(dsc->getLineThickness()));
         glColor3f(0.0, 1.0, 0.0);
         
         for (int i = 0; i < numContours; i++) {
            const CaretContour* contour = cf->getContour(i);
            const float z = contour->getSectionNumber();
            
            const int numPoints = contour->getNumberOfPoints();
            
            glBegin(GL_POINTS);
               for (int j = 0; j < numPoints; j++) {
                  if ((j == 0) && (dsc->getShowEndPoints())) {
                     glColor3ub(255, 0, 0);
                  }
                  else {
                     glColor3ub(0, 255, 0);
                  }
                  float x, y;
                  contour->getPointXY(j, x, y);
                  
                  float xyz[3] = { x, y, z };
                  if (fabs(xyz[axis] - axisCoord) < halfVoxelSize) {
                     convertVolumeItemXYZToScreenXY(axis, xyz);
                     glVertex3fv(xyz);
                  }
               }
            glEnd();
         }
      
         if (dsc->getDisplayContourCells()) {
            ContourCellFile* ccf = brainSet->getContourCellFile();
            const int numCells = ccf->getNumberOfCells();

            if (numCells > 0) {
               const float cellSize = dsc->getContourCellSize();
               const ContourCellColorFile* colorFile = brainSet->getContourCellColorFile();
               //
               // Set color for cells with missing colors
               //
               unsigned char noColorRed, noColorGreen, noColorBlue;
               PreferencesFile* pref = brainSet->getPreferencesFile();
               pref->getSurfaceForegroundColor(noColorRed, noColorGreen, noColorBlue);
               
               const int numColors = colorFile->getNumberOfColors();
               for (int i = 0; i < numCells; i++) {
                  const CellData* cd = ccf->getCell(i);
                  
                  const int colorIndex = cd->getColorIndex();
                  float pointSize = 1;
                  unsigned char r = 0, g = 0, b = 0;
                  if ((colorIndex >= 0) && (colorIndex < numColors)) {
                     const ColorFile::ColorStorage* cs = colorFile->getColor(colorIndex);
                     cs->getRgb(r, g, b);
                     pointSize = cs->getPointSize();
                  }
                  else {
                     r = noColorRed;
                     g = noColorGreen;
                     b = noColorBlue;
                  }
                  
                  if (pointSize < 1) {
                     pointSize = 1;
                  }
                  
                  float size = pointSize * cellSize;
                  
                  float xyz[3];
                  cd->getXYZ(xyz);
                  xyz[2] = cd->getSectionNumber() * cf->getSectionSpacing();
                  
                  if (fabs(xyz[axis] - axisCoord) < halfVoxelSize) {
                     convertVolumeItemXYZToScreenXY(axis, xyz);
                     glColor3ub(r, g, b);
                     //
                     // Points must be at least 1.0 for OpenGL to draw something
                     //
                     size = std::max(size, 1.0f);
                     glPointSize(getValidPointSize(size));
                     glBegin(GL_POINTS);
                        glVertex3f(xyz[0], xyz[1], xyz[2]);
                     glEnd();
                  }
               }
            }
         }
      }
}

/**
 * Draw the volume cell file.
 */
void 
BrainModelOpenGL::drawVolumeCellFile(const VolumeFile::VOLUME_AXIS axis,
                                     const float axisCoord,
                                     const float voxelSize)
{
   const CellFile* cf = brainSet->getVolumeCellFile();
   const DisplaySettingsCells* dsc = brainSet->getDisplaySettingsCells();
   const CellColorFile* colorFile = brainSet->getCellColorFile();
   
   //
   // Set color for cells/foci with missing colors
   //
   unsigned char noColorRed, noColorGreen, noColorBlue;
   PreferencesFile* pref = brainSet->getPreferencesFile();
   pref->getSurfaceForegroundColor(noColorRed, noColorGreen, noColorBlue);

   const float halfVoxelSize = voxelSize * 0.6;
   
   //
   // Check for selection mode
   //
   bool selectFlag = false;
   if (selectionMask & SELECTION_MASK_VOLUME_CELL) {
      glPushName(SELECTION_MASK_VOLUME_CELL);
      selectFlag = true;
   }
   if ((selectFlag == false) && (selectionMask != SELECTION_MASK_OFF)) {
      return;
   }
   
   int axisIndex = 0;
   switch (axis) {
      case VolumeFile::VOLUME_AXIS_X:
         axisIndex = 0;
         break;
      case VolumeFile::VOLUME_AXIS_Y:
         axisIndex = 1;
         break;
      case VolumeFile::VOLUME_AXIS_Z:
         axisIndex = 2;
         break;
      case VolumeFile::VOLUME_AXIS_ALL:
      case VolumeFile::VOLUME_AXIS_OBLIQUE:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
      case VolumeFile::VOLUME_AXIS_UNKNOWN:
         return;
         break;
   } 

   //
   // Draw cells larger when selecting on a flat surface
   //
   float cellSize = dsc->getDrawSize();

   const int numCells = cf->getNumberOfCells();

   if (numCells > 0) {
      const int numColors = colorFile->getNumberOfColors();
      for (int i = 0; i < numCells; i++) {
         const CellData* cd = cf->getCell(i);
         
         if (cd->getDisplayFlag() == false) {
            continue;
         }
                  
         ColorFile::ColorStorage::SYMBOL symbol = ColorFile::ColorStorage::SYMBOL_OPENGL_POINT;
         const int colorIndex = cd->getColorIndex();
         float pointSize = 1;
         float lineSize  = 1;
         unsigned char r = 0, g = 0, b = 0, alpha = 255;
         if ((colorIndex >= 0) && (colorIndex < numColors)) {
            const ColorFile::ColorStorage* cs = colorFile->getColor(colorIndex);
            cs->getRgba(r, g, b, alpha);
            lineSize = cs->getLineSize();
            pointSize = cs->getPointSize();
            symbol = cs->getSymbol();
         }
         else {
            r = noColorRed;
            g = noColorGreen;
            b = noColorBlue;
         }
         
         if (dsc->getSymbolOverride() != ColorFile::ColorStorage::SYMBOL_NONE) {
            symbol = dsc->getSymbolOverride();
         }
         if (pointSize < 1) {
            pointSize = 1;
         }
         
         float size = pointSize * cellSize;
         
         float xyz[3];
         cd->getXYZ(xyz);
         if (fabs(xyz[axisIndex] - axisCoord) < halfVoxelSize) {
            convertVolumeItemXYZToScreenXY(axis, xyz);
            glColor3ub(r, g, b);
            if (selectFlag) {
               glPushName(i);
            }

            if (alpha < 255) {
               glEnable(GL_BLEND);
               glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            drawSymbol(symbol, xyz[0], xyz[1], xyz[2], size, NULL);
            if (selectFlag) {
               glPopName();
            }
            glDisable(GL_BLEND);
            glDisable(GL_LIGHTING);
            glDisable(GL_COLOR_MATERIAL);   
         }
      }
   }
   if (selectFlag) {
      glPopName();
   }
}      

/**
 * Draw the volume foci file.
 */
void 
BrainModelOpenGL::drawVolumeFociFile(const VolumeFile::VOLUME_AXIS axis,
                                     const float axisCoord,
                                     const float voxelSize)
{
   const FociProjectionFile* fpf = brainSet->getFociProjectionFile();
   const DisplaySettingsFoci* dsf = brainSet->getDisplaySettingsFoci();
   const FociColorFile* colorFile = brainSet->getFociColorFile();
   
   //
   // Set color for cells/foci with missing colors
   //
   unsigned char noColorRed, noColorGreen, noColorBlue;
   PreferencesFile* pref = brainSet->getPreferencesFile();
   pref->getSurfaceForegroundColor(noColorRed, noColorGreen, noColorBlue);

   const float halfVoxelSize = voxelSize * 0.6;

   //
   // Check for selection mode
   //
   bool selectFlag = false;
   if (selectionMask & SELECTION_MASK_VOLUME_FOCI) {
      glPushName(SELECTION_MASK_VOLUME_FOCI);
      selectFlag = true;
   }
   if ((selectFlag == false) && (selectionMask != SELECTION_MASK_OFF)) {
      return;
   }
   
   int axisIndex = 0;
   switch (axis) {
      case VolumeFile::VOLUME_AXIS_X:
         axisIndex = 0;
         break;
      case VolumeFile::VOLUME_AXIS_Y:
         axisIndex = 1;
         break;
      case VolumeFile::VOLUME_AXIS_Z:
         axisIndex = 2;
         break;
      case VolumeFile::VOLUME_AXIS_ALL:
      case VolumeFile::VOLUME_AXIS_OBLIQUE:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
      case VolumeFile::VOLUME_AXIS_UNKNOWN:
         return;
         break;
   } 

   //
   // Draw foci larger when selecting on a flat surface
   //
   float fociSize = dsf->getDrawSize();

   const int numFoci = fpf->getNumberOfCellProjections();

   if (numFoci > 0) {
      const int numColors = colorFile->getNumberOfColors();
      for (int i = 0; i < numFoci; i++) {
         const CellProjection* focus = fpf->getCellProjection(i);
         
         if (focus->getDisplayFlag() == false) {
            continue;
         }
                  
         ColorFile::ColorStorage::SYMBOL symbol = ColorFile::ColorStorage::SYMBOL_OPENGL_POINT;
         const int colorIndex = focus->getColorIndex();
         float pointSize = 1;
         float lineSize  = 1;
         unsigned char r = 0, g = 0, b = 0, alpha = 255;
         if ((colorIndex >= 0) && (colorIndex < numColors)) {
            const ColorFile::ColorStorage* cs = colorFile->getColor(colorIndex);
            cs->getRgba(r, g, b, alpha);
            lineSize = cs->getLineSize();
            pointSize = cs->getPointSize();
            symbol = cs->getSymbol();
         }
         else {
            r = noColorRed;
            g = noColorGreen;
            b = noColorBlue;
         }
         
         if (dsf->getSymbolOverride() != ColorFile::ColorStorage::SYMBOL_NONE) {
            symbol = dsf->getSymbolOverride();
         }
         if (pointSize < 1) {
            pointSize = 1;
         }
         
         float size = pointSize * fociSize;
         
         //
         // Double size for highlighting
         //
         if (focus->getHighlightFlag()) {
            size *= 2.0;
         }

         float xyz[3];
         focus->getVolumeXYZ(xyz);
         if ((xyz[0] != 0.0) || (xyz[1] != 0.0) || (xyz[2] != 0.0)) {
            if (fabs(xyz[axisIndex] - axisCoord) < halfVoxelSize) {
               convertVolumeItemXYZToScreenXY(axis, xyz);
               glColor3ub(r, g, b);
               if (selectFlag) {
                  glPushName(i);
               }

               if (alpha < 255) {
                  glEnable(GL_BLEND);
                  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
               }
               drawSymbol(symbol, xyz[0], xyz[1], xyz[2], size, NULL);
               if (selectFlag) {
                  glPopName();
               }
               glDisable(GL_BLEND);
               glDisable(GL_LIGHTING);
               glDisable(GL_COLOR_MATERIAL);   
            }
         }
      }
   }
   if (selectFlag) {
      glPopName();
   }
}      

/**
 * Draw the volume border file
 */
void
BrainModelOpenGL::drawVolumeBorderFile(const VolumeFile::VOLUME_AXIS axis,
                                          const float axisCoord,
                                          const float voxelSize)
{
   BorderFile* bf = brainSet->getVolumeBorderFile();
   
   const float halfVoxelSize = voxelSize * 0.6;
   
   //
   // Set color for borders with missing colors
   //
   unsigned char noColorRed, noColorGreen, noColorBlue;
   PreferencesFile* pref = brainSet->getPreferencesFile();
   pref->getSurfaceForegroundColor(noColorRed, noColorGreen, noColorBlue);

   BorderColorFile* colorFile = brainSet->getBorderColorFile();
   const int numColors = colorFile->getNumberOfColors();
   
   //
   // Draw borders larger when selecting on a flat surface
   //
   DisplaySettingsBorders* dsb = brainSet->getDisplaySettingsBorders();
   float drawSize = dsb->getDrawSize();

   //
   // Check for selection mode
   //
   bool selectFlag = false;
   if (selectionMask & SELECTION_MASK_VOLUME_BORDER) {
      glPushName(SELECTION_MASK_VOLUME_BORDER);
      selectFlag = true;
   }
   if ((selectFlag == false) && (selectionMask != SELECTION_MASK_OFF)) {
      return;
   }
   
   int axisIndex = 0;
   switch (axis) {
      case VolumeFile::VOLUME_AXIS_X:
         axisIndex = 0;
         break;
      case VolumeFile::VOLUME_AXIS_Y:
         axisIndex = 1;
         break;
      case VolumeFile::VOLUME_AXIS_Z:
         axisIndex = 2;
         break;
      case VolumeFile::VOLUME_AXIS_ALL:
      case VolumeFile::VOLUME_AXIS_OBLIQUE:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
      case VolumeFile::VOLUME_AXIS_UNKNOWN:
         return;
         break;
   } 

   const int numBorders = bf->getNumberOfBorders();
   for (int i = 0; i < numBorders; i++) {
      const Border* b = bf->getBorder(i);
      if (b->getDisplayFlag()) {
         const int numLinks = b->getNumberOfLinks();
         
         const int colorIndex = b->getBorderColorIndex();
         float pointSize = 1;
         float lineSize  = 1;
         unsigned char red = 0, green = 0, blue = 0;
         if ((colorIndex >= 0) && (colorIndex < numColors)) {
            colorFile->getColorByIndex(colorIndex, red, green, blue);
            colorFile->getPointLineSizeByIndex(colorIndex, pointSize, lineSize);
         }
         else {
            red   = noColorRed;
            green = noColorGreen;
            blue  = noColorBlue;
         }
         
         if (selectFlag) {
            glPushName(i);
               glPointSize(getValidPointSize(pointSize * drawSize));
               for (int j = 0; j < numLinks; j++) {
                  glPushName(j);
                     glBegin(GL_POINTS);
                        float xyz[3];
                        b->getLinkXYZ(j, xyz);
                        if (fabs(xyz[axisIndex] - axisCoord) < halfVoxelSize) {
                           convertVolumeItemXYZToScreenXY(axis, xyz);
                           glVertex3fv(xyz);
                        }
                     glEnd();
                  glPopName();
               }
            glPopName();
         }
         else {
            if ((dsb->getDrawMode() == DisplaySettingsBorders::BORDER_DRAW_AS_SYMBOLS) ||
                (dsb->getDrawMode() == DisplaySettingsBorders::BORDER_DRAW_AS_SYMBOLS_AND_LINES)) {
               glPointSize(getValidPointSize(pointSize * drawSize));
               int startLink = 0;
               glBegin(GL_POINTS);
                  if (dsb->getDisplayFirstLinkRed()) {
                     glColor3ub(255, 0, 0);
                     float xyz[3];
                     b->getLinkXYZ(0, xyz);
                     if (fabs(xyz[axisIndex] - axisCoord) < halfVoxelSize) {
                        convertVolumeItemXYZToScreenXY(axis, xyz);
                        glVertex3fv(xyz);
                     }
                     startLink = 1;
                  }
                  glColor3ub(red, green, blue);
                  float xyz[3];
                  for (int j = startLink; j < numLinks; j++) {
                     b->getLinkXYZ(j, xyz);
                     if (fabs(xyz[axisIndex] - axisCoord) < halfVoxelSize) {
                        convertVolumeItemXYZToScreenXY(axis, xyz);
                        glVertex3fv(xyz);
                     }
                  }
               glEnd();
            }
            if ((dsb->getDrawMode() == DisplaySettingsBorders::BORDER_DRAW_AS_LINES) ||
                (dsb->getDrawMode() == DisplaySettingsBorders::BORDER_DRAW_AS_UNSTRETCHED_LINES) || 
                (dsb->getDrawMode() == DisplaySettingsBorders::BORDER_DRAW_AS_SYMBOLS_AND_LINES)) {
               glLineWidth(getValidLineWidth(lineSize * drawSize));
               glBegin(GL_LINES);
                  int startLink = 0;
                  if (dsb->getDisplayFirstLinkRed()) {
                     glColor3ub(255, 0, 0);
                     float xyz[3];
                     b->getLinkXYZ(0, xyz);
                     if (fabs(xyz[axisIndex] - axisCoord) < halfVoxelSize) {
                        convertVolumeItemXYZToScreenXY(axis, xyz);
                        glVertex3fv(xyz);
                        glVertex3fv(xyz);
                     }
                     startLink = 1;
                  }
                  glColor3ub(red, green, blue);
                  float xyz[3];
                  for (int j = startLink + 1; j < numLinks; j++) {
                     b->getLinkXYZ(j - 1, xyz);
                     if (fabs(xyz[axisIndex] - axisCoord) < halfVoxelSize) {
                        convertVolumeItemXYZToScreenXY(axis, xyz);
                        glVertex3fv(xyz);
                        b->getLinkXYZ(j, xyz);
                        convertVolumeItemXYZToScreenXY(axis, xyz);
                        glVertex3fv(xyz);
                     }
                  }
               glEnd();
            }
         }
      }
   }
   
   if (selectFlag) {
      glPopName();
   }
}

/**
 * Draw the volume identify symbols
 */
void
BrainModelOpenGL::drawVolumeIdentifySymbols(const VolumeFile::VOLUME_AXIS axis,
                                            const float axisCoord)
{
   BrainModelVolume* bmv = brainSet->getBrainModelVolume();
   if (bmv == NULL) {
      return;
   }
   
   //
   // Check for selection mode
   //
   if (selectionMask != SELECTION_MASK_OFF) {
      return;
   }
   
   int axisIndex = 0;
   switch (axis) {
      case VolumeFile::VOLUME_AXIS_X:
         axisIndex = 0;
         break;
      case VolumeFile::VOLUME_AXIS_Y:
         axisIndex = 1;
         break;
      case VolumeFile::VOLUME_AXIS_Z:
         axisIndex = 2;
         break;
      case VolumeFile::VOLUME_AXIS_ALL:
      case VolumeFile::VOLUME_AXIS_OBLIQUE:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
      case VolumeFile::VOLUME_AXIS_UNKNOWN:
         return;
         break;
   } 

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   
   glColor4f(0.0, 1.0, 0.0, 0.5);

   const BrainModelSurface* bms = brainSet->getActiveFiducialSurface();
   if (bms != NULL) {
      const CoordinateFile* cf = bms->getCoordinateFile();
      const int numCoords = cf->getNumberOfCoordinates();
      
      //
      // See if node uncertainty should be displayed
      //
      const SurfaceShapeFile* ssf = brainSet->getSurfaceShapeFile();
      const DisplaySettingsSurfaceShape* dsss = brainSet->getDisplaySettingsSurfaceShape();
      int shapeColumn = -1;
      if (dsss->getNodeUncertaintyEnabled()) {
         shapeColumn = dsss->getNodeUncertaintyColumn();
         if (shapeColumn >= ssf->getNumberOfColumns()) {
            shapeColumn = -1;
         }
      }

      BrainSetNodeAttribute* attributes = brainSet->getNodeAttributes(0);
   
      //
      // Show node highlighting
      //
      for (int i = 0; i < numCoords; i++) {
         if (attributes[i].getDisplayFlag()) {
            BrainSetNodeAttribute* bna = brainSet->getNodeAttributes(i);
            float xyz[3];
            cf->getCoordinate(i, xyz);
            bool drawIt = false;
            if (bna->getHighlighting() == BrainSetNodeAttribute::HIGHLIGHT_NODE_LOCAL) {
               glColor4f(0, 1.0, 0, 0.5);
               drawIt = true;
            }
            else if (bna->getHighlighting() == BrainSetNodeAttribute::HIGHLIGHT_NODE_REMOTE) {
               glColor4f(0, 0, 1.0, 0.5);
               drawIt = true;
            }
            if (drawIt) {
               if (shapeColumn >= 0) {
                  const float radius = fabs(ssf->getValue(i, shapeColumn));
                  const float dist = fabs(xyz[axisIndex] - axisCoord);
                  if (dist < radius) {
                     float size = radius;
                     if (dist > 0.0) {
                        //
                        // Disk should be smaller the further it is from the ID node
                        //
                        const float angle = std::acos(dist / radius);
                        size = radius * std::sin(angle);
                     }
                     convertVolumeItemXYZToScreenXY(axis, xyz);
                     glPushMatrix();
                        glTranslatef(xyz[0], xyz[1], xyz[2]);
                        drawDisk(size * 2.0);
                     glPopMatrix();
                  }
               }
               else {
               //   glBegin(GL_POINTS);
               //      glVertex3fv(xyz);
               //   glEnd();
               }
            }
         }
      }
   }



   glDisable(GL_BLEND);
}

/**
 * Draw a volume file sliece
 */
void
BrainModelOpenGL::drawVolumeFileSlice(VolumeFile* vf, const VolumeFile::VOLUME_AXIS axis,
                                 const int currentSlice,
                                 const unsigned long maskForThisUnderlayOrOverlay,
                                 const bool overlayFlag)
{   
   if (vf == NULL) {
      return;
   }
   
   int voldim[3];
   vf->getDimensions(voldim);
   if ((voldim[0] <= 0) || (voldim[1] <= 0) || (voldim[2] <= 0)) {
      return;
   }

   //
   // Do not draw invalid slices
   //
   switch(axis) {
      case VolumeFile::VOLUME_AXIS_X:  // PARASAGITTAL
         if ((currentSlice < 0) || (currentSlice >= voldim[0])) {
            return;
         }
         break;
      case VolumeFile::VOLUME_AXIS_Y:  // CORONAL
         if ((currentSlice < 0) || (currentSlice >= voldim[1])) {
            return;
         }
         break;
      case VolumeFile::VOLUME_AXIS_Z:  // HORIZONTAL
         if ((currentSlice < 0) || (currentSlice >= voldim[2])) {
            return;
         }
         break;
      case VolumeFile::VOLUME_AXIS_ALL:
      case VolumeFile::VOLUME_AXIS_OBLIQUE:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
      case VolumeFile::VOLUME_AXIS_UNKNOWN:
         std::cout << "PROGAM ERROR: Invalid volume axis at " << __LINE__ << " in " 
                   << __FILE__ << std::endl;
         return;
   }
   
   //
   // Get volume display settings
   //
   DisplaySettingsVolume* dsv = brainSet->getDisplaySettingsVolume();
   BrainModelVolume* bmv = brainSet->getBrainModelVolume();
   BrainModelVolumeVoxelColoring* voxelColoring = brainSet->getVoxelColoring();
   
   //
   // Setup these dimensions for displaying the surface
   //
   int dim[3] = { 0, 0, 0 };
   switch(axis) {
      case VolumeFile::VOLUME_AXIS_X:  // PARASAGITTAL
         dim[0] = voldim[1];
         dim[1] = voldim[2];
         dim[2] = voldim[0];
         break;
      case VolumeFile::VOLUME_AXIS_Y:  // CORONAL
         dim[0] = voldim[0];
         dim[1] = voldim[2];
         dim[2] = voldim[1];
         break;
      case VolumeFile::VOLUME_AXIS_Z:  // HORIZONTAL
         dim[0] = voldim[0];
         dim[1] = voldim[1];
         dim[2] = voldim[2];
         break;
      case VolumeFile::VOLUME_AXIS_ALL:
      case VolumeFile::VOLUME_AXIS_OBLIQUE:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
      case VolumeFile::VOLUME_AXIS_UNKNOWN:
         std::cout << "PROGAM ERROR: Invalid volume axis at " << __LINE__ << " in " 
                   << __FILE__ << std::endl;
         return;
   }
   
   //
   // Get the volume spacing
   //
   float spacing[3];
   vf->getSpacing(spacing);
   
   float voxelSizeX = 1.0;
   float voxelSizeY = 1.0;

   //
   // Get the origin
   //
   float origin[3];
   vf->getOrigin(origin);

   float voxelOriginX = 1.0;
   float voxelOriginY = 1.0;   

   //
   // See if this is a vector volume
   //
   const bool drawVectorsFlag = (vf->getVolumeType() == VolumeFile::VOLUME_TYPE_VECTOR);
   
   //
   // See if anatomy volume and if it is being thresholded
   //
   bool thresholdValid = false;
   float thresholdValue[2] = { 0.0, 1.0e10 };
   if (vf->getVolumeType() == VolumeFile::VOLUME_TYPE_ANATOMY) {
      if (dsv->getAnatomyThresholdValid()) {
         thresholdValid = true;
         dsv->getAnatomyThreshold(thresholdValue[0], thresholdValue[1]);
      }
   }
   
   //
   // Check for selection mode
   //
   bool selectFlag = false;
   bool drawAllVoxels = false;
   if (selectionMask & maskForThisUnderlayOrOverlay) {
      glPushName(maskForThisUnderlayOrOverlay);
      selectFlag = true;
      
      //
      // If segmentation volume editing
      //
      if (editingSegmentationVolumeFlag) {
         //
         // See if the volume being drawn is the segmentation volume
         //
         BrainModelVolume* bmv = brainSet->getBrainModelVolume();
         if (bmv != NULL) {
            if (vf == bmv->getSelectedVolumeSegmentationFile()) {
               drawAllVoxels = true;
            }
         }
      }
      //
      // If paint volume editing
      //
      if (editingPaintVolumeFlag) {
         //
         // See if the volume being drawn is the paint volume
         //
         BrainModelVolume* bmv = brainSet->getBrainModelVolume();
         if (bmv != NULL) {
            if (vf == bmv->getSelectedVolumePaintFile()) {
               drawAllVoxels = true;
            }
         }
      }
   }
   if ((selectFlag == false) && (selectionMask != SELECTION_MASK_OFF)) {
      return;
   }
   
   //
   // Disable blending
   //
   unsigned char alphaValue = 255;
   
   //
   // drawing type for segmentation volume
   //
   DisplaySettingsVolume::SEGMENTATION_DRAW_TYPE drawType = 
                          DisplaySettingsVolume::SEGMENTATION_DRAW_TYPE_SOLID;
                          
   //
   // If this is a segmentation volume
   //
   if ((vf->getVolumeType() == VolumeFile::VOLUME_TYPE_SEGMENTATION) &&
       (selectFlag == false)) {
      //
      // Get the segmentation drawing type
      //
      drawType = dsv->getSegmentationDrawType();
      
      //
      // If draw type is blending
      //
      if (drawType == DisplaySettingsVolume::SEGMENTATION_DRAW_TYPE_BLEND) {
         //
         // If underlay is anatomy
         //
         BrainModelVolumeVoxelColoring* voxelColoring = brainSet->getVoxelColoring();
         if (voxelColoring->getUnderlay() ==
             BrainModelVolumeVoxelColoring::UNDERLAY_OVERLAY_ANATOMY) {
            //
            // If the anatomy volume is valid
            //
            if (bmv->getSelectedVolumeAnatomyFile() != NULL) {
               //
               // Enable blending
               //
               alphaValue = static_cast<unsigned char>(dsv->getSegmentationTranslucency() * 255);
            }
         }
      }
   }
   else if (selectFlag == false) {
      if (overlayFlag) {
         alphaValue = static_cast<unsigned char>(dsv->getOverlayOpacity() * 255);
      }
   }
   
   //
   // ROI volumes always use alpha
   //
   if (vf->getVolumeType() == VolumeFile::VOLUME_TYPE_ROI) {
      //alphaValue = 165;
   }
   
   //
   // If alpha blending is on
   //
   if (alphaValue < 255) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   }

   int increment = 1;
   if (drawVectorsFlag) {
      increment = dsv->getVectorVolumeSparsity();
   }
   
   //
   // Draw the voxels of the selected slice
   //
   for (int i = 0; i < dim[0]; i += increment) {
      for (int j = 0; j < dim[1]; j += increment) {
         int ijk[3];
         switch(axis) {
            case VolumeFile::VOLUME_AXIS_X:  // PARASAGITTAL
               ijk[0] = currentSlice;
               ijk[1] = i;
               ijk[2] = j;
               voxelSizeX = spacing[1];
               voxelSizeY = spacing[2];
               voxelOriginX = origin[1];
               voxelOriginY = origin[2];
               break;
            case VolumeFile::VOLUME_AXIS_Y:  // CORONAL
               ijk[0] = i;
               ijk[1] = currentSlice;
               ijk[2] = j;
               voxelSizeX = spacing[0];
               voxelSizeY = spacing[2];
               voxelOriginX = origin[0];
               voxelOriginY = origin[2];
               break;
            case VolumeFile::VOLUME_AXIS_Z:  // HORIZONTAL
               ijk[0] = i;
               ijk[1] = j;
               ijk[2] = currentSlice;
               voxelSizeX = spacing[0];
               voxelSizeY = spacing[1];
               voxelOriginX = origin[0];
               voxelOriginY = origin[1];
               break;
            case VolumeFile::VOLUME_AXIS_ALL:
            case VolumeFile::VOLUME_AXIS_OBLIQUE:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
            case VolumeFile::VOLUME_AXIS_UNKNOWN:
               break;
         }
         
         //
         // Don't draw black voxels
         // Note: Only draw voxel if its 4th color component is non-zero
         //
         unsigned char rgb[4];
         voxelColoring->getVoxelColoring(vf, ijk[0], ijk[1], ijk[2], rgb);
         if ((rgb[3] == VolumeFile::VOXEL_COLOR_STATUS_VALID) || drawAllVoxels) {
            //
            // Override coloring if thresholding an anatomical volume
            //
            if (thresholdValid) {
               float val = vf->getVoxel(ijk);
               if ((val > thresholdValue[0]) &&
                   (val < thresholdValue[1])) {
                  //
                  // Above threshold is green so turn off red and blue
                  //
                  rgb[0] = 0;
                  rgb[2] = 0;
               }
            }
            
            //
            // Set the voxels color
            //
            glColor4ub(rgb[0], rgb[1], rgb[2], alphaValue);
            
            //
            // If selecting voxels
            //
            if (selectFlag) {
               glPushName(ijk[0]);
               glPushName(ijk[1]);
               glPushName(ijk[2]);
               glPushName(axis);
            }
            
            //
            // Set voxel coordinate and draw it
            //
            float voxelX = i * voxelSizeX + voxelOriginX;
            float voxelY = j * voxelSizeY + voxelOriginY;
            
            //
            // Is voxel coordinate at center of voxel?
            // JWH 07/11/2008
            //
            voxelX -= (voxelSizeX * 0.5);
            voxelY -= (voxelSizeY * 0.5);
            
            if (drawVectorsFlag) {
               voxelX += voxelSizeX * 0.5;
               voxelY += voxelSizeY * 0.5;
               glBegin(GL_LINES);
                  glColor4ub(255, 0, 0, 255);
                  glVertex2f(voxelX, voxelY);
                  glColor4ub(255, 255, 0, 255);
                  const float mag = vf->getVoxel(ijk, 3);
                  switch(axis) {
                     case VolumeFile::VOLUME_AXIS_X:  // PARASAGITTAL
                        glVertex2f(voxelX + vf->getVoxel(ijk, 1) * mag,
                                   voxelY + vf->getVoxel(ijk, 2) * mag);
                        break;
                     case VolumeFile::VOLUME_AXIS_Y:  // CORONAL
                        glVertex2f(voxelX + vf->getVoxel(ijk, 0) * mag,
                                   voxelY + vf->getVoxel(ijk, 2) * mag);
                        break;
                     case VolumeFile::VOLUME_AXIS_Z:  // HORIZONTAL
                        glVertex2f(voxelX + vf->getVoxel(ijk, 0) * mag,
                                   voxelY + vf->getVoxel(ijk, 1) * mag);
                        break;
                     case VolumeFile::VOLUME_AXIS_ALL:
                     case VolumeFile::VOLUME_AXIS_OBLIQUE:
                     case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
                     case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
                     case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
                     case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
                     case VolumeFile::VOLUME_AXIS_UNKNOWN:
                        break;
                  }
               glEnd();
            }
            else {
               switch (drawType) {
                  case DisplaySettingsVolume::SEGMENTATION_DRAW_TYPE_SOLID:
                     glRectf(voxelX, voxelY, voxelX + voxelSizeX, voxelY + voxelSizeY);
                     break;
                  case DisplaySettingsVolume::SEGMENTATION_DRAW_TYPE_BLEND:
                     glRectf(voxelX, voxelY, voxelX + voxelSizeX, voxelY + voxelSizeY);
                     break;
                  case DisplaySettingsVolume::SEGMENTATION_DRAW_TYPE_BOX:
                     glBegin(GL_LINE_LOOP);
                        glVertex2f(voxelX, voxelY);
                        glVertex2f(voxelX + voxelSizeX, voxelY);
                        glVertex2f(voxelX + voxelSizeX, voxelY + voxelSizeY);
                        glVertex2f(voxelX, voxelY + voxelSizeY);
                     glEnd();
                     break;
                  case DisplaySettingsVolume::SEGMENTATION_DRAW_TYPE_CROSS:
                     glBegin(GL_LINES);
                        glVertex2f(voxelX, voxelY);
                        glVertex2f(voxelX + voxelSizeX, voxelY + voxelSizeY);
                        glVertex2f(voxelX + voxelSizeX, voxelY);
                        glVertex2f(voxelX, voxelY + voxelSizeY);
                     glEnd();
                     break;
               }
            }
            
            
            //
            // If selecting voxels
            //
            if (selectFlag) {
               glPopName();
               glPopName();
               glPopName();
               glPopName();
            }
         }
      }
   }
   
   //
   // Turn off alpha blending
   //   
   glDisable(GL_BLEND);
   
   if (selectFlag) {
      glPopName();
   }
}

/*
 * Called if an error occurs while creating a quadric object.
 */
static void
quadricErrorCallback(GLenum errorCode)
{
    const GLubyte* str = gluErrorString(errorCode);
    std::cout << "GLU Quadric Error: " << str << std::endl;
}

/**
 * draw a sphere.
 */
void 
BrainModelOpenGL::drawSphere(const float diameter)
{
   bool useDisplayList = false;
   
   if (useDisplayListsForShapes) {
      if (sphereDisplayList == 0) {
         std::cout << "ERROR: Sphere Display List has not been created.  Was initializeOpenGL() called ?" 
                   << std::endl;
      }
      else if (glIsList(sphereDisplayList) == GL_TRUE) {
         useDisplayList = true;
      }
      else {
         std::cout << "ERROR: sphere display list number is an invalid display list number." << std::endl;
      }
   }
   
   glPushMatrix();
      glScalef(diameter, diameter, diameter);
      if (useDisplayList) {
         glCallList(sphereDisplayList);
      }
      else {
         drawingCommandsSphere();
      }
   glPopMatrix();
}

/**
 * Create a cone quadric and put it in a display list.
 * The cone is one unit in diameter and one unit in height.
 */
void
BrainModelOpenGL::createCylinderQuadricAndDisplayList()
{
   if (cylinderQuadric != NULL) {
      gluDeleteQuadric(cylinderQuadric);
      cylinderQuadric = NULL;
   }
   
   cylinderQuadric = gluNewQuadric();
#ifdef Q_OS_MACX
   gluQuadricCallback(cylinderQuadric, GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#ifdef Q_OS_WIN32
   gluQuadricCallback(cylinderQuadric, GLU_ERROR, (void (__stdcall*)(void))quadricErrorCallback);
#endif
#ifdef Q_OS_LINUX 
   gluQuadricCallback(cylinderQuadric, (GLenum)GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#ifdef Q_OS_UNIX
#ifndef Q_OS_MACX
#ifndef Q_OS_LINUX 
   gluQuadricCallback(cylinderQuadric, GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#endif
#endif

#ifdef Q_OS_LINUX
   gluQuadricDrawStyle(cylinderQuadric, (GLenum)GLU_FILL);
   gluQuadricOrientation(cylinderQuadric, (GLenum)GLU_OUTSIDE);
   gluQuadricNormals(cylinderQuadric, (GLenum)GLU_SMOOTH);
#else
   gluQuadricDrawStyle(cylinderQuadric, GLU_FILL);
   gluQuadricOrientation(cylinderQuadric, GLU_OUTSIDE);
   gluQuadricNormals(cylinderQuadric, GLU_SMOOTH);
#endif

   if (useDisplayListsForShapes) {
      if (glIsList(cylinderDisplayList)) {
         glDeleteLists(cylinderDisplayList, 1);
      }
      cylinderDisplayList = glGenLists(1);
      glNewList(cylinderDisplayList, GL_COMPILE);
         drawingCommandsCylinder();
      glEndList();
   }
}

/**
 * Execute OpenGL commands to draw a cylinder.
 */
void 
BrainModelOpenGL::drawingCommandsCylinder()
{
   //gluCylinder(cylinderQuadric, 0.5, 0.5, 1.0, 10, 10);
   gluCylinder(cylinderQuadric, 0.5, 0.5, 1.0, QUADRIC_NUMBER_OF_SLICES, 1);
}
      
/**
 * Create a cone quadric and put it in a display list.
 * The cone is one unit in diameter and one unit in height.
 */
void
BrainModelOpenGL::createConeQuadricAndDisplayList()
{
   if (coneQuadric != NULL) {
      gluDeleteQuadric(coneQuadric);
      coneQuadric = NULL;
   }
   
   coneQuadric = gluNewQuadric();
#ifdef Q_OS_MACX
   gluQuadricCallback(coneQuadric, GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#ifdef Q_OS_WIN32
   gluQuadricCallback(coneQuadric, GLU_ERROR, (void (__stdcall*)(void))quadricErrorCallback);
#endif
#ifdef Q_OS_LINUX 
   gluQuadricCallback(coneQuadric, (GLenum)GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#ifdef Q_OS_UNIX
#ifndef Q_OS_MACX
#ifndef Q_OS_LINUX 
   gluQuadricCallback(coneQuadric, GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#endif
#endif

#ifdef Q_OS_LINUX
   gluQuadricDrawStyle(coneQuadric, (GLenum)GLU_FILL);
   gluQuadricOrientation(coneQuadric, (GLenum)GLU_OUTSIDE);
   gluQuadricNormals(coneQuadric, (GLenum)GLU_SMOOTH);
#else
   gluQuadricDrawStyle(coneQuadric, GLU_FILL);
   gluQuadricOrientation(coneQuadric, GLU_OUTSIDE);
   gluQuadricNormals(coneQuadric, GLU_SMOOTH);
#endif

   if (useDisplayListsForShapes) {
      if (glIsList(coneDisplayList)) {
         glDeleteLists(coneDisplayList, 1);
      }
      coneDisplayList = glGenLists(1);
      glNewList(coneDisplayList, GL_COMPILE);
         drawingCommandsCone();
      glEndList();
   }
}

/**
 * Execute OpenGL commands to draw a cone.
 */
void 
BrainModelOpenGL::drawingCommandsCone()
{
   gluCylinder(coneQuadric, 0.5, 0.0, 1.0, QUADRIC_NUMBER_OF_SLICES, 1); //10);
}

/**
 * Create a sphere quadric and put it in a display list.
 * The sphere is one unit in diameter.
 */
void
BrainModelOpenGL::createSphereQuadricAndDisplayList()
{
   if (sphereQuadric != NULL) {
      gluDeleteQuadric(sphereQuadric);
      sphereQuadric = NULL;
   }
   
   sphereQuadric = gluNewQuadric();
#ifdef Q_OS_MACX
   gluQuadricCallback(sphereQuadric, GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#ifdef Q_OS_WIN32
   gluQuadricCallback(sphereQuadric, GLU_ERROR, (void (__stdcall*)(void))quadricErrorCallback);
#endif
#ifdef Q_OS_LINUX 
   gluQuadricCallback(sphereQuadric, (GLenum)GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#ifdef Q_OS_UNIX
#ifndef Q_OS_MACX
#ifndef Q_OS_LINUX 
   gluQuadricCallback(sphereQuadric, GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#endif
#endif

#ifdef Q_OS_LINUX
   gluQuadricDrawStyle(sphereQuadric, (GLenum)GLU_FILL);
   gluQuadricOrientation(sphereQuadric, (GLenum)GLU_OUTSIDE);
   gluQuadricNormals(sphereQuadric, (GLenum)GLU_SMOOTH);
#else
   gluQuadricDrawStyle(sphereQuadric, GLU_FILL);
   gluQuadricOrientation(sphereQuadric, GLU_OUTSIDE);
   gluQuadricNormals(sphereQuadric, GLU_SMOOTH);
#endif

   
   if (useDisplayListsForShapes) {
      if (glIsList(sphereDisplayList)) {
         glDeleteLists(sphereDisplayList, 1);
      }
      sphereDisplayList = glGenLists(1);
      if (sphereDisplayList == 0) {
         std::cout << "ERROR: Unable to create a display list for the sphere." << std::endl;
         return;
      }
      glNewList(sphereDisplayList, GL_COMPILE);
         drawingCommandsSphere();
      glEndList();
   }
}

/**
 * Execute OpenGL commands to draw a sphere.
 */
void BrainModelOpenGL::drawingCommandsSphere()
{
   gluSphere(sphereQuadric, 0.5, 10, 10);
}

/**
 * draw a 2D Disk (filled circle).
 */
void 
BrainModelOpenGL::drawDisk(const float diameter)
{
   bool useDisplayList = false;
   
   if (useDisplayListsForShapes) {
      if (diskDisplayList == 0) {
         std::cout << "Disk Display List has not been created.  Was initializeOpenGL() called ?" 
                   << std::endl;
      }
      else if (glIsList(diskDisplayList) == GL_TRUE) {
         useDisplayList = true;
      }
      else {
         std::cout << "ERROR: Disk display list number is an invalid display list number." << std::endl;
      }
   }
   
   glPushMatrix();
      glScalef(diameter, diameter, diameter);
      if (useDisplayList) {
         glCallList(diskDisplayList);
      }
      else {
         drawingCommandsDisk();
      }
   glPopMatrix();
}

/**
 * draw a cone.
 */
void 
BrainModelOpenGL::drawCone()
{
   bool useDisplayList = false;
   
   if (useDisplayListsForShapes) {
      if (coneDisplayList == 0) {
         std::cout << "ERROR: Cone Display List has not been created.  Was initializeOpenGL() called ?" 
                   << std::endl;
      }
      else if (glIsList(coneDisplayList) == GL_TRUE) {
         useDisplayList = true;
      }
      else {
         std::cout << "ERROR: Cone display list number is an invalid display list number." << std::endl;
      }
   }
   
   glPushMatrix();
      if (useDisplayList) {
         glCallList(coneDisplayList);
      }
      else {
         drawingCommandsCone();
      }
   glPopMatrix();
}

/**
 * draw cylinder.
 */
void 
BrainModelOpenGL::drawCylinder()
{
   bool useDisplayList = false;
   
   if (useDisplayListsForShapes) {
      if (cylinderDisplayList == 0) {
         std::cout << "ERROR: Cylinder Display List has not been created.  Was initializeOpenGL() called ?" 
                   << std::endl;
      }
      else if (glIsList(cylinderDisplayList) == GL_TRUE) {
         useDisplayList = true;
      }
      else {
         std::cout << "ERROR: Cylinder display list number is an invalid display list number." << std::endl;
      }
   }
   
   glPushMatrix();
      if (useDisplayList) {
         glCallList(cylinderDisplayList);
      }
      else {
         drawingCommandsCylinder();
      }
   glPopMatrix();
}

/**
 * draw a square.
 */
void 
BrainModelOpenGL::drawSquare()
{
   bool useDisplayList = false;
   
   if (useDisplayListsForShapes) {
      if (squareDisplayList == 0) {
         std::cout << "ERROR: Square Display List has not been created.  Was initializeOpenGL() called ?" 
                   << std::endl;
      }
      else if (glIsList(squareDisplayList) == GL_TRUE) {
         useDisplayList = true;
      }
      else {
         std::cout << "ERROR: Square display list number is an invalid display list number." << std::endl;
      }
   }
   
   glPushMatrix();
      if (useDisplayList) {
         glCallList(squareDisplayList);
      }
      else {
         drawingCommandsSquare();
      }
   glPopMatrix();
}
      
/**
 * draw a box.
 */
void 
BrainModelOpenGL::drawBox()
{
   bool useDisplayList = false;
   
   if (useDisplayListsForShapes) {
      if (boxDisplayList == 0) {
         std::cout << "ERROR: Box Display List has not been created.  Was initializeOpenGL() called ?" 
                   << std::endl;
      }
      else if (glIsList(boxDisplayList) == GL_TRUE) {
         useDisplayList = true;
      }
      else {
         std::cout << "ERROR: Box display list number is an invalid display list number." << std::endl;
      }
   }
   
   glPushMatrix();
      if (useDisplayList) {
         glCallList(boxDisplayList);
      }
      else {
         drawingCommandsBox();
      }
   glPopMatrix();
}

/**
 * a ring.
 */
void 
BrainModelOpenGL::drawRing()
{
   bool useDisplayList = false;
   
   if (useDisplayListsForShapes) {
      if (ringDisplayList == 0) {
         std::cout << "ERROR: Ring Display List has not been created.  Was initializeOpenGL() called ?" 
                   << std::endl;
      }
      else if (glIsList(ringDisplayList) == GL_TRUE) {
         useDisplayList = true;
      }
      else {
         std::cout << "ERROR: Ring display list number is an invalid display list number." << std::endl;
      }
   }
   
   glPushMatrix();
      if (useDisplayList) {
         glCallList(ringDisplayList);
      }
      else {
         drawingCommandsRing();
      }
   glPopMatrix();
}

/**
 * draw a diamond.
 */
void 
BrainModelOpenGL::drawDiamond()
{
   bool useDisplayList = false;
   
   if (useDisplayListsForShapes) {
      if (diamondDisplayList == 0) {
         std::cout << "ERROR: Diamond Display List has not been created.  Was initializeOpenGL() called ?" 
                   << std::endl;
      }
      else if (glIsList(diamondDisplayList) == GL_TRUE) {
         useDisplayList = true;
      }
      else {
         std::cout << "ERROR: Diamond display list number is an invalid display list number." << std::endl;
      }
   }
   
   glPushMatrix();
      if (useDisplayList) {
         glCallList(diamondDisplayList);
      }
      else {
         drawingCommandsDiamond();
      }
   glPopMatrix();
}

/**
 * Create a 2D disk quadric (filled circle) and put it in a display list.
 * The disk is one unit in diameter.
 */
void
BrainModelOpenGL::createDiskQuadricAndDisplayList()
{
   if (diskQuadric != NULL) {
      gluDeleteQuadric(diskQuadric);
      diskQuadric = NULL;
   }
   
   diskQuadric = gluNewQuadric();
#ifdef Q_OS_MACX
   gluQuadricCallback(diskQuadric, GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#ifdef Q_OS_WIN32
   gluQuadricCallback(diskQuadric, GLU_ERROR, (void (__stdcall*)(void))quadricErrorCallback);
#endif
#ifdef Q_OS_LINUX 
   gluQuadricCallback(diskQuadric, (GLenum)GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#ifdef Q_OS_UNIX
#ifndef Q_OS_MACX
#ifndef Q_OS_LINUX 
   gluQuadricCallback(diskQuadric, GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#endif
#endif

#ifdef Q_OS_LINUX
   gluQuadricDrawStyle(diskQuadric, (GLenum)GLU_FILL);
   gluQuadricOrientation(diskQuadric, (GLenum)GLU_OUTSIDE);
   gluQuadricNormals(diskQuadric, (GLenum)GLU_SMOOTH);
#else
   gluQuadricDrawStyle(diskQuadric, GLU_FILL);
   gluQuadricOrientation(diskQuadric, GLU_OUTSIDE);
   gluQuadricNormals(diskQuadric, GLU_SMOOTH);
#endif

   if (useDisplayListsForShapes) {
      if (glIsList(diskDisplayList)) {
         glDeleteLists(diskDisplayList, 1);
      }
      diskDisplayList = glGenLists(1);
      glNewList(diskDisplayList, GL_COMPILE);
         drawingCommandsDisk();
      glEndList();
   }
}

/**
 * Execute OpenGL commands to draw a disk.
 */
void 
BrainModelOpenGL::drawingCommandsDisk()
{
   gluDisk(diskQuadric, 0.0, 0.5, QUADRIC_NUMBER_OF_SLICES, 1);
}
      
/**
 * Create a square and put it in a display list.
 */
void 
BrainModelOpenGL::createSquareDisplayList()
{
   if (useDisplayListsForShapes) {
      if (glIsList(squareDisplayList)) {
         glDeleteLists(squareDisplayList, 1);
      }
      squareDisplayList = glGenLists(1);
      glNewList(squareDisplayList, GL_COMPILE);
         drawingCommandsSquare();
      glEndList();
   }
}

/**
 * Create a box and put it in a display list.
 */
void 
BrainModelOpenGL::createBoxDisplayList()
{
   if (useDisplayListsForShapes) {
      if (glIsList(boxDisplayList)) {
         glDeleteLists(boxDisplayList, 1);
      }
      boxDisplayList = glGenLists(1);
      glNewList(boxDisplayList, GL_COMPILE);
         drawingCommandsBox();
      glEndList();
   }
}


/**
 * Execute OpenGL commands to draw a square.
 */
void 
BrainModelOpenGL::drawingCommandsSquare()
{
   glBegin(GL_QUADS);
      glNormal3f(0.0, 0.0, 1.0);
      glVertex3f(-0.5, -0.5, 0.0);
      glVertex3f( 0.5, -0.5, 0.0);
      glVertex3f( 0.5,  0.5, 0.0);
      glVertex3f(-0.5,  0.5, 0.0);
   glEnd();
}
      

/**
 * Execute OpenGL commands to draw a box.
 */
void 
BrainModelOpenGL::drawingCommandsBox()
{
   glBegin(GL_QUADS);
      //
      // Bottom
      //
      glNormal3f(0.0, 0.0, -1.0);
      glVertex3f(-0.5,  0.5, -0.5);
      glVertex3f( 0.5,  0.5, -0.5);
      glVertex3f( 0.5, -0.5, -0.5);
      glVertex3f(-0.5, -0.5, -0.5);
      //
      // Top
      //
      glNormal3f(0.0, 0.0, 1.0);
      glVertex3f(-0.5, -0.5, 0.5);
      glVertex3f( 0.5, -0.5, 0.5);
      glVertex3f( 0.5,  0.5, 0.5);
      glVertex3f(-0.5,  0.5, 0.5);
      //
      // Left
      //
      glNormal3f(-1.0, 0.0, 0.0);
      glVertex3f(-0.5,  0.5, -0.5);
      glVertex3f(-0.5, -0.5, -0.5);
      glVertex3f(-0.5, -0.5,  0.5);
      glVertex3f(-0.5,  0.5,  0.5);
      //
      // Right
      //
      glNormal3f(1.0, 0.0, 0.0);
      glVertex3f( 0.5, -0.5, -0.5);
      glVertex3f( 0.5,  0.5, -0.5);
      glVertex3f( 0.5,  0.5,  0.5);
      glVertex3f( 0.5, -0.5,  0.5);
      //
      // Front
      //
      glNormal3f(0.0, -1.0, 0.0);
      glVertex3f(-0.5, -0.5, -0.5);
      glVertex3f( 0.5, -0.5, -0.5);
      glVertex3f( 0.5, -0.5,  0.5);
      glVertex3f(-0.5, -0.5,  0.5);
      //
      // Back
      //
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f( 0.5, 0.5, -0.5);
      glVertex3f(-0.5, 0.5, -0.5);
      glVertex3f(-0.5, 0.5, 0.5);
      glVertex3f( 0.5, 0.5, 0.5);
   glEnd();
}

/**
 * Create a ring and put it in a display list.
 */
void 
BrainModelOpenGL::createRingQuadricAndDisplayList()
{
   if (ringQuadric != NULL) {
      gluDeleteQuadric(ringQuadric);
      ringQuadric = NULL;
   }
   
   ringQuadric = gluNewQuadric();
#ifdef Q_OS_MACX
   gluQuadricCallback(ringQuadric, GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#ifdef Q_OS_WIN32
   gluQuadricCallback(ringQuadric, GLU_ERROR, (void (__stdcall*)(void))quadricErrorCallback);
#endif
#ifdef Q_OS_LINUX 
   gluQuadricCallback(ringQuadric, (GLenum)GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#ifdef Q_OS_UNIX
#ifndef Q_OS_MACX
#ifndef Q_OS_LINUX 
   gluQuadricCallback(ringQuadric, GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#endif
#endif

#ifdef Q_OS_LINUX
   gluQuadricDrawStyle(ringQuadric, (GLenum)GLU_FILL);
   gluQuadricOrientation(ringQuadric, (GLenum)GLU_OUTSIDE);
   gluQuadricNormals(ringQuadric, (GLenum)GLU_SMOOTH);
#else
   gluQuadricDrawStyle(ringQuadric, GLU_FILL);
   gluQuadricOrientation(ringQuadric, GLU_OUTSIDE);
   gluQuadricNormals(ringQuadric, GLU_SMOOTH);
#endif

   if (useDisplayListsForShapes) {
      if (glIsList(ringDisplayList)) {
         glDeleteLists(ringDisplayList, 1);
      }
      ringDisplayList = glGenLists(1);
      glNewList(ringDisplayList, GL_COMPILE);
         drawingCommandsRing();
      glEndList();
   }
}      

/**
 * Execute OpenGL commands to draw a ring.
 */
void 
BrainModelOpenGL::drawingCommandsRing()
{
   gluDisk(ringQuadric, 0.4, 0.5, 8, 3);
}

/**
 * Create a diamond and put it in a display list.
 */
void 
BrainModelOpenGL::createDiamondQuadricAndDisplayList()
{
   if (diamondQuadric != NULL) {
      gluDeleteQuadric(diamondQuadric);
      diamondQuadric = NULL;
   }
   
   diamondQuadric = gluNewQuadric();
#ifdef Q_OS_MACX
   gluQuadricCallback(diamondQuadric, GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#ifdef Q_OS_WIN32
   gluQuadricCallback(diamondQuadric, GLU_ERROR, (void (__stdcall*)(void))quadricErrorCallback);
#endif
#ifdef Q_OS_LINUX 
   gluQuadricCallback(diamondQuadric, (GLenum)GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#ifdef Q_OS_UNIX
#ifndef Q_OS_MACX
#ifndef Q_OS_LINUX 
   gluQuadricCallback(diamondQuadric, GLU_ERROR, (GLvoid (*)())quadricErrorCallback);
#endif
#endif
#endif

#ifdef Q_OS_LINUX
   gluQuadricDrawStyle(diamondQuadric, (GLenum)GLU_FILL);
   gluQuadricOrientation(diamondQuadric, (GLenum)GLU_OUTSIDE);
   gluQuadricNormals(diamondQuadric, (GLenum)GLU_FLAT); //GLU_SMOOTH);
#else
   gluQuadricDrawStyle(diamondQuadric, GLU_FILL);
   gluQuadricOrientation(diamondQuadric, GLU_OUTSIDE);
   gluQuadricNormals(diamondQuadric, GLU_FLAT); //GLU_SMOOTH);
#endif

   if (useDisplayListsForShapes) {
      if (glIsList(diamondDisplayList)) {
         glDeleteLists(diamondDisplayList, 1);
      }
      diamondDisplayList = glGenLists(1);
      glNewList(diamondDisplayList, GL_COMPILE);
         drawingCommandsDiamond();
      glEndList();
   }
}

/**
 * Execute OpenGL commands to draw a diamond.
 */
void 
BrainModelOpenGL::drawingCommandsDiamond()
{
   gluSphere(diamondQuadric, 0.5, 4, 2);
}
      
/**
 * Draw the surface forces
 */
void
BrainModelOpenGL::drawSurfaceForces(const CoordinateFile* cf,
                                    const int numCoords)
{
   DisplaySettingsSurface* dsn = brainSet->getDisplaySettingsSurface();
   const float length = dsn->getForceVectorDisplayLength();
   
   glLineWidth(getValidLineWidth(1.0));
   
   glBegin(GL_LINES);
   for (int i = 0; i < numCoords; i++) {
      BrainSetNodeAttribute* bna = brainSet->getNodeAttributes(i);
      if (bna->getDisplayFlag()) {
         float linearForce[3], angularForce[3], totalForce[3];
         bna->getMorphingForces(linearForce, angularForce, totalForce);
         
         const float* pos = cf->getCoordinate(i);
         if (dsn->getShowMorphingTotalForces()) {
            float vect[3] = { pos[0] + length * totalForce[0],
                              pos[1] + length * totalForce[1],
                              pos[2] + length * totalForce[2] };
            glColor3ub(0, 0, 255);
            glVertex3fv(pos);
            glVertex3fv(vect);
         }
   
         if (dsn->getShowMorphingAngularForces()) {
            float vect[3] = { pos[0] + length * angularForce[0],
                              pos[1] + length * angularForce[1],
                              pos[2] + length * angularForce[2] };
            glColor3ub(0, 255, 0);
            glVertex3fv(pos);
            glVertex3fv(vect);
         }
         
         if (dsn->getShowMorphingLinearForces()) {
            float vect[3] = { pos[0] + length * linearForce[0],
                              pos[1] + length * linearForce[1],
                              pos[2] + length * linearForce[2] };
            glColor3ub(255, 0, 0);
            glVertex3fv(pos);
            glVertex3fv(vect);
         }
      }
   }
   glEnd();
}

/**
 * Draw the surface as points.
 */
void 
BrainModelOpenGL::drawSurfaceNodes(const BrainModelSurfaceNodeColoring* bs,
                                   const int modelNumber,
                                   const CoordinateFile* cf, 
                                   const int numCoords,
                                   const bool drawInSurfaceEditColor)
{
   DisplaySettingsSurface* dsn = brainSet->getDisplaySettingsSurface();
   const BrainSetNodeAttribute* attributes = brainSet->getNodeAttributes(0);
   const DisplaySettingsSurface::DRAW_MODE surfaceDrawingMode = dsn->getDrawMode();

   glPointSize(getValidPointSize(dsn->getNodeSize()));
   
   if (drawInSurfaceEditColor) {
      glColor3ubv(surfaceEditDrawColor);
      glBegin(GL_POINTS);
         for (int i = 0; i < numCoords; i++) {
            if (attributes[i].getDisplayFlag()) {
               glVertex3fv(cf->getCoordinate(i));
            }
         }
      glEnd();
      
      if (nodeSpecialHighlighting.empty() == false) {
         glPointSize(getValidPointSize(dsn->getNodeSize() * 2.0));
         glColor3ub(255, 0, 0);
         glBegin(GL_POINTS);
         for (int i = 0; i < static_cast<int>(nodeSpecialHighlighting.size()); i++) {
            glVertex3fv(cf->getCoordinate(nodeSpecialHighlighting[i]));
         }
         glEnd();
      }
      return;
   }

   if (selectionMask == SELECTION_MASK_OFF) {
#ifdef GL_VERSION_1_1
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_COLOR_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, cf->getCoordinate(0));
      glColorPointer(4, GL_UNSIGNED_BYTE, 0, bs->getNodeColor(modelNumber, 0));
      if (brainSet->getDisplayAllNodes()) {
         glDrawArrays(GL_POINTS, 0, numCoords);
      }
      else {
         for (int i = 0; i < numCoords; i++) {
            if (attributes[i].getDisplayFlag()) {
               glArrayElement(i);
            }
         }
      }
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_COLOR_ARRAY);
#else  // GL_VERSION_1_1
      glBegin(GL_POINTS);
         for (int i = 0; i < numCoords; i++) {
            if (attributes[i]->getDisplayFlag()) {
               glColor4ubv(bs->getNodeColor(i));
               glVertex3fv(cf->getCoordinate(i));
            }
         }
      glEnd();
#endif  // GL_VERSION_1_1
      if (nodeSpecialHighlighting.empty() == false) {
         glPointSize(getValidPointSize(dsn->getNodeSize() * 2.0));
         glColor3ub(255, 0, 0);
         glBegin(GL_POINTS);
         for (int i = 0; i < static_cast<int>(nodeSpecialHighlighting.size()); i++) {
            glVertex3fv(cf->getCoordinate(nodeSpecialHighlighting[i]));
         }
         glEnd();
      }
   }
   else if (selectionMask & SELECTION_MASK_NODE) {
      glPushName(SELECTION_MASK_NODE);
      for (int i = 0; i < numCoords; i++) {
         if (attributes[i].getDisplayFlag()) {
            bool drawIt = true;
            if (surfaceDrawingMode == DisplaySettingsSurface::DRAW_MODE_LINKS_EDGES_ONLY) {
               drawIt = (attributes[i].getClassification() != 
                         BrainSetNodeAttribute::CLASSIFICATION_TYPE_INTERIOR);
            }
            if (drawIt) {
               glPushName(i);
               glBegin(GL_POINTS);
                  glVertex3fv(cf->getCoordinate(i));
               glEnd();
               glPopName();
            }
         }
      }
      glPopName();
   }
}

/**
 * Draw the surface as links.
 */
void 
BrainModelOpenGL::drawSurfaceLinks(const BrainModelSurfaceNodeColoring* bs,
                                   const int modelNumber,
                                   const CoordinateFile* cf,
                                   const TopologyFile* tf, const int numTiles,
                                   const bool drawEdgesOnly,
                                   const bool drawInSurfaceEditColor)
{
   DisplaySettingsSurface* dsn = brainSet->getDisplaySettingsSurface();
   const BrainSetNodeAttribute* attributes = brainSet->getNodeAttributes(0);
   
   glLineWidth(getValidLineWidth(dsn->getLinkSize()));
   
   const bool idLinkMode = (selectionMask & SELECTION_MASK_LINK);
   if (idLinkMode) {
      glLineWidth(getValidLineWidth(5.0));
   }
   
   if (idLinkMode == false) {
      glBegin(GL_LINES);
   }
      for (int i = 0; i < numTiles; i++) {
         int v1, v2, v3;
         tf->getTile(i, v1, v2, v3);
         
         bool drawV1 = attributes[v1].getDisplayFlag();
         bool drawV2 = attributes[v2].getDisplayFlag();
         bool drawV3 = attributes[v3].getDisplayFlag();
         if (drawEdgesOnly) {
            if (drawV1) {
               drawV1 = (attributes[v1].getClassification() != 
                         BrainSetNodeAttribute::CLASSIFICATION_TYPE_INTERIOR);
            }
            if (drawV2) {
               drawV2 = (attributes[v2].getClassification() != 
                         BrainSetNodeAttribute::CLASSIFICATION_TYPE_INTERIOR);
            }
            if (drawV3) {
               drawV3 = (attributes[v3].getClassification() != 
                         BrainSetNodeAttribute::CLASSIFICATION_TYPE_INTERIOR);
            }
         }
         
         if (drawV1 && drawV2) {
            if (idLinkMode) {
               glPushName(SELECTION_MASK_LINK);
               glPushName(v1);
               glPushName(v2);
               glBegin(GL_LINES);
            }
            if (drawInSurfaceEditColor) {
               glColor3ubv(surfaceEditDrawColor);
            }
            else {
               glColor4ubv(bs->getNodeColor(modelNumber, v1));
            }
            glVertex3fv(cf->getCoordinate(v1));
            if (drawInSurfaceEditColor) {
               glColor3ubv(surfaceEditDrawColor);
            }
            else {
               glColor4ubv(bs->getNodeColor(modelNumber, v2));
            }
            glVertex3fv(cf->getCoordinate(v2));
            if (idLinkMode) {
               glEnd();
               glPopName();
               glPopName();
               glPopName();
            }
         } 
         if (drawV2 && drawV3) {
            if (idLinkMode) {
               glPushName(SELECTION_MASK_LINK);
               glPushName(v2);
               glPushName(v3);
               glBegin(GL_LINES);
            }
            if (drawInSurfaceEditColor) {
               glColor3ubv(surfaceEditDrawColor);
            }
            else {
               glColor4ubv(bs->getNodeColor(modelNumber, v2));
            }
            glVertex3fv(cf->getCoordinate(v2));
            if (drawInSurfaceEditColor) {
               glColor3ubv(surfaceEditDrawColor);
            }
            else {
               glColor4ubv(bs->getNodeColor(modelNumber, v3));
            }
            glVertex3fv(cf->getCoordinate(v3));
            if (idLinkMode) {
               glEnd();
               glPopName();
               glPopName();
               glPopName();
            }
         }
         if (drawV1 && drawV3) {
            if (idLinkMode) {
               glPushName(SELECTION_MASK_LINK);
               glPushName(v1);
               glPushName(v3);
               glBegin(GL_LINES);
            }
            if (drawInSurfaceEditColor) {
               glColor3ubv(surfaceEditDrawColor);
            }
            else {
               glColor4ubv(bs->getNodeColor(modelNumber, v1));
            }
            glVertex3fv(cf->getCoordinate(v1));
            if (drawInSurfaceEditColor) {
               glColor3ubv(surfaceEditDrawColor);
            }
            else {
               glColor4ubv(bs->getNodeColor(modelNumber, v3));
            }
            glVertex3fv(cf->getCoordinate(v3));
            if (idLinkMode) {
               glEnd();
               glPopName();
               glPopName();
               glPopName();
            }
         }
      }
   if (idLinkMode == false) {
      glEnd();
   }
}
   
/**
 * Draw the surface as links with hidden line removal.
 * From page 585 of OpenGL Programming Guide Version 1.2.
 */
/*
void 
BrainModelOpenGL::drawSurfaceLinksNoBackside(const BrainModelSurfaceNodeColoring* bs,
                                        const int modelNumber,
                                        const CoordinateFile* cf,
                                        const TopologyFile* tf, const int numTiles)
{
   glDisable(GL_LIGHTING);
   glDisable(GL_COLOR_MATERIAL);

   BrainSetNodeAttribute* attributes = brainSet->getNodeAttributes(0);
   DisplaySettingsSurface* dsn = brainSet->getDisplaySettingsSurface();

   glLineWidth(getValidLineWidth(dsn->getLinkSize()));

   //
   // First, draw as wireframe
   //
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   for (int i = 0; i < numTiles; i++) {
      int v1, v2, v3;
      tf->getTile(i, v1, v2, v3);
      if (attributes[v1].getDisplayFlag() ||
          attributes[v2].getDisplayFlag() ||
          attributes[v3].getDisplayFlag()) {
         glBegin(GL_POLYGON);
            glColor4ubv(bs->getNodeColor(modelNumber, v1));
            glVertex3fv(cf->getCoordinate(v1));
            glColor4ubv(bs->getNodeColor(modelNumber, v2));
            glVertex3fv(cf->getCoordinate(v2));
            glColor4ubv(bs->getNodeColor(modelNumber, v3));
            glVertex3fv(cf->getCoordinate(v3));
         glEnd();
      }
   }
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   //
   // set background color
   //
   unsigned char rb, gb, bb;
   PreferencesFile* pref = brainSet->getPreferencesFile();
   pref->getSurfaceBackgroundColor(rb, gb, bb);
   glColor3ub(rb, gb, bb);

   //
   // Now draw with tiles to hide the "hidden" portion
   //
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glEnable(GL_POLYGON_OFFSET_FILL);
   glPolygonOffset(1.0, 1.0);
   // glLineWidth(getValidLineWidth(1.0));
   for (int i = 0; i < numTiles; i++) {
      int v1, v2, v3;
      tf->getTile(i, v1, v2, v3);
      if (attributes[v1].getDisplayFlag() ||
          attributes[v2].getDisplayFlag() ||
          attributes[v3].getDisplayFlag()) {
         glBegin(GL_POLYGON);
            glVertex3fv(cf->getCoordinate(v1));
            glVertex3fv(cf->getCoordinate(v2));
            glVertex3fv(cf->getCoordinate(v3));
         glEnd();
      }
   }
   glDisable(GL_POLYGON_OFFSET_FILL);
}
*/

/**
 * Draw the surface as tiles, possibly with lighting.
 */
void 
BrainModelOpenGL::drawSurfaceTiles(const BrainModelSurfaceNodeColoring* bs,
                                   const BrainModelSurface* s,
                                   const CoordinateFile* cf,
                                   TopologyFile* tf, const int numTiles,
                                   const int numCoords)
{  
   //
   // Enable opacity
   //
   const DisplaySettingsSurface* dsn = brainSet->getDisplaySettingsSurface();
   if (dsn->getOpacity() < 1.0) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   }

   const BrainSetNodeAttribute* attributes = brainSet->getNodeAttributes(0);
   
   const int modelNumber = s->getBrainModelIndex();
   
   glDisable(GL_LIGHTING);
   glDisable(GL_COLOR_MATERIAL);

   bool partialLighting = false;
   
   //
   // Do not light a flat surface
   //
   if ((s->getSurfaceType() != BrainModelSurface::SURFACE_TYPE_FLAT) &&
       (s->getSurfaceType() != BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR)) {
      glEnable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);

      int numLightsOn = 0;
      for (int i = 0; i < brainSet->getNumberOfSurfaceOverlays(); i++) {
         if (brainSet->getSurfaceOverlay(i)->getLightingEnabled()) {
            numLightsOn++;
         }
      }
      if (numLightsOn > 0) {
         if (numLightsOn < brainSet->getNumberOfSurfaceOverlays()) {
            partialLighting = true;
         }
      }
      else {
         glDisable(GL_LIGHTING);
         glDisable(GL_COLOR_MATERIAL);
      }
   }
   
   if (selectionMask & SELECTION_MASK_TILE) {
      //
      // Set partialLighting so that each tile is drawn individually and push the mask
      //
      partialLighting = true;
      glPushName(SELECTION_MASK_TILE);
   }
   
#ifdef GL_VERSION_1_1
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_COLOR_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, cf->getCoordinate(0));
      glColorPointer(4, GL_UNSIGNED_BYTE, 0, bs->getNodeColor(modelNumber, 0));
      glNormalPointer(GL_FLOAT, 0, s->getNormal(0));
#endif  // GL_VERSION_1_1

   if (partialLighting) {
      static std::vector<bool> lightNodeFlag;
      if (numCoords != static_cast<int>(lightNodeFlag.size())) {
         lightNodeFlag.resize(numCoords);
      }
      
      for (int j = 0; j < numCoords; j++) {
         lightNodeFlag[j] = false;
         const int nodeOverlayIndex = bs->getNodeColorSource(modelNumber, j);
         if (nodeOverlayIndex >= 0) {
            if (brainSet->getSurfaceOverlay(nodeOverlayIndex)->getLightingEnabled()) {
               lightNodeFlag[j] = true;
            }
         }
      }
      for (int k = 0; k < 2; k++) {
         if (k == 1) {
            glDisable(GL_LIGHTING);
            glDisable(GL_COLOR_MATERIAL);
         }
         
         for (int i = 0; i < numTiles; i++) {
            int v1, v2, v3;
            tf->getTile(i, v1, v2, v3);
            if (attributes[v1].getDisplayFlag() || 
                attributes[v2].getDisplayFlag() || 
                attributes[v3].getDisplayFlag()) {
               
               bool drawIt = false;
               if (k == 0) {
                  if (lightNodeFlag[v1] && lightNodeFlag[v2] && lightNodeFlag[v3]) {
                     drawIt = true;
                  }
               }
               else {
                  if ((lightNodeFlag[v1] == false) ||
                      (lightNodeFlag[v2] == false) ||
                      (lightNodeFlag[v3] == false)) {
                     drawIt = true;
                  }
               }
               
               if (drawIt) {
                  if (selectionMask & SELECTION_MASK_TILE) {
                     glPushName(i);
                  }
#ifdef GL_VERSION_1_1
                  unsigned int triangle[3] = { v1, v2, v3 };
                  glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 
                              static_cast<const GLvoid*>(triangle));
#else  // GL_VERSION_1_1
                  glBegin(GL_TRIANGLES);
                     glColor4ubv(bs->getNodeColor(modelNumber, v1));
                     glNormal3fv(s->getNormal(v1));
                     glVertex3fv(cf->getCoordinate(v1));
                     glColor4ubv(bs->getNodeColor(modelNumber, v2));
                     glNormal3fv(s->getNormal(v2));
                     glVertex3fv(cf->getCoordinate(v2));
                     glColor4ubv(bs->getNodeColor(modelNumber, v3));
                     glNormal3fv(s->getNormal(v3));
                     glVertex3fv(cf->getCoordinate(v3));
                  glEnd();
#endif  // GL_VERSION_1_1
                  if (selectionMask & SELECTION_MASK_TILE) {
                     glPopName();
                  }
               }
            }
         }
      }
   }
   else {
#ifdef GL_VERSION_1_1
      if (brainSet->getDisplayAllNodes()) {
         glDrawElements(GL_TRIANGLES, (3 * numTiles), GL_UNSIGNED_INT, 
                        static_cast<const GLvoid*>(tf->getTile(0)));
      }
      else {
         for (int i = 0; i < numTiles; i++) {
            const int* theTile = tf->getTile(i);
            if (attributes[theTile[0]].getDisplayFlag() ||
                attributes[theTile[1]].getDisplayFlag() ||
                attributes[theTile[2]].getDisplayFlag()) {
               glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 
                              static_cast<const GLvoid*>(theTile));
            }
         }
      }
#else  // GL_VERSION_1_1
      glBegin(GL_TRIANGLES);
         for (int i = 0; i < numTiles; i++) {
            unsigned int v1, v2, v3;
            tf->getTile(i, v1, v2, v3);
            if (attributes[v1].getDisplayFlag() || 
                attributes[v2].getDisplayFlag() || 
                attributes[v3].getDisplayFlag()) {
               glColor4ubv(bs->getNodeColor(modelNumber, v1));
               glNormal3fv(s->getNormal(v1));
               glVertex3fv(cf->getCoordinate(v1));
               glColor4ubv(bs->getNodeColor(modelNumber, v2));
               glNormal3fv(s->getNormal(v2));
               glVertex3fv(cf->getCoordinate(v2));
               glColor4ubv(bs->getNodeColor(modelNumber, v3));
               glNormal3fv(s->getNormal(v3));
               glVertex3fv(cf->getCoordinate(v3));
            }
         }
      glEnd();
#endif  // GL_VERSION_1_1
      
   }
   glDisable(GL_LIGHTING);
   glDisable(GL_COLOR_MATERIAL);
   
#ifdef GL_VERSION_1_1
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
#endif  // GL_VERSION_1_1

   if (selectionMask & SELECTION_MASK_TILE) {
      glPopName();
   }
   
   //
   // Disable opacity
   //
   glDisable(GL_BLEND);

   glDisable(GL_POLYGON_STIPPLE);
}

/**
 * Draw Surface ROI members.
 */
void 
BrainModelOpenGL::drawSurfaceROIMembers(const BrainModelSurface* bms, const int numCoords)
{
   const CoordinateFile* cf = bms->getCoordinateFile();
   BrainModelSurfaceROINodeSelection* 
                   surfaceROI = brainSet->getBrainModelSurfaceRegionOfInterestNodeSelection();
   const DisplaySettingsSurface* dsn = brainSet->getDisplaySettingsSurface();
                   
   surfaceROI->update();
   if (surfaceROI->getDisplaySelectedNodes()) {
      glPointSize(getValidPointSize(dsn->getNodeSize()));
      glColor3ub(0, 200, 0);  // darker green
      glBegin(GL_POINTS);
         for (int i = 0; i < numCoords; i++) {
            const BrainSetNodeAttribute* bna = brainSet->getNodeAttributes(i);
            if (bna->getDisplayFlag()) { // node displayed check
               if (surfaceROI->getNodeSelected(i)) {
                  glVertex3fv(cf->getCoordinate(i));
               }
            }
         }
      glEnd();
   }
}
      
/**
 * Draw node highlighting.
 */
void 
BrainModelOpenGL::drawNodeHighlighting(const BrainModelSurface* bms, const int numCoords)
{
   const CoordinateFile* cf = bms->getCoordinateFile();
   DisplaySettingsSurface* dsn = brainSet->getDisplaySettingsSurface();
   BrainSetNodeAttribute* attributes = brainSet->getNodeAttributes(0);
   
   //
   // See if node uncertainty should be displayed
   //
   const SurfaceShapeFile* ssf = brainSet->getSurfaceShapeFile();
   const DisplaySettingsSurfaceShape* dsss = brainSet->getDisplaySettingsSurfaceShape();
   int shapeColumn = -1;
   if (dsss->getNodeUncertaintyEnabled()) {
      shapeColumn = dsss->getNodeUncertaintyColumn();
      if (shapeColumn >= ssf->getNumberOfColumns()) {
         shapeColumn = -1;
      }
   }
   
   if (shapeColumn >= 0) {
      glEnable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
   }
   
   //
   // Show node highlighting
   //
   glPointSize(getValidPointSize(3.0 * dsn->getNodeSize()));
   for (int i = 0; i < numCoords; i++) {
      if (attributes[i].getDisplayFlag()) {
         BrainSetNodeAttribute* bna = brainSet->getNodeAttributes(i);
         const float* xyz = cf->getCoordinate(i);
         bool drawIt = false;
         
         if ((bna->getHighlighting() == BrainSetNodeAttribute::HIGHLIGHT_NODE_LOCAL) ||
             (bna->getHighlighting() == BrainSetNodeAttribute::HIGHLIGHT_NODE_REMOTE)) {
            glColor3ub(0, 255, 0);
            drawIt = true;
            
            switch (dsn->getIdentifyNodeColor()) {
               case DisplaySettingsSurface::IDENTIFY_NODE_COLOR_BLACK:
                  glColor3ub(0, 0, 0);
                  break;
               case DisplaySettingsSurface::IDENTIFY_NODE_COLOR_BLUE:
                  glColor3ub(0, 0, 255);
                  break;
               case DisplaySettingsSurface::IDENTIFY_NODE_COLOR_GREEN:
                  glColor3ub(0, 255, 0);
                  break;
               case DisplaySettingsSurface::IDENTIFY_NODE_COLOR_RED:
                  glColor3ub(255, 0, 0);
                  break;
               case DisplaySettingsSurface::IDENTIFY_NODE_COLOR_WHITE:
                  glColor3ub(255, 255, 255);
                  break;
            }
         }
         //else if (bna->getHighlighting() == BrainSetNodeAttribute::HIGHLIGHT_NODE_REMOTE) {
         //   glColor3ub(0, 0, 255);
         //   drawIt = true;
         //}
         
         if (drawIt) {
            if (shapeColumn >= 0) {
               const float radius = fabs(ssf->getValue(i, shapeColumn)) * 2.0; // sphere radius is 0.5
               glPushMatrix();
                  glTranslatef(xyz[0], xyz[1], xyz[2]);
                  drawSphere(radius);
               glPopMatrix();
            }
            else {
               glBegin(GL_POINTS);
                  glVertex3fv(xyz);
               glEnd();
            }
         }
      }
   }
   
   glDisable(GL_LIGHTING);
   glDisable(GL_COLOR_MATERIAL);
             
   const int crossNode = brainSet->getDisplayCrossForNode();
   if ((crossNode > 0) && (crossNode < numCoords) &&
       (offScreenRenderingFlag == false)) {
      bool showCross = true;
      const BrainModelSurface* idSurface = brainSet->getDisplayCrossSurface();
      //
      // Avoid showing cross on surface that was used for the ID selection
      //
      if (idSurface == bms) {
         showCross = false;
      }
         //if (idSurface->getCoordinateFile() == cf) {
         //   showCross = false;
         //}
      
      if (showCross) {
         glDisable(GL_DEPTH_TEST);
         glPushMatrix();
            const float gap = 10.0;
            const float lineLength = 500.0;
            const float* pos = cf->getCoordinate(crossNode);
            glColor3ub(0, 0, 255);
            glBegin(GL_LINES);
               glVertex3f(pos[0] + gap,        pos[1], pos[2]);
               glVertex3f(pos[0] + lineLength, pos[1], pos[2]);
               glVertex3f(pos[0] - gap,        pos[1], pos[2]);
               glVertex3f(pos[0] - lineLength, pos[1], pos[2]);
               glVertex3f(pos[0], pos[1] + gap,        pos[2]);
               glVertex3f(pos[0], pos[1] + lineLength, pos[2]);
               glVertex3f(pos[0], pos[1] - gap,        pos[2]);
               glVertex3f(pos[0], pos[1] - lineLength, pos[2]);
               glVertex3f(pos[0], pos[1], pos[2] + gap);
               glVertex3f(pos[0], pos[1], pos[2] + lineLength);
               glVertex3f(pos[0], pos[1], pos[2] - gap);
               glVertex3f(pos[0], pos[1], pos[2] - lineLength);
            glEnd();
         glPopMatrix();
         glEnable(GL_DEPTH_TEST);
      }
   }
}

/**
 * Draw the surface normals
 */
void
BrainModelOpenGL::drawSurfaceNormals(const BrainModelSurface* bms,
                                     const CoordinateFile* cf,
                                     const int numCoords)
{
   const BrainSetNodeAttribute* attributes = brainSet->getNodeAttributes(0);
   
   const float length = 10.0;
   
   glLineWidth(getValidLineWidth(1.0));
   
   glBegin(GL_LINES);
   glColor3ub(255, 0, 0);
   for (int i = 0; i < numCoords; i++) {
      if (attributes[i].getDisplayFlag()) {
         const float* pos = cf->getCoordinate(i);
         const float* normal = bms->getNormal(i);
         float vect[3] = { pos[0] + length * normal[0],
                           pos[1] + length * normal[1],
                           pos[2] + length * normal[2] };
         glVertex3fv(pos);
         glVertex3fv(vect);
      }
   }
   glEnd();
}

/**
 * Draw a transformation cell or foci file
 */
void
BrainModelOpenGL::drawTransformationCellOrFociFile(BrainModel* bm,
                                     CellFile* cellFile, 
                                     ColorFile* colorFile,
                                     const DisplaySettingsCells* dsc,
                                     const int transformFileNumber,
                                     const int dataSelectionMask)
{
   //
   // Set color for cells/foci with missing colors
   //
   unsigned char noColorRed, noColorGreen, noColorBlue;
   PreferencesFile* pref = brainSet->getPreferencesFile();
   pref->getSurfaceForegroundColor(noColorRed, noColorGreen, noColorBlue);
   
   //
   // Check for selection mode
   //
   bool selectFlag = false;
   if (selectionMask & dataSelectionMask) {
      glPushName(dataSelectionMask);
      glPushName(transformFileNumber);
      selectFlag = true;
   }
   if ((selectFlag == false) && (selectionMask != SELECTION_MASK_OFF)) {
      return;
   }
   
   //
   // Draw cells larger when selecting on a flat surface
   //
   float cellSize = dsc->getDrawSize();
   
   const int numCells = cellFile->getNumberOfCells();
   
   if (numCells > 0) {
      const int numColors = colorFile->getNumberOfColors();
      for (int i = 0; i < numCells; i++) {
         CellData* cd = cellFile->getCell(i);
         
         ColorFile::ColorStorage::SYMBOL symbol = ColorFile::ColorStorage::SYMBOL_OPENGL_POINT;
         const int colorIndex = cd->getColorIndex();
         float pointSize = 1;
         float lineSize  = 1;
         unsigned char r = noColorRed, g = noColorGreen, b = noColorBlue, alpha = 255;
         if ((colorIndex >= 0) && (colorIndex < numColors)) {
            const ColorFile::ColorStorage* cs = colorFile->getColor(colorIndex);
            symbol = cs->getSymbol();
            cs->getRgba(r, g, b, alpha);
            lineSize = cs->getLineSize();
            pointSize = cs->getPointSize();
         }
         
         float size = pointSize * cellSize;
         //if (cd->getSize() > 0.0) {
         //   size *= cd->getSize();
         //}
         
         const float* xyz = cd->getXYZ();
         glColor4ub(r, g, b, alpha);
         float z = xyz[2];
         if (selectFlag) {
            glPushName(i);
         }

         if (dsc->getSymbolOverride() != ColorFile::ColorStorage::SYMBOL_NONE) {
            symbol = dsc->getSymbolOverride();
         }

         if (alpha < 255) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         }
         
         drawSymbol(symbol, xyz[0], xyz[1], z, size, bm);
   
         if (selectFlag) {
            glPopName();
         }
      }
      
      glDisable(GL_BLEND);
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);
   }
   if (selectFlag) {
      glPopName();
      glPopName();
   }
}

/**
 * draw a symbol.
 */
void 
BrainModelOpenGL::drawSymbol(const ColorFile::ColorStorage::SYMBOL symbol,
                             const float x,
                             const float y,
                             const float z,
                             const float sizeIn,
                             const BrainModel* bm)
{
   bool surfaceFlag = false;
   if (bm != NULL) {
     if (dynamic_cast<const BrainModelSurface*>(bm) != NULL) {
        surfaceFlag = true;
     }
   }
   
   //
   // Need lighting
   //
   glEnable(GL_LIGHTING);
   glEnable(GL_COLOR_MATERIAL);

   float size = sizeIn;
   switch (symbol) {
      case ColorFile::ColorStorage::SYMBOL_OPENGL_POINT:
         //
         // No lighting for OpenGL Points
         //
         glDisable(GL_LIGHTING);
         glDisable(GL_COLOR_MATERIAL);

         //
         // Points must be at least 1.0 for OpenGL to draw something
         //
         size = std::max(size, 1.0f);
         glPointSize(getValidPointSize(size));
         glBegin(GL_POINTS);
            glVertex3f(x, y, z);
         glEnd();
         break;
      case ColorFile::ColorStorage::SYMBOL_SPHERE:
         glPushMatrix();
             glTranslatef(x, y, z);
             drawSphere(size);
         glPopMatrix();
         break;
      case ColorFile::ColorStorage::SYMBOL_BOX:
         glPushMatrix();
             glTranslatef(x, y, z);
             glScalef(size, size, size);
             drawBox();
         glPopMatrix();
         break;
      case ColorFile::ColorStorage::SYMBOL_DIAMOND:
         glPushMatrix();
             glTranslatef(x, y, z);
             glScalef(size, size, size);
             drawDiamond();
         glPopMatrix();
         break;
      case ColorFile::ColorStorage::SYMBOL_DISK:
         glPushMatrix();
         {
             glTranslatef(x, y, z);
             //
             // Undo Rotation
             //
             if (surfaceFlag) {
                float matrix[16];
                bm->getRotationMatrixInverse(viewingWindowNumber, matrix);
                glMultMatrixf(matrix);
             }
             drawDisk(size);
         }
         glPopMatrix();
         break;
      case ColorFile::ColorStorage::SYMBOL_RING:
         glPushMatrix();
         {
             glTranslatef(x, y, z);
             //
             // Undo Rotation
             //
             if (surfaceFlag) {
                float matrix[16];
                bm->getRotationMatrixInverse(viewingWindowNumber, matrix);
                glMultMatrixf(matrix);
             }
             glScalef(size, size, size);
             drawRing();
         }
         glPopMatrix();
         break;
      case ColorFile::ColorStorage::SYMBOL_NONE:
         break;
      case ColorFile::ColorStorage::SYMBOL_SQUARE:
         glPushMatrix();
         {
             glTranslatef(x, y, z);
             //
             // Undo Rotation
             //
             if (surfaceFlag) {
                float matrix[16];
                bm->getRotationMatrixInverse(viewingWindowNumber, matrix);
                glMultMatrixf(matrix);
             }
             glScalef(size, size, size);
             drawSquare();
         }
         glPopMatrix();
         break;
   }
}
                      
/**
 * Draw cell projection and foci projections.
 */
void 
BrainModelOpenGL::drawCellAndFociProjections(BrainModelSurface* s)
{
   DisplaySettingsCells* dsc = brainSet->getDisplaySettingsCells();
   if (dsc->getDisplayCells()) {
      drawCellOrFociProjectionFile(s,
                                   brainSet->getCellProjectionFile(),
                                   brainSet->getCellColorFile(),
                                   dsc,
                                   false);
   }
      
   DisplaySettingsFoci* dsf = brainSet->getDisplaySettingsFoci();
   if (dsf->getDisplayCells()) {
      drawCellOrFociProjectionFile(s,
                                   brainSet->getFociProjectionFile(),
                                   brainSet->getFociColorFile(),
                                   dsf,
                                   true);
   }
}
      
/**
 * Draw a cell or foci projection file
 */
void
BrainModelOpenGL::drawCellOrFociProjectionFile(BrainModelSurface* bms,
                                     CellProjectionFile* cellProjectionFile, 
                                     ColorFile* colorFile,
                                     const DisplaySettingsCells* dsc,
                                     const bool fociFileFlag)
{
   const bool flatFlag = ((bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FLAT) ||
                          (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR));
   //
   // Set color for cells/foci with missing colors
   //
   unsigned char noColorRed, noColorGreen, noColorBlue;
   PreferencesFile* pref = brainSet->getPreferencesFile();
   pref->getSurfaceForegroundColor(noColorRed, noColorGreen, noColorBlue);
   
   //
   // Check for selection mode (transform cell files have negative fileNumber)
   //
   bool selectFlag = false;
   if (fociFileFlag) {
      if (selectionMask & SELECTION_MASK_FOCUS_PROJECTION) {
         glPushName(SELECTION_MASK_FOCUS_PROJECTION);
         selectFlag = true;
      }
   }
   else {
      if (selectionMask & SELECTION_MASK_CELL_PROJECTION) {
         glPushName(SELECTION_MASK_CELL_PROJECTION);
         selectFlag = true;
      }
   }   
   if ((selectFlag == false) && (selectionMask != SELECTION_MASK_OFF)) {
      return;
   }
   
   const Structure::STRUCTURE_TYPE surfaceStructure = bms->getStructure().getType();
   
   bool drawRaised = false;
   if (flatFlag) {
      if (dsc->getDisplayFlatCellsRaised()) {
         drawRaised = true;
      }
   }
   else if (fociFileFlag) {
      if (dsc->getDisplayPasteCellsOnto3D()) {
         drawRaised = true;
      }
   }
   const float opacity = dsc->getOpacity();

   //
   // Draw cells larger when selecting on a flat surface
   //
   float cellSize = dsc->getDrawSize();
   if (selectFlag && flatFlag) {
      cellSize *= 2.0;
   }
   
   //
   // Check for fiducial surface
   //
   const bool fiducialSurfaceFlag = 
      ((bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_RAW) ||
       (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL));
       
   const int numCells = cellProjectionFile->getNumberOfCellProjections();
   
   if (numCells > 0) {
      //
      // If backfaces are on with opacity, one can get foci that are
      // half dark and half light
      //
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);

      const int numColors = colorFile->getNumberOfColors();
      const CoordinateFile* cf = bms->getCoordinateFile();
      const TopologyFile* tf = bms->getTopologyFile();
      for (int i = 0; i < numCells; i++) {
         CellProjection* cp = cellProjectionFile->getCellProjection(i);
         
         if (cp->getDisplayFlag() == false) {
            continue;
         }
          
         //
         // Skip cells/foci with invalid structures
         //
         const Structure::STRUCTURE_TYPE cellStructure = cp->getCellStructure();
         if ((cellStructure == Structure::STRUCTURE_TYPE_INVALID) ||
             (surfaceStructure == Structure::STRUCTURE_TYPE_INVALID)) {
            continue;
         }
         
         //
         // Using cell structure, determine on which surfaces the
         // cells should be displayed
         //
         // From DVE 28 FEB 2008
         //    In the Foci Main page, when 'show foci on correct hemisphere only' 
         //    is selected, all cerebellar foci should be displayed only on the 
         //    cerebellum; Cerebellum_Left-Cerebral or Cerebellum_Right-Cerebral 
         //    should be displayed on both the cerebellum and the appropriate 
         //    cerebral hemisphere.  If 'show foci on correct hemisphere only' 
         //    is DE-selected, the cerebellar foci should still be shown ONLY on 
         //    the cerebellum.  
         //   
         //
         bool displayOnLeftSurface = false;
         bool displayOnRightSurface = false;
         bool displayOnCerebellumSurface = false;
         switch (cellStructure) {
            case Structure::STRUCTURE_TYPE_CORTEX_LEFT:
               displayOnLeftSurface = true;
               if (dsc->getDisplayCellsOnCorrectHemisphereOnly() == false) {
                  displayOnRightSurface = true;
               }
               break;
            case Structure::STRUCTURE_TYPE_CORTEX_LEFT_OR_CEREBELLUM:
               if (dsc->getDisplayCellsOnCorrectHemisphereOnly()) {
                  displayOnLeftSurface = true;
               }
               break;
            case Structure::STRUCTURE_TYPE_CORTEX_RIGHT:
               displayOnRightSurface = true;
               if (dsc->getDisplayCellsOnCorrectHemisphereOnly() == false) {
                  displayOnLeftSurface = true;
               }
               break;
            case Structure::STRUCTURE_TYPE_CORTEX_RIGHT_OR_CEREBELLUM:
               if (dsc->getDisplayCellsOnCorrectHemisphereOnly()) {
                  displayOnRightSurface = true;
               }
               break;
            case Structure::STRUCTURE_TYPE_CORTEX_BOTH:
               break;
            case Structure::STRUCTURE_TYPE_CEREBELLUM:
               displayOnCerebellumSurface = true;
               break;
            case Structure::STRUCTURE_TYPE_CEREBELLUM_OR_CORTEX_LEFT:
               if (dsc->getDisplayCellsOnCorrectHemisphereOnly()) {
                  displayOnCerebellumSurface = true;
               }
               break;
            case Structure::STRUCTURE_TYPE_CEREBELLUM_OR_CORTEX_RIGHT:
               if (dsc->getDisplayCellsOnCorrectHemisphereOnly()) {
                  displayOnCerebellumSurface = true;
               }
               break;
            case Structure::STRUCTURE_TYPE_CEREBRUM_CEREBELLUM:
            case Structure::STRUCTURE_TYPE_SUBCORTICAL:
            case Structure::STRUCTURE_TYPE_ALL:
            case Structure::STRUCTURE_TYPE_INVALID:
               break;
         }

         //
         // Skip over foci inappropriate for surface structure
         //
         switch (surfaceStructure) {
            case Structure::STRUCTURE_TYPE_CORTEX_LEFT:
               if (displayOnLeftSurface == false) {
                  continue;
               }
               break;
            case Structure::STRUCTURE_TYPE_CORTEX_LEFT_OR_CEREBELLUM:
               continue;
               break;
            case Structure::STRUCTURE_TYPE_CORTEX_RIGHT:
               if (displayOnRightSurface == false) {
                  continue;
               }
               break;
            case Structure::STRUCTURE_TYPE_CORTEX_RIGHT_OR_CEREBELLUM:
               continue;
               break;
            case Structure::STRUCTURE_TYPE_CORTEX_BOTH:
               if ((displayOnLeftSurface == false) &&
                   (displayOnRightSurface == false)) {
               }
               break;
            case Structure::STRUCTURE_TYPE_CEREBELLUM:
               if (displayOnCerebellumSurface == false) {
                  continue;
               }
               break;
            case Structure::STRUCTURE_TYPE_CEREBELLUM_OR_CORTEX_LEFT:
                continue;
                break;
           case Structure::STRUCTURE_TYPE_CEREBELLUM_OR_CORTEX_RIGHT:
               continue;
               break;
            case Structure::STRUCTURE_TYPE_CEREBRUM_CEREBELLUM:
            case Structure::STRUCTURE_TYPE_SUBCORTICAL:
            case Structure::STRUCTURE_TYPE_ALL:
            case Structure::STRUCTURE_TYPE_INVALID:
               continue;
               break;
         }
         
         ColorFile::ColorStorage::SYMBOL symbol = ColorFile::ColorStorage::SYMBOL_OPENGL_POINT;
         const int colorIndex = cp->getColorIndex();
         float pointSize = 1;
         float lineSize  = 1;
         unsigned char r = noColorRed, g = noColorGreen, b = noColorBlue, alpha = 255;
         if ((colorIndex >= 0) && (colorIndex < numColors)) {
            const ColorFile::ColorStorage* cs = colorFile->getColor(colorIndex);
            symbol = cs->getSymbol();
            cs->getRgba(r, g, b, alpha);
            lineSize = cs->getLineSize();
            pointSize = cs->getPointSize();
         }
         
         float size = pointSize * cellSize;
         
         //
         // Double size for highlighting
         //
         if (cp->getHighlightFlag()) {
            size *= 2.0;
         }
   
         float xyz[3];
         if (cp->getProjectedPosition(cf, tf, fiducialSurfaceFlag, flatFlag, drawRaised, xyz) == false) {
            //
            // No position available
            //
            continue;
         }
         
         //
         // Use display setting's opacity
         //
         if ((opacity >= 0.0) && (opacity < 1.0)) {
            alpha = static_cast<unsigned char>(alpha * opacity);
         }
         
         glColor4ub(r, g, b, alpha);
         float z = xyz[2];
         if (selectFlag) {
            glPushName(i);
         }

         if (dsc->getSymbolOverride() != ColorFile::ColorStorage::SYMBOL_NONE) {
            symbol = dsc->getSymbolOverride();
         }

         if (alpha < 255) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         }
      
         drawSymbol(symbol, xyz[0], xyz[1], z, size, bms);
         if (selectFlag) {
            glPopName();
         }
      }
      
      glDisable(GL_CULL_FACE);

      glDisable(GL_BLEND);
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);
   }
   if (selectFlag) {
      glPopName();
   }
}

/**
 * draw the geodesic path.
 */
void 
BrainModelOpenGL::drawGeodesicPath(const CoordinateFile* cf)
{
   DisplaySettingsGeodesicDistance* dsgd = brainSet->getDisplaySettingsGeodesicDistance();
   GeodesicDistanceFile* gdf = brainSet->getGeodesicDistanceFile();
   
   const int column = dsgd->getDisplayColumn();

   if ((column < 0) || (column >= gdf->getNumberOfColumns())) {
      return;
   }
   const int rootNode = gdf->getRootNode(column);
   if ((rootNode < 0) || (rootNode >= cf->getNumberOfCoordinates())) {
      return;
   }
   
   const float* coords = cf->getCoordinate(0);

   const ColorFile* colorFile = brainSet->getAreaColorFile();
   
   if (dsgd->getShowRootNode()) {
      unsigned char r = 255, g = 0, b = 0;
      bool match;
      colorFile->getColorByName("GEODESIC_SYMBOL", match, r, g, b);
      glColor3ub(r, g, b);
      glEnable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
      glPushMatrix();
         glTranslatef(coords[rootNode*3], coords[rootNode*3+1], coords[rootNode*3+2]);
         drawSphere(5.0);
      glPopMatrix();
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);
   }
   
   if (dsgd->getPathDisplayEnabled()) {
      int node = dsgd->getPathDisplayNodeNumber();
      const int numCoord = cf->getNumberOfCoordinates();
      if ((node >= 0) && (node < numCoord)) {
         const float* coords = cf->getCoordinate(0);
         glLineWidth(getValidLineWidth(dsgd->getPathLineWidth()));
         unsigned char r = 0, g = 255, b = 255;
         bool match;
         colorFile->getColorByName("GEODESIC_PATH", match, r, g, b);
         glColor3ub(r, g, b);
         glBegin(GL_LINE_STRIP);
            bool done = false;
            while (done == false) {
               const int parent = gdf->getNodeParent(node, column);
               if ((parent == node) || (node == rootNode)) {
                  done = true;
               }
               else if (parent == -1) {
                  done = true;
               }
               glVertex3fv(&coords[parent*3]);
               node = parent;
            }
         glEnd();
      }
   }
}

/**
 * Draw a cuts file.
 */
void
BrainModelOpenGL::drawCuts()
{
   //
   // Only show cuts in main window
   //
   if (mainWindowFlag == false) {
      return;
   }
   
   DisplaySettingsCuts* dsc = brainSet->getDisplaySettingsCuts();
   if (dsc->getDisplayCuts() == false) {
      return;
   }
   
   BorderFile* bf = brainSet->getCutsFile();
   
   //
   // Check for selection mode
   //
   bool selectFlag = false;
   if (selectionMask & SELECTION_MASK_CUT) {
      glPushName(SELECTION_MASK_CUT);
      selectFlag = true;
   }
   if ((selectFlag == false) && (selectionMask != SELECTION_MASK_OFF)) {
      return;
   }
   
   const int numBorders = bf->getNumberOfBorders();
   
   if (numBorders > 0) {
      glColor3f(0.0, 0.0, 1.0);
      for (int i = 0; i < numBorders; i++) {
         Border* b = bf->getBorder(i);
         const int numLinks = b->getNumberOfLinks();
         
         if ((numLinks > 0) && b->getDisplayFlag()) {
            const float pointSize = 2;
            if (selectFlag) {
               glPushName(i);
                  glPointSize(getValidPointSize(pointSize));
                  for (int j = 0; j < numLinks; j++) {
                     glPushName(j);
                        glBegin(GL_POINTS);
                           float xyz[3];
                           b->getLinkXYZ(j, xyz);
                           xyz[2] = 1.0;
                           glVertex3fv(xyz);
                        glEnd();
                     glPopName();
                  }
               glPopName();
            }
            else {
               glPointSize(getValidPointSize(pointSize));
               glBegin(GL_POINTS);
                  float xyz[3];
                  for (int j = 0; j < numLinks; j++) {
                     b->getLinkXYZ(j, xyz);
                     xyz[2] = 1.0;
                     glVertex3fv(xyz);
                  }
               glEnd();
            }
         }
      }
   }
   
   if (selectFlag) {
      glPopName();
   }
}


/**
 * Draw the skull surface and electrode trajectory
 */
void
BrainModelOpenGL::drawElectrodeTrajectory(BrainModelSurface* s)
{
	double matrix[16];
	double T[3][3];
	const int nsides = 20;
	double vtemp[3];
	ANNidxArray idx;
	int nidx;
	double xc[3], yc[3], zc[3], zrest;
	double dCenter[3];
	double dTarget[3];
	double dEntry[3];
	double dID, dOD, dH;
	double gridx, gridy;
	double dPlanePt[3], dPlaneNormal[3], dPlaneNormalRot[3];
	TrajectoryFile *ptf = s->getBrainSet()->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();

	if (!ep) return;

	const int brainModelIndex = brainSet->getBrainModelIndex(s);
	if (brainModelIndex < 0)
	{
		std::cout << "PROGRAM ERROR: invalid brain model index at " << __LINE__
				<< " in " << __FILE__ << std::endl;
		return;
	}

  	// Only draw skull surface if selection mask is off
  	if (selectionMask == SELECTION_MASK_OFF)
  	{
		if (s->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL)
		{
			double xyz[3];
			int i;

			if (ep->getDisplaySkull())
			{
				const std::multimap<int, int>& mapFaces = ptf->getSkullFaceMap();
				std::set<int> uniqueFaces;
				std::set<int> uniqueFacesAnnulus;

				// Find faces present in the nearby list of vertices
				ep->getSkullNearbyIdx(&idx, &nidx);
				if (nidx > 0)
				{
					for (i=0; i<nidx; i++)
					{
						std::pair< std::map<int, int>::const_iterator, std::map<int, int>::const_iterator > p;
						p = mapFaces.equal_range(idx[i]);
						for ( std::map<int, int>::const_iterator it = p.first; it!= p.second; it++)
						{
							uniqueFaces.insert(it->second);
						}
					}
				}

				// Now get faces present in the annulus list of indices, but which are NOT in the nearby list.
				ep->getSkullAnnulusIdx(&idx, &nidx);
				if (nidx > 0)
				{
					for (i=0; i<nidx; i++)
					{
						std::pair< std::map<int, int>::const_iterator, std::map<int, int>::const_iterator > p;
						p = mapFaces.equal_range(idx[i]);
						for ( std::map<int, int>::const_iterator it = p.first; it!= p.second; it++)
						{
							// Only add the face to the annulus list if its NOT in the other list
							if (uniqueFaces.find(it->second) == uniqueFaces.end())
							{
								uniqueFacesAnnulus.insert(it->second);
							}
						}
					}
				}

				// Now draw

				float fv1[3], fv2[3], fv3[3];
				float fn1[3], fn2[3], fn3[3];

				glEnable(GL_LIGHTING);
				glEnable(GL_COLOR_MATERIAL);
				glColor3ub(ep->getSkullSurfaceColor().red(), ep->getSkullSurfaceColor().green(), ep->getSkullSurfaceColor().blue());
				glBegin(GL_TRIANGLES);
				for (std::set<int>::iterator it=uniqueFaces.begin(); it != uniqueFaces.end(); it++)
				{
					ptf->getFace(*it, fv1, fv2, fv3, fn1, fn2, fn3);
					glVertex3fv(fv1);
					glNormal3fv(fn1);
					glVertex3fv(fv2);
					glNormal3fv(fn2);
					glVertex3fv(fv3);
					glNormal3fv(fn3);
				}
				glEnd();

				glColor3ub(ep->getSkullSurfaceAnnulusColor().red(), ep->getSkullSurfaceAnnulusColor().green(), ep->getSkullSurfaceAnnulusColor().blue());
				glBegin(GL_TRIANGLES);
				for (std::set<int>::iterator it=uniqueFacesAnnulus.begin(); it != uniqueFacesAnnulus.end(); it++)
				{
					ptf->getFace(*it, fv1, fv2, fv3, fn1, fn2, fn3);
					glVertex3fv(fv1);
					glNormal3fv(fn1);
					glVertex3fv(fv2);
					glNormal3fv(fn2);
					glVertex3fv(fv3);
					glNormal3fv(fn3);
				}
				glEnd();

				glDisable(GL_LIGHTING);
				glDisable(GL_COLOR_MATERIAL);
			}

			ep->getCylinderCoords(xc, yc, zc, &zrest);
			ep->getCylinderCenter(dCenter);
			ep->getEntry(dEntry);
			ep->getTarget(dTarget);
			ep->getGridEntry(&gridx, &gridy);
			ep->getCylinderFitValues(dPlanePt, dPlaneNormal);		// plane normal in world coord
			dID = ep->getCylinderID();
			dOD = ep->getCylinderOD();
			dH = ep->getCylinderH();

			// Set up a transformation matrix and rotate cylinder fit plane normal
			for (i=0; i<3; i++)
			{
				T[0][i] = xc[i];
				T[1][i] = yc[i];
				T[2][i] = zc[i];
			}
			vtkMath::Multiply3x3(T, dPlaneNormal, dPlaneNormalRot);
			// Set up the modelview matrix.
			// Cylinder coords become the rotation matrix.
			// Cylinder center becomes the translation center.

			matrix[0] = xc[0]; matrix[1] = xc[1]; matrix[2] = xc[2]; matrix[3] = 0;
			matrix[4] = yc[0]; matrix[5] = yc[1]; matrix[6] = yc[2]; matrix[7] = 0;
			matrix[8] = zc[0]; matrix[9] = zc[1]; matrix[10] = zc[2]; matrix[11] = 0;
			matrix[12] = dCenter[0]; matrix[13] = dCenter[1]; matrix[14] = dCenter[2]; matrix[15] = 1;

			// display cylinder
			if (ep->getDisplayCylinder())
			{
				// Generate points for the top and bottom of cylinder
				double dTopInner[nsides*3], dTopInnerNorms[nsides*3];
				double dTopOuter[nsides*3], dTopOuterNorms[nsides*3];
				double dBotInner[nsides*3], dBotInnerNorms[nsides*3];
				double dBotOuter[nsides*3], dBotOuterNorms[nsides*3];
				double dRestInner[nsides*3];
				double dRestOuter[nsides*3];
//djs 12-22-08	liftoff	problem		float center[3] = { 0, 0, zrest+fH };
				double center[3] = { 0, 0, dH };
				double p0[3];
				generateCircle(center, dID/2, nsides, dTopInner, dTopInnerNorms, false);
				generateCircle(center, dOD/2, nsides, dTopOuter, dTopOuterNorms, true);
				//djs 12-22-08	liftoff	problem				center[2] = zrest;
				center[2] = 0;
				generateCircle(center, dID/2, nsides, dBotInner, dBotInnerNorms, false);
				generateCircle(center, dOD/2, nsides, dBotOuter, dBotOuterNorms, true);

				// Now get intersections with the fit plane
				for (i=0; i<3; i++) p0[i] = dPlanePt[i] - dCenter[i];
				for (i=0; i<nsides; i++)
				{
					intersectLinePlane(p0, dPlaneNormalRot, dTopInner+i*3, dBotInner+i*3, dRestInner+i*3);
					intersectLinePlane(p0, dPlaneNormalRot, dTopOuter+i*3, dBotOuter+i*3, dRestOuter+i*3);
				}

				// Now start drawing
				glEnable(GL_LIGHTING);
				glEnable(GL_COLOR_MATERIAL);
				glPushMatrix();
				glMultMatrixd(matrix);

				// ...top
				glColor3ub(ep->getCylinderEndsColor().red(), ep->getCylinderEndsColor().green(), ep->getCylinderEndsColor().blue());
				glBegin(GL_QUAD_STRIP);
				for (i=0; i<nsides; i++)
				{
					glVertex3dv(dTopInner+i*3);
					glNormal3dv(dTopInnerNorms+i*3);
					glVertex3dv(dTopOuter+i*3);
					glNormal3dv(dTopOuterNorms+i*3);
				}
				glVertex3dv(dTopInner);
				glNormal3dv(dTopInnerNorms);
				glVertex3dv(dTopOuter);
				glNormal3dv(dTopOuterNorms);
				glEnd();

				// ...bottom - reverse order of points
				glColor3ub(ep->getCylinderEndsColor().red(), ep->getCylinderEndsColor().green(), ep->getCylinderEndsColor().blue());
				glBegin(GL_QUAD_STRIP);
				for (i=0; i<nsides; i++)
				{
					glVertex3dv(dRestOuter+i*3);
					glNormal3dv(dBotOuterNorms+i*3);
					glVertex3dv(dRestInner+i*3);
					glNormal3dv(dBotInnerNorms+i*3);
				}
				glVertex3dv(dRestOuter);
				glNormal3dv(dBotOuterNorms);
				glVertex3dv(dRestInner);
				glNormal3dv(dBotInnerNorms);
				glEnd();

				// ...outside
				glColor3ub(ep->getCylinderSidesColor().red(), ep->getCylinderSidesColor().green(), ep->getCylinderSidesColor().blue());
				glBegin(GL_QUAD_STRIP);
				for (i=0; i<nsides; i++)
				{
					glVertex3dv(dTopOuter+i*3);
					glNormal3dv(dTopOuterNorms+i*3);
					glVertex3dv(dRestOuter+i*3);
					glNormal3dv(dBotOuterNorms+i*3);
				}
				glVertex3dv(dTopOuter);
				glNormal3dv(dTopOuterNorms);
				glVertex3dv(dRestOuter);
				glNormal3dv(dBotOuterNorms);
				glEnd();

				// ...inside
				glColor3ub(ep->getCylinderSidesColor().red(), ep->getCylinderSidesColor().green(), ep->getCylinderSidesColor().blue());
				glBegin(GL_QUAD_STRIP);
				for (i=0; i<nsides; i++)
				{
					glVertex3dv(dTopInner+i*3);
					glNormal3dv(dTopInnerNorms+i*3);
					glVertex3dv(dRestInner+i*3);
					glNormal3dv(dBotInnerNorms+i*3);
				}
				glVertex3dv(dTopInner);
				glNormal3dv(dTopInnerNorms);
				glVertex3dv(dRestInner);
				glNormal3dv(dBotInnerNorms);
				glEnd();

				// symmetry axis
				glBegin(GL_LINES);
				glColor3ub(0, 0, 0);
				glVertex3f(0, 0, 0);
				glVertex3f(0, 0, 1.5*(dH + zrest));
				glEnd();

				// Disable lighting here so the crosshairs and symmetry axis don't get the
				// lighting funkiness.
				glDisable(GL_LIGHTING);
				glDisable(GL_COLOR_MATERIAL);

				// crosshairs that represent grid orientation. Heavier line is the "north" direction
				// of the grid.
				// 12-17-08 Negative sign on rotation angle is because our definition of a positive rotation of the
				// grid is opposite that of OpenGL.
				// 12-22-08 Draw North line as black, all others as white.
				glRotatef(-ep->getGridRotation(), 0, 0, 1);
				glLineWidth(1.0);
				glColor3ub(0, 0, 0);
				glBegin(GL_LINES);
				glVertex3d(0, 0, dH + 0.1);
				glVertex3d(0, 0.5*dOD, dH + 0.1);
				glEnd();
				glLineWidth(1.0);
				glColor3ub(255, 255, 255);
				glBegin(GL_LINES);
				glVertex3d(0, 0, dH + 0.1);
				glVertex3d(0, -0.5*dOD, dH + 0.1);
				glVertex3d(0, 0, dH + 0.1);
				glVertex3d(0.5*dOD, 0, dH + 0.1);
				glVertex3d(0, 0, dH + 0.1);
				glVertex3d(-0.5*dOD, 0, dH + 0.1);
				glEnd();

				glPopMatrix();
			}

			if (ep->getDisplayTrajectory())
			{
				double b[3];
				double d;
				double gridx, gridy;
				double entryDepth;
				ep->getGridEntry(&gridx, &gridy);
				for (i=0; i<3; i++) vtemp[i] = dCenter[i] - dTarget[i];
				entryDepth = vtkMath::Dot(vtemp, zc); // actually this is the opposite sign than we'll need.
				d = vtkMath::Normalize(vtemp);
				for (i=0; i<3; i++) b[i] = dTarget[i] + (d+zrest+dH)*vtemp[i];
				glColor3ub(ep->getPathLineColor().red(), ep->getPathLineColor().green(), ep->getPathLineColor().blue());

				// push the cylinder coords matrix, along with a rotation matrix for the grid rotation.
				glPushMatrix();
				glMultMatrixd(matrix);
				glRotated(-ep->getGridRotation(), 0, 0, 1);
				glBegin(GL_LINES);
				glVertex3d(gridx, gridy, -entryDepth);
				glVertex3d(gridx, gridy, d+zrest+dH);
				glEnd();
				glPopMatrix();
			}


			if (ep->getDisplayBrain())
			{
				// draw brain surface
				glColor3ub(ptf->getBrainSurfaceColor().red(), ptf->getBrainSurfaceColor().green(), ptf->getBrainSurfaceColor().blue());
				glPointSize(1.0);
				glBegin(GL_POINTS);
				for (i=0; i<ptf->getNumBrainVertices(); i++)
				{
					ptf->getBrainVertex(i, xyz);
					glVertex3dv(xyz);
				}
				glEnd();
			}
		}
		else if (s->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FLAT)
		{
			const std::map<double, int>& mapPathNodes = ep->getPathNodes();
			const CoordinateFile* cf = s->getCoordinateFile();
			double xyz[3];

			if (ep->getDisplayTrajectory())
			{
				// Draw nodes within tolerance of path. mapPathNodes is a map of nodes indexed by depth as measured from entry point.
				// The actual depth only matters if the depth tool is in use -- in that case compute the actual depth and find all nodes
				// within the depth tool tolerance of that depth (depth tool tolerance is different than the path radius tolerance).
				// If the depth tool is not in use, use all nodes. The depthMin/depthMax should be set to ridiculous values so that they will
				// catch all nodes (when depth tool not in use).
				if (mapPathNodes.size() > 0)
				{
					double ddepthMin = -999999.99;
					double ddepthMax = 999999.99;

					// check if the depth tool is on. If its checked on, then calculate min/max
					// depth values.
					if (ep->getUseDepthTool())
					{
						double ddepth = (float)ep->getDepthToolDepthPct()/100.0f * ep->getMaxPenetrationDepth();
						double ddepthTol = ep->getDepthToolTolerance();
						ddepthMin = ddepth - ddepthTol;
						ddepthMax = ddepth + ddepthTol;
					}
					glColor3ub(ep->getFlatMapNodesColor().red(), ep->getFlatMapNodesColor().green(), ep->getFlatMapNodesColor().blue());
					glPointSize(2.0);
					glBegin(GL_POINTS);
					for (std::map<double, int>::const_iterator it=mapPathNodes.begin(); it!=mapPathNodes.end(); it++)
					{
						int iNode = (*it).second;
						double ddist = (*it).first;

						if (ddist > ddepthMin && ddist < ddepthMax)
						{
							// Get coord of this node from flat file.....???????
							cf->getCoordinate(iNode, xyz);
							glVertex3dv(xyz);
						}
					}
					glEnd();
				}
			}
		}
  	}
	return;
}



/**
 * Draw electrode trajectory and selected nodes on volume
 */
void BrainModelOpenGL::drawVolumeElectrodeTrajectory(BrainModelVolume* bmv, const VolumeFile::VOLUME_AXIS volumeSliceAxis, const float axisCoord)
{
	int iaxis;			// index of coordinate which is perp to screen
	int iaxisX, iaxisY;	// indices of axes in plane of screen - used for drawing voxels
	float dTrajectoryVolumeTolerance = 1.0;
	float spacing[3];
	float origin[3];
	float voxelX, voxelY;
	float voxelSizeX = 1.0;
	float voxelSizeY = 1.0;
	float voxelOriginX = 1.0;
	float voxelOriginY = 1.0;
	VolumeFile *vf;
	TrajectoryFile *ptf = brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;

   //
   // Get the origin and spacing
   //
   vf = bmv->getUnderlayVolumeFile();
   if (NULL == bmv) return;
   vf->getOrigin(origin);
   vf->getSpacing(spacing);

   switch(volumeSliceAxis)
   {
      case VolumeFile::VOLUME_AXIS_X:  // PARASAGITTAL
			iaxis = 0;
			iaxisX = 1;
			iaxisY = 2;
         voxelSizeX = spacing[1];
         voxelSizeY = spacing[2];
         voxelOriginX = origin[1];
         voxelOriginY = origin[2];
         break;
      case VolumeFile::VOLUME_AXIS_Y:  // CORONAL
			iaxis = 1;
			iaxisX = 0;
			iaxisY = 2;
         voxelSizeX = spacing[0];
         voxelSizeY = spacing[2];
         voxelOriginX = origin[0];
         voxelOriginY = origin[2];
         break;
      case VolumeFile::VOLUME_AXIS_Z:  // HORIZONTAL
			iaxis = 2;
			iaxisX = 0;
			iaxisY = 1;
         voxelSizeX = spacing[0];
         voxelSizeY = spacing[1];
         voxelOriginX = origin[0];
         voxelOriginY = origin[1];
			break;
		default:
			std::cout << "BrainModelOpenGL::drawVolumeElectrodeTrajectory(): UNKNOWN AXIS" << std::endl;
			break;
   }


	const std::map<double, int>& mapPathNodes = ep->getPathNodes();
	const CoordinateFile* cf = brainSet->getActiveFiducialSurface()->getCoordinateFile();
	float xyz[3];
	if (mapPathNodes.size() > 0)
	{
		// TODO: Get color and size from dialog. Also LINE_LOOP or POLYGON or POINTS
		glColor3ub(ep->getVolumeNodesColor().red(), ep->getVolumeNodesColor().green(), ep->getVolumeNodesColor().blue());
		for (std::map<double, int>::const_iterator it=mapPathNodes.begin(); it!=mapPathNodes.end(); it++)
		{
			int iNode = (*it).second;
			double dDepth = it->first;

			// If depth tool in use, check depth against penetration depth. If outside
			// tolerance skip this node
			if (ep->getUseDepthTool())
			{
				if (abs(dDepth - ep->getPenetrationDepth()) > ep->getDepthToolTolerance())
				{
					continue;
				}
			}

			// Get coord of this node.
			cf->getCoordinate(iNode, xyz);
			if (fabs(xyz[iaxis]-axisCoord) < dTrajectoryVolumeTolerance)
			{
				// these are the voxel numbers (0...) in the drawing-x and -y directions
				//
				int nvoxelX = 0;
				int nvoxelY = 0;

				nvoxelX = (int)((xyz[iaxisX]-voxelOriginX)/voxelSizeX);
				nvoxelY = (int)((xyz[iaxisY]-voxelOriginY)/voxelSizeY);

				voxelX = nvoxelX * voxelSizeX + voxelOriginX;
				voxelY = nvoxelY * voxelSizeY + voxelOriginY;

				glBegin(GL_POLYGON);
				glVertex2f(voxelX, voxelY);
				glVertex2f(voxelX + voxelSizeX, voxelY);
				glVertex2f(voxelX + voxelSizeX, voxelY + voxelSizeY);
				glVertex2f(voxelX, voxelY + voxelSizeY);
				glEnd();
			}
		}
	}
}

/**
 * Draw the borders.
 */
void
BrainModelOpenGL::drawBorders(BrainModelSurface* s)
{
   DisplaySettingsBorders* dsb = brainSet->getDisplaySettingsBorders();
   if (dsb->getDisplayBorders() == false) {
      return;
   }
   
   bool flatFlag = false;
   switch(s->getSurfaceType()) {
      case BrainModelSurface::SURFACE_TYPE_RAW:
         break;
      case BrainModelSurface::SURFACE_TYPE_FIDUCIAL:
         break;
      case BrainModelSurface::SURFACE_TYPE_INFLATED:
         break;
      case BrainModelSurface::SURFACE_TYPE_VERY_INFLATED:
         break;
      case BrainModelSurface::SURFACE_TYPE_SPHERICAL:
         break;
      case BrainModelSurface::SURFACE_TYPE_ELLIPSOIDAL:
         break;
      case BrainModelSurface::SURFACE_TYPE_COMPRESSED_MEDIAL_WALL:
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT:
         flatFlag = true;
         break;
      case BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR:
         flatFlag = true;
         break;
      case BrainModelSurface::SURFACE_TYPE_HULL:
         break;
      case BrainModelSurface::SURFACE_TYPE_UNKNOWN:
         break;
      case BrainModelSurface::SURFACE_TYPE_UNSPECIFIED:
         break;
   }
   
   const int brainModelIndex = brainSet->getBrainModelIndex(s);
   if (brainModelIndex < 0) {
      std::cout << "PROGRAM ERROR: invalid brain model index at " << __LINE__
                << " in " << __FILE__ << std::endl;
      return;
   }
   
   //
   // Set color for borders with missing colors
   //
   unsigned char noColorRed, noColorGreen, noColorBlue;
   PreferencesFile* pref = brainSet->getPreferencesFile();
   pref->getSurfaceForegroundColor(noColorRed, noColorGreen, noColorBlue);
   
   //
   // Check for selection mode
   //
   bool selectFlag = false;
   if (selectionMask & SELECTION_MASK_BORDER) {
      glPushName(SELECTION_MASK_BORDER);
      glPushName(brainModelIndex);
      selectFlag = true;
   }
   if ((selectFlag == false) && (selectionMask != SELECTION_MASK_OFF)) {
      return;
   }
   
   bool drawRaised = false;
   if (flatFlag && dsb->getDisplayFlatBordersRaised()) {
      drawRaised = true;
   }
   
   ColorFile* areaColorFile = brainSet->getAreaColorFile();
   const int numAreaColors = areaColorFile->getNumberOfColors();
   ColorFile* borderColorFile = brainSet->getBorderColorFile();
   const int numBorderColors = borderColorFile->getNumberOfColors();
   
   //
   // Draw borders larger when selecting on a flat surface
   //
   float drawSize = dsb->getDrawSize();
   if (selectFlag && flatFlag) {
      drawSize *= 2.0;
   }
   const float stretchFactorSquared = dsb->getDrawAsStretchedLinesStretchFactor()
                                  * dsb->getDrawAsStretchedLinesStretchFactor();
   BrainModelBorderSet* bmbs = brainSet->getBorderSet();
   const int numBorders = bmbs->getNumberOfBorders();
   const bool overrideWithAreaColorsFlag = dsb->getOverrideBorderColorsWithAreaColors();
   const float opacity = dsb->getOpacity();
   unsigned char alpha = 255;
   if ((opacity >= 0.0) && (opacity < 1.0)) {
      alpha = static_cast<unsigned char>(opacity * 255.0);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   }
   
   for (int i = 0; i < numBorders; i++) {
      const BrainModelBorder* b = bmbs->getBorder(i);
      if (b->getValidForBrainModel(brainModelIndex) &&
          b->getDisplayFlag()) {
         const int numLinks = b->getNumberOfBorderLinks();
         
         if ((numLinks > 0) && b->getDisplayFlag()) {
            const int borderColorIndex = b->getBorderColorFileIndex();
            const int areaColorIndex = b->getAreaColorFileIndex();
            float pointSize = 1;
            float lineSize  = 1;
            unsigned char red = 0, green = 0, blue = 0;
            if (overrideWithAreaColorsFlag &&
                ((areaColorIndex >= 0) && (areaColorIndex < numAreaColors))) {
               areaColorFile->getColorByIndex(areaColorIndex, red, green, blue);
               if ((borderColorIndex >= 0) && (borderColorIndex < numBorderColors)) {
                  borderColorFile->getPointLineSizeByIndex(borderColorIndex, pointSize, lineSize);
               }
            }
            else if ((borderColorIndex >= 0) && (borderColorIndex < numBorderColors)) {
               borderColorFile->getColorByIndex(borderColorIndex, red, green, blue);
               borderColorFile->getPointLineSizeByIndex(borderColorIndex, pointSize, lineSize);
            }
            else {
               red   = noColorRed;
               green = noColorGreen;
               blue  = noColorBlue;
            }
            
            //
            // Double size if highlighted
            //
            if (b->getHighlightFlag()) {
               pointSize *= 2.0;
               lineSize  *= 2.0;
            }
            
            const ColorFile::ColorStorage::SYMBOL symbol = dsb->getSymbolType();
            
            if (selectFlag) {
               glPushName(i);
                  for (int j = 0; j < numLinks; j++) {
                     const BrainModelBorderLink* link = b->getBorderLink(j);
                     glPushName(j);
                        float xyz[3];
                        link->getLinkPosition(brainModelIndex, xyz);
                        if (drawRaised) {
                           xyz[2] = 1.0;
                        }
                        drawSymbol(symbol, xyz[0], xyz[1], xyz[2], pointSize*drawSize, s);
                     glPopName();
                  }
               glPopName();
               glDisable(GL_LIGHTING);      // get turned on when drawing symbols
               glDisable(GL_COLOR_MATERIAL);
            }
            else {
               if ((dsb->getDrawMode() == DisplaySettingsBorders::BORDER_DRAW_AS_SYMBOLS) ||
                   (dsb->getDrawMode() == DisplaySettingsBorders::BORDER_DRAW_AS_SYMBOLS_AND_LINES)) {
                  int startLink = 0;
                  if (dsb->getDisplayFirstLinkRed()) {
                     glColor4ub(255, 0, 0, alpha);
                     float xyz[3];
                     const BrainModelBorderLink* link = b->getBorderLink(0);
                     link->getLinkPosition(brainModelIndex, xyz);
                     if (drawRaised) {
                        xyz[2] = 1.0;
                     }
                     drawSymbol(symbol, xyz[0], xyz[1], xyz[2], pointSize*drawSize, s);
                     startLink = 1;
                  }
                  glColor4ub(red, green, blue, alpha);
                  float xyz[3];
                  for (int j = startLink; j < numLinks; j++) {
                     const BrainModelBorderLink* link = b->getBorderLink(j);
                     link->getLinkPosition(brainModelIndex, xyz);
                     if (drawRaised) {
                        xyz[2] = 1.0;
                     }
                     drawSymbol(symbol, xyz[0], xyz[1], xyz[2], pointSize*drawSize, s);
                  }
                  glDisable(GL_LIGHTING);      // get turned on when drawing symbols
                  glDisable(GL_COLOR_MATERIAL);
               }
               if ((dsb->getDrawMode() == DisplaySettingsBorders::BORDER_DRAW_AS_VARIABILITY) ||
                   (dsb->getDrawMode() == DisplaySettingsBorders::BORDER_DRAW_AS_VARIABILITY_AND_LINES)) {
                  glEnable(GL_LIGHTING);
                  glEnable(GL_COLOR_MATERIAL);
                  for (int j = 0; j < numLinks; j++) {
                     if ((j == 0) &&
                         dsb->getDisplayFirstLinkRed()) {
                        glColor4ub(255, 0, 0, alpha);
                     }
                     else {
                        glColor4ub(red, green, blue, alpha);
                     }
                     const BrainModelBorderLink* link = b->getBorderLink(j);
                     const float radius = link->getRadius();
                     if (radius > 0.0) {
                        glPushMatrix();
                           float xyz[3];
                           link->getLinkPosition(brainModelIndex, xyz);
                           glTranslatef(xyz[0], xyz[1], xyz[2]);
                           drawSphere(radius);
                        glPopMatrix();
                     }
                  }
                  glEnable(GL_COLOR_MATERIAL);
                  glDisable(GL_LIGHTING);
               }
               if ((dsb->getDrawMode() == DisplaySettingsBorders::BORDER_DRAW_AS_LINES) ||
                   (dsb->getDrawMode() == DisplaySettingsBorders::BORDER_DRAW_AS_UNSTRETCHED_LINES) || 
                   (dsb->getDrawMode() == DisplaySettingsBorders::BORDER_DRAW_AS_SYMBOLS_AND_LINES) ||
                   (dsb->getDrawMode() == DisplaySettingsBorders::BORDER_DRAW_AS_VARIABILITY_AND_LINES)) {
                  //
                  // See if drawing as unstretched lines
                  //
                  bool unstretched = false;
                  if (flatFlag &&
                     (dsb->getDrawMode() == 
                     DisplaySettingsBorders::BORDER_DRAW_AS_UNSTRETCHED_LINES)) {
                     unstretched = true;
                  }
                  glLineWidth(getValidLineWidth(lineSize * drawSize));
                  glBegin(GL_LINES);
                     glColor4ub(red, green, blue, alpha);
                     float pos1[3];
                     float pos2[3];
                     for (int j = 1; j < numLinks; j++) {
                        const BrainModelBorderLink* link1 = b->getBorderLink(j - 1);
                        const BrainModelBorderLink* link2 = b->getBorderLink(j);
                        link1->getLinkPosition(brainModelIndex, pos1);
                        link2->getLinkPosition(brainModelIndex, pos2);
                        if (unstretched) {
                           if (vtkMath::Distance2BetweenPoints(pos1, pos2) > 
                               stretchFactorSquared) {
                              continue;
                           }
                        }
                        if (drawRaised) {
                           pos1[2] = 1.0;
                           pos2[2] = 1.0;
                        }
                        glVertex3fv(pos1);
                        glVertex3fv(pos2);
                     }
                  glEnd();
                  glLineWidth(getValidLineWidth(1.0));
               }
            }
            
            if (dsb->getDisplayUncertaintyVectors() && (selectFlag == false)) {
               float xyz[3];
               const float normalLength = b->getArealUncertainty();
               
               glBegin(GL_LINES);
                  for (int j = 0; j < numLinks; j++) {
                     const BrainModelBorderLink* link = b->getBorderLink(j);
                     link->getLinkPosition(brainModelIndex, xyz);
                     if (drawRaised) {
                        xyz[2] = 1.0;
                     }
                     glVertex3fv(xyz);
                     
                     const float* normal = link->getFlatNormal();
                     xyz[0] += normal[0] * normalLength;
                     xyz[1] += normal[1] * normalLength;
                     xyz[2] = 0.0;
                     glVertex3fv(xyz);
                  }
               glEnd();
            }
         }
      }
   }
   
   glDisable(GL_BLEND);
   
   if (selectFlag) {
      glPopName();
      glPopName();
   }
}

/**
 * Draw the Surface Shape colorbar.
 */
void
BrainModelOpenGL::drawShapePalette(const int modelNumber)
{
   //
   // Check for selection mode
   //
   bool selectFlag = false;
   if (selectionMask & SELECTION_MASK_PALETTE_SHAPE) {
      selectFlag = true;
   }
   else if (selectionMask != SELECTION_MASK_OFF) {
      return;
   }
   
   DisplaySettingsSurfaceShape* dsss = brainSet->getDisplaySettingsSurfaceShape();
   if (dsss->getDisplayColorBar() == false) {
      return;
   }
   SurfaceShapeFile* ssf = brainSet->getSurfaceShapeFile();
   if (ssf->getNumberOfColumns() <= 0) {
      return;
   }
   
   BrainModelSurfaceNodeColoring* bsnc = brainSet->getNodeColoring();
   if (brainSet->isASurfaceOverlay(modelNumber,
             BrainModelSurfaceOverlay::OVERLAY_SURFACE_SHAPE) == false) {
      return;
   }

   glPushMatrix();
   glDisable(GL_DEPTH_TEST);
   
   glMatrixMode(GL_PROJECTION);
   
   // save the projection matrix for the surface
   GLfloat savedProjectionMatrix[16];
   glGetFloatv(GL_PROJECTION_MATRIX, savedProjectionMatrix);
   
   // save the viewport
   GLint savedViewport[4];
   glGetIntegerv(GL_VIEWPORT, savedViewport);
   
   //
   // colorbar is in lower right corner of graphics window
   //
   const GLint colorbarViewportWidth =
                    static_cast<int>(viewport[2] * 0.25);
   const GLint colorbarViewportHeight = 50;

   const GLint colorbarViewportLeft = 10;
                                    
   const int yOffset = 15;   // prevent colorbar under resize tab
   glViewport(colorbarViewportLeft, yOffset, 
              colorbarViewportWidth, colorbarViewportHeight + yOffset);

   const GLdouble halfWidth = static_cast<GLdouble>(colorbarViewportWidth / 2);
   const GLdouble halfHeight = static_cast<GLdouble>(colorbarViewportHeight / 2);

   glLoadIdentity();
   
   if (selectFlag) {
      GLint vp[4];
      glGetIntegerv(GL_VIEWPORT, vp);
      gluPickMatrix((GLdouble)selectionX, (GLdouble)(selectionY),  //(savedViewport[3] - selectionY),
                  5.0, 5.0, vp);
   }
      
   const GLdouble margin = 1.1;
   const GLdouble orthoWidth = halfWidth * margin;
   const GLdouble orthoHeight = halfHeight * margin;
   glOrtho(-orthoWidth,  orthoWidth, 
           -orthoHeight, orthoHeight, 
           -1.0, 1.0);
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity();
   
   //
   // set background color
   //
   unsigned char rb, gb, bb;
   PreferencesFile* pref = brainSet->getPreferencesFile();
   pref->getSurfaceBackgroundColor(rb, gb, bb);
   glColor3ub(rb, gb, bb);
   glRectf(-orthoWidth, -orthoHeight, orthoWidth, orthoHeight);

   Palette* palette = NULL;
   bool interpolatePaletteColor = false;
   
   switch (dsss->getColorMap()) {   
      case DisplaySettingsSurfaceShape::SURFACE_SHAPE_COLOR_MAP_GRAY:
         glBegin(GL_POLYGON);
            glColor3ub(0, 0, 0);
            glVertex3f(-halfWidth, -halfHeight, 0.0);
            glVertex3f(-halfWidth, 0.0, 0.0);
            glColor3ub(255, 255, 255);
            glVertex3f(halfWidth, 0.0, 0.0);
            glVertex3f(halfWidth, -halfHeight, 0.0);
         glEnd();
         break;
      case DisplaySettingsSurfaceShape::SURFACE_SHAPE_COLOR_MAP_ORANGE_YELLOW:
         {
            unsigned char lut[256][3];
            bsnc->getShapeLookupTable(lut);
            glPushMatrix();
               glTranslatef(-halfWidth, 0.0, 0.0);
               for (int i = 0; i < 255; i++) {
                  glBegin(GL_POLYGON);
                     glColor3ubv(lut[i]);
                     glVertex3f(((i * halfWidth * 2) / 256.0), -halfHeight, 0.0);
                     glColor3ubv(lut[i+1]);
                     glVertex3f((((i + 1) * halfWidth * 2) / 256.0), -halfHeight, 0.0);
                     glColor3ubv(lut[i+1]);
                     glVertex3f((((i + 1) * halfWidth * 2) / 256.0), 0.0, 0.0);
                     glColor3ubv(lut[i]);
                     glVertex3f(((i * halfWidth * 2) / 256.0), 0.0, 0.0);
                  glEnd();
               }
            glPopMatrix();
         }
         break;
      case DisplaySettingsSurfaceShape::SURFACE_SHAPE_COLOR_MAP_PALETTE:
         {
            PaletteFile* pf = brainSet->getPaletteFile();
            if (pf->getNumberOfPalettes() <= 0) {
               glPopMatrix();
               return;
            }
            const int paletteNumber = dsss->getSelectedPaletteIndex();
            palette = pf->getPalette(paletteNumber);
            if (palette->getNumberOfPaletteEntries() <= 1) {
               glPopMatrix();
               return;
            }
            
            //
            // Always interpolate if the palette has only two colors
            //
            interpolatePaletteColor = (dsss->getInterpolatePaletteColors() != 0);
            if (palette->getNumberOfPaletteEntries() == 2) {
               interpolatePaletteColor = true;
            }
         
            if (selectFlag) {
               glPushName(SELECTION_MASK_PALETTE_SHAPE);
               glPushName(paletteNumber);
            }

            //
            // Draw the colorbar
            //
            for (int i = 1; i < palette->getNumberOfPaletteEntries(); i++) {
               PaletteEntry* lastPE = (PaletteEntry*)palette->getPaletteEntry(i - 1);
               float lastValue = lastPE->getValue();
               const int lastColorIndex = lastPE->getColorIndex();
               const PaletteColor* lastColor = pf->getPaletteColor(lastColorIndex);
               const QString lastName = lastColor->getName();
               const bool lastNoneColorFlag = lastColor->isNoneColor();
               unsigned char lastRGB[3];
               lastColor->getRGB(lastRGB);

               const PaletteEntry* pe = palette->getPaletteEntry(i);
               float value = pe->getValue();
               const int colorIndex = pe->getColorIndex();
               const PaletteColor* color = pf->getPaletteColor(colorIndex);
               const QString name = color->getName();
               unsigned char rgb[3];
               color->getRGB(rgb);

               if (selectFlag) {
                  glPushName(i - 1);
               }
               if (lastNoneColorFlag == false) {
                  if (interpolatePaletteColor) {
                     glBegin(GL_POLYGON);
                        glColor3ubv(rgb);
                        glVertex3f(value * halfWidth, -halfHeight, 0.0);
                        glVertex3f(value * halfWidth, 0.0, 0.0);
                        glColor3ubv(lastRGB);
                        glVertex3f(lastValue * halfWidth, 0.0, 0.0);
                        glVertex3f(lastValue * halfWidth, -halfHeight, 0.0);
                     glEnd();
                  }
                  else {
                     glColor3ubv(lastRGB);
                     glRectf(value * halfWidth, -halfHeight, 
                             lastValue * halfWidth, 0.0);
                  }
               }

               if (selectFlag) {
                  if (lastNoneColorFlag) {
                     glRectf(value * halfWidth, -halfHeight,
                             lastValue * halfWidth, 0.0);
                  }
                  glPopName();
               }

               lastPE = (PaletteEntry*)pe;
               
               //
               // is this the last palette entry
               //
               if (i == (palette->getNumberOfPaletteEntries() - 1)) {
                  if ((value > -1.0) && (value < 0.0)) {
                     if (selectFlag) {
                        glPushName(i);
                     }
                     glColor3ubv(rgb);
                     glRectf(-1.0 * halfWidth, -halfHeight, value * halfWidth, 0.0);
                     if (selectFlag) {
                        glPopName();
                     }
                  }
               }
            }
            
            if (selectFlag) {
               glPopName();
               glPopName();
            }
         }
         break;
   } 
   
   if (selectFlag == false) {
      //
      // Draw the min/max numbers
      //
      GLdouble maxX, minX, winY, winZ;
      GLdouble modelMatrix[16];
      GLdouble projMatrix[16];
      GLint viewport[4];
      glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
      glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
      glGetIntegerv(GL_VIEWPORT, viewport);
      gluProject(halfWidth, 0.0, 0.0, modelMatrix, projMatrix, viewport,
               &maxX, &winY, &winZ);                 
      gluProject(-halfWidth, 0.0, 0.0, modelMatrix, projMatrix, viewport,
               &minX, &winY, &winZ);                 
      
      float minShape = -1.0;
      float maxShape = 1.0;
      const int col = dsss->getShapeColumnForPaletteAndColorMapping();
      if (col < 0) {
         return;
      }
      ssf->getColumnColorMappingMinMax(col, minShape, maxShape);

      //
      // do everything in pixel display
      //
      glViewport(0, 0, viewport[2], viewport[3]);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0.0, viewport[2], 
            0.0, viewport[3], 
            -1.0, 1.0);
      glMatrixMode (GL_MODELVIEW);
      
      PreferencesFile* pf = brainSet->getPreferencesFile();
      unsigned char rf, gf, bf;
      pf->getSurfaceForegroundColor(rf, gf, bf);
      glColor3ub(rf, gf, bf);
      
      if (glWidget != NULL) {
         //
         // Font for drawing numbers
         //
         const int fontHeight = 12;
         QFont font("times", fontHeight);
         QFontMetrics fontMetrics(font);
         const double fontY = winY + (fontHeight * 0.75) - 5;
         
         //
         // draw minimum numbers
         //
         const double minFontX = minX;
         QString qs(QString::number(minShape, 'f', 2));
         if (glWidget != NULL) {
            glWidget->renderText(minFontX, fontY, 0.0, qs, font);
         }
         
         //
         // draw maximum numbers
         //
         qs = QString::number(maxShape, 'f', 2);
         const double maxFontX = maxX - fontMetrics.width(qs);
         if (glWidget != NULL) {
            glWidget->renderText(maxFontX, fontY, 0.0, qs, font);
         }

         if (palette != NULL) {
            if ((palette->getNumberOfPaletteEntries() == 2) &&
               interpolatePaletteColor) {
               // nothing
            }
            else {
               //
               // draw 0.0 
               //
               const QString qs("0.0");
               const double fontX = (minX + maxX)*0.5 - fontMetrics.width(qs) * 0.5;
               if (glWidget != NULL) {
                  glWidget->renderText(fontX, fontY, 0.0, qs, font);
               }
            }
         }
      }
      
      glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);
      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(savedProjectionMatrix);
      glMatrixMode(GL_MODELVIEW);
      
      glEnable(GL_DEPTH_TEST);
      glPopMatrix(); 
   }
}

/**
 * Draw the metric's palette colorbar.
 */
void
BrainModelOpenGL::drawMetricPalette(const int modelNumber,
                                    const bool surfaceFlag)
{   
   //
   // Check for selection mode
   //
   bool selectFlag = false;
   if (selectionMask & SELECTION_MASK_PALETTE_METRIC) {
      selectFlag = true;
   }
   else if (selectionMask != SELECTION_MASK_OFF) {
      return;
   }
   
   bool displayIt = false;
   if (surfaceFlag) {
      if (brainSet->isASurfaceOverlay(modelNumber,
              BrainModelSurfaceOverlay::OVERLAY_METRIC)) {
         displayIt = true;
      }
   }
   else if (surfaceFlag == false) {
      BrainModelVolume* bmv = brainSet->getBrainModelVolume();
      if (bmv != NULL) {
         VolumeFile* vf = bmv->getSelectedVolumeFunctionalViewFile();
         if (vf != NULL) {
            const BrainModelVolumeVoxelColoring* vc = brainSet->getVoxelColoring();
            if (vc->isUnderlayOrOverlay(BrainModelVolumeVoxelColoring::UNDERLAY_OVERLAY_FUNCTIONAL)) {
               displayIt = true;
            }
         }
      }
   }
   
   if (displayIt) {
      DisplaySettingsMetric* dsm = brainSet->getDisplaySettingsMetric();
      if (surfaceFlag) {
         if (dsm->getDisplayColorBar() == false) {
            return;
         }
      }
      else {
         DisplaySettingsVolume* dsv = brainSet->getDisplaySettingsVolume();
         if (dsv->getDisplayColorBar() == false) {
            return;
         }
      }
      PaletteFile* pf = brainSet->getPaletteFile();
      if (pf->getNumberOfPalettes() <= 0) {
         return;
      }
      const int paletteNumber = dsm->getSelectedPaletteIndex();
      const Palette* palette = pf->getPalette(paletteNumber);
      if (palette->getNumberOfPaletteEntries() <= 1) {
         return;
      }
      MetricFile* mf = brainSet->getMetricFile();
      if (surfaceFlag) {
         if (mf->getNumberOfColumns() <= 0) {
            return;
         }
      }

      glPushMatrix();
      glDisable(GL_DEPTH_TEST);
      
      glMatrixMode(GL_PROJECTION);
      
      // save the projection matrix for the surface
      GLfloat savedProjectionMatrix[16];
      glGetFloatv(GL_PROJECTION_MATRIX, savedProjectionMatrix);
      
      // save the viewport
      GLint savedViewport[4];
      glGetIntegerv(GL_VIEWPORT, savedViewport);
      
      //
      // colorbar is in lower right corner of graphics window
      //
      const GLint colorbarViewportWidth =
                       static_cast<int>(viewport[2] * 0.25);
      const GLint colorbarViewportHeight = 50;

      const GLint colorbarViewportLeft = viewport[2]
                                       - colorbarViewportWidth;
      const int yOffset = 15;   // prevent colorbar under resize tab
      glViewport(colorbarViewportLeft, yOffset, 
                 colorbarViewportWidth, colorbarViewportHeight + yOffset);

      const GLdouble halfWidth = static_cast<GLdouble>(colorbarViewportWidth / 2);
      const GLdouble halfHeight = static_cast<GLdouble>(colorbarViewportHeight / 2);

      glLoadIdentity();
      
      if (selectFlag) {
         GLint vp[4];
         glGetIntegerv(GL_VIEWPORT, vp);
         gluPickMatrix((GLdouble)selectionX, (GLdouble)(selectionY),  //(savedViewport[3] - selectionY),
                     5.0, 5.0, vp);
      }
      
      const GLdouble margin = 1.1;
      const GLdouble orthoWidth = halfWidth * margin;
      const GLdouble orthoHeight = halfHeight * margin;
      glOrtho(-orthoWidth,  orthoWidth, 
              -orthoHeight, orthoHeight, 
              -1.0, 1.0);
      glMatrixMode (GL_MODELVIEW);
      glLoadIdentity();
      
      //
      // set background color
      //
      unsigned char rb, gb, bb;
      PreferencesFile* pref = brainSet->getPreferencesFile();
      pref->getSurfaceBackgroundColor(rb, gb, bb);
      glColor3ub(rb, gb, bb);
      glRectf(-orthoWidth, -orthoHeight, orthoWidth, orthoHeight);

      bool haveNegativeInPalette = false;
         
      //
      // Always interpolate if the palette has only two colors
      //
      bool interpolateColor = (dsm->getInterpolateColors() != 0);
      if (palette->getNumberOfPaletteEntries() == 2) {
         interpolateColor = true;
      }
   
      if (selectFlag) {
         glPushName(SELECTION_MASK_PALETTE_METRIC);
         glPushName(paletteNumber);
      }
      
      //
      // Draw the colorbar
      //
      for (int i = 1; i < palette->getNumberOfPaletteEntries(); i++) {
         PaletteEntry* lastPE = (PaletteEntry*)palette->getPaletteEntry(i - 1);
         float lastValue = lastPE->getValue();
         const int lastColorIndex = lastPE->getColorIndex();
         const PaletteColor* lastColor = pf->getPaletteColor(lastColorIndex);
         const QString lastName = lastColor->getName();
         const bool lastNoneColorFlag = lastColor->isNoneColor();
         unsigned char lastRGB[3];
         lastColor->getRGB(lastRGB);

         const PaletteEntry* pe = palette->getPaletteEntry(i);
         float value = pe->getValue();
         const int colorIndex = pe->getColorIndex();
         const PaletteColor* color = pf->getPaletteColor(colorIndex);
         const QString name = color->getName();
         unsigned char rgb[3];
         color->getRGB(rgb);

         if (selectFlag) {
            glPushName(i - 1);
         }
         if (lastNoneColorFlag == false) {
            if (interpolateColor) {
               glBegin(GL_POLYGON);
                  glColor3ubv(rgb);
                  glVertex3f(value * halfWidth, -halfHeight, 0.0);
                  glVertex3f(value * halfWidth, 0.0, 0.0);
                  glColor3ubv(lastRGB);
                  glVertex3f(lastValue * halfWidth, 0.0, 0.0);
                  glVertex3f(lastValue * halfWidth, -halfHeight, 0.0);
               glEnd();
            }
            else {
               glColor3ubv(lastRGB);
               switch(dsm->getDisplayMode()) {
                  case DisplaySettingsMetric::METRIC_DISPLAY_MODE_POSITIVE_AND_NEGATIVE:
                     glRectf(value * halfWidth, -halfHeight, 
                             lastValue * halfWidth, 0.0);
                     break;
                  case DisplaySettingsMetric::METRIC_DISPLAY_MODE_NEGATIVE_ONLY:
                     if (value <= 0.0) {
                        if (lastValue > 0.0) {
                           lastValue = 0.0;
                        }
                        glRectf(value * halfWidth, -halfHeight, 
                                lastValue * halfWidth, 0.0);
                     }
                     break;
                  case DisplaySettingsMetric::METRIC_DISPLAY_MODE_POSITIVE_ONLY:
                     if (lastValue >= 0.0) {
                        if (value < 0.0) {
                           value = 0.0;
                        }
                        glRectf(value * halfWidth, -halfHeight, 
                                lastValue * halfWidth, 0.0);
                     }
                     break;
               }
            }
         }
         if (selectFlag) {
            if (lastNoneColorFlag) {
               glRectf(value * halfWidth, -halfHeight,
                       lastValue * halfWidth, 0.0);
            }
            glPopName();
         }
         
         lastPE = (PaletteEntry*)pe;
         
         if (value < 0) {
            haveNegativeInPalette = true;
         }
         
         //
         // is this the last palette entry
         //
         if (i == (palette->getNumberOfPaletteEntries() - 1)) {
            if ((value > -1.0) && (value < 0.0)) {
               if (dsm->getDisplayMode() != 
                  DisplaySettingsMetric::METRIC_DISPLAY_MODE_POSITIVE_ONLY) {
                  if (selectFlag) {
                     glPushName(i);
                  }
                  glColor3ubv(rgb);
                  glRectf(-1.0 * halfWidth, -halfHeight, value * halfWidth, 0.0);
                  if (selectFlag) {
                     glPopName();
                  }
               }
            }
         }
      }
           
      float posMinMetric = 0.0, posMaxMetric = 0.0, negMinMetric = 0.0, negMaxMetric = 0.0;
      int displayColumn, thresholdColumn;
      dsm->getMetricsForColoringAndPalette(displayColumn,
                                          thresholdColumn,
                                          negMaxMetric,
                                          negMinMetric,
                                          posMinMetric,
                                          posMaxMetric);                                          
      //
      // Are thresholds shown in green 
      //
      if (dsm->getShowSpecialColorForThresholdedNodes()) {
         if ((thresholdColumn >= 0) &&
             (thresholdColumn < mf->getNumberOfColumns())) {
            unsigned char negColor[3];
            unsigned char posColor[3];
            dsm->getSpecialColorsForThresholdedNodes(negColor, posColor);

            float negThresh, posThresh;
            mf->getColumnThresholding(thresholdColumn, negThresh, posThresh);
            
            if (negThresh < 0.0) {
               if (negMaxMetric < 0.0) {
                  const float negPct = negThresh / negMaxMetric;
                  glColor3ubv(negColor);
                  glRectf(-negPct * halfWidth, -halfHeight, 0.0, 0.0);
               }
            }
            
            if (posThresh > 0.0) {
               if (posMaxMetric > 0.0) {
                  const float posPct = posThresh / posMaxMetric;
                  glColor3ubv(posColor);
                  glRectf(0.0, -halfHeight, posPct * halfWidth, 0.0);
               }
            }
         }
      }
      
      if (selectFlag) {
         glPopName();
         glPopName();
      }
      else if (glWidget != NULL) {
         //
         // Draw the min/max numbers
         //
         GLdouble maxX, minX, winY, winZ;
         GLdouble modelMatrix[16];
         GLdouble projMatrix[16];
         GLint viewport[4];
         glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
         glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
         glGetIntegerv(GL_VIEWPORT, viewport);
         gluProject(halfWidth, 0.0, 0.0, modelMatrix, projMatrix, viewport,
                  &maxX, &winY, &winZ);                 
         gluProject(-halfWidth, 0.0, 0.0, modelMatrix, projMatrix, viewport,
                  &minX, &winY, &winZ);                 
         
         //
         // do everything in pixel display
         //
         glViewport(0, 0, savedViewport[2], savedViewport[3]);
         glMatrixMode(GL_PROJECTION);
         glLoadIdentity();
         glOrtho(0.0, savedViewport[2], 
                 0.0, savedViewport[3], 
                -1.0, 1.0);
         glMatrixMode (GL_MODELVIEW);
         
         PreferencesFile* prf = brainSet->getPreferencesFile();
         unsigned char rf, gf, bf;
         prf->getSurfaceForegroundColor(rf, gf, bf);
         glColor3ub(rf, gf, bf);
         
         //
         // Font for drawing numbers
         //
         const int fontHeight = 12;
         QFont font("times", fontHeight);
         QFontMetrics fontMetrics(font);
         const double fontY = winY + (fontHeight * 0.75) - 5;
         
         if (dsm->getDisplayMode() != DisplaySettingsMetric::METRIC_DISPLAY_MODE_POSITIVE_ONLY) {
            //
            // draw minimum numbers
            //
            if (palette->getPositiveOnly() == false) { 
               if ((negMaxMetric < 0.0) && haveNegativeInPalette) {
                  const double minFontX = minX;
                  const QString qs(QString::number(negMaxMetric, 'f', 1));
                  if (glWidget != NULL) {
                     glWidget->renderText(minFontX, fontY, 0.0, qs, font);
                  }
               }
            }
         }
         
         if (dsm->getDisplayMode() != DisplaySettingsMetric::METRIC_DISPLAY_MODE_NEGATIVE_ONLY) {
            //
            // draw maximum numbers
            //
            if (posMaxMetric > 0.0) {
               const QString qs(QString::number(posMaxMetric, 'f', 1));
               const double maxFontX = maxX - fontMetrics.width(qs);
               if (glWidget != NULL) {
                  glWidget->renderText(maxFontX, fontY, 0.0, qs, font);
               }
            }
         }
         
         if ((palette->getNumberOfPaletteEntries() == 2) &&
            interpolateColor) {
            // nothing
         }
         else {
            //
            // Determine if pos and/or neg min should be shown
            //
            bool showNeg = false;
            bool showPos = false;
            switch (dsm->getDisplayMode()) {
               case DisplaySettingsMetric::METRIC_DISPLAY_MODE_POSITIVE_AND_NEGATIVE:
                  showNeg = true;
                  if (negMinMetric != posMinMetric) {
                     showPos = true;
                  }
                  break;
               case DisplaySettingsMetric::METRIC_DISPLAY_MODE_POSITIVE_ONLY:
                  showPos = true;
                  break;
               case DisplaySettingsMetric::METRIC_DISPLAY_MODE_NEGATIVE_ONLY:
                  showNeg = true;
                  break;
            }
            
            QString qs;            
            if (showNeg) {
               qs += QString::number(negMinMetric, 'f', 1);
            }
            if (showPos) {
               if (qs.isEmpty() == false) {
                  qs += "/";
               }
               qs += QString::number(posMinMetric, 'f', 1);
            }

            const double fontX = (minX + maxX)*0.5 - fontMetrics.width(qs) * 0.5;
            if (glWidget != NULL) {
               glWidget->renderText(fontX, fontY, 0.0, qs, font);
            }
         }
      }
      
      glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);
      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(savedProjectionMatrix);
      glMatrixMode(GL_MODELVIEW);
      
      glEnable(GL_DEPTH_TEST);
      glPopMatrix(); 
   }
}   

/**
 * draw main window caption.
 */
void 
BrainModelOpenGL::drawMainWindowCaption()
{
   if (mainWindowCaption.isEmpty()) {
      return;
   }
   
   glDisable(GL_DEPTH_TEST);
   
   PreferencesFile* prf = brainSet->getPreferencesFile();
   unsigned char rf, gf, bf;
   prf->getSurfaceForegroundColor(rf, gf, bf);
   glColor3ub(rf, gf, bf);
   
   //
   // Font for drawing numbers
   //
   const int fontHeight = 18;
   QFont font("times", fontHeight);
   QFontMetrics fontMetrics(font);
   const int halfWidth = static_cast<int>(fontMetrics.width(mainWindowCaption) * 0.5);
   const int fontX = static_cast<int>(viewport[0] + (viewport[2] * 0.50))
                   - halfWidth;
   const int fontY = static_cast<int>(viewport[1] + (viewport[3] * 0.90));
   
   if (glWidget != NULL) {
      glWidget->renderText(fontX, fontY, mainWindowCaption, font);
   }
      
   glEnable(GL_DEPTH_TEST);
}      

/**
 * Display a QImage in the OpenGL window.
 */
void
BrainModelOpenGL::displayAnImage(QImage* image)
{
   DisplaySettingsImages* dsi = brainSet->getDisplaySettingsImages();
   
   const float xScale = static_cast<float>(viewport[2])  
                        / static_cast<float>(image->width());
   const float yScale = static_cast<float>(viewport[3]) 
                        / static_cast<float>(image->height());
            
   float pixelZoom = 1.0;
   float xPos = 0.0;
   float yPos = 0.0;
            
   switch (dsi->getImagePositionMode()) {
      case DisplaySettingsImages::IMAGE_POSITION_MODE_CENTER_OF_WINDOW:
         {
            const float imageCenterX = image->width() * 0.5;
            const float imageCenterY = image->height() * 0.5;
            const float windowCenterX = viewport[2] * 0.5;
            const float windowCenterY = viewport[3] * 0.5;
            xPos = windowCenterX - imageCenterX;
            yPos = windowCenterY - imageCenterY;
            
            if (xScale < yScale) {
               pixelZoom = xScale;
            }
            else {
               pixelZoom = yScale;
            }
            xPos = windowCenterX - (imageCenterX * pixelZoom);
            xPos = std::max(0.0f, xPos);
            yPos = windowCenterY - (imageCenterY * pixelZoom);
            yPos = std::max(0.0f, yPos);
         }
         break;
      case DisplaySettingsImages::IMAGE_POSITION_MODE_SCALE_TO_WINDOW:
         {
            bool centerImageX = false, centerImageY = false;
            if (xScale < yScale) {
               pixelZoom = xScale;
               centerImageY = true;
            }
            else {
               pixelZoom = yScale;
               centerImageX = true;
            }
            
            if (centerImageY) {
               // center image vertically
               const float ySize = image->height() * pixelZoom;
               const float margin = viewport[3] - ySize;
               yPos = margin * 0.5;
            }
            if (centerImageX) {
               // center image horizontally
               const float xSize = image->width() * pixelZoom;
               const float margin = viewport[3] - xSize;
               xPos = margin * 0.5;
            }

         }
         break;
   }
        
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
      glLoadIdentity();
      glOrtho(0, viewport[2], 0, viewport[3], 
              orthographicNear[viewingWindowNumber], orthographicFar[viewingWindowNumber]); 
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
         glLoadIdentity();

         //
         // Draw image near far clipping plane
         //
         const float imageZ = 10.0 -orthographicFar[viewingWindowNumber];
         glRasterPos3f(xPos, yPos, imageZ); //-500.0); // set Z so image behind surface
         glPixelZoom(pixelZoom, pixelZoom);

         glDrawPixels(image->width(), image->height(),
                      GL_RGBA, GL_UNSIGNED_BYTE, image->bits());
      glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
}

/**
 * Display background and splash images
 */
void
BrainModelOpenGL::displayImages()
{
   if (brainSet->getDisplaySplashImage()) {
      displayAnImage(brainSet->getSplashImage());
   }
   
   DisplaySettingsImages* dsi = brainSet->getDisplaySettingsImages();
   if (dsi->getShowImageInMainWindow() && 
       (dsi->getMainWindowImageNumber() >= 0) &&
       (dsi->getMainWindowImageNumber() < brainSet->getNumberOfImageFiles())) {
      QImage* image = dsi->getMainWindowImage();
      if (image != NULL) {
         displayAnImage(image);
      }
   }
}

/**
 * Draw the linear object (typically used while drawing borders)
 */
void
BrainModelOpenGL::drawLinearObject()
{
   glDisable(GL_DEPTH_TEST);
   const int num = linearObjectBeingDrawn.getNumberOfLinks();
   glPointSize(getValidPointSize(2.0));
   glColor3f(1.0, 0.0, 0.0);
   glBegin(GL_POINTS);
      for (int i = 0; i < num; i++) {
         glVertex3fv(linearObjectBeingDrawn.getLinkXYZ(i));
      }
   glEnd();
   glEnable(GL_DEPTH_TEST);
}

/**
 * Draw the VTK models.
 */
void
BrainModelOpenGL::drawAllVtkModels()
{
   const int num = brainSet->getNumberOfVtkModelFiles();
   for (int modelNum = 0; modelNum < num; modelNum++) {
      VtkModelFile* vmf = brainSet->getVtkModelFile(modelNum);
      drawVtkModelFile(vmf, modelNum);
   }
}


/**
 * Draw a VTK model file.
 */
void 
BrainModelOpenGL::drawVtkModelFile(VtkModelFile* vmf, const int modelNum)
{
   DisplaySettingsModels* dsm = brainSet->getDisplaySettingsModels();
   TransformationMatrixFile* tmf = brainSet->getTransformationMatrixFile();
   
   bool drawModelFlag = false;
   bool doingSelectionAnyFlag = false;
   bool doingSelectionTrianglesFlag = false;
   bool doingSelectionVerticesFlag = false;
   if (vmf->getDisplayFlag()) {
      drawModelFlag = true;
      if (selectionMask != SELECTION_MASK_OFF) {
         if (selectionMask & SELECTION_MASK_VTK_MODEL) {
            if (modelNum >= 0) {
               doingSelectionAnyFlag      = true;
               doingSelectionVerticesFlag = true;
               doingSelectionTrianglesFlag = true;
            }
         }
         else {
            drawModelFlag = false;
         }
      }
   }
   
   const float opacityFloat = dsm->getOpacity();
   const unsigned char alphaOverride = static_cast<unsigned char>(opacityFloat * 255.0);
   
   if (drawModelFlag) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      
      glPushMatrix();
      {
         if (doingSelectionTrianglesFlag
             || doingSelectionAnyFlag) {
            glPushName(SELECTION_MASK_VTK_MODEL);
            glPushName(modelNum);
         }
         
         const TransformationMatrix* tm = vmf->getAssociatedTransformationMatrix();
         if (tmf->getMatrixIndex(tm) >= 0) {
            float matrix[16];
            tm->getMatrix(matrix);
            glMultMatrixf(matrix);
         }
         
         const CoordinateFile* cf = vmf->getCoordinateFile();
         if (cf->getNumberOfCoordinates() > 0) {
            const float* coords = cf->getCoordinate(0);
            const unsigned char* colors = vmf->getPointColor(0);
            const float* normals = vmf->getPointNormal(0);
            
            //
            // Draw polygons
            //
            if (dsm->getLightPolygonsEnabled()) {
               glEnable(GL_LIGHTING);
               glEnable(GL_COLOR_MATERIAL);
            }
            else {
               glDisable(GL_LIGHTING);
               glDisable(GL_COLOR_MATERIAL);
            } 
            if (dsm->getShowPolygons()) {           
               const int numPolys = vmf->getNumberOfPolygons();
               for (int i = 0; i < numPolys; i++) {
                  const VtkModelFile::VtkModelObject* poly = vmf->getPolygon(i);
                  glBegin(GL_POLYGON);
                     const int num = poly->getNumberOfItems();
                     const int* pts = poly->getPointIndex(0);
                     for (int n = 0; n < num; n++) {
                        const int p3 = pts[n] * 3;
                        const int p4 = pts[n] * 4;
                        unsigned char alpha = colors[p4+3];
                        if (alphaOverride < 255) {
                           alpha = alphaOverride;
                        }
                        glColor4ub(colors[p4], colors[p4+1], colors[p4+2], alpha);
                        glNormal3fv(&normals[p3]);
                        glVertex3fv(&coords[p3]);
                     }
                  glEnd();
               }
            }
            
            //
            // Draw triangles
            //
            if (dsm->getShowTriangles()) {
               const int numTriangles = vmf->getNumberOfTriangles();
               if (numTriangles > 0) {
                  for (int i = 0; i < numTriangles; i++) {
                     if (doingSelectionTrianglesFlag) {
                        glPushName(GL_TRIANGLES);  // selecting triangles
                        glPushName(i);
                     }
                     glBegin(GL_TRIANGLES);
                        const int* t = vmf->getTriangle(i);
                        const int p1 = t[0] * 3;
                        const int p2 = t[1] * 3;
                        const int p3 = t[2] * 3;
                        const int p14 = t[0] * 4;
                        const int p24 = t[1] * 4;
                        const int p34 = t[2] * 4;
                        
                        unsigned char alpha1 = colors[p14+3];
                        unsigned char alpha2 = colors[p24+3];
                        unsigned char alpha3 = colors[p34+3];
                        if (alphaOverride < 255) {
                           alpha1 = alphaOverride;
                           alpha2 = alphaOverride;
                           alpha3 = alphaOverride;
                        }
                        
                        glColor4ub(colors[p14], colors[p14+1], colors[p14+2], alpha1);
                        glNormal3fv(&normals[p1]);
                        glVertex3fv(&coords[p1]);
                        
                        glColor4ub(colors[p24], colors[p24+1], colors[p24+2], alpha2);
                        glNormal3fv(&normals[p2]);
                        glVertex3fv(&coords[p2]);
                        
                        glColor4ub(colors[p34], colors[p34+1], colors[p34+2], alpha3);
                        glNormal3fv(&normals[p3]);
                        glVertex3fv(&coords[p3]);
                     glEnd();
                     if (doingSelectionTrianglesFlag) {
                        glPopName();
                        glPopName();
                     }
                  }
               }
            }
            
            //
            // Draw lines
            //
            if (dsm->getLightLinesEnabled()) {
               glEnable(GL_LIGHTING);
               glEnable(GL_COLOR_MATERIAL);
            }
            else {
               glDisable(GL_LIGHTING);
               glDisable(GL_COLOR_MATERIAL);
            }
            if (dsm->getShowLines()) {
               const int numLines = vmf->getNumberOfLines();
               glLineWidth(getValidLineWidth(dsm->getLineWidth()));
               for (int i = 0; i < numLines; i++) {
                  const VtkModelFile::VtkModelObject* line = vmf->getLine(i);
                  glBegin(GL_LINE_STRIP);
                     const int num = line->getNumberOfItems();
                     const int* pts = line->getPointIndex(0);
                     for (int n = 0; n < num; n++) {
                        const int p3 = pts[n] * 3;
                        const int p4 = pts[n] * 4;
                        unsigned char alpha = colors[p4+3];
                        if (alphaOverride < 255) {
                           alpha = alphaOverride;
                        }
                        glColor4ub(colors[p4], colors[p4+1], colors[p4+2], alpha);
                        glNormal3fv(&normals[p3]);
                        glVertex3fv(&coords[p3]);
                     }
                  glEnd();
               }
            }
            
            
            //
            // Draw vertices
            //
            if (dsm->getLightVerticesEnabled()) {
               glEnable(GL_LIGHTING);
               glEnable(GL_COLOR_MATERIAL);
            }
            else {
               glDisable(GL_LIGHTING);
               glDisable(GL_COLOR_MATERIAL);
            }
            if (dsm->getShowVertices()) {
               const int numVerts = vmf->getNumberOfVertices();
               const float s = dsm->getVertexSize() * 0.5;
               for (int i = 0; i < numVerts; i++) {
                  const int pointNumber = *vmf->getVertex(i);
                  const int p3 = pointNumber * 3;
                  const int p4 = pointNumber * 4;
                  unsigned char alpha = colors[p4+3];
                  if (alphaOverride < 255) {
                     alpha = alphaOverride;
                  }
                  glPushMatrix();
                     if (doingSelectionVerticesFlag) {
                        glPushName(GL_POINTS);  // selecting points
                        glPushName(i);
                     }
                     glColor4ub(colors[p4], colors[p4+1], colors[p4+2], alpha);
                     glTranslatef(coords[p3], coords[p3+1], coords[p3+2]);
                     drawSphere(s);
                     if (doingSelectionVerticesFlag) {
                        glPopName();
                        glPopName();
                     }
                  glPopMatrix();
               }
            }
         }
         
         if (doingSelectionAnyFlag) {
            glPopName();
            glPopName();
         }
      }
      
      glPopMatrix();
      
      glDisable(GL_BLEND);
   }
   
   glDisable(GL_LIGHTING);
   glDisable(GL_COLOR_MATERIAL);
}

/**
 * Draw transformation data files.
 */
void 
BrainModelOpenGL::drawTransformationDataFiles(const TransformationMatrix* tm)
{
   const int num = brainSet->getNumberOfTransformationDataFiles();
   for (int i = 0; i < num; i++) {
      AbstractFile* af = brainSet->getTransformationDataFile(i);
      if (af->getAssociatedTransformationMatrix() == tm) {
         FociFile* ff = dynamic_cast<FociFile*>(af);
         if (ff != NULL) {
            drawTransformationCellOrFociFile(NULL,
                               ff,
                               brainSet->getFociColorFile(),
                               brainSet->getDisplaySettingsFoci(),
                               i,
                               SELECTION_MASK_TRANSFORM_FOCI);
         }

         ContourCellFile* ccf = dynamic_cast<ContourCellFile*>(af);
         if (ccf != NULL) {
            drawTransformationCellOrFociFile(NULL,
                               ccf,
                               brainSet->getContourCellColorFile(),
                               brainSet->getDisplaySettingsCells(),
                               i,
                               SELECTION_MASK_TRANSFORM_CONTOUR_CELL);
         }

         CellFile* cf = dynamic_cast<CellFile*>(af);
         if ((ff == NULL) && (ccf == NULL) && (cf != NULL)) {
            drawTransformationCellOrFociFile(NULL,
                               cf,
                               brainSet->getCellColorFile(),
                               brainSet->getDisplaySettingsCells(),
                               i,
                               SELECTION_MASK_TRANSFORM_CELL);
         }

         VtkModelFile* vmf = dynamic_cast<VtkModelFile*>(af);
         if (vmf != NULL) {
            drawVtkModelFile(vmf, -1);
         }
         
         {
            glPushMatrix();
            
            ContourFile* cf = dynamic_cast<ContourFile*>(af);
            if (cf != NULL) {
               const DisplaySettingsContours* dsc = brainSet->getDisplaySettingsContours();
               const int numContours = cf->getNumberOfContours();
               const float spacing = cf->getSectionSpacing();
               for (int k = 0; k < numContours; k++) {
                  const CaretContour* contour = cf->getContour(k);
                  const int sectionNumber = contour->getSectionNumber();
                  const float z = sectionNumber * spacing;
                  const int numPoints = contour->getNumberOfPoints();
                  
                  if (dsc->getDrawMode() != DisplaySettingsContours::DRAW_MODE_POINTS) {
                     glBegin(GL_LINE_LOOP);
                  }
                  else {
                     glBegin(GL_POINTS);
                  }
                     for (int j = 0; j < numPoints; j++) {
                        if ((j == 0) && (dsc->getShowEndPoints())) {
                           glColor3ub(255, 0, 0);
                        }
                        else {
                           glColor3ub(0, 255, 0);
                        }
                        float x, y;
                        contour->getPointXY(j, x, y);
                        glVertex3f(x, y, z);
                     }
                  glEnd();
               }
            } // if
            
            glPopMatrix();
         }
      }
   }
}      

/**
 * Draw the deformation field vectors.
 */
void 
BrainModelOpenGL::drawDeformationFieldVectors(BrainModelSurface* bms)
{
   DeformationFieldFile* dff = brainSet->getDeformationFieldFile();
   if (dff->getNumberOfColumns() <= 0) {
      return;
   }
   
   DisplaySettingsDeformationField* dsdf = brainSet->getDisplaySettingsDeformationField();
   switch (dsdf->getDisplayMode()) {
      case DisplaySettingsDeformationField::DISPLAY_MODE_ALL:
         break;
      case DisplaySettingsDeformationField::DISPLAY_MODE_NONE:
         break;
      case DisplaySettingsDeformationField::DISPLAY_MODE_SPARSE:
         break;
   }
   const CoordinateFile* coords = bms->getCoordinateFile();
   const int numNodes = bms->getNumberOfNodes();

   const int column = dsdf->getSelectedDisplayColumn();
   
   const TopologyHelper* th = bms->getTopologyFile()->getTopologyHelper(false, true, false);
   
   const BrainModelSurface* fiducialBMS = brainSet->getActiveFiducialSurface();
   const CoordinateFile* fiducialCoords = (fiducialBMS != NULL) ? fiducialBMS->getCoordinateFile()
                                                                : NULL;
   const float *fiducialXYZ = (fiducialCoords != NULL) ? fiducialCoords->getCoordinate(0)
                                                       : NULL;

   float unstretchFactor = 1.0;
   bool showUnstretched = false;
   dsdf->getShowUnstretchedOnFlat(unstretchFactor, showUnstretched);
   
   int nodes[3];
   float areas[3];
   const float* xyz = coords->getCoordinate(0);
   glBegin(GL_LINES);
      for (int i = 0; i < numNodes; i++) {
         const int i3 = i * 3;
         
         //
         // If vector should be displayed
         //
         if (dsdf->getDisplayVectorForNode(i)) {
            //
            // If node has neighbors
            //
            if (th->getNodeHasNeighbors(i)) {
               //
               // Get tip of vector
               //
               DeformationFieldNodeInfo* dfni = dff->getDeformationInfo(i, column);
               dfni->getData(nodes, areas);
               
               bool showIt = true;
               for (int j = 0; j < 3; j++) {
                  if ((nodes[j] < 0) || (nodes[j] >= numNodes)) {
                     showIt = false;
                     break;
                  }
                  if (th->getNodeHasNeighbors(nodes[j]) == false) {
                     showIt = false;
                     break;
                  }
               }
               
               if (showIt) {
                  //
                  // Unproject
                  //
                  float tipXYZ[3];
                  BrainModelSurfacePointProjector::unprojectPoint(nodes, areas, coords, tipXYZ);
                  
                  if ((bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FLAT) ||
                      (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FLAT_LOBAR)) {
                     if (showUnstretched) {                          
                        if (fiducialBMS != NULL) {
                           float fidTipXYZ[3];
                           BrainModelSurfacePointProjector::unprojectPoint(nodes, areas, 
                                                                fiducialCoords, fidTipXYZ);
                           const float vecLength = MathUtilities::distance3D(&xyz[i3], tipXYZ);
                           const float fidLength = MathUtilities::distance3D(&fiducialXYZ[i3],
                                                                             fidTipXYZ);
                           if (fidLength != 0.0) {
                              if ((vecLength / fidLength) > unstretchFactor) {
                                 showIt = false;
                              }
                           }
                        }
                     }
                  }

                  if (showIt) {
                     //
                     // Start drawing at node and continue to tip
                     //
                     glColor3ub(255, 255, 0);
                     glVertex3fv(&xyz[i3]);
                     glColor3ub(255, 0, 0);
                     glVertex3fv(tipXYZ);
                     
                     if (DebugControl::getDebugOn()) {
                        if (DebugControl::getDebugNodeNumber() == i) {
                           const float vecLength = MathUtilities::distance3D(&xyz[i3], tipXYZ);
                           std::cout << "Def Field Vector Tip ("
                                     << FileUtilities::basename(coords->getFileName()).toAscii().constData()
                                     << ") " << tipXYZ[0] << ", " << tipXYZ[1] << ", " << tipXYZ[2]
                                     << " length: " << vecLength << std::endl;
                        }
                     }
                  }
               }
            }
         }
      }
   glEnd();
}

/**
 * Check a vector's orientation (true if orientation is valid for display).
 */
bool
BrainModelOpenGL::checkVectorOrientation(const float vector[3])
{
   bool valid = true;

   DisplaySettingsVectors* dsv = brainSet->getDisplaySettingsVectors();
   const DisplaySettingsVectors::DISPLAY_ORIENTATION orientation =
      dsv->getDisplayOrientation();
   if (orientation != DisplaySettingsVectors::DISPLAY_ORIENTATION_ANY) {
      float axisVector[3];
      switch (orientation) {
         case DisplaySettingsVectors::DISPLAY_ORIENTATION_ANY:
            break;
         case DisplaySettingsVectors::DISPLAY_ORIENTATION_LEFT_RIGHT:
            axisVector[0] = 1.0;
            axisVector[1] = 0.0;
            axisVector[2] = 0.0;
            break;
         case DisplaySettingsVectors::DISPLAY_ORIENTATION_POSTERIOR_ANTERIOR:
            axisVector[0] = 0.0;
            axisVector[1] = 1.0;
            axisVector[2] = 0.0;
            break;
         case DisplaySettingsVectors::DISPLAY_ORIENTATION_INFERIOR_SUPERIOR:
            axisVector[0] = 0.0;
            axisVector[1] = 0.0;
            axisVector[2] = 1.0;
            break;
      }

      const DisplaySettingsVectors::VECTOR_TYPE vectorType = dsv->getVectorType();
      bool directedFlag = false;
      switch (vectorType) {
         case DisplaySettingsVectors::VECTOR_TYPE_BIDIRECTIONAL:
            break;
         case DisplaySettingsVectors::VECTOR_TYPE_UNIDIRECTIONAL_ARROW:
         case DisplaySettingsVectors::VECTOR_TYPE_UNIDIRECTIONAL_CYLINDER:
            directedFlag = true;
            break;
      }

      //
      // Dot produce is cosine of angle between vectors
      //
      const float angle = dsv->getDisplayOrientationAngle();
      if (directedFlag) {
         float angleCosine = std::cos(angle
                                      * MathUtilities::degreesToRadians());
         float dot = MathUtilities::dotProduct(vector, axisVector);
         if (angle < 0.0) {
            angleCosine = -angleCosine;
            if (dot >= angleCosine) {
               valid = false;
            }
         }
         else {
            if (dot < angleCosine) {
               valid = false;
            }
         }
      }
      else {
         const float angleCosine = std::cos(angle
                                            * MathUtilities::degreesToRadians());
         float dot = std::fabs(MathUtilities::dotProduct(vector, axisVector));
         if (dot < angleCosine) {
            valid = false;
         }
      }
   }

   return valid;
}

/**
 * Draw the vectors in 3D space.
 */
void 
BrainModelOpenGL::drawVectorFile3D(BrainModelSurface* bms)
{
   const int numVectorFiles = brainSet->getNumberOfVectorFiles();
   if (numVectorFiles <= 0) {
      return;
   }

   DisplaySettingsVectors* dsv = brainSet->getDisplaySettingsVectors();
   int displayIncrement = 1;
   switch (dsv->getDisplayModeSurface()) {
      case DisplaySettingsVectors::DISPLAY_MODE_ALL:
         break;
      case DisplaySettingsVectors::DISPLAY_MODE_NONE:
         return;
         break;
      case DisplaySettingsVectors::DISPLAY_MODE_SPARSE:
         displayIncrement = dsv->getSparseDisplayDistance();
         break;
   }

   DisplaySettingsSurface* dss = brainSet->getDisplaySettingsSurface();
   //
   // Setup clipping planes
   //
   bool applyClippingPlanesFlag = false;
   switch (dss->getClippingPlaneApplication()) {
      case DisplaySettingsSurface::CLIPPING_PLANE_APPLICATION_MAIN_WINDOW_ONLY:
         if (viewingWindowNumber == BrainModel::BRAIN_MODEL_VIEW_MAIN_WINDOW) {
            applyClippingPlanesFlag = true;
         }
         break;
      case DisplaySettingsSurface::CLIPPING_PLANE_APPLICATION_FIDUCIAL_SURFACES_ONLY:
         if (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL) {
            applyClippingPlanesFlag = true;
         }
         break;
      case DisplaySettingsSurface::CLIPPING_PLANE_APPLICATION_ALL_SURFACES:
         applyClippingPlanesFlag = true;
         break;
   }

   //
   // Surface clipping, use coordinate of vector rather than clipping
   // planes since clipping planes chop off part of a vector instead
   // of the whole thing.
   //
   float xMin = -std::numeric_limits<float>::max();
   float xMax =  std::numeric_limits<float>::max();
   float yMin = -std::numeric_limits<float>::max();
   float yMax =  std::numeric_limits<float>::max();
   float zMin = -std::numeric_limits<float>::max();
   float zMax =  std::numeric_limits<float>::max();
   if (applyClippingPlanesFlag) {
      if (dss->getClippingPlaneEnabled(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_X_NEGATIVE)) {
         xMin = dss->getClippingPlaneCoordinate(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_X_NEGATIVE);
      }
      if (dss->getClippingPlaneEnabled(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_X_POSITIVE)) {
         xMax = dss->getClippingPlaneCoordinate(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_X_POSITIVE);
      }
      if (dss->getClippingPlaneEnabled(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Y_NEGATIVE)) {
         yMin = dss->getClippingPlaneCoordinate(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Y_NEGATIVE);
      }
      if (dss->getClippingPlaneEnabled(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Y_POSITIVE)) {
         yMax = dss->getClippingPlaneCoordinate(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Y_POSITIVE);
      }
      if (dss->getClippingPlaneEnabled(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Z_NEGATIVE)) {
         zMin = dss->getClippingPlaneCoordinate(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Z_NEGATIVE);
      }
      if (dss->getClippingPlaneEnabled(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Z_POSITIVE)) {
         zMax = dss->getClippingPlaneCoordinate(DisplaySettingsSurface::CLIPPING_PLANE_AXIS_Z_POSITIVE);
      }
   }
   
   
   const DisplaySettingsVectors::COLOR_MODE colorMode = dsv->getColorMode();
   const DisplaySettingsVectors::VECTOR_TYPE vectorType = dsv->getVectorType();
   const DisplaySettingsVectors::SURFACE_SYMBOL surfaceSymbol = dsv->getSurfaceSymbol();
   const bool drawWithMagnitudeFlag = dsv->getDrawWithMagnitude();
   const float lengthMultiplier = dsv->getLengthMultiplier();
   const float minimumMagnitude = dsv->getMagnitudeThreshold();
   const float lineRadius = dsv->getSurfaceVectorLineWidth();

   VolumeFile* segmentationMaskVolume = NULL;
   if (dsv->getSegmentationMaskingVolumeEnabled()) {
      segmentationMaskVolume = dsv->getSegmentationMaskingVolumeFile();
   }
   VolumeFile* functionalMaskVolume = NULL;
   const float functionalMaskNegThresh = dsv->getFunctionalMaskingVolumeNegativeThreshold();
   const float functionalMaskPosThresh = dsv->getFunctionalMaskingVolumePositiveThreshold();
   if (dsv->getFunctionalMaskingVolumeEnabled()) {
      functionalMaskVolume = dsv->getFunctionalMaskingVolumeFile();
   }

   GLboolean cullFaceStatus = glIsEnabled(GL_CULL_FACE);
   glEnable(GL_CULL_FACE);

   glColor3f(0.0, 1.0, 0.0);
   switch (surfaceSymbol) {
      case DisplaySettingsVectors::SURFACE_SYMBOL_3D:
         glEnable(GL_COLOR_MATERIAL);
         glEnable(GL_LIGHTING);
         break;
      case DisplaySettingsVectors::SURFACE_SYMBOL_2D_LINE:
         glDisable(GL_COLOR_MATERIAL);
         glDisable(GL_LIGHTING);
         break;
   }

   for (int m = 0; m < numVectorFiles; m++) {
      if (dsv->getDisplayVectorFile(m)) {
         VectorFile* vf = brainSet->getVectorFile(m);
         const int numVectors = vf->getNumberOfVectors();
         for (int i = 0; i < numVectors; i += displayIncrement) {
            float xyz[3], vector[3], rgba[4], magnitude, radius;
            int nodeNumber;
            vf->getVectorData(i, xyz, vector, magnitude, nodeNumber, rgba, radius);
            if (magnitude < minimumMagnitude) {
               continue;
            }
            float length = lengthMultiplier;
            if (drawWithMagnitudeFlag) {
               length *= magnitude;
            }

            //
            // Adjust radius for surface vector line width for drawing as lines
            //
            switch (surfaceSymbol) {
               case DisplaySettingsVectors::SURFACE_SYMBOL_3D:
                  break;
               case DisplaySettingsVectors::SURFACE_SYMBOL_2D_LINE:
                  radius *= lineRadius;
                  break;
            }

            switch (vectorType) {
               case DisplaySettingsVectors::VECTOR_TYPE_BIDIRECTIONAL:
                  {
                     xyz[0] -= (vector[0] * length * 0.5);
                     xyz[1] -= (vector[1] * length * 0.5);
                     xyz[2] -= (vector[2] * length * 0.5);
                  }
                  break;
               case DisplaySettingsVectors::VECTOR_TYPE_UNIDIRECTIONAL_ARROW:
               case DisplaySettingsVectors::VECTOR_TYPE_UNIDIRECTIONAL_CYLINDER:
                  break;
            }

            //
            // Check surface clipping
            //
            if (applyClippingPlanesFlag) {
               if (xyz[0] < xMin) {
                  continue;
               }
               if (xyz[0] > xMax) {
                  continue;
               }
               if (xyz[1] < yMin) {
                  continue;
               }
               if (xyz[1] > yMax) {
                  continue;
               }
               if (xyz[2] < zMin) {
                  continue;
               }
               if (xyz[2] > zMax) {
                  continue;
               }
            }

            //
            // Is coordinate within the mask volume
            //
            if (segmentationMaskVolume != NULL) {
               int ijk[3];
               if (segmentationMaskVolume->convertCoordinatesToVoxelIJK(xyz, ijk)) {
                  if (segmentationMaskVolume->getVoxel(ijk) == 0.0) {
                     continue;
                  }
               }
               else {
                  continue;
               }
            }
            if (functionalMaskVolume != NULL) {
               int ijk[3];
               if (functionalMaskVolume->convertCoordinatesToVoxelIJK(xyz, ijk)) {
                  const float value = functionalMaskVolume->getVoxel(ijk);
                  if (value > 0.0) {
                     if (value < functionalMaskPosThresh) {
                        continue;
                     }
                  }
                  else if (value < 0.0) {
                     if (value > functionalMaskNegThresh) {
                        continue;
                     }
                  }
                  else {
                     continue;
                  }
               }
               else {
                  continue;
               }
            }

            if (checkVectorOrientation(vector) == false) {
               continue;
            }

            float endPoint[3] = {
               xyz[0] + vector[0] * length,
               xyz[1] + vector[1] * length,
               xyz[2] + vector[2] * length
            };

            switch(colorMode) {
               case DisplaySettingsVectors::COLOR_MODE_VECTOR_COLORS:
                  break;
               case DisplaySettingsVectors::COLOR_MODE_XYZ_AS_RGB:
                  rgba[0] = std::fabs(vector[0]);
                  rgba[1] = std::fabs(vector[1]);
                  rgba[2] = std::fabs(vector[2]);
                  rgba[3] = 1.0;
                  break;
            }
            glColor4fv(rgba);
            switch (vectorType) {
               case DisplaySettingsVectors::VECTOR_TYPE_BIDIRECTIONAL:
                  switch (surfaceSymbol) {
                     case DisplaySettingsVectors::SURFACE_SYMBOL_3D:
                        drawCylinderSymbol(xyz, endPoint, radius);
                        break;
                     case DisplaySettingsVectors::SURFACE_SYMBOL_2D_LINE:
                        {
                           glLineWidth(getValidLineWidth(radius));
                           glBegin(GL_LINES);
                              glVertex3fv(xyz);
                              glVertex3fv(endPoint);
                           glEnd();
                        }
                        break;
                  }
                  break;
               case DisplaySettingsVectors::VECTOR_TYPE_UNIDIRECTIONAL_ARROW:
                  switch (surfaceSymbol) {
                     case DisplaySettingsVectors::SURFACE_SYMBOL_3D:
                        drawArrowSymbol(xyz, endPoint, radius);
                        break;
                     case DisplaySettingsVectors::SURFACE_SYMBOL_2D_LINE:
                        {
                           glLineWidth(getValidLineWidth(radius));
                           glBegin(GL_LINES);
                              glVertex3fv(xyz);
                              glVertex3fv(endPoint);
                           glEnd();
                           glPointSize(getValidPointSize(radius * 3.0));
                           glBegin(GL_POINTS);
                              glVertex3fv(endPoint);
                           glEnd();
                        }
                        break;
                  }
                  break;
               case DisplaySettingsVectors::VECTOR_TYPE_UNIDIRECTIONAL_CYLINDER:
                  switch (surfaceSymbol) {
                     case DisplaySettingsVectors::SURFACE_SYMBOL_3D:
                        drawCylinderSymbol(xyz, endPoint, radius);
                        break;
                     case DisplaySettingsVectors::SURFACE_SYMBOL_2D_LINE:
                        {
                           glLineWidth(getValidLineWidth(radius));
                           glBegin(GL_LINES);
                              glVertex3fv(xyz);
                              glVertex3fv(endPoint);
                           glEnd();
                        }
                        break;
                  }
                  break;
            }
         }
      }
   }

   if (cullFaceStatus == GL_FALSE) {
       glDisable(GL_CULL_FACE);
   }
   glDisable(GL_COLOR_MATERIAL);
   glDisable(GL_LIGHTING);

   glDisable(GL_BLEND);
}

/**
 * Draw the volume foci file.
 */
void
BrainModelOpenGL::drawVectorsOnVolume(const VolumeFile::VOLUME_AXIS axis,
                                      const float axisCoord,
                                      const float /*voxelSize*/)
{
   //const float halfVoxelSize = voxelSize * 0.6;
   const int numVectorFiles = brainSet->getNumberOfVectorFiles();
   if (numVectorFiles <= 0) {
      return;
   }

   DisplaySettingsVectors* dsv = brainSet->getDisplaySettingsVectors();
   int displayIncrement = 1;
   switch (dsv->getDisplayModeVolume()) {
      case DisplaySettingsVectors::DISPLAY_MODE_ALL:
         break;
      case DisplaySettingsVectors::DISPLAY_MODE_NONE:
         return;
         break;
      case DisplaySettingsVectors::DISPLAY_MODE_SPARSE:
         displayIncrement = dsv->getSparseDisplayDistance();
         break;
   }

   int axisIndex = 0;
   switch (axis) {
      case VolumeFile::VOLUME_AXIS_X:
         axisIndex = 0;
         break;
      case VolumeFile::VOLUME_AXIS_Y:
         axisIndex = 1;
         break;
      case VolumeFile::VOLUME_AXIS_Z:
         axisIndex = 2;
         break;
      case VolumeFile::VOLUME_AXIS_ALL:
      case VolumeFile::VOLUME_AXIS_OBLIQUE:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
      case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
      case VolumeFile::VOLUME_AXIS_UNKNOWN:
         return;
         break;
   }

   const DisplaySettingsVectors::COLOR_MODE colorMode = dsv->getColorMode();
   const bool drawWithMagnitudeFlag = dsv->getDrawWithMagnitude();
   const DisplaySettingsVectors::VECTOR_TYPE vectorType = dsv->getVectorType();
   const float lengthMultiplier = dsv->getLengthMultiplier();
   const float aboveLimit = dsv->getVolumeSliceDistanceAboveLimit();
   const float belowLimit = dsv->getVolumeSliceDistanceBelowLimit();
   const float minimumMagnitude = dsv->getMagnitudeThreshold();
   VolumeFile* segmentationMaskVolume = NULL;
   if (dsv->getSegmentationMaskingVolumeEnabled()) {
      segmentationMaskVolume = dsv->getSegmentationMaskingVolumeFile();
   }
   VolumeFile* functionalMaskVolume = NULL;
   const float functionalMaskNegThresh = dsv->getFunctionalMaskingVolumeNegativeThreshold();
   const float functionalMaskPosThresh = dsv->getFunctionalMaskingVolumePositiveThreshold();
   if (dsv->getFunctionalMaskingVolumeEnabled()) {
      functionalMaskVolume = dsv->getFunctionalMaskingVolumeFile();
   }


   glLineWidth(1.0);

   for (int m = 0; m < numVectorFiles; m++) {
      VectorFile* vf = brainSet->getVectorFile(m);
      const int numVectors = vf->getNumberOfVectors();
      if (dsv->getDisplayVectorFile(m)) {
         for (int i = 0; i < numVectors; i += displayIncrement) {
            float xyzOrigin[3], vector[3], rgba[4], magnitude, radius;
            int nodeNumber;
            vf->getVectorData(i, xyzOrigin, vector, magnitude,
                              nodeNumber, rgba, radius);
            if (magnitude < minimumMagnitude) {
               continue;
            }
            const float dist = xyzOrigin[axisIndex] - axisCoord;
            if ((dist > belowLimit) &&
                (dist < aboveLimit)) {
               //
               // Is coordinate within the mask volume
               //
               if (segmentationMaskVolume != NULL) {
                  int ijk[3];
                  if (segmentationMaskVolume->convertCoordinatesToVoxelIJK(xyzOrigin, ijk)) {
                     if (segmentationMaskVolume->getVoxel(ijk) == 0.0) {
                        continue;
                     }
                  }
                  else {
                     continue;
                  }
               }
               if (functionalMaskVolume != NULL) {
                  int ijk[3];
                  if (functionalMaskVolume->convertCoordinatesToVoxelIJK(xyzOrigin, ijk)) {
                     const float value = functionalMaskVolume->getVoxel(ijk);
                     if (value > 0.0) {
                        if (value < functionalMaskPosThresh) {
                           continue;
                        }
                     }
                     else if (value < 0.0) {
                        if (value > functionalMaskNegThresh) {
                           continue;
                        }
                     }
                     else {
                        continue;
                     }
                  }
                  else {
                     continue;
                  }
               }

               if (checkVectorOrientation(vector) == false) {
                  continue;
               }

               float xyzScreen[3] = { xyzOrigin[0], xyzOrigin[1], xyzOrigin[2] };
               convertVolumeItemXYZToScreenXY(axis, xyzScreen);

               float lengthMag = lengthMultiplier;
               if (drawWithMagnitudeFlag) {
                  lengthMag *= magnitude;
               }
               float endPoint[3] = {
                  xyzOrigin[0] + vector[0] * lengthMag,
                  xyzOrigin[1] + vector[1] * lengthMag,
                  xyzOrigin[2] + vector[2] * lengthMag
               };
               convertVolumeItemXYZToScreenXY(axis, endPoint);

               const float dz = endPoint[2] - xyzScreen[2];
               const float dy = endPoint[1] - xyzScreen[1];
               const float dx = endPoint[0] - xyzScreen[0];
               const float length = std::sqrt(dx*dx + dy*dy + dz*dz);
               const float rotation = std::atan2(dy, dx);
               glPushMatrix();
               glTranslatef(xyzScreen[0], xyzScreen[1], xyzScreen[2]);
               glRotatef(rotation * MathUtilities::radiansToDegrees(),
                         0.0, 0.0, 1.0);
               const float z = xyzScreen[2];

               glLineWidth(getValidLineWidth(radius));

               switch(colorMode) {
                  case DisplaySettingsVectors::COLOR_MODE_VECTOR_COLORS:
                     break;
                  case DisplaySettingsVectors::COLOR_MODE_XYZ_AS_RGB:
                     rgba[0] = std::fabs(vector[0]);
                     rgba[1] = std::fabs(vector[1]);
                     rgba[2] = std::fabs(vector[2]);
                     rgba[3] = 1.0;
                     break;
               }

               switch (vectorType) {
                  case DisplaySettingsVectors::VECTOR_TYPE_BIDIRECTIONAL:
                     {
                        glScalef(length, length * radius, 1.0);
                        glBegin(GL_LINES);
                        glColor4fv(rgba);
                        glVertex3f(-0.5, 0.0, z);
                        glVertex3f( 0.5, 0.0, z);
                        glEnd();
                     }
                     break;
                  case DisplaySettingsVectors::VECTOR_TYPE_UNIDIRECTIONAL_ARROW:
                     {
                        glScalef(length, length * radius, 1.0);
                        glBegin(GL_LINES);
                        glColor4fv(rgba);
                        glVertex3f(0.0, 0.0, z);
                        glVertex3f(1.0, 0.0, z);
                        glVertex3f(1.0, 0.0, z);
                        glVertex3f(0.75, 0.25, z);
                        glVertex3f(1.0, 0.0, z);
                        glVertex3f(0.75, -0.25, z);
                        glEnd();
                     }
                     break;
                  case DisplaySettingsVectors::VECTOR_TYPE_UNIDIRECTIONAL_CYLINDER:
                     {
                        glScalef(length, length * radius, 1.0);
                        glBegin(GL_LINES);
                        glColor4fv(rgba);
                        glVertex3f(0.0, 0.0, z);
                        glVertex3f(1.0, 0.0, z);
                        glEnd();
                     }
                     break;
               }

               glPopMatrix();
            }
         }
      }
   }
}

/**
 * Draw an arrow symbol.
 * From: http://lifeofaprogrammergeek.blogspot.com/2008/07/rendering-cylinder-between-two-points.html
 */
void
BrainModelOpenGL::drawArrowSymbol(const float xyz[3],
                                  const float tipXYZ[3],
                                  const float radius)
{
    const float x1 = xyz[0];
    const float y1 = xyz[1];
    const float z1 = xyz[2];
    float vx = tipXYZ[0] - x1; //x2-x1;
    float vy = tipXYZ[1] - y1; //y2-y1;
    float vz = tipXYZ[2] - z1; //z2-z1;

    float v = std::sqrt( vx*vx + vy*vy + vz*vz );
    double ax = 0.0;



    double zero = 1.0e-3;

    if (std::fabs(vz) < zero) {
        ax = 57.2957795*std::acos( vx/v ); // rotation angle in x-y plane
        //if ( vx <= 0.0 ) ax = -ax;  // JWH Fixes when Z is near zero
        if ( vy <= 0.0 ) ax = -ax; // JWH Fixes when Z is near zero
    }
    else {
        ax = 57.2957795*std::acos( vz/v ); // rotation angle
        if ( vz <= 0.0 ) ax = -ax;
    }

    float rx = -vy*vz;
    float ry = vx*vz;
    glPushMatrix();
        glTranslatef( x1,y1,z1 );
        if (fabs(vz) < zero)  {
            glRotated(90.0, 0, 1, 0.0); // Rotate & align with x axis
            glRotated(ax, -1.0, 0.0, 0.0); // Rotate to point 2 in x-y plane
        }
        else {
            glRotated(ax, rx, ry, 0.0); // Rotate about rotation vector
        }
        
        glPushMatrix();
            glScalef(radius, radius, v);
            drawCylinder();
        glPopMatrix();

        glPushMatrix();
           glTranslatef(0.0, 0.0, v);
           glScalef(radius*2, radius*2, 1.0);
           drawCone();
        glPopMatrix();

        glPushMatrix();
           glTranslatef(0.0, 0.0, 0.0);
           glScalef(radius, radius, 1.0);
           drawDisk(1.0);
        glPopMatrix();
    glPopMatrix();
}

/**
 * Draw a cylinder symbol.
 * From: http://lifeofaprogrammergeek.blogspot.com/2008/07/rendering-cylinder-between-two-points.html
 */
void
BrainModelOpenGL::drawCylinderSymbol(const float xyz[3],
                                     const float tipXYZ[3],
                                     const float radius)
{
    const float x1 = xyz[0];
    const float y1 = xyz[1];
    const float z1 = xyz[2];
    float vx = tipXYZ[0] - x1; //x2-x1;
    float vy = tipXYZ[1] - y1; //y2-y1;
    float vz = tipXYZ[2] - z1; //z2-z1;

    float v = std::sqrt( vx*vx + vy*vy + vz*vz );
    double ax = 0.0;



    double zero = 1.0e-3;

    if (std::fabs(vz) < zero) {
        ax = 57.2957795*std::acos( vx/v ); // rotation angle in x-y plane
        //if ( vx <= 0.0 ) ax = -ax;  // JWH Fixes when Z is near zero
        if ( vy <= 0.0 ) ax = -ax;  // JWH Fixes when Z is near zero
    }
    else {
        ax = 57.2957795*std::acos( vz/v ); // rotation angle
        if ( vz <= 0.0 ) ax = -ax;
    }

    float rx = -vy*vz;
    float ry = vx*vz;
    glPushMatrix();
        glTranslatef( x1,y1,z1 );
        if (fabs(vz) < zero)  {
            glRotated(90.0, 0, 1, 0.0); // Rotate & align with x axis
            glRotated(ax, -1.0, 0.0, 0.0); // Rotate to point 2 in x-y plane
        }
        else {
            glRotated(ax, rx, ry, 0.0); // Rotate about rotation vector
        }

        glPushMatrix();
            glScalef(radius, radius, v);
            drawCylinder();
        glPopMatrix();

/*
        glPushMatrix();
           glTranslatef(0.0, 0.0, v);
           glScalef(radius, radius, 1.0);
           drawDisk(1.0);
        glPopMatrix();

        glPushMatrix();
           glTranslatef(0.0, 0.0, 0.0);
           glScalef(radius, radius, 1.0);
           drawDisk(1.0);
        glPopMatrix();
*/
    glPopMatrix();
}

/**
 * get the orthographic size.
 */
void 
BrainModelOpenGL::getOrtho(float& orthoRight, float& orthoTop) const
{
   orthoRight = orthographicRight[0];
   orthoTop   = orthographicTop[0];
}      

/**
 * Called to get a 3D point on a model at a screen X/Y.
 * Returns "true" if the pointOut is valid.
 */
bool
BrainModelOpenGL::getSurfacePointAtDisplayXY(BrainSet* bs,
                                           BrainModelSurface* bms,
                                           const int viewingWindowNumber,
                                           const int viewport[4],
                                           const int displayX, 
                                           const int displayY,
                                           float pointOut[3])
{
   //
   // Must be a surface
   //
   if (bms == NULL) {
      return false;
   }
   brainSet = bs;
   
   if (DebugControl::getDebugOn()) {
      std::cout << "Doing special tile selection." << std::endl;
   }
   
   GLboolean ditheringOn = glIsEnabled(GL_DITHER);
   glDisable(GL_DITHER);
   
   glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   const double aspectRatio = (static_cast<double>(viewport[2])) /
                              (static_cast<double>(viewport[3]));

   DisplaySettingsSurface* dss = brainSet->getDisplaySettingsSurface();
   switch (dss->getViewingProjection()) {
      case DisplaySettingsSurface::VIEWING_PROJECTION_ORTHOGRAPHIC:
         glOrtho(orthographicLeft[viewingWindowNumber], orthographicRight[viewingWindowNumber],
                 orthographicBottom[viewingWindowNumber], orthographicTop[viewingWindowNumber],
                 orthographicNear[viewingWindowNumber], orthographicFar[viewingWindowNumber]);
         break;
      case DisplaySettingsSurface::VIEWING_PROJECTION_PERSPECTIVE:
         gluPerspective(bms->getPerspectiveFieldOfView(),
                        aspectRatio,
                        1.0, 1000.0);
         break;
   }

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   switch (dss->getViewingProjection()) {
         case DisplaySettingsSurface::VIEWING_PROJECTION_ORTHOGRAPHIC:
            break;
         case DisplaySettingsSurface::VIEWING_PROJECTION_PERSPECTIVE:
            gluLookAt(0.0, 0.0, bms->getPerspectiveZooming(viewingWindowNumber), 
                      0.0, 0.0,0.0, 0.0,1.0, 0.0);
            break;
   }

   float translate[3];
   bms->getTranslation(viewingWindowNumber, translate);
   glTranslatef(translate[0], translate[1], translate[2]);

   float matrix[16];
   bms->getRotationMatrix(viewingWindowNumber, matrix);
   glMultMatrixf(matrix);
   const BrainSetNodeAttribute* attributes = brainSet->getNodeAttributes(0);
   
   float scale[3];
   bms->getScaling(viewingWindowNumber, scale);
   glScalef(scale[0], scale[1], scale[2]);

   glDisable(GL_LIGHTING);
   glDisable(GL_COLOR_MATERIAL);

   glDrawBuffer(GL_BACK);
   glClearColor(0.0, 0.0, 0.0, 1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
   const CoordinateFile* cf = bms->getCoordinateFile();
   const TopologyFile* tf = bms->getTopologyFile();
   const float* coords = cf->getCoordinate(0);
   const int numTiles = tf->getNumberOfTiles();
   
   for (int i = 0; i < numTiles; i++) {
      int v1, v2, v3;
      tf->getTile(i, v1, v2, v3);
      if (attributes[v1].getDisplayFlag() || 
          attributes[v2].getDisplayFlag() || 
          attributes[v3].getDisplayFlag()) {
         
         const int maskFF = 0xff;
         const int tile = i + 1;   /// can't have zero since zero is where no drawing occurs
         const unsigned char blue = tile & maskFF;
         const unsigned char green = (tile >> 8) & maskFF;
         const unsigned char red = (tile >> 16) & maskFF;
         glColor3ub(red, green, blue);
         
         glBegin(GL_TRIANGLES);
            glVertex3fv(&coords[v1*3]);
            glVertex3fv(&coords[v2*3]);
            glVertex3fv(&coords[v3*3]);
         glEnd();
      }
   }

   //
   // Figure out which tile was pickes
   //
   glReadBuffer(GL_BACK);
      
   unsigned char pixels[3] = { 0, 0, 0 };
   glPixelStorei( GL_PACK_SKIP_ROWS, 0);
   glPixelStorei( GL_PACK_SKIP_PIXELS, 0);
   glPixelStorei( GL_PACK_ALIGNMENT, 1);
   glReadPixels(displayX, 
                viewport[3] - displayY,
                1,
                1,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                (GLvoid*)pixels);
   const int red = pixels[0];
   const int green = pixels[1];
   const int blue = pixels[2];
   int tile = (red << 16) + (green << 8) + blue;
   tile--;
   
   if (ditheringOn) {
      glEnable(GL_DITHER);
   }

   if (DebugControl::getDebugOn()) {
      std::cout << "Tile selected: " << tile << std::endl;
   }
   if (tile < 0) {
      brainSet = NULL;
      return false;
   }
   
 // CONVERT DISPLAY X/Y INTO BARYCENTRIC COORD IN TILE AND THEN
 // POINT IN TILE TO 3D
   
   //
   // Get vertices of tile
   //
   int v1, v2, v3;
   tf->getTile(tile, v1, v2, v3);

   //
   // Get coordinates of vertices
   //   
   float nodePos[3][3];
   cf->getCoordinate(v1, nodePos[0]);
   cf->getCoordinate(v2, nodePos[1]);
   cf->getCoordinate(v3, nodePos[2]);

   //
   // Convert vertices of tile to display coordinates
   //
   GLdouble modelMatrix[16];
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
   GLdouble projMatrix[16];
   glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
   float nodeDispPos[3][3];
   for (int i = 0; i < 3; i++) {
      GLdouble wx, wy, wz;
      if (gluProject(nodePos[i][0], nodePos[i][1], nodePos[i][2],
                     modelMatrix, projMatrix, (GLint*)viewport,
                     &wx, &wy, &wz) == GL_FALSE) {
         brainSet = NULL;
         return false;
      }
      nodeDispPos[i][0] = wx;
      nodeDispPos[i][1] = wy;
      nodeDispPos[i][2] = 0.0;  // WANT ZERO 
   }
   
   
   //
   // Compute area of tile in display coordinates
   //
   const float tileArea = MathUtilities::triangleArea(nodeDispPos[0],
                                                    nodeDispPos[1],
                                                    nodeDispPos[2]);
   if (DebugControl::getDebugOn()) {
      std::cout << "Triangle area: " << tileArea << std::endl;
   } 
                                                   
   //
   // If tile area is small use a vertex from the tile
   //
   if (tileArea < 0.001) {
      pointOut[0] = nodePos[0][0];
      pointOut[1] = nodePos[0][1];
      pointOut[2] = nodePos[0][2];
   }
   else {
      //
      // Determine barycentric coordinates of display X/Y in tile
      //
      float displayXYZ[3] = { displayX, viewport[3] - displayY, 0.0 };
      const float areaU = MathUtilities::triangleArea(displayXYZ, nodeDispPos[1], nodeDispPos[2])
                          / tileArea;
      const float areaV = MathUtilities::triangleArea(displayXYZ, nodeDispPos[2], nodeDispPos[0])
                          / tileArea;
      const float areaW = MathUtilities::triangleArea(displayXYZ, nodeDispPos[0], nodeDispPos[1])
                          / tileArea;
      float totalArea = areaU + areaV + areaW;
      if (totalArea <= 0) {
         totalArea = 1.0;
      }
      if ((areaU < 0.0) || (areaV < 0.0) || (areaW < 0.0)) {
         std::cout << "Invalid tile area: less than zero." << std::endl;
         brainSet = NULL;
         return false;
      }
      
      //
      // Convert to surface coordinates
      //
      pointOut[0] = (nodePos[0][0]*areaU + nodePos[1][0]*areaV + nodePos[2][0]*areaW) / totalArea;
      pointOut[1] = (nodePos[0][1]*areaU + nodePos[1][1]*areaV + nodePos[2][1]*areaW) / totalArea;
      pointOut[2] = (nodePos[0][2]*areaU + nodePos[1][2]*areaV + nodePos[2][2]*areaW) / totalArea;
   }
   if (DebugControl::getDebugOn()) {
      std::cout << "Point in tile: " 
                << pointOut[0] << " "
                << pointOut[1] << " "
                << pointOut[2] << std::endl;
   }
   
   //
   // Determing position of surface point on window
   //
   GLdouble winX, winY, winZ;
   if (gluProject(pointOut[0], pointOut[1], pointOut[2],
                  modelMatrix, projMatrix, (GLint*)viewport,
                  &winX, &winY, &winZ) == GL_FALSE) {
      brainSet = NULL;
      return false;
   }
   winY = viewport[3] - winY;
   
   //
   // For some reason, point occaisionally project to some weird location.
   // So check their distance from the display location input to this method
   // and throw out those that do not appear to be correct.
   //
   const float dist = std::sqrt((winX - displayX) * (winX - displayX) +
                           (winY - displayY) * (winY - displayY));
   if (DebugControl::getDebugOn()) {
      std::cout << "INPUT (" << displayX << ", " << displayY << ") "
                << "PROJ-TO (" << winX << ", " << winY << ")"
                << std::endl;
   }
   if (dist > 1.8) {
      brainSet = NULL;
      return false;
   }
             
   return true;
}

/**
 * identify items in the brain model (returns text for ID display).
 */
QString 
BrainModelOpenGL::identifyBrainModelItem(BrainSet* bs,
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
                                         const bool enableVocabularyLinks)
{
   for (int i = 0; i < BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; i++) {
      allWindowBrainModelsForIdentification[i] = allWindowBrainModelsForIdentificationIn[i];
   }
   
   //
   // Select items
   //
   selectBrainModelItem(bs,
                        bm,
                        viewingWindowNumberIn,
                        viewportIn,
                        glWidgetIn,
                        selectionMaskIn,
                        selectionXIn,
                        selectionYIn,
                        viewModeFlag);
                        
   //
   // Generate identification text
   //
   BrainModelIdentification* bmid = bs->getBrainModelIdentification();
   const QString idText = bmid->getIdentificationText(this, 
                                                      enableHtml,
                                                      enableVocabularyLinks);
   //std::cout << "ID Information: " << std::endl
   //          << idText.toAscii().constData() << std::endl;

   return idText;
}

/**
 * Called when an Identify mouse click has been made on a brain model.
 */
void
BrainModelOpenGL::selectBrainModelItem(BrainSet* bs,
                                       BrainModel* bm,
                                       const int viewingWindowNumberIn,
                                       const int viewportIn[4],
                                       QGLWidget* glWidgetIn,
                                       const unsigned long selectionMaskIn,
                                       const int selectionXIn, 
                                       const int selectionYIn,
                                       const bool viewModeFlag)
{
   brainSet = bs;
   brainModel     = bm;
   viewport[0]    = viewportIn[0];
   viewport[1]    = viewportIn[1];
   viewport[2]    = viewportIn[2];
   viewport[3]    = viewportIn[3];
   viewingWindowNumber = viewingWindowNumberIn;
   glWidget       = glWidgetIn;
   glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
   
   selectedSurfaceTile.reset(brainSet, brainModel, viewingWindowNumber);
   selectedNode.reset(brainSet, brainModel, viewingWindowNumber);
   selectedBorder1.reset(brainSet, brainModel, viewingWindowNumber);
   selectedBorder2.reset(brainSet, brainModel, viewingWindowNumber);
   selectedVolumeBorder.reset(brainSet, brainModel, viewingWindowNumber);
   selectedCellProjection.reset(brainSet, brainModel, viewingWindowNumber);
   selectedVolumeCell.reset(brainSet, brainModel, viewingWindowNumber);
   selectedCut.reset(brainSet, brainModel, viewingWindowNumber);
   selectedFocusProjection.reset(brainSet, brainModel, viewingWindowNumber);
   selectedVolumeFoci.reset(brainSet, brainModel, viewingWindowNumber);
   selectedPaletteMetric.reset(brainSet, brainModel, viewingWindowNumber);
   selectedPaletteShape.reset(brainSet, brainModel, viewingWindowNumber);
   selectedContour.reset(brainSet, brainModel, viewingWindowNumber);
   selectedContourCell.reset(brainSet, brainModel, viewingWindowNumber);
   selectedTransformCell.reset(brainSet, brainModel, viewingWindowNumber);
   selectedTransformFoci.reset(brainSet, brainModel, viewingWindowNumber);
   selectedTransformContour.reset(brainSet, brainModel, viewingWindowNumber);
   selectedTransformContourCell.reset(brainSet, brainModel, viewingWindowNumber);
   selectedVoxelUnderlay.reset(brainSet, brainModel, viewingWindowNumber);
   selectedVoxelOverlaySecondary.reset(brainSet, brainModel, viewingWindowNumber);
   selectedVoxelOverlayPrimary.reset(brainSet, brainModel, viewingWindowNumber);
   selectedVoxelFunctionalCloud.reset(brainSet, brainModel, viewingWindowNumber);
   selectedLink.reset(brainSet, brainModel, viewingWindowNumber);
   selectedTransformationMatrixAxes.reset(brainSet, brainModel, viewingWindowNumber);
   selectedVtkModel.reset(brainSet, brainModel, viewingWindowNumber);
   
   selectionMask = selectionMaskIn;
   
   //GLint viewport[4];
   //glGetIntegerv(GL_VIEWPORT, viewport);

   glSelectBuffer(SELECTION_BUFFER_SIZE, selectionBuffer);
   
   glRenderMode(GL_SELECT);
   
   glInitNames();

   glMatrixMode(GL_PROJECTION);
   //GLfloat projectionMatrix[16];
   //glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);
   
   glLoadIdentity();
   selectionX = selectionXIn;
   selectionY = selectionViewport[viewingWindowNumber][3] - selectionYIn;
   GLdouble pickWidth  = 5.0;
   GLdouble pickHeight = 5.0;
   
   DisplaySettingsVolume* dsv = brainSet->getDisplaySettingsVolume();

   //
   // Special stuff for some volume modes that draw more than one volume slice
   //
   if (bm->getModelType() == BrainModel::BRAIN_MODEL_VOLUME) {
      BrainModelVolume* bmv = brainSet->getBrainModelVolume();
      if (bmv != NULL) {
         switch (bmv->getSelectedAxis(viewingWindowNumber)) {
            case VolumeFile::VOLUME_AXIS_X:
            case VolumeFile::VOLUME_AXIS_Y:
            case VolumeFile::VOLUME_AXIS_Z:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_X:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_Y:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_Z:
               if (dsv->getMontageViewSelected()) {
                  //
                  // Get montage info
                  //
                  int rows, columns, sliceIncrement;
                  dsv->getMontageViewSettings(rows, columns, sliceIncrement);
                  const int vpHeight = viewport[3] / rows;
                  const int vpWidth  = viewport[2] / columns;
                  for (int i = (rows - 1); i >= 0; i--) {
                     for (int j = 0; j < columns; j++) {
                     //for (int i = 0; i < rows; i++) {         
                        const int vpX = j * vpWidth;
                        const int vpY = i * vpHeight;
                        if ((selectionX > vpX) &&
                            (selectionY > vpY) &&
                            (selectionX < (vpX + vpWidth)) &&
                            (selectionY < (vpY + vpHeight))) {
                           selectionViewport[viewingWindowNumber][0] = vpX;
                           selectionViewport[viewingWindowNumber][1] = vpY;
                           selectionViewport[viewingWindowNumber][2] = vpWidth;
                           selectionViewport[viewingWindowNumber][3] = vpHeight;
                        }
                     }
                  }
               }
               break;
            case VolumeFile::VOLUME_AXIS_ALL:
            case VolumeFile::VOLUME_AXIS_OBLIQUE_ALL:
               {
                  int startX = 0;
                  int startY = 0;
                  const int halfX = viewport[2] / 2;
                  const int halfY = viewport[3] / 2;
                  selectionX = selectionXIn;
                  selectionY = viewportIn[3] - selectionYIn;
                  if (selectionX > halfX) {
                     startX = halfX;
                  }
                  if (selectionY > halfY) {
                     startY = halfY;
                  }
                  selectionViewport[viewingWindowNumber][0] = startX;
                  selectionViewport[viewingWindowNumber][1] = startY;
                  selectionViewport[viewingWindowNumber][2] = halfX;
                  selectionViewport[viewingWindowNumber][3] = halfY;
               }
               break;
            case VolumeFile::VOLUME_AXIS_OBLIQUE:
            case VolumeFile::VOLUME_AXIS_UNKNOWN:
               break;
         }
      }
   }
   
   //
   // If only picking tiles
   //
   if (selectionMask == SELECTION_MASK_TILE) {
      pickWidth  = 0.0;
      pickHeight = 0.0;
   }
   gluPickMatrix((GLdouble)selectionXIn, 
                 //(GLdouble)(selectionViewport[viewingWindowNumber][3] - selectionYIn),
                 (GLdouble)(viewportIn[3] - selectionYIn),
                 pickWidth, pickHeight, selectionViewport[viewingWindowNumber]);
   
   glOrtho(orthographicLeft[viewingWindowNumber], orthographicRight[viewingWindowNumber], 
           orthographicBottom[viewingWindowNumber], orthographicTop[viewingWindowNumber], 
           orthographicNear[viewingWindowNumber], orthographicFar[viewingWindowNumber]);
           
   drawBrainModelPrivate(bm,
                  viewingWindowNumber,
                  viewportIn,
                  glWidgetIn);
                  
   const GLint numHits = glRenderMode(GL_RENDER);
   
   processSelectedItems(numHits);
   
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixd(selectionProjectionMatrix[viewingWindowNumber]);
   glMatrixMode(GL_MODELVIEW);
   
   //
   // If both a tile and node found
   //
   if ((selectionMask & SELECTION_MASK_NODE) &&
       (selectionMask & SELECTION_MASK_TILE)) {
      if ((selectedNode.getItemIndex1() >= 0) &&
          (selectedSurfaceTile.getItemIndex1() >= 0)) {
         //
         // If tile is closer
         //
         if (selectedSurfaceTile.getDepth() < 
             selectedNode.getDepth()) {
            //
            // Use tile
            //
            selectedNode.setItemIndex1(-1);
         }
      }
   }
   
   //
   // If no nodes found but a tile was found (user may be zoomed in on surface)
   //
   if ((selectionMask & SELECTION_MASK_NODE) &&
       (selectionMask & SELECTION_MASK_TILE)) {
      if ((selectedNode.getItemIndex1() < 0) &&
          (selectedSurfaceTile.getItemIndex1() >= 0)) {
         //
         // Get the nodes in the tile
         //
         BrainModelSurface* bms = dynamic_cast<BrainModelSurface*>(bm);
         if (bms != NULL) {
            const TopologyFile* tf = bms->getTopologyFile();
            if (tf != NULL) {
               int nodes[3];
               tf->getTile(selectedSurfaceTile.getItemIndex1(), nodes);
               
               //
               // Find node nearest cursor
               //
               int nearestNode = -1;
               float nearestDistance = std::numeric_limits<float>::max();
               for (int k = 0; k < 3; k++) {
                  const CoordinateFile* cf = bms->getCoordinateFile();
                  float pos[3];
                  cf->getCoordinate(nodes[k], pos);
                  GLdouble windowPos[3];
                  if (gluProject(pos[0], pos[1], pos[2],
                                 getSelectionModelviewMatrix(viewingWindowNumberIn),
                                 getSelectionProjectionMatrix(viewingWindowNumberIn), 
                                 getSelectionViewport(viewingWindowNumberIn),
                                 &windowPos[0], &windowPos[1], &windowPos[2]) == GL_TRUE) {
                     const double dx = windowPos[0] - selectionX;
                     const double dy = windowPos[1] - selectionY;
                     const double dist = std::sqrt(dx*dx + dy*dy);
                     if (dist < nearestDistance) {
                        nearestDistance = dist;
                        nearestNode = nodes[k];
                     }
                  }
               }
               if (nearestNode >= 0) {
                  selectedNode.replaceIfCloser(0.0, 0.0, 
                                               BrainModelOpenGLSelectedItem::ITEM_TYPE_NODE,
                                               nearestNode);
                  if (DebugControl::getDebugOn()) {
                     std::cout << "Setting selectedNode via selectedTile, node: "
                               << nearestNode << std::endl;
                  }
               }
            }
         }
      }
   }

   //
   // In a viewing mode?
   //
   if (viewModeFlag) {
      //
      // Is this surface and volume ?
      //
      if (dynamic_cast<BrainModelSurfaceAndVolume*>(bm) != NULL) {
         //
         // In surface and volume, display ID for only node/voxel nearest viewer
         //
         if ((selectedNode.getItemIndex1() >= 0) &&
             (selectedVoxelUnderlay.getItemIndex1() >= 0)) {
            if (selectedNode.getDepth() < selectedVoxelUnderlay.getDepth()) {
               selectedVoxelUnderlay.setItemIndex1(-1);
            }
            else {
               selectedNode.setItemIndex1(-1);
            }
         }
      }   
      
      //
      // Is this surface ?  If so, highlight voxel in overlay/underlay volumes and contours
      //
      //if ((dynamic_cast<BrainModelSurfaceAndVolume*>(bm) == NULL) &&
      //         (dynamic_cast<BrainModelSurface*>(bm) != NULL)) {
      if (dynamic_cast<BrainModelSurface*>(bm) != NULL) {
         BrainModelSurface* bms = dynamic_cast<BrainModelSurface*>(bm);
         switch (bms->getStructure().getType()) {
            case Structure::STRUCTURE_TYPE_CORTEX_LEFT:
               bms = bs->getLeftFiducialVolumeInteractionSurface();
               break;
            case Structure::STRUCTURE_TYPE_CORTEX_RIGHT:
               bms = bs->getRightFiducialVolumeInteractionSurface();
               break;
            case Structure::STRUCTURE_TYPE_CORTEX_BOTH:
               bms = NULL;
               break;
            case Structure::STRUCTURE_TYPE_CEREBELLUM:
               bms = bs->getCerebellumFiducialVolumeInteractionSurface();
               break;
            case Structure::STRUCTURE_TYPE_CEREBELLUM_OR_CORTEX_LEFT:
               bms = NULL;
               break;
            case Structure::STRUCTURE_TYPE_CEREBELLUM_OR_CORTEX_RIGHT:
               bms = NULL;
               break;
            case Structure::STRUCTURE_TYPE_CORTEX_LEFT_OR_CEREBELLUM:
               bms = NULL;
               break;
            case Structure::STRUCTURE_TYPE_CORTEX_RIGHT_OR_CEREBELLUM:
               bms = NULL;
               break;
            case Structure::STRUCTURE_TYPE_CEREBRUM_CEREBELLUM:
            case Structure::STRUCTURE_TYPE_SUBCORTICAL:
            case Structure::STRUCTURE_TYPE_ALL:
            case Structure::STRUCTURE_TYPE_INVALID:
               bms = NULL;
               break;
         }
         if (bms == NULL) {
            bms = bs->getActiveFiducialSurface();
         }
         const int nodeNumber = selectedNode.getItemIndex1();
         if ((bms != NULL) && (nodeNumber >= 0)) {
            //
            // Get the nodes position
            //
            CoordinateFile* cf = bms->getCoordinateFile();
            float xyz[3];
            cf->getCoordinate(nodeNumber, xyz);
            
            //
            // See if contours are loaded
            //
            BrainModelContours* bmc = bs->getBrainModelContours(-1);
            if (bmc != NULL) {
               //
               // highlight nearest contour point
               //
               const ContourFile* cf = bmc->getContourFile();
               int contourNumber;
               int contourPointNumber;
               cf->findContourPoint(xyz, 3.0, contourNumber, contourPointNumber);
               if ((contourNumber >= 0) && (contourPointNumber >= 0)) {
                  selectedContour.reset(bs,
                                        bms,
                                        viewingWindowNumber);
                  selectedContour.setItemIndex1(contourNumber);
                  selectedContour.setItemIndex2(contourPointNumber);
               }
            }
            
            //
            // See if there is a volume loaded
            //
            BrainModelVolume* bmv = bs->getBrainModelVolume(-1);
            if (bmv != NULL) {
               //
               // Highlight primary overlay voxel nearest node
               //
               VolumeFile* vf3 = bmv->getOverlayPrimaryVolumeFile();
               if (vf3 != NULL) {
                  int ijk[3];
                  if (vf3->convertCoordinatesToVoxelIJK(xyz, ijk)) {
                     selectedVoxelOverlayPrimary.reset(bs,
                                                       bmv,
                                                       viewingWindowNumber);
                     selectedVoxelOverlayPrimary.setItemIndex1(ijk[0]);
                     selectedVoxelOverlayPrimary.setItemIndex2(ijk[1]);
                     selectedVoxelOverlayPrimary.setItemIndex3(ijk[2]);
                  }
               }
               //
               // Highlight secondary overlay voxel nearest node
               //
               VolumeFile* vf2 = bmv->getOverlaySecondaryVolumeFile();
               if (vf2 != NULL) {
                  int ijk[3];
                  if (vf2->convertCoordinatesToVoxelIJK(xyz, ijk)) {
                     selectedVoxelOverlaySecondary.reset(bs,
                                                         bmv,
                                                         viewingWindowNumber);
                     selectedVoxelOverlaySecondary.setItemIndex1(ijk[0]);
                     selectedVoxelOverlaySecondary.setItemIndex2(ijk[1]);
                     selectedVoxelOverlaySecondary.setItemIndex3(ijk[2]);
                  }
               }
               //
               // Highlight underlay voxel nearest node
               //
               VolumeFile* vf1 = bmv->getUnderlayVolumeFile();
               if (vf1 != NULL) {
                  int ijk[3];
                  if (vf1->convertCoordinatesToVoxelIJK(xyz, ijk)) {
                     selectedVoxelUnderlay.reset(bs,
                                                 bmv,
                                                 viewingWindowNumber);
                     selectedVoxelUnderlay.setItemIndex1(ijk[0]);
                     selectedVoxelUnderlay.setItemIndex2(ijk[1]);
                     selectedVoxelUnderlay.setItemIndex3(ijk[2]);
                  }
               }
               
               //
               // Set slices in "lowest volume the windows selected slices 
               //
               VolumeFile* vfm = bmv->getMasterVolumeFile();
               if (vfm != NULL) {
                  int ijk[3];
                  if (vfm->convertCoordinatesToVoxelIJK(xyz, ijk)) {
                     for (int m = 0; m < BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; m++) {
                        bmv->setSelectedOrthogonalSlices(m, ijk);
                     }
                  }
               }
            }
         }
      }
      
      //
      // Is this volume ?
      //
      if (dynamic_cast<BrainModelVolume*>(bm) != NULL) {
         if (selectedVoxelUnderlay.getItemIndex1() >= 0) {
            BrainModelVolume* bmv = bs->getBrainModelVolume();
            if (bmv != NULL) {
               //
               // Get underlay volume file
               //
               VolumeFile* vf1 = bmv->getUnderlayVolumeFile();
               if (vf1 != NULL) {
                  //
                  // Convert voxel to coordinate
                  //
                  const int ijk[3] = {
                     selectedVoxelUnderlay.getItemIndex1(),
                     selectedVoxelUnderlay.getItemIndex2(),
                     selectedVoxelUnderlay.getItemIndex3()
                  };
                  float xyz[3];
                  vf1->getVoxelCoordinate(ijk, 
                                          xyz);
                  
                  //
                  // Find closest fiducial/volume interaction surface
                  //
                  float fiducialDistance = std::numeric_limits<float>::max();
                  int fiducialNodeNumber = -1;
                  BrainModelSurface* fiducialBMS = NULL;
                  BrainModelSurface* leftBMS = bs->getLeftFiducialVolumeInteractionSurface();
                  if (leftBMS != NULL) {
                     //
                     // Find nearest node 
                     //
                     const CoordinateFile* cf = leftBMS->getCoordinateFile();
                     const int node = cf->getCoordinateIndexClosestToPoint(xyz[0], xyz[1], xyz[2]);
                     if (node >= 0) {
                        const float dist = cf->getDistanceToPoint(node, xyz);
                        if (dist < fiducialDistance) {
                           fiducialDistance = dist;
                           fiducialNodeNumber = node;
                           fiducialBMS = leftBMS;
                        }
                     }
                  }
                  BrainModelSurface* rightBMS = bs->getRightFiducialVolumeInteractionSurface();
                  if (rightBMS != NULL) {
                     //
                     // Find nearest node 
                     //
                     const CoordinateFile* cf = rightBMS->getCoordinateFile();
                     const int node = cf->getCoordinateIndexClosestToPoint(xyz[0], xyz[1], xyz[2]);
                     if (node >= 0) {
                        const float dist = cf->getDistanceToPoint(node, xyz);
                        if (dist < fiducialDistance) {
                           fiducialDistance = dist;
                           fiducialNodeNumber = node;
                           fiducialBMS = rightBMS;
                        }
                     }
                  }
                  BrainModelSurface* cerebellumBMS = bs->getCerebellumFiducialVolumeInteractionSurface();
                  if (cerebellumBMS != NULL) {
                     //
                     // Find nearest node 
                     //
                     const CoordinateFile* cf = cerebellumBMS->getCoordinateFile();
                     const int node = cf->getCoordinateIndexClosestToPoint(xyz[0], xyz[1], xyz[2]);
                     if (node >= 0) {
                        const float dist = cf->getDistanceToPoint(node, xyz);
                        if (dist < fiducialDistance) {
                           fiducialDistance = dist;
                           fiducialNodeNumber = node;
                           fiducialBMS = cerebellumBMS;
                        }
                     }
                  }
                   
                  if (fiducialNodeNumber >= 0) {
                     //
                     // Highlight nearest node
                     //
                     selectedNode.reset(bs,
                                        fiducialBMS,
                                        viewingWindowNumber);
                     selectedNode.setItemIndex1(fiducialNodeNumber);
                  }
               }
            }
         }
      }
      
      //
      // Special processing for CoCoMac
      //
      if (selectionMask & SELECTION_MASK_NODE) {
         const int modelIndex = bm->getBrainModelIndex();
         BrainModelSurfaceNodeColoring* bsnc = brainSet->getNodeColoring();
         DisplaySettingsCoCoMac* dsc = brainSet->getDisplaySettingsCoCoMac();
         if (brainSet->isASurfaceOverlay(modelIndex,
                             BrainModelSurfaceOverlay::OVERLAY_COCOMAC)) {
            if (selectedNode.getItemIndex1() >= 0) {
               dsc->setSelectedNode(selectedNode.getItemIndex1());
            }
            else {
               dsc->setSelectedNode(-1);
            }
            bsnc->assignColors();
         }
         else {
            dsc->setSelectedNode(-1);
         }
      }
      
      //
      // Is this contours ?
      //
      if (dynamic_cast<BrainModelContours*>(bm) != NULL) {
         BrainModelContours* bmc = dynamic_cast<BrainModelContours*>(bm);
         const int contourNumber = selectedContour.getItemIndex1();
         const int contourPointNumber = selectedContour.getItemIndex2();
         if ((contourNumber >= 0) && (contourPointNumber >= 0)) {
            const ContourFile* cf = bmc->getContourFile();
            const CaretContour* cc = cf->getContour(contourNumber);
            float xyz[3];
            cc->getPointXYZ(contourPointNumber, xyz);
            
            //
            // Find fiducial coordinate file
            //
            BrainModelSurface* bms = bs->getActiveFiducialSurface();
            if (bms != NULL) {
               //
               // Find nearest node
               //
               const CoordinateFile* cf = bms->getCoordinateFile();
               const int node = cf->getCoordinateIndexClosestToPoint(xyz[0], xyz[1], xyz[2]);
               
               //
               // Highlight nearest node
               //
               if (node > 0) {
                  selectedNode.reset(bs,
                                     bms,
                                     viewingWindowNumber);
                  selectedNode.setItemIndex1(node);
               }
            }
         }
      }
      
      //
      // If axes selected in a view mode disable any node or voxel identification
      //
      if (selectedTransformationMatrixAxes.getItemIndex1() >= 0) {
         selectedNode.setItemIndex1(-1);
         selectedVoxelUnderlay.setItemIndex1(-1);
         selectedVoxelOverlaySecondary.setItemIndex1(-1);
         selectedVoxelOverlayPrimary.setItemIndex1(-1);
         selectedVoxelFunctionalCloud.setItemIndex1(-1);
      }
   } // if (viewModeFlag)
   
   selectionMask = SELECTION_MASK_OFF;
   brainSet = NULL;
}

/**
 * Called to process hits made while selecting objects with mouse.
 */
void
BrainModelOpenGL::processSelectedItems(const int numItems)
{
   GLuint* ptr = &selectionBuffer[0];

   const float depthScale = static_cast<float>(0x7fffffff);

   if (DebugControl::getDebugOn()) {
      std::cout << "Number of Hits: " << numItems << std::endl;
   }
   
   for (int i = 0; i < numItems; i++) {
      //
      // Number of names associated with this hit
      //
      const int nameStackSize = *ptr;
      ptr++;
      
      if (DebugControl::getDebugOn()) {
         std::cout << "  Hit " << i << " contains " << nameStackSize << " items." << std::endl;
      }
      
      //
      // Depth of this hit
      //
      const float minDepth = static_cast<float>(*ptr) / depthScale;
      ptr++;
      //const float maxDepth = static_cast<float>(*ptr) / depthScale;
      ptr++;  // skip over max depth
      

      unsigned long sm = SELECTION_MASK_OFF;
      int index1 = -1;
      int index2 = -1;
      int index3 = -1;
      int index4 = -1;
      int index5 = -1;
      int index6 = -1;
                      
      for (int j = 0; j < nameStackSize; j++) {
         if (j == 0) {
            sm = static_cast<unsigned long>(*ptr);
            ptr++;            
         }
         else {
            if (j == 1) {
               index1 = *ptr;
            }
            else if (j == 2) {
               index2 = *ptr;
            }
            else if (j == 3) {
               index3 = *ptr;
            }
            else if (j == 4) {
               index4 = *ptr;
            }
            else if (j == 5) {
               index5 = *ptr;
            }
            else if (j == 6) {
               index6 = *ptr;
            }
            ptr++;
         }
      }
      
      if (DebugControl::getDebugOn()) {
         QString name("UNKNOWN");
         switch(sm) {
            case SELECTION_MASK_OFF:
               name = "OFF";
               break;
            case SELECTION_MASK_NODE:
               name = "NODE";
               break;
            case SELECTION_MASK_BORDER:
               name = "BORDER";
               break;
            case SELECTION_MASK_VOLUME_BORDER:
               name = "BORDER-VOLUME";
               break;
            case SELECTION_MASK_CELL_PROJECTION:
               name = "CELL";
               break;
            case SELECTION_MASK_VOLUME_CELL:
               name = "CELL-VOLUME";
               break;
            case SELECTION_MASK_FOCUS_PROJECTION:
               name = "FOCIUS";
               break;
            case SELECTION_MASK_VOLUME_FOCI:
               name = "FOCI-VOLUME";
               break;
            case SELECTION_MASK_PALETTE_METRIC:
               name = "Palette Metric";
               break;
            case SELECTION_MASK_PALETTE_SHAPE:
               name = "Palette Shape";
               break;
            case SELECTION_MASK_CUT:
               name = "CUT";
               break;
            case SELECTION_MASK_CONTOUR:
               name = "CONTOUR";
               break;
            case SELECTION_MASK_CONTOUR_CELL:
               name = "CONTOUR CELL";
               break;
            case SELECTION_MASK_VOXEL_UNDERLAY:
               name = "VOXEL UNDERLAY";
               break;
            case SELECTION_MASK_VOXEL_OVERLAY_SECONDARY:
               name = "VOXEL SECONDARY OVERLAY";
               break;
            case SELECTION_MASK_VOXEL_OVERLAY_PRIMARY:
               name = "VOXEL PRIMARY OVERLAY";
               break;
            case SELECTION_MASK_VOXEL_FUNCTIONAL_CLOUD:
               name = "VOXEL_FUNCTIONAL";
               break;
            case SELECTION_MASK_TILE:
               name = "TILE";
               break;
            case SELECTION_MASK_LINK:
               name = "LINK";
               break;
            case SELECTION_MASK_TRANSFORMATION_MATRIX_AXES:
               name = "TRANSFORMATION MATRIX AXES";
               break;
            case SELECTION_MASK_VTK_MODEL:
               name = "VTK MODEL";
               break;
            case SELECTION_MASK_ALL:
               name = "ALL";
               break;
            case SELECTION_MASK_TRANSFORM_CELL:
               name = "CELL-TRANSFORM";
               break;
            case SELECTION_MASK_TRANSFORM_FOCI:
               name = "FOCI-TRANSFORM";
               break;
            case SELECTION_MASK_TRANSFORM_CONTOUR:
               name = "CONTOUR-TRANSFORM";
               break;
            case SELECTION_MASK_TRANSFORM_CONTOUR_CELL:
               name = "CONTOUR-CELL-TRANSFORM";
               break;
         }
         std::cout << "  name: " << name.toAscii().constData() << " mask: " << sm << " indices: (" 
                   << index1 << ", " << index2 << ", " << index3 << ", " 
                   << index4 << ", " << index5 << ", " << index6 
                   << ")" << std::endl;
      }
      
      if (index1 < 0) {
         continue;
      }
      
      float objectPos[3];
      bool objectPosValid = false;
      
      BrainModelSurface* bms = dynamic_cast<BrainModelSurface*>(brainModel);
      BrainModelSurfaceAndVolume* bmsv = dynamic_cast<BrainModelSurfaceAndVolume*>(brainModel);
      BrainModelContours* bmc = dynamic_cast<BrainModelContours*>(brainModel);
      BrainModelVolume* bmv = dynamic_cast<BrainModelVolume*>(brainModel);
      
      //
      // Check for fiducial surface
      //
      bool fiducialSurfaceFlag = false;
      if (bms != NULL) {
         fiducialSurfaceFlag = ((bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_RAW) ||
                                (bms->getSurfaceType() == BrainModelSurface::SURFACE_TYPE_FIDUCIAL));
      }
      
      switch(sm) {
         case SELECTION_MASK_LINK:
            {
               if (bms != NULL) {
                  const CoordinateFile* cf = bms->getCoordinateFile();
                  if (cf != NULL) {
                     cf->getCoordinate(index1, objectPos);
                     objectPosValid = true;
                  }
               }
               else {
                  if (bmsv != NULL) {
                     const CoordinateFile* cf = bmsv->getCoordinateFile();
                     if (cf != NULL) {
                        cf->getCoordinate(index1, objectPos);
                        objectPosValid = true;
                     }
                  }
               }
            }
            break;
         case SELECTION_MASK_TILE:
            {
               if (bms != NULL) {
                  const TopologyFile* tf = bms->getTopologyFile();
                  if (tf != NULL) {
                     int nodes[3];
                     tf->getTile(index1, nodes);
                     const CoordinateFile* cf = bms->getCoordinateFile();
                     cf->getCoordinate(nodes[0], objectPos);
                     objectPosValid = true;
                  }
               }
               else {
                  if (bmsv != NULL) {
                     const TopologyFile* tf = bmsv->getTopologyFile();
                     if (tf != NULL) {
                        int nodes[3];
                        tf->getTile(index1, nodes);
                        const CoordinateFile* cf = bmsv->getCoordinateFile();
                        cf->getCoordinate(nodes[0], objectPos);
                        objectPosValid = true;
                     }
                  }
               }
            }
            break;
         case SELECTION_MASK_NODE:
            {
               if (bms != NULL) {
                  const CoordinateFile* cf = bms->getCoordinateFile();
                  cf->getCoordinate(index1, objectPos);
                  objectPosValid = true;
               }
               else {
                  if (bmsv != NULL) {
                     const CoordinateFile* cf = bmsv->getCoordinateFile();
                     cf->getCoordinate(index1, objectPos);
                     objectPosValid = true;
                  }
               }
            }
            break;
         case SELECTION_MASK_VOLUME_BORDER:
            if ((index1 >= 0) && (index2 >= 0)) {
               const BorderFile* bf = brainSet->getVolumeBorderFile();
               const Border* b = bf->getBorder(index1);
               b->getLinkXYZ(index2, objectPos);
               objectPosValid = true;
            }
            break;
         case SELECTION_MASK_BORDER:
            if ((index2 >= 0) && (index3 >= 0)) {
               const BrainModelBorderSet* bmbs = brainSet->getBorderSet();
               const BrainModelBorder* b = bmbs->getBorder(index2);
               const BrainModelBorderLink* link = b->getBorderLink(index3);
               link->getLinkPosition(index1, objectPos);
               objectPosValid = true;
            }
            break;
         case SELECTION_MASK_CELL_PROJECTION:
            if (index1 >= 0) {
               CellProjectionFile* cpf = brainSet->getCellProjectionFile();
               if (index1 < cpf->getNumberOfCellProjections()) {
                  if (bms != NULL) {
                     CellProjection* cp = cpf->getCellProjection(index1);
                     objectPosValid = cp->getProjectedPosition(bms->getCoordinateFile(),
                                                               bms->getTopologyFile(),
                                                               fiducialSurfaceFlag,
                                                               false,
                                                               false,
                                                               objectPos);
                     
                  }
               }
            }
            break;
         case SELECTION_MASK_VOLUME_CELL:
            if (index1 >= 0) {
               CellFile* cf = brainSet->getVolumeCellFile();
               CellData* cell = cf->getCell(index1);
               cell->getXYZ(objectPos);
               objectPosValid = true;
            }
            break;
         case SELECTION_MASK_CUT:
            if (index1 >= 0) {
               BorderFile* bf = brainSet->getCutsFile();
               Border* b = bf->getBorder(index1);
               b->getLinkXYZ(index2, objectPos);
               objectPosValid = true;
            }
            break;
         case SELECTION_MASK_FOCUS_PROJECTION:
            if (index1 >= 0) {
               FociProjectionFile* fpf = brainSet->getFociProjectionFile();
               if (index1 < fpf->getNumberOfCellProjections()) {
                  if (bms != NULL) {
                     CellProjection* cp = fpf->getCellProjection(index1);
                     objectPosValid = cp->getProjectedPosition(bms->getCoordinateFile(),
                                                               bms->getTopologyFile(),
                                                               fiducialSurfaceFlag,
                                                               false,
                                                               false,
                                                               objectPos);
                     
                  }
               }
            }
            break;
         case SELECTION_MASK_VOLUME_FOCI:
            if (index1 >= 0) {
               FociProjectionFile* fpf = brainSet->getFociProjectionFile();
               CellProjection* focus = fpf->getCellProjection(index1);
               focus->getVolumeXYZ(objectPos);
               objectPosValid = true;
            }
            break;
         case SELECTION_MASK_TRANSFORM_FOCI:
            if ((index1 >= 0) && (index2 >= 0)) {
               FociFile* cf = dynamic_cast<FociFile*>(brainSet->getTransformationDataFile(index1));
               if (cf != NULL) {
                  CellData* cell = cf->getCell(index2);
                  cell->getXYZ(objectPos);
                  objectPosValid = true;
               }
            }
            break;
         case SELECTION_MASK_TRANSFORM_CELL:
            if ((index1 >= 0) && (index2 >= 0)) {
               CellFile* cf = dynamic_cast<CellFile*>(brainSet->getTransformationDataFile(index1));
               if (cf != NULL) {
                  CellData* cell = cf->getCell(index2);
                  cell->getXYZ(objectPos);
                  objectPosValid = true;
               }
            }
            break;
         case SELECTION_MASK_TRANSFORM_CONTOUR:
            if ((index1 >= 0) && (index2 >= 0)) {
               ContourFile* cf = dynamic_cast<ContourFile*>(brainSet->getTransformationDataFile(index1));
               if (cf != NULL) {
                  CaretContour* cc = cf->getContour(index2);
                  cc->getPointXY(index2, objectPos[0], objectPos[1]);
                  objectPos[2] = cc->getSectionNumber() * cf->getSectionSpacing();
                  objectPosValid = true;
               }
            }
            break;
         case SELECTION_MASK_TRANSFORM_CONTOUR_CELL:
            if ((index1 >= 0) && (index2 >= 0)) {
               ContourCellFile* cf = dynamic_cast<ContourCellFile*>(brainSet->getTransformationDataFile(index1));
               if (cf != NULL) {
                  CellData* cell = cf->getCell(index2);
                  cell->getXYZ(objectPos);
                  objectPosValid = true;
               }
            }
            break;
         case SELECTION_MASK_PALETTE_METRIC:
            selectedPaletteMetric.replaceIfCloser(0, 0, 
                                            BrainModelOpenGLSelectedItem::ITEM_TYPE_PALETTE_METRIC,
                                            index1, index2);
            break;
         case SELECTION_MASK_PALETTE_SHAPE:
            selectedPaletteShape.replaceIfCloser(0, 0, 
                                            BrainModelOpenGLSelectedItem::ITEM_TYPE_PALETTE_SHAPE,
                                            index1, index2);
            break;
         case SELECTION_MASK_CONTOUR:
            if (index2 >= 0) {
               if (bmc != NULL) {
                  ContourFile* cf = bmc->getContourFile();
                  CaretContour* cc = cf->getContour(index1);
                  cc->getPointXY(index2, objectPos[0], objectPos[1]);
                  objectPos[2] = cc->getSectionNumber() * cf->getSectionSpacing();
                  objectPosValid = true;
               }
            }
            break;
         case SELECTION_MASK_CONTOUR_CELL:
            if (index1 >= 0) {
               if (bmc != NULL) {
                  ContourFile* contourFile = bmc->getContourFile();
                  CellFile* cf = brainSet->getContourCellFile();
                  if (cf != NULL) {
                     CellData* cd = cf->getCell(index1);
                     cd->getXYZ(objectPos);
                     objectPos[2] = cd->getSectionNumber() * contourFile->getSectionSpacing();
                     objectPosValid = true;
                  }
               }
            }
            break;
         case SELECTION_MASK_VOXEL_UNDERLAY:
            if ((index1 >= 0) && (index2 >= 0) && (index3 >= 0)) {
               if (bmv != NULL) {
                  VolumeFile* vf = bmv->getUnderlayVolumeFile();
                  if (vf != NULL) {
                     float spacing[3];
                     float origin[3];
                     vf->getOrigin(origin);
                     vf->getSpacing(spacing);
                     objectPos[0] = spacing[0]*index1 + spacing[0]*0.5 + origin[0];
                     objectPos[1] = spacing[1]*index2 + spacing[1]*0.5 + origin[1];
                     objectPos[2] = spacing[2]*index3 + spacing[2]*0.5 + origin[2];
                     objectPosValid = true;
                  }
               }
               else {
                  if (bmsv != NULL) {
                     VolumeFile* vf = bmsv->getAnatomyVolumeFile();
                     if (vf != NULL) {
                        float spacing[3];
                        float origin[3];
                        vf->getOrigin(origin);
                        vf->getSpacing(spacing);
                        objectPos[0] = spacing[0]*index1 + spacing[0]*0.5 + origin[0];
                        objectPos[1] = spacing[1]*index2 + spacing[1]*0.5 + origin[1];
                        objectPos[2] = spacing[2]*index3 + spacing[2]*0.5 + origin[2];
                        objectPosValid = true;
                     }
                  }
               }
            }
            break;
         case SELECTION_MASK_VOXEL_OVERLAY_SECONDARY:
            if ((index1 >= 0) && (index2 >= 0) && (index3 >= 0)) {
               if (bmv != NULL) {
                  VolumeFile* vf = bmv->getOverlaySecondaryVolumeFile();
                  if (vf != NULL) {
                     float spacing[3];
                     float origin[3];
                     vf->getOrigin(origin);
                     vf->getSpacing(spacing);
                     objectPos[0] = spacing[0]*index1 + spacing[0]*0.5 + origin[0];
                     objectPos[1] = spacing[1]*index2 + spacing[1]*0.5 + origin[1];
                     objectPos[2] = spacing[2]*index3 + spacing[2]*0.5 + origin[2];
                     objectPosValid = true;
                  }
               }
               else {
                  if (bmsv != NULL) {
                     VolumeFile* vf = bmsv->getOverlaySecondaryVolumeFile();
                     if (vf != NULL) {
                        float spacing[3];
                        float origin[3];
                        vf->getOrigin(origin);
                        vf->getSpacing(spacing);
                        objectPos[0] = spacing[0]*index1 + spacing[0]*0.5 + origin[0];
                        objectPos[1] = spacing[1]*index2 + spacing[1]*0.5 + origin[1];
                        objectPos[2] = spacing[2]*index3 + spacing[2]*0.5 + origin[2];
                        objectPosValid = true;
                     }
                  }
               }
            }
            break;
         case SELECTION_MASK_VOXEL_OVERLAY_PRIMARY:
            if ((index1 >= 0) && (index2 >= 0) && (index3 >= 0)) {
               if (bmv != NULL) {
                  VolumeFile* vf = bmv->getOverlayPrimaryVolumeFile();
                  if (vf != NULL) {
                     float spacing[3];
                     float origin[3];
                     vf->getOrigin(origin);
                     vf->getSpacing(spacing);
                     objectPos[0] = spacing[0]*index1 + spacing[0]*0.5 + origin[0];
                     objectPos[1] = spacing[1]*index2 + spacing[1]*0.5 + origin[1];
                     objectPos[2] = spacing[2]*index3 + spacing[2]*0.5 + origin[2];
                     objectPosValid = true;
                  }
               }
               else {
                  if (bmsv != NULL) {
                     VolumeFile* vf = bmsv->getOverlayPrimaryVolumeFile();
                     if (vf != NULL) {
                        float spacing[3];
                        float origin[3];
                        vf->getOrigin(origin);
                        vf->getSpacing(spacing);
                        objectPos[0] = spacing[0]*index1 + spacing[0]*0.5 + origin[0];
                        objectPos[1] = spacing[1]*index2 + spacing[1]*0.5 + origin[1];
                        objectPos[2] = spacing[2]*index3 + spacing[2]*0.5 + origin[2];
                        objectPosValid = true;
                     }
                  }
               }
            }
            break;
         case SELECTION_MASK_VOXEL_FUNCTIONAL_CLOUD:
            {
               if (bmsv != NULL) {
                  VolumeFile* vf = bmsv->getFunctionalVolumeFile();
                  if (vf != NULL) {
                     float spacing[3];
                     float origin[3];
                     vf->getOrigin(origin);
                     vf->getSpacing(spacing);
                     objectPos[0] = spacing[0]*index1 + spacing[0]*0.5 + origin[0];
                     objectPos[1] = spacing[1]*index2 + spacing[1]*0.5 + origin[1];
                     objectPos[2] = spacing[2]*index3 + spacing[2]*0.5 + origin[2];
                     objectPosValid = true;
                  }
               }
            }
            break;
         case SELECTION_MASK_TRANSFORMATION_MATRIX_AXES:
            {
               TransformationMatrixFile* tmf = brainSet->getTransformationMatrixFile();
               if ((index1 >= 0) && (index1 < tmf->getNumberOfMatrices())) {
                  TransformationMatrix* tm = tmf->getTransformationMatrix(index1);
                  tm->getTranslation(objectPos[0], objectPos[1], objectPos[2]);
                  objectPosValid = true;
               }
            }
            break;
         case SELECTION_MASK_VTK_MODEL:
            {
               if ((index1 >= 0) && (index1 < brainSet->getNumberOfVtkModelFiles())) {
                  VtkModelFile* vmf = brainSet->getVtkModelFile(index1);
                  vmf->getTriangleCoordinate(index3, objectPos);
                  
                  const TransformationMatrixFile* tmf = brainSet->getTransformationMatrixFile();
                  const TransformationMatrix* tm = vmf->getAssociatedTransformationMatrix();
                  if (tmf->getMatrixValid(tm)) {
                     //tm->multiplyPoint(objectPos);
                  }
      
                  objectPosValid = true;
               }
            }
            break;
         case SELECTION_MASK_OFF:
         case SELECTION_MASK_ALL:
         default:
            break;
      }
      
      if (objectPosValid) {
         GLdouble windowPos[3];
         if (gluProject(objectPos[0], objectPos[1], objectPos[2],
                        selectionModelviewMatrix[viewingWindowNumber],
                        selectionProjectionMatrix[viewingWindowNumber], 
                        selectionViewport[viewingWindowNumber],
                        &windowPos[0], &windowPos[1], &windowPos[2]) == GL_TRUE) {
            const double dx = windowPos[0] - selectionX;
            const double dy = windowPos[1] - selectionY;
            const double dist = std::sqrt(dx*dx + dy*dy);
            
            if (DebugControl::getDebugOn()) {
               std::cout << "   minDepth=" << minDepth
                         << ", dist=" << dist
                         << std::endl;
            }
            switch(sm) {
               case SELECTION_MASK_TILE:
                  selectedSurfaceTile.replaceIfCloser(minDepth, dist,
                                               BrainModelOpenGLSelectedItem::ITEM_TYPE_TILE,
                                                index1, index2);
                  break;
               case SELECTION_MASK_NODE:
                  selectedNode.replaceIfCloser(minDepth, dist, 
                                               BrainModelOpenGLSelectedItem::ITEM_TYPE_NODE,
                                               index1, index2);
                  break;
               case SELECTION_MASK_LINK:
                   selectedLink.replaceIfCloser(minDepth, dist,
                                                BrainModelOpenGLSelectedItem::ITEM_TYPE_LINK,
                                                index1, index2);
                  break;
               case SELECTION_MASK_VOLUME_BORDER:
                  selectedVolumeBorder.replaceIfCloser(minDepth, dist,
                                                       BrainModelOpenGLSelectedItem::ITEM_TYPE_VOLUME_BORDER,
                                                       index1, index2);
                  break;
               case SELECTION_MASK_BORDER:
                  {
                     const BrainModelOpenGLSelectedItem::ITEM_TYPE itemType = 
                              BrainModelOpenGLSelectedItem::ITEM_TYPE_BORDER;
                     //
                     // Two borders can be highlighted.
                     //
                     const BrainModelOpenGLSelectedItem savedBorder1 = selectedBorder1;
                     if (selectedBorder1.replaceIfCloser(minDepth, dist, 
                                                 itemType,
                                                 index1, index2, index3)) {
                        //
                        // Do not let same border in both border selections
                        // index1 is border file number, index2 is border number
                        //
                        if (savedBorder1.getItemIndex1() != index1) {
                           selectedBorder2 = savedBorder1;
                        }
                        else if (savedBorder1.getItemIndex2() != index2) {
                           selectedBorder2 = savedBorder1;
                        }
                        //
                        // If new border 1 selection is same border as in border 2
                        //
                        else if (index1 == selectedBorder2.getItemIndex1()) {
                           selectedBorder2 = savedBorder1;
                        }
                     }
                     else {
                        //
                        // Do not allow same border for both border selections
                        // index1 is border file number, index2 is border number
                        //
                        if (selectedBorder1.getItemIndex1() != index1) {
                           selectedBorder2.replaceIfCloser(minDepth, dist, 
                                                   itemType,
                                                   index1, index2, index3);
                        }
                        else if (selectedBorder1.getItemIndex2() != index2) {
                           selectedBorder2.replaceIfCloser(minDepth, dist, 
                                                   itemType,
                                                   index1, index2, index3);
                        }
                     }
                  }
                  break;
               case SELECTION_MASK_CELL_PROJECTION:
                  selectedCellProjection.replaceIfCloser(minDepth, dist, 
                                               BrainModelOpenGLSelectedItem::ITEM_TYPE_CELL_PROJECTION,
                                               index1);
                  break;
               case SELECTION_MASK_VOLUME_CELL:
                  selectedVolumeCell.replaceIfCloser(minDepth, dist, 
                                               BrainModelOpenGLSelectedItem::ITEM_TYPE_VOLUME_CELL,
                                               index1);
                  break;
               case SELECTION_MASK_CUT:
                  selectedCut.replaceIfCloser(minDepth, dist,
                                               BrainModelOpenGLSelectedItem::ITEM_TYPE_CUT,
                                               index1, index2);
                  break;
               case SELECTION_MASK_FOCUS_PROJECTION:
                  selectedFocusProjection.replaceIfCloser(minDepth, dist, 
                                               BrainModelOpenGLSelectedItem::ITEM_TYPE_FOCUS_PROJECTION,
                                               index1);
                  break;
               case SELECTION_MASK_VOLUME_FOCI:
                  selectedVolumeFoci.replaceIfCloser(minDepth, dist, 
                                               BrainModelOpenGLSelectedItem::ITEM_TYPE_VOLUME_FOCI,
                                               index1);
                  break;
               case SELECTION_MASK_TRANSFORM_CELL:
                  selectedTransformCell.replaceIfCloser(minDepth, dist,
                                               BrainModelOpenGLSelectedItem::ITEM_TYPE_TRANSFORM_CELL,
                                               index1, index2);
                  break;
               case SELECTION_MASK_TRANSFORM_FOCI:
                  selectedTransformFoci.replaceIfCloser(minDepth, dist,
                                               BrainModelOpenGLSelectedItem::ITEM_TYPE_TRANSFORM_FOCI,
                                               index1, index2);
                  break;
               case SELECTION_MASK_TRANSFORM_CONTOUR:
                  selectedTransformContour.replaceIfCloser(minDepth, dist,
                                               BrainModelOpenGLSelectedItem::ITEM_TYPE_TRANSFORM_CONTOUR,
                                               index1, index2);
                  break;
               case SELECTION_MASK_TRANSFORM_CONTOUR_CELL:
                  selectedTransformContourCell.replaceIfCloser(minDepth, dist,
                                               BrainModelOpenGLSelectedItem::ITEM_TYPE_TRANSFORM_CONTOUR_CELL,
                                               index1, index2);
               case SELECTION_MASK_PALETTE_METRIC:
                  break;
               case SELECTION_MASK_PALETTE_SHAPE:
                  break;
               case SELECTION_MASK_CONTOUR:
                  selectedContour.replaceIfCloser(minDepth, dist,
                                                  BrainModelOpenGLSelectedItem::ITEM_TYPE_CONTOUR,
                                                  index1, index2, index3);
                  break;
               case SELECTION_MASK_CONTOUR_CELL:
                  selectedContourCell.replaceIfCloser(minDepth, dist,
                                                      BrainModelOpenGLSelectedItem::ITEM_TYPE_CONTOUR_CELL,
                                                      index1);
                  break;
               case SELECTION_MASK_VOXEL_UNDERLAY:
                  selectedVoxelUnderlay.replaceIfCloser(minDepth, dist,
                                             BrainModelOpenGLSelectedItem::ITEM_TYPE_VOXEL_UNDERLAY,
                                             index1, index2, index3, index4, index5, index6);
                  break;
               case SELECTION_MASK_VOXEL_OVERLAY_SECONDARY:
                  selectedVoxelOverlaySecondary.replaceIfCloser(minDepth, dist,
                                    BrainModelOpenGLSelectedItem::ITEM_TYPE_VOXEL_OVERLAY_SECONDARY,
                                    index1, index2, index3, index4, index5, index6);
                  break;
               case SELECTION_MASK_VOXEL_OVERLAY_PRIMARY:
                  selectedVoxelOverlayPrimary.replaceIfCloser(minDepth, dist,
                                    BrainModelOpenGLSelectedItem::ITEM_TYPE_VOXEL_OVERLAY_PRIMARY,
                                    index1, index2, index3, index4, index5, index6);
                  break;
               case SELECTION_MASK_VOXEL_FUNCTIONAL_CLOUD:
                  selectedVoxelFunctionalCloud.replaceIfCloser(minDepth, dist,
                                    BrainModelOpenGLSelectedItem::ITEM_TYPE_VOXEL_FUNCTIONAL_CLOUD,
                                    index1, index2, index3);
                  break;
               case SELECTION_MASK_TRANSFORMATION_MATRIX_AXES:
                  selectedTransformationMatrixAxes.replaceIfCloser(minDepth, dist,
                                    BrainModelOpenGLSelectedItem::ITEM_TYPE_TRANSFORMATION_MATRIX_AXES,
                                    index1);
                  break;
               case SELECTION_MASK_VTK_MODEL:
                  selectedVtkModel.replaceIfCloser(minDepth, dist,
                                    BrainModelOpenGLSelectedItem::ITEM_TYPE_VTK_MODEL,
                                    index1, index2, index3);
                  break;
               case SELECTION_MASK_OFF:
               case SELECTION_MASK_ALL:
               default:
                  break;
            }
         }
      }
      
   }
}

/**
 * Get the dimensions of the surface orthographic projection
 */ 
void
BrainModelOpenGL::getOrthographicBox(const int modelViewNumber,
                                     double& orthoLeft,   double& orthoRight,
                                     double& orthoBottom, double& orthoTop,
                                     double& orthoNear,   double& orthoFar) const
{
   orthoLeft   = orthographicLeft[modelViewNumber];
   orthoRight  = orthographicRight[modelViewNumber];
   orthoBottom = orthographicBottom[modelViewNumber];
   orthoTop    = orthographicTop[modelViewNumber];
   orthoNear   = orthographicNear[modelViewNumber];
   orthoFar    = orthographicFar[modelViewNumber];
}

/**
 * set the image subregion box/display.
 */
void 
BrainModelOpenGL::setImageSubRegion(const int box[4], const bool showFlag)
{
   imageSubRegionBox[0] = box[0];
   imageSubRegionBox[1] = box[1];
   imageSubRegionBox[2] = box[2];
   imageSubRegionBox[3] = box[3];
   
   drawImageSubRegionBoxFlag = showFlag;
}

/**
 * get valid line width.
 */
GLfloat 
BrainModelOpenGL::getValidLineWidth(const float widthIn) const
{
   GLfloat width = widthIn;

   if (width > maximumLineWidth) {
      width = maximumLineWidth;
   }
   else if (width < minimumLineWidth) {
      width = minimumLineWidth;
   }
   
   return width;
}

/**
 * get valid point size.
 */
GLfloat 
BrainModelOpenGL::getValidPointSize(const float pointSizeIn) const
{
   GLfloat pointSize = pointSizeIn;

   if (pointSize > maximumPointSize) {
      pointSize = maximumPointSize;
   }
   else if (pointSize < minimumPointSize) {
      pointSize = minimumPointSize;
   }
   
   return pointSize;
}
      
/**
 *	Generate a circle in xy plane.
 */

void generateCircle(double center[3], double r, int N, double *pts, double *norms, bool bOutward)
{
	int i, j;
	double alpha = 2*vtkMath::Pi()/N;
	double c, s, a;
	for (i=0; i<N; i++)
	{
		j = i*3;			// index into pts and norms
		a = i*alpha;
		c = cos(a);
		s = sin(a);

		pts[j] = center[0] + r*c;
		pts[j+1] = center[1] + r*s;
		pts[j+2] = center[2];

		if (bOutward)
		{
			norms[j] = c;
			norms[j+1] = s;
			norms[j+2] = 0;
		}
		else
		{
			norms[j] = -c;
			norms[j+1] = -s;
			norms[j+2] = 0;
		}
	}
}

/**
 *	Find intersection between line and plane
 */

void intersectLinePlane(double p0[3], double n[3], double p1[3], double p2[3], double p[3])
{
	double a[3], b[3], da, db, t;
	int i;
	for (i=0; i<3; i++) a[i] = p2[i] - p1[i];
	for (i=0; i<3; i++) b[i] = p0[i] - p1[i];
	da = vtkMath::Dot(a, n);
	db = vtkMath::Dot(b, n);
	t = db/da;
	for (i=0; i<3; i++) p[i] = p1[i] + t * a[i];
}

void dumpvec2(std::string s, double d[3])
{
	std::cout << s << " " << d[0] << ", " << d[1] << ", " << d[2] << std::endl;
}
