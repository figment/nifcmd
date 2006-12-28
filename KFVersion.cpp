#include "stdafx.h"

#include "NifCmd.h"
#include "nifutils.h"
#include "obj/NiTriBasedGeom.h"
#include "obj/NiTriBasedGeomData.h"
#include "obj/NiTriShape.h"
#include "obj/NiTriStrips.h"
#include "obj/NiControllerSequence.h"
using namespace std;

static void AssignKFPriority(vector<NiControllerSequenceRef>& sequences, int priority)
{
   for (vector<NiControllerSequenceRef>::iterator itr = sequences.begin(); itr != sequences.end(); ++itr)
   {
      NiControllerSequenceRef seq = (*itr);
      int nctrl = seq->GetNumControllers();
      for (int i = 0; i < nctrl; ++i) {
         seq->SetControllerPriority(i, priority);
      }
   }
}

static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "KFVersion - Change version of a KF file."; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " KFVersion [-opts[modifiers]]" << endl 
            << "  Change version of a KF file." << endl
            << endl
            << "<Options>" << endl
            << "  p #               Priority for Oblivion KF Controller upgrades" << endl
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
   int argc = cmdLine.argc;
   char **argv = cmdLine.argv;
   int priority = 50;

   Matrix44 tm = Matrix44::IDENTITY;

   list<NifCmd *> plugins;
   for (int i = 0; i < argc; i++)
   {
      char *arg = argv[i];
      if (arg == NULL)
         continue;
      if (arg[0] == '-' || arg[0] == '/')
      {
         switch (tolower(arg[1]))
         {
         case 'p': // priority
            {
               const char *param = arg+2;
               argv[i] = NULL;
               if ( param[0] == 0 && ( i+1<argc && ( argv[i+1][0] != '-' || argv[i+1][0] != '/' ) ) ) {
                  param = argv[++i];
                  argv[i] = NULL;
               }
               if ( param[0] == 0 )
                  break;

               char *end;
               priority = strtol(param, &end, 0);
            }
            break;

         default:
            fputs( "ERROR: Unknown argument specified \"", stderr );
            fputs( arg, stderr );
            fputs( "\".\n", stderr );
            return false;
         }
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
      if (!IsSupportedVersion(cmdLine.outver)) {
         cerr << "Invalid version" << endl;
         return false;
      }

      // Finally alter block tree
      vector<NiObjectRef> blocks = ReadNifList( current_file );
      NiObjectRef root = blocks[0];

      AssignKFPriority(DynamicCast<NiControllerSequence>(blocks), priority);

      WriteNifTree(outfile, root, Niflib::NifInfo(cmdLine.outver, cmdLine.uoutver, cmdLine.uoutver));
      return true;
   }
   return true;
}

REGISTER_COMMAND(KFVersion, HelpString, ExecuteCmd);