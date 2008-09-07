#include "stdafx.h"

#include "NifCmd.h"
#include "obj/NiTriBasedGeom.h"
#include "obj/NiTriBasedGeomData.h"
#include "obj/NiTriShape.h"
#include "obj/NiTriStrips.h"
#include "obj/bhkRigidBody.h"

using namespace std;


// sprintf for std::string without having to worry about buffer size.
static std::string FormatString(const TCHAR* format,...)
{
	TCHAR buffer[512];
	std::string text;
	va_list args;
	va_start(args, format);
	int nChars = _vsntprintf(buffer, _countof(buffer), format, args);
	if (nChars != -1) {
		text = buffer;
	} else {
		size_t Size = _vsctprintf(format, args);
		TCHAR* pbuf = (TCHAR*)_alloca(Size);
		nChars = _vsntprintf(pbuf, Size, format, args);
		text = pbuf;
	}
	va_end(args);
	return text;
}


static void CalcRigidBodyProperties(vector<bhkRigidBodyRef>& items)
{
	for (vector<bhkRigidBodyRef>::iterator itr = items.begin(); itr != items.end(); ++itr)
	{
		bhkRigidBodyRef item = (*itr);
		float mass = item->GetMass();
		Vector4 com = item->GetCenter();
		InertiaMatrix I = item->GetInertia();

		printf("Before: bhkRigidBody %g <%g,%g,%g,%g> [[%g,%g,%g,%g],[%g,%g,%g,%g],[%g,%g,%g,%g]]\n"
			, mass
			, com[0], com[1], com[2], com[3]
			, I[0][0], I[0][1], I[0][2], I[0][3]
			, I[1][0], I[1][1], I[1][2], I[1][3]
			, I[2][0], I[2][1], I[2][2], I[2][3]
		);
		item->UpdateMassCenterInertia(1.0f, true, item->GetMass());

		mass = item->GetMass();
		com = item->GetCenter();
		I = item->GetInertia();

		printf("After:  bhkRigidBody %g <%g,%g,%g,%g> [[%g,%g,%g,%g],[%g,%g,%g,%g],[%g,%g,%g,%g]]\n"
			, mass
			, com[0], com[1], com[2], com[3]
			, I[0][0], I[0][1], I[0][2], I[0][3]
			, I[1][0], I[1][1], I[1][2], I[1][3]
			, I[2][0], I[2][1], I[2][2], I[2][3]
		);

	}
}

static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "UpdateMassProps - Update Rigid Body mass properties."; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " UpdateMassProps [-opts[modifiers]]" << endl 
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
   bool readonly = false;

   list<NifCmd *> plugins;

   for (int i = 0; i < argc; i++)
   {
      char *arg = argv[i];
      if (arg == NULL)
         continue;
      if (arg[0] == '-' || arg[0] == '/')
      {
		  if (tolower(arg[1]) == 'r')
			  readonly = true;
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

      CalcRigidBodyProperties(DynamicCast<bhkRigidBody>(blocks));

	  if (!readonly)
	      WriteNifTree(outfile, root, Niflib::NifInfo(cmdLine.outver, cmdLine.uoutver, cmdLine.uoutver));
      return true;
   }
   return true;
}

REGISTER_COMMAND(UpdateMassProps, HelpString, ExecuteCmd);