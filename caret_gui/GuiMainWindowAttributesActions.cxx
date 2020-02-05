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

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QLineEdit>
#include <QMessageBox>

#include "AreaColorFile.h"
#include "BorderToTopographyConverter.h"
#include "BrainModelBorderSet.h"
#include "BrainModelSurfaceToVolumeConverter.h"
#include "BrainModelSurfaceNodeColoring.h"
#include "BrainSet.h"
#include "DisplaySettingsProbabilisticAtlas.h"
#include "GuiBrainModelOpenGL.h"
#include "GuiChooseNodeAttributeColumnDialog.h"
#include "GuiColorFileEditorDialog.h"
#include "GuiCurrentColoringToRgbPaintDialog.h"
#include "GuiDeformationFieldDialog.h"
#include "GuiGenerateArealEstimationDialog.h"
#include "GuiFilesModified.h"
#include "GuiMainWindow.h"
#include "GuiMainWindowAttributesActions.h"
#include "GuiMapFmriDialog.h"
#include "GuiChooseNodeAttributeColumnDialog.h"
#include "GuiNodeAttributeColumnSelectionComboBox.h"
#include "GuiNodeAttributeFileClearResetDialog.h"
#include "GuiPaintNameEditorDialog.h"
#include "GuiSurfaceToVolumeDialog.h"
#include "GuiShapeOrVectorsFromCoordinateSubtractionDialog.h"
#include "LatLonFile.h"
#include "MetricFile.h"
#include "PaintFile.h"
#include "ProbabilisticAtlasFile.h"
#include "QtTableDialog.h"
#include "RgbPaintFile.h"
#include "StudyMetaDataFile.h"
#include "SurfaceShapeFile.h"
#include "VectorFile.h"
#include "VocabularyFile.h"
#include "WuQDataEntryDialog.h"
#include "global_variables.h"

/**
 * Constructor.
 */
GuiMainWindowAttributesActions::GuiMainWindowAttributesActions(GuiMainWindow* parent)
   : QObject(parent)
{
   setObjectName("GuiMainWindowAttributesActions");
   
   generateTopographyAction = new QAction(parent);
   generateTopographyAction->setText("Generate Topography From Borders and Paint");
   //generateTopographyAction->setName("generateTopographyAction");
   QObject::connect(generateTopographyAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotGenerateTopography()));

   generateArealEstimationMapAction = new QAction(parent);
   generateArealEstimationMapAction->setText("Generate Areal Estimation Map...");
   //generateArealEstimationMapAction->setName("generateArealEstimationMapAction");
   QObject::connect(generateArealEstimationMapAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotGenerateArealEstimationMap()));

   displayParamsFileEditorAction = new QAction(parent);
   displayParamsFileEditorAction->setText("Edit...");
   //displayParamsFileEditorAction->setName("displayParamsFileEditorAction");
   QObject::connect(displayParamsFileEditorAction, SIGNAL(triggered(bool)),
                    parent, SLOT(displayParamsFileEditorDialog()));

   displayPaletteEditorAction = new QAction(parent);
   displayPaletteEditorAction->setText("Edit...");
   //displayPaletteEditorAction->setName("displayPaletteEditorAction");
   QObject::connect(displayPaletteEditorAction, SIGNAL(triggered(bool)),
                    parent, SLOT(displayPaletteEditorDialog()));

   areaColorsEditAction = new QAction(parent);
   areaColorsEditAction->setText("Edit...");
   //areaColorsEditAction->setName("areaColorsEditAction");
   QObject::connect(areaColorsEditAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotAreaColorsEdit()));

   metricMathAction = new QAction(parent);
   metricMathAction->setText("Mathematical Operations...");
   //metricMathAction->setName("metricMathAction");
   QObject::connect(metricMathAction, SIGNAL(triggered(bool)),
                    parent, SLOT(displayMetricMathDialog()));

   metricToRgbPaintAction = new QAction(parent);
   metricToRgbPaintAction->setText("Convert Metric to RGB Paint...");
   //metricToRgbPaintAction->setName("metricToRgbPaintAction");
   QObject::connect(metricToRgbPaintAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotMetricToRgbPaint()));

   metricAverageDeviationAction = new QAction(parent);
   metricAverageDeviationAction->setText("Compute Average and Sample Standard Deviation of All Columns...");
   //metricAverageDeviationAction->setName("metricAverageDeviationAction");
   QObject::connect(metricAverageDeviationAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotMetricAverageDeviation()));

   metricToVolumeAction = new QAction(parent);
   metricToVolumeAction->setText("Convert Metric Column to Functional Volume...");
   //metricToVolumeAction->setName("metricToVolumeAction");
   QObject::connect(metricToVolumeAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotMetricToVolume()));

   metricClearAllOrPartAction = new QAction(parent);
   metricClearAllOrPartAction->setText("Clear All or Part of Metric File...");
   //metricClearAllOrPartAction->setName("metricClearAllOrPartAction");
   QObject::connect(metricClearAllOrPartAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotMetricClearAllOrPart()));

   volumeToSurfaceMapperAction = new QAction(parent);
   volumeToSurfaceMapperAction->setText("Map Volume(s) to Surface(s)...");
   //volumeToSurfaceMapperAction->setName("volumeToSurfaceMapperAction");
   QObject::connect(volumeToSurfaceMapperAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotVolumeToSurfaceMapper()));

   metricModificationAction = new QAction(parent);
   metricModificationAction->setText("Clustering and Smoothing...");
   //metricModificationAction->setName("metricModificationAction");
   QObject::connect(metricModificationAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotMetricModification()));

   surfaceShapeToVolumeAction = new QAction(parent);
   surfaceShapeToVolumeAction->setText("Convert Surface Shape Column to Functional Volume...");
   //surfaceShapeToVolumeAction->setName("surfaceShapeToVolumeAction");
   QObject::connect(surfaceShapeToVolumeAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotSurfaceShapeToVolume()));

   surfaceShapeClearAllOrPartAction = new QAction(parent);
   surfaceShapeClearAllOrPartAction->setText("Clear All or Part of Surface Shape File...");
   //surfaceShapeClearAllOrPartAction->setName("surfaceShapeClearAllOrPartAction");
   QObject::connect(surfaceShapeClearAllOrPartAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotSurfaceShapeClearAllOrPart()));

   surfaceShapeAverageDeviationAction = new QAction(parent);
   surfaceShapeAverageDeviationAction->setText("Compute Average and Sample Standard Deviation of All Columns...");
   //surfaceShapeAverageDeviationAction->setName("surfaceShapeAverageDeviationAction");
   QObject::connect(surfaceShapeAverageDeviationAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotSurfaceShapeAverageDeviation()));

   shapeMathAction = new QAction(parent);
   shapeMathAction->setText("Mathematical Operations...");
   //shapeMathAction->setName("shapeMathAction");
   QObject::connect(shapeMathAction, SIGNAL(triggered(bool)),
                    parent, SLOT(displayShapeMathDialog()));

   shapeModificationAction = new QAction(parent);
   shapeModificationAction->setText("Clustering and Smoothing...");
   //shapeModificationAction->setName("shapeModificationAction");
   QObject::connect(shapeModificationAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotShapeModification()));

   paintCleanNamesAction = new QAction(parent);
   paintCleanNamesAction->setText("Cleanup Paint Names");
   //paintCleanNamesAction->setName("paintCleanNamesAction");
   QObject::connect(paintCleanNamesAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotPaintCleanNames()));

   generateColorsForPaints = new QAction(parent);
   generateColorsForPaints->setText("Generate Colors for Paints Without Colors");
   QObject::connect(generateColorsForPaints, SIGNAL(triggered(bool)),
                    this, SLOT(slotGenerateColorsForPaints()));
                    
   paintToVolumeAction = new QAction(parent);
   paintToVolumeAction->setText("Convert Paint Column to Paint Volume...");
   //paintToVolumeAction->setName("paintToVolumeAction");
   QObject::connect(paintToVolumeAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotPaintToVolume()));

   paintNamesEditAction = new QAction(parent);
   paintNamesEditAction->setText("Edit Paint Names and Properties...");
   //paintNamesEditAction->setName("paintNamesEditAction");
   QObject::connect(paintNamesEditAction, SIGNAL(triggered(bool)),
                    parent, SLOT(displayPaintEditorDialog()));

   paintAssignWithinDisplayedBordersAction = new QAction(parent);
   paintAssignWithinDisplayedBordersAction->setText("Assign Nodes Within Displayed Borders...");
   //paintAssignWithinDisplayedBordersAction->setName("paintAssignWithinDisplayedBordersAction");
   QObject::connect(paintAssignWithinDisplayedBordersAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotPaintAssignWithinDisplayedBorders()));

   paintClearAllOrPartAction = new QAction(parent);
   paintClearAllOrPartAction->setText("Clear All or Part of Paint File...");
   //paintClearAllOrPartAction->setName("paintClearAllOrPartAction");
   QObject::connect(paintClearAllOrPartAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotPaintClearAllOrPart()));

   arealEstimationClearAllOrPartAction = new QAction(parent);
   arealEstimationClearAllOrPartAction->setText("Clear All or Part of Areal Estimation File...");
   QObject::connect(arealEstimationClearAllOrPartAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotArealEstimationClearAllOrPart()));
                    
   copyColoringToRgbPaintAction = new QAction(parent);
   copyColoringToRgbPaintAction->setText("Copy Current Coloring to RGB Paint...");
   //copyColoringToRgbPaintAction->setName("copyColoringToRgbPaintAction");
   QObject::connect(copyColoringToRgbPaintAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotCopyColoringToRgbPaint()));

   copyNormalsToRgbPaintAction = new QAction(parent);
   copyNormalsToRgbPaintAction->setText("Copy Surface Normals to RGB Paint");
   //copyNormalsToRgbPaintAction->setName("copyNormalsToRgbPaintAction");
   QObject::connect(copyNormalsToRgbPaintAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotCopyNormalsToRgbPaint()));

   copyColoringToVolumeAction = new QAction(parent);
   copyColoringToVolumeAction->setText("Copy Current Coloring to RGB Volume...");
   //copyColoringToVolumeAction->setName("copyColoringToVolumeAction");
   QObject::connect(copyColoringToVolumeAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotCopyColoringToVolume()));

   latLonClearAllOrPartAction = new QAction(parent);
   latLonClearAllOrPartAction->setText("Clear All or Part of Lat/Long File...");
   //latLonClearAllOrPartAction->setName("latLonClearAllOrPartAction");
   QObject::connect(latLonClearAllOrPartAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotLatLonClearAllOrPart()));

   latLonGenerateOnSphereAction = new QAction(parent);
   latLonGenerateOnSphereAction->setText("Generate Lat/Long on Spherical Surface...");
   //latLonGenerateOnSphereAction->setName("latLonGenerateOnSphereAction");
   QObject::connect(latLonGenerateOnSphereAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotLatLonGenerateOnSphere()));

   generateDeformationFieldAction = new QAction(parent);
   generateDeformationFieldAction->setText("Generate Deformation Field...");
   //generateDeformationFieldAction->setName("generateDeformationFieldAction");
   QObject::connect(generateDeformationFieldAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotGenerateDeformationField()));

   probAtlasThresholdToPaintAction = new QAction(parent);
   probAtlasThresholdToPaintAction->setText("Convert Threshold Coloring to Paint...");
   //probAtlasThresholdToPaintAction->setName("probAtlasThresholdToPaintAction");
   QObject::connect(probAtlasThresholdToPaintAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotProbAtlasThresholdToPaint()));

   coordsToVectorsAction = new QAction(parent);
   coordsToVectorsAction->setText("Create Vectors From Surface Subtraction...");
   //coordsToVectorsAction->setName("coordsToVectorsAction");
   QObject::connect(coordsToVectorsAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotCoordsToVectors()));

   normalsToVectorsAction = new QAction(parent);
   normalsToVectorsAction->setText("Create Vectors From Surface Normals");
   QObject::connect(normalsToVectorsAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotNormalsToVectors()));
                    
   modelsEditorAction = new QAction(parent);
   modelsEditorAction->setText("Edit...");
   //modelsEditorAction->setName("modelsEditorAction");
   QObject::connect(modelsEditorAction, SIGNAL(triggered(bool)),
                    parent, SLOT(displayModelsEditorDialog()));
         
   studyMetaDataEditorDialogAction = new QAction(parent);
   studyMetaDataEditorDialogAction->setText("Edit Studies...");
   QObject::connect(studyMetaDataEditorDialogAction, SIGNAL(triggered(bool)),
                    parent, SLOT(displayStudyMetaDataFileEditorDialog()));
                    
   studyCollectionEditorDialogAction = new QAction(parent);
   studyCollectionEditorDialogAction->setText("Edit Collections...");
   QObject::connect(studyCollectionEditorDialogAction, SIGNAL(triggered(bool)),
                    parent, SLOT(displayStudyCollectionFileEditorDialog()));
                    
   vocabularyFileEditorDialogAction = new QAction(parent);
   vocabularyFileEditorDialogAction->setText("Edit...");
   QObject::connect(vocabularyFileEditorDialogAction, SIGNAL(triggered(bool)),
                    parent, SLOT(displayVocabularyFileEditorDialog()));
                    
   vocabularyMoveStudyInfoToStudyMetaDataAction = new QAction(parent);
   vocabularyMoveStudyInfoToStudyMetaDataAction->setText("Move Vocabulary Study Info to Study Metadata File");
   QObject::connect(vocabularyMoveStudyInfoToStudyMetaDataAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotVocabularyMoveStudyInfoToStudyMetaData()));

   connectivityAction = new QAction(parent);
   connectivityAction->setText("Connectivity...");
   //connectivityAction->setShortcut(Qt::CTRL+Qt::Key_L);
   QObject::connect(connectivityAction, SIGNAL(triggered(bool)),
                    parent, SLOT(displayConnectivityDialog()));
}

/**
 * Destructor.
 */
GuiMainWindowAttributesActions::~GuiMainWindowAttributesActions()
{
}

/**
 * convert vocabulary study info to study metadata file.
 */
void 
GuiMainWindowAttributesActions::slotVocabularyMoveStudyInfoToStudyMetaData()
{
   StudyMetaDataFile* smdf = theMainWindow->getBrainSet()->getStudyMetaDataFile();
   VocabularyFile* vf = theMainWindow->getBrainSet()->getVocabularyFile();
   smdf->append(*vf);

   GuiFilesModified fm;
   fm.setFociModified();
   fm.setVocabularyModified();
   theMainWindow->fileModificationUpdate(fm);

   const QString msg("The StudyInfo from the Vocabulary File has been moved\n"
                     "to the Study Metadata File.  As a result, both the \n"
                     "StudyMetaData and Vocabulary Files need to be saved.");
   QMessageBox::information(theMainWindow, "INFO", msg);
}

/**
 * Called to map volume data to a surface data file.
 */
void
GuiMainWindowAttributesActions::slotVolumeToSurfaceMapper()
{
   static GuiMapFmriDialog* mfd = NULL;
   if (mfd == NULL) {
      mfd = new GuiMapFmriDialog(theMainWindow, true, false);
      QObject::connect(mfd, SIGNAL(signalMappingComplete()),
                       this, SLOT(slotVolumeToSurfaceMapperComplete()));
   }
   mfd->initializeDialog();
   mfd->show();
}

/**
 * slot for volume to surface mapper complete.
 */
void 
GuiMainWindowAttributesActions::slotVolumeToSurfaceMapperComplete()
{
   GuiFilesModified fm;
   fm.setStatusForAll(true);
   fm.setInhibitSurfaceDefaultScaling();
   theMainWindow->fileModificationUpdate(fm);
   GuiBrainModelOpenGL::updateAllGL(NULL);
}
      
/**
 * generate a deformation field.
 */
void 
GuiMainWindowAttributesActions::slotGenerateDeformationField()
{
   GuiDeformationFieldDialog gdfd(theMainWindow);
   gdfd.exec();
}      

/**
 * Generate topography.
 */
void
GuiMainWindowAttributesActions::slotGenerateTopography()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

   BrainModelSurface* bms = theMainWindow->getBrainModelSurface();
   BrainModelBorderSet* bmbs = theMainWindow->getBrainSet()->getBorderSet();
   BorderFile borders;
   bmbs->copyBordersToBorderFile(bms, borders);
   BorderToTopographyConverter btc(theMainWindow->getBrainSet(),
                                   bms,
                                   &borders,
                                   theMainWindow->getBrainSet()->getPaintFile(),
                                   theMainWindow->getBrainSet()->getTopographyFile(),
                                   -1,
                                   "Topography");
   try {
      btc.execute();
   }
   catch (BrainModelAlgorithmException& e) {
      QApplication::restoreOverrideCursor();
      QMessageBox::critical(theMainWindow, "Error", e.whatQString());
   }
   QApplication::restoreOverrideCursor();
}

/**
 * Generate an areal estimation map.
 */
void
GuiMainWindowAttributesActions::slotGenerateArealEstimationMap()
{
   GuiGenerateArealEstimationDialog aed(theMainWindow);
   aed.exec();
}

/**
 * Edit and Add area colors
 */
void
GuiMainWindowAttributesActions::slotAreaColorsEdit()
{
   GuiColorFileEditorDialog* gfed = new GuiColorFileEditorDialog(theMainWindow,
                                                                 theMainWindow->getBrainSet()->getAreaColorFile(),
                                                                 false, 
                                                                 false,
                                                                 false,
                                                                 false,
                                                                 false);
   QObject::connect(gfed, SIGNAL(redrawRequested()),
                    this, SLOT(slotAreaColorsChanged()));
   gfed->show();
}

/**
 * This slot is called by the color editor when user pressed apply or dialog isclosed.
 */
void
GuiMainWindowAttributesActions::slotAreaColorsChanged()
{
   GuiFilesModified fm;
   fm.setAreaColorModified();
   theMainWindow->fileModificationUpdate(fm);
   theMainWindow->getBrainSet()->getNodeColoring()->assignColors();
   GuiBrainModelOpenGL::updateAllGL(NULL);
}

/**
 * Called to modify metric data.
 */
void
GuiMainWindowAttributesActions::slotMetricModification()
{
   theMainWindow->getMetricModificationDialog(true);
}

/**
 * Clear all or part of the file.
 */
void
GuiMainWindowAttributesActions::slotMetricClearAllOrPart()
{
   GuiFilesModified fm;
   fm.setMetricModified();
   GuiNodeAttributeFileClearResetDialog d(theMainWindow, GUI_NODE_FILE_TYPE_METRIC);
   d.exec();
}

/**
 * Called to create a volume using a metric column.
 */
void
GuiMainWindowAttributesActions::slotMetricToVolume()
{
   GuiSurfaceToVolumeDialog svd(theMainWindow, GuiSurfaceToVolumeDialog::DIALOG_MODE_METRIC,
                                "Copy Surface Metric Column to Functional Volume");
   if (svd.exec() == QDialog::Accepted) {
      const int metricColumn = svd.getSelectedNodeAttributeColumn();
      if ((metricColumn < 0) || (metricColumn >= theMainWindow->getBrainSet()->getMetricFile()->getNumberOfColumns())) {
         QMessageBox::critical(theMainWindow, "Error", "Invalid metric Column");
         return;
      }
      
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
      
      float offset[3];
      float voxelSize[3];
      float origin[3];
      int dim[3];
      svd.getSurfaceOffset(offset);
      svd.getVolumeDimensions(dim);
      svd.getVolumeVoxelSizes(voxelSize);
      svd.getVolumeOrigin(origin);
      BrainModelSurfaceToVolumeConverter stv(theMainWindow->getBrainSet(),
                                             svd.getSelectedSurface(),
                                             svd.getStandardVolumeSpace(),
                                             offset,
                                             dim,
                                             voxelSize,
                                             origin,
                                             svd.getSurfaceInnerBoundary(),
                                             svd.getSurfaceOuterBoundary(),
                                             svd.getSurfaceThicknessStep(),
                                             svd.getMetricConversionMode());
      stv.setNodeAttributeColumn(metricColumn);
      stv.setNodeToVoxelMappingEnabled(svd.getNodeToVoxelMappingEnabled(),
                                       svd.getNodeToVoxelMappingFileName());
      
      try {
         stv.execute();
      }
      catch (BrainModelAlgorithmException& e) {
         QMessageBox::critical(theMainWindow, "Error", e.whatQString());
         return;
      }
      
      theMainWindow->speakText("The metric data has been converted to a volume.", false);
      
      GuiBrainModelOpenGL* openGL = theMainWindow->getBrainModelOpenGL();
      openGL->displayBrainModelVolume();
      GuiFilesModified fm;
      fm.setVolumeModified();
      theMainWindow->fileModificationUpdate(fm);
      GuiBrainModelOpenGL::updateAllGL(NULL);
      QApplication::restoreOverrideCursor();
   }
}

/**
 * Called to display metric to rgb paint dialog
 */
void
GuiMainWindowAttributesActions::slotMetricToRgbPaint()
{
   theMainWindow->getMetricsToRgbPaintDialog(true);
}

/**
 * Called to compute average and deviation of metrics.
 */
void
GuiMainWindowAttributesActions::slotMetricAverageDeviation()
{
   QCheckBox* meanCheckBox = new QCheckBox("");
   QLineEdit* meanLineEdit = new QLineEdit;
   meanLineEdit->setText("Mean");
   QObject::connect(meanCheckBox, SIGNAL(toggled(bool)),
                    meanLineEdit, SLOT(setEnabled(bool)));
   meanCheckBox->setChecked(true);
   meanLineEdit->setEnabled(true);
   
   QCheckBox* stdDevCheckBox = new QCheckBox("");
   QLineEdit* stdDevLineEdit = new QLineEdit;
   stdDevLineEdit->setText("Sample Standard Deviation");
   QObject::connect(stdDevCheckBox, SIGNAL(toggled(bool)),
                    stdDevLineEdit, SLOT(setEnabled(bool)));
   stdDevCheckBox->setChecked(true);
   stdDevLineEdit->setEnabled(true);
   
   QCheckBox* stdErrorCheckBox = new QCheckBox("");
   QLineEdit* stdErrorLineEdit = new QLineEdit;
   stdErrorLineEdit->setText("Standard Error");
   QObject::connect(stdErrorCheckBox, SIGNAL(toggled(bool)),
                    stdErrorLineEdit, SLOT(setEnabled(bool)));
   stdErrorCheckBox->setChecked(false);
   stdErrorLineEdit->setEnabled(false);
   
   QCheckBox* minAbsCheckBox = new QCheckBox("");
   QLineEdit* minAbsLineEdit = new QLineEdit;
   minAbsLineEdit->setText("Minimum Absolute Value");
   QObject::connect(minAbsCheckBox, SIGNAL(toggled(bool)),
                    minAbsLineEdit, SLOT(setEnabled(bool)));
   minAbsCheckBox->setChecked(false);
   minAbsLineEdit->setEnabled(false);
   
   QCheckBox* maxAbsCheckBox = new QCheckBox("");
   QLineEdit* maxAbsLineEdit = new QLineEdit;
   maxAbsLineEdit->setText("Maximum Absolute Value");
   QObject::connect(maxAbsCheckBox, SIGNAL(toggled(bool)),
                    maxAbsLineEdit, SLOT(setEnabled(bool)));
   maxAbsCheckBox->setChecked(false);
   maxAbsLineEdit->setEnabled(false);
   
   WuQDataEntryDialog ded(theMainWindow);
   ded.setWindowTitle("Metric Statistics");
   ded.setTextAtTop("Choose Statistical Measurements", false);
   ded.addWidgetsToNextRow(meanCheckBox, meanLineEdit);
   ded.addWidgetsToNextRow(stdDevCheckBox, stdDevLineEdit);
   ded.addWidgetsToNextRow(stdErrorCheckBox, stdErrorLineEdit);
   ded.addWidgetsToNextRow(minAbsCheckBox, minAbsLineEdit);
   ded.addWidgetsToNextRow(maxAbsCheckBox, maxAbsLineEdit);
   
   if (ded.exec() == WuQDataEntryDialog::Accepted) {
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
      
      QString meanName, devName, errorName, minAbsName, maxAbsName;
      if (meanCheckBox->isChecked()) {
         meanName = meanLineEdit->text();
      }
      if (stdDevCheckBox->isChecked()) {
         devName = stdDevLineEdit->text();
      }
      if (stdErrorCheckBox->isChecked()) {
         errorName = stdErrorLineEdit->text();
      }
      if (minAbsCheckBox->isChecked()) {
         minAbsName = minAbsLineEdit->text();
      }
      if (maxAbsCheckBox->isChecked()) {
         maxAbsName = maxAbsLineEdit->text();
      }
      
      MetricFile* mf = theMainWindow->getBrainSet()->getMetricFile();
      mf->computeStatistics(meanName,
                             devName,
                             errorName,
                             minAbsName,
                             maxAbsName);
                             
      theMainWindow->getBrainSet()->getNodeColoring()->assignColors();
      GuiBrainModelOpenGL::updateAllGL(NULL);
      GuiFilesModified fm;
      fm.setMetricModified();
      theMainWindow->fileModificationUpdate(fm);
      QApplication::restoreOverrideCursor();
   }
}

/**
 * convert prob atlas threshold coloring to paint.
 */
void 
GuiMainWindowAttributesActions::slotProbAtlasThresholdToPaint()
{
   ProbabilisticAtlasFile* probAtlasFile = theMainWindow->getBrainSet()->getProbabilisticAtlasSurfaceFile();
   if (probAtlasFile->getNumberOfColumns() < 1) {
      QMessageBox::critical(theMainWindow, "ERROR", "Probabilistic Atlas File is Empty.");
      return;
   }
   
   //
   // Get node coloring
   //
   BrainModelSurfaceNodeColoring* bsnc = theMainWindow->getBrainSet()->getNodeColoring();
   
   //
   // An overlay or underlay must be prob atlas
   //
   bool error = false;
   if (theMainWindow->getBrainSet()->isASurfaceOverlay(0, 
           BrainModelSurfaceOverlay::OVERLAY_PROBABILISTIC_ATLAS)) {
      error = true;
   }
   
   //
   // Threshold display mode must be set for prob atlas
   //
   DisplaySettingsProbabilisticAtlas* dspa = theMainWindow->getBrainSet()->getDisplaySettingsProbabilisticAtlasSurface();
   if (dspa->getDisplayType() != 
       DisplaySettingsProbabilisticAtlas::PROBABILISTIC_DISPLAY_TYPE_THRESHOLD) {
      error = true;
   }
   
   if (error) {
      QMessageBox::critical(theMainWindow, "ERROR",
         "The overlay or underlay must be set to Probabilistic\n"
         "Atlas with the Display Mode set to Threshold.");
      return;
   }
   
   //
   // Use dialog to choose paint column and its name
   //
   GuiChooseNodeAttributeColumnDialog cacd(theMainWindow,
                                           GUI_NODE_FILE_TYPE_PAINT,
                                           "",
                                           true,
                                           false);
   if (cacd.exec() == QDialog::Accepted) {
      
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
      
      //
      // Modify paint file for new column or update
      //
      PaintFile* pf = theMainWindow->getBrainSet()->getPaintFile();
      int columnNumber = cacd.getSelectedColumnNumber();
      if (pf->getNumberOfNodes() == 0) {
         pf->setNumberOfNodesAndColumns(theMainWindow->getBrainSet()->getNumberOfNodes(), 1);
         columnNumber = 0;
      }
      else if (columnNumber < 0) {
         pf->addColumns(1);
         columnNumber = pf->getNumberOfColumns() - 1;
      }
      pf->setColumnName(columnNumber, cacd.getColumnName());
            
      //
      // Update paint file with colors
      //
      bsnc->addProbAtlasThresholdingToPaintFile(pf, columnNumber);
      
      GuiFilesModified fm;
      fm.setPaintModified();
      theMainWindow->fileModificationUpdate(fm);
      bsnc->assignColors();
      GuiBrainModelOpenGL::updateAllGL(NULL);
      
      QApplication::restoreOverrideCursor();
   }
}      

/**
 * slot for assigning paints within displayed borders.
 */
void 
GuiMainWindowAttributesActions::slotPaintAssignWithinDisplayedBorders()
{
   //
   // Get the surface in the main window and its brain model index
   //
   BrainModelSurface* bms = theMainWindow->getBrainModelSurface();
   if (bms != NULL) {
      const int modelIndex = theMainWindow->getBrainSet()->getBrainModelIndex(bms);
      if (modelIndex >= 0) {
         //
         // Get the coordinates
         //
         CoordinateFile* cf = bms->getCoordinateFile();
         const float* coords = cf->getCoordinate(0);
         const int numCoords = bms->getNumberOfNodes();
         
         //
         // Allocate the inside flags
         //
         std::vector<bool> nodeInsideFlag(numCoords);
         
         //
         // Get the paint file
         //
         PaintFile* pf = theMainWindow->getBrainSet()->getPaintFile();
         
         //
         // Popup a dialog to choose the paint column that is to be assigned
         //
         GuiChooseNodeAttributeColumnDialog nacd(theMainWindow,
                                                 GUI_NODE_FILE_TYPE_PAINT,
                                                 "",
                                                 true,
                                                 false);
         nacd.setWindowTitle("Choose Paint Column");
         if (nacd.exec() == QDialog::Accepted) {
         
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            
            //
            // Modify paint file for column change
            //
            int paintColumn = nacd.getSelectedColumnNumber();
            if (paintColumn == GuiNodeAttributeColumnSelectionComboBox::CURRENT_ITEM_NEW) {
               if (pf->getNumberOfColumns() <= 0) {
                  pf->setNumberOfNodesAndColumns(numCoords, 1);
               }
               else {
                  pf->addColumns(1);
               }
               paintColumn = pf->getNumberOfColumns() - 1;
            }
            pf->setColumnName(paintColumn, nacd.getColumnName());
            
            //
            // Find borders that are valid for this surface and displayed
            //
            BrainModelBorderSet* borderSet = theMainWindow->getBrainSet()->getBorderSet();
            const int numBorders = borderSet->getNumberOfBorders();
            for (int i = 0; i < numBorders; i++) {
               BrainModelBorder* border = borderSet->getBorder(i);
               if (border->getDisplayFlag() && 
                  border->getValidForBrainModel(modelIndex)) {
                  //
                  // Find nodes within the border
                  //
                  border->pointsInsideBorder(bms, 
                                             coords,
                                             numCoords,
                                             nodeInsideFlag,
                                             false);
                                             
                  //
                  // Assign paint nodes for nodes that are within the border
                  //
                  int paintIndex = -1;
                  for (int j = 0; j < numCoords; j++) {
                     if (nodeInsideFlag[j]) {
                        if (paintIndex < 0) {
                           paintIndex = pf->addPaintName(border->getName());
                        }
                        pf->setPaint(j, paintColumn, paintIndex);
                     }
                  }
               }
            }
            
            //
            // Assign colors and update graphics
            //
            GuiFilesModified fm;
            fm.setPaintModified();
            theMainWindow->fileModificationUpdate(fm);
            theMainWindow->getBrainSet()->getNodeColoring()->assignColors();
            GuiBrainModelOpenGL::updateAllGL(NULL);
            QApplication::restoreOverrideCursor();
         }
      }
   }
}

/**
 * Clear all or part of the paint file.
 */
void
GuiMainWindowAttributesActions::slotPaintClearAllOrPart()
{
   GuiFilesModified fm;
   fm.setPaintModified();
   GuiNodeAttributeFileClearResetDialog d(theMainWindow, GUI_NODE_FILE_TYPE_PAINT);
   d.exec();
}

/**
 * Clear all or part of the areal estimation file.
 */
void
GuiMainWindowAttributesActions::slotArealEstimationClearAllOrPart()
{
   GuiFilesModified fm;
   fm.setArealEstimationModified();
   GuiNodeAttributeFileClearResetDialog d(theMainWindow, GUI_NODE_FILE_TYPE_AREAL_ESTIMATION);
   d.exec();
}

/**
 * Called to create a volume using a paint column.
 */
void
GuiMainWindowAttributesActions::slotPaintToVolume()
{
   GuiSurfaceToVolumeDialog svd(theMainWindow, GuiSurfaceToVolumeDialog::DIALOG_MODE_PAINT,
                                "Copy Surface Paint Column to Paint Volume");
   if (svd.exec() == QDialog::Accepted) {
      const int paintColumn = svd.getSelectedNodeAttributeColumn();
      if ((paintColumn < 0) || (paintColumn >= theMainWindow->getBrainSet()->getPaintFile()->getNumberOfColumns())) {
         QMessageBox::critical(theMainWindow, "Error", "Invalid paint Column");
         return;
      }
      
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
      
      float offset[3];
      float voxelSize[3];
      float origin[3];
      int dim[3];
      svd.getSurfaceOffset(offset);
      svd.getVolumeDimensions(dim);
      svd.getVolumeVoxelSizes(voxelSize);
      svd.getVolumeOrigin(origin);
      BrainModelSurfaceToVolumeConverter stv(theMainWindow->getBrainSet(),
                                             svd.getSelectedSurface(),
                                             svd.getStandardVolumeSpace(),
                                             offset,
                                             dim,
                                             voxelSize,
                                             origin,
                                             svd.getSurfaceInnerBoundary(),
                                             svd.getSurfaceOuterBoundary(),
                                             svd.getSurfaceThicknessStep(),
                   BrainModelSurfaceToVolumeConverter::CONVERT_TO_ROI_VOLUME_USING_PAINT);
      stv.setNodeAttributeColumn(paintColumn);
      stv.setNodeToVoxelMappingEnabled(svd.getNodeToVoxelMappingEnabled(),
                                       svd.getNodeToVoxelMappingFileName());

      try {
         stv.execute();
      }
      catch (BrainModelAlgorithmException& e) {
         QMessageBox::critical(theMainWindow, "Error", e.whatQString());
         return;
      }
      
      theMainWindow->speakText("The paint data has been converted to a volume.", false);
      
      GuiBrainModelOpenGL* openGL = theMainWindow->getBrainModelOpenGL();
      openGL->displayBrainModelVolume();
      GuiFilesModified fm;
      fm.setVolumeModified();
      theMainWindow->fileModificationUpdate(fm);
      GuiBrainModelOpenGL::updateAllGL(NULL);
      QApplication::restoreOverrideCursor();
   }
}

/**
 * Called to copy current coloring to a volume.
 */
void
GuiMainWindowAttributesActions::slotCopyColoringToVolume()
{
   GuiSurfaceToVolumeDialog svd(theMainWindow, GuiSurfaceToVolumeDialog::DIALOG_MODE_NORMAL,
                                "Copy Coloring to RGB Volume...");
   if (svd.exec() == QDialog::Accepted) {
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
      
      float offset[3];
      float voxelSize[3];
      float origin[3];
      int dim[3];
      svd.getSurfaceOffset(offset);
      svd.getVolumeDimensions(dim);
      svd.getVolumeVoxelSizes(voxelSize);
      svd.getVolumeOrigin(origin);
      BrainModelSurfaceToVolumeConverter stv(theMainWindow->getBrainSet(),
                                             svd.getSelectedSurface(),
                                             svd.getStandardVolumeSpace(),
                                             offset,
                                             dim,
                                             voxelSize,
                                             origin,
                                             svd.getSurfaceInnerBoundary(),
                                             svd.getSurfaceOuterBoundary(),
                                             svd.getSurfaceThicknessStep(),
                   BrainModelSurfaceToVolumeConverter::CONVERT_TO_RGB_VOLUME_USING_NODE_COLORING);
      stv.setNodeToVoxelMappingEnabled(svd.getNodeToVoxelMappingEnabled(),
                                       svd.getNodeToVoxelMappingFileName());

      try {
         stv.execute();
      }
      catch (BrainModelAlgorithmException& e) {
         QMessageBox::critical(theMainWindow, "Error", e.whatQString());
         return;
      }
      
      theMainWindow->speakText("The R G B data has been converted to a volume.", false);
      
      GuiBrainModelOpenGL* openGL = theMainWindow->getBrainModelOpenGL();
      openGL->displayBrainModelVolume();
      GuiFilesModified fm;
      fm.setVolumeModified();
      theMainWindow->fileModificationUpdate(fm);
      GuiBrainModelOpenGL::updateAllGL(NULL);
      QApplication::restoreOverrideCursor();
   }
}

/**
 * Called to copy current coloring to RGB Paint.
 */
void
GuiMainWindowAttributesActions::slotCopyColoringToRgbPaint()
{
   GuiCurrentColoringToRgbPaintDialog* d = new GuiCurrentColoringToRgbPaintDialog(theMainWindow);
   d->exec();
}

/**
 * Called to copy surface normals to RGB Paint.
 */
void
GuiMainWindowAttributesActions::slotCopyNormalsToRgbPaint()
{
   BrainModelSurface* bms = theMainWindow->getBrainModelSurface();
   bms->convertNormalsToRgbPaint(theMainWindow->getBrainSet()->getRgbPaintFile());
   GuiFilesModified fm;
   fm.setRgbPaintModified();
   theMainWindow->fileModificationUpdate(fm);
}

/**
 * Called to clean paint file names
 */
void
GuiMainWindowAttributesActions::slotPaintCleanNames()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   PaintFile* pf = theMainWindow->getBrainSet()->getPaintFile();
   pf->cleanUpPaintNames();
   GuiFilesModified fm;
   fm.setPaintModified();
   theMainWindow->fileModificationUpdate(fm);
   GuiBrainModelOpenGL::updateAllGL(NULL);
   QApplication::restoreOverrideCursor();
}

/**
 * slot for generating colors for non-matching paint names.
 */
void 
GuiMainWindowAttributesActions::slotGenerateColorsForPaints()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   PaintFile* pf = theMainWindow->getBrainSet()->getPaintFile();
   const int numNames = pf->getNumberOfPaintNames();
   std::vector<QString> paintNames;
   for (int i = 0; i < numNames; i++) {
      paintNames.push_back(pf->getPaintNameFromIndex(i));
   }
   ColorFile* cf = theMainWindow->getBrainSet()->getAreaColorFile();
   cf->generateColorsForNamesWithoutColors(paintNames, true);
   theMainWindow->getBrainSet()->getNodeColoring()->assignColors();
   GuiBrainModelOpenGL::updateAllGL(NULL);
   GuiFilesModified fm;
   fm.setAreaColorModified();
   theMainWindow->fileModificationUpdate(fm);
   QApplication::restoreOverrideCursor();
}
      
/**
 * Called to modify metric data.
 */
void
GuiMainWindowAttributesActions::slotShapeModification()
{
   theMainWindow->getShapeModificationDialog(true);
}

/**
 * Called to compute average and deviation of metrics.
 */
void
GuiMainWindowAttributesActions::slotSurfaceShapeAverageDeviation()
{
   QCheckBox* meanCheckBox = new QCheckBox("");
   QLineEdit* meanLineEdit = new QLineEdit;
   meanLineEdit->setText("Mean");
   QObject::connect(meanCheckBox, SIGNAL(toggled(bool)),
                    meanLineEdit, SLOT(setEnabled(bool)));
   meanCheckBox->setChecked(true);
   meanLineEdit->setEnabled(true);
   
   QCheckBox* stdDevCheckBox = new QCheckBox("");
   QLineEdit* stdDevLineEdit = new QLineEdit;
   stdDevLineEdit->setText("Sample Standard Deviation");
   QObject::connect(stdDevCheckBox, SIGNAL(toggled(bool)),
                    stdDevLineEdit, SLOT(setEnabled(bool)));
   stdDevCheckBox->setChecked(true);
   stdDevLineEdit->setEnabled(true);
   
   QCheckBox* stdErrorCheckBox = new QCheckBox("");
   QLineEdit* stdErrorLineEdit = new QLineEdit;
   stdErrorLineEdit->setText("Standard Error");
   QObject::connect(stdErrorCheckBox, SIGNAL(toggled(bool)),
                    stdErrorLineEdit, SLOT(setEnabled(bool)));
   stdErrorCheckBox->setChecked(false);
   stdErrorLineEdit->setEnabled(false);
   
   QCheckBox* minAbsCheckBox = new QCheckBox("");
   QLineEdit* minAbsLineEdit = new QLineEdit;
   minAbsLineEdit->setText("Minimum Absolute Value");
   QObject::connect(minAbsCheckBox, SIGNAL(toggled(bool)),
                    minAbsLineEdit, SLOT(setEnabled(bool)));
   minAbsCheckBox->setChecked(false);
   minAbsLineEdit->setEnabled(false);
   
   QCheckBox* maxAbsCheckBox = new QCheckBox("");
   QLineEdit* maxAbsLineEdit = new QLineEdit;
   maxAbsLineEdit->setText("Maximum Absolute Value");
   QObject::connect(maxAbsCheckBox, SIGNAL(toggled(bool)),
                    maxAbsLineEdit, SLOT(setEnabled(bool)));
   maxAbsCheckBox->setChecked(false);
   maxAbsLineEdit->setEnabled(false);
   
   WuQDataEntryDialog ded(theMainWindow);
   ded.setWindowTitle("Surface Shape Statistics");
   ded.setTextAtTop("Choose Statistical Measurements", false);
   ded.addWidgetsToNextRow(meanCheckBox, meanLineEdit);
   ded.addWidgetsToNextRow(stdDevCheckBox, stdDevLineEdit);
   ded.addWidgetsToNextRow(stdErrorCheckBox, stdErrorLineEdit);
   ded.addWidgetsToNextRow(minAbsCheckBox, minAbsLineEdit);
   ded.addWidgetsToNextRow(maxAbsCheckBox, maxAbsLineEdit);
   
   if (ded.exec() == WuQDataEntryDialog::Accepted) {
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
      
      QString meanName, devName, errorName, minAbsName, maxAbsName;
      if (meanCheckBox->isChecked()) {
         meanName = meanLineEdit->text();
      }
      if (stdDevCheckBox->isChecked()) {
         devName = stdDevLineEdit->text();
      }
      if (stdErrorCheckBox->isChecked()) {
         errorName = stdErrorLineEdit->text();
      }
      if (minAbsCheckBox->isChecked()) {
         minAbsName = minAbsLineEdit->text();
      }
      if (maxAbsCheckBox->isChecked()) {
         maxAbsName = maxAbsLineEdit->text();
      }
      
      SurfaceShapeFile* ssf = theMainWindow->getBrainSet()->getSurfaceShapeFile();
      ssf->computeStatistics(meanName,
                             devName,
                             errorName,
                             minAbsName,
                             maxAbsName);
                             
      theMainWindow->getBrainSet()->getNodeColoring()->assignColors();
      GuiBrainModelOpenGL::updateAllGL(NULL);
      GuiFilesModified fm;
      fm.setSurfaceShapeModified();
      theMainWindow->fileModificationUpdate(fm);
      QApplication::restoreOverrideCursor();
   }
}


/**
 * Clear all or part of the surface shape file.
 */
void
GuiMainWindowAttributesActions::slotSurfaceShapeClearAllOrPart()
{
   GuiFilesModified fm;
   fm.setSurfaceShapeModified();
   GuiNodeAttributeFileClearResetDialog d(theMainWindow, GUI_NODE_FILE_TYPE_SURFACE_SHAPE);
   d.exec();
}

/**
 * Called to create a volume using a surface shape column.
 */
void
GuiMainWindowAttributesActions::slotSurfaceShapeToVolume()
{
   GuiSurfaceToVolumeDialog svd(theMainWindow, GuiSurfaceToVolumeDialog::DIALOG_MODE_SURFACE_SHAPE,
                                "Copy Surface Shape Column to Functional Volume");
   if (svd.exec() == QDialog::Accepted) {
      const int shapeColumn = svd.getSelectedNodeAttributeColumn();
      if ((shapeColumn < 0) || (shapeColumn >= theMainWindow->getBrainSet()->getSurfaceShapeFile()->getNumberOfColumns())) {
         QMessageBox::critical(theMainWindow, "Error", "Invalid surface shape Column");
         return;
      }
      
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
      
      float offset[3];
      float voxelSize[3];
      float origin[3];
      int dim[3];
      svd.getSurfaceOffset(offset);
      svd.getVolumeDimensions(dim);
      svd.getVolumeVoxelSizes(voxelSize);
      svd.getVolumeOrigin(origin);
      BrainModelSurfaceToVolumeConverter stv(theMainWindow->getBrainSet(),
                                             svd.getSelectedSurface(),
                                             svd.getStandardVolumeSpace(),
                                             offset,
                                             dim,
                                             voxelSize,
                                             origin,
                                             svd.getSurfaceInnerBoundary(),
                                             svd.getSurfaceOuterBoundary(),
                                             svd.getSurfaceThicknessStep(),
                   BrainModelSurfaceToVolumeConverter::CONVERT_TO_ROI_VOLUME_USING_SURFACE_SHAPE);
      stv.setNodeAttributeColumn(shapeColumn);
      stv.setNodeToVoxelMappingEnabled(svd.getNodeToVoxelMappingEnabled(),
                                       svd.getNodeToVoxelMappingFileName());

      try {
         stv.execute();
      }
      catch (BrainModelAlgorithmException& e) {
         QMessageBox::critical(theMainWindow, "Error", e.whatQString());
         return;
      }
      
      theMainWindow->speakText("The surface shape data has been converted to a volume.", false);
      
      GuiBrainModelOpenGL* openGL = theMainWindow->getBrainModelOpenGL();
      openGL->displayBrainModelVolume();
      GuiFilesModified fm;
      fm.setVolumeModified();
      theMainWindow->fileModificationUpdate(fm);
      GuiBrainModelOpenGL::updateAllGL(NULL);
      QApplication::restoreOverrideCursor();
   }
}

/**
 * Clear all or part of the surface shape file.
 */
void
GuiMainWindowAttributesActions::slotLatLonClearAllOrPart()
{
   GuiFilesModified fm;
   fm.setLatLonModified();
   GuiNodeAttributeFileClearResetDialog d(theMainWindow, GUI_NODE_FILE_TYPE_LAT_LON);
   d.exec();
}

/**
 * Called to create a volume using a surface shape column.
 */
void
GuiMainWindowAttributesActions::slotLatLonGenerateOnSphere()
{
   BrainModelSurface* bms = theMainWindow->getBrainModelSurface();
   if (bms == NULL) {
      QMessageBox::critical(theMainWindow, "Error", "There is no surface in the main window.");
      return;
   }
   
   if (bms->getSurfaceType() != BrainModelSurface::SURFACE_TYPE_SPHERICAL) {
      if (QMessageBox::warning(theMainWindow, 
                               "Warning", 
                               "The surface in the main window does\n"
                                                "not appear to be a sphere.", 
                               (QMessageBox::Ok | QMessageBox::Cancel),
                               QMessageBox::Cancel)
                                  == QMessageBox::Cancel) {
         return;
      }
   }
   
   GuiChooseNodeAttributeColumnDialog cnacd(theMainWindow,
                                            GUI_NODE_FILE_TYPE_LAT_LON,
                                            "",
                                            true,
                                            false);
   if (cnacd.exec() == QDialog::Accepted) {
      const int columnNumber = cnacd.getSelectedColumnNumber();
      const QString columnName = cnacd.getColumnName();
      bms->createLatitudeLongitude(theMainWindow->getBrainSet()->getLatLonFile(),
                                   columnNumber,
                                   columnName,
                                   false,
                                   true);
      GuiFilesModified fm;
      fm.setLatLonModified();
      theMainWindow->fileModificationUpdate(fm);
   }
}

/**
 * convert coord files to vectors.
 */
void 
GuiMainWindowAttributesActions::slotCoordsToVectors()
{
   GuiShapeOrVectorsFromCoordinateSubtractionDialog csd(theMainWindow,
                       GuiShapeOrVectorsFromCoordinateSubtractionDialog::MODE_VECTOR);
   csd.exec();
}

/**
 * convert normals to vectors.
 */
void 
GuiMainWindowAttributesActions::slotNormalsToVectors()
{
   BrainModelSurface* bms = theMainWindow->getBrainModelSurface();
   if (bms != NULL) {
      VectorFile* vf = new VectorFile();
      bms->copyNormalsToVectorFile(vf);
      theMainWindow->getBrainSet()->addVectorFile(vf);
      GuiFilesModified fm;
      fm.setVectorModified();
      theMainWindow->fileModificationUpdate(fm);
   }
}

/**
 * update the actions (typically called when menu is about to show)
 */
void 
GuiMainWindowAttributesActions::updateActions()
{
   const VocabularyFile* vf = theMainWindow->getBrainSet()->getVocabularyFile();
   vocabularyMoveStudyInfoToStudyMetaDataAction->setEnabled(vf->getNumberOfStudyInfo() > 0);
}

      
