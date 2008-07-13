#include "stdafx.h"

#include "NifCmd.h"
using namespace std;

static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "About - Help about this program."; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " about" << endl 
              << "  About this program." << endl
              << endl
            ;
      }
      break;
   }
}


static bool ExecuteCmd(NifCmdLine &cmdLine)
{
   cout << "Copyright (c) 2006-2008, NIF File Format Library and Tools" << endl
        << "All rights reserved." << endl
        << endl;
   cout <<
	 _T("Redistribution and use in source and binary forms, with or without\n"
	    "modification, are permitted provided that the following conditions\n"
	    "are met:\n"
	    "\n"
	    "  * Redistributions of source code must retain the above copyright\n"
	    "     notice, this list of conditions and the following disclaimer.\n"
	    "\n"
	    "   * Redistributions in binary form must reproduce the above\n"
	    "     copyright notice, this list of conditions and the following\n"
	    "     disclaimer in the documentation and/or other materials provided\n"
	    "     with the distribution.\n"
	    "\n"
	    "   * Neither the name of the NIF File Format Library and Tools\n"
	    "     project nor the names of its contributors may be used to endorse\n"
	    "     or promote products derived from this software without specific\n"
	    "     prior written permission.\n"
		"\n"
		);

   cout <<
   _T("THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n"
      "\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n"
      "LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS\n"
      "FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE\n"
      "COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,\n"
      "INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,\n"
      "BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n"
      "LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER\n"
      "CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\n"
      "LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN\n"
      "ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE\n"
      "POSSIBILITY OF SUCH DAMAGE.\n"
      );

   cout <<
   _T("\n"
	   "NOTICE:\n"
       "  Parts of NifCmd (ie. included libraries) might be subject to\n"
	   "  other licenses. This license only applies to original NifCmd\n"
	   "  sources.\n"
	   );

   cout <<
	_T("\n"
	   "\"NifCmd\" uses Havok�.  �Copyright 1999-2008 Havok.com Inc. (and its Licensors).\n"
	   "  All Rights Reserved.  See www.havok.com for details.\n"
	   );

   return true;
}

REGISTER_COMMAND(About, HelpString, ExecuteCmd);