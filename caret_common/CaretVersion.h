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


#ifndef __CARET_VERSION_H__
#define __CARET_VERSION_H__

#include <QString>

#ifdef CARET_BUILDID
# undef  SHSH
# undef  SHSHSH
# undef  CARET_BUILDID_STRING
# define SHSH(x)   #x
# define SHSHSH(x) SHSH(x)
# define CARET_BUILDID_STRING  SHSHSH(CARET_BUILDID)   /* now in "quotes" */
#else
# undef  CARET_BUILDID_STRING
#endif

/// class for caret version information
class CaretVersion {
   public:
      /// Version 5.65, January 23, 2012
      /// get the version of caret as a QString
      static QString getCaretVersionAsString() { return "5.66" " " CARET_BUILDID_STRING; }
};

#endif // __CARET_VERSION_H__
