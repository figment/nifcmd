#include "stdafx.h"

#include "NifCmd.h"
using namespace std;

static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "NOP - Read/write with no modifications."; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " nop [-opts[modifiers]]" << endl 
            << "  Simply read and write the file back out." << endl
            << endl
            << "<Switches>" << endl
            << "  i <path>          Input File" << endl
            << "  o <path>          Output File - Defaults to input file with '-out' appended" << endl
            << "  v x.x.x.x         Nif Version to write as - Defaults to input version" << endl
            << endl
            ;
      }
      break;
   }
}


static bool ExecuteCmd(NifCmdLine &cmdLine)
{
   string current_file = cmdLine.current_file;
   string outfile = cmdLine.outfile;
   unsigned outver = cmdLine.outver;
   int argc = cmdLine.argc;
   char **argv = cmdLine.argv;

   list<NifCmd *> plugins;

   for (int i = 0; i < argc; i++)
   {
      char *arg = argv[i];
      if (arg == NULL)
         continue;
      if (arg[0] == '-' || arg[0] == '/')
      {
         fputs( "ERROR: Unknown argument specified \"", stderr );
         fputs( arg, stderr );
         fputs( "\".\n", stderr );
      }
      else if (current_file.empty())
      {
         current_file = arg;
      }
   }
   if (current_file.empty()){
      NifCmd::PrintHelp();
      return false;
   }
   if (outfile.empty()) {
      char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH];
      _splitpath(current_file.c_str(), drive, dir, fname, ext);
      strcat(fname, "-out");
      _makepath(path, drive, dir, fname, ext);
      outfile = path;
   }

   unsigned int ver = GetNifVersion( current_file );
   if ( ver == VER_UNSUPPORTED ) cout << "unsupported...";
   else if ( ver == VER_INVALID ) cout << "invalid...";
   else 
   {
      if (!IsSupportedVersion(outver))
         outver = ver;

      // Finally alter block tree
      vector<NiObjectRef> blocks = ReadNifList( current_file );
      NiObjectRef root = blocks[0];
      WriteNifTree(outfile, root, Niflib::NifInfo(cmdLine.outver, cmdLine.uoutver, cmdLine.uoutver));
      return true;
   }
   return true;
}

REGISTER_COMMAND(Nop, HelpString, ExecuteCmd);