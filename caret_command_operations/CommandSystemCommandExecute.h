
#ifndef __COMMAND_SYSTEM_COMMAND_EXECUTE_H__
#define __COMMAND_SYSTEM_COMMAND_EXECUTE_H__

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

#include <QStringList>

#include "CommandBase.h"

/// class for
class CommandSystemCommandExecute : public CommandBase {
   public:
      // constructor 
      CommandSystemCommandExecute();
      
      // destructor
      ~CommandSystemCommandExecute();
      
      // get full help information
      QString getHelpInformation() const;
      
      // get the script builder parameters
      virtual void getScriptBuilderParameters(ScriptBuilderParameters& paramsOut) const;
      
      /// get the system command
      QString getSystemCommandName() const { return systemCommandName; }
      
      ///get  the command parameters
      QStringList getSystemCommandParameters() const { return systemCommandParameters; }
      
   protected:
      // execute the command
      void executeCommand() throw (BrainModelAlgorithmException,
                                   CommandException,
                                   FileException,
                                   ProgramParametersException,
                                   StatisticException);

      /// the system command
      QString systemCommandName;
      
      /// the command parameters
      QStringList systemCommandParameters;
};

#endif // __COMMAND_SYSTEM_COMMAND_EXECUTE_H__

