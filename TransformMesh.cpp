#include "stdafx.h"

#include "NifCmd.h"
#include "nifutils.h"
#include "obj/NiTriBasedGeom.h"
#include "obj/NiTriBasedGeomData.h"
#include "obj/NiTriShape.h"
#include "obj/NiTriStrips.h"

using namespace std;

static Matrix33 FROMEULER( float x, float y, float z )
{
   float sinX = sin( x );
   float cosX = cos( x );
   float sinY = sin( y );
   float cosY = cos( y );
   float sinZ = sin( z );
   float cosZ = cos( z );

   Matrix33 m;
   m[0][0] = cosY * cosZ;
   m[0][1] = - cosY * sinZ;
   m[0][2] = sinY;
   m[1][0] = sinX * sinY * cosZ + sinZ * cosX;
   m[1][1] = cosX * cosZ - sinX * sinY * sinZ;
   m[1][2] = - sinX * cosY;
   m[2][0] = sinX * sinZ - cosX * sinY * cosZ;
   m[2][1] = cosX * sinY * sinZ + sinX * cosZ;
   m[2][2] = cosX * cosY;
   return m;
}

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
static void TransformMeshTransforms(vector<NiTriBasedGeomRef>& shapes, Matrix44& tm)
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

      TransformVector3(tm, verts);
      TransformVector3(tm, norms);

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
   case NifCmd::htShort: cout << "TransformMesh - Transform mesh transforms."; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " TransformMesh [-opts[modifiers]]" << endl 
            << "  Transform mesh transforms." << endl
            << endl
            << "<Options>" << endl
            << "  r \"X,Y,Z\"           Rotate by X,Y,Z in EulerAngles" << endl
            << "  t \"X,Y,Z\"           Translate by X,Y,Z" << endl
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
         case 'r': // rotate
            {
               const char *param = arg+2;
               argv[i] = NULL;
               if ( param[0] == 0 && ( i+1<argc && ( argv[i+1][0] != '-' || argv[i+1][0] != '/' ) ) ) {
                  param = argv[++i];
                  argv[i] = NULL;
               }
               if ( param[0] == 0 )
                  break;

               stringvector vals = TokenizeString(param, ",");
               if (vals.size() != 3)
               {
                  fputs( "ERROR: Invalid Rotation Argument \"", stderr );
                  fputs( param, stderr );
                  fputs( "\".\n", stderr );
                  return false;
               }
               float X = ConvertTo<float>(vals[0]);
               float Y = ConvertTo<float>(vals[1]);
               float Z = ConvertTo<float>(vals[2]);
               Matrix44 m4(Vector3(0,0,0), FROMEULER(TORAD(X),TORAD(Y),TORAD(Z)), 1.0f);
               tm *= m4;
            }
            break;
         case 't': // translate
            {
               const char *param = arg+2;
               argv[i] = NULL;
               if ( param[0] == 0 && ( i+1<argc && ( argv[i+1][0] != '-' || argv[i+1][0] != '/' ) ) ) {
                  param = argv[++i];
                  argv[i] = NULL;
               }
               if ( param[0] == 0 )
                  break;

               stringvector vals = TokenizeString(param, ",");
               if (vals.size() != 3)
               {
                  fputs( "ERROR: Invalid Tranlation Argument \"", stderr );
                  fputs( param, stderr );
                  fputs( "\".\n", stderr );
                  return false;
               }
               float X = ConvertTo<float>(vals[0]);
               float Y = ConvertTo<float>(vals[1]);
               float Z = ConvertTo<float>(vals[2]);
               Matrix44 m4(Vector3(X,Y,Z), Matrix33::IDENTITY, 1.0f);
               tm *= m4;
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
      if (!IsSupportedVersion(outver))
         outver = ver;

      // Finally alter block tree
      vector<NiObjectRef> blocks = ReadNifList( current_file );
      NiObjectRef root = blocks[0];

      TransformMeshTransforms(DynamicCast<NiTriBasedGeom>(blocks), tm);

      WriteNifTree(outfile, root, Niflib::NifInfo(cmdLine.outver, cmdLine.uoutver, cmdLine.uoutver));
      return true;
   }
   return true;
}

REGISTER_COMMAND(TransformMesh, HelpString, ExecuteCmd);