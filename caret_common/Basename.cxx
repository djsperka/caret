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

#include "QGlobalStatic"

// The "basename()" function on some older Linux versions does not work 
// correctly.  It leaves a leading "/" on the returned value.
// Mac OSX has neither basename nor dirname.
//

#if !defined(Q_OS_MAC) && !defined(Q_OS_WIN32)
#include <libgen.h>
#endif

#include <string.h>

#if defined(Q_OS_LINUX) || defined(Q_OS_GLIBC) || defined(Q_OS_FREEBSD_KERNEL) || defined(Q_OS_HURD)
static const char *period = ".";
#endif
#ifdef Q_OS_MAC
static const char *period = ".";
#endif
#ifdef Q_OS_WIN32
static const char *period = ".";
#endif
#ifdef Q_OS_FREEBSD
static const char *period = ".";
#endif

const char*
Basename(char* path)
{

#if defined(Q_OS_IRIX) || defined(Q_OS_SOLARIS)
   return basename(path);
#else  
 
   //
   // return "." if path is NULL or zero length
   //
   if (path == NULL) {
      return period;
   }
   const int len = strlen(path);
   if (len == 0) {
      return period;
   }
   
   //
   // if path is merely "/", return it
   //
   if ((len == 1) && (path[0] == '/')) {
      return path;
   }
   
   //
   // remove any trailing "/"
   //
   if ((path[len-1] == '/') || (path[len-1] == '\\')) {
      path[len-1] = '\0';
   }
   
   //
   // return anything after the last "/"
   //
   char* lastSlash1 = strrchr(path, '/');
   char* lastSlash2 = strrchr(path, '\\');
   if (lastSlash1 != NULL) {
      if (lastSlash2 != NULL) {
         if (lastSlash2 > lastSlash1) {
            return (lastSlash2 + 1);
         }
         else {
            return (lastSlash1 + 1);
         }
      }
      else {
         return (lastSlash1 + 1);
      }
   }
   else if (lastSlash2 != NULL) {
      return (lastSlash2 + 1);
   }
   return path;
   
#endif
}

//------------------------------------------------------------

const char* Dirname(char* path)
{
#if defined(Q_OS_MAC) || defined(Q_OS_WIN32)
   if (path == NULL) {
      return period;
   }
   else if (strlen(path) == 0) {
      return period;
   }
   else if (strcmp(path, "/") == 0) {
      return path;
   }
   else if (strcmp(path, ".") == 0) {
      return period;
   }
   else if (strcmp(path, "..") == 0) {
      return period;
   }

   const int length = strlen(path);
   if ((path[length - 1] == '/') || (path[length - 1] == '\\')) {
      path[length - 1] = '\0';
   }
   for (int i = (strlen(path) - 1); i >= 0; i--) {
      if ((path[i] == '/') || (path[i] == '\\')) {
         if (i == 0) {
            if (path[0] == '/') {
               path[1] = '\0';
            }
         }
         else {
            path[i] = '\0';
         }
         return path;
      }
   }
   
   return period;
#else  
   return dirname(path);
#endif
}
