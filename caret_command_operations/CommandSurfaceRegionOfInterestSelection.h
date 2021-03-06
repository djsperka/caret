
#ifndef __COMMAND_SURFACE_REGION_OF_INTEREST_SELECTION_H__
#define __COMMAND_SURFACE_REGION_OF_INTEREST_SELECTION_H__

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

#include "BrainModelSurfaceROINodeSelection.h"
#include "CommandBase.h"

/// class for
class CommandSurfaceRegionOfInterestSelection : public CommandBase {
   public:
      // constructor 
      CommandSurfaceRegionOfInterestSelection();
      
      // destructor
      ~CommandSurfaceRegionOfInterestSelection();
      
      // get full help information
      QString getHelpInformation() const;
      
      // get the script builder parameters
      virtual void getScriptBuilderParameters(ScriptBuilderParameters& paramsOut) const;

   protected:
      // extent xyz limit
      void extentLimit(const BrainModelSurface* bms,
                       BrainModelSurfaceROINodeSelection* roi,
                       const QString& operationName,
                       const float extentValue);
                       
      // focus xyz limit
      void focusLimit(const BrainModelSurface* bms,
                      BrainModelSurfaceROINodeSelection* roi,
                      const QString& operationName);
                      
      // execute the command
      void executeCommand() throw (BrainModelAlgorithmException,
                                   CommandException,
                                   FileException,
                                   ProgramParametersException,
                                   StatisticException);

      // get the selection type
      BrainModelSurfaceROINodeSelection::SELECTION_LOGIC
         getSelectionType(const QString& selectionTypeName) const 
            throw (CommandException);
};

#endif // __COMMAND_SURFACE_REGION_OF_INTEREST_SELECTION_H__

