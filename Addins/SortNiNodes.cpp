#include "stdafx.h"

#include "NifCmd.h"
using namespace std;

struct SortNodeEquivalence
{
   inline bool operator()(const NiAVObjectRef& lhs, const NiAVObjectRef& rhs) const
   {
      if (!lhs) return !rhs;
      if (!rhs) return true;
      string ltype = lhs->GetType().GetTypeName();
      string rtype = rhs->GetType().GetTypeName();
      if (ltype == "NiTriShape" || ltype == "NiTriStrips")
      {
         if (ltype == rtype)
            return false;
         if (rtype == "NiTriShape" || rtype == "NiTriStrips")
            return (ltype < rtype);
         return true;
      }
      if (rtype == "NiTriShape" || rtype == "NiTriStrips")
         return false;
      if (ltype == rtype)
         return false;
      return false; 
   }
};


static bool SortNodes(NiAVObjectRef object, NiNodeRef root)
{
   bool rv = true;
   if (!object)
      return rv;

   NiNodeRef node = object;
   if (node) {
      vector<NiAVObjectRef> links = node->GetChildren();
      for (vector<NiAVObjectRef>::iterator itr = links.begin(), end = links.end(); itr != end; ++itr)
      {
         rv |= SortNodes(*itr, root);
      }
      node->SortChildren(SortNodeEquivalence());
   }
   return rv;
}

static bool SortNodes(vector<NiObjectRef>& blocks)
{
   bool rv = false;
   NiObjectRef block = blocks[0];
   if (NiNodeRef nodeObj = DynamicCast<NiNode>(block)){
      rv = SortNodes(nodeObj, nodeObj);
      nodeObj->SortChildren(SortNodeEquivalence());
   }
   return rv;
}

static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "Sort child NiNodes"; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " SortNodes <input> [-opts[modifiers]]" << endl 
            << "  Sort child NiNodes so that Geometry objects appear first." << endl
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
      if (!IsSupportedVersion(outver)) {
         cerr << "Invalid version" << endl;
         return false;
      }
      
      // Finally alter block tree
      vector<NiObjectRef> blocks = ReadNifList( current_file );
      NiObjectRef root = blocks[0];
      if (SortNodes(blocks))
         WriteNifTree(outfile, root, Niflib::NifInfo(cmdLine.outver, cmdLine.uoutver, cmdLine.uoutver));
      return true;
   }
   return false;
}

REGISTER_COMMAND(SortNodes, HelpString, ExecuteCmd);