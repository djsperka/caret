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
#include <QMenu>
#include "GuiMainWindow.h"
#include "GuiMainWindowHelpActions.h"
#include "GuiMainWindowHelpMenu.h"

/**
 * Constructor.
 */
GuiMainWindowHelpMenu::GuiMainWindowHelpMenu(GuiMainWindow* parent) :
   QMenu("Help", parent)
{
   setObjectName("GuiMainWindowHelpMenu");
   
   GuiMainWindowHelpActions* helpActions = parent->getHelpActions();
   
   QObject::connect(this, SIGNAL(aboutToShow()),
                    helpActions, SLOT(updateActions()));
                    
   addAction(helpActions->getAboutAction());

   addAction(helpActions->getAboutSplashAction());
   
   addAction(helpActions->getAboutQtAction());

   addAction(helpActions->getAboutOpenGLAction());

   addSeparator();
   
   addAction(helpActions->getCaretHelpAction());

   ///addAction(helpActions->getCaretHelpAssistantAction());

   addAction(helpActions->getOnlineCaretHelpAction());
   
   addAction(helpActions->getCaretTipsAction());
   
   addAction(helpActions->getCaretUsersListArchiveAction());
   
   addAction(helpActions->getCaretWebSiteAction());

   addAction(helpActions->getCaretSearchWebSiteAction());
   
   addSeparator();
   
   addAction(helpActions->getSumbsWebSiteAction());

   //addAction(helpActions->getSumsAtlasAction());

   //addAction(helpActions->getSumsTutorialsAction());
}

/**
 * Destructor.
 */
GuiMainWindowHelpMenu::~GuiMainWindowHelpMenu()
{
}

