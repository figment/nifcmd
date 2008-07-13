#include "stdafx.h"
#include "nifutils.h"

// Macro to create a dynamically allocated strdup on the stack
#define STRDUPA(p) (_tcscpy((TCHAR*)alloca((_tcslen(p)+1)*sizeof(*p)),p))

extern int wildcmp(const TCHAR *wild, const TCHAR *string);
extern int wildcmpi(const TCHAR *wild, const TCHAR *string);

// Original Source: Jack Handy www.codeproject.com
int wildcmp(const TCHAR *wild, const TCHAR *string) {
   const TCHAR *cp, *mp;

   while ((*string) && (*wild != '*')) {
      if ((*wild != *string) && (*wild != '?')) {
         return 0;
      }
      wild++;
      string++;
   }

   while (*string) {
      if (*wild == '*') {
         if (!*++wild) {
            return 1;
         }
         mp = wild;
         cp = string+1;
      } else if ((*wild == *string) || (*wild == '?')) {
         wild++;
         string++;
      } else {
         wild = mp;
         string = cp++;
      }
   }

   while (*wild == '*') {
      wild++;
   }
   return !*wild;
}

// Same as above but case insensitive
int wildcmpi(const TCHAR *wild, const TCHAR *string) {
   const TCHAR *cp, *mp;
   int f,l;
   while ((*string) && (*wild != '*')) {
      f = _totlower( (_TUCHAR)(*string) );
      l = _totlower( (_TUCHAR)(*wild) );
      if ((f != l) && (l != '?')) {
         return 0;
      }
      wild++, string++;
   }
   while (*string) {
      if (*wild == '*') {
         if (!*++wild) return 1;
         mp = wild, cp = string+1;
      } else {
         f = _totlower( (_TUCHAR)(*string) );
         l = _totlower( (_TUCHAR)(*wild) );
         if ((f == l) || (l == '?')) {
            wild++, string++;
         } else {
            wild = mp, string = cp++;
         }
      }
   }
   while (*wild == '*') wild++;
   return !*wild;
}

bool wildmatch(const string& match, const std::string& value) 
{
   return (wildcmpi(match.c_str(), value.c_str())) ? true : false;
}

bool wildmatch(const stringlist& matches, const std::string& value)
{
   for (stringlist::const_iterator itr=matches.begin(), end=matches.end(); itr != end; ++itr){
      if (wildcmpi((*itr).c_str(), value.c_str()))
         return true;
   }
   return false;
}


// Search NiNode collection for a specific name
NiNodeRef FindNodeByName( const vector<NiNodeRef>& blocks, const string& name )
{
   for (vector<NiNodeRef>::const_iterator itr = blocks.begin(), end = blocks.end(); itr != end; ++itr)
   {
      const NiNodeRef& block = (*itr);
      if (name == block->GetName())
         return block;
   }
   return NiNodeRef();
}

// Search NiNode collection names that match a wildcard 
vector<NiNodeRef> SelectNodesByName( const vector<NiNodeRef>& blocks, LPCTSTR match)
{
   vector<NiNodeRef> nodes;
   for (vector<NiNodeRef>::const_iterator itr = blocks.begin(), end = blocks.end(); itr != end; ++itr)
   {
      const NiNodeRef& block = (*itr);
      if (wildcmpi(match, block->GetName().c_str()))
         nodes.insert(nodes.end(), block);
   }
   return nodes;
}

// Count number of NiNodes that match a wildcard 
int CountNodesByName( const vector<NiNodeRef>& blocks, LPCTSTR match )
{
   int count = 0;
   for (vector<NiNodeRef>::const_iterator itr = blocks.begin(), end = blocks.end(); itr != end; ++itr) {
      const NiNodeRef& block = (*itr);
      if (wildcmpi(match, block->GetName().c_str()))
         ++count;
   }
   return count;
}

// Get a vector of names from an NiNode vector
vector<string> GetNamesOfNodes( const vector<Niflib::NiNodeRef>& nodes )
{
   vector<string> slist;
   for (vector<NiNodeRef>::const_iterator itr = nodes.begin(), end = nodes.end(); itr != end; ++itr) {
      const NiNodeRef& block = (*itr);
      slist.push_back(block->GetName());
   }
   return slist;
}

// Tokenize a string using strtok and return it as a stringlist
stringvector TokenizeString(LPCTSTR str, LPCTSTR delims)
{
   stringvector values;
   LPTSTR buf = STRDUPA(str);
   for (LPTSTR p = _tcstok(buf, delims); p && *p; p = _tcstok(NULL, delims)){
      values.push_back(string(p));
   }
   return values;
}
