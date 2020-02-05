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

#include <QGlobalStatic>

#include <fstream>
#include <iostream>
#include <sstream>

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGlobalStatic>
#include <QGroupBox>
#include <QImage>
#include <QImageWriter>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextStream>

#include "vtkDataArray.h"
#include "vtkImageData.h"
#include "vtkJPEGWriter.h"
#include "vtkPointData.h"

#define HAVE_AVI_WRITER 1
#undef HAVE_AVI_WRITER

#ifdef HAVE_MPEG
#ifdef HAVE_AVI_WRITER
#include "vtkAVIWriter.h"
#endif // HAVE_AVI_WRITER
#include "vtkMPEG1Writer.h"
#include "vtkMPEG2Writer.h"
#endif // HAVE_MPEG

#include "BrainModelRunExternalProgram.h"
#include "BrainSet.h"
#include "DebugControl.h"
#include "FileUtilities.h"
#include "GuiBrainModelOpenGL.h"
#include "GuiFileSelectionButton.h"
#include "GuiMainWindow.h"
#include "GuiRecordingDialog.h"
#include "QtUtilities.h"

#include "global_variables.h"
#include "zlib.h"

/**
 * Constructor.
 */
GuiRecordingDialog::GuiRecordingDialog(QWidget* parent) 
   : WuQDialog(parent)
{
   setWindowTitle("Recording");
   
   recordingOnFlag = false;
   imageCounter = 0;
      
   
   //
   // create the recording section
   //
   QWidget* recordingWidget = createRecordingSection();
   
   //
   // create the image size section
   //
   QWidget* imageSizeWidget = createImageSizeSection();
   
   //
   // create the output movie section
   //
   QWidget* outputWidget = createOutputMovieSection();
   
   //
   // Create the MPEG Create options
   //
   QWidget* mpegCreateWidget = createMpegCreateOptionsSection();
   
   //
   // Widgets and layout for main tab
   //
   QWidget* mainWidget = new QWidget;
   QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);
   mainLayout->addWidget(recordingWidget);
   mainLayout->addWidget(outputWidget);
   mainLayout->addStretch();
   
   //
   // Widgets and layout for options tab
   //
   QWidget* optionsWidget = new QWidget;
   QVBoxLayout* optionsLayout = new QVBoxLayout(optionsWidget);
   optionsLayout->addWidget(imageSizeWidget);
   optionsLayout->addWidget(mpegCreateWidget);
   optionsLayout->addStretch();
   
   //
   // Tab widget
   //
   QTabWidget* tabWidget = new QTabWidget;
   tabWidget->addTab(mainWidget, "Main");
   tabWidget->addTab(optionsWidget, "Options");
   
   //
   // Close button connects to QDialogs close() slot.
   //
   QPushButton* closeButton = new QPushButton("Close");
   closeButton->setAutoDefault(false);
   QObject::connect(closeButton, SIGNAL(clicked()),
                    this, SLOT(close()));
     
   //
   // Help button
   //
   QPushButton* helpButton = new QPushButton("Help");
   helpButton->setAutoDefault(false);
   QObject::connect(helpButton, SIGNAL(clicked()),
                    this, SLOT(slotHelpPushButton()));
   
   QtUtilities::makeButtonsSameSize(closeButton, helpButton);
   
   //
   // Horizontal layout for buttons
   //
   QHBoxLayout* buttonsLayout = new QHBoxLayout;
   buttonsLayout->addWidget(closeButton);
   buttonsLayout->addWidget(helpButton);
   
   //
   // Vertical box for all items in dialog
   //
   QVBoxLayout* dialogLayout = new QVBoxLayout(this);
   dialogLayout->addWidget(tabWidget);
   dialogLayout->addLayout(buttonsLayout);
   
#ifdef HAVE_MPEG
   vtkMpeg1MovieWriter = NULL;
   vtkMpeg2MovieWriter = NULL;
#ifdef HAVE_AVI_WRITER
   vtkAviMovieWriter = NULL;
#endif // HAVE_AVI_WRITER
#endif // HAVE_MPEG

   deleteTemporaryImages();
}
                    
/**
 * Destructor.
 */
GuiRecordingDialog::~GuiRecordingDialog()
{
#ifdef HAVE_MPEG
   if (vtkMpeg2MovieWriter != NULL) {
      vtkMpeg2MovieWriter->Delete();
      vtkMpeg2MovieWriter = NULL;
   }
   if (vtkMpeg1MovieWriter != NULL) {
      vtkMpeg1MovieWriter->Delete();
      vtkMpeg1MovieWriter = NULL;
   }
#ifdef HAVE_AVI_WRITER
   if (vtkAviMovieWriter != NULL) {
      vtkAviMovieWriter->Delete();
      vtkAviMovieWriter = NULL;
   }
#endif // HAVE_AVI_WRITER
#endif // HAVE_MPEG
}

/**
 * Called when help button is pressed.
 */
void
GuiRecordingDialog::slotHelpPushButton()
{
   theMainWindow->showHelpViewerDialog("dialogs/record_as_mpeg_dialog.html");   
}

/**
 * create the options section.
 */
QWidget* 
GuiRecordingDialog::createMpegCreateOptionsSection()
{
   //
   // Compress temporary images check box
   //
   compressTemporaryImages = new QCheckBox("Compress Temporary Images");
#ifdef Q_OS_WIN32
   compressTemporaryImages->setChecked(false);
#else
   compressTemporaryImages->setChecked(true);
#endif

   //
   // Quality label and spin box
   //
   QLabel* qualityLabel = new QLabel("Quality (1=High  31=Compression)");
   qualitySpinBox = new QSpinBox;
   qualitySpinBox->setMinimum(1);
   qualitySpinBox->setMaximum(31);
   qualitySpinBox->setSingleStep(1);
   qualitySpinBox->setValue(8);
   QHBoxLayout* qualityLayout = new QHBoxLayout;
   qualityLayout->addWidget(qualityLabel);
   qualityLayout->addWidget(qualitySpinBox);
   qualityLayout->addStretch();   
   
   //
   // Group box and layout for recording control
   //
   QGroupBox* mpegCreateGroupBox = new QGroupBox("MPEG Create Options");
   QVBoxLayout* mpegCreateLayout = new QVBoxLayout(mpegCreateGroupBox);
   mpegCreateLayout->addWidget(compressTemporaryImages);
   mpegCreateLayout->addLayout(qualityLayout);

   return mpegCreateGroupBox;   
}

/**
 * Create the recording control section.
 */
QWidget*
GuiRecordingDialog::createRecordingSection()
{
   //
   // automatic recording checkbox
   //
   recordAutomaticRadioButton = new QRadioButton("Automatic");
   
   //
   // manual recording checkbox and capture push button
   //
   recordManualRadioButton = new QRadioButton("Manual");
   manualCapturePushButton = new QPushButton("Capture");
   manualCapturePushButton->setAutoDefault(false);
   QObject::connect(manualCapturePushButton, SIGNAL(clicked()),
                    this, SLOT(slotManualCapturePushButton()));
   QHBoxLayout* manualLayout = new QHBoxLayout; 
   manualLayout->addWidget(recordManualRadioButton);
   manualLayout->addWidget(manualCapturePushButton);
   manualLayout->addStretch();
                      
   //
   // Button group for automatic and manual radio buttons
   //
   QButtonGroup* recordModeButtonGroup = new QButtonGroup(this);
   QObject::connect(recordModeButtonGroup, SIGNAL(buttonClicked(int)),
                    this, SLOT(slotRecordModeSelection()));
   recordModeButtonGroup->addButton(recordAutomaticRadioButton, 0);
   recordModeButtonGroup->addButton(recordManualRadioButton, 1);
   recordManualRadioButton->setChecked(true);
                    
   //
   // Record each image X times
   //
   QLabel* xTimesStartLabel = new QLabel("Record each image ");
   duplicateSpinBox = new QSpinBox;
   duplicateSpinBox->setMinimum(1);
   duplicateSpinBox->setMaximum(128);
   duplicateSpinBox->setSingleStep(1);
   duplicateSpinBox->setValue(1);
   QLabel* xTimesEndLabel = new QLabel(" times");
   QHBoxLayout* xTimesLayout = new QHBoxLayout;
   xTimesLayout->addWidget(xTimesStartLabel);
   xTimesLayout->addWidget(duplicateSpinBox);
   xTimesLayout->addWidget(xTimesEndLabel);
   xTimesLayout->addStretch();
   
   //
   // Group box and layout for recording control
   //
   QGroupBox* recordingControlGroupBox = new QGroupBox("Recording Control");
   QVBoxLayout* recordingLayout = new QVBoxLayout(recordingControlGroupBox);
   recordingLayout->addWidget(recordAutomaticRadioButton);
   recordingLayout->addLayout(manualLayout);
   recordingLayout->addLayout(xTimesLayout);

   return recordingControlGroupBox;   
}

/**
 * called when manual recording capture push button pressed.
 */
void 
GuiRecordingDialog::slotManualCapturePushButton()
{
   //
   // Draw and record the main window display
   //
   recordingOnFlag = true;
   GuiBrainModelOpenGL::updateAllGL(theMainWindow->getBrainModelOpenGL());
   recordingOnFlag = false;
}

/**
 * called when auto/manual radio button is selected.
 */
void 
GuiRecordingDialog::slotRecordModeSelection()
{
   manualCapturePushButton->setEnabled(recordManualRadioButton->isChecked());
   recordingOnFlag = (recordAutomaticRadioButton->isChecked());
}
      
/**
 * Create the image size section.
 */
QWidget*
GuiRecordingDialog::createImageSizeSection()
{
   //
   // Image size combo box and label                                                 
   //
   QLabel* imageSizeLabel = new QLabel("Image Size");
   imageSizeComboBox = new QComboBox;
   QObject::connect(imageSizeComboBox, SIGNAL(activated(int)),
                    this, SLOT(slotImageSizeComboBox(int)));
   imageSizeComboBox->insertItem(IMAGE_SIZE_TYPE_CARET_WINDOW, "Size of Caret Window");
   imageSizeComboBox->insertItem(IMAGE_SIZE_TYPE_320x240, "320 x 240");
   imageSizeComboBox->insertItem(IMAGE_SIZE_TYPE_352x240, "352 x 240");
   imageSizeComboBox->insertItem(IMAGE_SIZE_TYPE_640x480, "640 x 480");
   imageSizeComboBox->insertItem(IMAGE_SIZE_TYPE_720x480, "720 x 480");
   imageSizeComboBox->insertItem(IMAGE_SIZE_TYPE_CUSTOM, "Custom Size");
   QHBoxLayout* imageSizeLayout = new QHBoxLayout;
   imageSizeLayout->addWidget(imageSizeLabel);
   imageSizeLayout->addWidget(imageSizeComboBox);
   imageSizeLayout->addStretch();
   
   //
   // custom size line edits and labels
   //
   customSizeLabel = new QLabel("Custom Size ");
   customImageXSpinBox = new QSpinBox;
   customImageXSpinBox->setMinimum(1);
   customImageXSpinBox->setMaximum(1000000);
   customImageXSpinBox->setSingleStep(1);
   customImageXSpinBox->setFixedWidth(130);
   customImageYSpinBox = new QSpinBox;
   customImageYSpinBox->setMinimum(1);
   customImageYSpinBox->setMaximum(1000000);
   customImageYSpinBox->setSingleStep(1);
   customImageYSpinBox->setFixedWidth(130);
   QHBoxLayout* customSizeLayout = new QHBoxLayout;
   customSizeLayout->addWidget(customSizeLabel);
   customSizeLayout->addWidget(customImageXSpinBox);
   customSizeLayout->addWidget(customImageYSpinBox);
   customSizeLayout->addStretch();
   
   //
   // Group box and layout
   //
   imageSizeGroupBox = new QGroupBox("Image Size Control");
   QVBoxLayout* imageGroupLayout = new QVBoxLayout(imageSizeGroupBox);
   imageGroupLayout->addLayout(imageSizeLayout);
   imageGroupLayout->addLayout(customSizeLayout);

   //
   // Initialize some stuff
   //   
   slotImageSizeComboBox(IMAGE_SIZE_TYPE_CARET_WINDOW);

   return imageSizeGroupBox;
}

/**
 * Create the output move section.
 */
QWidget*
GuiRecordingDialog::createOutputMovieSection()
{
   //
   // Movie type and label
   //
   QLabel* movieFileTypeLabel = new QLabel("Movie File Type");
   movieFileTypeComboBox = new QComboBox;
   movieFileTypeWidget = new QWidget;
   QHBoxLayout* movieFileTypeLayout = new QHBoxLayout(movieFileTypeWidget);
   movieFileTypeLayout->addWidget(movieFileTypeLabel);
   movieFileTypeLayout->addWidget(movieFileTypeComboBox);
   movieFileTypeLayout->addStretch();
   
   //
   // Load the move type combo box
   //
   movieFileTypeComboBox->addItem("MPEG (using mpeg_create)", 
                                  static_cast<int>(MOVIE_FILE_TYPE_MPEG_CREATE));
#ifdef HAVE_MPEG
   movieFileTypeComboBox->addItem("MPEG1 (using VTK)", 
                                  static_cast<int>(MOVIE_FILE_TYPE_MPEG1_VTK));
   const int mpeg1Index = movieFileTypeComboBox->count() - 1;
   movieFileTypeComboBox->addItem("MPEG2 (using VTK)", 
                                  static_cast<int>(MOVIE_FILE_TYPE_MPEG2_VTK));
#ifdef HAVE_AVI_WRITER
   movieFileTypeComboBox->addItem("AVI (using VTK)", 
                                  static_cast<int>(MOVIE_FILE_TYPE_AVI_VTK));
#endif // HAVE_AVI_WRITER
   movieFileTypeComboBox->setCurrentIndex(mpeg1Index);
#endif // HAVE_MPEG
   movieFileTypeComboBox->addItem("JPEG Images",
                                  static_cast<int>(MOVIE_FILE_TYPE_JPEG_IMAGES));
                                  
   //
   // Movie name label and line edit
   //
   GuiFileSelectionButton* movieNamePushButton = new GuiFileSelectionButton(0,
                                                                            "Movie File...",
                                                                            "MPEG File (*.mpg)",
                                                                            false);
   movieNamePushButton->setAutoDefault(false);
   movieNamePushButton->setFixedSize(movieNamePushButton->sizeHint());
   movieNameLineEdit = new QLineEdit;
   movieNameLineEdit->setText("output.mpg");
   QObject::connect(movieNamePushButton, SIGNAL(fileSelected(const QString&)),
                    movieNameLineEdit, SLOT(setText(const QString&)));   
   movieNameWidget = new QWidget;
   QHBoxLayout* movieNameLayout = new QHBoxLayout(movieNameWidget);
   movieNameLayout->addWidget(movieNamePushButton);
   movieNameLayout->addWidget(movieNameLineEdit);
   movieNameLayout->setStretchFactor(movieNamePushButton, 0);
   movieNameLayout->setStretchFactor(movieNameLineEdit, 100);
   
   //
   // total images label
   //
   numberOfImagesLabel = new QLabel("");
   
   //
   // Create movie button
   //
   createMovieButton = new QPushButton("Create Movie");
   createMovieButton->setAutoDefault(false);
   createMovieButton->setFixedSize(createMovieButton->sizeHint());
   QObject::connect(createMovieButton, SIGNAL(clicked()),
                    this, SLOT(slotCreateMovieButton()));
   
   //
   // Reset button
   //
   resetPushButton = new QPushButton("Reset");
   resetPushButton->setAutoDefault(false);
   resetPushButton->setFixedSize(resetPushButton->sizeHint());
   QObject::connect(resetPushButton, SIGNAL(clicked()),
                    this, SLOT(deleteTemporaryImages()));
   
   //
   // Layout for create/reset buttons
   //
   QHBoxLayout* createResetLayout = new QHBoxLayout;
   createResetLayout->addWidget(createMovieButton);
   createResetLayout->addWidget(resetPushButton);
   createResetLayout->addStretch();
   
   //
   // Group box for page and layout
   //
   QGroupBox* outputGroupBox = new QGroupBox("Output Movie Control");
   QVBoxLayout* outputLayout = new QVBoxLayout(outputGroupBox);
   outputLayout->addWidget(movieFileTypeWidget);
   outputLayout->addWidget(movieNameWidget);
   outputLayout->addWidget(numberOfImagesLabel);
   outputLayout->addLayout(createResetLayout);
   
   return outputGroupBox;
}

/**
 *
 */
void
GuiRecordingDialog::updateControls()
{
   const bool haveImages = (imageCounter > 0);
   createMovieButton->setEnabled(haveImages);
   imageSizeGroupBox->setEnabled((haveImages == false));
   numberOfImagesLabel->setText(QString("Image Count: %1").arg(imageCounter));
}

/**
 * Called when create movie button is pressed.
 */
void
GuiRecordingDialog::slotCreateMovieButton()
{
   const QString movieName(movieNameLineEdit->text());
   if (movieName.isEmpty()) {
      QMessageBox::critical(this, "ERROR", "Movie name is blank.");
      return;
   }

   if (imageCounter <= 0) {
      QMessageBox::critical(this, "ERROR", "No images (frames) have been captured.");
      return;
   }
      
   //
   // Create the movie
   //
   switch (getMovieFileType()) {
      case MOVIE_FILE_TYPE_MPEG_CREATE:
         createMovieWithMpegCreate(movieName);
         break;
      case MOVIE_FILE_TYPE_MPEG1_VTK:
         createMovieWithMpeg1VTK();
         break;
      case MOVIE_FILE_TYPE_MPEG2_VTK:
         createMovieWithMpeg2VTK();
         break;
      case MOVIE_FILE_TYPE_AVI_VTK:
         createMovieWithAviVTK();
      case MOVIE_FILE_TYPE_JPEG_IMAGES:
         QMessageBox::information(this, 
                                  "INFO",
                                  "You will need to use a program such as \n"
                                  "Apple's iMovie or Microsoft's Media \n"
                                  "encoder to create a movie from the \n"
                                  "sequence of images.");
         break;
   }
}

/// get the movie file type
GuiRecordingDialog::MOVIE_FILE_TYPE 
GuiRecordingDialog::getMovieFileType() const
{
   //
   // Determine the movie file type
   //
   const int fileTypeIndex = movieFileTypeComboBox->currentIndex();
   const MOVIE_FILE_TYPE movieType = 
      static_cast<MOVIE_FILE_TYPE>(movieFileTypeComboBox->itemData(fileTypeIndex).toInt());

   return movieType;
}
      
/**
 * create the movie using VTK's MPEG1.
 */
void 
GuiRecordingDialog::createMovieWithMpeg1VTK()
{
#ifdef HAVE_MPEG
   //
   // Finish the movie
   //
   vtkMpeg1MovieWriter->End();
   
   if (DebugControl::getDebugOn()) {
      std::cout << "MPEG Writer Error Code: " << vtkMpeg1MovieWriter->GetErrorCode() << std::endl;
   }
   
   //
   // Cannot append images so delete all captured images
   //
   deleteTemporaryImages();
#endif // HAVE_MPEG
}

/**
 * create the movie using VTK's MPEG2.
 */
void 
GuiRecordingDialog::createMovieWithMpeg2VTK()
{
#ifdef HAVE_MPEG
   //
   // Finish the movie
   //
   vtkMpeg2MovieWriter->End();
   
   if (DebugControl::getDebugOn()) {
      std::cout << "MPEG Writer Error Code: " << vtkMpeg2MovieWriter->GetErrorCode() << std::endl;
   }
   
   //
   // Cannot append images so delete all captured images
   //
   deleteTemporaryImages();
#endif // HAVE_MPEG
}

/**
 * create the movie using VTK's AVI.
 */
void 
GuiRecordingDialog::createMovieWithAviVTK()
{
#ifdef HAVE_MPEG
#ifdef HAVE_AVI_WRITER
   //
   // Finish the movie
   //
   vtkAviMovieWriter->End();
   
   //
   // Cannot append images so delete all captured images
   //
   deleteTemporaryImages();
#endif // HAVE_AVI_WRITER
#endif // HAVE_MPEG
}
      
/**
 * create the movie using mpeg_create.
 */
void 
GuiRecordingDialog::createMovieWithMpegCreate(const QString& movieName)
{      
   const int numImages = static_cast<int>(imageNames.size());
   const int quality = qualitySpinBox->value();
   
   const QString mpegParamsName("mpeg_encode.mpeg_params");
   QFile file(mpegParamsName);
   if (file.open(QIODevice::WriteOnly)) {
      
      QTextStream stream(&file);
      
      stream << "PATTERN IBBPBBPBBPBBPBBP" << "\n";
      stream << "GOP_SIZE 16" << "\n";
      stream << "OUTPUT " << movieName << "\n";
      stream << "SLICES_PER_FRAME        1" << "\n";
      stream << "INPUT_DIR ." << "\n";
      stream << "INPUT" << "\n";
      for (int i = 0; i < numImages; i++) {
         stream << imageNames[i] << "\n";
      }
      stream << "END_INPUT" << "\n";
      stream << "BASE_FILE_FORMAT PPM" << "\n";
      if (compressTemporaryImages->isChecked()) {
         stream << "INPUT_CONVERT gunzip -c *" << "\n";
      }
      else {
         stream << "INPUT_CONVERT *" << "\n";
      }
      stream << "BSEARCH_ALG     CROSS2" << "\n";
      stream << "PIXEL           HALF" << "\n";
      stream << "IQSCALE         " << quality << "\n";
      stream << "PQSCALE         " << quality << "\n";
      stream << "BQSCALE         " << quality << "\n";
      stream << "REFERENCE_FRAME ORIGINAL" << "\n";
      stream << "RANGE           10" << "\n";
      stream << "PSEARCH_ALG     LOGARITHMIC" << "\n";
      stream << "FRAME_RATE      30" << "\n";
      
      file.close();

#ifdef Q_OS_WIN32
      QString msg("We have seen a problem on MS Windows systems that causes\n"
                      "this process to hang if there are about 20 or more frames\n"
                      "in the movie.  You can choose to continue and see what happens\n"
                      "or choose to create a command file that you can execute\n"
                      "separately from Caret to create the movie.");
      QMessageBox msgBox(this);
      msgBox.setWindowTitle("MS Windows Problem");
      msgBox.setText(msg);
      msgBox.addButton("Continue", QMessageBox::ActionRole);
      QPushButton* createCommandPushButton = msgBox.addButton("Create Command File",
                                                              QMessageBox::ActionRole);
      msgBox.exec();
      if (msgBox.clickedButton() == createCommandPushButton) { 
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         
         const QString fileName("create_movie.bat");
         QFile file(fileName);
         if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            
            QString programName(theMainWindow->getBrainSet()->getCaretHomeDirectory());
            programName.append("/bin/mpeg_encode.exe");
            stream << programName << " " << mpegParamsName << "\n";
            
            file.close();
         }
         else {
            QMessageBox::critical(this, "ERROR", "Unable to create command file.");
            return;
         }
         
         QString msg("The command file \"");
         msg.append(fileName);
         msg.append("\" has been created in the directory\n");
         msg.append(QDir::currentPath());
         msg.append("\n");
         msg.append("Run it by double-clicking it or run it from the command line.");
         QMessageBox::information(this, "Command File Created", msg);
         
         QApplication::restoreOverrideCursor();
         
         return;
      }
#endif
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

      //
      // Run the program to convert images into an MPEG movie
      //
      BrainModelRunExternalProgram cup("mpeg_encode", 
                                       QStringList(mpegParamsName), 
                                       true);
      try {
         cup.execute();
      }
      catch (BrainModelAlgorithmException& e) {
         QApplication::restoreOverrideCursor();
         QMessageBox::critical(this, "ERROR", e.whatQString());
         return;
      }
      QApplication::restoreOverrideCursor();
      theMainWindow->speakText("The movie has been created.", false);
   }
   else {
      QApplication::restoreOverrideCursor();
      QMessageBox::critical(this, "ERROR", 
                            "Error creating command file for creating movie.");
      return;
   }
}

/**
 * called when an image size selection is made.
 */
void 
GuiRecordingDialog::slotImageSizeComboBox(int item)
{
   const IMAGE_SIZE_TYPE st = static_cast<IMAGE_SIZE_TYPE>(item);
   
   customImageXSpinBox->setEnabled(st == IMAGE_SIZE_TYPE_CUSTOM);
   customImageYSpinBox->setEnabled(st == IMAGE_SIZE_TYPE_CUSTOM);
   customSizeLabel->setEnabled(st == IMAGE_SIZE_TYPE_CUSTOM);
}
      
/**
 * add an image to the recording.
 */
void 
GuiRecordingDialog::addImageToRecording(const QImage& imageIn)
{
   //
   // Once an image has been captured, disable the movie type until Reset button pressed
   //
   movieFileTypeWidget->setEnabled(false);
   movieNameWidget->setEnabled(false);
   
   //
   // Make a copy of the image since its size might be modified
   //
   QImage image = imageIn;

   //
   // Adjust the image size if needed
   //
   int newWidth  = 0;
   int newHeight = 0;
   switch(static_cast<IMAGE_SIZE_TYPE>(imageSizeComboBox->currentIndex())) {
      case IMAGE_SIZE_TYPE_CARET_WINDOW:
         break;
      case IMAGE_SIZE_TYPE_320x240:
         newWidth  = 320;
         newHeight = 240;
         break;
      case IMAGE_SIZE_TYPE_352x240:
         newWidth  = 352;
         newHeight = 240;
         break;
      case IMAGE_SIZE_TYPE_640x480:
         newWidth  = 640;
         newHeight = 480;
         break;
      case IMAGE_SIZE_TYPE_720x480:
         newWidth  = 720;
         newHeight = 480;
         break;
      case IMAGE_SIZE_TYPE_CUSTOM:
         newWidth  = customImageXSpinBox->value();
         newHeight = customImageYSpinBox->value();
         break;
   }
   if ((newWidth > 0) && (newHeight > 0)) {
      image = image.scaled(newWidth, newHeight);
   }
   
   //
   // Add the image to the movie
   //
   switch(getMovieFileType()) {
      case MOVIE_FILE_TYPE_MPEG_CREATE:
         addImageToMpegCreate(image);
         break;
      case MOVIE_FILE_TYPE_MPEG1_VTK:
         addImageToMpeg1VTK(image);
         break;
      case MOVIE_FILE_TYPE_MPEG2_VTK:
         addImageToMpeg2VTK(image);
         break;
      case MOVIE_FILE_TYPE_AVI_VTK:
         addImageToAviVTK(image);
         break;
      case MOVIE_FILE_TYPE_JPEG_IMAGES:
         addImageToJpeg(image);
         break;
   }
   
   imageCounter++;
   
   updateControls();
}
   
/**
 * convert a QImage into VTK's ImageData (user must delete returned vtkImageData object)
 */
vtkImageData* 
GuiRecordingDialog::convertQImagetoVTKImageData(const QImage& image)
{
   const int width = image.width();
   const int height = image.height();

   vtkImageData* vtkImage = vtkImageData::New();
   vtkImage->SetDimensions(width, height, 1);
#ifdef HAVE_VTK6
   vtkImage->AllocateScalars(VTK_FLOAT, 3);
#else
   vtkImage->SetScalarType(VTK_UNSIGNED_CHAR);
   vtkImage->SetNumberOfScalarComponents(3);
   vtkImage->AllocateScalars();
#endif
   unsigned char* vtkData = static_cast<unsigned char*>(vtkImage->GetScalarPointer());
   
   //
   // Copy the image data
   //
   //for (int j = 0; j < height; j++) {
   for (int j = (height - 1); j >= 0; j--) {
      for (int i = 0; i < width; i++) {
         const QColor rgb = QColor(image.pixel(i, j));
         const unsigned char r = static_cast<unsigned char>(rgb.red());
         const unsigned char g = static_cast<unsigned char>(rgb.green());
         const unsigned char b = static_cast<unsigned char>(rgb.blue());
         
         *vtkData++ = r;
         *vtkData++ = g;
         *vtkData++ = b;
      }
   }
   
   return vtkImage;
}
      
/**
 * add image to movie being created with VTK's MPEG1.
 */
void 
GuiRecordingDialog::addImageToMpeg1VTK(const QImage& image)
{
#ifdef HAVE_MPEG
   const QString fileName(movieNameLineEdit->text());
   vtkImageData* vtkImage = convertQImagetoVTKImageData(image);
   if (vtkMpeg1MovieWriter == NULL) {
      vtkMpeg1MovieWriter = vtkMPEG1Writer::New();
      vtkMpeg1MovieWriter->DebugOn();
      vtkMpeg1MovieWriter->SetFileName(movieNameLineEdit->text().toAscii().constData());
#ifdef HAVE_VTK6
      vtkMpeg1MovieWriter->SetInputData(vtkImage);
#else
      vtkMpeg1MovieWriter->SetInput(vtkImage);
#endif
      vtkMpeg1MovieWriter->Start();
   }
#ifdef HAVE_VTK6
   vtkMpeg1MovieWriter->SetInputData(vtkImage);
#else
   vtkMpeg1MovieWriter->SetInput(vtkImage);
#endif
   vtkMpeg1MovieWriter->Write();

   vtkImage->Delete();

   if (DebugControl::getDebugOn()) {
      std::cout << "MPEG Writer Error Code: " << vtkMpeg1MovieWriter->GetErrorCode() << std::endl;
   }
#endif // HAVE_MPEG
}

/**
 * add image to movie being created with VTK's MPEG2.
 */
void 
GuiRecordingDialog::addImageToMpeg2VTK(const QImage& image)
{
#ifdef HAVE_MPEG
   const QString fileName(movieNameLineEdit->text());
   vtkImageData* vtkImage = convertQImagetoVTKImageData(image);
   if (vtkMpeg2MovieWriter == NULL) {
      vtkMpeg2MovieWriter = vtkMPEG2Writer::New();
      vtkMpeg2MovieWriter->DebugOn();
      vtkMpeg2MovieWriter->SetFileName(fileName.toAscii().constData());
#ifdef HAVE_VTK6
      vtkMpeg2MovieWriter->SetInputData(vtkImage);
#else
      vtkMpeg2MovieWriter->SetInput(vtkImage);
#endif
      vtkMpeg2MovieWriter->Start();
   }
#ifdef HAVE_VTK6
   vtkMpeg2MovieWriter->SetInputData(vtkImage);
#else
   vtkMpeg2MovieWriter->SetInput(vtkImage);
#endif
   vtkMpeg2MovieWriter->Write();

   vtkImage->Delete();

   if (DebugControl::getDebugOn()) {
      std::cout << "MPEG Writer Error Code: " << vtkMpeg2MovieWriter->GetErrorCode() << std::endl;
   }
#endif // HAVE_MPEG
}

/**
 * add image to movie being created with VTK's AVI.
 */
void 
GuiRecordingDialog::addImageToAviVTK(const QImage& /*image*/)
{
#ifdef HAVE_MPEG
#ifdef HAVE_AVI_WRITER
   vtkImageData* vtkImage = convertQImagetoVTKImageData(image);
   if (vtkAviMovieWriter == NULL) {
      vtkAviMovieWriter = vtkAVIWriter::New();
      vtkAviMovieWriter->DebugOn();
      vtkAviMovieWriter->SetFileName(movieNameLineEdit->text().toAscii().constData());
#ifdef HAVE_VTK6
      vtkAviMovieWriter->SetInputData(vtkImage);
#else
      vtkAviMovieWriter->SetInput(vtkImage);
#endif
      vtkAviMovieWriter->Start();
   }
/*
 *     vtkJPEGWriter* jpeg = vtkJPEGWriter::New();
 *     jpeg->SetFileName("jpeg.jpg");
 *     jpeg->SetInput(vtkImage);
 *     jpeg->Write();
 */
#ifdef HAVE_VTK6
   vtkAviMovieWriter->SetInputData(vtkImage);
#else
   vtkAviMovieWriter->SetInput(vtkImage);
#endif
   vtkAviMovieWriter->Write();

   vtkImage->Delete();
   
#endif // HAVE_AVI_WRITER
#endif // HAVE_MPEG
}

/**
 * add image to movie for JPEG images.
 */
void 
GuiRecordingDialog::addImageToJpeg(const QImage& image)
{
   //
   // Create name of image
   //
   const QString outputName(
      QString().sprintf("record_image_%06d.jpg", static_cast<int>(imageCounter)));
      
   //
   // Write the image
   //
   QImageWriter writer(outputName);
   writer.setFormat("JPG");
   writer.setQuality(100);
   if (writer.write(image) == false) {
      QMessageBox::critical(this,
                            "ERROR",
                            "Error writing " + outputName);
      return;
   }
   
   imageNames.push_back(outputName);
}
      
/**
 * add image to movie being created with mpeg_create.
 */
void 
GuiRecordingDialog::addImageToMpegCreate(const QImage& image)
{      
   //
   // Buffers for normal and compressed images
   //
   static char* imageBuffer = NULL;
   static int imageBufferSize = -1;
   
   //
   // width and height of the image
   //
   const int  imageWidth = image.width();
   const int imageHeight = image.height();
   QString widthString(QString::number(imageWidth));
   QString heightString(QString::number(imageHeight));
   
   //
   // Buffer size needed to store the image in ppm format
   //
   const int bufferSizeNeeded = (imageWidth * imageHeight * 3) + 1000;

   //
   // Adjust buffer size if needed
   //
   if ((imageBuffer == NULL) || (bufferSizeNeeded > imageBufferSize)) {
      if (imageBuffer != NULL) {
         delete[] imageBuffer;
      }
      imageBufferSize = bufferSizeNeeded;
      imageBuffer = new char[imageBufferSize];
   }
   
   //
   // index to buffer
   //
   int bufferIndex = 0;
   
   //
   // Add "P6" to indicate binary PPM
   //
   imageBuffer[bufferIndex++] = 'P';
   imageBuffer[bufferIndex++] = '6';
   imageBuffer[bufferIndex++] = '\n';
   
   //
   // Add image width and height
   //
   for (int i = 0; i < widthString.length(); i++) {
      imageBuffer[bufferIndex++] = widthString[i].toLatin1();
   }
   imageBuffer[bufferIndex++] = ' ';
   for (int i = 0; i < heightString.length(); i++) {
      imageBuffer[bufferIndex++] = heightString[i].toLatin1();
   }
   imageBuffer[bufferIndex++] = '\n';
   
   //
   // Add in max value of pixel
   //
   imageBuffer[bufferIndex++] = '2';
   imageBuffer[bufferIndex++] = '5';
   imageBuffer[bufferIndex++] = '5';
   imageBuffer[bufferIndex++] = '\n';
   
   //
   // Add in the pixels
   //
   for (int j = 0; j < imageHeight; j++) {
      for (int i = 0; i < imageWidth; i++) {
         QRgb rgb = image.pixel(i, j);
         imageBuffer[bufferIndex++] = static_cast<char>(qRed(rgb));
         imageBuffer[bufferIndex++] = static_cast<char>(qGreen(rgb));
         imageBuffer[bufferIndex++] = static_cast<char>(qBlue(rgb));
      }
   }
   
   //
   // Check to see if images should be compressed
   //
   const bool useCompressedData = compressTemporaryImages->isChecked();
   
   //
   // Create name of image
   //
   QString outputName;
   outputName.sprintf("record_image_%06d.ppm", static_cast<int>(imageCounter));
   if (useCompressedData) {
      outputName.append(".gz");
   }
   
   //
   // write the image data
   //
   if (useCompressedData) {
      gzFile gzipImageFile = gzopen(outputName.toAscii().constData(), "w");
      if (gzipImageFile == NULL) {
         QString msg("Error writing image ");
         msg.append(outputName);
         QMessageBox::critical(this, "ERROR", msg);
         return;
      }
      gzwrite(gzipImageFile, imageBuffer, bufferIndex);
      gzclose(gzipImageFile);
   }
   else {
      std::ofstream ppmImageFile(outputName.toAscii().constData());
      if (!ppmImageFile) {
         QString msg("Error writing image ");
         msg.append(outputName);
         QMessageBox::critical(this, "ERROR", msg);
         return;
      }
      ppmImageFile.write(imageBuffer, bufferIndex);
      ppmImageFile.close();
   }
   
   for (int i = 0; i < duplicateSpinBox->value(); i++) {
      imageNames.push_back(outputName);
   }
}
      
/**
 * Delete any temporary images.
 */
void
GuiRecordingDialog::deleteTemporaryImages()
{
#ifdef HAVE_MPEG
   if (vtkMpeg1MovieWriter != NULL) {
      vtkMpeg1MovieWriter->Delete();
      vtkMpeg1MovieWriter = NULL;
   }
   if (vtkMpeg2MovieWriter != NULL) {
      vtkMpeg2MovieWriter->Delete();
      vtkMpeg2MovieWriter = NULL;
   }
#ifdef HAVE_AVI_WRITER
   if (vtkAviMovieWriter != NULL) {
      vtkAviMovieWriter->Delete();
      vtkAviMovieWriter = NULL;
   }
#endif // HAVE_AVI_WRITER
#endif // HAVE_MPEG

   for (unsigned int i = 0; i < imageNames.size(); i++) {
      QFile::remove(imageNames[i]);
   }
   imageCounter = 0;
   imageNames.clear();
   movieNameWidget->setEnabled(true);
   movieFileTypeWidget->setEnabled(true);
   updateControls();
}
