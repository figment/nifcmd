// NifTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NifCmd.h"
using namespace std;

#pragma comment(lib, "shlwapi.lib")
#ifdef _DEBUG
#  pragma comment(lib, "niflib_static_debug.lib")
#else
#  pragma comment(lib, "niflib_static.lib")
#endif

using namespace std;

#pragma warning(disable : 4073)
#pragma init_seg(lib)
NifCmd::NifCmdListType NifCmd::NifCmdList;


// sprintf for std::string without having to worry about buffer size.
std::string FormatString(const TCHAR* format,...)
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

// routine for parsing white space separated lines.  Handled like command line parameters w.r.t quotes.
void NifCmd::ParseLine (
                        const char *start,
                        char **argv,
                        char *args,
                        int *numargs,
                        int *numchars
                        )
{
   const char NULCHAR    = '\0';
   const char SPACECHAR  = ' ';
   const char TABCHAR    = '\t';
   const char RETURNCHAR = '\r';
   const char LINEFEEDCHAR = '\n';
   const char DQUOTECHAR = '\"';
   const char SLASHCHAR  = '\\';
   const char *p;
   int inquote;                    /* 1 = inside quotes */
   int copychar;                   /* 1 = copy char to *args */
   unsigned numslash;              /* num of backslashes seen */

   *numchars = 0;
   *numargs = 0;                   /* the program name at least */

   p = start;

   inquote = 0;

   /* loop on each argument */
   for(;;) 
   {
      if ( *p ) { while (*p == SPACECHAR || *p == TABCHAR || *p == RETURNCHAR || *p == LINEFEEDCHAR) ++p; }

      if (*p == NULCHAR) break; /* end of args */

      /* scan an argument */
      if (argv)
         *argv++ = args;     /* store ptr to arg */
      ++*numargs;

      /* loop through scanning one argument */
      for (;;) 
      {
         copychar = 1;
         /* Rules: 2N backslashes + " ==> N backslashes and begin/end quote
         2N+1 backslashes + " ==> N backslashes + literal "
         N backslashes ==> N backslashes */
         numslash = 0;
         while (*p == SLASHCHAR) 
         {
            /* count number of backslashes for use below */
            ++p;
            ++numslash;
         }
         if (*p == DQUOTECHAR) 
         {
            /* if 2N backslashes before, start/end quote, otherwise copy literally */
            if (numslash % 2 == 0) {
               if (inquote) {
                  if (p[1] == DQUOTECHAR)
                     p++;    /* Double quote inside quoted string */
                  else        /* skip first quote char and copy second */
                     copychar = 0;
               } else
                  copychar = 0;       /* don't copy quote */

               inquote = !inquote;
            }
            numslash /= 2;          /* divide numslash by two */
         }

         /* copy slashes */
         while (numslash--) {
            if (args)
               *args++ = SLASHCHAR;
            ++*numchars;
         }

         /* if at end of arg, break loop */
         if (*p == NULCHAR || (!inquote && (*p == SPACECHAR || *p == TABCHAR || *p == RETURNCHAR || *p == LINEFEEDCHAR)))
            break;

         /* copy character into argument */
         if (copychar) 
         {
            if (args)
               *args++ = *p;
            ++*numchars;
         }
         ++p;
      }
      /* null-terminate the argument */
      if (args)
         *args++ = NULCHAR;          /* terminate string */
      ++*numchars;
   }
   /* We put one last argument in -- a null ptr */
   if (argv)
      *argv++ = NULL;
   ++*numargs;
}

bool NifCmd::ParseArgs(LPCTSTR line)
{
   int nargs = 0, nchars = 0;
   ParseLine(line, NULL, NULL, &nargs, &nchars);
   char **largv = (char **)malloc(nargs * sizeof(char *) + nchars * sizeof(char));
   ParseLine(line, largv, ((char*)largv) + nargs * sizeof(char*), &nargs, &nchars);
   bool rv = ParseArgs(nargs, largv);
   free(largv);
   return rv;
}

void NifCmd::PrintHelp()
{
   char fullName[MAX_PATH], exeName[MAX_PATH];
   GetModuleFileName(NULL, fullName, MAX_PATH);
   _splitpath(fullName, NULL, NULL, exeName, NULL);
   cout << exeName << " built on " << __TIMESTAMP__ << endl
      << "Usage: " << exeName << " <command> [-opts[modifiers]]" << endl
      << endl
      << "<Commands>" << endl;

   for (NifCmdListType::iterator itr = NifCmdList.begin(), end = NifCmdList.end(); itr != end; ++itr){
      NifCmd* p = (*itr);
      cout << FormatString("  %-13s ", p->Name.c_str());
      p->HelpCmd(htShort);
      cout << endl;
   }
   cout << endl
      << "<Global Switches>" << endl
      << FormatString("  %-13s %s", "help", "List of additional help options") << endl
      << endl;
}

NifCmd* NifCmd::GetNifCmd(std::string name)
{
   for (NifCmdListType::iterator itr = NifCmdList.begin(), end = NifCmdList.end(); itr != end; ++itr){
      NifCmd* p = (*itr);
      if (0 == _tcsicmp(p->Name.c_str(), name.c_str())){
         return p;
      }
   }
   return NULL;
}
list<NifCmd*> NifCmd::GetNifCmds()
{
   list<NifCmd*> list;
   for (NifCmdListType::iterator itr = NifCmdList.begin(), end = NifCmdList.end(); itr != end; ++itr){
      NifCmd* p = (*itr);
      list.push_back(p);
   }
   return list;
}

bool NifCmd::ParseArgs(int argc, char **argv)
{
   bool rv = false;
   try
   {
      if (argc == 0)
      {
         PrintHelp();
         return false;
      }
      else if (argv[0] && ( 0 == _tcsicmp(argv[0], "help")) )
      {
         if (argc > 1 && argv[1] && argv[1][0])
         {
            if (NifCmd* p = GetNifCmd(argv[1])) {
               p->HelpCmd(htLong);
               return false;
            }
         }
         PrintHelp();
      }
      else
      {
         if (NifCmd* p = GetNifCmd(argv[0])) {
            NifCmdLine cmdLine(argc-1, &argv[1]);
            rv |= p->ExecuteCmd(cmdLine);
         }
         else
         {
            cerr << "Unknown command " << argv[0] <<endl;
            PrintHelp();
         }
      }
   }
   catch (exception* e)
   {
      cerr << "Exception occurred:" << endl;
      cerr << "  " << e->what() << endl;
   }
   catch (exception& e)
   {
      cerr << "Exception occurred:" << endl;
      cerr << "  " << e.what() << endl;
   }
   catch (...)
   {
      cerr << "Unknown exception occurred." << endl;
   }
   return rv;
}

NifCmdLine::NifCmdLine(int argc, char **argv, bool zeroargs)
{
   this->argc = argc;
   this->argv = argv;
   DefaultToUnknown();

   for (int i = 0; i < argc; i++)
   {
      char *arg = argv[i];
      if (arg == NULL)
         continue;
      if (arg[0] == '-' || arg[0] == '/')
      {
         switch ( tolower(arg[1]) )
         {
         case 'v':
            {
               const char *param = arg+2;
               argv[i] = NULL;
               if ( param[0] == 0 && ( i+1<argc && ( argv[i+1][0] != '-' || argv[i+1][0] != '/' ) ) ) {
                  param = argv[++i];
                  argv[i] = NULL;
               }
               if ( param[0] == 0 )
                  break;
               outver = Niflib::ParseVersionString(string(param));
            }
            break;

         case 'u':
            {
               const char *param = arg+2;
               argv[i] = NULL;
               if ( param[0] == 0 && ( i+1<argc && ( argv[i+1][0] != '-' || argv[i+1][0] != '/' ) ) ) {
                  param = argv[++i];
                  argv[i] = NULL;
               }
               if ( param[0] == 0 )
                  break;
               char *end;
               uoutver = strtol(param, &end, 0);
            }
            break;

         case 'o':
            {
               const char *param = arg+2;
               argv[i] = NULL;
               if ( param[0] == 0 && ( i+1<argc && ( argv[i+1][0] != '-' || argv[i+1][0] != '/' ) ) ) {
                  param = argv[++i];
                  argv[i] = NULL;
               }
               if ( param[0] == 0 )
                  break;
               if (outfile.empty())
               {
                  outfile = param;
               }
               else
               {
                  cerr << "ERROR: Input file already specified as \"" << current_file << "\"" << endl;
               }
            }
            break;

         case 'i':
            {
               const char *param = arg+2;
               argv[i] = NULL;
               if ( param[0] == 0 && ( i+1<argc && ( argv[i+1][0] != '-' || argv[i+1][0] != '/' ) ) ) {
                  param = argv[++i];
                  argv[i] = NULL;
               }
               if ( param[0] == 0 )
                  break;
               if (current_file.empty())
               {
                  current_file = param;
               }
               else
               {
                  cerr << "ERROR: Input file already specified as \"" << current_file << "\"" << endl;
               }
            }
            break;
         }
      }
   }
   if (!current_file.empty() && outfile.empty()) {
      char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH];
      _splitpath(current_file.c_str(), drive, dir, fname, ext);
      strcat(fname, "-out");
      _makepath(path, drive, dir, fname, ext);
      outfile = path;
   }
   DefaultToLatest();
}


int _tmain(int argc, _TCHAR* argv[])
{
   return (NifCmd::ParseArgs(argc-1, &argv[1]) ? 0 : 1);
}
