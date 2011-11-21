#ifndef __ADDIN_H
#define __ADDIN_H

#include <string>
#include <functional>
#include <set>
#include "loki/Functor.h"

class NifCmdLine
{
public:
   NifCmdLine(int argc, char **argv, bool zeroargs=true);

   string current_file;
   string outfile;
   unsigned int outver;
   int uoutver;

   int argc;
   char **argv;

public:
   void DefaultToUnknown()
   {
      outver = VER_INVALID;
      uoutver = 0;
   }
   void DefaultToVersion(int ver, int uver = 0) {
      if (outver == VER_INVALID) {
         outver = ver;
         uoutver = uver;
      }
   }
   void DefaultToLatest()
   {
      if (outver == VER_INVALID) {
         outver = VER_20_0_0_5;
         uoutver = 11;
      }
   }

};

class NifCmd
{
   struct Less
   {
      bool operator()(const NifCmd& lhs, const NifCmd& rhs) const{
         return (stricmp(lhs.Name.c_str(), rhs.Name.c_str()) < 0);
      }
      bool operator()(const NifCmd* lhs, const NifCmd* rhs) const{
         return (stricmp(lhs->Name.c_str(), rhs->Name.c_str()) < 0);
      }
   };
   typedef std::set<NifCmd*, NifCmd::Less> NifCmdListType;
   static NifCmdListType NifCmdList;
public:

   enum HelpType {
      htShort,
      htLong
   };

   NifCmd(std::string name
      , Loki::Functor<void, LOKI_TYPELIST_1(HelpType)> helpCmd
      , Loki::Functor<bool, LOKI_TYPELIST_1(NifCmdLine&)> executeCmd)
      : Name(name)
      , HelpCmd(helpCmd)
      , ExecuteCmd(executeCmd)
   {
      NifCmdList.insert(this);
   }

   std::string Name;
   Loki::Functor<void, LOKI_TYPELIST_1(HelpType)> HelpCmd;
   Loki::Functor<bool, LOKI_TYPELIST_1(NifCmdLine&)> ExecuteCmd;

   static NifCmd* GetNifCmd(std::string name);
   static list<NifCmd*> GetNifCmds();
   static void PrintHelp();
   static bool ParseArgs(LPCTSTR line);
   static bool ParseArgs(int argc, char **argv);
   static void ParseLine(const char *start,char **argv,char *args,int *numargs,int *numchars);

};

extern std::string FormatString(const TCHAR* format,...);

#define REGISTER_COMMAND(name,help,cmd) \
   NifCmd plugin_ ## name (#name, help, cmd);

#endif //__ADDIN_H