#include "stdafx.h"

#include "NifCmd.h"
using namespace std;

static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "Chain multiple commands together"; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " Chain <cmd1> <cmd2> [-opts[modifiers]]" << endl 
            << "  Chain multiple commands together in sequence" << endl
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
      else {
         argv[i] = NULL;
         if (_tcsicmp(arg, "all") == 0) {
            plugins = NifCmd::GetNifCmds();
            plugins.erase( std::find(plugins.begin(), plugins.end(), NifCmd::GetNifCmd("Chain")));
         }
         else if (NifCmd* p = NifCmd::GetNifCmd(arg))
         {
            plugins.push_back(p);
         }
         else
         {
            cerr << "ERROR: Command \"" << current_file << "\" is not a valid name." << endl;
            return false;
         }
      }
   }
   if (current_file.empty()){
      NifCmd::PrintHelp();
      return false;
   }
   if (plugins.empty()){
      cerr << "ERROR: No commands have been specified." << endl;
      return false;
   }
   if (outfile.empty()) {
      char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH];
      _splitpath(current_file.c_str(), drive, dir, fname, ext);
      strcat(fname, "-out");
      _makepath(path, drive, dir, fname, ext);
      outfile = path;
   }
   for (list<NifCmd *>::iterator itr = plugins.begin(), end = plugins.end(); itr != end; ++itr) {
      NifCmd *p = (*itr);
      if (!p->ExecuteCmd(cmdLine))
         return false;
      cmdLine.current_file = cmdLine.outfile; // first command reads/writes subsequent all use the output 
   }
   return true;
}

REGISTER_COMMAND(Chain, HelpString, ExecuteCmd);