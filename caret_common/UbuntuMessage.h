#ifndef __UBUNTU_MESSAGE_H__
#define __UBUNTU_MESSAGE_H__

#include <QString>

/**
 * Provides a warning message to Ubuntu users.
 */
class UbuntuMessage {
public:
   static QString getWarningMessage() {
      QString s = 
      "The official version of Caret software is produced by the "
      "Van Essen Laboratory at Washington University and is "
      "available from:\n"
      "   http://brainvis.wustl.edu/wiki/index.php/Caret:About\n"
      "\n"
      "Only versions of Caret downloaded from the above web site "
      "are supported by the Van Essen Laboratory."
      "\n\n"
      "This version of Caret is a Debian package and is neither "
      "distributed, nor supported, by the Van Essen Laboratory."
      "\n\n"
      "Some functionality may be missing if Caret cannot locate "
      "resources that are included in the official distribution. "
      "These functions include, but are not limited to: \n"
      "   - Help Menu Documentation\n"
      "   - Landmark Border Drawing\n"
      "   - Map Volumes to Surface via PALS Atlas\n"
      "   - Movie Making\n"
      "   - Multi-Resolution Morphing\n"
      "   - Projection of Foci via PALS Atlas\n"
      "   - Surface-Based Registration\n"
      "   - Surface Flattening\n"
      "\n"
      "Please see /usr/share/doc/caret/README.Debian for information "
      "on how to obtain the relevant resources as Debian package. "
      "This message is shown, because the resource package (caret-data) "
      "has not been found on this system.\n"
      "\n"
      "The official Caret distribution contains specific versions "
      "of third party software known to function correctly with "
      "Caret.  However, this Debian version of Caret relies on "
      "separately installed Debian packages that contain the third party "
      "software and these third party packages may not be "
      "compatible with Caret.  As a result, some functions in Caret "
      "may not operate correctly.  Faulty surface drawing is one "
      "symptom.";
      
      return s;
   }
};
#endif // __UBUNTU_MESSAGE_H__
