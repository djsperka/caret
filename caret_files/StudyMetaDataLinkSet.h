
#ifndef __STUDY_META_DATA_LINK_SET_H__
#define __STUDY_META_DATA_LINK_SET_H__

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

#include <vector>

#include "StudyMetaDataLink.h"

class QDomDocument;
class QDomElement;
class QDomNode;

/// class for accessing and storing a group of StudyMetaDataLink
class StudyMetaDataLinkSet {
   public:
      // constructor
      StudyMetaDataLinkSet();
      
      // destructor
      ~StudyMetaDataLinkSet();
      
      // add a StudyMetaDataLink
      void addStudyMetaDataLink(const StudyMetaDataLink& smdl);
      
      // remove all links
      void clear();
      
      /// get the number of study meta data links
      int getNumberOfStudyMetaDataLinks() const { return links.size(); }
      
      // get a StudyMetaDataLink
      StudyMetaDataLink getStudyMetaDataLink(const int indx) const;
      
      // get a pointer to a StudyMetaDataLink
      StudyMetaDataLink* getStudyMetaDataLinkPointer(const int indx);
      
      // get all linked PubMed IDs
      void getAllLinkedPubMedIDs(std::vector<QString>& pmidsOut) const;
      
      // remove a study meta data link
      void removeStudyMetaDataLink(const int indx);
   
      // set a study meta data link
      void setStudyMetaDataLink(const int indx, const StudyMetaDataLink& smdl);
      
      /// get the entire link set in an "coded" text form
      QString getLinkSetAsCodedText() const;
      
      /// set the link set from "coded" text form
      void setLinkSetFromCodedText(const QString& txt);
      
      // called to read from an XML structure
      void readXML(QDomNode& node) throw (FileException);
      
      // called to write to an XML structure
      void writeXML(QDomDocument& xmlDoc,
                    QDomElement&  parentElement) const throw (FileException);
   
   protected:
      /// the StudyMetaDataLink
      std::vector<StudyMetaDataLink> links;
      
      //
      //----- tags for reading and writing 
      //
      /// tag for reading and writing study metadata
      static const QString tagStudyMetaDataLinkSet;
      
      /// get the link separator for when stored as a string
      static const QString encodedTextLinkSeparator;  
      
      friend class CellBase;
};

#ifdef __STUDY_META_DATA_LINK_SET_MAIN__
      const QString StudyMetaDataLinkSet::tagStudyMetaDataLinkSet = "StudyMetaDataLinkSet";
      const QString StudyMetaDataLinkSet::encodedTextLinkSeparator = ":::::"; 
#endif // __STUDY_META_DATA_LINK_SET_MAIN__

#endif // __STUDY_META_DATA_LINK_SET_H__
