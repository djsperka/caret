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

#ifndef __GUI_VECTOR_FILE_COMBO_BOX_H__
#define __GUI_VECTOR_FILE_COMBO_BOX_H__

#include <vector>
#include <QComboBox>

class VectorFile;

/// class for combo box used for selecting a vector file
class GuiVectorFileComboBox : public QComboBox {
   public:
      /// Constructor
      GuiVectorFileComboBox(QWidget* parent = 0,
                            const char* name = 0);
                              
      /// Destructor
      ~GuiVectorFileComboBox();
      
      /// get the selected vector file
      VectorFile* getSelectedVectorFile() const;
      
      /// get the selected vector file index
      int getSelectedVectorFileIndex() const;
      
      /// set the selected vector file
      void setSelectedVectorFile(const VectorFile* tf);
      
      /// update the items in the combo box
      void updateComboBox();
      
   private:
      /// keeps track of topology files
      std::vector<VectorFile*> vectorFilePointers;
};

#endif // __GUI_VECTOR_FILE_COMBO_BOX_H__

