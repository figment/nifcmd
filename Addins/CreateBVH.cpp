#include "stdafx.h"
#include "NifCmd.h"
#include "nifutils.h"
#include "obj\NiExtraData.h"
#include "obj\BSBound.h"

using namespace std;

template<typename T>
inline void WriteLine(ostream out, int indent, const T& v){
   out << IndentString(indent) << v << endl;
}
template<typename T1, typename T2>
inline void WriteLine(ostream out, int indent, const T1& v1, const T2& v2) {
   out << IndentString(indent) << v1 << ' ' << v2 << endl;
}
template<typename T1, typename T2, typename T3>
inline void WriteLine(ostream out, int indent, const T1& v1, const T2& v2, const T3& v3) {
   out << IndentString(indent) << v1 << ' ' << v2 << ' ' << v3 << endl;
}
template<typename T1, typename T2, typename T3, typename T4>
inline void WriteLine(ostream out, int indent, const T1& v1, const T2& v2, const T3& v3, const T4& v4) {
   out << IndentString(indent) << v1 << ' ' << v2 << ' ' << v3 << ' ' << v4 << endl;
}
template<typename T1, typename T2, typename T3, typename T4, typename T5>
inline void WriteLine(ostream out, int indent, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) {
   out << IndentString(indent) << v1 << ' ' << v2 << ' ' << v3 << ' ' << v4 << ' ' << v5 << endl;
}
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline void WriteLine(ostream out, int indent, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) {
   out << IndentString(indent) << v1 << ' ' << v2 << ' ' << v3 << ' ' << v4 << ' ' << v5 << ' ' << v6 << endl;
}

static inline string IndentString(int depth, int nspace = 2)
{
   return string(depth*nspace, ' ');
}

static void BuildNodes(NiNodeRef object, vector<NiNodeRef>& nodes)
{
   if (!object)
      return;
   nodes.push_back(object);
   vector<NiNodeRef> links = DynamicCast<NiNode>(object->GetChildren());
   for (vector<NiNodeRef>::iterator itr = links.begin(), end = links.end(); itr != end; ++itr)
      BuildNodes(*itr, nodes);
}

static inline string GetWorldPosition(vector<NiNodeRef>& nodes, string name)
{
   if (NiNodeRef node = FindNodeByName(nodes, name.c_str())) {
      Vector3 pos = node->GetLocalTranslation();
      return FormatString("%.6f %.6f %.6f");
   }
}

static void DumpFile(ostream out, vector<NiNodeRef>& nodes)
{
   vector<NiNodeRef> bipedRoots = SelectNodesByName(nodes, "Bip??");
   std::stable_sort(bipedRoots.begin(), bipedRoots.end(), NiNodeNameEquivalence());
   for (vector<NiNodeRef>::iterator bipedItr = bipedRoots.begin(); bipedItr != bipedRoots.end(); ++bipedItr)
   {
      string bipname = (*bipedItr)->GetName();
      string match = bipname + "*";
      vector<NiNodeRef> bipedNodes = SelectNodesByName(nodes, match.c_str());

      //float angle = TORAD(bipedAngle);
      //Point3 wpos(0.0f,0.0f,0.0f);
      BOOL arms = (CountNodesByName(bipedNodes, FormatText("%s L UpperArm", bipname.c_str())) > 0) ? TRUE : FALSE;
      //BOOL triPelvis = bipedTrianglePelvis ? TRUE : FALSE;
      int nnecklinks=CountNodesByName(bipedNodes, FormatText("%s Neck*", bipname.c_str()));
      int nspinelinks=CountNodesByName(bipedNodes, FormatText("%s Spine*", bipname.c_str()));
      int nleglinks = 3 + CountNodesByName(bipedNodes, FormatText("%s L HorseLink", bipname.c_str()));
      int ntaillinks = CountNodesByName(bipedNodes, FormatText("%s Tail*", bipname.c_str()));
      int npony1links = CountNodesByName(bipedNodes, FormatText("%s Ponytail1*", bipname.c_str()));
      int npony2links = CountNodesByName(bipedNodes, FormatText("%s Ponytail2*", bipname.c_str()));
      int numfingers = CountNodesByName(bipedNodes, FormatText("%s L Finger?", bipname.c_str()));
      int nfinglinks = CountNodesByName(bipedNodes, FormatText("%s L Finger0*", bipname.c_str()));
      int numtoes = CountNodesByName(bipedNodes, FormatText("%s L Toe?", bipname.c_str()));
      int ntoelinks = CountNodesByName(bipedNodes, FormatText("%s L Toe0*", bipname.c_str()));
      BOOL prop1exists = CountNodesByName(bipedNodes, FormatText("%s Prop1", bipname.c_str())) ? TRUE : FALSE;
      BOOL prop2exists = CountNodesByName(bipedNodes, FormatText("%s Prop2", bipname.c_str())) ? TRUE : FALSE;
      BOOL prop3exists = CountNodesByName(bipedNodes, FormatText("%s Prop3", bipname.c_str())) ? TRUE : FALSE;
      int forearmTwistLinks = CountNodesByName(bipedNodes, FormatText("%s L Fore*Twist*", bipname.c_str()));
      int upperarmTwistLinks = CountNodesByName(bipedNodes, FormatText("%s L Up*Twist*", bipname.c_str()));
      int thighTwistLinks = CountNodesByName(bipedNodes, FormatText("%s L Thigh*Twist*", bipname.c_str()));
      int calfTwistLinks = CountNodesByName(bipedNodes, FormatText("%s L Calf*Twist*", bipname.c_str()));
      int horseTwistLinks = CountNodesByName(bipedNodes, FormatText("%s L Horse*Twist*", bipname.c_str()));

      int lvl = 0;
      WriteLine(out, lvl, "HIERARCHY" );
      WriteLine(out, lvl, "ROOT", "Hips");
      WriteLine(out, lvl++, "{" );
      WriteLine(out, lvl, "OFFSET", GetWorldPosition(nodes, FormatText("%s Pelvis", bipname.c_str())));
      WriteLine(out, lvl, "CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation" );
      {
         WriteLine(out, lvl, "JOINT", "Chest");
         WriteLine(out, lvl++, "{" );
         WriteLine(out, lvl, "OFFSET", GetWorldPosition(nodes, FormatText("%s Spine", bipname.c_str())));
         WriteLine(out, lvl, "CHANNELS 3 Zrotation Xrotation Yrotation" );
         for (int i=1; i<nspinelinks; i++) {
            WriteLine(out, lvl, "JOINT", FormatText("Chest%d", i+1));
            WriteLine(out, lvl++, "{" );
            WriteLine(out, lvl, "OFFSET", GetWorldPosition(nodes, FormatText("%s Spine%d", bipname.c_str(), i)));
            WriteLine(out, lvl, "CHANNELS 3 Zrotation Xrotation Yrotation" );
         }
         {
            WriteLine(out, lvl, "JOINT", "Neck");
            WriteLine(out, lvl++, "{" );
            WriteLine(out, lvl, "OFFSET", GetWorldPosition(nodes, FormatText("%s Neck", bipname.c_str())));
            WriteLine(out, lvl, "CHANNELS 3 Zrotation Xrotation Yrotation" );
            {
               WriteLine(out, lvl, "JOINT", "Head");
               WriteLine(out, lvl++, "{" );
               WriteLine(out, lvl, "OFFSET", GetWorldPosition(nodes, FormatText("%s Neck1", bipname.c_str())));
               WriteLine(out, lvl, "CHANNELS 3 Zrotation Xrotation Yrotation" );
               {
                  WriteLine(out, lvl, "End Site");
                  WriteLine(out, lvl++, "{" );
                  WriteLine(out, lvl, "OFFSET", GetWorldPosition(nodes, FormatText("%s Head", bipname.c_str())));
                  WriteLine(out, --lvl, "}" );
               }
               WriteLine(out, --lvl, "}" );
            }
            WriteLine(out, --lvl, "}" );

            WriteLine(out, lvl, "JOINT", "LeftCollar");
            WriteLine(out, lvl++, "{" );
            WriteLine(out, lvl, "OFFSET", GetWorldPosition(nodes, FormatText("%s L Clavicle", bipname.c_str())));
            WriteLine(out, lvl, "CHANNELS 3 Zrotation Xrotation Yrotation" );
            {
               WriteLine(out, lvl, "JOINT", "LeftUpArm");
               WriteLine(out, lvl++, "{" );
               WriteLine(out, lvl, "OFFSET", GetWorldPosition(nodes, FormatText("%s L UpperArm", bipname.c_str())));
               WriteLine(out, lvl, "CHANNELS 3 Zrotation Xrotation Yrotation" );
               {
                  WriteLine(out, lvl, "JOINT", "LeftLowArm");
                  WriteLine(out, lvl++, "{" );
                  WriteLine(out, lvl, "OFFSET", GetWorldPosition(nodes, FormatText("%s L Forearm", bipname.c_str())));
                  WriteLine(out, lvl, "CHANNELS 3 Zrotation Xrotation Yrotation" );
                  {
                     WriteLine(out, lvl, "JOINT", "LeftHand");
                     WriteLine(out, lvl++, "{" );
                     WriteLine(out, lvl, "OFFSET", GetWorldPosition(nodes, FormatText("%s L Hand", bipname.c_str())));
                     WriteLine(out, lvl, "CHANNELS 3 Zrotation Xrotation Yrotation" );

                     for (int i=0; i<numfingers; i++) {
                        WriteLine(out, lvl, "JOINT", FormatText("Finger%d", i+1));
                        WriteLine(out, lvl++, "{" );
                        WriteLine(out, lvl, "OFFSET", GetWorldPosition(nodes, FormatText("%s Finger%d", bipname.c_str(), i)));
                        WriteLine(out, lvl, "CHANNELS 3 Zrotation Xrotation Yrotation" );
                        for (int j=1; j<nfinglinks; j++)
                        {
                           WriteLine(out, lvl, "JOINT", FormatText("Finger%d%d", i+1, j));
                           WriteLine(out, lvl++, "{" );
                           WriteLine(out, lvl, "OFFSET", GetWorldPosition(nodes, FormatText("%s L Hand", bipname.c_str())));
                           WriteLine(out, lvl, "CHANNELS 3 Zrotation Xrotation Yrotation" );

                           WriteLine(out, --lvl, "}" );
                        }
                     }
                     {
                     }
                     for (int i=0; i<numfingers; i++)
                        WriteLine(out, --lvl, "}" );

                     WriteLine(out, --lvl, "}" );
                  }
                  WriteLine(out, --lvl, "}" );
               }
               WriteLine(out, --lvl, "}" );
            }
            WriteLine(out, --lvl, "}" );
         }
         for (int i=0; i<nspinelinks; i++)
            WriteLine(out, --lvl, "}" );
      }
      WriteLine(out, --lvl, "}" );

      WriteLine(out, lvl, "MOTION" );
      WriteLine(out, lvl, "Frames: 0" );
   }
}


static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "CreateBVH - Create BVH file for Biped imports"; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " CreateBVH <input> [-opts[modifiers]]" << endl 
            << "  Create BVH file for Biped Skeleton imports " << endl
            << endl
            << "<Switches>" << endl
            << "  i <path>          Input File" << endl
            << "  o <path>          Output File - Defaults to input file with .BVH extension" << endl
            << "  v x.x.x.x         Nif Version to write as - Defaults to input version" << endl
            << endl
            ;
      }
      break;
   }
}

static bool IsBiped(NiObjectRef root)
{
   NiNodeRef rootNode = root;
   if (rootNode){
      list<NiExtraDataRef> extraData = rootNode->GetExtraData();
      if (!extraData.empty()) {
         return ( SelectFirstObjectOfType<BSBound>(extraData) != NULL );
      }
   }
   return false;
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
         switch ( tolower(arg[1]) )
         {
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
      else if (outfile.empty())
      {
         outfile = arg;
      }
      else
      {
         cerr << "ERROR: Input and output files already specified." << endl;
         return false;
      }
   }
   if (current_file.empty()){
      NifCmd::PrintHelp();
      return false;
   }
   if (outfile.empty()) {
      char path[MAX_PATH];
      _tcscpy(path, current_file.c_str());
      PathRenameExtension(path, ".BVH");
      outfile = path;
   }
   unsigned int ver = CheckNifHeader( current_file );
   if ( ver == VER_UNSUPPORTED ) cout << "unsupported...";
   else if ( ver == VER_INVALID ) cout << "invalid...";
   else 
   {
      if (!IsVersionSupported(outver))
         outver = ver;

      vector<NiObjectRef> blocks = ReadNifList( current_file );
      NiObjectRef root = blocks[0];
      if (!IsBiped(root)){
         cerr << "File does not contain a valid skeleton." << endl;
      } else  {
         vector<NiNodeRef> nodes;

         ofstream out(outfile.c_str());
         BuildNodes(NiNodeRef(root), nodes);
         DumpFile(out, nodes);
      }
      return true;
   }
   return false;
}

REGISTER_COMMAND(CreateBVH, HelpString, ExecuteCmd);