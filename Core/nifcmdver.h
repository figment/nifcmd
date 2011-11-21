/**********************************************************************
*<
FILE: BaseImporter.h

DESCRIPTION:	Base Importer class

CREATED BY: tazpn (Theo)

HISTORY:

*>	Copyright (c) 2006, All Rights Reserved.
**********************************************************************/
#ifndef __NIFVERSION_H__
#define __NIFVERSION_H__

#include "config.h"

/*
* Define the major, minor, build, patch version numbers
*/
#define VERSION_MAJOR_INT  1
#define VERSION_MINOR_INT  1
#define VERSION_BUILD_INT  0

#ifndef NIFTOOLS_REVISION_INT
#define VERSION_PATCH_INT  0
#else
#define  VERSION_PATCH_INT  NIFTOOLS_REVISION_INT
#endif

#ifndef NIFTOOLS_REVISION
#define VERSION_STRING "1, 1, 0, 0"
#else
#define VERSION_STRING "1, 1, 0, " NIFTOOLS_REVISION
#endif

//#define DEF_VERSION_STRING(a,b,c,d) "##a, ##b , ##c , ##d"
//#define VERSION_STRING DEF_VERSION_STRING(a,b,c,d)

#define COPYRIGHTYEARINT 2008
#define COPYRIGHTYEAR "2008"

#define DLLNAME "NifCmd"
#define PRODUCTNAME "Nif Command Line"
#define RESFILETYPE VFT_APP

#endif
