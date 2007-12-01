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

#include "CommandScriptVariableSet.h"
#include "FileFilters.h"
#include "ProgramParameters.h"
#include "ScriptBuilderParameters.h"

/**
 * constructor.
 */
CommandScriptVariableSet::CommandScriptVariableSet()
   : CommandBase("-script-variable-set",
                 "SCRIPT VARIABLE SET")
{
}

/**
 * destructor.
 */
CommandScriptVariableSet::~CommandScriptVariableSet()
{
}

/**
 * get the script builder parameters.
 */
void 
CommandScriptVariableSet::getScriptBuilderParameters(ScriptBuilderParameters& paramsOut) const
{
   paramsOut.clear();
   paramsOut.addString("Variable Name", "$name");
   paramsOut.addString("Variable Value", "value");
}

/**
 * get full help information.
 */
QString 
CommandScriptVariableSet::getHelpInformation() const
{
   QString helpInfo =
      (indent3 + getShortDescription() + "\n"
       + indent6 + parameters->getProgramNameWithoutPath() + " " + getOperationSwitch() + "  \n"
       + indent9 + "<variable-name-beginning-with-dollar-symbol>\n"
       + indent9 + "<variable-value>\n"
       + indent9 + " \n"
       + indent9 + "Set a variable for use in a caret script file. \n"
       + indent9 + " \n"
       + indent9 + "The variable name must begin with a \"$\" symbol. \n"
       + indent9 + "\n");
      
   return helpInfo;
}

/**
 * execute the command.
 */
void 
CommandScriptVariableSet::executeCommand() throw (BrainModelAlgorithmException,
                                     CommandException,
                                     FileException,
                                     ProgramParametersException,
                                     StatisticException)
{
   variableName = parameters->getNextParameterAsString("Variable Name");
   variableValue = parameters->getNextParameterAsString("Variable Value");
   checkForExcessiveParameters();
   
   if (variableName.isEmpty()) {
      throw CommandException("Variable Name is empty.");
   }
   if (variableName[0] != '$') {
      throw CommandException("Variable Name ("
                             + variableName
                             + ") must begin with \"$\" symbol.");
   }
   
   if (variableValue.isEmpty() == false) {
      if (variableValue[0] == '$') {
         throw CommandException("Variable Name ("
                                + variableName
                                + ") cannot have a value that is another variable.");
      }
   }
}

      

