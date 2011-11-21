#include "stdafx.h"
#include "NifCmd.h"
#include "nifutils.h"
using namespace std;

static bool CullNodes(stringlist matches, NiNodeRef block)
{
   bool ok = false;
   NiNodeRef node = DynamicCast<NiNode>(block);
   if (node)
   {
      string name = node->GetName();
      if (wildmatch(matches, name))
      {
         node->ClearControllers();
         node->ClearChildren();
         ok = true;
      }
      vector<NiAVObjectRef> links = node->GetChildren();
      for (vector<NiAVObjectRef>::iterator itr = links.begin(), end = links.end(); itr != end; ++itr)
      {
         NiNodeRef nodeObj = DynamicCast<NiNode>(*itr);
         if (nodeObj) ok |= CullNodes(matches, nodeObj);
      }
   }
   return ok;
}

static bool CullNodes(stringlist matches, vector<NiObjectRef>& blocks)
{
   NiObjectRef block = blocks[0];
   NiNodeRef nodeObj = DynamicCast<NiNode>(block);
   return (nodeObj) ? CullNodes(matches, nodeObj) : false;
}

static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "Clean Nodes of any references or children"; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " CullChild <input> [-opts[modifiers]]" << endl 
            << "  Cleans any Nodes of any references or children. " << endl
            << endl
            << "<Switches>" << endl
            << "  e <expr>          Wild Card expressions to remove - Defaults to 'Bip01*'" << endl
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

   stringlist expr;
   string rootPath;
   for (int i = 0; i < argc; i++)
   {
      char *arg = argv[i];
      if (arg == NULL)
         continue;
      if (arg[0] == '-' || arg[0] == '/')
      {
         switch ( tolower(arg[1]) )
         {
         case 'o':
            {
               const char *param = arg+2;
               if ( param[0] == 0 && ( i+1<argc && ( argv[i+1][0] != '-' || argv[i+1][0] != '/' ) ) )
                  param = argv[++i];
               if ( param[0] == 0 )
                  break;
               if (outfile.empty())
               {
                  outfile = param;
               }
               else
               {
                  cerr << "ERROR: Input file already specified as \"" << current_file << "\"" << endl;
                  return false;
               }
            }
            break;

         case 'i':
            {
               const char *param = arg+2;
               if ( param[0] == 0 && ( i+1<argc && ( argv[i+1][0] != '-' || argv[i+1][0] != '/' ) ) )
                  param = argv[++i];
               if ( param[0] == 0 )
                  break;
               if (current_file.empty())
               {
                  current_file = param;
               }
               else
               {
                  cerr << "ERROR: Input file already specified as \"" << current_file << "\"" << endl;
                  return false;
               }
            }
            break;

         case 'e':
            {
               const char *param = arg+2;
               if ( param[0] == 0 && ( i+1<argc && ( argv[i+1][0] != '-' || argv[i+1][0] != '/' ) ) )
                  param = argv[++i];
               if ( param[0] == 0 )
                  break;
               expr.push_back(string(param));
               while (i < argc && argv[i] && argv[i][0] != '/' && argv[i][0] != '-')
                  expr.push_back(string(argv[i++]));
            }
            break;

         default:
            fputs( "ERROR: Unknown argument specified \"", stderr );
            fputs( arg, stderr );
            fputs( "\".\n", stderr );
            break;
         }
      }
      else if (current_file.empty())
      {
         current_file = arg;
      }
      else
      {
         cerr << "ERROR: Input file already specified as \"" << current_file << "\"" << endl;
         return false;
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

   if (expr.empty()) {
      expr.push_back("Bip01*");
      expr.push_back("*Helper");
      expr.push_back("BackWeapon");
      expr.push_back("SideWeapon");
      expr.push_back("MagicNode");
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
      if (CullNodes(expr, blocks))
         WriteNifTree(outfile, root, Niflib::NifInfo(cmdLine.outver, cmdLine.uoutver, cmdLine.uoutver));
      return true;
   }
   return false;
}

REGISTER_COMMAND(CullChild, HelpString, ExecuteCmd);