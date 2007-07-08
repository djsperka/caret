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



#ifndef __GUI_MAP_FMRI_MAIN_WINDOW_ATLAS_MENU_H__
#define __GUI_MAP_FMRI_MAIN_WINDOW_ATLAS_MENU_H__

#include <qpopupmenu.h>

class GuiMapFmriMainWindow;

/// This class creates the Main Window's Atlas Menu.
class GuiMapFmriMainWindowAtlasMenu : public QPopupMenu {

   Q_OBJECT
   
   public:
      /// constructor
      GuiMapFmriMainWindowAtlasMenu(GuiMapFmriMainWindow* parent);
      
      /// destructor
      ~GuiMapFmriMainWindowAtlasMenu();
   
   private slots:
      /// called when  menu is selected
      void slotAddAtlas();
      
      /// called when  menu is selected
      void slotDeleteAtlas();
      
   private:
      /// The main window this menu is attached to
      GuiMapFmriMainWindow* mainWindow;
      
};

#endif // __GUI_MAP_FMRI_MAIN_WINDOW_ATLAS_MENU_H__

