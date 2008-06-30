
#ifndef __FOCI_COLOR_FILE_H__
#define __FOCI_COLOR_FILE_H__

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

#include "ColorFile.h"

class FociProjectionFile;

/// class for foci colors file
class FociColorFile : public ColorFile {
   public:
      /// Constructor
      FociColorFile();
                
      /// Destructor
      ~FociColorFile();
            
      // remove colors not matching the names in the cell projection file
      void removeNonMatchingColors(const FociProjectionFile* fpf);

      /// get color indices sorted by name case insensitive
      void getColorIndicesSortedByName(const FociProjectionFile* fpf,
                                       std::vector<int>& indicesSortedByNameOut,
                                       const bool reverseOrderFlag,
                                       const bool limitToColorsUsedByDisplayedFociFlag) const;
      
   protected:
};


#endif // __FOCI_COLOR_FILE_H__

