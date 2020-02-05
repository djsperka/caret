
#ifndef __GUI_MAIN_WINDOW_HELP_ACTIONS_H__
#define __GUI_MAIN_WINDOW_HELP_ACTIONS_H__

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

#include <QObject>

class GuiHelpAssistantWindow;
class QAction;
class QWidget;

/// the main window's help actions
class GuiMainWindowHelpActions : public QObject {
   Q_OBJECT
   
   public:
      // constructor
      GuiMainWindowHelpActions(QWidget* parent);
      
      // destructor
      ~GuiMainWindowHelpActions();
   
      /// about caret action
      QAction* getAboutAction() { return aboutAction; }
      
      /// about QT action
      QAction* getAboutQtAction() { return aboutQtAction; }

      /// about SPLASH action
      QAction* getAboutSplashAction() { return aboutSplashAction; }

      /// about OpenGL action
      QAction* getAboutOpenGLAction() { return aboutOpenGLAction; }
      
      /// caret help action
      QAction* getCaretHelpAction() { return caretHelpAction; }
      
      /// caret help action
      QAction* getCaretHelpAssistantAction() { return caretHelpAssistantAction; }
      
      /// caret online help action
      QAction* getOnlineCaretHelpAction() { return caretOnlineHelpAction; }
      
      /// caret users list email archive help action
      QAction* getCaretUsersListArchiveAction() { return caretUsersListArchiveAction; }
      
      /// caret tips action
      QAction* getCaretTipsAction() { return caretTipsAction; }
      
      /// caret web site action
      QAction* getCaretWebSiteAction() { return caretWebSiteAction; }
      
      /// search caret web site action
      QAction* getCaretSearchWebSiteAction() { return caretSearchWebSiteAction; }
      
      /// sums web site action
      QAction* getSumbsWebSiteAction() { return sumbsWebSiteAction; }
      
      /// sums atlas action
      QAction* getSumsAtlasAction() { return sumsAtlasAction; }
      
      /// sums tutorial action
      QAction* getSumsTutorialsAction() { return sumsTutorialsAction; }
      
   public slots:
      /// update the actions (typically called when menu is about to show)
      void updateActions();
      
      /// Called when About chosen from Help menu.
      void helpMenuAbout();
      
      /// display information about the qt version
      void helpMenuAboutQT();

      /// display splash info
      void helpMenuAboutSplash();
      
      /// display information about the OpenGL version
      void helpMenuAboutOpenGL();
      
      /// display caret help browser
      void helpMenuCaretHelp();

      /// display caret help assistant
      void helpMenuCaretHelpAssistant();

      /// display online caret help
      void helpMenuOnlineCaretHelp();
      
      /// display caret users email archive
      void slotCaretUsersListArchiveAction();
      
      /// display caret tips dialog
      void helpMenuCaretTips();

      /// display caret web site in a browser
      void helpMenuCaretWebSite();
      
      /// display caret search web site in a browser
      void helpMenuCaretSearchWebSite();
      
      /// search SuMS database web site in a browser 
      void helpMenuSumsWebSite();
      
      /// display SuMS atalses database web site in a browser 
      void helpMenuSumsAtlases();
      
      /// display SuMS tutorials database web site in a browser 
      void helpMenuSumsTutorials();
   
   protected:
      /// about caret action
      QAction* aboutAction;
      
      /// about QT action
      QAction* aboutQtAction;

      /// about Splash action
      QAction* aboutSplashAction;

      /// about OpenGL action
      QAction* aboutOpenGLAction;
      
      /// caret help action
      QAction* caretHelpAction;
      
      /// caret help assistant action
      QAction* caretHelpAssistantAction;
      
      /// caret users list archive action
      QAction* caretUsersListArchiveAction;
      
      /// caret online help action
      QAction* caretOnlineHelpAction;
      
      /// caret tips action
      QAction* caretTipsAction;
      
      /// caret web site action
      QAction* caretWebSiteAction;
      
      /// caret search web site action
      QAction* caretSearchWebSiteAction;
      
      /// sums web site action
      QAction* sumbsWebSiteAction;
      
      /// sums atlas action
      QAction* sumsAtlasAction;
      
      /// sums tutorial action
      QAction* sumsTutorialsAction;
      
      /// Qt Assistant Client for Helpl
      GuiHelpAssistantWindow* assistantWindow;
};

#endif // __GUI_MAIN_WINDOW_HELP_ACTIONS_H__

