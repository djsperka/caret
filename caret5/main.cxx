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


//
// This file contains the main function
//

#include <QGlobalStatic>

#ifdef Q_OS_WIN32
//#include <crtdbg.h>
#endif

#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QGLWidget>
#include <QImageIOPlugin>
#include <QLocale>
#include <QMessageBox>
#include <QPluginLoader>
#include <QStringList>
#include <QStyleFactory>

#include "BrainSet.h"
#include "CaretVersion.h"
#include "DebugControl.h"
#include "FileUtilities.h"
#include "GuiMacOSXApplication.h"
#include "GuiMainWindow.h"
#include "GuiMainWindowFileActions.h"
#include "GuiMainWindowFileMenu.h"
#include "GiftiDataArrayFile.h"
#include "SpecFile.h"
#include "SpecFileUtilities.h"
#include "StringUtilities.h"
#include "UbuntuMessage.h"
#include "WuQFileDialog.h"
#include "QtTextEditDialog.h"

#define CARET_MAIN_FLAG
#include "global_variables.h"
#undef  CARET_MAIN_FLAG

enum INITIAL_SPEC_MODE {
   INITIAL_SPEC_MODE_NONE,
   INITIAL_SPEC_MODE_LOAD,
   INITIAL_SPEC_MODE_DIALOG
};

static bool debugFlag = false;
static std::vector<QString> initialSpecFiles;
static INITIAL_SPEC_MODE initialSpecMode = INITIAL_SPEC_MODE_NONE;
static bool glTimingFlag = false;

static int openGLsizeX = -1;
static int openGLsizeY = -1;

/**
 * List the available QT GUI styles.
 */
static void
listGuiStyles()
{
   QStringList styles = QStyleFactory::keys();
   if (styles.count() > 0) {
      std::cout << "      -style  style-name" << std::endl
                << "         Set the user-interface style where \"style-name\" is one of:" << std::endl;
      for (QStringList::Iterator it = styles.begin(); it != styles.end(); it++) {
         QString s = *it;
         std::cout << "            " << s.toAscii().constData() << std::endl;
      }
      std::cout << "" << std::endl;
   }
}

/**
 * Print command line option help.
 */
static void
printHelp(const QString& programNameIn)
{
   const QString programName(FileUtilities::basename(programNameIn));
   
   std::cout 
   << "NAME" << std::endl
   << "   " << programName.toAscii().constData() << std::endl
   << "" << std::endl
   << "SYNOPSIS" << std::endl
   << "   " << programName.toAscii().constData() << " [options]" << std::endl
   << "" << std::endl
   << "DESCRIPTION" << std::endl
   << "   Visualize cerebral and cerebellar cortical maps." << std::endl
   << "" << std::endl
   << "   If spec-file-name is specified, the spec file by that name will" << std::endl
   << "   be loaded into a spec file dialog." << std::endl
   << "" << std::endl
   << "   " << programName.toAscii().constData() << " has the following options:" << std::endl
   << "" << std::endl
   << "      -debug" << std::endl
   << "         Enable the printing of debugging information to the console." << std::endl
   << "" << std::endl
   << "      -gl-timing" << std::endl
   << "         Adds a menu item to the view menu to enable the timing" << std::endl
   << "         of OpenGL graphics." << std::endl
   << "" << std::endl
   << "      -h  or  -help" << std::endl
   << "         Displays this help information." << std::endl
   << "" << std::endl
   << "      -loadspec  <spec-file-name-1> ... <spec-file-name-N>" << std::endl
   << "         Automatically loads all of the data files in each of the spec" << std::endl
   << "         files at startup." << std::endl
   << "" << std::endl
   << "      -notext" << std::endl
   << "         Inhibits use of text in OpenGL drawing such as the text" << std::endl
   << "         shown on volume slices indicating stereotaxic coordinates." << std::endl
   << "         This option may prevent crashes on Linux systems with " << std::endl
   << "         FreeType library problems." << std::endl
   << "" << std::endl
   << "      -read-time" << std::endl
   << "         Enable printing of time required to read each file." << std::endl
   << "" << std::endl
   << "      -spec  <spec-file-name>" << std::endl
   << "         Automatically loads \"spec-file-name\" into the spec file dialog at" << std::endl
   << "         startup." << std::endl
   << "" << std::endl;
   
   listGuiStyles();

   std::cout
   << "      -v" << std::endl
   << "         Print the version number." << std::endl
   << std::endl
   << "      -xy  X  Y" << std::endl
   << "         Set the main window graphics to be \"X by Y\" size." << std::endl
   << "         This is typically used when recording to MPEG so that" << std::endl
   << "         the video is a specific size." << std::endl
   << std::endl;
   
   exit(0);
}

/**
 * process the command line options
 */
static void
processCommandLineOptions(int argc, char* argv[])
{
   const QString programName(FileUtilities::basename(QString(argv[0])));
   
   for (int i = 1; i < argc; i++) {
      const QString arg(argv[i]);
      if (arg == "-debug") {
         debugFlag = true;
      }
      else if ((arg == "-h") || (arg == "-help")) {
         printHelp(argv[0]);
      }
      else if (arg == "-loadspec") {
         for (i = i + 1; i < argc; i++) {
            QString argValue = argv[i];
            if (argValue.startsWith("-")) {
               i--;
               break;
            }
            else {
               initialSpecFiles.push_back(argv[i]);
               initialSpecMode = INITIAL_SPEC_MODE_LOAD;
            }
         }
/*
         i++;
         if (i >= argc) {
            std::cerr << "ERROR: missing name of spec file on \"-loadspec\" option." << std::endl;
            exit(-1);
         }
         initialSpecFiles.push_back(argv[i]);
         initialSpecMode = INITIAL_SPEC_MODE_LOAD;
*/
      }
      else if (arg == "-spec") {
         i++;
         if (i >= argc) {
            std::cerr << "ERROR: missing name of spec file on \"-spec\" option." << std::endl;
            exit(-1);
         }
         initialSpecFiles.push_back(argv[i]);
         initialSpecMode = INITIAL_SPEC_MODE_DIALOG;
      }
      else if (arg == "-style") {
         i++;  // -style is used by QApplication
      }
      else if (arg == "-test1") {
         DebugControl::setTestFlag1(true);
      }
      else if (arg == "-test2") {
         DebugControl::setTestFlag2(true);
      }
      else if (arg == "-gl-timing") {
         glTimingFlag = true;
      }
      else if (arg == "-notext") {
         BrainModelOpenGL::setOpenGLTextEnabled(false);
      }
      else if (arg == "-read-time") {
         DebugControl::setFileReadTimingFlag(true);
      }
      else if ((arg == "-v") ||
               (arg == "-version")) {
         std::cout << programName.toAscii().constData()
                   << " version "
                   << CaretVersion::getCaretVersionAsString().toAscii().constData()
                   << std::endl;
         std::exit(0);
      }
      else if (arg == "-xy") {
         i++;
         if (i >= argc) {
            std::cerr << "ERROR: Window size missing on \"-xy\" option." << std::endl;
            exit(-1);
         }
         openGLsizeX = QString(argv[i]).toInt();
         i++;
         if (i >= argc) {
            std::cerr << "ERROR: Window Y size missing on \"-xy\" option." << std::endl;
            exit(-1);
         }
         openGLsizeY = QString(argv[i]).toInt();
      }
      else if (arg.mid(0, 4) == "-psn") {
         // ignore this arg which occurs if a spec file is double clicked on in mac finder
      }
      else {
         if (StringUtilities::endsWith(arg, SpecFile::getSpecFileExtension())) {
            initialSpecFiles.push_back(arg);
            initialSpecMode = INITIAL_SPEC_MODE_DIALOG;
         }
         else {
            std::cerr << "ERROR: Unrecognized command line option: " << arg.toAscii().constData() << std::endl;
            //exit(-1);
         }
      }
   }
}

/*
static void
testRE(const QString& filenameIn)
{
   QString directory,
   QString species;
   QString casename;  
   QString anatomy;
   QString hemisphere;   
   QString description;
   QString descriptionNoType;
   QString theDate;
   QString numNodes;
   QString extension; 
   
   if (FileUtilities::parseCaretDataFileName(filenameIn,
                                         directory,
                                         species,
                                         casename,
                                         anatomy,
                                         hemisphere,
                                         description,
                                         descriptionNoType,
                                         theDate,
                                         numNodes,
                                         extension)) {
      std::cout << "reassembled: "
         << FileUtilities::reassembleCaretDataFileName(
                                         directory,
                                         species,
                                         casename,
                                         anatomy,
                                         hemisphere,
                                         description,
                                         theDate,
                                         numNodes,
                                         extension) << std::endl << std::endl;
   }
}

static void
testRegularExpressions()
{
   
   std::cout << std::endl << std::endl;
   testRE("Human.colin.L.surface_shape");
   testRE("Human.colin.L.12345.surface_shape");
   testRE("Human.colin.L.2003-05-12.12345.surface_shape");
   testRE("Human.colin.L.03-05-12.12345.surface_shape");
   testRE("Human.colin.L.2003-05.12345.surface_shape");
   testRE("Human.colin.L.03-05.12345.surface_shape");
   testRE("Human.colin.Cerebral.R.71723.surface_shape");
   testRE("Human.colin.Cerebral.R.ATLAS.QuickStart.03-05.71723.spec");
   testRE("Human.colin.Cerebral.R.COMPOSITE.03-05-25.71723.paint");
   testRE("Human.colin.Cerebral.R.FIDUCIAL.TLRC.711-2B-afni.71723.coord");
   testRE("Human.colin.R.COMPOSITE_FUNCTIONAL_QuickStart.03-05-26.71723.metric");
   testRE("Human.colin.R.VISION.COMPOSITE.MapToSPM.03-05.fociproj");
   testRE("Human.colin.Cerebral.R.FLAT.CartSTD.71723.coord");
   testRE("Human.colin.Cerebral.R.SPHERE.STD.71723.coord");
   std::cout << std::endl << std::endl;
}
*/

/**
 * Initialize the File Dialog
 */
void
initializeFileDialog()
{
   WuQFileDialog::TypeExtensionToTypeNameMap typeMap;
   
   typeMap[SpecFile::getTopoFileExtension()] = "Topology";
   typeMap[SpecFile::getCoordinateFileExtension()] = "Coordinate";
   typeMap[SpecFile::getTransformationMatrixFileExtension()] = "Transformation Matrix";
   typeMap[SpecFile::getLatLonFileExtension()] = "Latitude/Longitude";
   typeMap[SpecFile::getSectionFileExtension()] = "Section";
   typeMap[SpecFile::getPaintFileExtension()] = "Paint";
   typeMap[SpecFile::getRegionOfInterestFileExtension()] = "Region of Interest";
   typeMap[SpecFile::getProbabilisticAtlasFileExtension()] = "Probabilistic Atlas";
   typeMap[SpecFile::getAreaColorFileExtension()] = "Area Color";
   typeMap[SpecFile::getRgbPaintFileExtension()] = "RGB Paint";
   typeMap[SpecFile::getBorderFileExtension()] = "Border";
   typeMap[SpecFile::getBorderColorFileExtension()] = "Border Color";
   typeMap[SpecFile::getBorderProjectionFileExtension()] = "Border Projection";
   typeMap[SpecFile::getPaletteFileExtension()] = "Palette";
   typeMap[SpecFile::getTopographyFileExtension()] = "Topography";
   typeMap[SpecFile::getCellFileExtension()] = "Cell";
   typeMap[SpecFile::getCellColorFileExtension()] = "Cell Color";
   typeMap[SpecFile::getCellProjectionFileExtension()] = "Cell Projection";
   typeMap[SpecFile::getContourFileExtension()] = "Contour";
   typeMap[SpecFile::getContourCellColorFileExtension()] = "Contour Cell Color";
   typeMap[SpecFile::getContourCellFileExtension()] = "Contour Cell";
   typeMap[SpecFile::getMetricFileExtension()] = "Metric";
   typeMap[SpecFile::getSurfaceShapeFileExtension()] = "Surface Shape";
   typeMap[SpecFile::getCocomacConnectivityFileExtension()] = "Cocomac Connectivity";
   typeMap[SpecFile::getArealEstimationFileExtension()] = "Areal Estimation";
   typeMap[SpecFile::getCutsFileExtension()] = "Cuts";
   typeMap[SpecFile::getFociFileExtension()] = "Foci";
   typeMap[SpecFile::getFociColorFileExtension()] = "Foci Color";
   typeMap[SpecFile::getFociProjectionFileExtension()] = "Foci Projection";
   typeMap[SpecFile::getParamsFileExtension()] = "Params File";
   typeMap[SpecFile::getDeformationMapFileExtension()] = "Deformation Map";
   typeMap[SpecFile::getDeformationFieldFileExtension()] = "Deformation Field";
   typeMap[SpecFile::getVtkModelFileExtension()] = "VTK Model";
   typeMap[SpecFile::getGeodesicDistanceFileExtension()] = "Geodesic Distance";
   typeMap[SpecFile::getAtlasSurfaceDirectoryFileExtension()] = "Atlas Surface Directory";
   typeMap[SpecFile::getBrainVoyagerFileExtension()] = "Brain Voyager";
   typeMap[SpecFile::getAtlasSpaceFileExtension()] = "Atlas Space";
   typeMap[SpecFile::getFreeSurferAsciiCurvatureFileExtension()] = "Free Surfer Curvature (ASCII)";
   typeMap[SpecFile::getFreeSurferBinaryCurvatureFileExtension()] = "Free Surfer Curvature (Binary)";
   typeMap[SpecFile::getFreeSurferAsciiFunctionalFileExtension()] = "Free Surfer Functional (ASCII)";
   typeMap[SpecFile::getFreeSurferBinaryFunctionalFileExtension()] = "Free Surfer Functional (Binary)";
   typeMap[SpecFile::getFreeSurferLabelFileExtension()] = "Free Surfer Label";
   typeMap[SpecFile::getFreeSurferAsciiSurfaceFileExtension()] = "Free Surfer Surface (ASCII)";
   typeMap[SpecFile::getFreeSurferBinarySurfaceFileExtension()] = "Free Surfer Surface (Binary)";
   typeMap[SpecFile::getSumaRgbFileExtension()] = "SUMA RGB";
   typeMap[SpecFile::getPreferencesFileExtension()] = "Preferences";
   typeMap[SpecFile::getSpecFileExtension()] = "Specification";
   typeMap[SpecFile::getAnalyzeVolumeFileExtension()] = "Volume - Analyze or SPM";
   typeMap[SpecFile::getAfniVolumeFileExtension().toLower()] = "Volume - AFNI";
   typeMap[SpecFile::getWustlVolumeFileExtension()] = "Volume - WUSTL";
   typeMap[SpecFile::getMincVolumeFileExtension()] = "Volume - MINC";
   typeMap[SpecFile::getNiftiVolumeFileExtension()] = "NIFTI";
   typeMap[SpecFile::getNiftiGzipVolumeFileExtension()] = "NIFTI - Compressed";
   typeMap[SpecFile::getSceneFileExtension()] = "Scene";
   typeMap[SpecFile::getSureFitVectorFileExtension()] = "SureFit Vector";
   typeMap[SpecFile::getWustlRegionFileExtension()] = "WUSTL Region";
   typeMap[SpecFile::getLimitsFileExtension()] = "Limits";
   typeMap[SpecFile::getMDPlotFileExtension()] = "MD Plot";
   typeMap[SpecFile::getGiftiCoordinateFileExtension()] = "GIFTI Coordinate";
   typeMap[SpecFile::getGiftiFunctionalFileExtension()] = "GIFTI Functional";
   typeMap[SpecFile::getGiftiLabelFileExtension()] = "GIFTI Label";
   typeMap[SpecFile::getGiftiRgbaFileExtension()] = "GIFTI RGBA";
   typeMap[SpecFile::getGiftiShapeFileExtension()] = "GIFTI Shape";
   typeMap[SpecFile::getGiftiSurfaceFileExtension()] = "GIFTI Surface";
   typeMap[SpecFile::getGiftiTensorFileExtension()] = "GIFTI Tensor";
   typeMap[SpecFile::getGiftiTopologyFileExtension()] = "GIFTI Topology";
   typeMap[SpecFile::getGiftiVectorFileExtension()] = "GIFTI Vector";
   typeMap[SpecFile::getGiftiGenericFileExtension()] = "GIFTI Generic";
   typeMap[SpecFile::getCommaSeparatedValueFileExtension()] = "Comma Separated Value";
   typeMap[SpecFile::getVocabularyFileExtension()] = "Vocabulary";
   typeMap[SpecFile::getStudyMetaDataFileExtension()] = "Study Metadata";
   typeMap[SpecFile::getStudyCollectionFileExtension()] = "Study Collection";
   typeMap[SpecFile::getXmlFileExtension()] = "XML";
   typeMap[SpecFile::getTextFileExtension()] = "Text";
   typeMap[SpecFile::getNeurolucidaFileExtension()] = "Neurolucida";
   typeMap[SpecFile::getCaretScriptFileExtension()] = "Caret Script";
   typeMap[SpecFile::getTrajectoryFileExtension()] = "Electrode trajectory";
   WuQFileDialog::setFileExtensionToTypeNameMap(typeMap);
}

/**
 * Unexpected handler
 */
void unexpectedHandler()
{
  std::cout << "WARNING: unhandled exception." << std::endl;
  if (theMainWindow != NULL) {
     const QString msg("Caret will be terminating due to an unexpected exception.\n"
                       "abort() will be called and a core file may be created.");
     QMessageBox::critical(theMainWindow, "ERROR", msg);
  }
  
  abort();
}

/**
 * New handler
 */
void newHandler()
{
   std::ostringstream str;
   str << "\n"
       << "OUT OF MEMORY\n"
       << "\n"
       << "This means that Caret is unable to get memory that it needs.\n"
       << "Possible causes:\n"
       << "   (1) Your computer lacks sufficient RAM.\n"
       << "   (2) Swap space is too small (you might increase it).\n"
       << "   (3) Your computer may be using an non-English character \n"
       << "       set.  Try switching to the English character set.\n"
       << "\n";
   std::cout << str.str().c_str() << std::endl;
   
   if (theMainWindow != NULL) {
      QMessageBox::critical(theMainWindow, "OUT OF MEMORY", 
                                "Out of memory, Caret terminating");
      std::exit(-1);
   }
}

/*
 * Can be used to print all events (and there are lots of them)
class CatchEvents : public QObject {
   protected:
      bool eventFilter(QObject* o, QEvent* e) {
         std::cout << "Class/Name: " << o->className() << ", " << o->name() << std::endl;
         return false;
      }
};
*/

/**
 * THE program's main.
 */
int
main(int argc, char* argv[])
{
#ifdef Q_OS_WIN32
   //
   // Disables some window debugging stuff that gets stuck in calls to new()
   //
   //_CrtSetDbgFlag(0);
#endif

   //
   // Set handlers in case sh*t happens
   //
   std::set_unexpected(unexpectedHandler);
   std::set_new_handler(newHandler);

/*
#ifdef Q_OS_LINUX
   //
   // Set style to Motif for Linux so that combo boxes use option menus
   //
   bool haveStyle = false;
   for (int i = 1; i < argc; i++) {
      if (QString(argv[i]) == "-style") {
         haveStyle = true;
         break;
      }
   }
   if (haveStyle == false) {
      //QT4 CONVERSION QApplication::setStyle(new QMotifStyle);
   }
#endif
*/

   //std::cout << "Home Directory: ";
   //std::cout << QDir::homePath().toAscii().constData()
   //          << std::endl;
   
   QString allArgs;
   for (int i = 1; i < argc; i++) {
      allArgs.append(argv[i]);
      allArgs.append("\n");
   }

   //
   // needed for static linking to have JPEG support
   //
#ifndef UBUNTU
#if QT_VERSION < 0x040600
   Q_IMPORT_PLUGIN(qjpeg) //QJpegPlugin)
   Q_IMPORT_PLUGIN(qgif)  //QGifPlugin)
   Q_IMPORT_PLUGIN(qtiff) //QTiffPlugin)
#endif //QT_VERSION
#endif
   //
   // Set the locale to prevent crashes due to non-english date formats
   //
   QLocale::setDefault(QLocale::c());

#ifdef Q_OS_MACX
   GuiMacOSXApplication app(argc, argv);
#else   
   QApplication app(argc, argv);
#endif

   
   /// shows all events  app.installEventFilter(new CatchEvents);
   
   processCommandLineOptions(argc, argv);
      
   //std::cout << "INFO: Set the environment variable CARET_DEBUG for debugging information." << std::endl;
   DebugControl::setDebugOnWithEnvironmentVariable("CARET_DEBUG");
   if (debugFlag) {
      DebugControl::setDebugOn(true);
   }

   if (getenv("CARET_TEST1") != NULL) {
      DebugControl::setTestFlag1(true);
   }
   if (getenv("CARET_TEST2") != NULL) {
      DebugControl::setTestFlag2(true);
   }
   
   //
   // Make sure OpenGL is available.
   //
   if (!QGLFormat::hasOpenGL()) {
      qWarning( "This system has no OpenGL support. Exiting." );
      return -1;
   }

   //
   // Setup OpenGL
   //
   QGLFormat glfmt;
   glfmt.setDoubleBuffer(true);
   glfmt.setDirectRendering(true);
   if ((getenv("CARET_TIMING") != NULL) || glTimingFlag) {
      glfmt.setDoubleBuffer(false);
      glTimingFlag = true;
   }
   QGLFormat::setDefaultFormat(glfmt);
   
   //
   // Get byte order
   //
   if (DebugControl::getDebugOn()) {
      if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
         std::cout << "big endian system" << std::endl;
      }
      else {
         std::cout << "little endian system" << std::endl;
      }
      
      QDesktopWidget* dt = QApplication::desktop();
      std::cout << "Screen size (" << dt->width() << ", " << dt->height() << ")" << std::endl;
   }
   
   //
   // Create and display the main window.
   //
   theMainWindow = new GuiMainWindow(glTimingFlag, openGLsizeX, openGLsizeY);
   //theMainWindow->resize(500, 500);
   //app.setMainWidget(theMainWindow);
   theMainWindow->show();
   app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
   
#ifdef Q_OS_MACX
   //
   // Enable getting the open file event
   //
   app.setGuiMainWindow(theMainWindow);
#endif

#ifdef UBUNTU
   if (QDir("/usr/share/caret/data_files").exists() == false) {
   QtTextEditDialog* te = new QtTextEditDialog(theMainWindow, true, true);
   te->setWindowTitle("UBUNTU WARNING");
   te->setMinimumSize(600, 600);
   te->setText(UbuntuMessage::getWarningMessage());
   te->exec();
   }
#endif //UBUNTU

   if (initialSpecFiles.empty() == false) {
      if (QFile::exists(initialSpecFiles[0]) == false) {
         std::cerr << "The spec file " << initialSpecFiles[0].toAscii().constData() 
                   << " specified on the command line not found." << std::endl;
         initialSpecFiles.clear();
      }
   }
   else if (initialSpecFiles.empty()) {
      std::vector<QString> specFiles;
      SpecFileUtilities::findSpecFilesInDirectory(QDir::currentPath(), specFiles);
      if (specFiles.size() == 1) {
         initialSpecFiles.push_back(specFiles[0]);
         initialSpecMode = INITIAL_SPEC_MODE_DIALOG;
      }
      else if (specFiles.size() > 1) {
         //
         // Show file dialog for choosing a spec file
         //
         theMainWindow->getFileActions()->openSpecFileSlot();
      }
   }
   
   //
   // Without this and if "-loadspec" option is used, the caret image will fail to be drawn
   //
   GuiBrainModelOpenGL::updateAllGL();
   
   QString welcomeMessage;
   const QTime timeNow(QTime::currentTime());
   if (timeNow.hour() < 12) {
      welcomeMessage.append("Good morning.");
   }
   else if (timeNow.hour() < 18) {
      welcomeMessage.append("Good afternoon.");
   }
   else {
      welcomeMessage.append("Good evening.");
   }
   welcomeMessage.append("  Welcome to carrot.");
   theMainWindow->speakText(welcomeMessage, false);
   
   if (initialSpecFiles.empty() == false) {
      QFileInfo fileInfo(initialSpecFiles[0]);
      switch (initialSpecMode) {
         case INITIAL_SPEC_MODE_NONE:
            break;
         case INITIAL_SPEC_MODE_LOAD:
            for (unsigned int i = 0; i < initialSpecFiles.size(); i++) {
               SpecFile sf;
               try {
                  QString path(FileUtilities::dirname(initialSpecFiles[i]));
                  if (QDir::isRelativePath(path)) {
                     path = QDir::currentPath();
                     path.append("/");
                     path.append(initialSpecFiles[i]);
                     initialSpecFiles[i] = QDir::cleanPath(path);
                  }
                  sf.readFile(initialSpecFiles[i]);
                  sf.setAllFileSelections(SpecFile::SPEC_TRUE);
                  theMainWindow->loadSpecFilesDataFiles(sf, NULL, true);
               }
               catch (FileException& e) {
                  QMessageBox::warning(theMainWindow, "ERROR", e.whatQString());
               }
            }
            break;
         case INITIAL_SPEC_MODE_DIALOG:
            theMainWindow->readSpecFile(fileInfo.absoluteFilePath());
            break;
      }
   }
   
   if (QFile::exists(theMainWindow->getBrainSet()->getCaretHomeDirectory()) == false) {
      QString msg("It appears that Caret is unable to determine its\n"
                      "installation directory.  Please set the environment\n"
                      "variable CARET5_HOME to the Caret installation\n"
                      "directory.  Until this is corrected, some Caret features\n"
                      "may not operate properly.");                      
       QMessageBox::warning(theMainWindow, "Install Warning", msg);
   }
   
   //
   // Initialize the file selection dialog
   //
   initializeFileDialog();
   
   //
   // Show command line arguments in an information dialog
   //
   //QMessageBox::information(theMainWindow, "args", allArgs);
   
   int result = -1;
   try {
      result = app.exec();
   }
   catch (BrainModelAlgorithmException& e) {
      QString msg("PROGRAM ERROR: BrainModelAlgorithmException not caught (this should not happen):\n"
                  "Please copy the contents of this message and send to john@brainvis.wustl.edu.");
      msg.append(e.whatQString());
      std::cerr << msg.toAscii().constData() << std::endl;
   }
   catch (FileException& e) {
      QString msg("PROGRAM ERROR: FileException not caught (this should not happen):\n"
                  "Please copy the contents of this message and send to john@brainvis.wustl.edu.");
      msg.append(e.whatQString());
      std::cerr << msg.toAscii().constData() << std::endl;
   }
   catch (std::exception& e) {
      QString msg("PROGRAM ERROR: std::exception not caught (this should not happen):\n"
                  "Please copy the contents of this message and send to john@brainvis.wustl.edu.");
      msg.append(e.what());
      std::cerr << msg.toAscii().constData() << std::endl;
   }
   //catch (std::bad_alloc) {
   //   std::cout << "Out of memory (bad_alloc) " << std::endl;
   //}
   theMainWindow->hide();//keeps paint events from causing assertion error in close
   return result;
}
