#include "stdafx.h"

#include "NifCmd.h"
#include "nifutils.h"
using namespace std;

static bool UnparentNodes(stringlist expr, NiAVObjectRef object, NiNodeRef root)
{
   bool rv = false;
   if (!object)
      return rv;

   NiNodeRef node = object;
   if (node) {
      vector<NiAVObjectRef> links = node->GetChildren();
      for (vector<NiAVObjectRef>::iterator itr = links.begin(), end = links.end(); itr != end; ++itr)
      {
         rv |= UnparentNodes(expr, *itr, root);
      }
   }
   string name = object->GetName();
   if (wildmatch(expr, name))
   {
      NiNodeRef parent = object->GetParent();
      if (parent && parent != root)
      {
         Matrix44 t = object->GetWorldTransform();
         bool hadSkin;
         if (node){
            vector<NiAVObjectRef> links = node->GetChildren();
            node->ClearChildren();
            hadSkin = node->IsSkinInfluence();
            node->SetSkinFlag(false); // remove to bypass remove child check
         }
         object->ClearControllers();
         parent->RemoveChild(object);
         root->AddChild(object);
         Matrix44 pt = root->GetWorldTransform();
         Matrix44 lt = object->GetLocalTransform() * t * pt.Inverse();

         Vector3 translate; Matrix33 rotation; float scale;
         lt.Decompose( translate, rotation, scale );
         object->SetLocalTranslation(translate);
         object->SetLocalRotation(rotation);
         object->SetLocalScale(1.0F);
         if (node)
            node->SetSkinFlag(hadSkin); // reset to 
         rv = true;
      }
      else if (node && !node->IsSkinInfluence())
      {
         parent->RemoveChild(object);
      }
   }
   return rv;
}

static bool UnparentNodes(stringlist expr, vector<NiObjectRef>& blocks)
{
   bool rv = false;
   NiObjectRef block = blocks[0];
   if (NiNodeRef nodeObj = DynamicCast<NiNode>(block)){
      rv = UnparentNodes(expr, nodeObj, nodeObj);
   }
   return rv;
}


static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "Strip out child NiNodes and collapse to Scene"; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " Unparent <input> [-opts[modifiers]]" << endl 
            << "  Unparent matching nodes to Scene Node." << endl
            << endl
            << "<Switches>" << endl
            << "  e <expr>          Wild Card expressions to remove - Defaults to 'Bip01* SideWeapon BackWeapon MagicNode *Helper'" << endl
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
   unsigned uoutver = cmdLine.uoutver;
   int argc = cmdLine.argc;
   char **argv = cmdLine.argv;

   stringlist expr;
   for (int i = 0; i < argc; i++)
   {
      char *arg = argv[i];
      if (arg == NULL)
         continue;
      if (arg[0] == '-' || arg[0] == '/')
      {
         switch (arg[0])
         {
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
      if (!IsSupportedVersion(outver)) {
         cerr << "Invalid version" << endl;
         return false;
      }

      // Finally alter block tree
      vector<NiObjectRef> blocks = ReadNifList( current_file );
      NiObjectRef root = blocks[0];
      if (UnparentNodes(expr, blocks))
      {
         WriteNifTree(outfile, root, Niflib::NifInfo(cmdLine.outver, cmdLine.uoutver, cmdLine.uoutver));
      }
      return true;
   }
   return false;
}

REGISTER_COMMAND(Unparent, HelpString, ExecuteCmd);