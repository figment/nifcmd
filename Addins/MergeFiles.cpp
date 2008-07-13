#include "stdafx.h"
#include "NifCmd.h"

#include "niflib.h"
#include "obj/NiObject.h"
#include "obj/NiNode.h"
#include "obj/NiControllerSequence.h"

using namespace Niflib;

#include <iomanip>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "Merge .NIF and .KF files together"; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " merge [-opts[modifiers]] <NIF> <KF> <output NIF>" << endl 
            << "  Merge .NIF and .KF files together." << endl
            << endl
            << "<Switches>" << endl
            << "  <nif>           Main NIF File" << endl
            << "  <kf>            KF File to merge into NIF" << endl
            << "  <output nif>    Output File - Defaults to input file with '-out' appended" << endl
            << "  v x.x.x.x       Nif Version to write as - Defaults to input version" << endl
            << endl
            ;
      }
      break;
   }
}

static bool ExecuteCmd(NifCmdLine &cmdLine)
{
   string first_file = cmdLine.current_file;
   string second_file;
   string outfile = cmdLine.outfile;
   unsigned outver = cmdLine.outver;
   int argc = cmdLine.argc;
   char **argv = cmdLine.argv;

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
      else if (first_file.empty())
      {
         first_file = arg;
      }
      else if (second_file.empty())
      {
         second_file = arg;
      }
      else if (outfile.empty())
      {
         outfile = arg;
      }
   }
   if (first_file.empty() || second_file.empty()){
      NifCmd::PrintHelp();
      return false;
   }
   if (outfile.empty()) {
      char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH];
      _splitpath(first_file.c_str(), drive, dir, fname, ext);
      strcat(fname, "-out");
      _makepath(path, drive, dir, fname, ext);
      outfile = path;
   }
   unsigned int ver = GetNifVersion( first_file );
   if ( ver == VER_UNSUPPORTED ) { 
      cerr << "unsupported..."; return false; 
   } else if ( ver == VER_INVALID ) { 
      cerr << "invalid..."; return false; 
   }
   unsigned int ver2 = GetNifVersion( second_file );
   if ( ver2 == VER_UNSUPPORTED ) { 
      cerr << "unsupported..."; return false; 
   } else if ( ver2 == VER_INVALID ) { 
      cerr << "invalid..."; return false; 
   }

   if (!IsSupportedVersion(outver))
      outver = ver;

   if (stricmp(PathFindExtension(first_file.c_str()), ".NIF") != 0) {
      cerr << "invalid extension on main file."; return false;
   }

   // Finally alter block tree
   NiObjectRef left = ReadNifTree( first_file );
   NiNodeRef target_node = DynamicCast<NiNode>( left );
   if (NULL == target_node) {
      cerr << "invalid file."; return false;
   }

   LPCSTR ext = PathFindExtension(second_file.c_str());
   if (stricmp(ext, ".NIF") == 0){
      cerr << "Invalid KF file."; return false;
      //if ( NiAVObjectRef right_node = DynamicCast<NiAVObject>( ReadNifTree(second_file) ) )
      //{
      //   MergeNifTrees( target_node, right_node, outver);
      //   WriteNifTree(outfile, target_node, outver);
      //}
   } else if (stricmp(ext, ".KF") == 0){
      vector<NiControllerSequenceRef> kf = DynamicCast<NiControllerSequence>( ReadNifList(second_file) );
      if (kf.empty()) {
         cerr << "Invalid KF file."; return false;
      } else {
         cout << "Attempting merge..." << endl;
         for (vector<NiControllerSequenceRef>::iterator it = kf.begin(); it != kf.end(); ++it) {
            MergeNifTrees( target_node, (*it), outver);
         }
         WriteNifTree(outfile, target_node, Niflib::NifInfo(cmdLine.outver, cmdLine.uoutver, cmdLine.uoutver));
      }
   }
   return true;
}

REGISTER_COMMAND(Merge, HelpString, ExecuteCmd);