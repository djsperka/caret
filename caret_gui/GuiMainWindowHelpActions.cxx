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

#include <sstream>

#include <QAction>
#include <QInputDialog>
#include <QMessageBox>

#include "CaretVersion.h"
#include "GuiBrainModelOpenGL.h"
#include "GuiCaretTipsDialog.h"
#include "GuiHelpAssistantWindow.h"
#include "GuiMainWindow.h"
#include "GuiMainWindowHelpActions.h"
#include "QtTextEditDialog.h"
#include "SystemUtilities.h"
#include "global_variables.h"
#include "vtkVersion.h"
#include "zlib.h"

#ifdef HAVE_ITK
#include "itkVersion.h"
#endif

/**
 * constructor.
 */
GuiMainWindowHelpActions::GuiMainWindowHelpActions(QWidget* parent)
 : QObject(parent)
{
   assistantWindow = NULL;
   setObjectName("GuiMainWindowHelpActions");
   
   aboutAction = new QAction(parent);
   aboutAction->setText("About Caret 5...");
   aboutAction->setObjectName("aboutAction");
   QObject::connect(aboutAction, SIGNAL(triggered(bool)),
                    this, SLOT(helpMenuAbout()));
   
   aboutQtAction = new QAction(parent);
   aboutQtAction->setText("About QT...");
   aboutQtAction->setObjectName("aboutQtAction");
   QObject::connect(aboutQtAction, SIGNAL(triggered(bool)),
                    this, SLOT(helpMenuAboutQT()));

   aboutSplashAction = new QAction(parent);
   aboutSplashAction->setText("About Splash...");
   aboutSplashAction->setObjectName("aboutSplashAction");
   QObject::connect(aboutSplashAction, SIGNAL(triggered(bool)),
                    this, SLOT(helpMenuAboutSplash()));

   aboutOpenGLAction = new QAction(parent);
   aboutOpenGLAction->setText("About OpenGL...");
   aboutOpenGLAction->setObjectName("aboutOpenGLAction");
   QObject::connect(aboutOpenGLAction, SIGNAL(triggered(bool)),
                    this, SLOT(helpMenuAboutOpenGL()));

   caretHelpAction = new QAction(parent);
   caretHelpAction->setText("Caret Help...");
   caretHelpAction->setObjectName("caretHelpAction");
   QObject::connect(caretHelpAction, SIGNAL(triggered(bool)),
                    this, SLOT(helpMenuCaretHelp()));

   caretHelpAssistantAction = new QAction(parent);
   caretHelpAssistantAction->setText("Caret Help Assistant...");
   caretHelpAssistantAction->setObjectName("caretHelpAssistantAction");
   QObject::connect(caretHelpAssistantAction, SIGNAL(triggered(bool)),
                    this, SLOT(helpMenuCaretHelpAssistant()));

   caretOnlineHelpAction = new QAction(parent);
   caretOnlineHelpAction->setText("Caret Help (Online)...");
   caretOnlineHelpAction->setObjectName("caretOnlineHelpAction");
   QObject::connect(caretOnlineHelpAction, SIGNAL(triggered(bool)),
                    this, SLOT(helpMenuOnlineCaretHelp()));

   caretUsersListArchiveAction = new QAction(parent);
   caretUsersListArchiveAction->setText("Caret User's Email Archive (Online)...");
   caretUsersListArchiveAction->setObjectName("caretUsersListArchiveAction");
   QObject::connect(caretUsersListArchiveAction, SIGNAL(triggered(bool)),
                    this, SLOT(slotCaretUsersListArchiveAction()));

   caretTipsAction = new QAction(parent);
   caretTipsAction->setText("Caret Tips...");
   caretTipsAction->setObjectName("caretTipsAction");
   QObject::connect(caretTipsAction, SIGNAL(triggered(bool)),
                    this, SLOT(helpMenuCaretTips()));

   caretWebSiteAction = new QAction(parent);
   caretWebSiteAction->setText("Go To Caret Web Site...");
   caretWebSiteAction->setObjectName("caretWebSiteAction");
   QObject::connect(caretWebSiteAction, SIGNAL(triggered(bool)),
                    this, SLOT(helpMenuCaretWebSite()));

   caretSearchWebSiteAction = new QAction(parent);
   caretSearchWebSiteAction->setText("Search Caret Web Site...");
   caretSearchWebSiteAction->setObjectName("caretSearchWebSiteAction");
   QObject::connect(caretSearchWebSiteAction, SIGNAL(triggered(bool)),
                    this, SLOT(helpMenuCaretSearchWebSite()));

   sumbsWebSiteAction = new QAction(parent);
   sumbsWebSiteAction->setText("Go To SuMS Database Web Site...");
   sumbsWebSiteAction->setObjectName("sumbsWebSiteAction");
   QObject::connect(sumbsWebSiteAction, SIGNAL(triggered(bool)),
                    this, SLOT(helpMenuSumsWebSite()));

   sumsAtlasAction = new QAction(parent);
   sumsAtlasAction->setText("Go To SuMS Database Atlases...");
   sumsAtlasAction->setObjectName("sumsAtlasAction");
   QObject::connect(sumsAtlasAction, SIGNAL(triggered(bool)),
                    this, SLOT(helpMenuSumsAtlases()));

   sumsTutorialsAction = new QAction(parent);
   sumsTutorialsAction->setText("Go To SuMS Database Tutorials...");
   sumsTutorialsAction->setObjectName("sumsTutorialsAction");
   QObject::connect(sumsTutorialsAction, SIGNAL(triggered(bool)),
                    this, SLOT(helpMenuSumsTutorials()));
}

/**
 * destructor.
 */
GuiMainWindowHelpActions::~GuiMainWindowHelpActions()
{
}

/**
 * Called when caret help selected
 */
void
GuiMainWindowHelpActions::helpMenuCaretHelp()
{
   theMainWindow->showHelpViewerDialog();
}

/**
 * Called when caret help selected
 */
void
GuiMainWindowHelpActions::helpMenuCaretHelpAssistant()
{
   //assistantWindow = new GuiHelpAssistantWindow("/usr/local/caret_libraries/qt-software/qt/bin/");
   //assistantWindow->showPage("");
}

/**
 * display caret users email archive.
 */
void 
GuiMainWindowHelpActions::slotCaretUsersListArchiveAction()
{
   theMainWindow->displayWebPage("http://www.mail-archive.com/caret-users@brainvis.wustl.edu/index.html");
}      

/**
 * display online caret help.
 */
void 
GuiMainWindowHelpActions::helpMenuOnlineCaretHelp()
{
   theMainWindow->displayWebPage("http://brainvis.wustl.edu/CaretHelpAccount/caret5_help/");
}

/**
 * called when Help Menu About selected.
 */
void
GuiMainWindowHelpActions::helpMenuAbout()
{
   QString msg;
   msg.append("CARET v");
   msg.append(CaretVersion::getCaretVersionAsString());
   msg.append("\n");
   msg.append("Computerized Anatomical Reconstruction and Editing Toolkit\n");
   msg.append("John Harwell, Heather Drury, Donna Hanlon, and David Van Essen\n");
   msg.append("Copyright 1995-2006 Washington University\n");
   msg.append("\n");
   msg.append("Washington University School of Medicine\n");
   msg.append("660 S. Euclid Ave\n");
   msg.append("St. Louis, MO 63110  USA\n");
   msg.append("\n");
   msg.append("http://brainmap.wustl.edu/caret.html\n");
   msg.append("\n");
   msg.append("Use the Caret Mailing List for questions about Caret.\n");
   msg.append("   http://brainmap.wustl.edu/resources/caretnew.html#Help\n");
   msg.append("\n");

   msg.append("This program is free software; you can redistribute it and/or\n");
   msg.append("modify it under the terms of the GNU General Public License\n");
   msg.append("as published by the Free Software Foundation; either version 2\n");
   msg.append("of the License, or (at your option) any later version.\n");
   msg.append("\n");
   msg.append("This program is distributed in the hope that it will be useful,\n");
   msg.append("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
   msg.append("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
   msg.append("GNU General Public License for more details.\n");
   msg.append("\n");
   msg.append("You should have received a copy of the GNU General Public License\n");
   msg.append("along with this program; if not, write to the Free Software\n");
   msg.append("Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n");
   msg.append("\n");
   msg.append("\n");
   msg.append("CARET may also contain the following software:\n");
   msg.append("\n");
   msg.append("AFNI\n");
   msg.append("    Copyright 1994-2000 Medical College of Wisconsin\n");
   msg.append("    http://afni.nimh.nih.gov\n");
   msg.append("\n");
   msg.append("\n");
#ifdef HAVE_ITK
   msg.append("ITK - Insight Segmentation & Registration Toolkit\n");
   msg.append("Copyright (c) Insight Software Consortium. All rights reserved.\n");
   msg.append("    http://www.itk.org\n");
   msg.append("    version: ");
   msg.append(itk::Version::GetITKVersion());
   msg.append("\n");
   msg.append("\n");
#endif // HAVE_ITK
   msg.append("QT\n");
   msg.append("    Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.\n");
   msg.append("    http://www.trolltech.com\n");
   msg.append("    version: ");
   msg.append(qVersion());
   msg.append("\n");
   msg.append("\n");
   msg.append("VTK - Visualization Toolkit\n");
   msg.append("    Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen\n");
   msg.append("    http://www.vtk.org\n");
   msg.append("    version: ");
   msg.append(vtkVersion::GetVTKVersion());
   msg.append("\n");
   msg.append("\n");
   msg.append("ZLIB\n");
   msg.append("    Copyright (c) 1995-2002 Jean-loup Gailly and Mark Adler\n");
   msg.append("    http://www.gzip.org/zlib/\n");
   msg.append("    header file version: ");
   msg.append(ZLIB_VERSION);
   msg.append("    library version: ");
   msg.append(zlibVersion());
   msg.append("\n");
   msg.append("\n");
   std::ostringstream str;
   str << "Number of CPUs: " << SystemUtilities::getNumberOfProcessors() << "\n";
   msg.append(str.str().c_str());
   msg.append("\n");
   
   QtTextEditDialog te(theMainWindow, true, true);
   te.setWindowTitle("Caret5");
   te.setText(msg);
   te.exec();
}


void GuiMainWindowHelpActions::helpMenuAboutSplash()
{
	QString msg;
	msg.append("SPLASh v1.0\n");
	msg.append("Stereotactic PLAnning Software\n");
	msg.append("\n");
	msg.append("Daniel Sperka and Jochen Ditterich\n");
	msg.append("Copyright 2009 Regents University of California\n");
	msg.append("\n");
	msg.append("Center for Neuroscience\n");
	msg.append("University of California, Davis\n");
	msg.append("\n");
	msg.append("http://systems.ucdavis.edu/ett.html\n");
	msg.append("djsperka@ucdavis.edu\n");
	msg.append("\n");
	msg.append("SPLASh also contains the following software:\n");
	msg.append("\n");
	msg.append("Approximate Nearest Neighbors (ANN) Library\n");
	msg.append("Copyright (c) 1997-2005 University of Maryland,\n");
	msg.append("    Sunil Arya and David Mount All Rights Reserved)\n");
	msg.append("    http://www.cs.umd.edu/~mount/ANN/\n");
	msg.append("\n");
	msg.append("The following algorithm has proved quite useful as well.\n");
	msg.append("http://codesuppository.blogspot.com/2006/03/best-fit-plane.html\n");
	msg.append("Copyright (c) 1998-2006.  All Rights Reserved, Geometric Tools, Inc. \n");
	msg.append("http://www.geometrictools.com\n");

	QMessageBox::information(theMainWindow, "Splash Information", msg);
}


/**
 *
 */
void
GuiMainWindowHelpActions::helpMenuAboutQT()
{
   QMessageBox::aboutQt(theMainWindow, "About QT");
}

/**
 *
 */
void
GuiMainWindowHelpActions::helpMenuAboutOpenGL()
{
   QString msg;
#ifdef GL_VERSION_1_9
   msg.append("OpenGL header is 1.9\n");
#elif GL_VERSION_1_8
   msg.append("OpenGL header is 1.8\n");
#elif GL_VERSION_1_7
   msg.append("OpenGL header is 1.7\n");
#elif GL_VERSION_1_6
   msg.append("OpenGL header is 1.6\n");
#elif GL_VERSION_1_5
   msg.append("OpenGL header is 1.5\n");
#elif GL_VERSION_1_4
   msg.append("OpenGL header is 1.4\n");
#elif GL_VERSION_1_3
   msg.append("OpenGL header is 1.3\n");
#elif GL_VERSION_1_2
   msg.append("OpenGL header is 1.2\n");
#elif GL_VERSION_1_1
   msg.append("OpenGL header is 1.1\n");
#else
   msg.append("OpenGL header is 1.0\n");
#endif

   msg.append("OpenGL version from library is "); 
   msg.append((char*)(glGetString(GL_VERSION)));
   msg.append("\n");
   msg.append("OpenGL vendor from library is "); 
   msg.append((char*)(glGetString(GL_VENDOR)));
   msg.append("\n");
            
   QGLFormat format = theMainWindow->getBrainModelOpenGL()->format();
   if (format.directRendering()) {
      msg.append("OpenGL Direct rendering: ON\n");
   }
   else {
      msg.append("Open Direct rendering: OFF\n");
   }
   
   msg.append("Double Buffer: ");
   if (format.doubleBuffer()) msg.append("on\n");
   else msg.append("off\n");
   
   QMessageBox::information(theMainWindow, "OpenGL Information", msg);
}

/**
 * Load the caret web site in a browser window
 */
void 
GuiMainWindowHelpActions::helpMenuCaretWebSite()
{
   theMainWindow->displayWebPage("http://brainmap.wustl.edu/resources/caretnew.html");
}

/**
 * search the caret web site in a browser window
 */
void 
GuiMainWindowHelpActions::helpMenuCaretSearchWebSite()
{
   bool valid = false;
   QString s = QInputDialog::getText(theMainWindow,
                                     "Search Caret Website",
                                     QString("Search For These Words (separate with blanks)").leftJustified(100, ' '),
                                     QLineEdit::Normal,
                                     "",
                                     &valid);
   if (valid) {
      if (s.isEmpty() == false) {
         s = s.split(QRegExp("\\s+")).join("+");
         QString url("http://www.google.com/search?as_q=");
         url += s;
         url += "&num=10&hl=en&client=safari&rls=en-us&btnG=Google+Search&as_epq=&as_oq=&as_eq=&lr=&as_ft=i&as_filetype=&as_qdr=all&as_nlo=&as_nhi=&as_occt=any&as_dt=i&as_sitesearch=brainvis.wustl.edu&as_rights=&safe=images";
         theMainWindow->displayWebPage(url);
      }
   }
}

/**
 * Load the sums web site in a browser window
 */
void 
GuiMainWindowHelpActions::helpMenuSumsWebSite()
{
   theMainWindow->displayWebPage("http://sumsdb.wustl.edu/sums/index.jsp");
}

/**
 * Load the sums atlases web site in a browser window
 */
void 
GuiMainWindowHelpActions::helpMenuSumsAtlases()
{
   theMainWindow->displayWebPage("http://sumsdb.wustl.edu/sums/directory.do?dir_id=636032");
}
/**
 * Load the sums tutorials web site in a browser window
 */
void 
GuiMainWindowHelpActions::helpMenuSumsTutorials()
{
   theMainWindow->displayWebPage("http://sumsdb.wustl.edu/sums/directory.do?dir_id=707139");
}

/**
 * display caret tips dialog.
 */
void 
GuiMainWindowHelpActions::helpMenuCaretTips()
{
   static GuiCaretTipsDialog* ctd = NULL;
   
   if (ctd == NULL) {
      ctd = new GuiCaretTipsDialog(theMainWindow);
   }
   
   ctd->show();
}

/**
 * update the actions (typically called when menu is about to show)
 */
void 
GuiMainWindowHelpActions::updateActions()
{
}

