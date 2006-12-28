#include "stdafx.h"
#include "NifCmd.h"
#include "NifUtils.h"
using namespace std;

static void BuildImageTable(NameValueCollection & collection, const char *root, const char *path)
{
   char buffer[MAX_PATH], buffer2[MAX_PATH], search[MAX_PATH];
   WIN32_FIND_DATA FindFileData;
   HANDLE hFind;
   ZeroMemory(&FindFileData, sizeof(FindFileData));
   if (path == NULL || path[0] == 0)
      return;
   strcpy(search, path);
   PathAddBackslash(search);
   strcat(search, "*");

   hFind = FindFirstFile(search, &FindFileData);
   if (hFind != INVALID_HANDLE_VALUE) 
   {
      stringlist list;
      for (BOOL ok = TRUE ; ok ; ok = FindNextFile(hFind, &FindFileData))
      {
         if (FindFileData.cFileName[0] == '.' || (FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
            continue;
         if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
         {
            PathCombine(buffer, path, FindFileData.cFileName);
            PathAddBackslash(buffer);
            list.push_back(buffer);
         }
         else
         {
            LPCSTR ext = PathFindExtension(FindFileData.cFileName);
            if (stricmp(ext, ".dds") == 0)
            {
               strcpy(buffer, FindFileData.cFileName);
               PathRemoveExtension(buffer);

               if (collection.find(buffer) == collection.end()) {
                  PathCombine(buffer, path, FindFileData.cFileName);
                  GetLongPathName(buffer, buffer, MAX_PATH);
                  PathRemoveExtension(FindFileData.cFileName);
                  PathRelativePathTo(buffer2, root, FILE_ATTRIBUTE_DIRECTORY, buffer, FILE_ATTRIBUTE_NORMAL);
                  char *p = buffer2; while (*p == '\\') ++p;
                  collection.insert(KeyValuePair(FindFileData.cFileName, p));					
               }
            }
         }
      }
      FindClose(hFind);
      for (stringlist::iterator itr = list.begin(), end = list.end(); itr != end; ++itr)
      {
         BuildImageTable(collection, root, (*itr).c_str());
      }
   }
}

bool FixImages(vector<NiObjectRef>& blocks, NameValueCollection& imgTable)
{
   vector<NiSourceTextureRef> texs = DynamicCast<NiSourceTexture>(blocks);

   bool rv = false;
   for (vector<NiSourceTextureRef>::iterator itr = texs.begin(), end = texs.end(); itr != end; ++itr)
   {
      NiSourceTextureRef& tex = (*itr);
      if (tex->IsTextureExternal())
      {
         string fname = tex->GetExternalFileName();

         char buffer[MAX_PATH];
         strcpy(buffer, fname.c_str());
         PathRemoveExtension(buffer);
         NameValueCollection::iterator itr = imgTable.find(buffer);
         if (itr != imgTable.end()){
            tex->SetExternalTexture((*itr).second, tex->GetExternalUnknownLink());
            rv = true;
         } else {
            for (string::iterator sitr = fname.begin(); sitr != fname.end(); ) {
               if (*sitr == '_') {
                  sitr = fname.erase(sitr);
               } else {
                  ++sitr;
               }
            }
            strcpy(buffer, fname.c_str());
            PathRemoveExtension(buffer);
            NameValueCollection::iterator itr = imgTable.find(buffer);
            if (itr != imgTable.end()){
               tex->SetExternalTexture((*itr).second, tex->GetExternalUnknownLink());
               rv = true;
            }
         }
      }
   }
   return rv;
}

static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "Fix texture paths"; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " FixTex <input> [-opts[modifiers]]" << endl 
            << "  Attempts to fix texture references by search for similarly named " << endl
            << "    textures in the specified root path or additional paths." << endl
            << endl
            << "<Switches>" << endl
            << "  i <path>          Input File" << endl
            << "  o <path>          Output File - Defaults to input file with '-out' appended" << endl
            << "  r <path>          Root Path - Defaults to Oblivion\\Data from registry" << endl
            << "  p [path1;path2]   Additional Search Paths to look in" << endl
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

   NameValueCollection imgTable;
   list<string> searchpaths;
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
         case 'p':
            {
               const char *param = arg+2;
               if ( param[0] == 0 && ( i+1<argc && ( argv[i+1][0] != '-' || argv[i+1][0] != '/' ) ) )
                  param = argv[++i];
               if ( param[0] == 0 )
                  break;
               char *p2 = strdup(param);
               for( const char *p = strtok(p2, ";")
                  ; p && *p
                  ; p = strtok(NULL, ";")
                  )
               {
                  searchpaths.push_back(p);
               }
               free(p2);
            }
            break;

         case 'r':
            {
               const char *param = arg+2;
               if ( param[0] == 0 && ( i+1<argc && ( argv[i+1][0] != '-' || argv[i+1][0] != '/' ) ) )
                  param = argv[++i];
               if ( param[0] == 0 )
                  break;
               if (rootPath.empty())
               {
                  rootPath = param;
               }
               else
               {
                  cerr << "ERROR: Root path already specified as \"" << rootPath << "\"" << endl;
                  return false;
               }
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
   unsigned int ver = GetNifVersion( current_file );
   if ( ver == VER_UNSUPPORTED ) cout << "unsupported...";
   else if ( ver == VER_INVALID ) cout << "invalid...";
   else 
   {
      if (!IsSupportedVersion(cmdLine.outver)) {
         cerr << "Invalid version" << endl;
         return false;
      }

      if (rootPath.empty()){
         NameValueCollection images;
         char regRootPath[MAX_PATH];
         regRootPath[0] = 0;
         HKEY hKey = NULL;
         if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Bethesda Softworks\\Oblivion", 0, KEY_READ, &hKey))
         {
            DWORD dwLen = MAX_PATH;
            regRootPath[0] = 0;
            if (ERROR_SUCCESS == RegQueryValueEx(hKey, "Installed Path", NULL, NULL, (LPBYTE)regRootPath, &dwLen) && dwLen > 0)
            {
            }
            RegCloseKey(hKey);
         }
         if (!regRootPath[0]){
            cerr << "ERROR: Could not locate a valid root path to search." << endl;
            return false;
         }
         PathAppend(regRootPath, "Data");
         PathAddBackslash(regRootPath);

         char texPath[MAX_PATH];
         PathCombine(texPath, regRootPath, "Textures");
         PathAddBackslash(texPath);

         rootPath = regRootPath;
         BuildImageTable(imgTable, rootPath.c_str(), texPath);
      } else {
         BuildImageTable(imgTable, rootPath.c_str(), rootPath.c_str());
      }
      for (list<string>::iterator itr = searchpaths.begin(), end = searchpaths.end(); itr != end; ++itr) {
         if (PathIsRelative((*itr).c_str()))
         {
            char texPath[MAX_PATH];
            PathCombine(texPath, rootPath.c_str(), (*itr).c_str());
            PathAddBackslash(texPath);
            BuildImageTable(imgTable, rootPath.c_str(), texPath);
         }
         else
         {
            BuildImageTable(imgTable, rootPath.c_str(), (*itr).c_str());
         }
      }

      // Finally alter block tree
      vector<NiObjectRef> blocks = ReadNifList( current_file );
      NiObjectRef root = blocks[0];

      FixImages(blocks, imgTable);
      WriteNifTree(outfile, root, Niflib::NifInfo(cmdLine.outver, cmdLine.uoutver, cmdLine.uoutver));
      return true;
   }
   return false;
}

REGISTER_COMMAND(FixTex, HelpString, ExecuteCmd);