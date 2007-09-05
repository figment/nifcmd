#include "stdafx.h"

#include "NifCmd.h"
#include "obj/NiBSplineCompTransformInterpolator.h"
#include "obj/NiBSplineData.h"
#include "obj/NiBSplineBasisData.h"
#include "obj/NiControllerSequence.h"
#include "obj/NiStringPalette.h"
using namespace std;

static bool dumpControl = false;

static inline float TODEG(float x) { return x * 180.0f / PI; }
static inline float TORAD(float x) { return x * PI / 180.0f; }

const float FramesPerSecond = 30.0f;
const float FramesIncrement = 1.0f/30.0f;
inline bool ISNAN(float f) { 
   return ((*(unsigned int*)&f) == 0x7f7fffff); 
}

void DumpBSpline(NiBSplineCompTransformInterpolatorRef spline, string target)
{
   float start = spline->GetStartTime();
   float stop = spline->GetStopTime();

   int nframes = int((stop-start) / FramesIncrement);

   Ref<NiBSplineData > data = spline->GetSplineData();
   Ref<NiBSplineBasisData > basis = spline->GetBasisData();
   int nctrl = basis->GetNumControlPt();

   cout.setf(ios::fixed, ios::floatfield);
   cout << setprecision(3); // << setiosflags(ios_base::showpos);

   if (dumpControl)
   {
      vector<short > control = data->GetShortControlPoints();
      cout << "Control data for " << target << endl
           << "Control Pt: " << control.size() << endl;
      int j=0;
      cout << "0" << '\t';
      for (int i=0, n=control.size(); i<n; ++i, ++j) {
         //cout << float(control[i]) / float (32767) << '\t';
         cout << i << '\t' << control[i] << endl;
      }
      cout << endl;

      return;
   }

   {
      vector<Vector3> control = spline->GetTranslateControlData();
      if (!control.empty()) {

         float mult = spline->GetTranslateMultiplier();
         float bias = spline->GetTranslateBias();

         cout << "Translation data for " << target << endl
            << "Control Pt: " << control.size() << endl
            << "Mult: " << mult << endl
            << "Bias: " << bias << endl
            << endl ;

         for (int i=0, n=control.size(); i<n; ++i){
            Vector3 xyz = control[i];
            cout << i << "\t[" << xyz.x << ",\t" << xyz.y << ",\t" << xyz.z << "]" << endl;
         }

         cout << endl;

         int npoints = control.size() * 2 + 1;
         vector< Key<Vector3> > keys = spline->SampleTranslateKeys(npoints, 3);
         for (int i=0, n=keys.size(); i<n; ++i){
            Vector3 xyz = keys[i].data;
            cout << i << "\t[" << xyz.x << ",\t" << xyz.y << ",\t" << xyz.z << "]" << endl;
         }
         cout << endl;
      }
   }

   {
      vector<Quaternion> control = spline->GetQuatRotateControlData();
      if (!control.empty()) {

         float mult = spline->GetRotationMultiplier();
         float bias = spline->GetRotationBias();

         cout << "Quaternion rotation data for " << target << endl
              << "Control Pt: " << control.size() << endl
              << "Mult: " << mult << endl
              << "Bias: " << bias << endl
              << endl ;

         for (int i=0, n=control.size(), j=1; i<n; ++i, ++j){
            Quaternion q = control[i];
            Float3 ypr = q.AsEulerYawPitchRoll();
               cout << i << "\t<" << q.w << ",\t" << q.x << ",\t" << q.y << ",\t" << q.z << ">"
                  << "\t[" << TODEG(ypr[0]) << ",\t" << TODEG(ypr[1]) << ",\t" << TODEG(ypr[2]) << "]" 
                  << endl;
            if (j==nctrl)
            {
               cout << endl;
               j = 0;
            }
         }

         cout << endl;

         int npoints = control.size() * 2 + 1;
         vector< Key<Quaternion> > keys = spline->SampleQuatRotateKeys(npoints, 3);
         for (int i=0, n=keys.size(); i<n; ++i){
            Quaternion q = keys[i].data;
            Float3 ypr = q.AsEulerYawPitchRoll();
            cout << i << "\t<" << q.w << ",\t" << q.x << ",\t" << q.y << ",\t" << q.z << ">"
               << "\t[" << TODEG(ypr[0]) << ",\t" << TODEG(ypr[1]) << ",\t" << TODEG(ypr[2]) << "]" 
               << endl;
         }
         cout << endl;
      }
   }
   {
      vector<float> control = spline->GetScaleControlData();
      if (!control.empty()) {

         float mult = spline->GetScaleMultiplier();
         float bias = spline->GetScaleBias();

         cout << "Scale data for " << target << endl
            << "Control Pt: " << control.size() << endl
            << "Mult: " << mult << endl
            << "Bias: " << bias << endl
            << endl ;

         for (int i=0, n=control.size(); i<n; ++i){
            float s = control[i];
            cout << i << "\t" << s << endl;
         }

         cout << endl;

         int npoints = control.size() * 2 + 1;
         vector< Key<float> > keys = spline->SampleScaleKeys(npoints, 3);
         for (int i=0, n=keys.size(); i<n; ++i){
            float s = keys[i].data;
            cout << i << "\t" << s << endl;
         }
         cout << endl;
      }
   }
   //extern void bspline(int n, int t, int l, float *control, float *output, int num_output);
   //
   //Float4 *p = new Float4[nctrl];
   //for (int i=0, j=0;i<nctrl; ++i) {
   //   p[i][0] = float(points[j++]) / float (32767);
   //   p[i][1] = float(points[j++]) / float (32767);
   //   p[i][2] = float(points[j++]) / float (32767);
   //   p[i][3] = float(points[j++]) / float (32767);

   //   float w = float(p[i][0]) * mult + bias;
   //   float x = float(p[i][1]) * mult + bias;
   //   float y = float(p[i][2]) * mult + bias;
   //   float z = float(p[i][3]) * mult + bias;

   //   Quaternion q (w,z,x,y);
   //   Float3 ypr = q.AsEulerYawPitchRoll();

   //   cout << i 
   //      << "\t" << w 
   //      << "\t" << x 
   //      << "\t" << y 
   //      << "\t" << z 
   //      << "\t" << TODEG(ypr[0])
   //      << "\t" << TODEG(ypr[1])
   //      << "\t" << TODEG(ypr[2])
   //      << endl;
   //}
   //cout << endl;
   //int res = (nframes+1)*2+1;
   //Float4 *out = new Float4[res];
   //bspline(nctrl-1, 4, 4, &p[0][0], &out[0][0], res);
   //for (int i = 0; i < res; ++i)
   //{
   //   float fT = ((float)i)/((float)res-1) * (stop-start) * FramesPerSecond;
   //   float w = float(out[i][0]) * mult + bias;
   //   float x = float(out[i][1]) * mult + bias;
   //   float y = float(out[i][2]) * mult + bias;
   //   float z = float(out[i][3]) * mult + bias;

   //   Quaternion q (w,z,x,y);
   //   Float3 ypr = q.AsEulerYawPitchRoll();

   //   cout << i 
   //      << "\t" << fT
   //      << "\t" << w 
   //      << "\t" << x 
   //      << "\t" << y 
   //      << "\t" << z 
   //      << "\t" << TODEG(ypr[0])
   //      << "\t" << TODEG(ypr[1])
   //      << "\t" << TODEG(ypr[2])
   //      << endl;
   //}
}

static void DumpBSplines(vector<NiBSplineCompTransformInterpolatorRef>& blocks)
{
   for ( vector<NiBSplineCompTransformInterpolatorRef>::iterator itr = blocks.begin()
       ; itr != blocks.end()
       ; ++itr 
       )
   {
      DumpBSpline(*itr, "");
   }
}
static void DumpControllerSequences(vector<NiControllerSequenceRef>& refs)
{
   for ( vector<NiControllerSequenceRef>::iterator itr = refs.begin(); itr != refs.end(); ++itr )
   {
      vector<ControllerLink> data = (*itr)->GetControllerData();
      for (size_t i=0; i<data.size(); ++i)
      {
         ControllerLink& link = data[i];
         string target = link.nodeName;
         if (target.empty())
            target = link.stringPalette->GetSubStr(link.nodeNameOffset);
         if (NiBSplineCompTransformInterpolatorRef spline = link.interpolator)
            DumpBSpline(spline, target);
      }
   }
}

static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "Dump BSpline curves"; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " BSplines <input> [-opts[modifiers]]" << endl 
            << "  Dump BSplines in the interpolators." << endl
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
   int argc = cmdLine.argc;
   char **argv = cmdLine.argv;

   for (int i = 0; i < argc; i++)
   {
      char *arg = argv[i];
      if (arg == NULL)
         continue;
      if (arg[0] == '-' || arg[0] == '/')
      {
         if (tolower(arg[1]) == 'c')
         {
            dumpControl = true;
         }
         else
         {
            fputs( "ERROR: Unknown argument specified \"", stderr );
            fputs( arg, stderr );
            fputs( "\".\n", stderr );
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
   unsigned int ver = GetNifVersion( current_file );
   if ( ver == VER_UNSUPPORTED ) cout << "unsupported...";
   else if ( ver == VER_INVALID ) cout << "invalid...";
   else 
   {
      // Finally alter block tree
      vector<NiObjectRef> blocks = ReadNifList( current_file );
      NiObjectRef root = blocks[0];

      vector<NiBSplineCompTransformInterpolatorRef> brefs = DynamicCast<NiBSplineCompTransformInterpolator>(blocks);
      if (brefs.empty()) {
         cerr << "No BSplines found in " << current_file << endl;
         return false;
      }

      vector<NiControllerSequenceRef> refs = DynamicCast<NiControllerSequence>(blocks);
      if (!refs.empty())
         DumpControllerSequences(refs);
      else
         DumpBSplines(brefs);
      return true;
   }
   return false;
}

REGISTER_COMMAND(BSpline, HelpString, ExecuteCmd);