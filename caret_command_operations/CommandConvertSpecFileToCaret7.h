#ifndef __COMMAND_CONVERT_SPEC_FILE_TO_CARET7_H__
#define __COMMAND_CONVERT_SPEC_FILE_TO_CARET7_H__

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

#include "CommandBase.h"

#include "AreaColorFile.h"
#include "BorderColorFile.h"
#include "FociColorFile.h"
class Structure;
class SpecFile;

/// class for
class CommandConvertSpecFileToCaret7 : public CommandBase {
   public:
      // constructor
      CommandConvertSpecFileToCaret7();

      // destructor
      ~CommandConvertSpecFileToCaret7();

      // get full help information
      QString getHelpInformation() const;

      // get the script builder parameters
      virtual void getScriptBuilderParameters(ScriptBuilderParameters& paramsOut) const;

   protected:
      // execute the command
      void executeCommand() throw (BrainModelAlgorithmException,
                                   CommandException,
                                   FileException,
                                   ProgramParametersException,
                                   StatisticException);

      // convert a file
      QString convertFile(const QString& inputFileName,
                          const Structure& structure);

      // convert a file
      QString convertCoordTopoToSurfaceFile(const QString& coordFileName,
                                            const QString& topoFileName,
                                            const Structure& structure,
                                            const QString& preExt);


      // read the color files
      void readColorFiles(SpecFile& sf);

      AreaColorFile areaColorFile;

      AreaColorFile borderColorFile;

      AreaColorFile fociColorFile;

      bool writeColorFilesFlag;
};

#endif // __COMMAND_CONVERT_SPEC_FILE_TO_CARET7_H__

