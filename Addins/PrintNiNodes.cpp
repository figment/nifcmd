#include "stdafx.h"

#include "NifCmd.h"
using namespace std;

static bool PrintNodes(NiAVObjectRef object, NiNodeRef root)
{
   bool rv = true;
   if (!object)
      return rv;

   NiNodeRef node = object;
   if (node) {
      vector<NiAVObjectRef> links = node->GetChildren();
      for (vector<NiAVObjectRef>::iterator itr = links.begin(), end = links.end(); itr != end; ++itr)
      {
         NiAVObjectRef avobj = (*itr);
         string name = avobj->GetName();
         Matrix33 m3 = avobj->GetLocalRotation();
         Vector3 v3 = avobj->GetLocalTranslation();
         float s = avobj->GetLocalScale();

         cout << "-------------------------------" << endl;
         cout << "Name:  " << name << endl;
         cout << "Local Transform:" << endl;
         cout << "Pos:   " << v3 << endl; 
         cout << "Rot:   " << m3 << endl;
         cout << "Scale: " << s << endl;
         cout << endl;

         Matrix44 m4 = avobj->GetWorldTransform();
         m4.Decompose(v3, m3, s);

         cout << "World Transform:" << endl;
         cout << "Pos:   " << v3 << endl; 
         cout << "Rot:   " << m3 << endl;
         cout << "Scale: " << s << endl;
         cout << endl;

         Matrix44 lm4(Vector3(0,1,0), Matrix33::IDENTITY, 1.0f);

         Matrix44 tm = lm4 * m4;
         tm.Decompose(v3, m3, s);
         cout << "End World Transform:" << endl;
         cout << "Pos:   " << v3 << endl; 
         cout << "Rot:   " << m3 << endl;
         cout << "Scale: " << s << endl;
         cout << endl;

         rv |= PrintNodes(*itr, root);
      }
   }
   return rv;
}

static bool PrintNodes(vector<NiObjectRef>& blocks)
{
   bool rv = false;
   NiObjectRef block = blocks[0];
   if (NiNodeRef nodeObj = DynamicCast<NiNode>(block)){
      rv = PrintNodes(nodeObj, nodeObj);
   }
   return rv;
}

static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "Dump NiNodes transform data"; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " PrintNodes <input> [-opts[modifiers]]" << endl 
            << "  Print NiNodes Geometry data" << endl
            << endl
            << "<Switches>" << endl
            << "  i <path>          Input File" << endl
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
   unsigned int ver = GetNifVersion( current_file );
   if ( ver == VER_UNSUPPORTED ) cout << "unsupported...";
   else if ( ver == VER_INVALID ) cout << "invalid...";
   else 
   {
      if (outver == VER_INVALID)
         outver = ver;

      if (!IsSupportedVersion(outver)) {
         cerr << "Invalid version" << endl;
         return false;
      }

      // Finally alter block tree
      vector<NiObjectRef> blocks = ReadNifList( current_file );
      NiObjectRef root = blocks[0];
      PrintNodes(blocks);
      return true;
   }
   return false;
}

REGISTER_COMMAND(PrintNodes, HelpString, ExecuteCmd);