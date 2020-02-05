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


#ifndef __GUI_SPEC_FILE_DIALOG_H__
#define __GUI_SPEC_FILE_DIALOG_H__

#include <QMainWindow>

#include "BrainModelSurface.h"
#include "SpecFile.h"
#include "TransformationMatrixFile.h"
#include "WuQDialog.h"

//
// forward declarations to avoid include files
//
class GuiCategoryComboBox;
class GuiSpeciesComboBox;
class GuiSpecFileDialogMainWindow;
class GuiStereotaxicSpaceComboBox;
class GuiStructureComboBox;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QScrollArea;
class QToolButton;
class QVBoxLayout;

/// Dialog for making selections from the specification file.
class GuiSpecFileDialog : public WuQDialog {
 
   Q_OBJECT
   
   public:
      /// dialog mode
      enum SPEC_DIALOG_MODE {
         /// open a spec file mode
         SPEC_DIALOG_MODE_OPEN_SPEC_FILE,
         /// fast open a data file
         SPEC_DIALOG_MODE_OPEN_DATA_FILE,
         /// view current files
         SPEC_DIALOG_MODE_VIEW_CURRENT_FILES
      };
      
      /// Constructor
      GuiSpecFileDialog(QWidget* parent,
                        SpecFile& sf,
                        const SPEC_DIALOG_MODE dialogModeIn);

      /// Destructor
      ~GuiSpecFileDialog();
    
       /// Display the dialog
       virtual void show();
             
   public slots:
      /// close the dialog
      void slotCloseDialog();
      
      /// close the dialog when close button pressed
      void slotCloseDialogButton();
      
   private:
      /// the main window in this dialog
      GuiSpecFileDialogMainWindow* specFileDialogMainWindow;
      
      /// multiple fiducial coord files flag
      bool multipleFiducialCoordFlag;
      
      /// the dialog mode
      SPEC_DIALOG_MODE dialogMode;
      
      /// auto close check box
      QCheckBox* autoCloseCheckBox;
      
      /// directory containing spec file
      QString* directoryName;
      
};

/// MainWindow placed in the Spec File Dialog
class GuiSpecFileDialogMainWindow : public QMainWindow {
   Q_OBJECT

   public:
      /// Constructor
      GuiSpecFileDialogMainWindow(QWidget* parent, SpecFile& sf,
                                  const GuiSpecFileDialog::SPEC_DIALOG_MODE dialogModeIn);
      
      /// Destructor
      ~GuiSpecFileDialogMainWindow();
    
      /// write the spec file if needed
      void writeSpecFileIfNeeded();

   signals:
      /// requests that the dialog containing this object be closed
      void signalCloseDialogRequested();
      
   public slots:
      /// select all files
      void slotSelectAllFiles();
      
      /// deselect all files
      void slotDeselectAllFiles();
      
      /// Called when OK button is pressed
      void slotOkButton();
      
      /// Called when Create Spec button is pressed
      void slotCreateSpecButton();
      
      /// Called when Load Scene(s) button is pressed
      void slotLoadScenesButton();
      
      /// called when Comment button is pressed
      void slotCommentPushButton();
      
   protected slots:
      
   private slots:
      /// Called when a fast open button is pressed
      void fastOpenButtonSlot(int buttonNumber);
      
      /// Called when an info button is pressed
      void infoButtonGroupSlot(int buttonNumber);
      
      /// Called when a delete (X) button is pressed
      void deleteButtonGroupSlot(int buttonNumber);
      
      /// Called when the toolbar all button is pressed
      void slotToolBarAllButton();
      
      /// Called when the toolbar geometry button is pressed
      void slotToolBarGeometryButton();
      
      /// Called when the toolbar border button is pressed
      void slotToolBarBorderButton();
      
      /// Called when the toolbar cell button is pressed
      void slotToolBarCellButton();
      
      /// Called when the toolbar foci button is pressed
      void slotToolBarFociButton();
      
      /// Called when the toolbar metric button is pressed
      void slotToolBarMetricButton();
      
      /// Called when the toolbar misc button is pressed
      void slotToolBarMiscButton();
      
      /// Called when the toolbar paint button is pressed
      void slotToolBarPaintButton();
      
      /// Called when the toolbar scene button is pressed
      void slotToolBarSceneButton();
      
      /// Called when the toolbar volume button is pressed
      void slotToolBarVolumeButton();
      
      /// Called when set transform button is pressed
      void slotSetTransformPushButton();
      
   private:  
      /// file filter types
      enum FILE_FILTER_TYPE {
         FILE_FILTER_ALL,
         FILE_FILTER_COORD_AND_TOPO,
         FILE_FILTER_BORDER,
         FILE_FILTER_PAINT,
         FILE_FILTER_METRIC_AND_PALETTE,
         FILE_FILTER_CELL,
         FILE_FILTER_FOCI,
         FILE_FILTER_SCENE,
         FILE_FILTER_VOLUME,
         FILE_FILTER_MISC
      };
      
      /// This private class is used to keep track of selections made by the user
      /// and set the SpecFile selections when the dialog is closed.
      class SelectionItem {
         public:
            QWidget*        groupWidget;         // group checkbox is within for select/deselect all
            QCheckBox*      checkbox;            // used in open spec file mode
            QPushButton*    pushbutton;          // used in fast open mode
            QPushButton*    infobutton;
            QPushButton*    deletebutton;
            QLabel*         typelabel;
            QLabel*         namelabel;
            SpecFile::SPEC_FILE_BOOL* selected;  // used in open spec file mode
            QString     specFileTag;         // Used in fast open mode
            QString*    fileName;            // Used in fast open mode and for removal from spec
            AbstractFile*   dataFile;
            bool            volumeFlag;          // file is a volume file
            
            SelectionItem(QWidget* groupWidgetIn,
                          QCheckBox* cb,
                          QPushButton* pb,
                          QPushButton* ib,
                          QPushButton* db,
                          QLabel* tl,
                          QLabel* nl,
                          SpecFile::SPEC_FILE_BOOL* sel,
                          const QString specFileTagIn,
                          QString* fileNameIn,
                          AbstractFile* dataFileIn,
                          const bool volumeFlagIn) {
               groupWidget = groupWidgetIn;
               checkbox = cb;
               pushbutton = pb;
               infobutton = ib;
               deletebutton = db;
               typelabel = tl;
               namelabel = nl;
               selected = sel;
               specFileTag = specFileTagIn;
               fileName = fileNameIn;
               dataFile = dataFileIn;
               volumeFlag = volumeFlagIn;
            };
      };
      
      /// create the toolbar
      void createToolBar();
      
      /// disable tool bar buttons
      void disableToolBarButtons();
      
      /// set the toolbar buttons on/off
      void setToolBarButtons();
      
      /// List the surfaces parameters.
      QGroupBox* listSurfaceParameters(QVBoxLayout* layout);

      /// change the filename so that the path, if there is one, is in () after the filename
      QString prepareFileName(const QString& name);
      
      /// list files for selection by the user
      QGroupBox* listFiles(QVBoxLayout* layout,
                     const QString& title,
                     AbstractFile* dataFile,
                     const QString& specFileTag,
                     SpecFile::Entry& files,
                     const bool allowCommentEditing = true,
                     const bool volumeFileFlag = false);
                     
      /// list border files for selection by user
      QGroupBox* listBorderFiles(QVBoxLayout* layout,
                               SpecFile& sf);
                     
      /// list surface files for selection by user
      QGroupBox* listSurfaceFiles(QVBoxLayout* layout,
                               SpecFile& sf);
                     
      /// list coordinate files for selection by user
      QGroupBox* listCoordinateFiles(QVBoxLayout* layout,
                               SpecFile& sf);
                     
      /// list topology files for selection by user
      QGroupBox* listTopologyFiles(QVBoxLayout* layout,
                             SpecFile& sf);

      /// Set the files that are checked as seleted in the spec file.
      void setCheckedFilesAsSelected();

      /// keeps association of QCheckBoxes and selected files
      std::vector<SelectionItem> selectionItems;
      
      /// the spec file that is being selected
      SpecFile specFile;
      
      /// the dialog mode
      GuiSpecFileDialog::SPEC_DIALOG_MODE dialogMode;
      
      /// button group for fast open buttons
      QButtonGroup* fastOpenButtonGroup;
      
      /// button group for info buttons
      QButtonGroup* infoButtonGroup;
      
      /// button group for delete X buttons
      QButtonGroup* deleteButtonGroup;
      
      /// group box containing  files
      QGroupBox* areaColorGroup;
      
      /// group box containing  files
      QGroupBox* arealEstGroup;
      
      /// group box containing  files
      QGroupBox* borderColorGroup;
      
      /// group box containing  files
      QGroupBox* borderProjGroup;
      
      /// group box containing  files
      QGroupBox* cellGroup;
      
      /// group box containing volume cell files
      QGroupBox* cellVolumeGroup;
      
      /// group box containing  files
      QGroupBox* cellColorGroup;
      
      /// group box containing  files
      QGroupBox* cellProjectionGroup;
      
      /// cerebral hull files
      QGroupBox* cerebralHullGroup;
      
      /// group box containing  files
      QGroupBox* cocomacGroup;
      
      /// group box containing  files
      QGroupBox* contourGroup;
      
      /// group box containing  files
      QGroupBox* contourCellGroup;
      
      /// group box containing  files
      QGroupBox* contourCellColorGroup;
      
      /// group box containing  files
      QGroupBox* cutsGroup;
      
      /// group box containing deformation field files
      QGroupBox* defFieldGroup;
      
      /// group box containing  files
      QGroupBox* defMapGroup;
      
      /// group box containing document files
      QGroupBox* documentGroup;
      
      /// group box containing  files
      QGroupBox* fociGroup;
      
      /// group box containing  files
      QGroupBox* fociColorGroup;
      
      /// group box containing  files
      QGroupBox* fociProjGroup;
      
      /// group box containing files
      QGroupBox* fociSearchGroup;
      
      /// group box containing image files
      QGroupBox* imagesGroup;
      
      /// group box containing  files
      QGroupBox* latLonGroup;
      
      /// group box containing  files
      QGroupBox* metricGroup;
      
      /// group box containing  files
      QGroupBox* paintGroup;
      
      /// group box containing  files
      QGroupBox* paletteGroup;
      
      /// group box containing  files
      QGroupBox* paramsGroup;
      
      /// group box containing  files
      QGroupBox* probAtlasGroup;
      
      /// group box containing  files
      QGroupBox* rgbPaintGroup;
      
      /// group box containing  files
      QGroupBox* sceneGroup;
      
      /// group box containing  files
      QGroupBox* sectionGroup;
      
      /// group box containing  files
      QGroupBox* shapeGroup;
      
      /// group box containing files
      QGroupBox* vectorGroup;
      
      /// group box containing  files
      QGroupBox* topographyGroup;
      
      /// group box containing  files
      QGroupBox* transMatrixGroup;
      
      /// group box containing  files
      QGroupBox* transDataGroup;
      
      /// group box containing  files
      QGroupBox* volumeAnatomyGroup;
      
      /// group box containing  files
      QGroupBox* volumeFuncGroup;
      
      /// group box containing  files
      QGroupBox* volumePaintGroup;
      
      /// group box containing  files
      QGroupBox* volumeProbAtlasGroup;
      
      /// group box containing  files
      QGroupBox* volumeRgbGroup;
      
      /// group box containing  files
      QGroupBox* volumeSegmentGroup;
      
      /// group box containing files
      QGroupBox* volumeVectorGroup;
      
      /// group box containing  files
      QGroupBox* topologyGroup;
      
      /// group box containing  files
      QGroupBox* coordGroup;
      
      /// group box containing files
      QGroupBox* surfaceGroup;
      
      /// group box containing  files
      QGroupBox* borderGroup;
      
      /// group box for surface parameters
      QGroupBox* surfaceParametersGroup;
      
      /// group box for study collection files
      QGroupBox* studyCollectionGroup;
      
      /// group box for study metadata files
      QGroupBox* studyMetaDataGroup;
      
      /// group box for vocabulary files
      QGroupBox* vocabularyGroup;
      
      /// group for vtk model files
      QGroupBox* vtkModelGroup;
      
      /// group box for wustl region files
      QGroupBox* wustlRegionGroup;
      
      /// group box for geodesic distance file
      QGroupBox* geodesicGroup;
      
      /// group box for trajectory file
      QGroupBox* trajectoryGroup;

      /// file filter values
      FILE_FILTER_TYPE fileFilterSelections;
      
      /// all filter tool button
      QToolButton* allButton;
      
      /// geometry filter tool button
      QToolButton* geomButton;
      
      /// border filter tool button
      QToolButton* borderButton;
      
      /// cell filter tool button
      QToolButton* cellButton;
      
      /// foci filter tool button
      QToolButton* fociButton;
      
      /// metric filter tool button
      QToolButton* metricButton;
      
      /// misc filter tool button
      QToolButton* miscButton;
      
      /// paint filter tool button
      QToolButton* paintButton;
      
      /// scene filter tool button
      QToolButton* sceneButton;
      
      /// volume filter tool button
      QToolButton* volumeButton;
      
      /// flag set when user removes a file from the spec file
      bool specFileNeedsToBeWritten;
      
      /// space combo box
      GuiStereotaxicSpaceComboBox* spaceComboBox;

      /// species combo box
      GuiSpeciesComboBox* speciesComboBox;
      
      /// category combo box
      GuiCategoryComboBox* categoryComboBox;

      /// subject line edit
      QLineEdit* subjectLineEdit;

      /// structure combo box
      GuiStructureComboBox* structureComboBox;
      
      /// comment value at time spec file loaded into dialog
      QString savedComment;
      
      /// species value at time spec file loaded into dialog
      Species savedSpecies;

      /// structure value at time spec file loaded into dialog
      Structure::STRUCTURE_TYPE savedStructure;

      /// space value at time spec file loaded into dialog
      StereotaxicSpace savedStereotaxicSpace;

      /// category value at time spec file loaded into dialog
      Category savedCategory;
      
      /// category value at time spec file loaded into dialog
      QString savedSubject;
      
      /// check for missing files flag
      bool checkForMissingFiles;
      
      /// the transformation matrix
      TransformationMatrix transformMatrix;
      
      /// scrolling for the the files lists
      QScrollArea* filesScrollArea;
      
      /// widget containing file selections
      QWidget* filesListWidget;
      
      /// action for all toolbar button
      QAction* allAction;
      
      /// action for  toolbar button
      QAction* geomAction;
      
      /// action for  toolbar button
      QAction* borderAction;
      
      /// action for  toolbar button
      QAction* cellAction;
      
      /// action for  toolbar button
      QAction* fociAction;
      
      /// action for  toolbar button
      QAction* metricAction;
      
      /// action for  toolbar button
      QAction* miscAction;
      
      /// action for  toolbar button
      QAction* paintAction;
      
      /// action for  toolbar button
      QAction* sceneAction;
      
      /// action for  toolbar button
      QAction* volumeAction;
      
      /// directory containing spec file
      QString directoryName;
};

#endif


