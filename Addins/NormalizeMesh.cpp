#include "stdafx.h"

#include "NifCmd.h"
#include "obj/NiTriBasedGeom.h"
#include "obj/NiTriBasedGeomData.h"
#include "obj/NiTriShape.h"
#include "obj/NiTriStrips.h"

using namespace std;

static void TransformVector3(Matrix44& tm, vector<Vector3>& pts)
{
   Matrix44::IDENTITY;
   for (vector<Vector3>::iterator itr = pts.begin(); itr != pts.end(); ++itr)
   {    
      Matrix44 m4(*itr, Matrix33::IDENTITY, 1.0f);
      Matrix44 ntm = m4 * tm;
      Vector3 v  = ntm.GetTranslation();
      (*itr) = v;
   }
}
static void NormalizeMeshTransforms(vector<NiTriBasedGeomRef>& shapes)
{
   for (vector<NiTriBasedGeomRef>::iterator itr = shapes.begin(); itr != shapes.end(); ++itr)
   {
      NiTriBasedGeomRef shape = (*itr);
      NiTriBasedGeomDataRef data = shape->GetData();
      vector<Vector3> verts = data->GetVertices();
      vector<Vector3> norms = data->GetNormals();
      int nuvsets = data->GetUVSetCount();
      vector< vector<TexCoord> > uvSets;
      uvSets.resize(nuvsets);
      for (int i=0; i<nuvsets; ++i)
         uvSets[i] = data->GetUVSet(i);

      Matrix44 ltm = shape->GetLocalTransform();
      Matrix44 invtm = ltm.Inverse();
      Matrix44 tm = ltm * invtm;
      shape->SetLocalTransform(tm);

      TransformVector3(ltm, verts);
      TransformVector3(ltm, norms);

      data->SetVertices(verts);
      data->SetNormals(norms);
      data->SetUVSetCount(nuvsets);
      for (int i=0; i<nuvsets; ++i)
         data->SetUVSet(i, uvSets[i]);
   }
}

static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "NormalizeMesh - Normalize mesh transforms."; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " NormalizeMesh [-opts[modifiers]]" << endl 
            << "  Normalize mesh transforms." << endl
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

      NormalizeMeshTransforms(DynamicCast<NiTriBasedGeom>(blocks));

      WriteNifTree(outfile, root, Niflib::NifInfo(cmdLine.outver, cmdLine.uoutver, cmdLine.uoutver));
      return true;
   }
   return true;
}

REGISTER_COMMAND(NormalizeMesh, HelpString, ExecuteCmd);